import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// Play ↔ Try It equivalence gate — one test per preview card.
//
// Both surfaces now render through the same `_poly_render` code path
// (engine-scheduler.ts on the card, wasm-host.js on Try It), so byte-identity
// is structural. This spec is the smoke gate that proves the wiring: it
// captures the SMF + params.json companion dumps from each surface for each
// chapter's preset and asserts:
//
//   1. Both surfaces produce the same set of note-on events (tick, note,
//      velocity, channel). Note-off ticks are compared separately and
//      reported but do NOT hard-fail — off-tick shape can still diverge
//      through legitimate engine paths (e.g. subdivision-derived duration).
//
//   2. Both surfaces agree on the intersection of preset params
//      (engineName, bpm, seed, per-lane {noteNumber, cycleSteps,
//      subdivision, stepLen, hits, rotation, velocity, probability}). This
//      catches "the two paths think the preset says different things"
//      before it even gets to render.
//
// Both surfaces use ?dumpBeats=32 to render a fixed 32-quarter window so a
// preset with a 12-beat loop and a preset with a 32-beat loop compare over
// the same interval. The Play card fires a scratch-context `_poly_render`
// synchronously on click; Try It fires poly_render in chunkQuarters=4
// slices inside fireDumpTryIt() on the iframe's #play click.

const DUMP_BEATS = 32;

interface ChapterSpec {
  preset: string;
  path: string;
  // URL-encoded slug used inside `dump-play-<slug>-<ts>.mid`. Sourced by
  // running slugifyPresetName over the resolved preset display name — the
  // same slug both surfaces emit, so a mismatch here would produce
  // capture-not-found errors, not a false pass.
  slug: string;
}

// Same 27 chapters as preset-consistency.spec.ts. Kept as a local copy so this
// spec is independently readable; a divergence in the two lists is a bug worth
// finding, not a maintenance burden worth hiding.
const CHAPTERS: ChapterSpec[] = [
  { preset: 'Polymetric Foundation', path: '/01-foundations/', slug: 'polymetric-foundation' },
  { preset: 'Ewe Polymetric Ensemble', path: '/02-sub-saharan-africa/', slug: 'ewe-polymetric-ensemble' },
  { preset: 'Manding Djembe', path: '/02-sub-saharan-africa/', slug: 'manding-djembe' },
  { preset: 'Cuban Son Montuno', path: '/03-afro-cuban/', slug: 'cuban-son-montuno' },
  { preset: 'Afrobeat Lagos', path: '/04-afrobeat/', slug: 'afrobeat-lagos' },
  { preset: 'Balinese Kotekan', path: '/05-gamelan/', slug: 'balinese-kotekan' },
  { preset: 'Javanese Colotomic', path: '/05-gamelan/', slug: 'javanese-colotomic' },
  { preset: 'Tintal Groove', path: '/06-indian-classical/', slug: 'tintal-groove' },
  { preset: 'Rupak Tal', path: '/06-indian-classical/', slug: 'rupak-tal' },
  { preset: 'Rachenitsa 7/8', path: '/07-balkan/', slug: 'rachenitsa-7-8' },
  { preset: 'Kopanitsa 11/8', path: '/07-balkan/', slug: 'kopanitsa-11-8' },
  { preset: 'Reich Phase Process', path: '/08-minimalism/', slug: 'reich-phase-process' },
  { preset: 'Riley Layered Entry', path: '/08-minimalism/', slug: 'riley-layered-entry' },
  { preset: 'Nancarrow Tempi', path: '/08-minimalism/', slug: 'nancarrow-tempi' },
  { preset: 'Minimal Techno', path: '/09-electronic/', slug: 'minimal-techno' },
  { preset: 'Deep House', path: '/09-electronic/', slug: 'deep-house' },
  { preset: 'Samba Batucada', path: '/10-brazilian/', slug: 'samba-batucada' },
  { preset: 'Bossa Nova Trio', path: '/10-brazilian/', slug: 'bossa-nova-trio' },
  { preset: 'Classic Funk', path: '/11-funk-soul/', slug: 'classic-funk' },
  { preset: 'Neo-Soul Pocket', path: '/11-funk-soul/', slug: 'neo-soul-pocket' },
  { preset: 'Jazz Bop Ride', path: '/12-jazz/', slug: 'jazz-bop-ride' },
  { preset: 'Elvin Jones Cascade', path: '/12-jazz/', slug: 'elvin-jones-cascade' },
  { preset: 'Jungle Break', path: '/13-drum-and-bass/', slug: 'jungle-break' },
  { preset: 'Liquid Drum and Bass', path: '/13-drum-and-bass/', slug: 'liquid-drum-and-bass' },
  { preset: 'Afro-Electronic Fusion', path: '/14-synthesis/', slug: 'afro-electronic-fusion' },
  { preset: 'Balkan Funk', path: '/14-synthesis/', slug: 'balkan-funk' },
  { preset: 'Compositional Arc', path: '/15-compositional-grammar/', slug: 'compositional-arc' },
];

