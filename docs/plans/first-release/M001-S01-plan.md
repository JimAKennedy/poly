# M001/S01 — Capture reaches the toolbar

**Slice:** M001/S01 in `docs/plans/first-release/ledger.md`
**Rows:** FR01 (the capture controls are gated to Cloth)
**Classification:** bounded. Two small edits to `webui/ui.js` in a flow that
already exists — `updateCaptureChips()` has narrated capture state onto the
toolbar since M047. No new mechanism and no interface change.

## Task status

- [x] 1. The capture controls are visible without entering Cloth
- [ ] 2. The bars chip carries per-bar progress while capturing

## Definition of Done

Copied verbatim from the slice:

- [ ] The capture bars control and Arm are visible without entering Cloth
- [ ] They sit beside Export, which is already toolbar-level and unconditional
- [ ] A test fails if either control becomes mode-dependent again

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |

## What was measured before this plan was written

`webui/ui.js:626` reads `if (capCtl) capCtl.classList.toggle('show', m === 'cloth');`
and `webui/ui.css:74` defines `#capCtl { display: none }` with `#capCtl.show
{ display: flex }`. So the controls are hidden by default and revealed only by
the mode switch.

`updateCaptureChips()` at `ui.js:357` already reflects the capture machine onto
the toolbar every frame: `armBtn` flips `Arm` → `Reset` and gains `.on`,
`capBarsBtn` renders `${capBars} bars` and gains `.locked` at `capState >= 2`,
and `exportChip` gains `.capReady` at `capState === 3`. **The state is already
in the toolbar; only its visibility is gated.**

The one thing the toolbar does not carry is per-bar progress. `updateClothChrome()`
at `ui.js:902` renders `Capturing · bar 3/8` into the capline, which lives
inside `#cloth` and therefore leaves with it in M001/S02. Task 2 moves that
information rather than losing it.

---

## Task 1 — The capture controls are visible without entering Cloth

**Consumes:** nothing. **Produces:** an unconditional `#capCtl`, which M001/S02
depends on before it may remove Cloth.

1. Add a spec case to `webui/tests/capture-timeline.spec.mjs` asserting that
   `#capCtl` is visible on load, with the UI in its default mode and no mode
   chip clicked. Name it so the intent survives the file:
   `capture controls are reachable without entering another view`.
2. Run `npm --prefix webui test -- capture-timeline`. Watch it fail: the
   default mode is desk, so `#capCtl` has no `.show` class and
   `#capCtl { display: none }` applies.
3. Remove the gating line at `ui.js:626` and make the control unconditional in
   `ui.css` — `#capCtl { display: flex }`, deleting the `.show` rule. Leave
   `#capCtl`'s `role="group"` and `aria-label` untouched; they are correct
   already.
4. Re-run. Watch it pass.
5. **Prove the case bites:** restore the gating line, confirm the spec fails,
   revert.
6. Run `format`, `webui-e2e`. Commit with `Slice: M001/S01`, `Rows: FR01` —
   the row does **not** close here, because its verification also requires the
   progress readout to survive.

## Task 2 — The bars chip carries per-bar progress while capturing

**Consumes:** task 1's unconditional controls. **Produces:** the readout that
lets M001/S02 delete the capline without losing information.

`capBarsBtn` renders `8 bars` today. While `capState >= 2` it renders
`3/8 bars`, where the numerator is the same value the capline used:
`Math.min(bars, Math.floor(prog) + 1)`.

1. Add a spec case to `webui/tests/capture-timeline.spec.mjs`:
   `the bars chip reports progress while capturing`. Drive the host to
   `capState = 2` with a known `capBars` and progress, and assert the chip's
   text.
2. Run it. Watch it fail — the chip renders `8 bars` regardless of state.
3. Edit `updateCaptureChips()` so the label is progress-aware while capturing
   and the plain bar count otherwise. Take the progress value from
   `lastFrame`, never from a local counter — the comment above the function
   records that the chips track host truth, and a second source of progress
   would be the bug that comment exists to prevent.
4. Re-run. Watch it pass.
5. **Prove it bites:** make the label unconditional again, confirm the spec
   fails, revert. Then confirm the idle label is still `8 bars`, so the change
   is additive rather than a replacement.
6. Run `format`, `webui-e2e`. Append the slice's evidence, tick both
   definition-of-done boxes that this slice owns plus the guard item — the
   guard is the spec from task 1, which fails if `#capCtl` becomes
   mode-dependent again — set FR01 `done` and the slice `done`, run
   `jk-standards ledger`, and commit with `Slice: M001/S01`, `Rows: FR01`.
