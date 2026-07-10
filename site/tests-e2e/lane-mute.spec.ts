import { test, expect } from '@playwright/test';

// M043 S14 T03 — lane mute equivalence on Play card and Try It modal.
//
// The engine render loop skips inactive lanes (engine/src/engine.cpp:359). This
// spec proves the UI mute state actually reaches the engine on both surfaces:
//
//   Play card:  a chip click updates the per-card muted Set, PolyPreviewCard
//               forwards Array.from(mutedSet) into dumpPresetAsSmf, and
//               renderEngineEvents calls _poly_edit_lane_int(Active, 0) on
//               the scratch context before rendering.
//
//   Try It:     the mute button emits host.edit("lane.<i>.active", 0). The
//               wasm-host handler now writes to BOTH engineCtx AND playbackCtx
//               so mid-play mutes stop audio immediately, and fireDumpTryIt
//               follows _poly_load_preset with _poly_copy_scenes(scratch,
//               engineCtx) so the dump carries the same UI-driven lane state.
//
// Target: Minimal Techno (chapter 09-electronic). All four lanes have
// distinct MIDI notes (36 kick, 42 shimmer, 39 ghost, 38 backbeat), so
// counting note-ons for note 42 is an unambiguous signal that lane 1's
// contribution is present or absent.

const CHAPTER_PATH = '/poly/09-electronic/';
const PRESET = 'Minimal Techno';
const TARGET_LANE_INDEX = 1;
const TARGET_NOTE = 42;
const DUMP_BEATS = 32;
const CAPTURE_TIMEOUT_MS = 30_000;
const IFRAME_URL_PATTERN = /\/poly\/webui\/index\.html\?/;

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

// SMF parser lifted from equivalence.spec.ts — kept local so this spec is
// independently runnable. If the pair drifts, the equivalence test still holds
// its own version of the truth.
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

async function waitForParentCaptures(
  page: import('@playwright/test').Page,
  prefix: string,
  minCount: number,
): Promise<CapturedDownload[]> {
  const handle = await page.waitForFunction(
    ({ prefix, minCount }) => {
      const w = window as unknown as { __dumpCaptures?: CapturedDownload[] };
      const c = w.__dumpCaptures ?? [];
      const filtered = c.filter((cap) => cap.filename.startsWith(prefix));
      return filtered.length >= minCount ? filtered : null;
    },
    { prefix, minCount },
    { timeout: CAPTURE_TIMEOUT_MS, polling: 250 },
  );
  return (await handle.jsonValue()) as CapturedDownload[];
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
  if (!match) throw new Error(`no .mid capture — got [${caps.map((c) => c.filename).join(', ')}]`);
  return b64ToBytes(match.base64);
}

