import { test, expect } from '@playwright/test';
import { pageUrl, setupWithActionLog, getActions } from './test-helpers.mjs';

// M035 S02 T03: lane-strip file-drop UX + the fitMidi bridge action.
//
// Dropping a .mid onto a lane strip reads the file bytes and fires
// host.action('fitMidi', {lane, bytes}); the mock host mirrors the C++ import
// path (parseSMF + fitEuclidean + applyFitToLane) so the lane populates from the
// reverse-Euclid fit of the dropped rhythm. A non-MIDI / unreadable drop is
// logged and ignored — the lane is untouched and the UI never crashes
// (unknown-message-drop contract, bridge-schema.md).

// ---- inline SMF fixture builder (git-tracked code, no binary fixture) --------
// Build a single-track (format 0) Standard MIDI File carrying note-ons at the
// given eighth-note steps of a one-bar (8-step) loop. The default step set is
// E(5,8) = [1,0,1,0,1,1,0,1] -> onset steps 0,2,4,5,7, a clean Euclidean rhythm
// the fitter recovers exactly (steps 8, hits 5, rotation 0, zero mismatch).
function vlq(n) {
  const out = [n & 0x7f];
  n >>>= 7;
  while (n > 0) { out.unshift((n & 0x7f) | 0x80); n >>>= 7; }
  return out;
}
function buildSMF({ steps = [0, 2, 4, 5, 7], tpqn = 480, eighth = 240, loopTicks = 1920, note = 36 } = {}) {
  const events = [];
  for (const s of steps) {
    const on = s * eighth;
    events.push({ tick: on, order: 1, data: [0x90, note, 100] });
    events.push({ tick: on + Math.floor(eighth / 2), order: 0, data: [0x80, note, 0] });
  }
  events.push({ tick: loopTicks, order: 2, data: [0xff, 0x2f, 0x00] }); // end-of-track
  events.sort((a, b) => a.tick - b.tick || a.order - b.order);
  const track = [];
  let last = 0;
  for (const e of events) {
    track.push(...vlq(e.tick - last), ...e.data);
    last = e.tick;
  }
  const be32 = (n) => [(n >>> 24) & 0xff, (n >>> 16) & 0xff, (n >>> 8) & 0xff, n & 0xff];
  const be16 = (n) => [(n >> 8) & 0xff, n & 0xff];
  const header = [0x4d, 0x54, 0x68, 0x64, ...be32(6), ...be16(0), ...be16(1), ...be16(tpqn)]; // MThd
  const chunk = [0x4d, 0x54, 0x72, 0x6b, ...be32(track.length), ...track];                    // MTrk
  return [...header, ...chunk];
}

// Dispatch a real file drop on a lane strip (what the browser fires when the
// user drops a .mid): a DragEvent carrying a DataTransfer whose files[] holds
// the built SMF. The ui.js handler reads file.arrayBuffer() asynchronously, so
// callers must waitForFunction on the resulting state change.
async function dropFileOnLane(page, lane, bytes, name = 'loop.mid') {
  await page.evaluate(({ lane, bytes, name }) => {
    const strip = document.querySelector(`.strip[data-lane="${lane}"]`);
    const file = new File([new Uint8Array(bytes)], name, { type: 'audio/midi' });
    const dt = new DataTransfer();
    dt.items.add(file);
    strip.dispatchEvent(new DragEvent('drop', { bubbles: true, cancelable: true, dataTransfer: dt }));
  }, { lane, bytes, name });
}

