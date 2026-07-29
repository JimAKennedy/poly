import { test, expect } from '@playwright/test';
import { pageUrl, getActions, clearActions } from './test-helpers.mjs';

// M053 S03d T05: parity spec for gap G06 (drag-and-drop MIDI export via a
// native drag-source window). The native platform_drag_source window
// (platform_drag_source_mac.mm / _win.cpp) is the real observable surface and
// cannot be driven from a browser test; what IS verifiable here is the WebUI
// trigger contract that opens it. This spec proves that contract end-to-end
// against the real ui.js render + the mock host:
//   - The Export chip becomes a drag source ONLY under the canExport capability
//     (surfaced in the mock via the ?export=1 seam; true unconditionally in the
//     native plugin host).
//   - A dragstart fires the `beginMidiDrag` bridge action ONLY when the capture
//     machine has latched `complete` (capState===3). In idle/armed/capturing the
//     drag is cancelled (preventDefault) and no action reaches the host — so the
//     native window can never open over a half-frozen / empty capture window.
// This mirrors the Save-As gating already covered elsewhere; the two share the
// capState===3 gate but are independent affordances on the same chip.

const exportUrl = pageUrl + '?export=1';

// Same action-log wrapper as setupWithActionLog, but pointed at the ?export=1
// capability seam so host.capabilities.canExport === true and the Export chip
// mounts its drag-source listeners (draggable + dragstart/dragend).
async function setupExport(page) {
  await page.goto(exportUrl);
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

// Force the capture machine to a state and wait for the rAF pump frame to carry
// it onto lastFrame — observable as the #exportBtn.capReady toggle, which
// updateCaptureChips flips exactly when lastFrame.capState===3.
async function setCapState(page, state) {
  await page.evaluate(
    (s) => window.PolyMockHost._setCapture({ state: s, bars: 8, prog: s === 3 ? 8 : 0 }),
    state
  );
  const btn = page.locator('#exportBtn');
  if (state === 3) await expect(btn).toHaveClass(/capReady/);
  else await expect(btn).not.toHaveClass(/capReady/);
}

// Dispatch a real dragstart carrying a DataTransfer (what the browser fires when
// the user grabs a draggable element) and report what the ui.js handler did.
async function fireDragStart(page) {
  return page.evaluate(() => {
    const btn = document.getElementById('exportBtn');
    const ev = new DragEvent('dragstart', {
      bubbles: true,
      cancelable: true,
      dataTransfer: new DataTransfer(),
    });
    btn.dispatchEvent(ev);
    return { defaultPrevented: ev.defaultPrevented, dragging: btn.classList.contains('dragging') };
  });
}

test.describe('G06 — drag-to-DAW export trigger gating', () => {
  test('the Export chip is a drag source only under the canExport capability', async ({ page }) => {
    // Default (no ?export=1): the mock reports canExport false, so the chip is
    // hidden entirely and never becomes a drag source.
    await page.goto(pageUrl);
    await expect(page.locator('#exportBtn')).toBeHidden();

    // ?export=1: canExport true -> chip visible and marked draggable.
    await page.goto(exportUrl);
    const btn = page.locator('#exportBtn');
    await expect(btn).toBeVisible();
    await expect(btn).toHaveAttribute('draggable', 'true');
  });

  test('dragstart is inert until the capture machine is complete', async ({ page }) => {
    await setupExport(page);
    // Boot state is idle (capState 0): dragging the chip must be cancelled and
    // must NOT reach the host — otherwise the native window would open over an
    // empty capture buffer.
    await setCapState(page, 0);
    await clearActions(page);
    const res = await fireDragStart(page);
    expect(res.defaultPrevented).toBe(true);
    expect(res.dragging).toBe(false);
    const acts = await getActions(page);
    expect(acts.filter((a) => a.name === 'beginMidiDrag')).toEqual([]);
    // The chip carries no in-flight drag marker.
    await expect(page.locator('#exportBtn')).not.toHaveClass(/dragging/);
  });

  test('dragstart fires beginMidiDrag once capture latches complete', async ({ page }) => {
    await setupExport(page);
    await setCapState(page, 3);
    await clearActions(page);
    const res = await fireDragStart(page);
    // The handler accepts the drag (does not cancel) and opens the native
    // drag-source window via the bridge action.
    expect(res.defaultPrevented).toBe(false);
    expect(res.dragging).toBe(true);
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'beginMidiDrag', payload: {} })
    );
    // Exactly one trigger per gesture — no duplicate/Save-As action piggybacks.
    expect(acts.filter((a) => a.name === 'beginMidiDrag')).toHaveLength(1);
    expect(acts.filter((a) => a.name === 'exportSaveAs')).toEqual([]);
    await expect(page.locator('#exportBtn')).toHaveClass(/dragging/);
  });

  test('dragend clears the in-flight drag marker', async ({ page }) => {
    await setupExport(page);
    await setCapState(page, 3);
    await fireDragStart(page);
    await expect(page.locator('#exportBtn')).toHaveClass(/dragging/);
    await page.evaluate(() =>
      document
        .getElementById('exportBtn')
        .dispatchEvent(new DragEvent('dragend', { bubbles: true }))
    );
    await expect(page.locator('#exportBtn')).not.toHaveClass(/dragging/);
  });

  test('every non-complete capture state cancels the drag; only complete fires it', async ({
    page,
  }) => {
    await setupExport(page);
    // Boundary sweep across the whole capState domain: 0 idle, 1 armed,
    // 2 capturing must all be inert; 3 complete is the sole state that triggers.
    for (const state of [0, 1, 2]) {
      await setCapState(page, state);
      await clearActions(page);
      const res = await fireDragStart(page);
      expect(res.defaultPrevented, `capState ${state} should cancel the drag`).toBe(true);
      expect(await getActions(page)).toEqual([]);
    }
    await setCapState(page, 3);
    await clearActions(page);
    const done = await fireDragStart(page);
    expect(done.defaultPrevented).toBe(false);
    expect((await getActions(page)).map((a) => a.name)).toEqual(['beginMidiDrag']);
  });
});
