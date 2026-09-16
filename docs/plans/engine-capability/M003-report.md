---
class: gated
---

# M003 — Review report

Status: current (2026-09-15)

Generated from `docs/plans/engine-capability/ledger.md`, git, and
`M003-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** Poly plays the uneven subdivisions the guide describes — samba and
jembe feel, and the long beats of additive meters — instead of approximating them
with swing, and the guide stops admitting the approximation.

**Branch:** `milestone/M003-non-isochronous-timing`, cut from `main` at `d025a11`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | Subdivision profiles | EC08 | done |
| M003/S02 | Cell-aware aksak swing | EC09 | done |
| M003/S03 | The measured jembe profile | EC10 | done |
| M003/S04 | The guide catches up | EC11 | done |

## Definition of done

**M003/S01**

- [x] A lane can play a non-isochronous subdivision profile rather than an even grid
- [x] A profile is expressible in a preset and reaches `site/src/generated/presets.json`
- [x] `renderRange()` gains no allocation, lock or blocking call
- [x] The engine builds and passes its tests with no VST3 SDK present

**M003/S02** — the fourth item was added mid-slice, see Decisions

- [x] Swing on an additive meter applies per cell rather than to alternate notes across the bar
- [x] A Balkan preset's long beat carries the feel the guide describes
- [x] `renderRange()` gains no allocation, lock or blocking call
- [x] `theory-balkan` Rule 8 no longer says the long beat is unreproducible on the grid

**M003/S03**

- [x] A measured jembe profile ships as data, with its source cited at a tier the citation check accepts
- [x] `theory-sub-saharan-africa` construction step 5 no longer says Poly ships no measured profile
- [x] The profile's values are reachable from a preset

**M003/S04**

- [x] `theory-brazilian`'s patch table carries a `Timing` column expressing the per-beat profile
- [x] Rule 6 reads `checkable`, with a predicate shown to fail when the profile is flattened
- [x] The "until subdivision profiles ship" sentence is gone from the page

## Validation

Run on the current head.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `unit` | `ctest --test-dir build` | pass, 626 tests |
| `engine-isolation` | `ctest --test-dir build-engine` | pass, 502 tests |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` | pass |
| `site-unit` | `npm --prefix site test` | pass, 287 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |
| — | `jk-standards ledger` | 2 ledgers conform |

## Traceability

Every commit carries a `Slice:` trailer. **No untraced commits.**

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `c72261f` | docs(plans): front-load M003's decisions and design M003/S01 | M003/S01, M003/S02 | — |
| `b65bb62` | docs(plans): plan all four M003 slices | M003/S01, M003/S02, M003/S03, M003/S04 | — |
| `6670eee` | feat(engine): place steps by a fractional subdivision profile | M003/S01 | — |
| `7421c0f` | feat(engine): honour the subdivision profile when placing a lane's steps | M003/S01 | — |
| `225d5a1` | feat(engine): serialize the subdivision profile at state version 20 | M003/S01 | — |
| `8c52f64` | feat(presets): ship the samba long-short-short-long profile | M003/S01 | — |
| `17b2834` | docs(plans): close M003/S01 with the profile shipping in a preset | M003/S01 | EC08 |
| `5c89e6c` | feat(engine): group a lane's steps into cells for feel | M003/S02 | — |
| `74e5dc9` | feat(engine): swing the final subdivision of each aksak cell | M003/S02 | — |
| `5df555d` | feat(presets): give the Balkan long beat the ratio it is played at | M003/S02 | EC09 |
| `37a6708` | feat(presets): ship the measured jembe subdivision | M003/S03 | — |
| `219b098` | docs(site): the jembe profile ships, so the page stops saying it does not | M003/S03 | EC10 |
| `1bcbe1c` | docs(site): Rule 6 states the profile and a predicate checks it | M003/S04 | EC11 |

Thirteen commits, none untraced. Two correct earlier work in this milestone and
say so in their own messages.

