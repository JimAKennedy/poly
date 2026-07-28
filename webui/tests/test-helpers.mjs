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
export async function pushEmbeddedFrame(page, { playing = true } = {}) {
  await page.evaluate((pl) => {
    const st = window.PolyMockHost.getState();
    const lanes = (st && st.lanes ? st.lanes : []).map(() => ({ ph: 0, step: 0 }));
    window.polyHostPush(JSON.stringify({
      type: 'frame',
      frame: { t8: 0, playing: pl, convLeft: 120, tsNum: 4, tsDen: 4, lanes },
    }));
  }, playing);
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
