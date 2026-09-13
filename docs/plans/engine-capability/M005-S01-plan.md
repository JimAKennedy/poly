---
class: gated
---

# M005/S01 — The generator rebuilds its emitter

**Slice:** M005/S01 — `docs/plans/engine-capability/ledger.md`
**Rows:** PIPE01
**Classification:** bounded. One early return is removed and one assertion is
retargeted; no mechanism is introduced.

## Task status

- [x] 1. Make the generator always build its emitter
- [ ] 2. Derive the schema test's preset count from the engine header, and close
      the slice

## Definition of Done

- [ ] Editing `engine/src/presets.cpp` and running the generator produces JSON
      that reflects the edit, with no explicit build step
- [ ] A stale `presets.json` fails the site suite mechanically, rather than
      depending on someone noticing the count is wrong
- [ ] The generator still succeeds from a clean tree, where the build directory
      does not yet exist
- [ ] The hardcoded preset count in `presets-json-schema.test.mjs` is gone,
      derived from `kFactoryPresetCount` instead

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |

## Context both tasks need

- **The defect.** `ensureEmitter()` in `site/scripts/generate-presets-json.mjs`
  opens with `if (existsSync(EMITTER)) return;`. It builds the emitter only when
  the binary is missing, so a change to `engine/src/presets.cpp` leaves a stale
  binary in place and the generator emits the previous engine's data while
  reporting success.
- **It has already cost.** M001/S02 added a preset and the generator wrote 43
  after the engine had 44. The file's own header calls a stale `presets.json`
  "a silent correctness bug we already paid for".
- **The fix is to delete the early return**, not to add staleness detection.
  `cmake --build --target poly_presets_emit` is incremental: on an unchanged
  tree it is a no-op costing a fraction of a second, which is cheaper than any
  mtime comparison this script could implement correctly.
- **Do not remove the configure branch.** The `CMakeCache.txt` check below the
  early return is what makes a clean tree work, and the definition of done
  requires that path to keep working.

## Task 1 — Make the generator always build its emitter

**Files:** `site/scripts/generate-presets-json.mjs`

1. **Reproduce the defect first**, so the fix is measured against an observed
   failure rather than an assumed one. Append a temporary sixth lane or change
   a lane's `baseVelocity` in `makeFourOnTheFloor` in `engine/src/presets.cpp`,
   then run `npm --prefix site run generate-presets` with no explicit build.
   Confirm `site/src/generated/presets.json` does **not** carry the change, and
   record what you changed so it can be reverted exactly.
2. Delete the `if (existsSync(EMITTER)) return;` line from `ensureEmitter()`, so
   the function always reaches the `cmake --build` call. Leave the
   `CMakeCache.txt` configure branch and the post-build existence check exactly
   as they are. Replace the removed line with a comment saying why the build is
   unconditional: the early return made a stale binary indistinguishable from a
   current one, and cmake's own incrementality is the thing that makes an
   unconditional build cheap.
3. Re-run the generator with no explicit build. Confirm the JSON now carries the
   change from step 1.
4. Revert the `presets.cpp` edit from step 1, re-run the generator, and confirm
   `git diff --quiet site/src/generated/presets.json engine/src/presets.cpp`.
5. **Prove the clean-tree path still works.** Move `build-presets/` aside
   (`mv build-presets build-presets.bak`), run the generator, and confirm it
   configures, builds, and emits successfully. Then remove the directory it
   created and restore the original (`rm -rf build-presets && mv
   build-presets.bak build-presets`), so the task leaves no stray tree.
6. Run `format` and `site-unit`. Append evidence to
   `docs/plans/engine-capability/evidence/M005-S01.md`, tick task 1, run
   `jk-standards ledger`, and commit with the slice's trailers.

## Task 2 — Derive the schema test's preset count from the engine header

The hardcoded count in `site/tests/presets-json-schema.test.mjs` was hand-edited
twice during M001/S02, once per preset added. Deriving it removes that chore and
makes a count mismatch — the exact shape of the M001 failure — fail the suite.

**Files:** `site/tests/presets-json-schema.test.mjs`,
`docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M005-S01.md`

1. In the test file, read `engine/include/poly/presets.h` and extract the
   integer from `static constexpr int kFactoryPresetCount = N;` with a regex.
   Fail the test with a clear message if the pattern does not match, so a
   rename of the constant surfaces as a test failure rather than as a silently
   skipped assertion — a regex that matches nothing must not read as a pass.
2. Replace the two hardcoded `45` occurrences — the `assert.equal` argument and
   the interpolated message — with that derived value.
3. Run `site-unit` and watch it pass. It cannot fail before the change, because
   the hardcoded value is currently correct; the proof is step 4.
4. **Mutation-prove it, twice, because the guard has two ways to be vacuous:**
   - edit `kFactoryPresetCount` in the header to a different number, re-run, and
     confirm the case fails naming both counts; revert and confirm
     `git diff --quiet engine/include/poly/presets.h`
   - rename the constant in the header so the regex cannot match, re-run, and
     confirm the case fails with the "pattern did not match" message rather
     than passing; revert and confirm the header is clean again
5. Run `format` and `site-unit`. Append evidence, tick task 2, set row PIPE01 to
   `done`, tick all four definition-of-done boxes, set slice M005/S01 to `done`,
   run `jk-standards ledger`, and commit with the slice's trailers.
