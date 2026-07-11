import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// M043 follow-up — sample-selection equivalence gate.
//
// For every chapter preset: click Play on the card, capture the resolved
// sample file per MIDI note, then open the Try It modal, click Play, and
// capture the WASM host's resolution. Assert per-note equality.
//
// This closes the gap M043's MIDI-equivalence gates (S11/S13) never covered:
// they proved the note events matched, not which sample file each note fired.
// The audible divergence users reported (jazz cymbal ride vs china) lives
// entirely in the file resolution layer — invisible to SMF diffs.
//
// Both surfaces now key selection on (MIDI note, preferred role) using the
// same pickEntryForNote algorithm:
//   site/src/audio/sample-loader.ts::pickEntryForNote   (canonical)
//   webui/wasm-host.js::pickEntryForNote                (mirrored)

interface ChapterSpec {
  preset: string;
  path: string;
}

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

interface DivergentNote {
  note: number;
  cardFile: string | null;
  wasmFile: string | null;
}

interface ChapterSummary {
  chapter: string;
  cardFilesByNote: Record<number, string>;
  wasmFilesByNote: Record<number, string>;
  divergentNotes: DivergentNote[];
  pass: boolean;
  failReason?: string;
}

const summaries: ChapterSummary[] = [];

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const outPath = path.join(outDir, 'sample-equivalence-summary.json');
  const passCount = summaries.filter((s) => s.pass).length;
  fs.writeFileSync(
    outPath,
    JSON.stringify(
      {
        gate: 'sample-equivalence',
        chapterCount: summaries.length,
        passCount,
        failCount: summaries.length - passCount,
        verdict: passCount === summaries.length ? 'pass' : 'fail',
        chapters: summaries,
      },
      null,
      2,
    ),
  );
  // eslint-disable-next-line no-console
  console.log(
    `[sample-equivalence] wrote ${outPath} (${summaries.length} chapters, ${passCount} pass, ${summaries.length - passCount} fail)`,
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
  await page.waitForTimeout(2200); // applyParamsToHost retry window
  return frame!;
}

for (const chapter of CHAPTERS) {
  test(`[${chapter.preset}] card and Try It resolve the same sample file per MIDI note`, async ({ page }) => {
    const record: ChapterSummary = {
      chapter: chapter.preset,
      cardFilesByNote: {},
      wasmFilesByNote: {},
      divergentNotes: [],
      pass: false,
    };
    summaries.push(record);

    await page.goto(chapter.path);

    // ---------- CARD SIDE ----------
    const card = page.locator(`.poly-preview[data-poly-preset="${chapter.preset}"]`);
    await expect(card, `[${chapter.preset}] card missing`).toBeVisible();
    const playBtn = card.locator('.poly-preview-play');
    await expect(playBtn, `[${chapter.preset}] Play chip disabled`).toBeEnabled();
    await playBtn.click();
    // Wait until at least one sample decodes so probe.sampleFilesByNote is
    // populated (startCard sets it eagerly, but only after the manifest load
    // resolves — a network stall could still leave it empty).
    await page
      .waitForFunction(
        () => {
          const p = (window as unknown as {
            __polyAudioProbe?: {
              sampleBuffersDecoded?: number;
              sampleFilesByNote?: Record<number, string>;
            };
          }).__polyAudioProbe;
          return !!(p && (p.sampleBuffersDecoded ?? 0) >= 1 && p.sampleFilesByNote && Object.keys(p.sampleFilesByNote).length > 0);
        },
        null,
        { timeout: 8000 },
      )
      .catch(() => {
        /* fall through to explicit read below */
      });

    const cardFiles = (await page.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe?: {
          sampleFilesByNote?: Record<number, string>;
          fallbackActive?: boolean;
        };
      }).__polyAudioProbe;
      return {
        files: p?.sampleFilesByNote ?? {},
        fallbackActive: p?.fallbackActive ?? false,
      };
    })) as { files: Record<number, string>; fallbackActive: boolean };
    record.cardFilesByNote = cardFiles.files;

    // Stop the card before opening the modal.
    await playBtn.click();
    await expect(card, `[${chapter.preset}] card did not stop`).toHaveAttribute('data-state', 'stopped');

    // ---------- WASM SIDE ----------
    const frame = await openTryItModal(page, chapter.preset);
    const playClicked = await frame.evaluate(() => {
      const btn = document.querySelector('#play') as HTMLButtonElement | null;
      if (!btn) return false;
      btn.click();
      return true;
    });
    expect(playClicked, `[${chapter.preset}] modal: #play button not found`).toBe(true);
    await frame
      .waitForFunction(
        () => {
          const p = (window as unknown as {
            __polyAudioProbe?: {
              samplesLoaded?: number;
              nodesStarted?: number;
              sampleFilesByNote?: Record<number, string>;
            };
          }).__polyAudioProbe;
          return !!(
            p
            && (p.samplesLoaded ?? 0) >= 1
            && (p.nodesStarted ?? 0) > 0
            && p.sampleFilesByNote
            && Object.keys(p.sampleFilesByNote).length > 0
          );
        },
        null,
        { timeout: 8000 },
      )
      .catch(() => {
        /* fall through to explicit read */
      });

    const wasmFiles = (await frame.evaluate(() => {
      const p = (window as unknown as {
        __polyAudioProbe?: {
          sampleFilesByNote?: Record<number, string>;
          fallbackActive?: boolean;
        };
      }).__polyAudioProbe;
      return {
        files: p?.sampleFilesByNote ?? {},
        fallbackActive: p?.fallbackActive ?? false,
      };
    })) as { files: Record<number, string>; fallbackActive: boolean };
    record.wasmFilesByNote = wasmFiles.files;

    // If either surface is in manifest-total-failure fallback the S10 gate
    // catches it — don't double-attribute here.
    if (cardFiles.fallbackActive || wasmFiles.fallbackActive) {
      test.info().annotations.push({
        type: 'sample-equivalence-skip',
        description: `[${chapter.preset}] skipping — a surface is in fallback (card=${cardFiles.fallbackActive} wasm=${wasmFiles.fallbackActive}); fault points to S10`,
      });
      record.pass = true;
      return;
    }

    try {
      const cardKeys = Object.keys(cardFiles.files)
        .map((n) => Number.parseInt(n, 10))
        .sort((a, b) => a - b);
      const wasmKeys = Object.keys(wasmFiles.files)
        .map((n) => Number.parseInt(n, 10))
        .sort((a, b) => a - b);

      expect(
        cardKeys.length,
        `[${chapter.preset}] card resolved no sample files`,
      ).toBeGreaterThan(0);
      expect(
        wasmKeys.length,
        `[${chapter.preset}] wasm resolved no sample files`,
      ).toBeGreaterThan(0);

      // Union of notes both surfaces reported. A note only present on one side
      // is a divergence (missing note = miss) too.
      const allNotes = new Set<number>([...cardKeys, ...wasmKeys]);
      for (const note of allNotes) {
        const cardFile = cardFiles.files[note] ?? null;
        const wasmFile = wasmFiles.files[note] ?? null;
        if (cardFile !== wasmFile) {
          record.divergentNotes.push({ note, cardFile, wasmFile });
        }
      }

      expect(
        record.divergentNotes,
        `[${chapter.preset}] sample-selection divergence: ${record.divergentNotes
          .map((d) => `note=${d.note} card=${d.cardFile ?? 'null'} wasm=${d.wasmFile ?? 'null'}`)
          .join('; ')}`,
      ).toEqual([]);

      record.pass = true;
    } catch (err) {
      record.failReason = err instanceof Error ? err.message : String(err);
      throw err;
    }
  });
}
