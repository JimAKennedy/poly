---
class: gated
---

# M001/S01 — Columns and their predicates

**Slice:** M001/S01 — `docs/plans/engine-capability/ledger.md`
**Rows:** EC01, EC02, EC03, EC04
**Classification:** bounded. This repeats the change shape M007 applied to 39
rules on these same pages; no new mechanism is introduced.

## Task status

- [x] 1. `theory-balkan` — Humanize column, Rule 7 restated in ms (EC01)
- [x] 2. `theory-electronic-breakbeat` — Swing column (EC02)
- [x] 3. `theory-gamelan` — Note column (EC03)
- [x] 4. `theory-sub-saharan-africa` — Note column, and slice close-out (EC04)

## Definition of Done

- [ ] Each of the four pages' patch table carries the column its rule needs,
      with values consistent with the page's own rules and the roles the table
      already names
- [ ] Rule 7's Humanize bound is stated in the same unit the new column uses
- [ ] Each of the four rules reads `checkable` in `RULE_TRIAGE`, with a
      predicate that has been shown to fail when the table is mutated
- [ ] The `absentColumn` reverse audit names four fewer rules, and still fails
      if a verdict claims a column the table actually has

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `jk-standards all` |

## Context every task needs

All four tasks edit the same two files and share one mechanism. Read this once.

- **The table** lives inside `<PolyPatch title="...">` in the page's `.mdx`.
  These patches are hand-authored illustrations — none carries a `preset` prop,
  so no value is derived from `site/src/generated/presets.json`. Values are
  authored to satisfy the page's own rules, and the predicate then locks them
  against future edits. That is the same contract every other rule on these
  pages already has.
- **The parser** is `parsePolyPatch(src, title)` in
  `site/tests/theory-patch-conformance.test.mjs`. It returns
  `{ columns, rows }`, where each row is `{ lineno, cell, role, steps, hits,
  rotationRaw, rotation }` and `cell` is keyed by the table's header text.
- **Reading a numeric cell** uses `cellNum(row, 'Column')`, which is
  `parseFloat`. It therefore reads `4ms` as `4` and `8%` as `8`; a suffix in
  the cell is safe.
- **A predicate** is `check: ({ rows, columns }) => null | 'why it failed'`.
  Returning `null` passes. Return a message naming the offending lanes.
- **Registration order is load-bearing.** Every `CHECKLIST.push(...)` must
  appear *above* the line `let liveMarkers = 0;` and the
  `for (const entry of CHECKLIST)` loop beneath it. A push placed after that
  loop is silently never registered — the suite stays green and the case never
  runs. This is the defect M007 nearly shipped inside its own fix. Add each new
  entry immediately after the last existing `CHECKLIST.push` block, well above
  that line.
- **`RULE_TRIAGE`** sits below the loop and is audited in both directions: every
  triage entry must name a registered case, and every registered case must be
  named by a triage entry. Flipping an entry to `checkable` without registering
  its case fails the guard, and vice versa.

## Task 1 — `theory-balkan`: Humanize column and Rule 7's unit (EC01)

Rule 7 reads "Humanize ≤ 0.15". The `Humanize` parameter is
`{6, "Humanize", Kind::LinearFloat, 0.0, 50.0, 0.0}` — milliseconds, 0 to 50 —
and every other rendering of Humanize in this repo is in ms. `0.15` is a
normalized fraction of that range, which is 7.5 ms. The decision recorded in
`M001-decisions.md` is to render the column in ms and restate the rule to match.

