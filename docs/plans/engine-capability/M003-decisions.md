---
class: gated
---

# M003 — Decisions

Append-only. One entry per decision, with the reason, so a later reader does not
have to reconstruct it from the diff.

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
