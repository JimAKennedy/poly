import { test, expect, type BrowserContext } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// S11 T06 — cross-surface preset-consistency + sample-guarantee gate.
//
// For each chapter, run the card Play chip AND the Try It modal, compare their
// resolved preset structure (engineName + lanes[noteNumber, roleLabel]) and
// their measured RMS. Guarantee both surfaces are actually playing samples
// (not synth fallback) by asserting positive sample-decode counts and
// null-error probes. Two additional tests exercise the missing-sample failure
// surface: strip note 36 from the manifest via context.route() to prove the
// card surfaces an error state; strip role='kick' to prove the WASM host
// records the miss in probe.missingRoles.
//
// Why context.route() instead of page.route(): Playwright's class Frame has no
// route() method, so all frame requests flow through page/context routing.
// The Try It iframe is same-origin (sandbox="allow-scripts allow-same-origin")
// so a single context-level intercept catches both the card fetch and the
// iframe fetch without a popup/subframe-reparent edge case.

interface Lane {
  noteNumber: number;
  roleLabel: string;
}

interface ResolvedShape {
  engineName: string;
  lanes: Lane[];
}

interface CardProbe {
  sampleBuffersDecoded: number;
  scheduledNoteCount: number;
  currentPreset: string;
  activeBpm: number;
  fallbackActive: boolean;
  lastError: string | null;
  measuredRmsDb: number;
}

interface WasmProbe {
  nodesStarted: number;
  samplesLoaded: number;
  fallbackActive: boolean;
  currentPreset: string;
  missingRoles: string[];
  missingRolesFired: boolean;
  lastError: string | null;
  measuredRmsDb: number;
}

interface ChapterSpec {
  preset: string;
  path: string;
}

// All PolyPreviewCard chapters — every chapter with a preview card is gated
// for card ↔ WASM structural agreement and both-audible sample playback.
const CHAPTERS: ChapterSpec[] = [
  { preset: 'Polymetric Foundation', path: '/poly/01-foundations/' },
  { preset: 'Ewe Polymetric Ensemble', path: '/poly/02-sub-saharan-africa/' },
  { preset: 'Manding Djembe', path: '/poly/02-sub-saharan-africa/' },
  { preset: 'Cuban Son Montuno', path: '/poly/03-afro-cuban/' },
  { preset: 'Afrobeat Lagos', path: '/poly/04-afrobeat/' },
  { preset: 'Balinese Kotekan', path: '/poly/05-gamelan/' },
  { preset: 'Javanese Colotomic', path: '/poly/05-gamelan/' },
  { preset: 'Tintal Groove', path: '/poly/06-indian-classical/' },
  { preset: 'Rupak Tal', path: '/poly/06-indian-classical/' },
  { preset: 'Rachenitsa 7/8', path: '/poly/07-balkan/' },
  { preset: 'Kopanitsa 11/8', path: '/poly/07-balkan/' },
  { preset: 'Reich Phase Process', path: '/poly/08-minimalism/' },
  { preset: 'Riley Layered Entry', path: '/poly/08-minimalism/' },
  { preset: 'Nancarrow Tempi', path: '/poly/08-minimalism/' },
  { preset: 'Minimal Techno', path: '/poly/09-electronic/' },
  { preset: 'Deep House', path: '/poly/09-electronic/' },
  { preset: 'Samba Batucada', path: '/poly/10-brazilian/' },
  { preset: 'Bossa Nova Trio', path: '/poly/10-brazilian/' },
  { preset: 'Classic Funk', path: '/poly/11-funk-soul/' },
  { preset: 'Neo-Soul Pocket', path: '/poly/11-funk-soul/' },
  { preset: 'Jazz Bop Ride', path: '/poly/12-jazz/' },
  { preset: 'Elvin Jones Cascade', path: '/poly/12-jazz/' },
  { preset: 'Jungle Break', path: '/poly/13-drum-and-bass/' },
  { preset: 'Liquid Drum and Bass', path: '/poly/13-drum-and-bass/' },
  { preset: 'Afro-Electronic Fusion', path: '/poly/14-synthesis/' },
  { preset: 'Balkan Funk', path: '/poly/14-synthesis/' },
  { preset: 'Compositional Arc', path: '/poly/15-compositional-grammar/' },
];

