import { fileURLToPath } from 'node:url';
import path from 'node:path';

export const pageUrl =
  'file://' + path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', 'index.html');

export async function setupWithActionLog(page) {
  await page.goto(pageUrl);
  await page.evaluate(() => {
    const host = window.PolyMockHost;
    const origAction = host.action;
    window.__actionLog = [];
    host.action = (name, payload) => {
      window.__actionLog.push({ name, payload: JSON.parse(JSON.stringify(payload)) });
      origAction.call(host, name, payload);
    };
  });
}

export async function getActions(page) {
  return page.evaluate(() => window.__actionLog.slice());
}

export async function clearActions(page) {
  await page.evaluate(() => { window.__actionLog.length = 0; });
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
