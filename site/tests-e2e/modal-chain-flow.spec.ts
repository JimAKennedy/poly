import { test, expect, Frame, Page } from '@playwright/test';

// M044 S07 — Modal Scene Chain flow regression gate.
//
// Guards against two bugs reported after M044 S06 Morph fix landed. With scenes
// A and B both loaded, opening the Try It modal's chain popover and toggling
// Enable did nothing audible AND the popover appeared to close before the click
// registered. Two independent causes:
//
//   1. webui/wasm-host.js — chain edits (chain.enabled, chain.mode,
//      chain.entry.*, chainAddEntry, chainRemoveEntry) mutated the JS shadow
//      only; the WASM engine's SceneChain buffer was never updated, and
//      poly_render's chain-driven scene override never fired. The plugin's
//      processor.cpp:492-523 pushes edits into the engine synchronously — the
//      modal host was missing the equivalent bridge. Added
//      poly_set_chain_enabled / mode / entry_scene / entry_bars, poly_chain_add
//      / remove_entry, and route poly_render through Context::chainState so the
//      chain actually drives scenes.select each block.
//   2. webui/ui.js — buildChainPopover() runs on every state emit
//      (renderChrome:848) to refresh the visible on/off/count. Each rebuild
//      registered a fresh `dismiss` handler on document but never removed the
//      prior one (the old dismiss captured the removed `pop`, so it saw the
//      user's next click as "outside" the DOM element it remembered and shut
//      the popover immediately). Same trap on the note-map modal. Fix: track
//      the current dismiss handler in a module-scoped `chainDismiss` /
//      `noteMapDismiss` and detach it in closeXxx / on rebuild.
//
// This spec locks in the observable behavior:
//   - Clicking chainBtn opens the popover.
//   - Clicking Enable inside the popover flips chain.enabled AND leaves the
//     popover in the DOM (no dismiss stacking).
//   - The engine mirror (Module._poly_chain_enabled(engineCtx), reported via
//     probe.engineChainEnabled) matches the JS shadow.
//   - Adding an entry increments both counts.

const PAGE_PATH = '/poly/13-drum-and-bass/';
const CARD_SELECTOR = '.poly-preview[data-poly-preset="Liquid Drum and Bass"]';

interface ChainProbe {
  engineChainEnabled: number;
  engineChainEntryCount: number;
}

async function openTryItModal(page: Page, cardSelector: string): Promise<Frame> {
  const card = page.locator(cardSelector);
  await expect(card, `card missing: ${cardSelector}`).toBeVisible();
  await card.locator('[data-role="tryit"]').click();
  const dialog = page.locator('dialog.poly-modal');
  await dialog.waitFor({ state: 'attached', timeout: 5000 });
  const iframeHandle = await dialog.locator('iframe').elementHandle();
  expect(iframeHandle, 'modal: iframe handle missing').not.toBeNull();
  const frame = await iframeHandle!.contentFrame();
  expect(frame, 'modal: iframe contentFrame missing').not.toBeNull();
  await frame!.waitForLoadState('networkidle', { timeout: 15_000 });
  await frame!.waitForFunction(
    () => {
      const w = window as unknown as {
        PolyWasmHost?: { getState?: () => unknown };
      };
      return !!(w.PolyWasmHost && w.PolyWasmHost.getState);
    },
    null,
    { timeout: 15_000 },
  );
  return frame!;
}

async function readChainState(frame: Frame): Promise<{ enabled: boolean; entryCount: number }> {
  return (await frame.evaluate(() => {
    const w = window as unknown as {
      PolyWasmHost?: {
        getState?: () => { chain?: { enabled?: boolean; entryCount?: number } };
      };
    };
    const s = w.PolyWasmHost?.getState?.();
    return {
      enabled: !!s?.chain?.enabled,
      entryCount: s?.chain?.entryCount ?? 0,
    };
  })) as { enabled: boolean; entryCount: number };
}