const CAPTURE_TIMEOUT_MS = 30_000;
const IFRAME_URL_PATTERN = /\/webui\/index\.html\?/;

interface CapturedDownload {
  filename: string;
  base64: string;
}

// addInitScript applies to every frame Playwright creates for this test: parent
// window AND the same-origin iframe. Both surfaces route Blob URLs through
// URL.createObjectURL + anchor.click, so a single shim catches all four
// downloads (2 from Play card, 2 from Try It modal).
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

// -------------------- SMF parsing --------------------

interface NoteOnEvent {
  tick: number;
  note: number;
  velocity: number;
  channel: number;
}

interface NoteOffEvent {
  tick: number;
  note: number;
  channel: number;
}

interface ParsedSmf {
  ppq: number;
  tempoUsPerQ: number | null;
  noteOns: NoteOnEvent[];
  noteOffs: NoteOffEvent[];
  totalBytes: number;
  trackLen: number;
}

function readBE16(bytes: Uint8Array, i: number): number {
  return (bytes[i] << 8) | bytes[i + 1];
}

function readBE32(bytes: Uint8Array, i: number): number {
  return (
    (bytes[i] << 24) >>> 0 |
    (bytes[i + 1] << 16) |
    (bytes[i + 2] << 8) |
    bytes[i + 3]
  );
}

function readVLQ(bytes: Uint8Array, i: number): { value: number; next: number } {
  let value = 0;
  let j = i;
  for (let n = 0; n < 4; n++) {
    const b = bytes[j++];
    value = (value << 7) | (b & 0x7f);
    if ((b & 0x80) === 0) return { value, next: j };
  }
  return { value, next: j };
}

function parseSmf(bytes: Uint8Array): ParsedSmf {
  if (bytes.length < 22) throw new Error(`SMF too short: ${bytes.length} bytes`);
  if (String.fromCharCode(...bytes.slice(0, 4)) !== 'MThd') {
    throw new Error(`missing MThd, got "${String.fromCharCode(...bytes.slice(0, 4))}"`);
  }
  const headerLen = readBE32(bytes, 4);
  if (headerLen !== 6) throw new Error(`unexpected MThd length ${headerLen}`);
  const ppq = readBE16(bytes, 12);

  const trackHeaderOff = 14;
  if (String.fromCharCode(...bytes.slice(trackHeaderOff, trackHeaderOff + 4)) !== 'MTrk') {
    throw new Error('missing MTrk');
  }
  const trackLen = readBE32(bytes, trackHeaderOff + 4);
  const trackStart = trackHeaderOff + 8;
  const trackEnd = trackStart + trackLen;

  const noteOns: NoteOnEvent[] = [];
  const noteOffs: NoteOffEvent[] = [];
  let tempoUsPerQ: number | null = null;

  let cur = trackStart;
  let tick = 0;
  let runningStatus = 0;

  while (cur < trackEnd) {
    const { value: delta, next } = readVLQ(bytes, cur);
    cur = next;
    tick += delta;
    let status = bytes[cur];
    if (status < 0x80) {
      // running status: reuse last status byte
      status = runningStatus;
    } else {
      runningStatus = status;
      cur++;
    }
    if (status === 0xff) {
      const metaType = bytes[cur++];
      const { value: metaLen, next: nx } = readVLQ(bytes, cur);
      cur = nx;
      if (metaType === 0x51 && metaLen === 3) {
        tempoUsPerQ = (bytes[cur] << 16) | (bytes[cur + 1] << 8) | bytes[cur + 2];
      }
      cur += metaLen;
      if (metaType === 0x2f) break; // end of track
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
      if (vel > 0) noteOns.push({ tick, note, velocity: vel, channel });
      else noteOffs.push({ tick, note, channel });
    } else if (kind === 0x80) {
      const note = bytes[cur++];
      cur++; // release velocity
      noteOffs.push({ tick, note, channel });
    } else if (kind === 0xc0 || kind === 0xd0) {
      cur++;
    } else {
      cur += 2;
    }
  }

  return { ppq, tempoUsPerQ, noteOns, noteOffs, totalBytes: bytes.length, trackLen };
}

