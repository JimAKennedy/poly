import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// M043 S15 T03 — Macro-diff gate.
//
// Directly tests the T01 fix (wasm_api.cpp poly_render calls resolveMacros +
// resolveConstraints) end-to-end on the Try It surface: for each canonical
// preset+macro pair we capture the SMF dump at macro=0 and macro=1 and assert
// the two byte-sequences differ. If they don't, either T01 was reverted or the
// dump path is not seeing the user's macro edits.
//
// Together with the T02 fix (playbackCtx macro seed) and the T03-bundled
// fireDumpTryIt macro seed, moving a macro slider before Play must produce a
// distinctly different SMF than the same preset at the opposite macro extreme.
//
// Presets:
//   - Deep House             (09-electronic/, slug deep-house)
//   - Rachenitsa 7/8         (07-balkan/,     slug rachenitsa-7-8)
//
// The plan named "Bulgarian Wedding" as the second preset; the actual preset
// list uses "Rachenitsa 7/8" for the Balkan chapter — same family, same
// polymetric character, so we use the canonical name.

const DUMP_BEATS = 32;
const CAPTURE_TIMEOUT_MS = 30_000;
const IFRAME_URL_PATTERN = /\/poly\/webui\/index\.html\?/;

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

// -------------------- SMF parsing (local copy — equivalence.spec.ts and
// control-audit.spec.ts each keep their own; divergence between the three
// would be a bug worth catching, not a shared-module burden) --------------

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

// -------------------- iframe boot / edit helpers --------------------

