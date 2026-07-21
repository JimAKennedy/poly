import { test, expect, Frame, Page } from '@playwright/test';

// M044 S06 — Modal Morph flow regression gate.
//
// Guards against the bug reported after the S06 first-bar fix landed: with
// two different presets loaded into scenes A and B, sweeping the Morph slider
// audibly moved the *pattern* from A to B but the *instrument selection*
// stayed frozen on A. Root cause was two-fold:
//
//   1. engine/src/wasm_api.cpp — the read accessors routed through state()
//      which under SceneSelect::Morph fell through to scene A. Every lane
//      metadata read (midiNote, role, macro, envelope) therefore reflected A,
//      even after the render pass snapped to B's snap fields (midiNote/role
//      snap at t=0.5).
//   2. webui/wasm-host.js — the scheduler read the interpolated event's
//      `note` field but then looked the sample up via state.lanes[laneIdx],
//      which mirrored A's stale note. The audible sample stayed on A even
//      when the event's own pitch had already flipped to B.
//
// The fix wires an interpolated snapshot into the engine's read accessors
// (poly::Context::readState) and threads the event's note through voice()
// into sampleVoice(). Both host caches (laneNamesA/B, laneRolesA/B) are
// activated at the t=0.5 snap so the desk labels move too.
//
// This spec locks in the observable behavior: at morph = 0 the modal's
// sampleFilesByNote reflects A's notes; past the 0.5 snap it reflects B's
// notes. A regression on either side of the fix will break the snap-flip
// assertions.

const PAGE_PATH = '/13-drum-and-bass/';
const CARD_SELECTOR = '.poly-preview[data-poly-preset="Liquid Drum and Bass"]';
const PRESET_A = 'Liquid Drum and Bass';
const PRESET_B = 'Sparse Pulse';

// From site/src/generated/presets.json:
//   Liquid Drum and Bass — notes {36 kick, 38 snare, 42 hat, 51 cymbal}
//   Sparse Pulse         — notes {36 kick, 37 rim,   39 clap}
// Overlap: 36. A-only: 38, 42, 51. B-only: 37, 39. The snap-flip assertions
// use A-only vs B-only sets to prove the read-accessor plumbing actually
// swaps the surface, not merely tweaks values.
const A_UNIQUE_NOTES = [38, 42, 51];
const B_UNIQUE_NOTES = [37, 39];

const FIRST_BAR_NOTE_THRESHOLD = 12;
const FIRST_BAR_TIMEOUT_MS = 10_000;
const SAMPLES_READY_TIMEOUT_MS = 10_000;

