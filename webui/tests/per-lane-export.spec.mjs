import { test, expect } from '@playwright/test';
import { pageUrl, getActions, clearActions } from './test-helpers.mjs';

// M032 S02 T03: per-lane MIDI export trigger contract. T02 gave the engine a
// laneFilter (renderPatternToSMF(..., laneFilter)); this spec proves the WebUI
// affordance that carries WHICH lane through to the native handler. Each desk
// lane strip mounts an export/drag handle (plugin-only, canExport-gated) that
// fires the SAME bridge actions as the global Export chip but with a {lane:N}
// payload:
//   - click  -> exportSaveAs { lane: N }   (native: renderCurrentPatternSmf(N))
//   - drag   -> beginMidiDrag { lane: N }   (native: same, into the drag source)
// The global Export chip is unchanged: it still fires an EMPTY payload so the
// all-lanes export is preserved. The native side (web_ui_view.cpp) is verified
// by the engine gtests + build; here we prove the JS trigger + payload contract
// against the real ui.js render and the mock host.
//
// canExport is surfaced in the mock via the ?export=1 seam (true unconditionally
// in the native plugin host); without it the per-lane handles never mount.

const exportUrl = pageUrl + '?export=1';

async function setupExport(page, url = exportUrl) {
  await page.goto(url);
  await page.evaluate(() => {
    const host = window.PolyMockHost;
    const origAction = host.action;
    window.__actionLog = [];
    host.action = (name, payload) => {
      window.__actionLog.push({ name, payload: JSON.parse(JSON.stringify(payload || {})) });
      origAction.call(host, name, payload);
    };
  });
}

// Dispatch a real dragstart on a per-lane handle and report what the handler did.
async function fireLaneDragStart(page, lane) {
  return page.evaluate((li) => {
    const btn = document.querySelector(`.strip[data-lane="${li}"] [data-lex]`);
    const ev = new DragEvent('dragstart', {
      bubbles: true,
      cancelable: true,
      dataTransfer: new DataTransfer(),
    });
    btn.dispatchEvent(ev);
    return { defaultPrevented: ev.defaultPrevented, dragging: btn.classList.contains('dragging') };
  }, lane);
}

test.describe('M032 S02 — per-lane MIDI export trigger', () => {
  test('per-lane export handles mount only under the canExport capability', async ({ page }) => {
    // Default (no ?export=1): mock reports canExport false -> no per-lane handles.
    await page.goto(pageUrl);
    await expect(page.locator('.strip [data-lex]')).toHaveCount(0);

    // ?export=1: canExport true -> one draggable handle per lane strip.
    await page.goto(exportUrl);
    const strips = await page.locator('.strip[data-lane]').count();
    expect(strips).toBeGreaterThan(0);
    const handles = page.locator('.strip [data-lex]');
    await expect(handles).toHaveCount(strips);
    await expect(handles.first()).toHaveAttribute('draggable', 'true');
  });

  test('clicking a lane handle fires exportSaveAs carrying that lane', async ({ page }) => {
    await setupExport(page);
    await clearActions(page);
    await page.click('.strip[data-lane="1"] [data-lex]');
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'exportSaveAs', payload: { lane: 1 } })
    );
    // Exactly one trigger per click — no drag/all-lanes action piggybacks.
    expect(acts.filter((a) => a.name === 'exportSaveAs')).toHaveLength(1);
    expect(acts.filter((a) => a.name === 'beginMidiDrag')).toEqual([]);
  });

  test('dragging a lane handle fires beginMidiDrag carrying that lane', async ({ page }) => {
    await setupExport(page);
    await clearActions(page);
    const res = await fireLaneDragStart(page, 2);
    expect(res.defaultPrevented).toBe(false);
    expect(res.dragging).toBe(true);
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'beginMidiDrag', payload: { lane: 2 } })
    );
    expect(acts.filter((a) => a.name === 'beginMidiDrag')).toHaveLength(1);
    expect(acts.filter((a) => a.name === 'exportSaveAs')).toEqual([]);
    await expect(page.locator('.strip[data-lane="2"] [data-lex]')).toHaveClass(/dragging/);
  });

  test('each lane handle carries its own index', async ({ page }) => {
    await setupExport(page);
    const strips = await page.locator('.strip[data-lane]').count();
    for (let li = 0; li < strips; li++) {
      await clearActions(page);
      await page.click(`.strip[data-lane="${li}"] [data-lex]`);
      const acts = await getActions(page);
      expect(acts.map((a) => ({ name: a.name, payload: a.payload }))).toEqual([
        { name: 'exportSaveAs', payload: { lane: li } },
      ]);
    }
  });

  test('the global Export chip still exports all lanes (empty payload)', async ({ page }) => {
    await setupExport(page);
    await clearActions(page);
    // The global chip carries NO lane -> all-lanes default is preserved.
    await page.click('#exportBtn');
    const acts = await getActions(page);
    expect(acts.map((a) => ({ name: a.name, payload: a.payload }))).toEqual([
      { name: 'exportSaveAs', payload: {} },
    ]);
  });

  test('dragend clears the in-flight per-lane drag marker', async ({ page }) => {
    await setupExport(page);
    await fireLaneDragStart(page, 0);
    const handle = page.locator('.strip[data-lane="0"] [data-lex]');
    await expect(handle).toHaveClass(/dragging/);
    await page.evaluate(() =>
      document
        .querySelector('.strip[data-lane="0"] [data-lex]')
        .dispatchEvent(new DragEvent('dragend', { bubbles: true }))
    );
    await expect(handle).not.toHaveClass(/dragging/);
  });
});
