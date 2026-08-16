import { test, expect } from '@playwright/test';
import {
  setupWithActionLog, getActions, clearActions, getEdits, clearEdits, expandStrip,
} from './test-helpers.mjs';

// M034 S03 T03 — WebUI seed controls. The engine (T01) and v18 state
// serialization (T02) carry a per-lane laneSeed + seedLocked so a locked lane's
// output is byte-identical across a global reroll. This slice surfaces three
// affordances in the WebUI:
//   - a dice button that rolls a fresh global seed through the existing 'seed'
//     edit path (mock host maps norm -> round(norm*999999), mirroring the
//     native kSeed registry scaling),
//   - back/forward buttons over a ring buffer of visited seeds (a navigation
//     restores an existing entry; it never branches or duplicates history), and
//   - a per-lane "Lock Seed" toggle in the Advanced pane emitting
//     host.edit('lane.N.seedLock', 0|1) — bridge_params.h resolves it to
//     laneCoreParam(N, kCoreSeedLock) and the processor captures the current
//     global seed into the lane's laneSeed on the false->true edge.
// These mock-host assertions exercise the same bridge contract the plugin host
// honours; the byte-identical invariance itself is proven engine-side
// (seed_lock_tests.cpp).

const seedVal = (page) => page.locator('#seedVal');
const seedDice = (page) => page.locator('#seedDice');
const seedBack = (page) => page.locator('#seedBack');
const seedFwd = (page) => page.locator('#seedFwd');
const seedHistPos = (page) => page.locator('#seedHistPos');

async function openAdvPane(page, lane = 0) {
  await expandStrip(page, lane);
  await page.click(`.strip[data-lane="${lane}"] [data-tab="adv"]`);
  await page.waitForSelector(`.strip[data-lane="${lane}"] [data-pane="adv"].on`);
}

function lockChip(page, lane = 0) {
  return page.locator(`.strip[data-lane="${lane}"] [data-pane="adv"] [data-seedlock]`);
}