**Files:** `site/src/content/docs/theory-balkan.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. In the test file, add a `CHECKLIST.push` entry (above `let liveMarkers = 0;`)
   for page `theory-balkan.mdx`, patch `Rule-Checked Kopanitsa (2+2+3+2+2)`,
   with one rule:
   - `id: 'balkan-humanize-bound'`
   - `description: 'Rule 7: tight ensemble, Humanize ≤ 7.5 ms'`
   - `check`: fail if any row has `cellNum(r, 'Humanize') > 7.5`, naming the
     offending roles and their values.
2. In `RULE_TRIAGE`, change `theory-balkan.mdx` entry `7` to
   `{ checkable: true, case: 'balkan-humanize-bound', why: ... }`, removing the
   `absentColumn` field.
3. Run `npm --prefix site test` and **watch it fail** — the column does not
   exist yet, so `cellNum` returns `NaN` and the bidirectional triage guard has
   a case to find. Confirm the failure names this rule, not something else.
4. In `theory-balkan.mdx`, add a `Humanize` column to the patch table (header,
   separator, and every numbered lane row). Author values that satisfy the rule
   and the page's "near-mechanical unison" framing: the tupan and quick-pulse
   lanes at `0ms`, the ornamental lanes no higher than `4ms`. Every value must
   be ≤ 7.5.
5. In the same file, restate Rule 7's final sentence from `Humanize ≤ 0.15.` to
   `Humanize ≤ 7.5 ms.` Leave the Rice 1994 citation and the rest of the
   sentence untouched — it sources "near-mechanical unison", not the number.
6. Run `npm --prefix site test` and watch it pass.
7. **Mutation-prove it:** set one lane's Humanize to `12ms`, re-run, confirm the
   named case fails and the message names that lane. Revert, confirm
   `git diff --quiet` on the mdx, re-run green.
8. Run the full validation set. Append evidence to
   `docs/plans/engine-capability/evidence/M001-S01.md`, tick task 1, set row
   EC01 to `done`, run `jk-standards ledger`, commit with the slice's trailers.

## Task 2 — `theory-electronic-breakbeat`: Swing column (EC02)

Rule 4: one swing value across the swung layers, kick and clap straight. The
decision recorded in `M001-decisions.md` is the strict reading — the patch
models the rule's default, not the "legitimate advanced move" it permits.

**Files:** `site/src/content/docs/theory-electronic-breakbeat.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. Add a `CHECKLIST.push` entry (above `let liveMarkers = 0;`) for page
   `theory-electronic-breakbeat.mdx`, patch `Rule-Checked Jungle Frame`, rule:
   - `id: 'ebb-swing-is-a-bus'`
   - `description: 'Rule 4: one swing value across swung layers; kick and clap straight'`
   - `check`: collect `new Set(rows.map(r => cellNum(r, 'Swing')).filter(v => v !== 0))`;
     fail if its size is greater than 1, naming the distinct values. Then fail
     if any row whose `role` matches `/kick|clap/i` has a non-zero Swing.
2. Change the `RULE_TRIAGE` entry for `theory-electronic-breakbeat.mdx` rule
   `4` to `{ checkable: true, case: 'ebb-swing-is-a-bus', why: ... }`, removing
   the `absentColumn` field.
3. Run the site suite and watch it fail for the absent column.
4. Add the `Swing` column to the patch table. Kick and clap `0`; the swung
   layers (hats, shakers, percussion) all carry the same single value.
5. Run and watch it pass.
6. **Mutation-prove it twice** — the predicate has two arms, and an arm that is
   never exercised is not proved:
   - give one shaker lane a different non-zero Swing → the set-size arm fails;
   - give the kick a non-zero Swing → the kick/clap arm fails.
   Revert after each, confirm `git diff --quiet`, re-run green.
7. Full validation set, evidence, tick task 2, EC02 `done`,
   `jk-standards ledger`, commit with trailers.

## Task 3 — `theory-gamelan`: Note column (EC03)

Rule 9: "Density scales with register, inversely. The lowest instruments play
slowest, the highest fastest."