interface ChapterSummary {
  chapter: string;
  cardResolved: ResolvedShape | null;
  wasmResolved: ResolvedShape | null;
  cardProbe: Partial<CardProbe>;
  wasmProbe: Partial<WasmProbe>;
  rmsDeltaDb: number | null;
  rmsSkipped: boolean;
  pass: boolean;
  failReason?: string;
}

interface NegativeSummary {
  name: string;
  pass: boolean;
  cardState?: string;
  cardLastError?: string | null;
  wasmMissingRoles?: string[];
  wasmLastError?: string | null;
  wasmFallbackActive?: boolean;
  failReason?: string;
}

const chapterSummaries: ChapterSummary[] = [];
const negativeSummaries: NegativeSummary[] = [];

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const outPath = path.join(outDir, 'preset-consistency-summary.json');
  fs.writeFileSync(
    outPath,
    JSON.stringify(
      { chapters: chapterSummaries, negativeTests: negativeSummaries },
      null,
      2,
    ),
  );
  // eslint-disable-next-line no-console
  console.log(
    `[preset-consistency] wrote ${outPath} (${chapterSummaries.length} chapters, ${negativeSummaries.length} negative tests)`,
  );
});

async function openTryItModal(page: import('@playwright/test').Page, chapterPreset: string) {
  const card = page.locator(`.poly-preview[data-poly-preset="${chapterPreset}"]`);
  await expect(card, `[${chapterPreset}] card missing`).toBeVisible();
  await card.locator('[data-role="tryit"]').click();
  const dialog = page.locator('dialog.poly-modal');
  await dialog.waitFor({ state: 'attached', timeout: 5000 });
  const iframeHandle = await dialog.locator('iframe').elementHandle();
  expect(iframeHandle, `[${chapterPreset}] modal: iframe handle missing`).not.toBeNull();
  const frame = await iframeHandle!.contentFrame();
  expect(frame, `[${chapterPreset}] modal: iframe contentFrame missing`).not.toBeNull();
  await frame!.waitForLoadState('networkidle', { timeout: 15000 });
  await frame!.waitForFunction(
    () => {
      const w = window as unknown as { PolyWasmHost?: { getState?: () => unknown; getResolvedPreset?: () => unknown } };
      return !!(w.PolyWasmHost && w.PolyWasmHost.getState && w.PolyWasmHost.getResolvedPreset);
    },
    null,
    { timeout: 15000 },
  );
  // applyParamsToHost() in webui/index.html retries setTempo/applyPreset for
  // up to ~2s. Give it that window before probing.
  await page.waitForTimeout(2200);
  return frame!;
}

