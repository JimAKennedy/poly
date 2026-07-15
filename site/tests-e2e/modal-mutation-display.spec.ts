import { test, expect, Frame, Page } from '@playwright/test';

// M045 S01 T04 — Truthful mutation display regression gate.
//
// Guards the whole S01 slice. The user report was that Jazz Bop Ride's snare
// "skips some beats and sprinkles extras" — the engine was doing exactly what
// its mutationRate/probability parameters described, but the desk ladder only
// ever rendered the base Euclidean pattern (webui/wasm-host.js:125 reads
// pattern = rotArr(euclid(hits, steps), rot)). The overlay landed in T03:
//
//   - engine emits classified EmissionEvents (Base/Ghost/Add/Drop) alongside
//     NoteEvents (engine/src/engine.cpp:classifyStep)
//   - wasm-host drains per lane into a 32-slot ring (webui/wasm-host.js:210)
//   - ui.js paints em-add / em-drop / em-ghost on ladder step buttons every
//     frame (webui/ui.js:997 updateEmissionOverlay)
//
// This spec locks in the observable outcome for the exact preset the user
// reported: while Jazz Bop Ride is playing, the snare lane's ladder shows
// mutation adds and drops; after Stop, both classes disappear within one
// frame budget (500 ms is generous — resetLaneEmissions clears the ring
// synchronously in togglePlay, the very next onFrame diff wipes the
// classes). If a regression to base-only rendering ships, both playback
// assertions fail immediately; if the Stop-clear path regresses, the
// tail assertion fails.
//
// Snare lane index: 3 (see engine/src/presets.cpp:makeJazzBop —
// ride/hhFoot/kick/snareComp). mutationRate = 0.25 with hitCount 5 out of
// 16 steps → ~4 mutation rolls per bar of which ~1.6 drop, ~1.2 add,
// ~1.2 ghost. Two bars typically surface ≥3 add and ≥3 drop; the poll
// budget covers up to ~5 bars for headroom.

const PAGE_PATH = '/poly/12-jazz/';
const CARD_SELECTOR = '.poly-preview[data-poly-preset="Jazz Bop Ride"]';
const SNARE_LANE = 3;

const SNARE_LADDER_ADD = `.strip[data-lane="${SNARE_LANE}"] .ladder button.em-add`;
const SNARE_LADDER_DROP = `.strip[data-lane="${SNARE_LANE}"] .ladder button.em-drop`;

const SAMPLES_READY_TIMEOUT_MS = 10_000;
const MUTATION_OBSERVATION_TIMEOUT_MS = 12_000;
const STOP_CLEAR_WAIT_MS = 500;

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

async function waitForSamplesReady(frame: Frame): Promise<void> {
  await frame.waitForFunction(
    () => {
      const p = (window as unknown as {
        __polyAudioProbe?: { samplesLoaded?: number; fallbackActive?: boolean };
      }).__polyAudioProbe;
      if (p?.fallbackActive) return true;
      return (p?.samplesLoaded ?? 0) > 0;
    },
    null,
    { timeout: SAMPLES_READY_TIMEOUT_MS },
  );
}

async function clickPlay(frame: Frame): Promise<void> {
  const ok = await frame.evaluate(() => {
    const btn = document.querySelector('#play') as HTMLButtonElement | null;
    if (!btn) return false;
    btn.click();
    return true;
  });
  expect(ok, '#play button not found in modal').toBe(true);
}

async function countOverlayClass(frame: Frame, selector: string): Promise<number> {
  return (await frame.evaluate((sel) => document.querySelectorAll(sel).length, selector)) as number;
}

test.describe('M045 S01 modal-mutation-display — Jazz Bop Ride snare shows mutation overlay', () => {
  test('snare ladder paints em-add + em-drop during playback, clears within 500ms of Stop', async ({
    page,
  }) => {
    await page.goto(PAGE_PATH);
    const frame = await openTryItModal(page, CARD_SELECTOR);

    // Modal auto-loads Jazz Bop Ride into scene A on open.
    await frame.waitForFunction(
      () => {
        const p = (window as unknown as { __polyAudioProbe?: { sceneAPreset?: string } })
          .__polyAudioProbe;
        return p?.sceneAPreset === 'Jazz Bop Ride';
      },
      null,
      { timeout: 5000 },
    );

    // Play triggers ensureSamplesLoaded → decode → scheduler start → drain
    // begins populating the emission ring.
    await clickPlay(frame);
    await waitForSamplesReady(frame);

    // Poll until both em-add and em-drop are present on the snare ladder.
    // The ring holds up to 32 emissions per lane; the overlay reflects the
    // newest kind per step, so counts are a live snapshot of "steps whose
    // most recent emission was add|drop". Any regression that stops the
    // drain, empties classifyStep of Add/Drop outcomes, or bypasses the
    // updateEmissionOverlay diff will time out here.
    await frame.waitForFunction(
      ({ addSel, dropSel }) => {
        const addCount = document.querySelectorAll(addSel).length;
        const dropCount = document.querySelectorAll(dropSel).length;
        return addCount >= 1 && dropCount >= 1;
      },
      { addSel: SNARE_LADDER_ADD, dropSel: SNARE_LADDER_DROP },
      { timeout: MUTATION_OBSERVATION_TIMEOUT_MS },
    );

    const addWhilePlaying = await countOverlayClass(frame, SNARE_LADDER_ADD);
    const dropWhilePlaying = await countOverlayClass(frame, SNARE_LADDER_DROP);
    expect(
      addWhilePlaying,
      'snare lane 3 should show at least one em-add during Jazz Bop Ride playback — ' +
        `overlay likely not being painted from the emission ring`,
    ).toBeGreaterThanOrEqual(1);
    expect(
      dropWhilePlaying,
      'snare lane 3 should show at least one em-drop during Jazz Bop Ride playback — ' +
        `classifyStep may have regressed to never emitting Drop, or updateEmissionOverlay ` +
        `is not applying em-drop to .hit buttons`,
    ).toBeGreaterThanOrEqual(1);

    // Stop the transport. resetLaneEmissions clears the ring and the very
    // next onFrame diff clears the overlay classes. 500 ms is well past any
    // reasonable frame budget (worst case: pump interval 100 ms + one
    // scheduler tick).
    await clickPlay(frame);
    await frame.waitForTimeout(STOP_CLEAR_WAIT_MS);

    const addAfterStop = await countOverlayClass(frame, SNARE_LADDER_ADD);
    const dropAfterStop = await countOverlayClass(frame, SNARE_LADDER_DROP);
    expect(
      addAfterStop,
      `snare em-add count did not clear within ${STOP_CLEAR_WAIT_MS}ms of Stop — ` +
        `togglePlay may not be calling resetLaneEmissions, or updateEmissionOverlay ` +
        `is skipping the clear branch when getLaneEmissions() returns []`,
    ).toBe(0);
    expect(
      dropAfterStop,
      `snare em-drop count did not clear within ${STOP_CLEAR_WAIT_MS}ms of Stop`,
    ).toBe(0);
  });
});
