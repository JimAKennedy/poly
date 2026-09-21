# M001/S02 — Cloth leaves the shipped build

**Slice:** M001/S02 in `docs/plans/first-release/ledger.md`
**Rows:** FR02 (Cloth is an unfinished second view), FR03 (Learn describes only
Cloth), FR04 (the guide documents the toggle), FR05 (nothing stops a mode chip
returning)
**Depends:** M001/S01 — capture must already be reachable from the toolbar.
**Classification:** bounded. A deletion across three files plus a spec triage
and one new guard. No new mechanism; the only design decision, where per-bar
progress goes, was taken in S01.

## Task status

- [x] 1. The specs stop depending on Cloth
- [ ] 2. Cloth leaves the markup, the script and the stylesheet
- [ ] 3. The guide stops describing a toggle that is gone
- [ ] 4. A guard keeps the single view single, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] A shipped build contains no Cloth chip, no `#cloth` node, no loom canvas
      and no draw loop
- [ ] Learn is gone, because its annotations only ever described the Cloth
      visualisation
- [ ] `guide-using-poly.mdx` no longer describes a Cloth/Desk toggle
- [ ] A test fails if a mode chip returns to the shipped UI

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `site-unit` | `npm --prefix site test` |

**`site-unit` was added to this slice during planning**, and the ledger is
updated to match. The slice owed `format`, `webui-e2e` and `doc-conformance`,
none of which runs a node source assertion inside the pre-push gate — and
`webui-e2e` is declared pre-push-exempt, so a Playwright-only guard would let a
regression reach CI before anyone saw it. The decision and its reason are in
`M001-decisions.md`.

## What was measured before this plan was written

| | |
|---|---|
| Cloth block in `ui.js` | lines 633–917, about 285 lines |
| Call sites outside that block | `ui.js:629, 649, 1296, 1960, 1971, 2260` |
| Cloth markup in `index.html` | `#modes`, `#cloth`, `#loom`, three `.ann`, `.capline`, `.hintline`, `learnBtn` |
| Cloth CSS in `ui.css` | `#modes`, `#cloth` and its descendants, `.ann`, `body.learn .ann`, `#learnBtn.mode-hidden` |
| **webui specs referencing Cloth** | **9 of 30** |

The nine, with their reference counts: `capture-timeline` (19),
`interaction` (12), `coverage-gaps` (11), `ui` (7), `timeline-emission` (5),
`screenshots` (4), `resilience` (3), `affordance-gaps` (2),
`drift-subdivision-viz` (2).

---

## Task 1 — The specs stop depending on Cloth

**Consumes:** S01's unconditional capture controls. **Produces:** a suite that
is green both before and after task 2, which is the whole point of doing this
first.

Retargeting comes before removal deliberately. A capture assertion pointed at
the toolbar passes with Cloth present *and* with Cloth gone, so the suite is
never red between commits. Removing Cloth first would leave a commit where nine
specs fail.

1. Read each of the nine specs and classify every Cloth-touching assertion as
   one of two kinds:
   - **capture behaviour** — arm, capturing, complete, bar counts, export
     readiness. These survive; retarget them at `#capCtl`, `#capBars`,
     `#armBtn` and `#exportBtn`, which S01 made unconditional.
   - **cloth behaviour** — the loom canvas, band rendering, the three
     annotations, the capline text, the hintline, mode switching by chip or by
     the `1` and `2` keys. These describe something that will not exist, and
     are deleted.
2. Apply the classification. Where a whole spec file is cloth-only, delete the
   file; where a file mixes both, edit it.
3. Run `npm --prefix webui test`. The suite must be green **with Cloth still
   present** — that is the check that the retarget was faithful rather than
   merely different.
4. Record in the evidence file, per spec file, how many assertions were
   retargeted and how many deleted. A triage that does not say what it dropped
   is indistinguishable from one that dropped everything.
5. Run `format`, `webui-e2e`. Commit with `Slice: M001/S02`, `Rows: FR02`.

## Task 2 — Cloth leaves the markup, the script and the stylesheet

**Consumes:** task 1's retargeted suite. **Produces:** the single-view UI.

1. `webui/index.html` — remove the `#modes` container and both chips, the whole
   `<div class="mode" id="cloth">` subtree including `#loom`, the three `.ann`
   nodes, `.capline` and `.hintline`, and the `learnBtn` chip. Correct the
   window's `aria-label`, which reads `desk and cloth modes` and would otherwise
   describe a UI that no longer exists.
2. `webui/ui.js` — remove the cloth block and every call site listed in the
   measurement table above. `setMode`, `sizeLoom`, `drawLoom`,
   `updateClothChrome`, `toggleLearn` and the `1`/`2` key handlers all go. The
   `mode` variable itself goes: with one view there is nothing to switch.
3. `webui/ui.css` — remove `#modes`, the `#cloth` rules, `.ann`,
   `body.learn .ann` and `#learnBtn.mode-hidden`.
4. Run `npm --prefix webui test`. Green, because task 1 already removed every
   assertion that depended on any of this.
5. Check the console gate: `webui/tests` includes a console-error spec, and a
   removed element referenced by a surviving handler would surface there rather
   than as a failed assertion.
6. Run `format`, `webui-e2e`. Commit with `Slice: M001/S02`,
   `Rows: FR02, FR03`.

## Task 3 — The guide stops describing a toggle that is gone

**Consumes:** task 2's UI. **Produces:** documentation that matches it.

`guide-using-poly.mdx` describes the header as carrying a **Cloth/Desk**
toggle, and describes lane columns "in **Desk** mode (shown above)". The
screenshot is already Desk, so it stays; the prose that frames Desk as one of
two modes does not.

1. Edit the passage so it describes the header and the lane columns without
   implying a second view. Do not add an explanation of what was removed — the
   guide describes the product, and the changelog is where a removal is
   recorded. That entry is M004's row FR22, not this task's.
2. Run `bash scripts/check-doc-conformance.sh` and
   `bash scripts/check-doc-discipline.sh`. The second is not one of this
   slice's tokens but the file is a gated doc, and a status-prose or drift
   failure here would surface at push time rather than now.
3. Run `format`, `doc-conformance`. Commit with `Slice: M001/S02`,
   `Rows: FR04`.

## Task 4 — A guard keeps the single view single, and the slice closes

**Consumes:** everything above. **Produces:** the row FR05 verification.

1. Add `site/tests/webui-single-view.test.mjs`. It reads
   `webui/index.html` and `webui/ui.js` from disk and asserts: no `id="modes"`,
   no `mCloth` or `mDesk`, no `id="cloth"`, no `setMode`. Follow
   `doc-conformance-wiring.test.mjs`, which is the precedent in this repo for
   asserting a repo-wide invariant from `site/tests/` — and follow it on the
   point that matters: a pattern that stops matching must **throw**, not
   silently assert nothing, so the test fails loudly if the files move.
2. Run `npm --prefix site test`. It passes immediately, because task 2 already
   removed everything it forbids — so this proves nothing yet, and step 3 is
   what earns it.
3. **Prove it bites**, one mutation per assertion, each reverted: re-add a
   `mCloth` chip to `index.html` → red; re-add a `setMode` function to `ui.js`
   → red; re-add an `id="cloth"` node → red.
4. Run `format`, `site-unit`, `webui-e2e`, `doc-conformance`. Append the
   closing evidence, tick every definition-of-done box, set FR02, FR03, FR04
   and FR05 `done`, set the slice `done`, run `jk-standards ledger`, and commit
   with `Slice: M001/S02`, `Rows: FR02, FR03, FR04, FR05`.