test.describe('M035 S02 T03 — lane-strip MIDI drop', () => {
  test('dropping a Euclidean .mid populates the target lane from the fit', async ({ page }) => {
    await setupWithActionLog(page);
    // Lane 1 (Afrobeat "Kick") boots as a 4-step Euclid; the dropped E(5,8) loop
    // must re-fit it to 8 steps / 5 hits / rotation 0.
    const before = await page.evaluate(() => window.PolyMockHost.getState().lanes[1].steps);
    expect(before).toBe(4);

    const smf = buildSMF();
    await dropFileOnLane(page, 1, smf);
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 8);

    const lane = await page.evaluate(() => {
      const l = window.PolyMockHost.getState().lanes[1];
      return { steps: l.steps, hits: l.hits, rot: l.rot, subdivision: l.subdivision, stepLen: l.stepLen, timeline: l.timeline, pattern: l.pattern.slice() };
    });
    expect(lane.steps).toBe(8);
    expect(lane.hits).toBe(5);
    expect(lane.rot).toBe(0);
    expect(lane.subdivision).toBe(8);
    expect(lane.stepLen).toBe(1);
    expect(lane.timeline).toBe(false); // pure Euclid -> no residual timeline
    expect(lane.pattern).toEqual([1, 0, 1, 0, 1, 1, 0, 1]);

    // The drop fired exactly one schema-shaped fitMidi action carrying the bytes.
    const actions = await getActions(page);
    const fits = actions.filter((a) => a.name === 'fitMidi');
    expect(fits).toHaveLength(1);
    expect(fits[0].payload.lane).toBe(1);
    expect(Array.isArray(fits[0].payload.bytes)).toBe(true);
    expect(fits[0].payload.bytes.length).toBe(smf.length);

    // The DOM reflects the re-fit: the ladder now shows 8 step buttons.
    await expect(page.locator('.strip[data-lane="1"] .ladder button')).toHaveCount(8);
  });

  test('a non-MIDI drop is rejected without crashing and leaves the lane untouched', async ({ page }) => {
    await page.goto(pageUrl);
    const before = await page.evaluate(() => {
      const l = window.PolyMockHost.getState().lanes[1];
      return { steps: l.steps, hits: l.hits, pattern: l.pattern.slice() };
    });

    // Drop plain text bytes (no MThd header): parseSMF rejects it.
    const junk = Array.from('this is definitely not a midi file', (c) => c.charCodeAt(0) & 0xff);
    await dropFileOnLane(page, 1, junk, 'notes.txt');
    // Give the async arrayBuffer read + rejected action time to run.
    await page.waitForTimeout(150);

    const after = await page.evaluate(() => {
      const l = window.PolyMockHost.getState().lanes[1];
      return { steps: l.steps, hits: l.hits, pattern: l.pattern.slice() };
    });
    expect(after).toEqual(before);
    // The UI is still alive and interactive.
    await expect(page.locator('.strip')).toHaveCount(5);
  });

  test('an empty / structurally-truncated MIDI drop is rejected safely', async ({ page }) => {
    await page.goto(pageUrl);
    const before = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].steps);
    // A bare 'MThd' with a truncated body — valid magic, unusable structure.
    await dropFileOnLane(page, 0, [0x4d, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 0, 0, 1, 1, 0xe0], 'trunc.mid');
    await page.waitForTimeout(150);
    const after = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].steps);
    expect(after).toBe(before);
    await expect(page.locator('.strip')).toHaveCount(5);
  });

  test('dragover with files marks the strip a hot drop target; dragleave clears it', async ({ page }) => {
    await page.goto(pageUrl);
    const hot = await page.evaluate(() => {
      const strip = document.querySelector('.strip[data-lane="2"]');
      const dt = new DataTransfer();
      dt.items.add(new File([new Uint8Array([0x4d, 0x54, 0x68, 0x64])], 'x.mid'));
      strip.dispatchEvent(new DragEvent('dragover', { bubbles: true, cancelable: true, dataTransfer: dt }));
      const on = strip.classList.contains('drop-hot');
      strip.dispatchEvent(new DragEvent('dragleave', { bubbles: true, dataTransfer: dt }));
      return { on, off: strip.classList.contains('drop-hot') };
    });
    expect(hot.on).toBe(true);
    expect(hot.off).toBe(false);
  });
});