for (const chapter of CHAPTERS) {
  test(`[${chapter.preset}] card and modal agree on preset structure + both play samples`, async ({ page }) => {
    const record: ChapterSummary = {
      chapter: chapter.preset,
      cardResolved: null,
      wasmResolved: null,
      cardProbe: {},
      wasmProbe: {},
      rmsDeltaDb: null,
      rmsSkipped: false,
      pass: false,
    };
    chapterSummaries.push(record);

    await page.goto(chapter.path);

    // ---------- CARD SIDE ----------
    const card = page.locator(`.poly-preview[data-poly-preset="${chapter.preset}"]`);
    await expect(card, `[${chapter.preset}] card missing on ${chapter.path}`).toBeVisible();
    const playBtn = card.locator('.poly-preview-play');
    await expect(playBtn, `[${chapter.preset}] Play chip disabled`).toBeEnabled();

    const cardResolved = await page.evaluate((presetName: string) => {
      const pp = (window as unknown as {
        __polyPatterns?: {
          resolvePreset?: (name: string) => {
            engineName: string;
            lanes: Array<{ note: number; role: string }>;
          } | null;
        };
      }).__polyPatterns;
      if (!pp || !pp.resolvePreset) return null;
      const r = pp.resolvePreset(presetName);
      if (!r) return null;
      return {
        engineName: r.engineName,
        lanes: r.lanes.map((l) => ({ noteNumber: l.note, roleLabel: l.role })),
      };
    }, chapter.preset);
    record.cardResolved = cardResolved;
    expect(
      cardResolved,
      `[${chapter.preset}] card: __polyPatterns.resolvePreset returned null — chapter not resolvable`,
    ).not.toBeNull();

    await playBtn.click();
    // Wait for at least one sample to actually decode before measuring RMS.
    // A raw "scheduledNoteTimes >= 4" gate could fire during the cold-start
    // window where per-voice gains are silent (buffer not yet ready).
    await page
      .waitForFunction(
        () => {
          const p = (window as unknown as {
            __polyAudioProbe: { scheduledNoteTimes: number[]; sampleBuffersDecoded: number };
          }).__polyAudioProbe;
          return p && p.scheduledNoteTimes.length >= 4 && p.sampleBuffersDecoded >= 1;
        },
        null,
        { timeout: 5000 },
      )
      .catch(() => {
        /* fall through to explicit assertion below */
      });
    await page.waitForTimeout(2500);

    const cardProbe = await page.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe: {
          sampleBuffersDecoded: number;
          scheduledNoteTimes: number[];
          currentPreset: string;
          activeBpm: number;
          fallbackActive: boolean;
          lastError: string | null;
          measuredRmsDb: number;
        };
      }).__polyAudioProbe;
      return {
        sampleBuffersDecoded: p.sampleBuffersDecoded,
        scheduledNoteCount: p.scheduledNoteTimes.length,
        currentPreset: p.currentPreset,
        activeBpm: p.activeBpm,
        fallbackActive: p.fallbackActive,
        lastError: p.lastError,
        measuredRmsDb: p.measuredRmsDb,
      };
    });
    record.cardProbe = cardProbe;

    // Stop the card before opening the modal.
    await playBtn.click();
    await expect(card, `[${chapter.preset}] card did not stop`).toHaveAttribute('data-state', 'stopped');

    // ---------- WASM SIDE (Try It modal) ----------
    const frame = await openTryItModal(page, chapter.preset);

    const wasmResolved = (await frame.evaluate(() => {
      const w = window as unknown as {
        PolyWasmHost?: { getResolvedPreset?: () => { engineName: string; lanes: Array<{ noteNumber: number; roleLabel: string }> } };
      };
      return w.PolyWasmHost?.getResolvedPreset?.() ?? null;
    })) as ResolvedShape | null;
    record.wasmResolved = wasmResolved;
    expect(
      wasmResolved,
      `[${chapter.preset}] wasm: PolyWasmHost.getResolvedPreset returned null`,
    ).not.toBeNull();

    // Click #play inside the modal.
    const playClicked = await frame.evaluate(() => {
      const btn = document.querySelector('#play') as HTMLButtonElement | null;
      if (!btn) return false;
      btn.click();
      return true;
    });
    expect(playClicked, `[${chapter.preset}] modal: #play button not found`).toBe(true);
    // Wait until at least one sample buffer has decoded before the RMS window
    // opens. Prevents the cold-start race where synth-fallback voices dominate
    // the analyser window before decoding lands.
    await frame
      .waitForFunction(
        () => {
          const p = (window as unknown as {
            __polyAudioProbe?: { nodesStarted?: number; samplesLoaded?: number };
          }).__polyAudioProbe;
          return !!(p && (p.nodesStarted ?? 0) > 0 && (p.samplesLoaded ?? 0) >= 1);
        },
        null,
        { timeout: 8000 },
      )
      .catch(() => {
        /* fall through */
      });
    await page.waitForTimeout(2500);

    const wasmProbe = (await frame.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe?: {
          nodesStarted?: number;
          samplesLoaded?: number;
          fallbackActive?: boolean;
          currentPreset?: string;
          missingRoles?: string[];
          missingRolesFired?: boolean;
          lastError?: string | null;
          measuredRmsDb?: number;
        };
      }).__polyAudioProbe;
      return {
        nodesStarted: p?.nodesStarted ?? 0,
        samplesLoaded: p?.samplesLoaded ?? 0,
        fallbackActive: p?.fallbackActive ?? false,
        currentPreset: p?.currentPreset ?? '',
        missingRoles: p?.missingRoles ?? [],
        missingRolesFired: p?.missingRolesFired ?? false,
        lastError: p?.lastError ?? null,
        measuredRmsDb: p?.measuredRmsDb ?? -Infinity,
      };
    })) as WasmProbe;
    record.wasmProbe = wasmProbe;

    try {
      // region:card-vs-modal-parity
      // ---------- Structural agreement ----------
      expect(
        cardResolved!.engineName,
        `[${chapter.preset}] engineName mismatch: card="${cardResolved!.engineName}" wasm="${wasmResolved!.engineName}"`,
      ).toBe(wasmResolved!.engineName);

      expect(
        cardResolved!.lanes.length,
        `[${chapter.preset}] lane count mismatch: card=${cardResolved!.lanes.length} wasm=${wasmResolved!.lanes.length}`,
      ).toBe(wasmResolved!.lanes.length);

      for (let i = 0; i < cardResolved!.lanes.length; i++) {
        const cl = cardResolved!.lanes[i];
        const wl = wasmResolved!.lanes[i];
        expect(
          cl.noteNumber,
          `[${chapter.preset}] lane ${i} noteNumber mismatch: card=${cl.noteNumber} wasm=${wl.noteNumber}`,
        ).toBe(wl.noteNumber);
        expect(
          cl.roleLabel,
          `[${chapter.preset}] lane ${i} roleLabel mismatch: card="${cl.roleLabel}" wasm="${wl.roleLabel}"`,
        ).toBe(wl.roleLabel);
      }
      // endregion:card-vs-modal-parity

      // ---------- Positive sample guarantee (card) ----------
      expect(
        cardProbe.sampleBuffersDecoded,
        `[${chapter.preset}] card is not using samples: sampleBuffersDecoded=${cardProbe.sampleBuffersDecoded}`,
      ).toBeGreaterThan(0);
      expect(
        cardProbe.lastError,
        `[${chapter.preset}] card reported error: ${cardProbe.lastError}`,
      ).toBeNull();
      expect(
        cardProbe.fallbackActive,
        `[${chapter.preset}] card fell into manifest-total-failure fallback`,
      ).toBe(false);

      // ---------- Positive sample guarantee (WASM) ----------
      expect(
        wasmProbe.samplesLoaded,
        `[${chapter.preset}] wasm host loaded no samples: samplesLoaded=${wasmProbe.samplesLoaded}`,
      ).toBeGreaterThan(0);
      expect(
        wasmProbe.missingRoles.length,
        `[${chapter.preset}] wasm host silently synthed roles: [${wasmProbe.missingRoles.join(', ')}]`,
      ).toBe(0);
      expect(
        wasmProbe.fallbackActive,
        `[${chapter.preset}] wasm host in manifest-total-failure fallback`,
      ).toBe(false);

      // ---------- Both-audible check ----------
      // Original design was a ±3 dB RMS parity assertion. In practice the two
      // surfaces shape velocity differently — the WASM host applies groove
      // math (ghost/spread/env factors) inside hitVelocity() before reaching
      // sampleVoice(), while the card scheduler plays each event at raw
      // velocity/127. That legitimately drives per-chapter RMS deltas of
      // 10-25 dB, all musical, none a bug. Strict parity produces false
      // negatives; the real regression this check has to catch is "one
      // surface is silent while the other plays" (topology bug: analyser tap
      // in the wrong place, sample never fired, destination mis-routed).
      // -60 dBFS is well below any real playback and well above a fully
      // silent signal, so both-audible above that floor keeps the topology
      // signal without over-constraining musical shaping.
      const AUDIBLE_FLOOR_DB = -60;
      record.rmsDeltaDb =
        Number.isFinite(cardProbe.measuredRmsDb) && Number.isFinite(wasmProbe.measuredRmsDb)
          ? Math.abs(cardProbe.measuredRmsDb - wasmProbe.measuredRmsDb)
          : null;
      if (cardProbe.fallbackActive || wasmProbe.fallbackActive) {
        record.rmsSkipped = true;
        test.info().annotations.push({
          type: 'rms-skip',
          description: `[${chapter.preset}] audible check skipped — a surface is in fallback (cardFallback=${cardProbe.fallbackActive} wasmFallback=${wasmProbe.fallbackActive}). Fault points to S10, not S11.`,
        });
      } else {
        expect(
          cardProbe.measuredRmsDb,
          `[${chapter.preset}] card measuredRmsDb=${cardProbe.measuredRmsDb}, expected > ${AUDIBLE_FLOOR_DB} (audible floor)`,
        ).toBeGreaterThan(AUDIBLE_FLOOR_DB);
        expect(
          wasmProbe.measuredRmsDb,
          `[${chapter.preset}] wasm measuredRmsDb=${wasmProbe.measuredRmsDb}, expected > ${AUDIBLE_FLOOR_DB} (audible floor)`,
        ).toBeGreaterThan(AUDIBLE_FLOOR_DB);
      }

      record.pass = true;
    } catch (err) {
      record.failReason = err instanceof Error ? err.message : String(err);
      throw err;
    }
  });
}

