import { fileURLToPath } from 'node:url';
import path from 'node:path';

export const pageUrl =
  'file://' + path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', 'index.html');

export async function setupWithActionLog(page) {
  await page.goto(pageUrl);
  await page.evaluate(() => {
    const host = window.PolyMockHost;
    const origAction = host.action;
    const origEdit = host.edit;
    window.__actionLog = [];
    window.__editLog = [];
    host.action = (name, payload) => {
      window.__actionLog.push({ name, payload: JSON.parse(JSON.stringify(payload)) });
      origAction.call(host, name, payload);
    };
    host.edit = (paramId, value, gesture) => {
      window.__editLog.push({ paramId, value, gesture });
      origEdit.call(host, paramId, value, gesture);
    };
  });
}

export async function getActions(page) {
  return page.evaluate(() => window.__actionLog.slice());
}

export async function clearActions(page) {
  await page.evaluate(() => { window.__actionLog.length = 0; });
}

export async function getEdits(page) {
  return page.evaluate(() => window.__editLog.slice());
}

export async function clearEdits(page) {
  await page.evaluate(() => { window.__editLog.length = 0; });
}

export async function startContinuousPush(page, intervalMs = 33) {
  await page.evaluate((ms) => {
    window.__pushCount = 0;
    window.__pushTimer = setInterval(() => {
      window.PolyMockHost._pushState();
      window.__pushCount++;
    }, ms);
  }, intervalMs);
  await page.waitForFunction(() => window.__pushCount >= 3);
}

export async function stopContinuousPush(page) {
  await page.evaluate(() => {
    clearInterval(window.__pushTimer);
    window.__pushTimer = null;
  });
}

export async function expandStrip(page, lane) {
  await page.click(`.strip[data-lane="${lane}"] .ex`);
  await page.waitForSelector(`.strip[data-lane="${lane}"].expanded`);
}

// Boot the UI the way the native plugin does: __POLY_EMBEDDED__ set before load
// (so PolyHost === PolyPluginHost), a stubbed native binding so JS→native calls
// are swallowed, and an optional shortened routing-banner dwell so specs don't
// wait the production 4s. addInitScript re-runs on every navigation, so this
// survives a page.reload() — needed by the dismiss-persistence spec.
export async function bootEmbedded(page, { bannerMs } = {}) {
  await page.addInitScript((ms) => {
    window.__POLY_EMBEDDED__ = true;
    window.polyHostCall = () => {}; // no plugin attached; drop JS→native traffic
    if (typeof ms === 'number') window.__POLY_ROUTING_BANNER_MS = ms;
  }, bannerMs);
  await page.goto(pageUrl);
}

// Push a state snapshot through the embedded bridge (C++→JS path). The plugin
// host renders nothing until the first state push arrives, so specs must call
// this before pushing frames. muteAllLanes deactivates every lane so no lane
// emits a hit (the "nothing routed but also nothing playing" negative case).
export async function pushEmbeddedState(page, { muteAllLanes = false } = {}) {
  await page.evaluate((muteAll) => {
    const st = window.PolyMockHost.getState();
    if (muteAll) st.lanes.forEach((l) => { l.active = false; });
    window.polyHostPush(JSON.stringify({ type: 'state', state: st }));
  }, muteAllLanes);
}

// Push one feedback frame through the embedded bridge with a lanes array sized
// to the current state so the desk-mode frame handler never dereferences undefined.
//
// driftOffsets/steps are optional index-aligned overrides so a spec can drive a
// deterministic per-lane drift offset (and drifted playhead step) into the desk
// frame pump — the ring <g> rotates by driftOffset/steps*360 (M053 S12 T03/T04).
// Both default to 0, so existing callers see the same {ph:0, step:0} frame.
export async function pushEmbeddedFrame(
  page,
  { playing = true, driftOffsets = null, steps = null } = {},
) {
  await page.evaluate(({ pl, drifts, stps }) => {
    const st = window.PolyMockHost.getState();
    const lanes = (st && st.lanes ? st.lanes : []).map((_, i) => ({
      ph: 0,
      step: stps && stps[i] != null ? stps[i] : 0,
      driftOffset: drifts && drifts[i] != null ? drifts[i] : 0,
    }));
    window.polyHostPush(JSON.stringify({
      type: 'frame',
      frame: { t8: 0, playing: pl, convLeft: 120, tsNum: 4, tsDen: 4, lanes },
    }));
  }, { pl: playing, drifts: driftOffsets, stps: steps });
}

