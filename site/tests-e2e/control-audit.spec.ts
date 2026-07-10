import { test, expect, type Page, type FrameLocator } from '@playwright/test';

// M043 S14 T04 + T05 — Try It modal layout probes + label→effect audit.
//
// This spec is the acceptance gate for both T04 (button parity, modal fits at
// mobile/tablet/desktop viewports) and T05 (every enabled control on the
// modal produces the effect its label promises for 3 representative
// chapters). Grouped here so one spec run gates both, and so a regression in
// either bucket points to a single file.
//
// Chapters were picked to cover the three broad preset families:
//   - Deep House (09-electronic): straight 4/4 electronic
//   - Jungle Break (13-drum-and-bass): chopped-break polyrhythm
//   - Reich Phase Process (08-minimalism): drift-based process music
//
// On failure, expect() messages name the specific control that broke so a
// regression can be attributed without re-instrumenting the test.

interface CapturedDownload {
  filename: string;
  base64: string;
}

interface NoteOnEvent {
  tick: number;
  note: number;
  velocity: number;
  channel: number;
}

const IFRAME_URL_PATTERN = /\/poly\/webui\/index\.html\?/;
const CAPTURE_TIMEOUT_MS = 30_000;
const DUMP_BEATS = 32;

const DOWNLOAD_SHIM = `
  (() => {
    const captures = [];
    window.__dumpCaptures = captures;
    const origCreate = URL.createObjectURL.bind(URL);
    const blobsByUrl = new Map();
    URL.createObjectURL = (blob) => {
      const url = origCreate(blob);
      blobsByUrl.set(url, blob);
      return url;
    };
    const origClick = HTMLAnchorElement.prototype.click;
    HTMLAnchorElement.prototype.click = function () {
      const blob = blobsByUrl.get(this.href);
      if (blob && this.download) {
        blob.arrayBuffer().then((buf) => {
          const bytes = new Uint8Array(buf);
          let bin = '';
          for (let i = 0; i < bytes.length; i++) bin += String.fromCharCode(bytes[i]);
          captures.push({ filename: this.download, base64: btoa(bin) });
        });
        return;
      }
      return origClick.apply(this, arguments);
    };
  })();
`;

// -------------------- SMF parse (kept local so this spec is independently
// runnable; equivalence.spec.ts holds its own copy — divergence between the
// two is a bug worth finding, not a maintenance burden) --------------------

function readBE32(b: Uint8Array, i: number): number {
  return ((b[i] << 24) >>> 0) | (b[i + 1] << 16) | (b[i + 2] << 8) | b[i + 3];
}
function readVLQ(b: Uint8Array, i: number): { value: number; next: number } {
  let value = 0;
  let j = i;
  for (let n = 0; n < 4; n++) {
    const c = b[j++];
    value = (value << 7) | (c & 0x7f);
    if ((c & 0x80) === 0) return { value, next: j };
  }
  return { value, next: j };
}

function parseNoteOns(bytes: Uint8Array): NoteOnEvent[] {
  if (bytes.length < 22) throw new Error(`SMF too short: ${bytes.length}`);
  if (String.fromCharCode(...bytes.slice(0, 4)) !== 'MThd') {
    throw new Error('missing MThd');
  }
  const trackHeaderOff = 14;
  const trackLen = readBE32(bytes, trackHeaderOff + 4);
  const trackStart = trackHeaderOff + 8;
  const trackEnd = trackStart + trackLen;
  const out: NoteOnEvent[] = [];
  let cur = trackStart;
  let tick = 0;
  let runningStatus = 0;
  while (cur < trackEnd) {
    const { value: delta, next } = readVLQ(bytes, cur);
    cur = next;
    tick += delta;
    let status = bytes[cur];
    if (status < 0x80) {
      status = runningStatus;
    } else {
      runningStatus = status;
      cur++;
    }
    if (status === 0xff) {
      const metaType = bytes[cur++];
      const { value: metaLen, next: nx } = readVLQ(bytes, cur);
      cur = nx + metaLen;
      if (metaType === 0x2f) break;
      continue;
    }
    if (status === 0xf0 || status === 0xf7) {
      const { value: sysLen, next: nx } = readVLQ(bytes, cur);
      cur = nx + sysLen;
      continue;
    }
    const kind = status & 0xf0;
    const channel = status & 0x0f;
    if (kind === 0x90) {
      const note = bytes[cur++];
      const vel = bytes[cur++];
      if (vel > 0) out.push({ tick, note, velocity: vel, channel });
    } else if (kind === 0x80) {
      cur += 2;
    } else if (kind === 0xc0 || kind === 0xd0) {
      cur++;
    } else {
      cur += 2;
    }
  }
  return out;
}

