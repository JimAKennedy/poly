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
