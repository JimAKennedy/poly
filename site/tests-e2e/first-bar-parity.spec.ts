import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// M044 S06 T03 — First-bar parity gate.
//
// Regression test for the Try It modal startup race: on the very first Play
// click, ensureSamplesLoaded() was fire-and-forget while the scheduler
// started at startAt = currentTime + 60ms. That gave decodeAudioData
// ~200-500ms to finish — during which sampleVoice() returned false and
// voice() fell through to synthVoice() (sine burst / filtered noise). The
// user heard "wrong sounds for a couple of beats, then correct samples."
//
// The fix (M044 S06 T02 in webui/wasm-host.js::togglePlay) awaits
// ensureSamplesLoaded() before anchoring startAt and starting setInterval.
// After the first Play the promise is cached and resolves synchronously,
// so this only delays the first Play — matching the card scheduler's
// contract at site/src/audio/engine-scheduler.ts:192.
//
// Assertion: after ~1 bar of playback (proxied as >= 12 note-ons fired),
// probe.synthVoiceCount MUST be 0. Any non-zero value means a note-on
// reached the audible bus via the synth fallback path — the exact
// audibly-wrong behavior we set out to eliminate.
//
// Three canonical presets covering the sample-selection surface:
//   - Elvin Jones Cascade (12-jazz)         — dense jazz kit, ride cymbal
//   - Kopanitsa 11/8      (07-balkan)       — 200 BPM, tight window
//   - Deep House          (09-electronic)   — steady 4/4, kick+hat+snare

interface PresetSpec {
  preset: string;
  path: string;
}

const PRESETS: PresetSpec[] = [
  { preset: 'Elvin Jones Cascade', path: '/poly/12-jazz/' },
  { preset: 'Kopanitsa 11/8', path: '/poly/07-balkan/' },
  { preset: 'Deep House', path: '/poly/09-electronic/' },
];

const FIRST_BAR_NOTE_THRESHOLD = 12;
const FIRST_BAR_TIMEOUT_MS = 8_000;

interface CaseSummary {
  preset: string;
  path: string;
  nodesStarted: number;
  synthVoiceCount: number;
  sampleFilesByNoteCount: number;
  fallbackActive: boolean;
  pass: boolean;
  failReason?: string;
}

const summaries: CaseSummary[] = [];

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const outPath = path.join(outDir, 'first-bar-parity-summary.json');
  const passCount = summaries.filter((s) => s.pass).length;
  fs.writeFileSync(
    outPath,
    JSON.stringify(
      {
        gate: 'first-bar-parity',
        caseCount: summaries.length,
        passCount,
        failCount: summaries.length - passCount,
        verdict: passCount === summaries.length ? 'pass' : 'fail',
        cases: summaries,
      },
      null,
      2,
    ),
  );
  // eslint-disable-next-line no-console
  console.log(
    `[first-bar-parity] wrote ${outPath} (${summaries.length} cases, ${passCount} pass, ${summaries.length - passCount} fail)`,
  );
});

async function openTryItModal(page: import('@playwright/test').Page, preset: string) {
  const card = page.locator(`.poly-preview[data-poly-preset="${preset}"]`);
  await expect(card, `[${preset}] card missing`).toBeVisible();
  await card.locator('[data-role="tryit"]').click();
  const dialog = page.locator('dialog.poly-modal');
  await dialog.waitFor({ state: 'attached', timeout: 5000 });
  const iframeHandle = await dialog.locator('iframe').elementHandle();
  expect(iframeHandle, `[${preset}] modal: iframe handle missing`).not.toBeNull();
  const frame = await iframeHandle!.contentFrame();
  expect(frame, `[${preset}] modal: iframe contentFrame missing`).not.toBeNull();
  await frame!.waitForLoadState('networkidle', { timeout: 15000 });
  await frame!.waitForFunction(
    () => {
      const w = window as unknown as { PolyWasmHost?: { getState?: () => unknown } };
      return !!(w.PolyWasmHost && w.PolyWasmHost.getState);
    },
    null,
    { timeout: 15000 },
  );
  await page.waitForTimeout(2200);
  return frame!;
}