// -------------------- comparators --------------------

interface NoteOnDiff {
  onlyInPlay: NoteOnEvent[];
  onlyInTryIt: NoteOnEvent[];
  common: number;
}

function keyOn(e: NoteOnEvent): string {
  return `${e.tick}|${e.channel}|${e.note}|${e.velocity}`;
}

function diffNoteOns(play: NoteOnEvent[], tryIt: NoteOnEvent[]): NoteOnDiff {
  const playSet = new Set(play.map(keyOn));
  const tryItSet = new Set(tryIt.map(keyOn));
  const onlyInPlay = play.filter((e) => !tryItSet.has(keyOn(e)));
  const onlyInTryIt = tryIt.filter((e) => !playSet.has(keyOn(e)));
  let common = 0;
  for (const k of playSet) if (tryItSet.has(k)) common++;
  return { onlyInPlay, onlyInTryIt, common };
}

interface ParamDiffField {
  path: string;
  play: unknown;
  tryIt: unknown;
}

function pushIfDiff(
  out: ParamDiffField[],
  path: string,
  a: unknown,
  b: unknown,
): void {
  // Number comparison with a tiny epsilon so 0.19999999 vs 0.2 (float
  // round-trip through JSON.stringify + parse) doesn't produce noise. Anything
  // wider than 1e-4 is a real difference worth surfacing.
  if (typeof a === 'number' && typeof b === 'number') {
    if (Math.abs(a - b) > 1e-4) out.push({ path, play: a, tryIt: b });
    return;
  }
  if (a !== b) out.push({ path, play: a, tryIt: b });
}

// Both surfaces now emit the same EngineParams shape (buildEngineParams on the
// Play side, buildTryItParams on the Try It side) since Play was refactored to
// render through _poly_render. The lane fields the two produce are structurally
// identical — the diff below compares the fields both agree on.
interface EngineDumpParams {
  source: string;
  displayName: string;
  engineName: string;
  bpm: number;
  seed: number;
  macros?: Record<string, number>;
  lanes: Array<{
    laneIndex: number;
    noteNumber: number;
    roleLabel: string;
    role: string;
    cycleSteps: number;
    subdivision: number;
    stepLen: number;
    hits: number;
    rotation: number;
    velocity: number;
    probability: number;
    velocitySpread?: number;
    humanizeMs?: number;
    swing?: number;
    noteDuration?: number;
    driftRate?: number;
    timingOffsetMs?: number;
    active?: boolean;
    ghostFloor?: number;
  }>;
}

