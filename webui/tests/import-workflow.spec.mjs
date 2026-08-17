import { test, expect } from '@playwright/test';
import { pageUrl, setupWithActionLog, getActions } from './test-helpers.mjs';

// M035 S03 T04: the full groove-import workflow on the S02 drop->populate baseline.
//
// Dropping a .mid onto a lane populates it (S02) AND raises a transient per-lane
// Accept/Revert confirm bar. Clicking Revert fires the revertImport {lane} bridge
// action, whose state push restores the lane exactly to its pre-import parameters
// and dismisses the bar. Clicking Accept keeps the fitted lane and just dismisses
// the bar. A rejected drop (non-MIDI) raises no bar and arms no snapshot, so there
// is nothing to revert into (must-have 6). This spec exercises the real DOM drop +
// button gestures against the mock host, which mirrors the engine snapshot/revert
// helpers all three surfaces share (D039).

// ---- inline SMF fixture builder (git-tracked code, no binary fixture) --------
// Mirrors midi-drop.spec.mjs. Default step set is E(5,8) = onset steps 0,2,4,5,7,
// which the reverse-Euclid fitter recovers exactly (steps 8, hits 5, rotation 0).
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

async function dropFileOnLane(page, lane, bytes, name = 'loop.mid') {
  await page.evaluate(({ lane, bytes, name }) => {
    const strip = document.querySelector(`.strip[data-lane="${lane}"]`);
    const file = new File([new Uint8Array(bytes)], name, { type: 'audio/midi' });
    const dt = new DataTransfer();
    dt.items.add(file);
    strip.dispatchEvent(new DragEvent('drop', { bubbles: true, cancelable: true, dataTransfer: dt }));
  }, { lane, bytes, name });
}

function readLane(page, lane) {
  return page.evaluate((li) => {
    const l = window.PolyMockHost.getState().lanes[li];
    return { steps: l.steps, hits: l.hits, rot: l.rot, subdivision: l.subdivision, timeline: l.timeline, pattern: l.pattern.slice() };
  }, lane);
}

const confirmBar = (page, lane) => page.locator(`.import-confirm[data-lane="${lane}"]`);

test.describe('M035 S03 T04 — import workflow: drop -> confirm -> revert/accept', () => {
  test('drop raises a per-lane confirm bar; Revert restores the pre-import lane', async ({ page }) => {
    await setupWithActionLog(page);

    // Lane 1 boots as a 4-step Euclid; capture its exact pre-import shape.
    const before = await readLane(page, 1);
    expect(before.steps).toBe(4);

    // Drop the E(5,8) loop: the lane re-fits and the confirm bar appears.
    await dropFileOnLane(page, 1, buildSMF());
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 8);
    await expect(confirmBar(page, 1)).toBeVisible();
    await expect(confirmBar(page, 1).locator('.import-revert')).toBeVisible();
    await expect(confirmBar(page, 1).locator('.import-accept')).toBeVisible();

    const imported = await readLane(page, 1);
    expect(imported.steps).toBe(8);
    expect(imported.hits).toBe(5);

    // Click Revert: fires revertImport {lane:1}, the state push restores the lane
    // and the bar is dismissed.
    await confirmBar(page, 1).locator('.import-revert').click();
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 4);

    const reverted = await readLane(page, 1);
    expect(reverted).toEqual(before); // byte-for-byte restore of every field
    await expect(confirmBar(page, 1)).toHaveCount(0);

    // Exactly one revertImport action fired, {lane}-only.
    const actions = await getActions(page);
    const reverts = actions.filter((a) => a.name === 'revertImport');
    expect(reverts).toHaveLength(1);
    expect(reverts[0].payload).toEqual({ lane: 1 });
  });

  test('Accept keeps the imported lane and dismisses the bar', async ({ page }) => {
    await setupWithActionLog(page);

    await dropFileOnLane(page, 1, buildSMF());
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 8);
    await expect(confirmBar(page, 1)).toBeVisible();

    // Accept: no state push, the fitted lane stays in place, the bar is removed.
    await confirmBar(page, 1).locator('.import-accept').click();
    await expect(confirmBar(page, 1)).toHaveCount(0);

    const kept = await readLane(page, 1);
    expect(kept.steps).toBe(8);
    expect(kept.hits).toBe(5);

    // Accept is a UI-only dismissal — it fires no revertImport.
    const actions = await getActions(page);
    expect(actions.filter((a) => a.name === 'revertImport')).toHaveLength(0);
  });

  test('after import the granular Euclid editor still adjusts the applied lane', async ({ page }) => {
    await setupWithActionLog(page);

    await dropFileOnLane(page, 1, buildSMF());
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 8);
    await expect(confirmBar(page, 1)).toBeVisible();

    // must-have 4: existing editors adjust the imported lane. Drive setEuclid the
    // way the granular editor does and confirm the applied lane re-fits.
    await page.evaluate(() => window.PolyMockHost.action('setEuclid', { lane: 1, steps: 12, hits: 7, rotation: 1 }));
    await page.waitForFunction(() => window.PolyMockHost.getState().lanes[1].steps === 12);
    const adjusted = await readLane(page, 1);
    expect(adjusted.steps).toBe(12);
    expect(adjusted.hits).toBe(7);
    expect(adjusted.rot).toBe(1);
  });

  test('a rejected non-MIDI drop raises no confirm bar and arms no revert', async ({ page }) => {
    await setupWithActionLog(page);

    const before = await readLane(page, 1);
    const junk = Array.from('this is definitely not a midi file', (c) => c.charCodeAt(0) & 0xff);
    await dropFileOnLane(page, 1, junk, 'notes.txt');
    await page.waitForTimeout(200); // let the async read + rejected action settle

    // must-have 6: no bar, no lane change, no snapshot to revert into.
    await expect(confirmBar(page, 1)).toHaveCount(0);
    const after = await readLane(page, 1);
    expect(after).toEqual(before);
    await expect(page.locator('.strip')).toHaveCount(5);
  });
});