// Read the rotate() angle (degrees) off the ring <g> for a lane in the desk view.
// The ring group is the <g> inside svg.ring; the frame pump sets
// transform="rotate(<deg> 32 32)" on it (ui.js). Returns 0 when no transform is
// set yet (a freshly built desk before the first frame).
export async function ringRotation(page, lane) {
  return page.evaluate((li) => {
    const g = document.querySelector(`.strip[data-lane="${li}"] svg.ring g`);
    if (!g) return null;
    const t = g.getAttribute('transform');
    if (!t) return 0;
    const m = /rotate\(\s*(-?[\d.]+)/.exec(t);
    return m ? parseFloat(m[1]) : 0;
  }, lane);
}

// Snapshot the ring's dot geometry (cx/cy/r of every <circle> in the ring <g>)
// for a lane. Ring dots sit at cycle fractions (i/steps or onset/cyc8), so this
// is the geometry that D008 says is invariant under a subdivision (stepLen) edit.
export async function ringDots(page, lane) {
  return page.evaluate((li) => {
    const g = document.querySelector(`.strip[data-lane="${li}"] svg.ring g`);
    if (!g) return null;
    return [...g.querySelectorAll('circle')].map((c) => ({
      cx: c.getAttribute('cx'),
      cy: c.getAttribute('cy'),
      r: c.getAttribute('r'),
    }));
  }, lane);
}

// Edit one lane parameter on the mock host through the real begin/perform/end
// gesture the UI uses, so state mutates and re-renders exactly as a user drag would.
export async function editLaneParam(page, lane, field, value) {
  await page.evaluate(({ li, f, v }) => {
    const id = `lane.${li}.${f}`;
    window.PolyMockHost.edit(id, v, 'begin');
    window.PolyMockHost.edit(id, v, 'perform');
    window.PolyMockHost.edit(id, v, 'end');
  }, { li: lane, f: field, v: value });
}

// Fingerprint the convergence-timeline canvas (#loom) so a spec can prove its
// geometry changed (or did not) across an edit. Returns canvas dims plus a cheap
// rolling hash of the pixel buffer and a count of non-transparent pixels.
export async function loomFingerprint(page) {
  return page.evaluate(() => {
    const c = document.getElementById('loom');
    const g = c.getContext('2d');
    const d = g.getImageData(0, 0, c.width, c.height).data;
    let hash = 0;
    let painted = 0;
    for (let i = 0; i < d.length; i += 4) {
      hash = (hash * 31 + d[i] + d[i + 1] * 7 + d[i + 2] * 13 + d[i + 3] * 17) >>> 0;
      if (d[i + 3] !== 0) painted++;
    }
    return { w: c.width, h: c.height, hash, painted };
  });
}

// Drive the routing banner's wall-clock dwell: the first frame arms the timer,
// the wait exceeds the shortened banner window, and the second emitting frame
// fires the banner (if the emit condition still holds). The banner fires only
// on the second-or-later frame by construction, so a single frame never shows it.
export async function driveRoutingDwell(page, { playing = true } = {}) {
  await pushEmbeddedFrame(page, { playing });
  await page.waitForTimeout(150);
  await pushEmbeddedFrame(page, { playing });
}

// Independent reference implementation of engine.cpp computeDriftedCycleStep's
// drift term (engine.cpp:361-364). Kept structurally separate from the
// PolyGrooveMath.driftOffset under test so the parity spec compares two
// independently-written expressions of the same formula rather than one helper
// against itself. barPos = ppq / kPpqPerBar (kPpqPerBar = 4.0 in the engine).
export function engineDriftOffsetRef(steps, driftRate, barPos) {
  if (steps <= 0 || driftRate === 0) return 0;
  const driftSteps = Math.floor(barPos * driftRate);
  let m = driftSteps % steps;
  if (m < 0) m += steps;
  return m === 0 ? 0 : m; // normalize JS -0 (e.g. -9 % 3) to +0
}