function diffParams(play: EngineDumpParams, tryIt: EngineDumpParams): ParamDiffField[] {
  const out: ParamDiffField[] = [];
  pushIfDiff(out, 'engineName', play.engineName, tryIt.engineName);
  pushIfDiff(out, 'bpm', play.bpm, tryIt.bpm);
  pushIfDiff(out, 'seed', play.seed, tryIt.seed);
  const laneCount = Math.max(play.lanes.length, tryIt.lanes.length);
  pushIfDiff(out, 'laneCount', play.lanes.length, tryIt.lanes.length);
  for (let i = 0; i < laneCount; i++) {
    const p = play.lanes[i];
    const t = tryIt.lanes[i];
    if (!p || !t) {
      out.push({ path: `lanes[${i}]`, play: p ?? null, tryIt: t ?? null });
      continue;
    }
    pushIfDiff(out, `lanes[${i}].noteNumber`, p.noteNumber, t.noteNumber);
    pushIfDiff(out, `lanes[${i}].roleLabel`, p.roleLabel, t.roleLabel);
    pushIfDiff(out, `lanes[${i}].cycleSteps`, p.cycleSteps, t.cycleSteps);
    pushIfDiff(out, `lanes[${i}].subdivision`, p.subdivision, t.subdivision);
    // stepLen intentionally NOT compared here even though both surfaces derive
    // it from `subdivision` the same way — the cache is redundant with the
    // subdivision check above, and skipping it keeps the diff focused on the
    // primary field.
    pushIfDiff(out, `lanes[${i}].hits`, p.hits, t.hits);
    pushIfDiff(out, `lanes[${i}].rotation`, p.rotation, t.rotation);
    pushIfDiff(out, `lanes[${i}].velocity`, p.velocity, t.velocity);
    pushIfDiff(out, `lanes[${i}].probability`, p.probability, t.probability);
  }
  return out;
}

// -------------------- capture helpers --------------------

async function waitForParentCaptures(
  page: import('@playwright/test').Page,
  prefix: string,
  count: number,
): Promise<CapturedDownload[]> {
  const handle = await page.waitForFunction(
    ({ prefix, count }) => {
      const w = window as unknown as { __dumpCaptures?: CapturedDownload[] };
      const c = w.__dumpCaptures ?? [];
      const filtered = c.filter((cap) => cap.filename.startsWith(prefix));
      return filtered.length >= count ? filtered : null;
    },
    { prefix, count },
    { timeout: CAPTURE_TIMEOUT_MS, polling: 250 },
  );
  return (await handle.jsonValue()) as CapturedDownload[];
}

async function waitForIframeCaptures(
  page: import('@playwright/test').Page,
  prefix: string,
  count: number,
): Promise<CapturedDownload[]> {
  const handle = await page.waitForFunction(
    ({ prefix, count, pattern }) => {
      const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
      for (const f of frames) {
        const src = (f as HTMLIFrameElement).src;
        if (!new RegExp(pattern).test(src)) continue;
        const w = (f as HTMLIFrameElement).contentWindow as
          | (Window & { __dumpCaptures?: CapturedDownload[] })
          | null;
        const c = (w && w.__dumpCaptures) ?? [];
        const filtered = c.filter((cap) => cap.filename.startsWith(prefix));
        if (filtered.length >= count) return filtered;
      }
      return null;
    },
    { prefix, count, pattern: IFRAME_URL_PATTERN.source },
    { timeout: CAPTURE_TIMEOUT_MS, polling: 250 },
  );
  return (await handle.jsonValue()) as CapturedDownload[];
}

function b64ToBytes(b64: string): Uint8Array {
  return new Uint8Array(Buffer.from(b64, 'base64'));
}

function jsonFromCapture(cap: CapturedDownload): unknown {
  const bin = Buffer.from(cap.base64, 'base64').toString('utf8');
  return JSON.parse(bin);
}

function pickCapture(caps: CapturedDownload[], suffix: string): CapturedDownload {
  const match = caps.find((c) => c.filename.endsWith(suffix));
  if (!match) {
    const list = caps.map((c) => c.filename).join(', ');
    throw new Error(`no capture ending in ${suffix} — got [${list}]`);
  }
  return match;
}

// -------------------- summary --------------------

