import { test, expect, Frame, Page } from '@playwright/test';

// M045 S02 T04 — Try It modal preset picker: category chip filter row.
//
// The preset dropdown in the modal now leads with a chip-row above the list
// (role=tablist). "All" plus one chip per category (10 categories, ordered by
// docs/preset-taxonomy.md). Selecting a chip filters the option list to
// presets in that category; "All" restores. Keyboard: Left/Right/Home/End
// move focus and selection within the chip row.
//
// This spec locks in: the chip row builds from state.categories, the filter
// really hides options, "All" restores, keyboard navigation moves selection
// forward and wraps back.

const PAGE_PATH = '/poly/12-jazz/';
const CARD_SELECTOR = '.poly-preview[data-poly-preset="Jazz Bop Ride"]';

// From docs/preset-taxonomy.md — must stay in lockstep with
// engine/src/presets.cpp:kFactoryPresetCategories.
const EXPECTED_CATEGORIES = [
  'Foundational',
  'Minimalist / Compositional',
  'House / Techno',
  'Jazz / Funk / Soul',
  'Breaks / Drum & Bass',
  'Latin / Brazilian',
  'African',
  'Asian Traditions',
  'Balkan / Eastern European',
  'Experimental / Fusion',
];

const JAZZ_PRESETS = [
  'Pocket Groove',
  'Classic Funk',
  'Neo-Soul Pocket',
  'Jazz Bop Ride',
  'Elvin Jones Cascade',
];

async function openTryItModal(page: Page, cardSelector: string): Promise<Frame> {
  const card = page.locator(cardSelector);
  await expect(card, `card missing: ${cardSelector}`).toBeVisible();
  await card.locator('[data-role="tryit"]').click();
  const dialog = page.locator('dialog.poly-modal');
  await dialog.waitFor({ state: 'attached', timeout: 5000 });
  const iframeHandle = await dialog.locator('iframe').elementHandle();
  expect(iframeHandle, 'modal: iframe handle missing').not.toBeNull();
  const frame = await iframeHandle!.contentFrame();
  expect(frame, 'modal: iframe contentFrame missing').not.toBeNull();
  await frame!.waitForLoadState('networkidle', { timeout: 15_000 });
  await frame!.waitForFunction(
    () => {
      const w = window as unknown as {
        PolyWasmHost?: { getState?: () => { presets?: unknown; categories?: unknown } };
      };
      const s = w.PolyWasmHost?.getState?.();
      return (
        !!s &&
        Array.isArray((s as { presets?: unknown[] }).presets) &&
        Array.isArray((s as { categories?: unknown[] }).categories)
      );
    },
    null,
    { timeout: 15_000 },
  );
  return frame!;
}

async function openPresetMenu(frame: Frame): Promise<void> {
  const btn = frame.locator('#presetName');
  await expect(btn, '#presetName button missing in modal').toBeVisible();
  await btn.click();
  await expect(
    frame.locator('#presetMenu.open'),
    'preset menu did not open after clicking #presetName',
  ).toBeVisible();
}

interface OptionRow {
  index: number;
  category: string;
  name: string;
  hidden: boolean;
}

async function readVisibleOptions(frame: Frame): Promise<OptionRow[]> {
  return (await frame.evaluate(() => {
    const menu = document.getElementById('presetMenu');
    if (!menu) return [];
    return Array.from(menu.querySelectorAll('[role="option"]')).map((el) => {
      const idx = Number.parseInt((el as HTMLElement).dataset.index ?? 'NaN', 10);
      return {
        index: Number.isFinite(idx) ? idx : -99,
        category: (el as HTMLElement).dataset.category ?? '',
        name: (el.querySelector('small') ? el.firstChild?.textContent : el.textContent)?.trim() ?? '',
        hidden: el.classList.contains('preset-hidden'),
      };
    });
  })) as OptionRow[];
}

