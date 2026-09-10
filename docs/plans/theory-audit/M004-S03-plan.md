# M004/S03 — Chapter 5 patch

**Slice:** M004/S03, in `docs/plans/theory-audit/ledger.md`
**Depends:** M004/S05, `done`
**Decisions:** `M004-decisions.md`

One row, F41, with two halves. The pokok half is uncontested. The Rule 4 half
required a decision, because the predicate S05 shipped demanded something Poly
cannot express.

## Task status

- [x] Task 1 — Redefine the Rule 4 predicate to what the page actually specifies
- [x] Task 2 — Give the Chapter 5 kotekan patch its pokok layer
- [x] Task 3 — Close the slice

## Definition of Done

- [x] The Chapter 5 kotekan patch carries a pokok layer, as `theory-gamelan`
      Rule 6 requires
- [x] It carries structural overlap at the cycle boundary, as Rule 4 requires

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `e2e` | `bash scripts/site-verify-local.sh` |

After any `e2e` run, restore `webui/poly_engine.js` and `webui/poly_engine.wasm`
from `origin/main` and stage explicit paths. `e2e` rebuilds them
non-reproducibly — ledger row B16, issue #282.

## Why the Rule 4 predicate changes

S05 wrote `gam-structural-overlap` as *the kotekan pair must share an onset;
under `Kotekan L<n>` sangsih is the strict complement, so they intersect nowhere
by construction*. Three things in the tree say that is too strict:

- `theory-gamelan`'s own reference patch uses `L6` for its sangsih, so the
  predicate condemns the worked example the rule is derived from.
- Rule 4's own parenthetical says so: *"Poly's Kotekan `L1` implements the
  strict case; add deliberate doublings via a third lane or accent masks until
  kotekan modes ship."*
- Construction step 4 names the available remedy: *"accent masks on both pair
  lanes, or a sparse third lane striking with both at the gong point."*

So the rule becomes what step 4 specifies: **a lane outside the kotekan pair
strikes at the cycle boundary together with a pair lane.** That is weaker than
what S05 shipped, and the weakening is deliberate and recorded — a rule that
demands the impossible gets suppressed, and a suppression that can never be
burned down is worse than an honest weaker rule.

## Task 1 — Redefine the Rule 4 predicate

Closes no row. Produces the corrected oracle Task 2 is measured against.

**Files:** `site/tests/theory-patch-conformance.test.mjs`

1. **Remove the `gam-structural-overlap` marker** from `05-gamelan.mdx` and run
   `node --test site/tests/theory-patch-conformance.test.mjs`. It must fail with
   the current strict message about `L1` and complementation. That failure is
   the thing being corrected, so see it before changing it.

2. **Rewrite the predicate.** Identify the pair by role — polos and sangsih —
   and treat every other lane as outside it. Require that some pair lane and
   some non-pair lane both sound at step 0 of their own cycle, which is the
   cycle boundary the construction calls the gong point. Use `laneOnsets`.

   Do **not** derive sangsih's onsets from its own triple when its `Kotekan`
   cell is `L<n>`: under L-mode the engine derives them from the source lane, so
   the table's triple does not describe what sounds. Polos is the pair member
   whose onsets the table does describe.

   The failure message must say which side is missing — no pair lane at the
   boundary, or no lane outside the pair doubling it — because those need
   different fixes.

3. **Run it.** As measured while planning, Chapter 5 should now pass: polos
   sounds on `{0,2,3,5,6}` and the Jegogan bass on `{0,4}`, so a lane outside
   the pair already doubles polos at the boundary. Treat that as a prediction to
   check. If it still fails, stop and report rather than adjusting the predicate
   until it passes — that would be fitting the rule to the patch.

4. **Prove it bites.** Rotate the Jegogan lane off the boundary, watch the rule
   fail naming the missing doubling, restore by inverse edit, confirm with
   `git diff --stat`.

5. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

6. **Commit** with `Rows: —`, ticking Task 1, appending to
   `docs/plans/theory-audit/evidence/M004-S03.md`. Say in the message that the
   rule was weakened and why, so nobody reads the diff as a silent relaxation.

## Task 2 — Give the Chapter 5 kotekan patch its pokok layer

Closes **F41**.

**Files:** `site/src/content/docs/05-gamelan.mdx`

`theory-gamelan` Rule 6: the interlock serves a melody, and its accents must
land on the pokok tones at the pokok's rate. Construction step 2 builds it as a
mid-register lane, 8 steps, 4–8 hits, even and calm. The theory page's own patch
carries it as `Pokok melody`, 8 steps, 4 hits.

1. **Remove the `gam-pokok-layer` marker** and watch the rule fail with `no
   pokok lane among Polos, Sangsih, Jegogan bass, Reyong accent`.

2. **Add the pokok lane.** Mid-register: the Jegogan bass is note 48 and the
   reyong 67, so the pokok belongs between them. 8 steps, 4 hits, rotation 0,
   1/4 subdivision to sit at the pokok's slower rate against the pair's 1/16,
   Ghost 0 and Kotekan off because it is a structural lane, velocity between the
   Jegogan's 100 and the reyong's 75. Place it before the pair, matching the
   construction's order, and renumber.

   The patch binds `preset="Balinese Kotekan"`, which is a display caption:
   `polypatch-preset-resolution.test.mjs` requires only that the name resolves.
   Do not edit the preset.

3. **Run and watch both `gam-` rules pass**, with the suppression count down
   from 3 to 1.

4. **Prove it bites.** Remove the pokok lane, watch `gam-pokok-layer` fail,
   restore by inverse edit.

5. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

6. **Commit** with `Rows: F41`, ticking Task 2, setting F41 `done` naming both
   `gam-pokok-layer` and `gam-structural-overlap` in its `Verification`.

## Task 3 — Close the slice

1. **Confirm no marker names F41.** Only `ebb-kick-avoids-snare`, which is
   B15's, should remain.
2. **Tick both definition-of-done boxes** in the slice and this plan, tick Task
   3, confirm F41 is `done`, set the slice `Status` to `done`.
3. **Run every token**, ending with `bash scripts/site-verify-local.sh`. Restore
   the two WASM artifacts afterwards and stage explicit paths.
4. **Run `jk-standards ledger`** with the slice `done`.
5. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (pokok layer) → Task 2. Item 2 (structural overlap) →
Task 1, which corrects the rule, plus Task 1 step 3, which establishes that the
patch already satisfies it as the construction defines it.

**Row coverage.** F41 → Task 2 step 6; its verification is both `gam-` rules,
one corrected in Task 1 and one already correct.

**Placeholder scan.** No TBDs. The pokok lane's values are derived from
construction step 2 and from the note numbers already in the table.

**Name consistency.** `gam-pokok-layer` and `gam-structural-overlap` throughout,
matching S05's ids.

**Ordering.** Task 1 must precede Task 2: until the Rule 4 predicate is
corrected, the suite cannot go green on this page whatever Task 2 does.
