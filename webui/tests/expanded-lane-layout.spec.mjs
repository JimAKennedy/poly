import { test, expect } from '@playwright/test';
import { pageUrl } from './test-helpers.mjs';

// M031 S02 — WebUI expanded-lane layout and minimum width.
//
// Two owner-reported failures in the DAW WebUI desk (the plugin editor ships at
// 1160x760 and fills 100vw when embedded — web_ui_view.cpp:65):
//   1. Init grown to 8 lanes: expanding a lane clips its 5-button tab row and
//      crushes the other 7 collapsed strips to an unreadable sliver.
//   2. Afro-House Phrases (5 lanes): expanding a lane clips the tab row.
//
// The expanded strip width is a fraction of the desk grid (expandStrip() in
// ui.js sets grid-template-columns to `<wide>fr` for the expanded lane, a
// `<narrow>fr` for each collapsed lane, plus a fixed master column). The strip
// has `overflow: hidden` and the `.tabs` buttons do not shrink (no flex on
// them), so once the expanded strip's `.deep` column is narrower than the
// natural tab-row width the last buttons clip off the right edge.
//
// T01 is a REPRODUCE + MEASURE task. The `measure` test always passes and logs
// the numbers the fix (T02/T03) must target. The `invariant` tests assert the
// target behavior and are RED until the fix lands (that red IS the reproduction).

// The DAW plugin editor default size (web_ui_view.cpp ViewRect(0,0,1160,760)).
const DAW = { width: 1160, height: 760 };

async function bootDaw(page) {
  // The desk grid math is host-independent. We boot the standalone mock host
  // (renders immediately) at the DAW editor's 1160px width, where #win =
  // min(1160px, 100%) matches the embedded 100vw desk width, so the measured
  // grid columns equal the DAW case. The live deskClientWidth is recorded too.
  await page.setViewportSize(DAW);
  await page.goto(pageUrl);
  await page.waitForSelector('#desk .strip');
}

// Apply a factory preset by visible name via the real menu flow.
async function applyPresetByName(page, name) {
  await page.click('#presetName');
  await page.click(`#presetMenu [role="option"]:has-text("${name}")`);
  await expect(page.locator('#presetName')).toHaveText(name);
}

async function setLaneCount(page, target) {
  for (let guard = 0; guard < 12; guard++) {
    const n = await page.locator('#desk .strip').count();
    if (n === target) return;
    await page.click(n < target ? '.lane-mgr .lane-add' : '.lane-mgr .lane-del');
    await expect(page.locator('#desk .strip')).toHaveCount(n < target ? n + 1 : n - 1);
  }
  throw new Error(`could not reach ${target} lanes`);
}

// Measure the expanded lane's tab row and the collapsed strips.
async function measureCase(page, laneToExpand) {
  await page.click(`.strip[data-lane="${laneToExpand}"] .ex`);
  await expect(page.locator(`.strip[data-lane="${laneToExpand}"].expanded`)).toBeVisible();
  // Let the grid-template-columns transition settle.
  await page.waitForTimeout(400);

  return page.evaluate((li) => {
    const strip = document.querySelector(`.strip[data-lane="${li}"].expanded`);
    const tabs = strip.querySelector('.tabs');
    const deep = strip.querySelector('.deep');
    const btns = [...tabs.querySelectorAll('button')];
    const tabsRect = tabs.getBoundingClientRect();
    const deepRect = deep.getBoundingClientRect();

    // Natural width the 5-button row wants: sum of button widths + gaps. The
    // buttons never shrink (no flex), so scrollWidth is the true requirement.
    const naturalTabsWidth = tabs.scrollWidth;
    // Last button's right edge vs the visible (clipped) container right edge.
    const lastBtn = btns[btns.length - 1].getBoundingClientRect();
    const lastBtnRightOverflow = Math.round(lastBtn.right - deepRect.right);
    const lastBtnFullyVisible = lastBtn.right <= deepRect.right + 0.5;
    const tabsClipped = tabs.scrollWidth > Math.ceil(tabsRect.width) + 1;

    // Collapsed strips: name (.nm) + ring (svg.ring) legibility floor.
    const collapsed = [...document.querySelectorAll('#desk .strip:not(.expanded)')].map((s) => {
      const r = s.getBoundingClientRect();
      const ring = s.querySelector('svg.ring');
      const nm = s.querySelector('.nm');
      return {
        lane: s.dataset.lane,
        width: Math.round(r.width),
        contentWidth: Math.round(r.width - 20), // 10px L/R padding on .strip
        ringWidth: ring ? Math.round(ring.getBoundingClientRect().width) : 0,
        nmClipped: nm ? nm.scrollWidth > nm.clientWidth + 1 : false,
      };
    });

    const desk = document.getElementById('desk');
    return {
      viewport: { w: window.innerWidth, h: window.innerHeight },
      deskClientWidth: desk.clientWidth,
      deskScrollWidth: desk.scrollWidth,
      deskOverflow: desk.scrollWidth > desk.clientWidth + 1,
      expandedStripWidth: Math.round(strip.getBoundingClientRect().width),
      deepWidth: Math.round(deepRect.width),
      naturalTabsWidth: Math.round(naturalTabsWidth),
      visibleTabsWidth: Math.round(tabsRect.width),
      tabsClipped,
      lastBtnRightOverflow,
      lastBtnFullyVisible,
      buttonWidths: btns.map((b) => Math.round(b.getBoundingClientRect().width)),
      minCollapsedWidth: collapsed.length ? Math.min(...collapsed.map((c) => c.width)) : null,
      collapsed,
    };
  }, laneToExpand);
}

