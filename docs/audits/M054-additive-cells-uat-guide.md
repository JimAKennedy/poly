---
class: gated
---

# Additive Cells — UAT Guide

> **Status (2026-07-31):** Draft UAT script (human-run), deferred — not a release
> blocker. Tracked against M073/M054 WebUI-fix work; not gating any milestone.
> **Lifecycle:** Verification artifact — a manual exploration guide for the
> additive-cells mode. Re-verify any behavioral claim against the cells editor in
> `webui/ui.js` (the `data-cells` render block) and `webui/groove-math.js`.

Scope: the "Additive cells" mode on a lane's Pattern pane only. Everything else
in the strip is out of scope for this guide.

## 1. What additive cells are (mental model)

Normal lanes are **Euclidean**: you pick `steps` (a fixed grid) and `hits`, and
the engine spreads the hits evenly across the grid (Bresenham). Rotation shifts
where they land.

Additive cells replace that with an **aksak / additive** rhythm: instead of "N
even steps with K hits," you define the bar as a **sum of small groups (cells)**,
and the lane fires **one onset at the start of each cell**. The cell's number is
its **length in base subdivisions**, not a hit count.

Example: cells `3 + 3 + 2`
- Cycle length = 8 base subdivisions (`cyc8` = sum = 8).
- Onsets fire at subdivision **0, 3, 6** (the start of each cell).
- 3 onsets total, unevenly spaced — the classic "long-long-short" Balkan feel.
- The first onset (cell 0) is the cycle downbeat and is treated as the accent.

Contrast with Euclidean E(3,8): same 3 hits over 8, but *evenly* distributed
(0, 3, 6 happens to match here, but E(3,7) vs cells `3+2+2` diverge sharply).
The point of additive cells is **deliberate uneven grouping** you control cell by
cell, rather than an even algorithm.

Example: cells `2 + 2 + 3` (the current default on enable)
- Cycle length = 7 base subdivisions (`cyc8` = sum = 7).
- Onsets fire at subdivision **0, 2, 4** (the start of each cell).
- 3 onsets over a 7-length cycle — an odd-metre aksak feel.

Reference (source of truth for expected behavior):
- `webui/groove-math.js` — `cyc8`, `onsets`, `laneHitAt` (cells branch).
- `webui/ui.js` — the cells editor UI (the `data-cl` toggle, `data-cells` cell
  buttons, and `data-addcell` button in the lane Pattern pane).
- Engine: `engine/src/engine.cpp` cycle/onset handling; `poly_action_set_cells`.

## 2. Preconditions

- Load any preset; expand one lane (click the ⤢ on the strip).
- Open the **Pattern** tab in the deep pane.
- Have transport running (site preview: hit play; DAW: start the host transport)
  so you can hear/see onsets.

## 3. Enabling / disabling

| # | Action | Expected |
|---|--------|----------|
| 3.1 | Toggle **Additive cells** switch ON | A `cells` row of variable-width buttons appears below the Rotation control, seeded to `2 2 3`. Steps / Hits / Rotation stay visible but no longer drive the pattern (the engine follows `cells` once present) |
| 3.2 | Read the hint line | Shows `cycle = N♪ (a+b+c)` where N = sum of the cells and `a+b+c` = current cells — e.g. `cycle = 7♪ (2+2+3)` on first enable |
| 3.3 | Toggle OFF | `cells` is cleared (set to `null`); lane returns to Euclidean; the Steps/Hits/Rotation values that were still on screen resume driving the pattern |