function b64ToBytes(b64: string): Uint8Array {
  return new Uint8Array(Buffer.from(b64, 'base64'));
}

function noteOnKey(e: NoteOnEvent): string {
  return `${e.tick}|${e.channel}|${e.note}|${e.velocity}`;
}

function countDivergence(a: NoteOnEvent[], b: NoteOnEvent[]): number {
  const setA = new Set(a.map(noteOnKey));
  const setB = new Set(b.map(noteOnKey));
  let d = 0;
  for (const k of setA) if (!setB.has(k)) d++;
  for (const k of setB) if (!setA.has(k)) d++;
  return d;
}

// -------------------- iframe boot helpers --------------------

async function waitForIframePreset(page: Page, expected: string): Promise<void> {
  await page.waitForFunction(
    (needle) => {
      const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
      const normalize = (s: string) => s.toLowerCase().replace(/[-_]/g, ' ');
      for (const f of frames) {
        const w = (f as HTMLIFrameElement).contentWindow as
          | (Window & { PolyWasmHost?: { getState?: () => { preset?: string } } })
          | null;
        if (!w || !w.PolyWasmHost || !w.PolyWasmHost.getState) continue;
        const s = w.PolyWasmHost.getState();
        if (s && typeof s.preset === 'string' && normalize(s.preset) === normalize(needle)) {
          return true;
        }
      }
      return false;
    },
    expected,
    { timeout: 25_000, polling: 200 },
  );
}

interface IframeProbe {
  playing: boolean;
  currentPreset: string;
  sceneAPreset: string;
  sceneBPreset: string;
  morphAmount: number;
  nodesStarted: number;
}

async function readIframeProbe(page: Page): Promise<IframeProbe> {
  return page.evaluate(() => {
    const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
    for (const f of frames) {
      const w = (f as HTMLIFrameElement).contentWindow as
        | (Window & { __polyAudioProbe?: IframeProbe })
        | null;
      const p = w?.__polyAudioProbe;
      if (p) return { ...p };
    }
    throw new Error('no iframe probe found');
  });
}

async function iframeEdit(page: Page, paramId: string, value: number): Promise<void> {
  await page.evaluate(
    ({ paramId, value }) => {
      const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
      for (const f of frames) {
        const w = (f as HTMLIFrameElement).contentWindow as
          | (Window & {
              PolyWasmHost?: {
                edit?: (id: string, v: number, gesture: string) => void;
              };
            })
          | null;
        if (w && w.PolyWasmHost && w.PolyWasmHost.edit) {
          w.PolyWasmHost.edit(paramId, value, 'begin');
          w.PolyWasmHost.edit(paramId, value, 'perform');
          w.PolyWasmHost.edit(paramId, value, 'end');
          return;
        }
      }
      throw new Error('no host reached');
    },
    { paramId, value },
  );
}

async function iframeApplyPresetByName(page: Page, name: string): Promise<void> {
  await page.evaluate((needle) => {
    const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
    const normalize = (s: string) => s.toLowerCase().replace(/[-_]/g, ' ');
    for (const f of frames) {
      const w = (f as HTMLIFrameElement).contentWindow as
        | (Window & {
            PolyWasmHost?: {
              getState?: () => { presets?: Array<{ name: string }> };
              action?: (n: string, p: Record<string, unknown>) => void;
            };
          })
        | null;
      if (!w || !w.PolyWasmHost) continue;
      const s = w.PolyWasmHost.getState?.();
      const idx = s?.presets?.findIndex((p) => normalize(p.name) === normalize(needle)) ?? -1;
      if (idx >= 0 && w.PolyWasmHost.action) {
        w.PolyWasmHost.action('applyPreset', { index: idx });
        return;
      }
    }
    throw new Error(`preset "${needle}" not found`);
  }, name);
}

async function waitForIframeCaptures(
  page: Page,
  prefix: string,
  minCount: number,
): Promise<CapturedDownload[]> {
  const handle = await page.waitForFunction(
    ({ prefix, minCount, pattern }) => {
      const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
      for (const f of frames) {
        const src = (f as HTMLIFrameElement).src;
        if (!new RegExp(pattern).test(src)) continue;
        const w = (f as HTMLIFrameElement).contentWindow as
          | (Window & { __dumpCaptures?: CapturedDownload[] })
          | null;
        const c = (w && w.__dumpCaptures) ?? [];
        const filtered = c.filter((cap) => cap.filename.startsWith(prefix));
        if (filtered.length >= minCount) return filtered;
      }
      return null;
    },
    { prefix, minCount, pattern: IFRAME_URL_PATTERN.source },
    { timeout: CAPTURE_TIMEOUT_MS, polling: 250 },
  );
  return (await handle.jsonValue()) as CapturedDownload[];
}