// Measure the Envelopes pane's effect on desk width / collapsed strips.
async function measureEnvelopePane(page, laneExpanded) {
  await page.click(`.strip[data-lane="${laneExpanded}"] .tabs button[data-tab="env"]`);
  await page.waitForTimeout(250);
  return page.evaluate(() => {
    const desk = document.getElementById('desk');
    const collapsed = [...document.querySelectorAll('#desk .strip:not(.expanded)')]
      .map((s) => Math.round(s.getBoundingClientRect().width));
    return {
      deskClientWidth: desk.clientWidth,
      deskScrollWidth: desk.scrollWidth,
      deskOverflow: desk.scrollWidth > desk.clientWidth + 1,
      minCollapsedWidth: collapsed.length ? Math.min(...collapsed) : null,
      collapsedWidths: collapsed,
    };
  });
}

const CASES = [
  { label: 'Init 8-lane', preset: 'Init', lanes: 8, expand: 0 },
  { label: 'Afro-House Phrases 5-lane', preset: 'Afro-House Phrases', lanes: 5, expand: 0 },
];

test.describe('M031 S02 — expanded-lane layout (reproduce + measure)', () => {
  for (const c of CASES) {
    test(`measure: ${c.label}`, async ({ page }) => {
      await bootDaw(page);
      await applyPresetByName(page, c.preset);
      await setLaneCount(page, c.lanes);
      const m = await measureCase(page, c.expand);
      const env = await measureEnvelopePane(page, c.expand);
      // Recorded evidence — read these from the Playwright `list` output.
      console.log(`\n[MEASURE ${c.label}] ${JSON.stringify({ pattern: m, env }, null, 2)}`);
      expect(m.buttonWidths.length).toBe(5); // sanity: five tab buttons exist
    });
  }

  // --- Target invariants: RED until T02/T03 fix, GREEN after (the reproduction) ---
  for (const c of CASES) {
    test(`invariant: ${c.label} — all 5 tab buttons visible on expand`, async ({ page }) => {
      await bootDaw(page);
      await applyPresetByName(page, c.preset);
      await setLaneCount(page, c.lanes);
      const m = await measureCase(page, c.expand);
      expect(m.tabsClipped, `tab row clipped by ${m.naturalTabsWidth - m.visibleTabsWidth}px`).toBe(false);
      expect(m.lastBtnFullyVisible, `last tab button overflows by ${m.lastBtnRightOverflow}px`).toBe(true);
    });

    test(`invariant: ${c.label} — collapsed strips stay usable`, async ({ page }) => {
      await bootDaw(page);
      await applyPresetByName(page, c.preset);
      await setLaneCount(page, c.lanes);
      const m = await measureCase(page, c.expand);
      // Usable floor: a collapsed strip must be at least wide enough to show the
      // 64px ring + padding without crushing to a sliver. 84px = 64 + 20 padding.
      expect(m.minCollapsedWidth, `narrowest collapsed strip ${m.minCollapsedWidth}px`).toBeGreaterThanOrEqual(84);
    });

    test(`invariant: ${c.label} — Envelopes pane does not blow out the desk`, async ({ page }) => {
      await bootDaw(page);
      await applyPresetByName(page, c.preset);
      await setLaneCount(page, c.lanes);
      await measureCase(page, c.expand);
      const env = await measureEnvelopePane(page, c.expand);
      expect(env.minCollapsedWidth, `narrowest collapsed strip on Envelopes ${env.minCollapsedWidth}px`).toBeGreaterThanOrEqual(84);
    });
  }
});
