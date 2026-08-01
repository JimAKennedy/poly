import { test, expect } from '@playwright/test';
import { pageUrl, editLaneParam } from './test-helpers.mjs';

// M073 S05 T03: the cloth capture timeline (drawCaptureTimeline in ui.js) now
// renders the engine emission stream (host.getLaneEmissions via laneStepKind)
// instead of the drift-prone laneHitAt re-derivation. This spec is the slice's
// regression gate: with an injected per-lane stream it proves an emitted hit
// draws a solid tick, a dropped on-pattern step draws a distinct hollow "hole",
// a ghost renders dimmed, and a macro-resolved add appears off the positional
// grid — and that changing the stream changes the drawn markers. It also proves
// the degradation contract: when getLaneEmissions returns [] (mock default / DAW
// / stopped) the timeline shows the positional pattern only, never a hole, and a
// muted lane (velocity 0, which records no emission) is never drawn as a drop.
//
// The mock host (webui/mock-host.js) exposes _setEmissions(li, stream) to inject
// a per-lane stream of { ppq, step, kind } (kind: base|ghost|add|drop); absent
// injection getLaneEmissions returns [] so the timeline degrades to laneHitAt.
//
// Fixture facts (default preset "Afrobeat 12/8", 4/4):
//   lane 0 "Bell": steps 12, stepLen 1, fixed pattern [1,0,1,0,1,1,0,1,0,1,0,1].
//   So (tick % 12) is the pattern step. Hit steps: 0,2,4,5,7,9,11.
//   tick 0 -> step 0 (hit), tick 2 -> step 2 (hit), tick 3 -> step 3 (NO hit),
//   tick 1 -> step 1 (NO hit; used as the off-grid "add" target).
// The timeline is forced to the bar-anchored view via capture state 3 (complete),
// where every tick is fully woven (full alpha) — a stable pixel surface.

const LANE = 0; // Bell
const DROP_TICK = 0; // step 0 — an on-pattern hit
const EMIT_TICK = 2; // step 2 — an on-pattern hit
const NOHIT_TICK = 1; // step 1 — no positional hit (off-grid add target)
const BG_TICK = 3; // step 3 — no positional hit, pure lane background reference

// Read per-tick "ink energy" off the #loom canvas for a lane band. Energy is the
// mean per-pixel colour distance of a tick's interior box from the lane's own
// background (sampled from bgTick, an empty tick). A solid emitted tick fills the
// box with the lane hue -> high energy; a hollow drop marker paints only thin
// strokes -> low energy; nothing drawn -> ~0. `sig` is a cheap rolling hash so a
// spec can prove two renders differ. Geometry mirrors drawCaptureTimeline:
// ticks = bars * eighthsPerBar (8 in 4/4), tickW = W/ticks, bandH = H/laneCount.
async function measure(page, ticks, { lane = LANE, bgTick = BG_TICK } = {}) {
  return page.evaluate(
    ({ ticks, lane, bgTick }) => {
      const c = document.getElementById('loom');
      const g = c.getContext('2d');
      const W = c.width;
      const H = c.height;
      const laneCount = document.querySelectorAll('#tags .tag').length || 5;
      const bars = (window.__polyClothState && window.__polyClothState.bars) || 8;
      const eighthsPerBar = 8; // beforeEach forces 4/4: round(4 * 8/4) = 8
      const perTick = W / (bars * eighthsPerBar);
      const bandH = H / laneCount;
      const box = (tick) => {
        const x0 = Math.max(0, Math.round(tick * perTick + perTick * 0.2));
        const x1 = Math.min(W, Math.round((tick + 1) * perTick - perTick * 0.2));
        const y0 = Math.round(lane * bandH + bandH * 0.3);
        const y1 = Math.round(lane * bandH + bandH * 0.7);
        return g.getImageData(x0, y0, Math.max(1, x1 - x0), Math.max(1, y1 - y0));
      };
      // Background reference = mean colour of an empty tick's interior.
      const bd = box(bgTick).data;
      let br = 0;
      let bgc = 0;
      let bb = 0;
      let bn = 0;
      for (let i = 0; i < bd.length; i += 4) {
        br += bd[i];
        bgc += bd[i + 1];
        bb += bd[i + 2];
        bn++;
      }
      br /= bn;
      bgc /= bn;
      bb /= bn;
      return ticks.map((tick) => {
        const d = box(tick).data;
        let energy = 0;
        let sig = 0;
        let n = 0;
        for (let i = 0; i < d.length; i += 4) {
          const dr = d[i] - br;
          const dg = d[i + 1] - bgc;
          const db = d[i + 2] - bb;
          energy += Math.sqrt(dr * dr + dg * dg + db * db);
          sig = (sig * 31 + d[i] + d[i + 1] * 7 + d[i + 2] * 13) >>> 0;
          n++;
        }
        return { tick, energy: energy / n, sig };
      });
    },
    { ticks, lane, bgTick },
  );
}

// Wait for at least one fresh drawLoom after a mutation. The mock host pumps
// frames via a continuous rAF loop (mock-host.js pump), and each frame in cloth
// mode calls drawLoom, so three rAFs guarantee a redraw that reads the new
// emission stream / lane state.
async function waitRedraw(page) {
  await page.evaluate(
    () =>
      new Promise((res) => {
        let n = 0;
        const step = () => {
          if (++n >= 3) res();
          else requestAnimationFrame(step);
        };
        requestAnimationFrame(step);
      }),
  );
}

// Inject (or clear) a lane's emission stream, then wait for a redraw.
async function setEmissions(page, lane, stream) {
  await page.evaluate(
    ({ lane, stream }) => window.PolyMockHost._setEmissions(lane, stream),
    { lane, stream },
  );
  await waitRedraw(page);
}