function pickMidi(caps: CapturedDownload[], suffix?: string): Uint8Array {
  const match = suffix
    ? [...caps].reverse().find((c) => c.filename.endsWith(suffix + '.mid') || c.filename.includes(suffix))
    : caps.find((c) => c.filename.endsWith('.mid'));
  if (!match) throw new Error(`no .mid capture — got [${caps.map((c) => c.filename).join(', ')}]`);
  return b64ToBytes(match.base64);
}

async function openTryItModal(
  page: Page,
  cardSelector: string,
  presetName: string,
): Promise<FrameLocator> {
  const card = page.locator(cardSelector);
  await expect(card).toBeVisible();
  const tryBtn = card.locator('.poly-preview-btn');
  await expect(tryBtn).toBeEnabled();
  await tryBtn.click();
  const frame = page.frameLocator('iframe.poly-modal-frame');
  const iframePlay = frame.locator('#play');
  await expect(iframePlay).toBeVisible({ timeout: 25_000 });
  await waitForIframePreset(page, presetName);
  return frame;
}

// -------------------- Chapters --------------------

interface ChapterSpec {
  preset: string;
  path: string;
  slug: string;
  // Second preset used for scene B isolation probe. Chosen to be a distinct
  // engineName from the primary so the probe currentPreset flip after
  // applyPreset can be observed reliably.
  otherPreset: string;
}

const AUDIT_CHAPTERS: ChapterSpec[] = [
  {
    preset: 'Deep House',
    path: '/poly/09-electronic/',
    slug: 'deep-house',
    otherPreset: 'Minimal Techno',
  },
  {
    preset: 'Jungle Break',
    path: '/poly/13-drum-and-bass/',
    slug: 'jungle-break',
    otherPreset: 'Liquid Drum and Bass',
  },
  {
    preset: 'Reich Phase Process',
    path: '/poly/08-minimalism/',
    slug: 'reich-phase-process',
    otherPreset: 'Riley Layered Entry',
  },
];

// -------------------- T04: layout probes --------------------

test.describe('S14 T04 — Try It modal fits at every tested viewport', () => {
  const VIEWPORTS = [
    { name: 'mobile', width: 375, height: 800 },
    { name: 'tablet', width: 768, height: 900 },
    { name: 'desktop', width: 1280, height: 900 },
  ];

  for (const vp of VIEWPORTS) {
    test(`modal has no horizontal overflow at ${vp.name} (${vp.width}px)`, async ({ page }) => {
      await page.setViewportSize({ width: vp.width, height: vp.height });
      await page.goto('/poly/09-electronic/');
      const card = page.locator('.poly-preview[data-poly-preset="Deep House"]');
      await expect(card).toBeVisible();
      await card.locator('.poly-preview-btn').click();
      const frame = page.frameLocator('iframe.poly-modal-frame');
      await expect(frame.locator('#play')).toBeVisible({ timeout: 25_000 });

      // scrollWidth captures the natural width of content; when a child
      // overflows the modal's overflow:hidden, scrollWidth > clientWidth.
      // The 1px cushion absorbs sub-pixel rounding that some browsers apply
      // to widths involving vw units.
      const dims = await page.evaluate(() => {
        const dlg = document.querySelector<HTMLElement>('.poly-modal');
        if (!dlg) throw new Error('no modal');
        return {
          scrollWidth: dlg.scrollWidth,
          clientWidth: dlg.clientWidth,
        };
      });
      expect(
        dims.scrollWidth,
        `${vp.name} viewport: modal scrollWidth ${dims.scrollWidth} > clientWidth ${dims.clientWidth} — right-edge truncation regression`,
      ).toBeLessThanOrEqual(dims.clientWidth + 1);
    });
  }
});