## What a reviewer should look at twice

1. **A plan step was wrong, and following it would have put a patch in
   contradiction with its own theory page.** S02's task 3 said to give the
   Balkan zurna "the swing the guide describes". `theory-balkan` Rule 6 is *"No
   swing"*, and its stated reason is the mechanism S02 had just built. The feel
   the guide describes is Rule 8's long beat — the second half of issue #157,
   absent from the plan. The run stopped and asked rather than proceeding.

2. **"Cell-aware swing" was implemented as the wrong rule, and the rachenitsa
   hid it.** Read as odd-parity-within-cell, it displaces steps 1, 3 and 5 of a
   2+2+3 lane — identical to the bar parity it replaced, so the change was a
   no-op against the very meter the slice exists for. Issue #157 says "the final
   subdivision of each 2- or 3-group": 1, 3 and 6. A case on 2+3+2, where the
   two readings diverge, now stops a silent revert.

3. **Two of S01's own tests were vacuous when written.** An even grid also holds
   its ratios across tempi and also does not lengthen the cycle, so both passed
   under a probe that discarded the profile entirely. Only a mutation round
   showed it. This is M007's finding recurring in new code, one milestone later.

4. **S01's normalisation rule had to be revised in S02.** A profile normalised to
   `profileCount × base` collapses the Balkan davul's bar from 3.5 PPQ to 1.5.
   Cells now set the length and the profile the distribution when the counts
   match; mismatched counts leave S01's shipped precedence untouched, so its
   test passes unedited rather than being adjusted to fit.

5. **A lock belonging to another programme had to move.**
   `scope-framing`'s `S04-F32` — a claim the theory-audit ledger put there —
   required a phrase that was the tail of the disclaimer S02 makes false. The
   lock now names the ratio the preset plays. What F32 protects is unchanged;
   what went was guarding a sentence that is no longer true. Worth a reviewer's
   eye precisely because editing another programme's locked claim should never
   be routine.

6. **Checks from three other milestones caught things this one would have
   shipped.** M007's absent-column guard fired the moment S04 added the `Timing`
   column. The `presets.json` schema gate caught a version raised in a comment
   but not in the emitted string. And `presets-json-schema.test.mjs` still
   expected schemaVersion 5 — a consumer S01 missed, because that test runs
   under `site-unit` and S01 owes only engine tokens. A slice's validation set
   bounds what it can discover.

7. **`GrooveState` grew about 30%,** 13776 → 17936 bytes across two slices. Each
   growth was answered with a measurement rather than a new constant: the
   three-copy pipeline went 0.45 → 0.68 µs/block and stayed at 0.02% of a
   128-sample block period. Inside budget, but the trend is obvious in aggregate
   and invisible one commit at a time.

8. **What shipped unused.** The cell-aware swing of S02 has no factory preset
   using it: Rule 6 forbids swing on aksak, so the Balkan preset takes the
   long-beat ratio instead. Both halves of issue #157 are implemented; only one
   has a shipped user.

9. **A hazard that could have hidden any of the above.** After restoring a
   mutation probe, `ctest` reported a failure against sources `git diff` showed
   were correct — the header edit had not been recompiled. The false-red
   direction is self-correcting. The opposite direction would certify a vacuous
   test as proved, which is exactly what item 3 was.

## Decisions

Verbatim from `M003-decisions.md`.

## 2026-09-15 — planning M003 (all four slices, front-loaded)

- **Q:** How should a subdivision profile be represented in the engine? —
  **A:** Fractional cell durations.
- **Decision:** A per-step array of fractional step durations on `LaneConfig`,
  fed through the existing `computeAdditiveCells`/`cumPpq` path. — **Why:**
  `cellSizes[]` is `int`, so it cannot express samba's long-short-short-long at
  all. Making the durations fractional generalises the mechanism that already
  places aksak cells rather than adding a second one beside it: today's `{2,2,3}`
  becomes the integer special case of one code path. It is tempo-relative by
  construction, which is what Polak's measured ratios actually are, and note
  durations follow the profile because `stepDurPpq` is derived from the same
  array. The rejected alternative — fractional *onset offsets*, a tempo-relative
  `microTimingMs` — moves onsets without changing durations, so a long note
  would still occupy an even cell.