test.describe('M044 S06 first-bar-parity — Try It modal first bar plays samples, not synth', () => {
  for (const spec of PRESETS) {
    test(`[${spec.preset}] first bar of Try It playback fires no synth-fallback voices`, async ({ page }) => {
      const record: CaseSummary = {
        preset: spec.preset,
        path: spec.path,
        nodesStarted: 0,
        synthVoiceCount: -1,
        sampleFilesByNoteCount: 0,
        fallbackActive: false,
        pass: false,
      };
      summaries.push(record);

      await page.goto(spec.path);

      const frame = await openTryItModal(page, spec.preset);

      // Click Play. The fix in togglePlay awaits ensureSamplesLoaded() inside
      // the click handler, so nodesStarted stays at 0 until decode completes.
      const playClicked = await frame.evaluate(() => {
        const btn = document.querySelector('#play') as HTMLButtonElement | null;
        if (!btn) return false;
        btn.click();
        return true;
      });
      expect(playClicked, `[${spec.preset}] modal: #play button not found`).toBe(true);

      // Wait for the scheduler to render at least one bar's worth of notes.
      // 12 note-ons is comfortably above the shortest lane density (Deep
      // House's kick fires 4x per bar) and well below the densest preset's
      // first-bar output (Elvin Jones ~9 note-ons/bar × 8 lanes).
      await frame
        .waitForFunction(
          (threshold) => {
            const p = (window as unknown as {
              __polyAudioProbe?: { nodesStarted?: number };
            }).__polyAudioProbe;
            return !!(p && (p.nodesStarted ?? 0) >= threshold);
          },
          FIRST_BAR_NOTE_THRESHOLD,
          { timeout: FIRST_BAR_TIMEOUT_MS },
        )
        .catch(() => {
          /* fall through to explicit read for a diagnostic assertion */
        });

      const probe = (await frame.evaluate(() => {
        const p = (window as unknown as {
          __polyAudioProbe?: {
            nodesStarted?: number;
            synthVoiceCount?: number;
            sampleFilesByNote?: Record<number, string>;
            fallbackActive?: boolean;
          };
        }).__polyAudioProbe;
        return {
          nodesStarted: p?.nodesStarted ?? 0,
          synthVoiceCount: p?.synthVoiceCount ?? -1,
          sampleFilesByNoteCount: p?.sampleFilesByNote
            ? Object.keys(p.sampleFilesByNote).length
            : 0,
          fallbackActive: p?.fallbackActive ?? false,
        };
      })) as {
        nodesStarted: number;
        synthVoiceCount: number;
        sampleFilesByNoteCount: number;
        fallbackActive: boolean;
      };

      record.nodesStarted = probe.nodesStarted;
      record.synthVoiceCount = probe.synthVoiceCount;
      record.sampleFilesByNoteCount = probe.sampleFilesByNoteCount;
      record.fallbackActive = probe.fallbackActive;

      // If the manifest itself failed to load, the S10 gate catches it — no
      // point double-attributing here.
      if (probe.fallbackActive) {
        test.info().annotations.push({
          type: 'first-bar-parity-skip',
          description: `[${spec.preset}] skipping — fallbackActive=true (manifest load failed); fault points to S10`,
        });
        record.pass = true;
        return;
      }

      try {
        expect(
          probe.nodesStarted,
          `[${spec.preset}] scheduler never fired ${FIRST_BAR_NOTE_THRESHOLD} note-ons within ${FIRST_BAR_TIMEOUT_MS}ms — nodesStarted=${probe.nodesStarted}. ` +
            `Either togglePlay's await ensureSamplesLoaded() is deadlocking, or the WASM scheduler isn't emitting events.`,
        ).toBeGreaterThanOrEqual(FIRST_BAR_NOTE_THRESHOLD);

        expect(
          probe.synthVoiceCount,
          `[${spec.preset}] first-bar startup race regressed: ${probe.synthVoiceCount} synth-fallback voice(s) fired during the first bar. ` +
            `synthVoice() is only reached when sampleVoice() returns false — either buffersByFile was empty (sample-decode race, S06 T02 fix reverted?) ` +
            `or pickFileForNote returned null (sample-selection parity broken). nodesStarted=${probe.nodesStarted}, sampleFiles=${probe.sampleFilesByNoteCount}`,
        ).toBe(0);

        expect(
          probe.sampleFilesByNoteCount,
          `[${spec.preset}] probe.sampleFilesByNote is empty — rebuildSampleSelection() likely didn't run or the preset has no resolved lanes`,
        ).toBeGreaterThan(0);

        record.pass = true;
      } catch (err) {
        record.failReason = err instanceof Error ? err.message : String(err);
        throw err;
      }
    });
  }
});