interface WasmProbe {
  sceneAPreset: string;
  sceneBPreset: string;
  currentPreset: string;
  morphAmount: number;
  sampleFilesByNote: Record<number, string>;
  samplesLoaded: number;
  nodesStarted: number;
  synthVoiceCount: number;
  fallbackActive: boolean;
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

async function readProbe(frame: Frame): Promise<WasmProbe> {
  return (await frame.evaluate(() => {
    const p = (window as unknown as { __polyAudioProbe?: WasmProbe }).__polyAudioProbe;
    return {
      sceneAPreset: p?.sceneAPreset ?? '',
      sceneBPreset: p?.sceneBPreset ?? '',
      currentPreset: p?.currentPreset ?? '',
      morphAmount: p?.morphAmount ?? -1,
      sampleFilesByNote: p?.sampleFilesByNote
        ? Object.fromEntries(
            Object.entries(p.sampleFilesByNote).map(([k, v]) => [Number(k), v]),
          )
        : {},
      samplesLoaded: p?.samplesLoaded ?? 0,
      nodesStarted: p?.nodesStarted ?? 0,
      synthVoiceCount: p?.synthVoiceCount ?? -1,
      fallbackActive: p?.fallbackActive ?? false,
    };
  })) as WasmProbe;
}

async function findPresetIndex(frame: Frame, name: string): Promise<number> {
  return (await frame.evaluate((n) => {
    const w = window as unknown as {
      PolyWasmHost?: { getState?: () => { presets?: Array<{ name: string }> } };
    };
    const state = w.PolyWasmHost?.getState?.();
    if (!state?.presets) return -2;
    return state.presets.findIndex((p) => p.name === n);
  }, name)) as number;
}

async function callAction(
  frame: Frame,
  name: string,
  payload?: Record<string, unknown>,
): Promise<void> {
  await frame.evaluate(
    ({ n, pl }) => {
      const w = window as unknown as {
        PolyWasmHost?: { action: (name: string, payload?: unknown) => void };
      };
      w.PolyWasmHost?.action(n, pl);
    },
    { n: name, pl: payload ?? {} },
  );
}

async function setMorph(frame: Frame, value: number): Promise<void> {
  await frame.evaluate((v) => {
    const w = window as unknown as {
      PolyWasmHost?: {
        edit: (paramId: string, value: number, gesture: string) => void;
      };
    };
    w.PolyWasmHost?.edit('scene.morph', v, 'begin');
    w.PolyWasmHost?.edit('scene.morph', v, 'perform');
    w.PolyWasmHost?.edit('scene.morph', v, 'end');
  }, value);
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

test.describe('M044 S06 modal-morph-flow — Morph mode instrument selection tracks the snap point', () => {
  test('A=Liquid D&B / B=Sparse Pulse — sampleFilesByNote reflects A below t=0.5 and B above', async ({
    page,
  }) => {
    await page.goto(PAGE_PATH);
    const frame = await openTryItModal(page, CARD_SELECTOR);

    // Modal auto-loads the card's preset into scene A on open. Wait for the
    // probe to reflect it.
    await frame.waitForFunction(
      (expected) => {
        const p = (window as unknown as { __polyAudioProbe?: { sceneAPreset?: string } })
          .__polyAudioProbe;
        return p?.sceneAPreset === expected;
      },
      PRESET_A,
      { timeout: 5000 },
    );

    // Load Sparse Pulse into scene B before Play — togglePlay's
    // _poly_copy_scenes() snapshot captures both scenes at Play-click time,
    // so scene B needs to be populated before we start audible playback.
    const bIndex = await findPresetIndex(frame, PRESET_B);
    expect(
      bIndex,
      `preset "${PRESET_B}" missing from PolyWasmHost.getState().presets`,
    ).toBeGreaterThanOrEqual(0);

    await callAction(frame, 'selectScene', { scene: 'B' });
    await callAction(frame, 'applyPreset', { index: bIndex });

    let probe = await readProbe(frame);
    expect(probe.sceneAPreset, 'scene A must not be disturbed by loading into B').toBe(PRESET_A);
    expect(probe.sceneBPreset, 'scene B should now hold Sparse Pulse').toBe(PRESET_B);

    // Enter Morph mode with morph pinned to 0. The engine's readState()
    // snapshot should return A's fields (activeLaneCount = 4, lane[1].note =
    // 38, etc.) and sampleFilesByNote should therefore key on A's notes.
    await callAction(frame, 'selectScene', { scene: 'Morph' });
    await setMorph(frame, 0);

    // Trigger sample loading via a Play click. rebuildSampleSelection
    // populates probe.sampleFilesByNote eagerly, but pickFileForNote returns
    // null until the manifest is decoded — so the map stays empty until
    // ensureSamplesLoaded() resolves. Playing also anchors playbackCtx from
    // the current engineCtx state (with both scenes populated).
    await clickPlay(frame);

    // Wait until at least one sample has decoded AND the scheduler has
    // fired enough notes to prove sample voicing is live.
    await frame.waitForFunction(
      (threshold) => {
        const p = (window as unknown as {
          __polyAudioProbe?: { samplesLoaded?: number; nodesStarted?: number; fallbackActive?: boolean };
        }).__polyAudioProbe;
        if (p?.fallbackActive) return true; // let the fallback branch below explain
        return (p?.samplesLoaded ?? 0) > 0 && (p?.nodesStarted ?? 0) >= threshold;
      },
      FIRST_BAR_NOTE_THRESHOLD,
      { timeout: SAMPLES_READY_TIMEOUT_MS },
    );

    probe = await readProbe(frame);
    // S10 gate covers manifest-load failure — this spec assumes samples
    // loaded successfully. Bail out with a diagnostic if not.
    expect(
      probe.fallbackActive,
      'samples must be loaded — S10 covers the fallback path, not this spec. Check /samples/manifest.json.',
    ).toBe(false);
    expect(
      probe.samplesLoaded,
      'ensureSamplesLoaded() completed without decoding any samples',
    ).toBeGreaterThan(0);

    // Re-affirm morph = 0 view. Setting morph inside Morph mode calls
    // rebuildSampleSelection() so the map is fresh.
    await setMorph(frame, 0);
    await frame.waitForFunction(
      () => {
        const p = (window as unknown as {
          __polyAudioProbe?: { morphAmount?: number };
        }).__polyAudioProbe;
        return p?.morphAmount === 0;
      },
      null,
      { timeout: 3000 },
    );
    const probeMorph0 = await readProbe(frame);
    const notesAtMorph0 = Object.keys(probeMorph0.sampleFilesByNote)
      .map(Number)
      .sort((a, b) => a - b);

    for (const n of A_UNIQUE_NOTES) {
      expect(
        probeMorph0.sampleFilesByNote[n],
        `at morph=0 the interpolated snapshot must expose A's note ${n} — ` +
          `got sampleFilesByNote keys [${notesAtMorph0.join(', ')}]. ` +
          `readState() likely not routing to sceneA under Morph, or rebuildSampleSelection didn't run.`,
      ).toBeTruthy();
    }
    for (const n of B_UNIQUE_NOTES) {
      expect(
        probeMorph0.sampleFilesByNote[n],
        `at morph=0 the interpolated snapshot must NOT expose B-only note ${n} — ` +
          `got sampleFilesByNote keys [${notesAtMorph0.join(', ')}]. ` +
          `snap-flip happening at wrong t? Or laneNamesB leaking through activateLabelsForScene?`,
      ).toBeFalsy();
    }

    // Move morph past the snap point. Under SceneSelect::Morph the engine
    // returns B's snap fields (midiNote, role, activeLaneCount) once t ≥ 0.5.
    await setMorph(frame, 0.9);
    await frame.waitForFunction(
      () => {
        const p = (window as unknown as {
          __polyAudioProbe?: { morphAmount?: number };
        }).__polyAudioProbe;
        return (p?.morphAmount ?? -1) >= 0.5;
      },
      null,
      { timeout: 3000 },
    );
    const probeMorph9 = await readProbe(frame);
    const notesAtMorph9 = Object.keys(probeMorph9.sampleFilesByNote)
      .map(Number)
      .sort((a, b) => a - b);

    for (const n of B_UNIQUE_NOTES) {
      expect(
        probeMorph9.sampleFilesByNote[n],
        `at morph=0.9 the interpolated snapshot must expose B's note ${n} — ` +
          `got sampleFilesByNote keys [${notesAtMorph9.join(', ')}]. ` +
          `This is the exact user-visible symptom of the bug — instrument stays on A regardless of morph.`,
      ).toBeTruthy();
    }
    for (const n of A_UNIQUE_NOTES) {
      expect(
        probeMorph9.sampleFilesByNote[n],
        `at morph=0.9 the interpolated snapshot must NOT expose A-only note ${n} — ` +
          `got sampleFilesByNote keys [${notesAtMorph9.join(', ')}]. ` +
          `readState() likely still routing to sceneA under Morph.`,
      ).toBeFalsy();
    }

    // Belt-and-braces: with morph pinned above 0.5, keep playing long enough
    // to fire another bar of notes and confirm no synth-fallback voice fired
    // across the whole session (samples-load, morph=0, morph=0.9). If the
    // JS voice() handoff regressed to reading state.lanes[laneIdx].note
    // instead of the event's interpolated note — and the map didn't happen
    // to include A's note — we'd fall through to synthVoice() here.
    const nodesBefore = probeMorph9.nodesStarted;
    await frame
      .waitForFunction(
        (target) => {
          const p = (window as unknown as {
            __polyAudioProbe?: { nodesStarted?: number };
          }).__polyAudioProbe;
          return (p?.nodesStarted ?? 0) >= target;
        },
        nodesBefore + FIRST_BAR_NOTE_THRESHOLD,
        { timeout: FIRST_BAR_TIMEOUT_MS },
      )
      .catch(() => {
        /* diagnostic asserted below */
      });

    const probeFinal = await readProbe(frame);
    expect(
      probeFinal.nodesStarted,
      `scheduler stalled after morph flip — nodesStarted stayed at ${probeFinal.nodesStarted} ` +
        `(needed ≥ ${nodesBefore + FIRST_BAR_NOTE_THRESHOLD}). Morph edit may have crashed the render loop.`,
    ).toBeGreaterThanOrEqual(nodesBefore + FIRST_BAR_NOTE_THRESHOLD);
    expect(
      probeFinal.synthVoiceCount,
      `${probeFinal.synthVoiceCount} synth-fallback voice(s) fired during Morph playback — ` +
        `the JS voice() handoff must pass the event's interpolated note into sampleVoice(), ` +
        `not state.lanes[laneIdx].note.`,
    ).toBe(0);
  });
});
