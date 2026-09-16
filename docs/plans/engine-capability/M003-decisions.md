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