async function waitForIframePreset(
  page: import('@playwright/test').Page,
  expected: string,
): Promise<void> {
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

async function iframeEdit(
  page: import('@playwright/test').Page,
  paramId: string,
  value: number,
): Promise<void> {
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

async function waitForIframeCaptures(
  page: import('@playwright/test').Page,
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

function pickMidi(caps: CapturedDownload[]): Uint8Array {
  const match = caps.find((c) => c.filename.endsWith('.mid'));
  if (!match) {
    throw new Error(`no .mid capture — got [${caps.map((c) => c.filename).join(', ')}]`);
  }
  return b64ToBytes(match.base64);
}

// -------------------- one macro-diff capture round --------------------

interface CaptureResult {
  bytes: Uint8Array;
  noteOns: NoteOnEvent[];
}

async function captureDumpAtMacro(
  page: import('@playwright/test').Page,
  chapterPath: string,
  preset: string,
  slug: string,
  macroName: string,
  macroValue: number,
): Promise<CaptureResult> {
  await page.goto(`${chapterPath}?dump=1&dumpBeats=${DUMP_BEATS}`);
  const card = page.locator(`.poly-preview[data-poly-preset="${preset}"]`);
  await expect(card).toBeVisible();
  const tryBtn = card.locator('.poly-preview-btn');
  await expect(tryBtn).toBeEnabled();
  await tryBtn.click();

  const frame = page.frameLocator('iframe.poly-modal-frame');
  await expect(frame.locator('#play')).toBeVisible({ timeout: 25_000 });
  await waitForIframePreset(page, preset);

  // Push the macro BEFORE the Play click so the fireDumpTryIt scratch
  // context sees the user-set value. The T03-bundled fireDumpTryIt macro
  // seed mirrors engineCtx macros into scratch after `_poly_load_preset`
  // resets them.
  await iframeEdit(page, `macro.${macroName}`, macroValue);

  await frame.locator('#play').click();
  const caps = await waitForIframeCaptures(page, `dump-tryit-${slug}-`, 2);
  const bytes = pickMidi(caps);
  return { bytes, noteOns: parseNoteOns(bytes) };
}

interface FirstDiff {
  tick: number;
  playLow: NoteOnEvent | null;
  playHigh: NoteOnEvent | null;
}

function firstDifferingNoteOn(
  low: NoteOnEvent[],
  high: NoteOnEvent[],
): FirstDiff | null {
  const lowSet = new Set(low.map(noteOnKey));
  const highSet = new Set(high.map(noteOnKey));
  const merged = [...low, ...high].sort((a, b) => a.tick - b.tick || a.note - b.note);
  for (const e of merged) {
    const k = noteOnKey(e);
    if (!lowSet.has(k) || !highSet.has(k)) {
      return {
        tick: e.tick,
        playLow: low.find((x) => noteOnKey(x) === k) ?? null,
        playHigh: high.find((x) => noteOnKey(x) === k) ?? null,
      };
    }
  }
  return null;
}

// -------------------- summary artifact --------------------

interface MacroDiffCaseResult {
  preset: string;
  slug: string;
  macro: string;
  lowBytes: number;
  highBytes: number;
  lowNoteOnCount: number;
  highNoteOnCount: number;
  byteIdentical: boolean;
  firstDifferingTick: number | null;
  pass: boolean;
  failReason?: string;
}

// Same per-file-then-merge pattern equivalence.spec.ts uses — survives
// Playwright's worker restart on test failure so a single spec crash
// doesn't lose earlier results.
const PER_DIR = path.resolve(process.cwd(), 'test-results', 'macro-diff-per');

function writePerCase(record: MacroDiffCaseResult): void {
  fs.mkdirSync(PER_DIR, { recursive: true });
  const fname = `${record.slug}_${record.macro}.json`;
  fs.writeFileSync(path.join(PER_DIR, fname), JSON.stringify(record, null, 2));
}

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const cases: MacroDiffCaseResult[] = [];
  if (fs.existsSync(PER_DIR)) {
    for (const file of fs.readdirSync(PER_DIR).sort()) {
      if (!file.endsWith('.json')) continue;
      cases.push(JSON.parse(fs.readFileSync(path.join(PER_DIR, file), 'utf8')));
    }
  }
  const pass = cases.filter((c) => c.pass).length;
  const fail = cases.length - pass;
  const summary = {
    gate: 'S15-macro-diff',
    verdict: fail === 0 && cases.length > 0 ? 'pass' : 'fail',
    caseCount: cases.length,
    passCount: pass,
    failCount: fail,
    cases,
  };
  const outPath = path.join(outDir, 'macro-diff-summary.json');
  fs.writeFileSync(outPath, JSON.stringify(summary, null, 2));
  // eslint-disable-next-line no-console
  console.log(
    `[macro-diff] wrote ${outPath} (${cases.length} cases, ${pass} pass, ${fail} fail)`,
  );
});

// -------------------- cases --------------------

interface Case {
  preset: string;
  path: string;
  slug: string;
  macro: string;
}

const CASES: Case[] = [
  { preset: 'Deep House', path: '/poly/09-electronic/', slug: 'deep-house', macro: 'complexity' },
  { preset: 'Rachenitsa 7/8', path: '/poly/07-balkan/', slug: 'rachenitsa-7-8', macro: 'complexity' },
  { preset: 'Deep House', path: '/poly/09-electronic/', slug: 'deep-house', macro: 'density' },
];

test.describe('S15 macro-diff — user macro edits reach the Try It dump path', () => {
  for (const c of CASES) {
    test(`[${c.preset}] macro.${c.macro}=0 and =1 produce different SMFs`, async ({
      page,
    }) => {
      const record: MacroDiffCaseResult = {
        preset: c.preset,
        slug: c.slug,
        macro: c.macro,
        lowBytes: 0,
        highBytes: 0,
        lowNoteOnCount: 0,
        highNoteOnCount: 0,
        byteIdentical: true,
        firstDifferingTick: null,
        pass: false,
      };
      const persist = () => writePerCase(record);

      try {
        await page.addInitScript(DOWNLOAD_SHIM);

        const low = await captureDumpAtMacro(page, c.path, c.preset, c.slug, c.macro, 0.0);
        const high = await captureDumpAtMacro(page, c.path, c.preset, c.slug, c.macro, 1.0);

        record.lowBytes = low.bytes.length;
        record.highBytes = high.bytes.length;
        record.lowNoteOnCount = low.noteOns.length;
        record.highNoteOnCount = high.noteOns.length;

        const identical =
          low.bytes.length === high.bytes.length &&
          low.bytes.every((b, i) => b === high.bytes[i]);
        record.byteIdentical = identical;

        const firstDiff = identical ? null : firstDifferingNoteOn(low.noteOns, high.noteOns);
        record.firstDifferingTick = firstDiff?.tick ?? null;

        // The gate: macro=0 and macro=1 MUST produce different byte-sequences.
        // If they don't, either T01's resolveMacros is missing or the dump
        // path is dropping the macro edit before the render call.
        expect(
          identical,
          `[${c.preset}] macro.${c.macro}: dumps are byte-identical (${low.bytes.length} bytes both) — ` +
            `either wasm_api.cpp poly_render dropped resolveMacros (T01 regression) or fireDumpTryIt ` +
            `is not mirroring engineCtx macros into scratch (T03 regression). ` +
            `note-on counts low=${low.noteOns.length} high=${high.noteOns.length}`,
        ).toBe(false);

        record.pass = true;
      } catch (err) {
        record.failReason = err instanceof Error ? err.message : String(err);
        throw err;
      } finally {
        persist();
      }
    });
  }
});
