# M004/S05 — named-rule conformance checklist

**Slice:** M004/S05, in `docs/plans/theory-audit/ledger.md`
**Design:** `M004-S05-design.md`, approved before this plan was written
**Decisions:** `M004-decisions.md`

S05 runs first in M004. It builds the machinery its sibling slices are verified
by; S01, S02 and S03 depend on it. Read the design first — it says why the
marker takes a rule id, why the nine existing tests are left alone, and the
three ways this mechanism could rot.

## Task status

- [x] Task 1 — Reach every patch in a file, not just the first
- [ ] Task 2 — The checklist and the divergence marker, proved on two theory pages
- [ ] Task 3 — Chapter entries, with markers naming the rows that own the gaps
- [ ] Task 4 — Close the slice and run the shipping gate

## Definition of Done

- [ ] `theory-patch-conformance.test.mjs` carries a per-page named-rule
      checklist declared as data, covering the Chapter 2, 3 and 5 patches its
      sibling slices need and the two theory pages that carry no assertion at
      all today, `theory-electronic-breakbeat` and `theory-minimalism`
- [ ] A rule a patch deliberately breaks is satisfied by an in-band divergence
      marker carrying a written reason, and by nothing else
- [ ] A patch that silently drops a rule fails the suite
- [ ] Every rule in the checklist is proved to fail when its lane is removed
- [ ] The remaining rule coverage is owned by M007, named here, not left
      implicit

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `jk-standards all` |
| `gate` | `bash scripts/pre-push-check.sh` |

`.jk/validations.yml` is authoritative if this table ever disagrees with it.
`site-unit` is `npm --prefix site test`; `node --test site/tests/` collects one
file from the repo root and exits 1.

## The state this slice starts from

Measured, not assumed — re-derive rather than trusting these numbers if the
tree has moved:

- `site/tests/theory-patch-conformance.test.mjs` is 295 lines with **nine**
  tests, one per theory page, each encoding one defect from the 2026-07-30
  conformance review. **All nine pass.**
- Eleven theory pages state **92** numbered rules between them.
- `theory-electronic-breakbeat` (9 rules) and `theory-minimalism` (8 rules)
  carry **no** assertion.
- `parsePolyPatch(src)` returns `{ columns, rows }` and reads the **first**
  `<PolyPatch>` table only. Helpers already present: `laneOnsets`,
  `laneOnPulseGrid(row, N)`, `findLane(rows, re)`, `laneTag(relPath, row)`.

## How the chapter entries end green

S05 adds the checklist entries; S01, S02 and S03 make the failing ones pass and
close their rows. An entry that fails today therefore ships with a marker whose
reason **names the row that owns the gap**:

```mdx
{/* patch-divergence-ok: ssa-construction-2 — M004/S01 adds this lane; ledger
    row F37 owns it. */}
```

That is a handoff, not a deliberate divergence, and the plan for each sibling
slice must say so: **removing its marker is part of making its rule pass.** A
marker naming an open row is a promise; one left standing after that row closes
is exactly the rot M007/S03 exists to catch.

Which entries need one is determined by running them, not by this table — but
as measured while planning: Chapter 2's dance-beat rule fails (F37), Chapter
3's clave-header rule fails (F38), Chapter 5's pokok and overlap rules fail
(F41), and Chapter 3's one-free-voice rule **passes** (F40 — all five lanes of
`Cuban Son Montuno` carry `mutationRate` 0.00).

---

## Task 1 — Reach every patch in a file, not just the first

Produces the multi-patch parsing Tasks 2 and 3 depend on. Closes no row.

**Files:** `site/tests/theory-patch-conformance.test.mjs`

Chapter 2 has two patches (Ewe, Manding) and Chapter 5 has two (Balinese
kotekan, Javanese colotomy). `parsePolyPatch` reads the first table in the
file, so the second of each is unreachable.