interface ChapterEquivalence {
  chapter: string;
  slug: string;
  pass: boolean;
  smfByteIdentical: boolean;
  playNoteOnCount: number;
  tryItNoteOnCount: number;
  noteOnDiff: {
    onlyInPlayCount: number;
    onlyInTryItCount: number;
    sampleOnlyInPlay: NoteOnEvent[];
    sampleOnlyInTryIt: NoteOnEvent[];
  };
  // Fuzzy match dimensions to distinguish "different rhythm" from "same
  // rhythm, different shaping". Ordered strict → loose so the user can read
  // down the list and stop at the first PASS to characterise the gap:
  //   fuzzy.exact         — same set of (tick, note, vel, ch)
  //   fuzzy.ignoreChannel — same set of (tick, note, vel), channels differ
  //   fuzzy.ignoreVelCh   — same set of (tick, note), vel + ch differ
  //   fuzzy.timingTol     — same set of (round(tick/60), note) — 30 ticks =
  //                          ~62.5 ms at 120 BPM 480 ppq, so humanize/swing
  //                          within a normal engine range still matches
  //   fuzzy.noteSet       — same set of notes at all, ignoring positions
  fuzzy: {
    exact: boolean;
    ignoreChannel: boolean;
    ignoreVelCh: boolean;
    timingTol: boolean;
    noteSet: boolean;
  };
  channelHistogramPlay: Record<string, number>;
  channelHistogramTryIt: Record<string, number>;
  noteOffDiffCount: number;
  paramsMismatchCount: number;
  paramsFirstMismatches: ParamDiffField[];
  failReason?: string;
}

function histChannels(events: NoteOnEvent[]): Record<string, number> {
  const h: Record<string, number> = {};
  for (const e of events) {
    const k = String(e.channel);
    h[k] = (h[k] ?? 0) + 1;
  }
  return h;
}

function fuzzyMatch(
  a: NoteOnEvent[],
  b: NoteOnEvent[],
  key: (e: NoteOnEvent) => string,
): boolean {
  const seta = new Set(a.map(key));
  const setb = new Set(b.map(key));
  if (seta.size !== setb.size) return false;
  for (const k of seta) if (!setb.has(k)) return false;
  return true;
}

// Playwright restarts the worker after a failed test by default (workerless
// isolation) so a module-level array wouldn't survive across tests. We write
// each record to a separate file under test-results/equivalence-per/ inside
// the test body; a single afterAll pass merges them into equivalence-summary.
// json at the end. Robust to any worker restart / re-run behaviour.
const PER_DIR = path.resolve(process.cwd(), 'test-results', 'equivalence-per');

function writePerChapter(record: ChapterEquivalence): void {
  fs.mkdirSync(PER_DIR, { recursive: true });
  fs.writeFileSync(
    path.join(PER_DIR, `${record.slug}.json`),
    JSON.stringify(record, null, 2),
  );
}

test.afterAll(() => {
  const outDir = path.resolve(process.cwd(), 'test-results');
  fs.mkdirSync(outDir, { recursive: true });
  const chapters: ChapterEquivalence[] = [];
  if (fs.existsSync(PER_DIR)) {
    for (const file of fs.readdirSync(PER_DIR).sort()) {
      if (!file.endsWith('.json')) continue;
      chapters.push(
        JSON.parse(fs.readFileSync(path.join(PER_DIR, file), 'utf8')),
      );
    }
  }
  const outPath = path.join(outDir, 'equivalence-summary.json');
  fs.writeFileSync(outPath, JSON.stringify({ chapters }, null, 2));
  // eslint-disable-next-line no-console
  console.log(
    `[equivalence] wrote ${outPath} (${chapters.length} chapters, ${chapters.filter((r) => r.pass).length} pass, ${chapters.filter((r) => !r.pass).length} fail)`,
  );
});

// -------------------- tests --------------------

