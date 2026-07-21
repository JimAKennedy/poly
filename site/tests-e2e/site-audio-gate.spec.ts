import { test, expect, type Response } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// S10 T02 — Playwright audio gate. For each gated chapter: exercise the
// preview-card Play chip and the Try It modal, verifying the audio path
// actually plays samples (not synth fallback) at the pattern's target BPM.
//
// Every assertion has a matching failure signal so a broken deploy fails
// with a chapter-scoped diagnostic instead of "something is wrong somewhere".

interface Chapter {
  preset: string;              // chapter card's data-poly-preset value
  path: string;                // page path relative to baseURL origin
  expectedFactoryName: string; // resolvePresetName() output
  expectedBpm: number;         // pattern.bpm
  resolutionPath: 'direct' | 'alias' | 'drift';
}

// All PolyPreviewCard chapters. Every display name is now a real engine preset
// (S11 T06 retired all chapter aliases), so expectedFactoryName === preset for
// every row. expectedBpm comes from PRESET_BPM in preset-patterns.ts (fallback
// 120 for presets without an override). resolutionPath is informational: mark
// 'drift' when the engine preset ships lane-level DriftRate, 'direct' otherwise.
const CHAPTERS: Chapter[] = [
  { preset: 'Polymetric Foundation', path: '/01-foundations/', expectedFactoryName: 'Polymetric Foundation', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Ewe Polymetric Ensemble', path: '/02-sub-saharan-africa/', expectedFactoryName: 'Ewe Polymetric Ensemble', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Manding Djembe', path: '/02-sub-saharan-africa/', expectedFactoryName: 'Manding Djembe', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Cuban Son Montuno', path: '/03-afro-cuban/', expectedFactoryName: 'Cuban Son Montuno', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Afrobeat Lagos', path: '/04-afrobeat/', expectedFactoryName: 'Afrobeat Lagos', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Balinese Kotekan', path: '/05-gamelan/', expectedFactoryName: 'Balinese Kotekan', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Javanese Colotomic', path: '/05-gamelan/', expectedFactoryName: 'Javanese Colotomic', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Tintal Groove', path: '/06-indian-classical/', expectedFactoryName: 'Tintal Groove', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Rupak Tal', path: '/06-indian-classical/', expectedFactoryName: 'Rupak Tal', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Rachenitsa 7/8', path: '/07-balkan/', expectedFactoryName: 'Rachenitsa 7/8', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Kopanitsa 11/8', path: '/07-balkan/', expectedFactoryName: 'Kopanitsa 11/8', expectedBpm: 200, resolutionPath: 'direct' },
  { preset: 'Reich Phase Process', path: '/08-minimalism/', expectedFactoryName: 'Reich Phase Process', expectedBpm: 100, resolutionPath: 'drift' },
  { preset: 'Riley Layered Entry', path: '/08-minimalism/', expectedFactoryName: 'Riley Layered Entry', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Nancarrow Tempi', path: '/08-minimalism/', expectedFactoryName: 'Nancarrow Tempi', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Minimal Techno', path: '/09-electronic/', expectedFactoryName: 'Minimal Techno', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Deep House', path: '/09-electronic/', expectedFactoryName: 'Deep House', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Samba Batucada', path: '/10-brazilian/', expectedFactoryName: 'Samba Batucada', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Bossa Nova Trio', path: '/10-brazilian/', expectedFactoryName: 'Bossa Nova Trio', expectedBpm: 130, resolutionPath: 'direct' },
  { preset: 'Classic Funk', path: '/11-funk-soul/', expectedFactoryName: 'Classic Funk', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Neo-Soul Pocket', path: '/11-funk-soul/', expectedFactoryName: 'Neo-Soul Pocket', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Jazz Bop Ride', path: '/12-jazz/', expectedFactoryName: 'Jazz Bop Ride', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Elvin Jones Cascade', path: '/12-jazz/', expectedFactoryName: 'Elvin Jones Cascade', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Jungle Break', path: '/13-drum-and-bass/', expectedFactoryName: 'Jungle Break', expectedBpm: 140, resolutionPath: 'direct' },
  { preset: 'Liquid Drum and Bass', path: '/13-drum-and-bass/', expectedFactoryName: 'Liquid Drum and Bass', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Afro-Electronic Fusion', path: '/14-synthesis/', expectedFactoryName: 'Afro-Electronic Fusion', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Balkan Funk', path: '/14-synthesis/', expectedFactoryName: 'Balkan Funk', expectedBpm: 120, resolutionPath: 'direct' },
  { preset: 'Compositional Arc', path: '/15-compositional-grammar/', expectedFactoryName: 'Compositional Arc', expectedBpm: 120, resolutionPath: 'direct' },
];

interface CardMetrics {
  sampleBuffersDecoded: number;
  oscillatorsCreated: number;
  scheduledNoteCount: number;
  firstFireTime: number;
  lastFireTime: number;
  firstFireBeat: number;
  lastFireBeat: number;
  currentPreset: string;
  activeBpm: number;
  fallbackActive: boolean;
  observedBpm: number;
  patternMeta: { bpm: number; laneCount: number; notesInBar: number } | null;
}

interface ModalMetrics {
  nodesStarted: number;
  samplesLoaded: number;
  fallbackActive: boolean;
  currentPreset: string;
  tempo: number;
}

interface ChapterSummary {
  chapter: string;
  resolutionPath: string;
  cardPass: boolean;
  cardFailReason?: string;
  cardMetrics: Partial<CardMetrics>;
  modalPass: boolean;
  modalFailReason?: string;
  modalMetrics: Partial<ModalMetrics>;
  consoleErrors: string[];
  failedResponses: string[];
}

const summary: ChapterSummary[] = [];

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const outPath = path.join(outDir, 'audio-gate-summary.json');
  fs.writeFileSync(outPath, JSON.stringify(summary, null, 2));
  // eslint-disable-next-line no-console
  console.log(`[audio-gate] wrote ${outPath} (${summary.length} chapters)`);
});