1. **Write the failing test.** Add a case named `S05-parse-by-title` asserting
   that parsing `05-gamelan.mdx` selecting `Javanese Colotomic Nesting` returns
   a different lane set than selecting `Balinese Kotekan Interlocking` — assert
   on a specific difference, such as the Role cell of lane 1, read from the
   file rather than written into the test.

2. **Run it and watch it fail** with a TypeError or a wrong-table assertion,
   because the selector argument does not exist yet.

3. **Extend the signature** to `parsePolyPatch(src, title)`. When `title` is
   omitted the behaviour must be byte-identical to today, because the nine
   existing tests call it that way and this task must not touch them. Scan
   forward from `<PolyPatch title="<title>"` to the closing `</PolyPatch>` and
   parse the table inside that window.

4. **Run it and watch it pass**, then run the whole file and confirm the nine
   existing tests still pass. If any of them changed behaviour, the default
   path was not preserved — fix that rather than editing the nine.

5. **Check.** `npm --prefix site test`, `bash scripts/check-doc-conformance.sh`,
   `pre-commit run --all-files`.

6. **Commit** with `Rows: —` (this task closes none), ticking Task 1, and
   appending the gate results to
   `docs/plans/theory-audit/evidence/M004-S05.md`. Check
   `git log --grep="Slice: M003/S05"` for the form the programme's
   no-rows commits use.

---

## Task 2 — The checklist and the divergence marker, proved on two theory pages

Consumes Task 1's parser. Produces the mechanism Task 3 populates.

**Files:** `site/tests/theory-patch-conformance.test.mjs`

Seed the checklist with the two pages that carry no assertion today and whose
rules pass, so the mechanism is proved on real rules before it is asked to
carry a failing one.

1. **Write the failing tests.** Add the `CHECKLIST` structure and its
   iteration, with five entries — each `{ id, description, predicate }`, the
   `id` descriptive rather than positional:

   | Page | Rule id | What it asserts |
   |---|---|---|
   | `theory-minimalism.mdx` | `min-one-variable` | exactly one lane has non-zero Drift (Rule 1) |
   | `theory-minimalism.mdx` | `min-voices-flat` | the fixed and phasing voices share a Velocity (Rule 4) |
   | `theory-minimalism.mdx` | `min-deterministic` | every lane's Mutation is 0% (Rule 8) |
   | `theory-electronic-breakbeat.mdx` | `ebb-anchor-immutable` | the fixed snare lane's Mutation is 0% (Rule 1) |
   | `theory-electronic-breakbeat.mdx` | `ebb-kick-avoids-snare` | kick and snare onsets do not intersect on a 16-pulse grid (Rule 7) |

   Use `laneOnPulseGrid(row, 16)` for `ebb-kick-avoids-snare` — the helper
   exists and already does this for the existing breakbeat-adjacent tests.

   The iteration must **fail on a page that has an entry but no matching
   patch**, and must **name the page, the patch title and the rule id** in
   every failure message, so a violation identifies itself without a debugger.

2. **Run them and watch them fail.** They cannot fail on the rules — those
   rules hold today — so they must fail on the machinery not existing. That is
   the honest red for this task: the checklist is the unit under test, not the
   pages. Confirm the failure is a missing `CHECKLIST`/iteration, not a
   predicate returning false.

3. **Implement the iteration** so all five pass.

4. **Add the marker parser and its counter.** A marker is an MDX comment
   `{/* patch-divergence-ok: <rule-id> — <reason> */}` in the source, and it
   satisfies **only** the rule whose id it names, on **only** the patch it
   precedes. Print the live marker total in the suite's output, the way M002's
   `citation-tier-ok` count is printed. With no markers in the tree yet the
   count is zero, and that zero is worth printing: it is the baseline the
   number is later read against.

5. **Prove all five rules bite.** For each, mutate the page — change the
   phasing voice's Drift to 0, change a Velocity, set a Mutation non-zero,
   rotate the kick onto a snare slot — run the case, watch it fail naming that
   rule id, and restore by **inverse edit**. Never `git checkout --`, which has
   destroyed uncommitted work in this programme. Confirm each restore with
   `git diff --stat`. Record all five failures in the evidence file: this is
   the fourth definition-of-done item, and without it a predicate that cannot
   bite is indistinguishable from one that can.