**Files:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. Add a `CHECKLIST.push` entry (above `let liveMarkers = 0;`) for page
   `theory-gamelan.mdx`, patch `Rule-Checked Kotekan Over Colotomy`, rule:
   - `id: 'gamelan-density-inverts-register'`
   - `description: 'Rule 9: higher register plays denser, lower plays sparser'`
   - `check`: build `{ note: cellNum(r, 'Note'), rate: r.hits / r.steps, role }`
     for every row; sort by `note` ascending; fail if any adjacent pair has a
     strictly higher note with a strictly lower rate, naming both lanes. Ties in
     either dimension pass — the rule is a monotonic trend, not a strict order.
2. Change the `RULE_TRIAGE` entry for `theory-gamelan.mdx` rule `9` to
   `{ checkable: true, case: 'gamelan-density-inverts-register', why: ... }`,
   removing the `absentColumn` field.
3. Run the site suite and watch it fail.
4. Add the `Note` column. Assign MIDI note numbers consistent with the roles the
   table already names, lowest to highest: gong ageng lowest, then kempul and
   kenong, with the kotekan pair highest — the pyramid the rule describes. The
   existing `Steps`/`Hits` already encode the rates, so choose notes that make
   the ordering hold rather than changing any rate.
5. Run and watch it pass.
6. **Mutation-prove it:** swap the gong's note number with a kotekan lane's so a
   low voice becomes the fastest. Re-run, confirm the case fails naming both
   lanes. Revert, confirm `git diff --quiet`, re-run green.
7. Full validation set, evidence, tick task 3, EC03 `done`,
   `jk-standards ledger`, commit with trailers.

## Task 4 — `theory-sub-saharan-africa`: Note column, and close the slice (EC04)

Rule 7: "Register and rate separate the voices … Two parts in the same stratum
at the same rate" is the failure the rule names.

**Files:** `site/src/content/docs/theory-sub-saharan-africa.mdx`,
`site/tests/theory-patch-conformance.test.mjs`,
`docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M001-S01.md`

1. Add a `CHECKLIST.push` entry (above `let liveMarkers = 0;`) for page
   `theory-sub-saharan-africa.mdx`, patch `Rule-Checked Ewe Texture`, rule:
   - `id: 'ssa-register-and-rate-separate'`
   - `description: 'Rule 7: no two voices share both stratum and rate'`
   - `check`: for each row read `cellNum(r, 'Note')` and compute its rate with
     the `voiceRate` helper task 3 introduced. Fail if any two rows share both
     the same Note and the same rate, naming both roles. Rule 7's first
     sentence asks for a distinct *combination* of register and note-rate, so
     no band boundaries are invented: the pair that is not individually audible
     is the one at a single pitch and a single rate.
2. Change the `RULE_TRIAGE` entry for `theory-sub-saharan-africa.mdx` rule `7`
   to `{ checkable: true, case: 'ssa-register-and-rate-separate', why: ... }`,
   removing the `absentColumn` field.
3. Run the site suite and watch it fail.
4. Add the `Note` column, giving each voice the register its role implies: bell
   high, dunun low, accompaniment djembes mid, lead djembe high. Kidi and sogo
   are different drums and take different notes — sogo is the larger and lower
   of the pair. Three lanes already share a rate of 2.67 (dance beat, kidi,
   sogo), so the notes are what make their combinations distinct.
5. Run and watch it pass.
6. **Mutation-prove it:** set Kidi's Note equal to Sogo's. They already share a
   rate, so this makes the combination non-distinct. Re-run, confirm the case
   fails naming both. Revert, confirm `git diff --quiet`, re-run green.
7. **Confirm the reverse audit moved.** Run the site suite and read the
   `absentColumn` audit's output: it must now name exactly one rule —
   `theory-brazilian` rule 6 — where it previously named five. If it names any
   other, stop: a verdict is wrong, which is exactly the defect row B18
   recorded, and it is not this task's to fix silently.
8. Full validation set. Append evidence, tick task 4, set EC04 `done`, tick all
   four definition-of-done boxes in the ledger, set slice M001/S01 to `done`,
   run `jk-standards ledger`, commit with trailers.