Confirm 3.1/3.3 round-trip: toggling off then on re-seeds `2 2 3` (it does **not** remember prior cell edits — worth flagging in UAT if that's undesirable).

## 4. Editing cells

Default cells on first enable are `2 2 3` (three cells, cycle 7). Each cell
button shows its integer value and is `flex`-weighted to its size, so a `3` cell
renders wider than a `2`.

| # | Action | Expected |
|---|--------|----------|
| 4.1 | Drag a cell **up** | Value increases toward 16 (drag maps bottom-of-button = 1, top = 16); the cell widens; `cyc8` and hint update; audible cycle lengthens |
| 4.2 | Drag a cell **down** | Value decreases toward 1 (min 1); narrows; updates as above |
| 4.3 | **Scroll** over a cell | Wheel up = +1, wheel down = −1, clamped to 1–16 |
| 4.4 | **Click** a cell (no drag) | Steps the value **+1, wrapping 16 → 1**. It is a step-up-with-wrap, not a direct set and not a 3-value cycle |
| 4.5 | Click **+** (add cell) | While there are **fewer than 6** cells, appends a `2`; onset count grows by one, cycle lengthens |
| 4.6 | Click **+** at **6 cells** | Cell count is capped at 6 — the button instead **resets the lane to the first two cells** (`slice(0,2)`). Confirm in UAT whether this "wrap back to 2 cells" is the intended max-cell behavior (it is surprising) |
| 4.7 | Reduce a lane to a **single cell** | Not reachable via the UI — there is no remove-cell affordance; the only count changes are +1 (add) and the 6→2 reset. Flag if per-cell removal is wanted |

Bounds confirmed in code (the `setCell` clamp and the `data-addcell` handler in
`webui/ui.js`): value clamped **1–16** per cell; cell **count** ranges 2–6 with
the 6→2 reset above. Still worth probing that
long cycles (6 cells all near 16, cycle ~96) don't break ring/ladder rendering.

## 5. Audible / visual expectations

| # | Check | Expected |
|---|-------|----------|
| 5.1 | Onset timing | Hits land at cumulative cell starts (0, a, a+b, …), NOT evenly |
| 5.2 | Accent | First cell's onset is the accent/downbeat |
| 5.3 | Ladder highlight | The "now" step highlight walks cell to cell in time with audio |
| 5.4 | Ring dots | One dot per cell onset, at fractional position `onset/cyc8` around the ring; first dot emphasized |
| 5.5 | Determinism | Same cells + seed + transport → identical output every pass (engine determinism guarantee; the site preview mirrors it via `laneHitAt`) |

Note (surface difference tracked in M073, not a bug to file here): the **site
preview** runs the full frame pump, so its viz reflects drift rotation and
emission (ghost/skip) overlays; the **DAW plugin** viz drives the ring and ladder
from the same math but is the audible-truth surface. Test 5.1–5.4's *positional*
correctness on the site preview; on the DAW, assert that onsets are audibly
correct rather than that the viz animates.

## 6. Interaction with other controls

| # | Check | Expected |
|---|-------|----------|
| 6.1 | Accents pane | Accent buttons map to **cell onsets** (one per cell), not to a fixed 8/16 grid |
| 6.2 | Micro-timing | One micro-timing bar per cell onset |
| 6.3 | Envelopes | Period is in bars; envelopes shape onsets normally |
| 6.4 | Macros (density/syncopation/etc.) | Confirm documented behavior — do macros mutate an additive lane, or is it treated as fixed like timeline mode? (UAT to establish the intended contract) |
| 6.5 | Save / reload | Cells persist across project save + reopen (DAW) and across preset re-selection |

## 7. Open questions for the UAT spec (to resolve later)

These are the genuine design decisions still open — the mechanical behaviors above
are already settled in code and captured in sections 3–4.

1. **Default cells** `2 2 3` — is odd-metre-7 the right first impression, or should
   enable seed a more familiar grouping (e.g. `3 3 2` = 8)?
2. The **6 → 2 reset** on the `+` button (test 4.6) is surprising — should hitting the
   cap be a no-op, a disabled button, or paired with an explicit remove-cell control?
3. There is **no remove-cell affordance** (test 4.7). Add per-cell delete, or is
   +1/reset the intended full editing vocabulary?
4. **Re-seed on toggle** (test 3.3) discards prior cell edits — should the lane
   remember its last cells across an off→on cycle?
5. Do **macros** apply to additive lanes, or should additive imply "fixed pattern"
   like timeline mode? (Behavior currently undocumented — establish the contract.)
6. Should the ring show **cell boundaries** (arcs/wedges) rather than just onset dots,
   to make the grouping legible? (This is the "not clear how to use" symptom.)

## 8. Pass/fail

This guide passes when sections 3–6 behave as described and the section-7 open
questions have documented answers folded into the shipping behavior. Until then
it stays a **draft UAT**, deferred, not gating any milestone.
