import { test, expect } from '@playwright/test';
import { pageUrl, bootEmbedded, pushEmbeddedState } from './test-helpers.mjs';

// M073 follow-up: desk-mode "played" timeline. A second vertical column beside
// the ladder (expanded strip only) that shows WHAT IS ACTUALLY PLAYED as a
// ROLLING WINDOW scrolling with the transport. Each emitted hit is a dot
// positioned by its AGE behind the playhead (age = frame.t8 − onset·2, where
// onset is the real shifted ppq), so swing / syncopation / micro-timing /
// humanize move a dot off its grid row (genuinely time-positioned, not
// quantized), the freshest hit sits at the top, older hits scroll down and fade,
// and anything older than one cycle (cyc8 eighths) drops off — so the column
// self-clears instead of filling up. The ladder shows pattern INTENT; this
// column ties what the user sees to what they hear. It reads host.getLaneEmissions
// → { ppq, shiftedPpq, step, kind }.
//
// Driven against the mock host (window.PolyMockHost), which owns getLaneEmissions
// and exposes _setEmissions(li, stream) to inject the { ppq, shiftedPpq, step,
// kind } shape the WASM emission drain produces. Headless there is no audio clock
// (now8()===0), so the pump reads window.__POLY_FORCE_T8 for a deterministic
// playhead. The native DAW path is covered by the "plugin-host emission bridge"
// describe at the bottom: the processor drains the engine emission buffer into
// the UISnapshot rings, web_ui_view.cpp serializes them into
// frame.lanes[i].emissions, and plugin-host.js maps the int kind back to a label.
//
// Positioning: win = cyc8(lane) (eighths); age = t8 − onset·2; a dot in
// [0, win] sits at bottom = (1 − age/win)·100% (freshest at top). Drops use the
// grid ppq (they never fire, so no shifted onset).
//
// Fixture: default preset "Afrobeat 12/8" lane 1 "Kick" — a plain pattern lane
// (steps 4, stepLen 2 → cyc8 8 → win 8 eighths). No cells/timeline special-casing.

const LANE = 1;
const WIN = 8; // cyc8 of the Kick lane, in eighths

async function waitRedraw(page) {
  await page.evaluate(
    () =>
      new Promise((res) => {
        let n = 0;
        const step = () => (++n >= 3 ? res() : requestAnimationFrame(step));
        requestAnimationFrame(step);
      }),
  );
}

async function expandStrip(page, lane) {
  await page.click(`.strip[data-lane="${lane}"] .ex`);
  await page.waitForSelector(`.strip[data-lane="${lane}"].expanded`);
}

// Drive a deterministic playhead (eighth-ticks) for the mock pump.
async function forceT8(page, t8) {
  await page.evaluate((v) => { window.__POLY_FORCE_T8 = v; }, t8);
}

async function setEmissions(page, lane, stream) {
  await page.evaluate(
    ({ lane, stream }) => window.PolyMockHost._setEmissions(lane, stream),
    { lane, stream },
  );
  await waitRedraw(page);
}

// Read the played dots for a lane: kind class + bottom% (rounded) + opacity, in DOM order.
async function playedDots(page, lane) {
  return page.evaluate((li) => {
    const col = document.querySelector(`.strip[data-lane="${li}"] .played`);
    if (!col) return null;
    return [...col.querySelectorAll('.pdot')].map((d) => {
      const kind = [...d.classList].find((c) => c.startsWith('p-')) || '';
      return {
        kind,
        bottom: Math.round(parseFloat(d.style.bottom) * 10) / 10,
        opacity: parseFloat(d.style.opacity),
      };
    });
  }, lane);
}

test.beforeEach(async ({ page }) => {
  await page.goto(pageUrl);
  await forceT8(page, WIN); // playhead at t8=8 (one full cycle in) by default
  await expandStrip(page, LANE);
});

test('a syncopated hit lands the played dot at its shifted onset, not the grid step', async ({ page }) => {
  // Playhead at t8=8. Grid step 2 sits at ppq 2.0 (onset t8=4.0), but the audible
  // onset is shifted late to ppq 2.8 (onset t8=5.6). age = 8−5.6 = 2.4 →
  // bottom = (1−2.4/8)·100 = 70%. The grid step would give age 4 → 50%.
  await setEmissions(page, LANE, [{ ppq: 2.0, shiftedPpq: 2.8, step: 2, kind: 'base' }]);

  const dots = await playedDots(page, LANE);
  expect(dots).toHaveLength(1);
  expect(dots[0].kind).toBe('p-base');
  expect(dots[0].bottom).toBe(70); // shifted onset
  expect(dots[0].bottom).not.toBe(50); // NOT the grid step
});