test.describe('M034 S03 T03 — WebUI seed controls', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('chrome exposes a dice + back/forward seed nav and a numeric readout', async ({ page }) => {
    await expect(seedDice(page)).toHaveCount(1);
    await expect(seedBack(page)).toHaveCount(1);
    await expect(seedFwd(page)).toHaveCount(1);
    // The initial state seeds the history with one visit — position "1/1", and
    // both navigation buttons are at a boundary (disabled).
    await expect(seedHistPos(page)).toHaveText('1/1');
    await expect(seedBack(page)).toBeDisabled();
    await expect(seedFwd(page)).toBeDisabled();
  });

  test('clicking the dice emits a seed edit and the readout updates in range', async ({ page }) => {
    const before = await seedVal(page).textContent();
    await clearEdits(page);
    await clearActions(page);
    await seedDice(page).click();

    const edits = await getEdits(page);
    const seedEdits = edits.filter((e) => e.paramId === 'seed');
    expect(seedEdits.length).toBeGreaterThan(0);
    // Driven as a begin/perform/end gesture like every other host edit.
    expect(seedEdits[0].gesture).toBe('begin');
    expect(seedEdits[seedEdits.length - 1].gesture).toBe('end');

    // Only a 'seed' edit — the dice is a global roll, never a lane edit or action.
    expect(edits.every((e) => e.paramId === 'seed')).toBe(true);
    expect(await getActions(page)).toEqual([]);

    // The readout moved to the round-tripped seed, in [0, 999999].
    const after = await seedVal(page).textContent();
    expect(after).not.toBe(before);
    const n = Number(after);
    expect(Number.isInteger(n)).toBe(true);
    expect(n).toBeGreaterThanOrEqual(0);
    expect(n).toBeLessThanOrEqual(999999);
    const stateSeed = await page.evaluate(() => window.PolyMockHost.getState().seed);
    expect(stateSeed).toBe(n);
  });

  test('back/forward restore visited seeds without branching history', async ({ page }) => {
    const initial = await seedVal(page).textContent();
    await seedDice(page).click();
    const second = await seedVal(page).textContent();
    await seedDice(page).click();
    const third = await seedVal(page).textContent();

    // Three distinct visits recorded.
    expect(new Set([initial, second, third]).size).toBe(3);
    await expect(seedHistPos(page)).toHaveText('3/3');
    await expect(seedFwd(page)).toBeDisabled();

    // Back once restores the second seed via a 'seed' edit; position moves but
    // the history length (denominator) never grows on navigation.
    await clearEdits(page);
    await seedBack(page).click();
    let edits = await getEdits(page);
    expect(edits.filter((e) => e.paramId === 'seed').length).toBeGreaterThan(0);
    await expect(seedVal(page)).toHaveText(second);
    await expect(seedHistPos(page)).toHaveText('2/3');

    // Back again reaches the initial seed; back is now at the boundary.
    await seedBack(page).click();
    await expect(seedVal(page)).toHaveText(initial);
    await expect(seedHistPos(page)).toHaveText('1/3');
    await expect(seedBack(page)).toBeDisabled();

    // Forward twice walks back out to the newest seed — still exactly 3 entries.
    await seedFwd(page).click();
    await expect(seedVal(page)).toHaveText(second);
    await expect(seedHistPos(page)).toHaveText('2/3');
    await seedFwd(page).click();
    await expect(seedVal(page)).toHaveText(third);
    await expect(seedHistPos(page)).toHaveText('3/3');
    await expect(seedFwd(page)).toBeDisabled();
  });

  test('rolling from the middle of history drops the stale forward branch', async ({ page }) => {
    await seedDice(page).click();
    await seedDice(page).click(); // history 3/3
    await seedBack(page).click(); // now at 2/3
    await expect(seedHistPos(page)).toHaveText('2/3');

    // A fresh roll from the middle truncates the forward entry and appends —
    // length becomes 3 again (was 3, dropped 1 forward, added 1), at the tail.
    await seedDice(page).click();
    await expect(seedHistPos(page)).toHaveText('3/3');
    await expect(seedFwd(page)).toBeDisabled();
  });

  test('Advanced pane exposes a per-lane Lock Seed toggle', async ({ page }) => {
    await openAdvPane(page, 0);
    const adv = page.locator('.strip[data-lane="0"] [data-pane="adv"]');
    await expect(adv).toContainText('Seed');
    await expect(lockChip(page, 0)).toHaveCount(1);
    await expect(lockChip(page, 0)).toHaveText('Lock Seed');
    await expect(lockChip(page, 0)).toHaveAttribute('aria-pressed', 'false');
  });

  test('the Lock Seed toggle emits lane.0.seedLock and round-trips through getState', async ({ page }) => {
    await openAdvPane(page, 0);
    await clearEdits(page);
    await clearActions(page);
    await lockChip(page, 0).click();

    const edits = await getEdits(page);
    const lockEdits = edits.filter((e) => e.paramId === 'lane.0.seedLock');
    expect(lockEdits.length).toBeGreaterThan(0);
    expect(lockEdits[0].gesture).toBe('begin');
    expect(lockEdits[lockEdits.length - 1].gesture).toBe('end');
    // Locking emits value 1; no seed edit, no action.
    expect(lockEdits[lockEdits.length - 1].value).toBe(1);
    expect(edits.find((e) => e.paramId === 'seed')).toBeUndefined();
    expect(await getActions(page)).toEqual([]);

    // Round-trips: the mock host reflects seedLocked=true on lane 0 only.
    const locked = await page.evaluate(() => window.PolyMockHost.getState().lanes.map((l) => l.seedLocked));
    expect(locked[0]).toBe(true);
    expect(locked.slice(1).every((v) => v === false)).toBe(true);

    // The chip re-renders to the locked affordance.
    await expect(lockChip(page, 0)).toHaveText('Locked');
    await expect(lockChip(page, 0)).toHaveAttribute('aria-pressed', 'true');
  });

  test('toggling the Lock Seed chip again emits 0 and unlocks the lane', async ({ page }) => {
    await openAdvPane(page, 0);
    await lockChip(page, 0).click();
    await expect(lockChip(page, 0)).toHaveText('Locked');

    await clearEdits(page);
    await lockChip(page, 0).click();
    const edits = await getEdits(page);
    const lockEdits = edits.filter((e) => e.paramId === 'lane.0.seedLock');
    expect(lockEdits.length).toBeGreaterThan(0);
    expect(lockEdits[lockEdits.length - 1].value).toBe(0);

    const locked = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].seedLocked);
    expect(locked).toBe(false);
    await expect(lockChip(page, 0)).toHaveText('Lock Seed');
  });
});