test.describe('Lane mute — dump reflects UI state on both surfaces', () => {
  test('Play card: baseline dump contains note-ons for the target lane', async ({ page }) => {
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(`${CHAPTER_PATH}?dump=1&dumpBeats=${DUMP_BEATS}`);
    const card = page.locator(`.poly-preview[data-poly-preset="${PRESET}"]`);
    await expect(card).toBeVisible();
    const playBtn = card.locator('.poly-preview-play');
    await expect(playBtn).toBeEnabled();
    await playBtn.click();
    const caps = await waitForParentCaptures(page, `dump-play-minimal-techno-`, 2);
    const notes = parseNoteOns(pickMidi(caps));
    const targetNotes = notes.filter((n) => n.note === TARGET_NOTE);
    expect(targetNotes.length, `baseline should contain note-ons for lane ${TARGET_LANE_INDEX} (note ${TARGET_NOTE})`).toBeGreaterThan(0);
  });

  test('Play card: muting lane suppresses its notes, unmuting restores them', async ({ page }) => {
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(`${CHAPTER_PATH}?dump=1&dumpBeats=${DUMP_BEATS}`);
    const card = page.locator(`.poly-preview[data-poly-preset="${PRESET}"]`);
    await expect(card).toBeVisible();
    const chip = card.locator(`.poly-lane-chip[data-lane-index="${TARGET_LANE_INDEX}"]`);
    await expect(chip).toBeVisible();
    await expect(chip).toHaveAttribute('aria-pressed', 'false');

    // Mute → Play → dump. Card has no dumpFiredThisPage guard, so subsequent
    // Play clicks produce additional dumps we can distinguish by array index.
    await chip.click();
    await expect(chip).toHaveAttribute('aria-pressed', 'true');
    const playBtn = card.locator('.poly-preview-play');
    await playBtn.click();
    const capsAfterMute = await waitForParentCaptures(page, `dump-play-minimal-techno-`, 2);
    const mutedNotes = parseNoteOns(pickMidi(capsAfterMute));
    expect(
      mutedNotes.filter((n) => n.note === TARGET_NOTE).length,
      `after mute, dump must contain zero note-ons for note ${TARGET_NOTE}`,
    ).toBe(0);
    // Sanity: dump must contain SOMETHING (other lanes still fire) so a bug
    // that produces empty dumps doesn't false-pass here.
    expect(mutedNotes.length, 'muted dump should still contain other lanes').toBeGreaterThan(0);

    // Stop, unmute, Play → new dump appended to the captures array.
    await playBtn.click();
    await expect(card).toHaveAttribute('data-state', 'stopped');
    await chip.click();
    await expect(chip).toHaveAttribute('aria-pressed', 'false');
    await playBtn.click();
    const capsAfterUnmute = await waitForParentCaptures(page, `dump-play-minimal-techno-`, 4);
    // The last 2 captures are the unmute run (2 files per Play: .mid + .params.json).
    const unmuteCaps = capsAfterUnmute.slice(-2);
    const restoredNotes = parseNoteOns(pickMidi(unmuteCaps));
    expect(
      restoredNotes.filter((n) => n.note === TARGET_NOTE).length,
      `after unmute, dump must contain note-ons for note ${TARGET_NOTE} again`,
    ).toBeGreaterThan(0);
  });

  test('Try It modal: baseline dump contains note-ons for the target lane', async ({ page }) => {
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(`${CHAPTER_PATH}?dump=1&dumpBeats=${DUMP_BEATS}`);
    const card = page.locator(`.poly-preview[data-poly-preset="${PRESET}"]`);
    await card.locator('.poly-preview-btn').click();
    const frame = page.frameLocator('iframe.poly-modal-frame');
    const iframePlay = frame.locator('#play');
    await expect(iframePlay).toBeVisible({ timeout: 25_000 });
    await page.waitForFunction(
      (expected) => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        const normalize = (s: string) => s.toLowerCase().replace(/[-_]/g, ' ');
        for (const f of frames) {
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { PolyWasmHost?: { getState?: () => { preset?: string } } })
            | null;
          if (!w || !w.PolyWasmHost || !w.PolyWasmHost.getState) continue;
          const s = w.PolyWasmHost.getState();
          if (s && typeof s.preset === 'string' && normalize(s.preset) === normalize(expected)) return true;
        }
        return false;
      },
      PRESET,
      { timeout: 25_000, polling: 250 },
    );
    await iframePlay.click();
    const caps = await waitForIframeCaptures(page, `dump-tryit-minimal-techno-`, 2);
    const notes = parseNoteOns(pickMidi(caps));
    expect(
      notes.filter((n) => n.note === TARGET_NOTE).length,
      `baseline Try It dump should contain note-ons for note ${TARGET_NOTE}`,
    ).toBeGreaterThan(0);
  });

  test('Try It modal: muting lane before Play suppresses its notes in the dump', async ({ page }) => {
    // Capture the wasm-host mute log so a regression reports the observable
    // signal (or its absence) in the failure message rather than making a
    // reader instrument the test themselves.
    const consoleLines: string[] = [];
    page.on('console', (msg) => {
      const t = msg.text();
      if (t.includes('lane') || t.includes('active')) {
        consoleLines.push(`[${msg.type()}] ${t}`);
      }
    });
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(`${CHAPTER_PATH}?dump=1&dumpBeats=${DUMP_BEATS}`);
    const card = page.locator(`.poly-preview[data-poly-preset="${PRESET}"]`);
    await card.locator('.poly-preview-btn').click();
    const frame = page.frameLocator('iframe.poly-modal-frame');
    const iframePlay = frame.locator('#play');
    await expect(iframePlay).toBeVisible({ timeout: 25_000 });
    await page.waitForFunction(
      (expected) => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        const normalize = (s: string) => s.toLowerCase().replace(/[-_]/g, ' ');
        for (const f of frames) {
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { PolyWasmHost?: { getState?: () => { preset?: string } } })
            | null;
          if (!w || !w.PolyWasmHost || !w.PolyWasmHost.getState) continue;
          const s = w.PolyWasmHost.getState();
          if (s && typeof s.preset === 'string' && normalize(s.preset) === normalize(expected)) return true;
        }
        return false;
      },
      PRESET,
      { timeout: 25_000, polling: 250 },
    );

    // Click the desk-mode mute button for the target lane before Play. The
    // handler emits host.edit("lane.<i>.active", 0) which now writes to both
    // engineCtx and playbackCtx; fireDumpTryIt then reads scene state from
    // engineCtx via _poly_copy_scenes so the mute lands in the dump.
    const muteBtn = frame.locator(`.strip[data-lane="${TARGET_LANE_INDEX}"] [data-mute]`);
    await expect(muteBtn).toBeVisible();
    await muteBtn.click();

    // Confirm the mute reached engineCtx before we click Play. Without this,
    // a race between the click event dispatch and Playwright's next command
    // could leave engineCtx untouched.
    await page.waitForFunction(
      ({ laneIdx }) => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        for (const f of frames) {
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { PolyWasmHost?: { getState?: () => { lanes: Array<{ active: boolean }> } } })
            | null;
          const s = w?.PolyWasmHost?.getState?.();
          if (s && s.lanes && s.lanes[laneIdx] && s.lanes[laneIdx].active === false) return true;
        }
        return false;
      },
      { laneIdx: TARGET_LANE_INDEX },
      { timeout: 5000, polling: 100 },
    );

    await iframePlay.click();
    const caps = await waitForIframeCaptures(page, `dump-tryit-minimal-techno-`, 2);
    const notes = parseNoteOns(pickMidi(caps));
    const targetCount = notes.filter((n) => n.note === TARGET_NOTE).length;
    expect(
      targetCount,
      `after mute, Try It dump must contain zero note-ons for note ${TARGET_NOTE} — console lines: ${consoleLines.join(' || ') || '(none)'}`,
    ).toBe(0);
    expect(notes.length, 'muted Try It dump should still contain other lanes').toBeGreaterThan(0);
  });
});