test.describe('S14 T04 — Play and Try It buttons share sizing', () => {
  test('Play + Try It buttons match height and border-radius on Deep House card', async ({ page }) => {
    await page.goto('/poly/09-electronic/');
    const card = page.locator('.poly-preview[data-poly-preset="Deep House"]');
    await expect(card).toBeVisible();
    const sizes = await card.evaluate((root) => {
      const play = root.querySelector<HTMLElement>('[data-role="play"]');
      const tryit = root.querySelector<HTMLElement>('[data-role="tryit"]');
      if (!play || !tryit) throw new Error('missing play or tryit button');
      const playRect = play.getBoundingClientRect();
      const tryRect = tryit.getBoundingClientRect();
      const playRadius = getComputedStyle(play).borderRadius;
      const tryRadius = getComputedStyle(tryit).borderRadius;
      return {
        playH: playRect.height,
        tryH: tryRect.height,
        playRadius,
        tryRadius,
      };
    });
    expect(
      Math.abs(sizes.playH - sizes.tryH),
      `button height delta ${Math.abs(sizes.playH - sizes.tryH).toFixed(2)}px (play=${sizes.playH.toFixed(2)}, tryit=${sizes.tryH.toFixed(2)})`,
    ).toBeLessThanOrEqual(2);
    expect(
      sizes.playRadius,
      `border-radius mismatch: play=${sizes.playRadius} tryit=${sizes.tryRadius}`,
    ).toBe(sizes.tryRadius);
  });
});

// -------------------- T05: per-control audit on 3 chapters --------------------