6. **Prove the marker works and is narrow.** Add a marker for
   `min-deterministic` to `theory-minimalism.mdx`, set a lane's Mutation
   non-zero, and confirm the case passes and the printed count is 1. Then check
   the marker does **not** rescue `min-voices-flat`: with the same marker in
   place, break the velocity rule and confirm it still fails. Remove both edits
   by inverse edit. A marker that excuses a whole page rather than one rule
   would pass this second check, which is why it is worth running.

7. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

8. **Commit** with `Rows: —`, ticking Task 2, appending the evidence.

---

## Task 3 — Chapter entries, with markers naming the rows that own the gaps

Consumes Tasks 1 and 2. Closes **F42**. Produces the entries S01, S02 and S03
are verified by.

**Files:** `site/tests/theory-patch-conformance.test.mjs`,
`site/src/content/docs/02-sub-saharan-africa.mdx`,
`site/src/content/docs/03-afro-cuban.mdx`,
`site/src/content/docs/05-gamelan.mdx`

Add checklist entries for the three chapter patches. **Change no patch.**
Correcting them is S01, S02 and S03's work; this task only makes the gaps
visible and hands each one to its owner.

1. **Write the entries**, deriving each rule from the companion theory page
   rather than from this plan:

   | Page | Patch | Rule id | From |
   |---|---|---|---|
   | `02-sub-saharan-africa.mdx` | Ewe-Inspired Polymetric Ensemble | `ssa-dance-beat` | `theory-sub-saharan-africa` construction step 2: a low drum at 12 steps, 4 hits |
   | `02-sub-saharan-africa.mdx` | Ewe-Inspired Polymetric Ensemble | `ssa-timeline-legible` | the table renders a `Timeline` column, so Rule 1's mode is visible |
   | `03-afro-cuban.mdx` | Cuban Son Ensemble | `ac-clave-approximation` | F38: the clave lane header marks itself the E(5,16) approximation and links the exact-timeline construction |
   | `03-afro-cuban.mdx` | Cuban Son Ensemble | `ac-one-free-voice` | `theory-afro-cuban` "One free voice": at most one lane carries a non-zero variation budget |
   | `05-gamelan.mdx` | Balinese Kotekan Interlocking | `gam-pokok-layer` | `theory-gamelan` Rule 6: the interlock serves a pokok melody |
   | `05-gamelan.mdx` | Balinese Kotekan Interlocking | `gam-structural-overlap` | `theory-gamelan` Rule 4: the parts overlap at structural tones |

   `ac-one-free-voice` cannot read the table — Chapter 3's patch has no
   Mutation column. Read `mutationRate` from the `Cuban Son Montuno` record in
   `site/src/generated/presets.json`, the copy
   `preset-table-conformance.test.mjs` imports. Assert against the preset's
   values, never against numbers copied from this plan.

2. **Run them and read which fail.** As measured while planning,
   `ac-one-free-voice` should pass and the other five should fail — but treat
   that as a prediction to check, not a fact to assume. If `ac-one-free-voice`
   fails, stop: the preset has changed since planning and F40's amendment needs
   revisiting rather than working around.

3. **Add a marker for each failing rule**, in the source above the patch it
   concerns, naming the row that owns it: F37 for the two `ssa-` rules, F38 for
   `ac-clave-approximation`, F41 for the two `gam-` rules. Write the reason as
   a handoff — which slice adds what — not as a justification for the gap.

4. **Run and watch the suite go green**, with the printed marker count now 5.

5. **Prove the passing rule bites.** `ac-one-free-voice` carries no marker, so
   it is the one entry here that must be shown to fail on its own terms: raise
   a second lane's `mutationRate` in `presets.json`, watch it fail naming the
   rule, restore by inverse edit, confirm with `git diff --stat`. The five
   marked rules were already proved to fail — that is why they are marked — and
   the evidence file should say so rather than repeat the exercise.