- **Q:** Should a profile be editable from the WebUI and automation, or
  state-only? — **A:** State-only, set from presets.
- **Decision:** The array lives in `LaneConfig` and serialised state; a named
  catalogue in `presets.cpp` supplies the shipped profiles. No new VST3
  parameters, no bridge-schema work. — **Why:** 64 values per lane is not a
  parameter family — the expression family is full at `kParamsPerLane == 16` and
  the core family sits at 14 — and the WebUI's lane edit path is parameter-ID
  based, so state-only and WebUI-editable cannot both hold. M002 asserted both
  for the kotekan fields and the measurement that would have caught it looked at
  only one of the two families. Nothing in M003's deliverable needs the editing
  surface: the shipped profiles reach users through presets.
- **Decision:** File the editing work rather than drop it. — **Why:** Deferred
  with a reason is recoverable; dropped is not.
  [#305](https://github.com/JimAKennedy/poly/issues/305) carries the scope, the
  bridge-schema route it needs, and why it is a milestone rather than a slice.

- **Q:** The ledger says `M003/S02` depends on nothing, but an aksak lane has
  exactly one step per cell, so its definition-of-done clause "the long cell's
  internal division differs from the short cells'" has nothing to divide. How
  should that be resolved? — **A:** Depend on S01 and use the profile.
- **Decision:** `M003/S02` gains `**Depends:** M003/S01`, and the ledger's
  Sequencing section records why. — **Why:** `prepareLaneContext` sets
  `stepsInCycle = additive.count` for an additive lane, so a `{2,2,3}` davul has
  three steps, one per cell, and `applyTimingShifts`' `(cycleStep % 2) == 1`
  swings whole cells rather than notes. Fixing that predicate alone satisfies
  the slice's first clause and leaves the second unsatisfiable. The rejected
  alternative — building sub-cell placement inside S02 — would be a second
  mechanism for one problem, which is the outcome this programme has repeatedly
  found expensive. The dependency was missing from the ledger, not invented
  here; the correction is recorded rather than quietly applied.

- **Q:** The guide says the jembe profile goes in Poly's per-step micro-timing
  offsets, but those are absolute milliseconds clamped to ±20 ms while the same
  paragraph calls the profile "particular ratios, stable across tempi". Which
  mechanism should `EC10`'s data use? — **A:** The new profile, and correct the
  page.
- **Decision:** Author Polak's ratios into the subdivision profile, and rewrite
  `theory-sub-saharan-africa` construction step 5. — **Why:** The page points
  readers at a mechanism that cannot hold what the page itself describes: at
  90 BPM a sixteenth is about 167 ms, so a ±20 ms absolute offset expresses a
  ratio only at the tempo it was measured at, and stops doing so the moment the
  tempo moves. That is the distinction the paragraph is otherwise careful about
  — systematic profile versus random jitter — applied one step further.
- **Note:** `fr-polak-2010` is already in the bibliography at tier A, described
  as "measured microtiming showing jembe subdivision is systematically uneven",
  so `EC10`'s citation requirement needs no new entry — only the citation.

## 2026-09-15 — executing M003/S01 task 1 (judgment call)

- **Finding:** `GrooveStateCopyBenchmark.ReportsFactSizes` failed on the field
  addition. `GrooveState` grows 13776 → 15888 bytes, +2112 — 264 bytes per lane
  across 8 lanes, from a 256-byte `std::array<float, kMaxSteps>` plus an `int`.
  This is the largest single growth the struct has taken; the three previous
  entries in that test's comment are +32, +96 and +64.
- **Decision:** Accept the growth, and record the measurement rather than
  bumping the number. — **Why:** The guard exists to force a conscious decision,
  so satisfying it with a new constant and nothing else would defeat it. The
  three-copy pipeline was measured on the grown struct: **0.45 µs/block, 0.02%
  of a 128-sample block period**, and `ThreeCopyPipelineFitsBlockBudget` passes.
  The copy budget is not the constraint at this size.
- **Decision:** The array is `kMaxSteps` wide rather than something smaller. —
  **Why:** The profile is per-step and `kMaxSteps` is the step bound, which is
  exactly why `cellSizes` beside it is already `std::array<int, kMaxSteps>`.
  Sizing it to a typical profile instead would make the one case it could not
  hold a silent truncation.

- **Finding:** The first run of `SubdivisionProfile.PlacesStepsInTheStatedProportions`
  failed at a tolerance of `1e-9`: `1.1f / 0.9f` differs from the double ratio
  by about 6e-8.
- **Decision:** Loosen the tolerance to `1e-6` and say why in the test. — **Why:**
  The engine was right and the test was wrong. The profile is stored as `float`
  by design — matching every other `LaneConfig` float — so a tolerance tighter
  than float precision tests the storage type rather than the arithmetic.

## 2026-09-15 — executing M003/S01 task 2 (judgment call)

- **Finding:** Two of task 2's three render-level cases passed under a probe
  that discarded the profile entirely — they were vacuous. An even grid holds
  its ratios across tempi and does not lengthen the cycle, so neither case
  discriminated.
- **Decision:** Strengthen both to assert unevenness before asserting the
  property, and re-run the probe until all three fail. — **Why:** This is M007's
  finding recurring in new code. The cost of not catching it is a suite that
  reports a capability it never exercises.
- **Finding:** `maxStepDur` reads `cfg.cellSizes[c]` on a path where
  `cellSizes` is unset, leaving the lookahead bound at the base step while a
  profiled step can be longer.
- **Decision:** Derive the bound from `cumPpq`, and record that the test written
  for it does not catch it. — **Why:** `SplitRenderingMatchesWholeRendering`
  passes before and after, because the additive path enumerates whole cycles
  with a one-cycle margin that swamps a 0.0008 PPQ shortfall. The bound is still
  wrong as written and worth fixing; calling the test a proof of the fix would
  be a claim the run did not earn.

## 2026-09-15 — executing M003/S01 task 3 (a hazard worth recording)

- **Finding:** After restoring a mutation probe in
  `engine/include/poly/state_io_read_lane.h`, `ctest` reported a failure against
  sources that `git diff` showed were correct. The header edit had not been
  recompiled; `touch` on the headers plus a rebuild turned it green.
- **Decision:** Treat a `ctest` result following a header-only edit as
  untrustworthy unless the build actually recompiled, and rebuild explicitly
  before reading any mutation result. — **Why:** The direction that bit here was
  a false red, which is noisy but self-correcting. The dangerous direction is
  the opposite: a probe that should have failed reading as passed, which would
  certify a vacuous test as proved. Given this milestone has already found two
  vacuous predicates by mutation, a stale build would have hidden them.

## 2026-09-15 — executing M003/S02 task 1 (judgment call)

- **Finding:** The size guard fired again. `GrooveState` grows 15888 → 17936,
  +2048, from `swingCellSizes` (256 bytes) plus `swingCellCount` per lane.
  Across M003 the struct is up about 30% and the three-copy pipeline from 0.45
  to 0.68 µs/block.
- **Decision:** Accept it, with the measurement in the guard's comment. — **Why:**
  0.68 µs is 0.02% of a 128-sample block period, unchanged in percentage terms
  from before the milestone. The budget is not the constraint. The trend is
  recorded because two slices adding 30% is the sort of thing that is obvious in
  aggregate and invisible one commit at a time.
- **Decision:** `swingCellSizes` is a new array rather than a reuse of
  `cellSizes` when `cellCount == 0`. — **Why:** Reuse would save 2 KB and give
  one field two meanings depending on the value of another — precisely what this
  slice's sibling decision rejected when it gave the profile explicit precedence
  over `cellSizes` rather than combining them. A reader who has to consult a
  second field to know what the first one means is the cost, and it is paid on
  every future read of the timing path.

## 2026-09-15 — executing M003/S02 task 2 (a wrong reading, caught by a test)

- **Finding:** "Cell-aware swing" was implemented as odd-parity-within-cell. On
  2+2+3 that is identical to the bar parity it replaced — steps 1, 3, 5 — so the
  change was a no-op against the rachenitsa, the very meter the slice is about.
  Issue #157 says "offset the final subdivision of each 2- or 3-group", which
  gives 1, 3, 6.
- **Decision:** Implement the tail rule, and add a case on 2+3+2 where the two
  readings diverge. — **Why:** Every other case in the file passes under either
  reading. Without a divergence case, a future revert to parity-within-cell
  would look correct.

## 2026-09-15 — executing M003/S02 task 3 (a wrong plan step, and two questions)

- **Finding:** The plan's task 3 said to give the Balkan zurna "the swing the
  guide describes". `theory-balkan` Rule 6 is "No swing", and its reason is the
  mechanism task 2 built. The feel the guide describes is Rule 8's long beat —
  the second half of issue #157, absent from the plan.
- **Q:** How should S02 satisfy "a Balkan preset's long beat carries the feel the
  guide describes"? — **A:** Long-beat ratio via the profile, swing stays 0.
- **Decision:** The davul and rim carry `{2.0, 2.0, 2.85}` over their `{2,2,3}`
  cells. — **Why:** Rule 6 stands untouched and the patch stops contradicting
  its own page, which is the defect class the theory-audit programme spent seven
  milestones removing. The cell-aware swing task 2 built therefore ships with no
  factory preset using it; recorded rather than hidden, and issue #157 wanted
  both halves.
- **Q:** Rule 8's "until Poly exposes a long-beat ratio control" disclaimer is
  now false. — **A:** Remove it in this slice.
- **Decision:** Rule 8 states what the engine does and what the preset plays;
  the slice gains a fourth definition-of-done item rather than the edit being
  slipped in under the existing three. — **Why:** A slice that widens should say
  so in the place the widening is checked.

- **Q:** A profile normalised to `profileCount × base` collapses the davul's bar
  from 3.5 PPQ to 1.5, and M003/S01 gave the profile precedence over
  `cellSizes`. — **A:** Cells set length, profile sets distribution.
- **Decision:** When the counts match, normalise to the cells' total; when they
  disagree, the profile governs alone. — **Why:** It is what S01's own design
  says the two fields are — structure and feel — and composing them is more
  faithful to that than competing. The mismatch branch preserves S01's shipped
  precedence exactly, so its test passes untouched rather than being edited to
  fit.

- **Finding:** `site/tests/scope-framing.test.mjs`'s `S04-F32` — a claim locked
  by the *theory-audit* programme's ledger — requires the phrase "the grid
  version is the", which is the tail of the disclaimer this slice deletes.
- **Decision:** Update the lock to the ratio the preset plays rather than
  loosening it. — **Why:** What F32 protects is that the long beat is measurably
  under 3:2, style-defining, cited to Goldberg; all of that survives. The phrase
  that went was protecting a sentence that is no longer true. Editing another
  programme's locked claim is recorded rather than done quietly.
- **Finding:** `presets-json-schema.test.mjs` still asserted `schemaVersion === 5`
  after M003/S01 raised it to 6 — a consumer S01 missed, because that test runs
  under `site-unit` and S01 owes only engine tokens.
- **Decision:** Fix it here, and note the gap. — **Why:** A slice's validation
  set bounds what it can discover. S01's set was right for its scope and still
  could not see this.