for (const chapter of AUDIT_CHAPTERS) {
  test.describe(`S14 T05 — control audit [${chapter.preset}]`, () => {
    test(`[${chapter.preset}] Play → probe.playing true, Stop → false`, async ({ page }) => {
      await page.goto(chapter.path);
      const frame = await openTryItModal(
        page,
        `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        chapter.preset,
      );
      const before = await readIframeProbe(page);
      expect(before.playing, `${chapter.preset}: probe.playing must start false`).toBe(false);
      await frame.locator('#play').click();
      await page.waitForFunction(
        () => {
          const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
          for (const f of frames) {
            const w = (f as HTMLIFrameElement).contentWindow as
              | (Window & { __polyAudioProbe?: { playing?: boolean } })
              | null;
            if (w?.__polyAudioProbe?.playing === true) return true;
          }
          return false;
        },
        null,
        { timeout: 5000, polling: 100 },
      );
      const playing = await readIframeProbe(page);
      expect(
        playing.playing,
        `${chapter.preset}: after Play click, probe.playing must be true`,
      ).toBe(true);
      await frame.locator('#play').click();
      await page.waitForFunction(
        () => {
          const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
          for (const f of frames) {
            const w = (f as HTMLIFrameElement).contentWindow as
              | (Window & { __polyAudioProbe?: { playing?: boolean } })
              | null;
            if (w?.__polyAudioProbe?.playing === false) return true;
          }
          return false;
        },
        null,
        { timeout: 5000, polling: 100 },
      );
      const stopped = await readIframeProbe(page);
      expect(
        stopped.playing,
        `${chapter.preset}: after Stop click, probe.playing must be false`,
      ).toBe(false);
    });

    test(`[${chapter.preset}] scene B load leaves scene A preset intact`, async ({ page }) => {
      await page.goto(chapter.path);
      const frame = await openTryItModal(
        page,
        `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        chapter.preset,
      );
      // Scene A already holds chapter.preset (set via applyParamsToHost on
      // boot). Switch to B, load a distinct preset there, switch back to A,
      // and confirm A's preset name survived — the exact regression the T01
      // fix protects against.
      await frame.locator('#scB').click();
      await iframeApplyPresetByName(page, chapter.otherPreset);
      const onB = await readIframeProbe(page);
      expect(
        onB.sceneBPreset.toLowerCase(),
        `${chapter.preset}: after loading ${chapter.otherPreset} into B, probe.sceneBPreset should reflect it`,
      ).toBe(chapter.otherPreset.toLowerCase());
      await frame.locator('#scA').click();
      const onA = await readIframeProbe(page);
      expect(
        onA.sceneAPreset.toLowerCase(),
        `${chapter.preset}: scene A/B isolation — after B load then back to A, sceneAPreset must still be ${chapter.preset}`,
      ).toBe(chapter.preset.toLowerCase());
      expect(
        onA.currentPreset.toLowerCase(),
        `${chapter.preset}: scene A/B isolation — probe.currentPreset must flip back to A's preset`,
      ).toBe(chapter.preset.toLowerCase());
    });

    test(`[${chapter.preset}] morph slider (0.5) reaches engine`, async ({ page }) => {
      await page.goto(chapter.path);
      const frame = await openTryItModal(
        page,
        `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        chapter.preset,
      );
      // Enter Morph mode so the slider is visible + the render path uses it.
      await frame.locator('#scM').click();
      await iframeEdit(page, 'scene.morph', 0.5);
      const probe = await readIframeProbe(page);
      expect(
        Math.abs(probe.morphAmount - 0.5),
        `${chapter.preset}: probe.morphAmount ${probe.morphAmount} != 0.5 after scene.morph edit`,
      ).toBeLessThan(0.01);
    });

    test(`[${chapter.preset}] muting a lane suppresses its note-ons in the dump`, async ({ page }) => {
      // Try It's `dumpFiredThisPage` guard fires the dump once per page load,
      // so the mute must happen BEFORE the first Play click. Lane-mute.spec
      // already proves note-ons return once the mute is released; here we
      // only need to prove the mute → dump path across every audit chapter.
      //
      // Reich-style presets share the SAME MIDI note across multiple lanes
      // by design (two hands, same bell pattern, different phases). Muting
      // lane 0 in that world doesn't zero the note — the other lane still
      // fires it. So we pick a lane whose MIDI note is unique across the
      // preset's lanes; that gives us a clean "before → 0" signal without
      // false-firing on legitimate shared-note designs. If no such lane
      // exists we skip and log — that's a preset-shape statement, not a
      // regression.
      await page.addInitScript(DOWNLOAD_SHIM);
      await page.goto(`${chapter.path}?dump=1&dumpBeats=${DUMP_BEATS}`);
      const frame = await openTryItModal(
        page,
        `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        chapter.preset,
      );
      const laneInfo = await page.evaluate(() => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        for (const f of frames) {
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { PolyWasmHost?: { getState?: () => { lanes: Array<{ note: number }> } } })
            | null;
          const s = w?.PolyWasmHost?.getState?.();
          if (s?.lanes) {
            const notes = s.lanes.map((l) => l.note);
            const counts = new Map<number, number>();
            for (const n of notes) counts.set(n, (counts.get(n) ?? 0) + 1);
            for (let i = 0; i < notes.length; i++) {
              if (counts.get(notes[i]) === 1) return { laneIndex: i, note: notes[i] };
            }
            return { laneIndex: -1, note: -1 };
          }
        }
        throw new Error('no lanes');
      });
      test.skip(
        laneInfo.laneIndex < 0,
        `every lane shares its MIDI note with another lane — no unique-note lane to isolate for mute`,
      );
      await frame.locator(`.strip[data-lane="${laneInfo.laneIndex}"] [data-mute]`).click();
      await page.waitForFunction(
        (idx) => {
          const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
          for (const f of frames) {
            const w = (f as HTMLIFrameElement).contentWindow as
              | (Window & { PolyWasmHost?: { getState?: () => { lanes: Array<{ active: boolean }> } } })
              | null;
            const s = w?.PolyWasmHost?.getState?.();
            if (s?.lanes?.[idx]?.active === false) return true;
          }
          return false;
        },
        laneInfo.laneIndex,
        { timeout: 5000, polling: 100 },
      );
      await frame.locator('#play').click();
      const caps = await waitForIframeCaptures(page, `dump-tryit-${chapter.slug}-`, 2);
      const notes = parseNoteOns(pickMidi(caps));
      const targetCount = notes.filter((n) => n.note === laneInfo.note).length;
      expect(
        targetCount,
        `${chapter.preset}: after muting lane ${laneInfo.laneIndex}, dump must contain zero note-ons for note ${laneInfo.note}`,
      ).toBe(0);
      expect(notes.length, 'muted dump should still contain other lanes').toBeGreaterThan(0);
    });

    test(`[${chapter.preset}] every macro slider edit reaches the engine`, async ({ page }) => {
      // Sliders promise "changing me changes the value in the engine". The
      // audible impact varies per preset (a deterministic-groove preset may
      // not show every macro in a short SMF window), so the honest label→
      // effect assertion is: after edit, the engine's macro readback equals
      // the value we set. macro readback lives on Context via
      // poly_macro_value(), surfaced through PolyWasmHost.getState().macros.
      await page.goto(chapter.path);
      await openTryItModal(
        page,
        `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        chapter.preset,
      );
      const macros = ['complexity', 'density', 'syncopation', 'swing', 'tension', 'humanize'];
      for (const macroName of macros) {
        await iframeEdit(page, `macro.${macroName}`, 0.73);
        const observed = await page.evaluate((needle) => {
          const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
          for (const f of frames) {
            const w = (f as HTMLIFrameElement).contentWindow as
              | (Window & { PolyWasmHost?: { getState?: () => { macros?: Record<string, number> } } })
              | null;
            const s = w?.PolyWasmHost?.getState?.();
            const v = s?.macros?.[needle];
            if (typeof v === 'number') return v;
          }
          throw new Error(`no macro readback for ${needle}`);
        }, macroName);
        expect(
          Math.abs(observed - 0.73),
          `${chapter.preset}: macro.${macroName} edit (0.73) did not reach engine — readback=${observed}`,
        ).toBeLessThan(0.01);
      }
    });
  });
}