test('positions are continuous in time, not quantized to grid steps', async ({ page }) => {
  // Two hits on the SAME grid step 1 (grid onset t8=2.0) but different sub-grid
  // shifted onsets 1.0 vs 1.25 quarters (t8 2.0 vs 2.5) land at DIFFERENT
  // fractional positions — proving the column reads real time, not the step grid.
  await setEmissions(page, LANE, [{ ppq: 1.0, shiftedPpq: 1.0, step: 1, kind: 'base' }]);
  const a = (await playedDots(page, LANE))[0].bottom; // age 8−2=6 → 25%
  expect(a).toBe(25);

  await setEmissions(page, LANE, [{ ppq: 1.0, shiftedPpq: 1.25, step: 1, kind: 'base' }]);
  const b = (await playedDots(page, LANE))[0].bottom; // age 8−2.5=5.5 → 31.25 → 31.3
  expect(b).toBeCloseTo(31.3, 1);
  expect(b).not.toBe(a); // a quarter-step time shift moves the dot
});

test('kinds map to distinct dot classes; a drop uses its grid position', async ({ page }) => {
  await setEmissions(page, LANE, [
    { ppq: 0.0, shiftedPpq: 0.0, step: 0, kind: 'base' }, // age 8 → drops off (== win)
    { ppq: 1.0, shiftedPpq: 1.4, step: 1, kind: 'ghost' }, // onset t8 2.8, age 5.2 → 35%
    { ppq: 2.0, shiftedPpq: 2.0, step: 2, kind: 'add' }, // onset t8 4.0, age 4 → 50%
    { ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 'drop' }, // grid onset t8 6.0, age 2 → 75%
  ]);

  const dots = await playedDots(page, LANE);
  const byKind = Object.fromEntries(dots.map((d) => [d.kind, d.bottom]));
  // base at age==win is at the far edge (bottom 0) — still just visible.
  expect(byKind['p-base']).toBe(0);
  expect(byKind['p-ghost']).toBe(35);
  expect(byKind['p-add']).toBe(50);
  expect(byKind['p-drop']).toBe(75); // grid ppq, not a shifted onset
  expect(new Set(dots.map((d) => d.kind)).size).toBe(4);
});

test('a hit fades as it ages and drops off past one cycle (no accumulation)', async ({ page }) => {
  const hit = [{ ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 'base' }]; // onset t8 = 6.0

  // Fresh: playhead just past the onset (t8=6.2, age 0.2) → near the top, near full opacity.
  await forceT8(page, 6.2);
  await setEmissions(page, LANE, hit);
  const fresh = (await playedDots(page, LANE))[0];
  expect(fresh.bottom).toBeGreaterThan(90);
  expect(fresh.opacity).toBeGreaterThan(0.9);

  // Older: playhead advanced (t8=11, age 5) → scrolled down and dimmer.
  await forceT8(page, 11);
  await setEmissions(page, LANE, hit);
  const old = (await playedDots(page, LANE))[0];
  expect(old.bottom).toBeLessThan(fresh.bottom);
  expect(old.opacity).toBeLessThan(fresh.opacity);

  // Past one full cycle behind the playhead (t8=15, age 9 > win 8) → gone.
  await forceT8(page, 15);
  await setEmissions(page, LANE, hit);
  expect(await playedDots(page, LANE)).toHaveLength(0);
});

test('an empty stream draws no dots (never claims an unobservable onset)', async ({ page }) => {
  await setEmissions(page, LANE, [{ ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 'base' }]);
  expect(await playedDots(page, LANE)).toHaveLength(1);

  await setEmissions(page, LANE, []);
  expect(await playedDots(page, LANE)).toHaveLength(0);
});

test('the played column only shows in the expanded strip', async ({ page }) => {
  await setEmissions(page, LANE, [{ ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 'base' }]);

  const expandedVisible = await page.evaluate((li) => {
    const col = document.querySelector(`.strip[data-lane="${li}"] .played`);
    return col && getComputedStyle(col).display !== 'none';
  }, LANE);
  expect(expandedVisible).toBe(true);

  const otherLane = LANE === 0 ? 1 : 0;
  const collapsedHidden = await page.evaluate((li) => {
    const col = document.querySelector(`.strip[data-lane="${li}"] .played`);
    return col && getComputedStyle(col).display === 'none';
  }, otherLane);
  expect(collapsedHidden).toBe(true);
});

// Regression guard for the disappearance UAT bug. The column once shipped
// display:block yet visually GONE — transparent background, no border, just a 1px
// hairline that vanished into the strip. `display !== 'none'` passed the whole time
// (that was the deficiency). So assert the RESOLVED PAINT that makes an EMPTY
// column a perceptible rail: a non-transparent background fill AND a bounding
// border with non-zero width and non-transparent colour, occupying its full box.
// This fails on the old invisible CSS (bgAlpha 0, borderWidth 0) and on any future
// change that strips the rail's fill/border while keeping it laid out.
test('the empty played column renders as a perceptible rail (not an invisible box)', async ({ page }) => {
  await setEmissions(page, LANE, []); // no dots — the rail itself must still be visible
  const rail = await page.$eval(`.strip[data-lane="${LANE}"] .played`, (el) => {
    const s = getComputedStyle(el);
    const alpha = (c) => {
      if (c === 'transparent') return 0;
      const m = c.match(/rgba?\(([^)]+)\)/);
      if (!m) return 1;
      const parts = m[1].split(',').map((x) => parseFloat(x));
      return parts.length === 4 ? parts[3] : 1;
    };
    const box = el.getBoundingClientRect();
    return {
      display: s.display,
      bgAlpha: alpha(s.backgroundColor),
      borderTopW: parseFloat(s.borderTopWidth),
      borderAlpha: alpha(s.borderTopColor),
      width: box.width,
      height: box.height,
    };
  });
  expect(rail.display).not.toBe('none');
  expect(rail.bgAlpha).toBeGreaterThan(0); // a filled surface, not transparent
  expect(rail.borderTopW).toBeGreaterThan(0); // a real border, not a hairline pseudo-element
  expect(rail.borderAlpha).toBeGreaterThan(0);
  expect(rail.width).toBeGreaterThan(1); // occupies its column, not a 1px sliver
  expect(rail.height).toBeGreaterThan(10);
});