6. **Close F42.** Set it `done`, name the checklist and the marker contract in
   its `Verification`, and record in its `Item` that the coverage is the five
   patches plus two theory pages, with the rest owned by M007.

7. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

8. **Commit** with `Rows: F42`, ticking Task 3, appending the evidence.

---

## Task 4 — Close the slice and run the shipping gate

Closes no row; closes the slice.

**Files:** `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M004-S05.md`, this plan

1. **Confirm the marker inventory.** `grep -rn patch-divergence-ok site/` must
   list exactly the five markers Task 3 added, each naming an open ledger row
   (F37, F38, F41). If it lists a marker naming a closed row, or one with the
   untriaged placeholder reason, stop and report — the first is rot and the
   second means an untriaged finding was never given its B row.

2. **Tick the five definition-of-done boxes** in the slice and in this plan's
   copy, tick Task 4, confirm F42 is `done`, and set the slice `Status` to
   `done`.

3. **Run every token the slice declares**, in this order:
   `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `jk-standards all`, then
   `bash scripts/pre-push-check.sh`. `gate` is the full pre-push suite
   including the native build and `ctest`; read its exit code, not its
   narrative. Note that `gate` does not subsume the others: its clang-format
   stage is not `pre-commit run --all-files`, and nothing in it runs the site
   suite (poly issue #272).

4. **Run `jk-standards ledger`.** With the slice `done` it enforces the
   stricter claim: every DoD box ticked, evidence present, every row closed.

5. **Append the final evidence entry**, naming each token and what it returned.
   Do not name a commit SHA — this file ships inside the commit it describes.

6. **Commit** with `Rows: —`, and say in the message which slice runs next:
   S01, S02 and S03 are now unblocked, and each must remove its own marker.

---

## Self-review

**DoD coverage.** Item 1 (checklist covering three chapter patches and the two
uncovered theory pages) → Tasks 2 and 3. Item 2 (marker satisfies a rule and
nothing else) → Task 2 step 6, which checks both that it works and that it does
not over-reach. Item 3 (a silently dropped rule fails) → Task 2 step 5 and Task
3 step 5. Item 4 (every rule proved to fail) → Task 2 step 5 for the five
theory rules, Task 3 step 5 for `ac-one-free-voice`, and Task 3 step 3 for the
five marked rules, whose failure is what makes the marker necessary. Item 5
(M007 named) → satisfied by the ledger, which M007 already occupies; Task 3
step 6 records it in F42's `Item`.

**Row coverage.** F42 → Task 3, closed at step 6, verification produced by the
checklist and marker contract Tasks 2 and 3 build. Tasks 1, 2 and 4 close no
row, which is why each carries `Rows: —`.

**Placeholder scan.** No TBDs, no "similar to task N". `parsePolyPatch`,
`laneOnsets`, `laneOnPulseGrid`, `findLane` and `laneTag` are existing
functions in the suite; `CHECKLIST` and the marker parser are defined by Task
2. No task references a helper no task defines.

**Name consistency.** Rule ids are `min-one-variable`, `min-voices-flat`,
`min-deterministic`, `ebb-anchor-immutable`, `ebb-kick-avoids-snare`,
`ssa-dance-beat`, `ssa-timeline-legible`, `ac-clave-approximation`,
`ac-one-free-voice`, `gam-pokok-layer`, `gam-structural-overlap` — eleven,
used identically in every mention. The marker token is `patch-divergence-ok`
throughout. The parse case is `S05-parse-by-title`.

**Ordering.** Task 1 must precede Tasks 2 and 3 because Chapter 5's Balinese
patch is the first of two and Chapter 2's Ewe patch is the first of two, so the
selector is needed to address them unambiguously even where the first happens
to be the target. Task 2 must precede Task 3 because Task 3's failing entries
need the marker to exist. Task 4 is last: it runs `gate` over the finished
tree.