for (const chapter of CHAPTERS) {
  test(`[${chapter.preset}] card play fires samples + Try It modal plays at pattern BPM`, async ({ page }) => {
    const record: ChapterSummary = {
      chapter: chapter.preset,
      resolutionPath: chapter.resolutionPath,
      cardPass: false,
      cardMetrics: {},
      modalPass: false,
      modalMetrics: {},
      consoleErrors: [],
      failedResponses: [],
    };
    summary.push(record);

    page.on('console', (msg) => {
      if (msg.type() === 'error') record.consoleErrors.push(msg.text());
    });
    page.on('pageerror', (err) => record.consoleErrors.push(`pageerror: ${err.message}`));
    page.on('response', (res: Response) => {
      const url = res.url();
      const status = res.status();
      // 4xx/5xx that aren't obvious noise (favicon, DevTools well-known).
      if (
        status >= 400 &&
        !/favicon\.ico$/.test(url) &&
        !/\/\.well-known\//.test(url)
      ) {
        record.failedResponses.push(`${status} ${url}`);
      }
    });

    await page.goto(chapter.path);

    // ---------- CARD PROBE ----------
    const card = page.locator(`.poly-preview[data-poly-preset="${chapter.preset}"]`);
    await expect(card, `[${chapter.preset}] card missing on ${chapter.path}`).toBeVisible();
    await expect(card, `[${chapter.preset}] card started in wrong state`).toHaveAttribute(
      'data-state',
      'stopped',
    );

    const playBtn = card.locator('.poly-preview-play');
    await expect(
      playBtn,
      `[${chapter.preset}] Play chip disabled — pattern resolution failed for this alias`,
    ).toBeEnabled();
    await playBtn.click();
    // Intentionally do NOT hard-assert data-state="playing" here — startCard()
    // catches manifest/scheduler failures, sets fallbackActive=true, and
    // rethrows, which flips the card back to "stopped". Letting the probe
    // assertions run first surfaces the specific metric that broke.

    // Wait up to 5s for a stable observation window: ≥ 4 scheduled notes AND
    // the first-to-last fire span ≥ 1s. The time-span gate prevents co-fired
    // downbeat stacks (Javanese Colotomic: 4 lanes strike at beat 0 together)
    // from satisfying the count-only gate with all events at the same beat,
    // which would degenerate beatSpan to 0 in the observedBpm derivation.
    await page.waitForFunction(
      () => {
        const p = (window as unknown as { __polyAudioProbe: { scheduledNoteTimes: number[] } })
          .__polyAudioProbe;
        const times = p?.scheduledNoteTimes;
        if (!times || times.length < 4) return false;
        return times[times.length - 1] - times[0] >= 1.0;
      },
      null,
      { timeout: 5000 },
    ).catch(() => { /* fall through to assertion for a chapter-scoped failure */ });

    // Give ~500ms more so the observed rate stabilises before we measure.
    await page.waitForTimeout(500);

    const cardProbe = await page.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe: {
          sampleBuffersDecoded: number;
          oscillatorsCreated: number;
          scheduledNoteTimes: number[];
          scheduledNoteBeats: number[];
          currentPreset: string;
          activeBpm: number;
          fallbackActive: boolean;
        };
      }).__polyAudioProbe;
      const times = p.scheduledNoteTimes;
      const beats = p.scheduledNoteBeats ?? [];
      return {
        sampleBuffersDecoded: p.sampleBuffersDecoded,
        oscillatorsCreated: p.oscillatorsCreated,
        scheduledNoteCount: times.length,
        firstFireTime: times[0] ?? 0,
        lastFireTime: times[times.length - 1] ?? 0,
        firstFireBeat: beats[0] ?? 0,
        lastFireBeat: beats[beats.length - 1] ?? 0,
        currentPreset: p.currentPreset,
        activeBpm: p.activeBpm,
        fallbackActive: p.fallbackActive,
      };
    });

    const patternMeta = await page.evaluate((presetName: string) => {
      const pp = (window as unknown as {
        __polyPatterns?: {
          resolvePreset: (name: string) => {
            bpm: number;
            lanes: Array<{ note: number }>;
            notesInBar: number;
          } | null;
        };
      }).__polyPatterns;
      if (!pp) return null;
      const resolved = pp.resolvePreset(presetName);
      if (!resolved) return null;
      return {
        bpm: resolved.bpm,
        laneCount: resolved.lanes.length,
        notesInBar: resolved.notesInBar,
      };
    }, chapter.preset);

    // Post-1b: Play is engine-driven. Every scheduled note carries its own PPQ
    // (recorded into scheduledNoteBeats by the engine scheduler), so observed
    // BPM is a direct beatSpan / timeSpan calculation — no need to reconstruct
    // TS-scheduler event ordering.
    let observedBpm = 0;
    if (
      cardProbe.scheduledNoteCount >= 2 &&
      cardProbe.lastFireTime > cardProbe.firstFireTime &&
      cardProbe.lastFireBeat > cardProbe.firstFireBeat
    ) {
      const beatSpan = cardProbe.lastFireBeat - cardProbe.firstFireBeat;
      const timeSpan = cardProbe.lastFireTime - cardProbe.firstFireTime;
      observedBpm = (beatSpan * 60) / timeSpan;
    }

    const cardMetrics: CardMetrics = { ...cardProbe, observedBpm, patternMeta };
    record.cardMetrics = cardMetrics;

    try {
      expect(
        cardProbe.scheduledNoteCount,
        `[${chapter.preset}] card: scheduledNoteTimes.length=${cardProbe.scheduledNoteCount}, expected >= 4 within 3s (scheduler never armed)`,
      ).toBeGreaterThanOrEqual(4);
      expect(
        cardProbe.sampleBuffersDecoded,
        `[${chapter.preset}] card: sampleBuffersDecoded=${cardProbe.sampleBuffersDecoded}, expected > 0 (manifest fetch or decodeAudioData failed)`,
      ).toBeGreaterThan(0);
      expect(
        cardProbe.oscillatorsCreated,
        `[${chapter.preset}] card: oscillatorsCreated=${cardProbe.oscillatorsCreated}, expected 0 (synth fallback path fired)`,
      ).toBe(0);
      expect(
        cardProbe.fallbackActive,
        `[${chapter.preset}] card: fallbackActive=true (loadManifest or scheduler.start threw)`,
      ).toBe(false);
      expect(
        cardProbe.currentPreset,
        `[${chapter.preset}] card: currentPreset="${cardProbe.currentPreset}", expected "${chapter.expectedFactoryName}"`,
      ).toBe(chapter.expectedFactoryName);
      expect(
        cardProbe.activeBpm,
        `[${chapter.preset}] card: activeBpm=${cardProbe.activeBpm}, expected ${chapter.expectedBpm}`,
      ).toBe(chapter.expectedBpm);
      expect(
        patternMeta,
        `[${chapter.preset}] card: __polyPatterns.resolvePreset("${chapter.preset}") returned null — probe helper missing`,
      ).not.toBeNull();
      const bpmTolerance = chapter.expectedBpm * 0.02;
      expect(
        Math.abs(observedBpm - chapter.expectedBpm),
        `[${chapter.preset}] card: observedBpm=${observedBpm.toFixed(2)}, expected ${chapter.expectedBpm} ±${bpmTolerance.toFixed(2)} (±2%)`,
      ).toBeLessThanOrEqual(bpmTolerance);
      record.cardPass = true;
    } catch (err) {
      record.cardFailReason = err instanceof Error ? err.message : String(err);
      throw err;
    }

    // Stop the card before opening the modal so the two audio paths don't overlap.
    await playBtn.click();
    await expect(card, `[${chapter.preset}] card did not stop`).toHaveAttribute('data-state', 'stopped');

    // ---------- MODAL PROBE (Try It → WASM host) ----------
    await card.locator('[data-role="tryit"]').click();

    const dialog = page.locator('dialog.poly-modal');
    await dialog.waitFor({ state: 'attached', timeout: 5000 });

    const iframeHandle = await dialog.locator('iframe').elementHandle();
    expect(iframeHandle, `[${chapter.preset}] modal: iframe handle missing`).not.toBeNull();
    const frame = await iframeHandle!.contentFrame();
    expect(frame, `[${chapter.preset}] modal: iframe contentFrame missing`).not.toBeNull();

    await frame!.waitForLoadState('networkidle', { timeout: 15000 });

    // Wait for the WASM host to boot (PolyWasmHost is set by initPolyWasmHost).
    // If it fails to load, index.html falls back to mock-host under window.PolyHost
    // — the assertion for PolyWasmHost specifically catches that regression.
    await frame!.waitForFunction(
      () => {
        const w = window as unknown as {
          PolyWasmHost?: { getState?: () => unknown };
        };
        return !!(w.PolyWasmHost && w.PolyWasmHost.getState);
      },
      null,
      { timeout: 15000 },
    ).catch(() => { /* fall through */ });

    // applyParamsToHost() in webui/index.html retries setTempo/applyPreset for
    // up to ~2s. Give it that window before probing.
    await page.waitForTimeout(2200);

    // Click Play (#play button) inside the modal.
    const playClicked = await frame!.evaluate(() => {
      const btn = document.querySelector('#play') as HTMLButtonElement | null;
      if (!btn) return false;
      btn.click();
      return true;
    });
    expect(playClicked, `[${chapter.preset}] modal: #play button not found in webui/index.html`).toBe(true);

    // Wait for the WASM scheduler to fire at least one voice.
    await frame!.waitForFunction(
      () => {
        const p = (window as unknown as { __polyAudioProbe?: { nodesStarted?: number } }).__polyAudioProbe;
        return !!(p && (p.nodesStarted ?? 0) > 0);
      },
      null,
      { timeout: 5000 },
    ).catch(() => { /* fall through */ });

    const modalProbe = await frame!.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe?: {
          nodesStarted?: number;
          samplesLoaded?: number;
          fallbackActive?: boolean;
          currentPreset?: string;
        };
      }).__polyAudioProbe;
      const wasm = (window as unknown as {
        PolyWasmHost?: { getState: () => { preset: string; tempo: number } };
      }).PolyWasmHost;
      const st = wasm?.getState();
      return {
        nodesStarted: p?.nodesStarted ?? 0,
        samplesLoaded: p?.samplesLoaded ?? 0,
        fallbackActive: p?.fallbackActive ?? false,
        currentPreset: p?.currentPreset || st?.preset || '',
        tempo: st?.tempo ?? 0,
      };
    });

    record.modalMetrics = { ...modalProbe };

    try {
      expect(
        modalProbe.nodesStarted,
        `[${chapter.preset}] modal: nodesStarted=${modalProbe.nodesStarted}, expected > 0 (WASM scheduler never fired)`,
      ).toBeGreaterThan(0);
      expect(
        modalProbe.fallbackActive,
        `[${chapter.preset}] modal: fallbackActive=true (manifest fetch or no samples decoded — synth fallback engaged)`,
      ).toBe(false);
      expect(
        modalProbe.samplesLoaded,
        `[${chapter.preset}] modal: samplesLoaded=${modalProbe.samplesLoaded}, expected > 0`,
      ).toBeGreaterThan(0);
      expect(
        modalProbe.currentPreset,
        `[${chapter.preset}] modal: currentPreset is empty — WASM host never applied any preset`,
      ).toBeTruthy();
      // The webui applyParamsToHost() reads ?tempo= and calls setTempo. If the
      // WASM host reaches ready state, this should match the pattern's BPM
      // regardless of preset-name resolution.
      const tempoTolerance = chapter.expectedBpm * 0.02;
      expect(
        Math.abs(modalProbe.tempo - chapter.expectedBpm),
        `[${chapter.preset}] modal: tempo=${modalProbe.tempo}, expected ${chapter.expectedBpm} ±${tempoTolerance.toFixed(2)} (±2%)`,
      ).toBeLessThanOrEqual(tempoTolerance);
      record.modalPass = true;
    } catch (err) {
      record.modalFailReason = err instanceof Error ? err.message : String(err);
      throw err;
    }

    // Console errors and failed responses are a hard fail per the S10 plan.
    expect(
      record.consoleErrors,
      `[${chapter.preset}] console errors during run: ${record.consoleErrors.join(' | ')}`,
    ).toEqual([]);
    expect(
      record.failedResponses,
      `[${chapter.preset}] 4xx/5xx network responses: ${record.failedResponses.join(' | ')}`,
    ).toEqual([]);
  });
}