const em = (step, kind) => ({ ppq: step, step, kind });

test.beforeEach(async ({ page }) => {
  await page.goto(pageUrl);
  await page.evaluate(() => window.PolyMockHost._setTimeSig(4, 4));
  await page.click('#mCloth');
  await expect(page.locator('#cloth')).toHaveClass(/on/);
  // Force the bar-anchored timeline (state 3 = complete: every tick woven).
  await page.evaluate(() => window.PolyMockHost._setCapture({ state: 3, bars: 8 }));
  await page.waitForFunction(
    () => window.__polyClothState && window.__polyClothState.mode === 'timeline',
  );
});

test('a dropped on-pattern step draws a hollow hole while an emitted step draws a solid tick', async ({
  page,
}) => {
  // step 0 dropped, step 2 emitted — both are on-pattern Bell hits, so any
  // difference is the emission classification, not the pattern.
  await setEmissions(page, LANE, [em(0, 'drop'), em(2, 'base')]);
  const [drop, emit] = await measure(page, [DROP_TICK, EMIT_TICK]);

  // The emitted tick is a solid, high-energy fill.
  expect(emit.energy).toBeGreaterThan(30);
  // The dropped step is visibly rendered (a strike marker), but hollow — far
  // less ink than a solid tick. It must NOT read as an emitted hit.
  expect(drop.energy).toBeGreaterThan(1);
  expect(drop.energy).toBeLessThan(emit.energy / 1.5);
  expect(drop.sig).not.toBe(emit.sig);
});

test('changing a step from drop to emitted changes the drawn marker', async ({ page }) => {
  await setEmissions(page, LANE, [em(0, 'drop')]);
  const [dropped] = await measure(page, [DROP_TICK]);

  await setEmissions(page, LANE, [em(0, 'base')]);
  const [emitted] = await measure(page, [DROP_TICK]);

  // Same step, same tick — only the injected kind changed, and the pixels move.
  expect(emitted.sig).not.toBe(dropped.sig);
  expect(emitted.energy).toBeGreaterThan(dropped.energy);
  expect(emitted.energy).toBeGreaterThan(30);
});

test('a ghost emission renders dimmer than a base emission at the same step', async ({ page }) => {
  await setEmissions(page, LANE, [em(0, 'base')]);
  const [base] = await measure(page, [DROP_TICK]);

  await setEmissions(page, LANE, [em(0, 'ghost')]);
  const [ghost] = await measure(page, [DROP_TICK]);

  // A ghost is still drawn (it is an emitted hit)...
  expect(ghost.energy).toBeGreaterThan(3);
  // ...but dimmed (ghost alpha 0.4) relative to a base hit.
  expect(base.energy).toBeGreaterThan(ghost.energy * 1.3);
  expect(ghost.sig).not.toBe(base.sig);
});

test('a macro-resolved add renders off the positional grid', async ({ page }) => {
  // step 1 has NO positional Bell hit, so an emitted add there can only come
  // from the emission stream — it must appear where laneHitAt draws nothing.
  const [emptyBefore] = await measure(page, [NOHIT_TICK]);
  expect(emptyBefore.energy).toBeLessThan(6); // nothing there without a stream

  await setEmissions(page, LANE, [em(1, 'add')]);
  const [added] = await measure(page, [NOHIT_TICK]);

  expect(added.energy).toBeGreaterThan(emptyBefore.energy + 6);
  expect(added.energy).toBeGreaterThan(6);
  expect(added.sig).not.toBe(emptyBefore.sig);
});

test('with no emission stream the timeline shows positional pattern only and never a hole', async ({
  page,
}) => {
  const errors = [];
  page.on('pageerror', (e) => errors.push(e.message));

  // Baseline: the mock default is getLaneEmissions -> [] (no injection).
  const [positional] = await measure(page, [DROP_TICK]);
  // The on-pattern step 0 still draws its solid positional tick via laneHitAt.
  expect(positional.energy).toBeGreaterThan(30);

  // A real drop (injected) is hollow — proving the absent-stream render above is
  // a solid tick, NOT a hole. The degraded view never claims an unobservable drop.
  await setEmissions(page, LANE, [em(0, 'drop')]);
  const [hole] = await measure(page, [DROP_TICK]);
  expect(hole.energy).toBeLessThan(positional.energy / 1.5);

  // Clearing the stream returns to the positional-only solid tick.
  await setEmissions(page, LANE, null);
  const [restored] = await measure(page, [DROP_TICK]);
  expect(restored.energy).toBeGreaterThan(30);
  expect(restored.energy).toBeGreaterThan(hole.energy);

  // No stream, no crash.
  expect(errors).toEqual([]);
  expect(await page.evaluate(() => window.__polyClothState.mode)).toBe('timeline');
});

test('a muted lane is never drawn as a drop (empty stream => no false hole)', async ({ page }) => {
  // Reference: an audible lane's real drop marker (a visible hole).
  await setEmissions(page, LANE, [em(0, 'drop')]);
  const [realDrop] = await measure(page, [DROP_TICK]);
  expect(realDrop.energy).toBeGreaterThan(1);

  // Mute the lane (velocity 0 records no EmissionEvent -> empty stream) and clear
  // any injected stream. The muted on-pattern step must draw NOTHING — not a hole.
  await setEmissions(page, LANE, null);
  await editLaneParam(page, LANE, 'velocity', 0);
  await waitRedraw(page);
  const [muted] = await measure(page, [DROP_TICK]);

  expect(muted.energy).toBeLessThan(realDrop.energy);
  expect(muted.energy).toBeLessThan(4); // ~background: no hit, no hole
});