test('collapsing the strip clears the played dots', async ({ page }) => {
  await setEmissions(page, LANE, [{ ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 'base' }]);
  expect(await playedDots(page, LANE)).toHaveLength(1);

  // Collapse (click the same expand toggle again), then let the pump repaint.
  await page.click(`.strip[data-lane="${LANE}"] .ex`);
  await waitRedraw(page);
  expect(await playedDots(page, LANE)).toHaveLength(0);
});

// The native DAW path: PolyPluginHost.getLaneEmissions reads the per-lane
// emission stream carried on each feedback frame (frame.lanes[i].emissions),
// with kind arriving as the int EmissionKind (0=base,1=ghost,2=add,3=drop) the
// C++ web_ui_view.cpp emits. This proves plugin-host.js maps that int to the
// label the desk played timeline consumes — the same rendering path the mock
// tests above exercise, now fed by the embedded bridge. The frame carries its own
// t8 (the native playhead), so the rolling window is driven by frame.t8 directly.
test.describe('plugin-host emission bridge (native DAW path)', () => {
  // Push a state then a frame carrying per-lane emissions (int kind) through the
  // embedded polyHostPush channel, exactly as the native plugin would.
  async function pushFrameWithEmissions(page, perLaneEmissions, t8 = WIN) {
    await page.evaluate(({ emByLane, t8v }) => {
      const st = window.PolyMockHost.getState();
      const lanes = (st && st.lanes ? st.lanes : []).map((_, i) => ({
        ph: 0,
        step: 0,
        emissions: emByLane[i] || [],
      }));
      window.polyHostPush(JSON.stringify({
        type: 'frame',
        frame: { t8: t8v, playing: true, convLeft: 120, tsNum: 4, tsDen: 4, lanes },
      }));
    }, { emByLane: perLaneEmissions, t8v: t8 });
    await waitRedraw(page);
  }

  test('getLaneEmissions maps int kinds and the played column renders in the DAW host', async ({ page }) => {
    await bootEmbedded(page);
    await pushEmbeddedState(page);
    await expandStrip(page, LANE);

    // Native wire shape: int kind (2 = add), grid + shifted onset. Playhead t8=8.
    const em = { [LANE]: [
      { ppq: 3.0, shiftedPpq: 3.0, step: 3, kind: 0 }, // base, grid onset t8=6, age 2 → 75%
      { ppq: 2.0, shiftedPpq: 2.8, step: 2, kind: 2 }, // add, shifted late, onset t8=5.6, age 2.4 → 70%
    ] };
    await pushFrameWithEmissions(page, em, WIN);

    // getLaneEmissions must return label-mapped entries (int → base/add).
    const mapped = await page.evaluate((li) => window.PolyHost.getLaneEmissions(li), LANE);
    expect(mapped).toHaveLength(2);
    expect(mapped[0].kind).toBe('base');
    expect(mapped[1].kind).toBe('add');
    expect(mapped[1].shiftedPpq).toBe(2.8);

    // And the played column renders those dots by age behind the frame playhead.
    const dots = await playedDots(page, LANE);
    expect(dots).toHaveLength(2);
    const byKind = Object.fromEntries(dots.map((d) => [d.kind, d.bottom]));
    expect(byKind['p-base']).toBe(75); // grid onset ppq 3.0
    expect(byKind['p-add']).toBe(70); // shifted onset ppq 2.8, not the 2.0 grid
  });

  test('an absent emissions array degrades to an empty stream (legacy frame)', async ({ page }) => {
    await bootEmbedded(page);
    await pushEmbeddedState(page);
    await expandStrip(page, LANE);

    // Legacy frame with no emissions field on any lane — getLaneEmissions is [].
    await page.evaluate(() => {
      const st = window.PolyMockHost.getState();
      const lanes = (st && st.lanes ? st.lanes : []).map(() => ({ ph: 0, step: 0 }));
      window.polyHostPush(JSON.stringify({
        type: 'frame',
        frame: { t8: 8, playing: true, convLeft: 120, tsNum: 4, tsDen: 4, lanes },
      }));
    });
    await waitRedraw(page);

    expect(await page.evaluate((li) => window.PolyHost.getLaneEmissions(li), LANE)).toHaveLength(0);
    expect(await playedDots(page, LANE)).toHaveLength(0);
  });
});