async function readChainProbe(frame: Frame): Promise<ChainProbe> {
  return (await frame.evaluate(() => {
    const p = (window as unknown as { __polyAudioProbe?: ChainProbe }).__polyAudioProbe;
    return {
      engineChainEnabled: p?.engineChainEnabled ?? -1,
      engineChainEntryCount: p?.engineChainEntryCount ?? -1,
    };
  })) as ChainProbe;
}

test.describe('M044 S07 modal-chain-flow — chain popover survives its own clicks and reaches the engine', () => {
  test('open popover, enable, add entry — popover stays open, engine mirror matches JS shadow', async ({
    page,
  }) => {
    await page.goto(PAGE_PATH);
    const frame = await openTryItModal(page, CARD_SELECTOR);

    // Sanity: chain starts disabled with no entries.
    const start = await readChainState(frame);
    expect(start.enabled, 'chain must start disabled').toBe(false);
    expect(start.entryCount, 'chain must start with zero entries').toBe(0);

    // 1. Open the popover.
    await frame.locator('#chainBtn').click();
    await expect(
      frame.locator('#chainPopover'),
      'popover must open on chainBtn click',
    ).toBeVisible();

    // 2. Toggle Enable inside the popover. The previous bug: stale dismiss
    //    handler from a prior rebuild fires on this click's document-bubble
    //    and removes the popover before the assertion below can see it.
    await frame.locator('#chainPopover [data-chain-enable]').click();

    // Wait for the click's synchronous state emit + rebuild to settle.
    await frame.waitForFunction(
      () => {
        const w = window as unknown as {
          PolyWasmHost?: { getState?: () => { chain?: { enabled?: boolean } } };
        };
        return w.PolyWasmHost?.getState?.().chain?.enabled === true;
      },
      null,
      { timeout: 2000 },
    );

    // Popover must still be attached AND the enable switch must reflect on.
    await expect(
      frame.locator('#chainPopover'),
      'popover must NOT close as a side effect of clicking Enable — stale dismiss handler bug',
    ).toBeVisible();
    await expect(
      frame.locator('#chainPopover [data-chain-enable]'),
      'Enable switch must show the "on" class after toggling',
    ).toHaveClass(/(^|\s)on(\s|$)/);

    // JS shadow AND engine mirror both flipped.
    const enabledState = await readChainState(frame);
    expect(enabledState.enabled, 'JS shadow chain.enabled must be true after click').toBe(true);
    const probeAfterEnable = await readChainProbe(frame);
    expect(
      probeAfterEnable.engineChainEnabled,
      'engine mirror (poly_chain_enabled) must be 1 after JS edit — proves the S07 wiring reaches WASM',
    ).toBe(1);

    // 3. Add a chain entry via the popover's + button. Ensures the same
    //    dismiss-listener leak does not trip once the popover has rebuilt at
    //    least twice.
    await frame.locator('#chainPopover [data-chain-add]').click();
    await frame.waitForFunction(
      () => {
        const w = window as unknown as {
          PolyWasmHost?: { getState?: () => { chain?: { entryCount?: number } } };
        };
        return (w.PolyWasmHost?.getState?.().chain?.entryCount ?? 0) === 1;
      },
      null,
      { timeout: 2000 },
    );
    await expect(
      frame.locator('#chainPopover'),
      'popover must remain visible after Add-entry — dismiss handler must not stack across rebuilds',
    ).toBeVisible();
    await expect(
      frame.locator('#chainPopover .chain-entry'),
      'popover must show one chain entry row after Add-entry',
    ).toHaveCount(1);

    const afterAdd = await readChainState(frame);
    expect(afterAdd.entryCount, 'JS shadow entryCount must be 1 after Add-entry').toBe(1);
    const probeAfterAdd = await readChainProbe(frame);
    expect(
      probeAfterAdd.engineChainEntryCount,
      'engine mirror (poly_chain_entry_count) must be 1 — proves chainAddEntry reached WASM',
    ).toBe(1);

    // 4. Genuine outside click closes the popover. Establishes the dismiss
    //    path still functions (rules out over-eager fix that broke close).
    await frame.locator('#play').click({ position: { x: 5, y: 5 } });
    await expect(
      frame.locator('#chainPopover'),
      'popover must close on a real outside click',
    ).toHaveCount(0);
  });
});