test.describe('Play ↔ Try It equivalence — every website preview card', () => {
  for (const chapter of CHAPTERS) {
    test(`[${chapter.preset}] Play and Try It produce equivalent MIDI + params`, async ({
      page,
    }) => {
      const record: ChapterEquivalence = {
        chapter: chapter.preset,
        slug: chapter.slug,
        pass: false,
        smfByteIdentical: false,
        playNoteOnCount: 0,
        tryItNoteOnCount: 0,
        noteOnDiff: {
          onlyInPlayCount: 0,
          onlyInTryItCount: 0,
          sampleOnlyInPlay: [],
          sampleOnlyInTryIt: [],
        },
        fuzzy: {
          exact: false,
          ignoreChannel: false,
          ignoreVelCh: false,
          timingTol: false,
          noteSet: false,
        },
        channelHistogramPlay: {},
        channelHistogramTryIt: {},
        noteOffDiffCount: 0,
        paramsMismatchCount: 0,
        paramsFirstMismatches: [],
      };
      const persist = () => writePerChapter(record);

      try {
        await page.addInitScript(DOWNLOAD_SHIM);
        await page.goto(`${chapter.path}?dump=1&dumpBeats=${DUMP_BEATS}`);

        // ---------- PLAY SURFACE ----------
        const card = page.locator(
          `.poly-preview[data-poly-preset="${chapter.preset}"]`,
        );
        await expect(card, `card missing on ${chapter.path}`).toBeVisible();
        const playBtn = card.locator('.poly-preview-play');
        await expect(playBtn).toBeEnabled();
        await playBtn.click();

        const playCaptures = await waitForParentCaptures(
          page,
          `dump-play-${chapter.slug}-`,
          2,
        );
        const playMidi = b64ToBytes(pickCapture(playCaptures, '.mid').base64);
        const playParams = jsonFromCapture(
          pickCapture(playCaptures, '.params.json'),
        ) as EngineDumpParams;

        // Stop the card so audio doesn't keep running during Try It boot.
        await playBtn.click();
        await expect(card).toHaveAttribute('data-state', 'stopped');

        // ---------- TRY IT SURFACE ----------
        const tryBtn = card.locator('.poly-preview-btn');
        await expect(tryBtn).toBeEnabled();
        await tryBtn.click();

        const frameLocator = page.frameLocator('iframe.poly-modal-frame');
        const iframePlayBtn = frameLocator.locator('#play');
        await expect(iframePlayBtn).toBeVisible({ timeout: 25_000 });

        // Wait for the WASM host to boot AND applyParamsToHost to land the
        // right preset. Otherwise fireDumpTryIt() would dump whatever
        // currentPresetIndex started at (9 = Deep House) instead of ours.
        await page.waitForFunction(
          (expected) => {
            const frames = Array.from(
              document.querySelectorAll('iframe.poly-modal-frame'),
            );
            const normalize = (s: string) =>
              s.toLowerCase().replace(/[-_]/g, ' ');
            for (const f of frames) {
              const w = (f as HTMLIFrameElement).contentWindow as
                | (Window & {
                    PolyWasmHost?: {
                      getState?: () => { preset?: string };
                    };
                  })
                | null;
              if (!w || !w.PolyWasmHost || !w.PolyWasmHost.getState)
                continue;
              const s = w.PolyWasmHost.getState();
              if (
                s &&
                typeof s.preset === 'string' &&
                normalize(s.preset) === normalize(expected)
              )
                return true;
            }
            return false;
          },
          chapter.preset,
          { timeout: 25_000, polling: 250 },
        );

        await iframePlayBtn.click();

        const tryItCaptures = await waitForIframeCaptures(
          page,
          `dump-tryit-${chapter.slug}-`,
          2,
        );
        const tryItMidi = b64ToBytes(pickCapture(tryItCaptures, '.mid').base64);
        const tryItParams = jsonFromCapture(
          pickCapture(tryItCaptures, '.params.json'),
        ) as EngineDumpParams;

        // ---------- SMF COMPARISON ----------
        const bytesIdentical =
          playMidi.length === tryItMidi.length &&
          playMidi.every((b, i) => b === tryItMidi[i]);
        record.smfByteIdentical = bytesIdentical;

        const playParsed = parseSmf(playMidi);
        const tryItParsed = parseSmf(tryItMidi);
        record.playNoteOnCount = playParsed.noteOns.length;
        record.tryItNoteOnCount = tryItParsed.noteOns.length;

        const noteOnDiff = diffNoteOns(playParsed.noteOns, tryItParsed.noteOns);
        record.noteOnDiff = {
          onlyInPlayCount: noteOnDiff.onlyInPlay.length,
          onlyInTryItCount: noteOnDiff.onlyInTryIt.length,
          sampleOnlyInPlay: noteOnDiff.onlyInPlay.slice(0, 8),
          sampleOnlyInTryIt: noteOnDiff.onlyInTryIt.slice(0, 8),
        };

        record.channelHistogramPlay = histChannels(playParsed.noteOns);
        record.channelHistogramTryIt = histChannels(tryItParsed.noteOns);

        record.fuzzy.exact = fuzzyMatch(
          playParsed.noteOns,
          tryItParsed.noteOns,
          (e) => `${e.tick}|${e.channel}|${e.note}|${e.velocity}`,
        );
        record.fuzzy.ignoreChannel = fuzzyMatch(
          playParsed.noteOns,
          tryItParsed.noteOns,
          (e) => `${e.tick}|${e.note}|${e.velocity}`,
        );
        record.fuzzy.ignoreVelCh = fuzzyMatch(
          playParsed.noteOns,
          tryItParsed.noteOns,
          (e) => `${e.tick}|${e.note}`,
        );
        // 30-tick timing bucket (~62.5 ms at 120 BPM 480 ppq). Wide enough
        // to absorb the engine's default humanize/swing shape, tight enough
        // to catch actual note-position shifts.
        record.fuzzy.timingTol = fuzzyMatch(
          playParsed.noteOns,
          tryItParsed.noteOns,
          (e) => `${Math.round(e.tick / 30)}|${e.note}`,
        );
        const notesA = new Set(playParsed.noteOns.map((e) => e.note));
        const notesB = new Set(tryItParsed.noteOns.map((e) => e.note));
        record.fuzzy.noteSet =
          notesA.size === notesB.size &&
          [...notesA].every((n) => notesB.has(n));

        const playOffKey = (e: NoteOffEvent) =>
          `${e.tick}|${e.channel}|${e.note}`;
        const playOffs = new Set(playParsed.noteOffs.map(playOffKey));
        const tryItOffs = new Set(tryItParsed.noteOffs.map(playOffKey));
        let offDiff = 0;
        for (const k of playOffs) if (!tryItOffs.has(k)) offDiff++;
        for (const k of tryItOffs) if (!playOffs.has(k)) offDiff++;
        record.noteOffDiffCount = offDiff;

        // ---------- PARAMS COMPARISON ----------
        const paramDiffs = diffParams(playParams, tryItParams);
        record.paramsMismatchCount = paramDiffs.length;
        record.paramsFirstMismatches = paramDiffs.slice(0, 12);

        // ---------- ASSERTIONS ----------
        // Params first: if params disagree, note-events disagreeing is a
        // consequence, not the root cause. Report both so the summary is
        // usable, but fail the test on the params leak that's the actionable
        // fix.
        expect(
          paramDiffs,
          `params mismatch (${paramDiffs.length} field(s)): ${paramDiffs
            .slice(0, 6)
            .map(
              (d) =>
                `${d.path}: play=${JSON.stringify(d.play)} tryIt=${JSON.stringify(d.tryIt)}`,
            )
            .join('; ')}`,
        ).toHaveLength(0);

        // Note-on events must match set-wise. Ordering within same-tick
        // events is handled by the writer's sort so the ticks are the true
        // comparison unit.
        expect(
          noteOnDiff.onlyInPlay,
          `note-on events only in Play (${noteOnDiff.onlyInPlay.length}): first few ${JSON.stringify(noteOnDiff.onlyInPlay.slice(0, 4))}`,
        ).toHaveLength(0);
        expect(
          noteOnDiff.onlyInTryIt,
          `note-on events only in Try It (${noteOnDiff.onlyInTryIt.length}): first few ${JSON.stringify(noteOnDiff.onlyInTryIt.slice(0, 4))}`,
        ).toHaveLength(0);

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
