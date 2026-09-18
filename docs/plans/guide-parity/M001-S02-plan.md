---
class: gated
---

# M001/S02 — Humanize drifts rather than jitters

**Slice:** `M001/S02` in `docs/plans/guide-parity/ledger.md`
**Classification:** bounded — a change to the noise source behind `humanizeMs`,
which `applyTimingShifts` already applies. Design approved in chat 2026-09-18;
see `M001-decisions.md`.

## Task status

- [x] 1. A correlated noise source, proved correlated
- [ ] 2. Humanize reads it, and a transport jump reproduces it
- [ ] 3. Two pages stop calling it random
- [ ] 4. Evidence and close-out

## Definition of Done

Copied verbatim from the slice, **except the last item**, which the ledger
states as "the pages recommending Humanize". Fourteen pages mention Humanize;
two describe its mechanism, and neither is false today — only imprecise. The
item is rescoped to those two, with the finding recorded in `M001-decisions.md`
so the small edit is not mistaken for an oversight.

- [ ] Successive humanize offsets on one lane are correlated rather than independent, asserted as a measurable property of the sequence rather than by eye
- [ ] The offsets remain a pure function of absolute step index — a locate or loop reproduces them exactly
- [ ] With the correlated mode off, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `renderRange()` gains no allocation, lock or blocking call
- [ ] `appendix-design-decisions.mdx` and `18-editors-and-views.mdx` name the
      correlated behaviour rather than calling it random displacement, each
      locked by a `scope-framing` claim

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Task 1 — A correlated noise source, proved correlated

Modifies `engine/include/poly/rng.h`; creates `tests/correlated_noise_tests.cpp`
and registers it in `tests/CMakeLists.txt`.

1. Write the failing tests first, against a `correlatedNoise(seed, laneId,
   absStep)` helper that does not exist yet. The cases must pin *correlation*,
   which is the whole point and the thing an eyeball cannot check:
   - **successive values are correlated**: over a long run, the mean absolute
     difference between consecutive samples is materially smaller than between
     samples drawn far apart. White noise gives roughly the same figure for
     both, so this is the case that distinguishes the new source from the old.
   - the output stays within [-1, 1], so it can scale `humanizeMs` the way the
     current draw does
   - it is a pure function of `absStep`: the same index yields the same value,
     asked twice
   - two lanes with the same seed but different `laneId` do not move together
2. Run them and watch them fail to compile, then implement: a sum of a few
   seeded hash octaves over the absolute step index, per issue #151's own
   proposal. Pure arithmetic, no allocation, no state between calls — it runs on
   the audio thread and must survive a locate.
3. **Prove the correlation case bites** by pointing the helper at the existing
   white-noise `deterministicRand` and watching it fail. A correlation test that
   passes for white noise is testing nothing, and this milestone's own decisions
   file records M003 shipping two such tests.
4. Run `format`, `unit`, `engine-isolation`. Commit; `GP02` stays open.

## Task 2 — Humanize reads it, and a transport jump reproduces it

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`.

1. Write the failing render-level cases first:
   - a lane with the correlated mode on emits humanized onsets whose successive
     displacements correlate, measured through the public render path rather
     than the helper
   - **the same passage rendered after a locate is identical**, which is the
     determinism requirement #151 states and the property an accumulating
     implementation would break
   - a lane with the mode off is byte-identical to the pre-change engine
2. Add `HumanizeMode { WhiteNoise = 0, Correlated = 1 }` and a field on
   `LaneConfig` beside `humanizeMs`, defaulting to `WhiteNoise`. Carry it in the
   same `kStateVersion` bump as `M001/S01`'s `swingMode` — one version for the
   milestone, not one per slice.
3. Branch in `applyTimingShifts` on the mode. The existing call is
   `deterministicRand(laneEffectiveSeed(cfg, state.seed), cfg.id, absStep, 3)`;
   the correlated path substitutes the new helper at the same site with the same
   inputs, so nothing else about humanize changes.
4. Add the byte-identity golden over the factory presets with the mode off.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 3 — Two pages stop calling it random

Modifies `site/src/content/docs/appendix-design-decisions.mdx`,
`site/src/content/docs/18-editors-and-views.mdx`, and
`site/tests/scope-framing.test.mjs`.

1. Write both failing claims first. Neither page is *wrong* today, so each claim
   must assert the new, more precise phrasing is present rather than that a
   false one is absent — state that in the claim's `rule` text, because a
   reviewer will otherwise wonder what was being corrected.
2. `appendix-design-decisions.mdx` lists "humanize jitter" among the engine's
   deterministic randomness sources. That stays true; it gains the distinction
   between the two modes.
3. `18-editors-and-views.mdx` contrasts micro-timing with "Humanize, which adds
   a different random displacement on every cycle". Still true of the default
   mode; it now says which mode it means, so the contrast survives the feature
   rather than being quietly outgrown by it.
4. Watch each claim fail with the old wording, restoring **by explicit edit**.
5. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit.

## Task 4 — Evidence and close-out

1. Append `evidence/M001-S02.md`: each token's result with headline counts, the
   white-noise mutation result from task 1, the post-locate reproduction from
   task 2, and the byte-identity golden.
2. Tick the four task boxes and the definition-of-done boxes, set `GP02` and the
   slice `done`.
3. Run `jk-standards ledger`, then the whole validation set once more on the
   final tree. Commit with `Slice: M001/S02` and `Rows: GP02`.