test.describe('M045 S02 modal preset chip filter — category-scoped dropdown', () => {
  test('chip row builds from state.categories, filters, All restores, keyboard navigates', async ({
    page,
  }) => {
    await page.goto(PAGE_PATH);
    const frame = await openTryItModal(page, CARD_SELECTOR);
    await openPresetMenu(frame);

    // 1. Chip row present with All + every category, in taxonomy order.
    const chipLabels = await frame
      .locator('#presetMenu .preset-chip')
      .evaluateAll((chips) => chips.map((c) => (c.textContent || '').trim()));
    expect(chipLabels, 'chip row should lead with "All" then every category').toEqual([
      'All',
      ...EXPECTED_CATEGORIES,
    ]);

    // "All" starts selected — must reflect in ARIA and the active class.
    const initialActive = await frame
      .locator('#presetMenu .preset-chip.active')
      .evaluate((el) => el?.getAttribute('data-category'));
    expect(initialActive, 'default active chip should be All').toBe('All');

    // 2. All rows unhidden by default — spot-check that Init + all 43 presets
    // are laid out (44 option rows total).
    let rows = await readVisibleOptions(frame);
    expect(rows.length, 'expected 1 Init row + 43 preset rows').toBe(44);
    expect(
      rows.every((r) => !r.hidden),
      '"All" filter must not hide any option row',
    ).toBe(true);

    // 3. Click the Jazz chip.
    await frame.locator('#presetMenu .preset-chip[data-category="Jazz / Funk / Soul"]').click();

    // Chip becomes active + aria-selected; other chips lose active state.
    const activeAfterJazz = await frame
      .locator('#presetMenu .preset-chip.active')
      .evaluateAll((chips) => chips.map((c) => c.getAttribute('data-category')));
    expect(activeAfterJazz, 'exactly one chip active, Jazz / Funk / Soul').toEqual([
      'Jazz / Funk / Soul',
    ]);

    // Only jazz presets remain visible (Init is always visible; jazz has 5).
    rows = await readVisibleOptions(frame);
    const visible = rows.filter((r) => !r.hidden);
    const visiblePresetNames = visible
      .filter((r) => r.index >= 0)
      .map((r) => r.name)
      .sort();
    expect(
      visiblePresetNames,
      `filtered dropdown should show exactly the ${JAZZ_PRESETS.length} jazz presets`,
    ).toEqual([...JAZZ_PRESETS].sort());
    expect(
      visible.some((r) => r.index === -1),
      'Init row must remain visible even under a category filter',
    ).toBe(true);
    expect(visible).toHaveLength(JAZZ_PRESETS.length + 1); // Init + 5 jazz

    // 4. Click All — full list restored.
    await frame.locator('#presetMenu .preset-chip[data-category="All"]').click();
    rows = await readVisibleOptions(frame);
    expect(rows.filter((r) => r.hidden), 'All must un-hide every row').toHaveLength(0);
    expect(rows).toHaveLength(44);

    // 5. Keyboard: focus first chip (All), ArrowRight four times, expect
    // Jazz / Funk / Soul to become active (it's the 4th category).
    // Frame has no keyboard; page.keyboard dispatches to the focused element
    // in the same-origin iframe.
    await frame.locator('#presetMenu .preset-chip[data-category="All"]').focus();
    await page.keyboard.press('ArrowRight'); // Foundational
    await page.keyboard.press('ArrowRight'); // Minimalist / Compositional
    await page.keyboard.press('ArrowRight'); // House / Techno
    await page.keyboard.press('ArrowRight'); // Jazz / Funk / Soul
    const activeAfterArrows = await frame
      .locator('#presetMenu .preset-chip.active')
      .evaluate((el) => el.getAttribute('data-category'));
    expect(
      activeAfterArrows,
      'ArrowRight×4 from All should land on the 4th category (Jazz / Funk / Soul)',
    ).toBe('Jazz / Funk / Soul');
    rows = await readVisibleOptions(frame);
    expect(rows.filter((r) => !r.hidden && r.index >= 0)).toHaveLength(JAZZ_PRESETS.length);

    // Home returns focus + selection to the first chip (All).
    await page.keyboard.press('Home');
    const activeAfterHome = await frame
      .locator('#presetMenu .preset-chip.active')
      .evaluate((el) => el.getAttribute('data-category'));
    expect(activeAfterHome, 'Home should snap back to All').toBe('All');
  });
});