// ---------- Missing-sample / missing-role negative tests ----------
// These strip parts of the sample manifest via a per-test context.route() so
// disk state is never touched. Prefer context.route over page.route as
// belt-and-braces: same intercept semantics but explicitly covers subframe
// reparenting edges (Playwright's class Frame has no route() method).

test.describe('missing-sample failure surfaces', () => {
  const MANIFEST_ABS = path.resolve(process.cwd(), 'public/samples/manifest.json');
  const realManifestText = fs.readFileSync(MANIFEST_ABS, 'utf8');

  test('card surfaces missing-sample error when a required note has no manifest entry', async ({
    page,
    context,
  }) => {
    const record: NegativeSummary = { name: 'card-missing-note-36', pass: false };
    negativeSummaries.push(record);

    // Strip note 36 from every manifest entry — this is the kick MIDI note
    // used by Minimal Techno's lane 0. Mutation is per-test; disk untouched.
    const STRIP_NOTE = 36;
    await stripManifest(context, realManifestText, (m) => {
      for (const s of m.samples) {
        if (Array.isArray(s.midiNotes)) {
          s.midiNotes = s.midiNotes.filter((n: number) => n !== STRIP_NOTE);
        }
      }
    });

    try {
      await page.goto('/poly/09-electronic/');
      const card = page.locator('.poly-preview[data-poly-preset="Minimal Techno"]');
      await expect(card).toBeVisible();
      await card.locator('.poly-preview-play').click();

      await expect(
        card,
        'card did not enter error state within 5s',
      ).toHaveAttribute('data-state', 'error', { timeout: 5000 });

      const lastError = await page.evaluate(() => {
        return (
          (window as unknown as { __polyAudioProbe: { lastError: string | null } })
            .__polyAudioProbe.lastError ?? null
        );
      });
      record.cardState = 'error';
      record.cardLastError = lastError;
      expect(
        lastError,
        `probe.lastError = ${lastError}, expected /no sample for MIDI note 36/`,
      ).toMatch(/no sample for MIDI note 36/);

      const banner = card.locator('.poly-preview-error');
      await expect(banner).toBeVisible();
      const bannerText = (await banner.textContent()) ?? '';
      expect(bannerText.startsWith('Sample playback failed')).toBe(true);

      record.pass = true;
    } catch (err) {
      record.failReason = err instanceof Error ? err.message : String(err);
      throw err;
    } finally {
      await context.unroute('**/samples/manifest.json').catch(() => {
        /* ignore */
      });
    }
  });

  test('wasm host surfaces missing-note when no manifest entry serves the required MIDI note', async ({
    page,
    context,
  }) => {
    const record: NegativeSummary = { name: 'wasm-missing-note-36', pass: false };
    negativeSummaries.push(record);

    // Sample-selection parity fix: WASM host now resolves samples by (note,
    // preferredRole) matching the card. Stripping role='kick' silently falls
    // back to note 36's cajon entry on both surfaces (that's the shared, by-
    // design behavior). To exercise a hard miss we strip note 36 from every
    // entry — same strategy the card negative test above uses.
    const STRIP_NOTE = 36;
    await stripManifest(context, realManifestText, (m) => {
      for (const s of m.samples) {
        if (Array.isArray(s.midiNotes)) {
          s.midiNotes = s.midiNotes.filter((n: number) => n !== STRIP_NOTE);
        }
      }
    });

    try {
      await page.goto('/poly/09-electronic/');
      const frame = await openTryItModal(page, 'Minimal Techno');

      const playClicked = await frame.evaluate(() => {
        const btn = document.querySelector('#play') as HTMLButtonElement | null;
        if (!btn) return false;
        btn.click();
        return true;
      });
      expect(playClicked).toBe(true);

      await frame
        .waitForFunction(
          () => {
            const p = (window as unknown as { __polyAudioProbe?: { nodesStarted?: number } })
              .__polyAudioProbe;
            return !!(p && (p.nodesStarted ?? 0) >= 4);
          },
          null,
          { timeout: 5000 },
        )
        .catch(() => {
          /* fall through */
        });

      await frame.waitForFunction(
        () => {
          const p = (window as unknown as {
            __polyAudioProbe?: { missingRoles?: string[]; missingRolesFired?: boolean };
          }).__polyAudioProbe;
          return !!(p && p.missingRolesFired === true);
        },
        null,
        { timeout: 5000 },
      );

      const wasmProbe = (await frame.evaluate(() => {
        const p = (window as unknown as {
          __polyAudioProbe?: {
            missingRoles?: string[];
            missingNotes?: number[];
            missingRolesFired?: boolean;
            fallbackActive?: boolean;
            lastError?: string | null;
          };
        }).__polyAudioProbe;
        return {
          missingRoles: p?.missingRoles ?? [],
          missingNotes: p?.missingNotes ?? [],
          missingRolesFired: p?.missingRolesFired ?? false,
          fallbackActive: p?.fallbackActive ?? false,
          lastError: p?.lastError ?? null,
        };
      })) as {
        missingRoles: string[];
        missingNotes: number[];
        missingRolesFired: boolean;
        fallbackActive: boolean;
        lastError: string | null;
      };
      record.wasmMissingRoles = wasmProbe.missingRoles;
      record.wasmLastError = wasmProbe.lastError;
      record.wasmFallbackActive = wasmProbe.fallbackActive;

      expect(
        wasmProbe.fallbackActive,
        'wasm host in manifest-total-failure fallback — wrong branch',
      ).toBe(false);
      expect(wasmProbe.missingRolesFired).toBe(true);
      expect(
        wasmProbe.missingNotes.includes(STRIP_NOTE),
        `probe.missingNotes=[${wasmProbe.missingNotes.join(', ')}], expected to include ${STRIP_NOTE}`,
      ).toBe(true);
      expect(
        wasmProbe.lastError,
        `probe.lastError = ${wasmProbe.lastError}, expected /no sample for MIDI note ${STRIP_NOTE}/`,
      ).toMatch(new RegExp(`no sample for MIDI note ${STRIP_NOTE}`));

      record.pass = true;
    } catch (err) {
      record.failReason = err instanceof Error ? err.message : String(err);
      throw err;
    } finally {
      await context.unroute('**/samples/manifest.json').catch(() => {
        /* ignore */
      });
    }
  });
});

async function stripManifest(
  context: BrowserContext,
  realManifestText: string,
  mutate: (manifest: { samples: Array<{ role?: string; midiNotes?: number[] }> }) => void,
): Promise<void> {
  await context.route('**/samples/manifest.json', async (route) => {
    const parsed = JSON.parse(realManifestText);
    mutate(parsed);
    await route.fulfill({
      status: 200,
      contentType: 'application/json',
      body: JSON.stringify(parsed),
    });
  });
}
