---
class: gated
---

# M001/S02 — Exact timelines

**Slice:** M001/S02 — `docs/plans/engine-capability/ledger.md`
**Rows:** EC05
**Classification:** bounded. Four presets already ship hand-authored exact
timelines (`makeAfrobeat12_8`, `makeBossaNova`, `makeEweAgbekor`,
`makeAfrobeatLagos`); this task follows that precedent rather than introducing a
mechanism.

## Task status

- [x] 1. Correct `Cuban Son Montuno`'s clave lane to the exact son clave
- [x] 2. Add the `Rumba Clave` preset
- [x] 3. Add the `Clapping Music` preset
- [x] 4. Retire the guide's workaround framing, and close the slice

## Definition of Done

- [ ] Son clave, rumba clave and Clapping Music ship as presets whose lanes
      carry hand-authored `fixedPattern` timelines, in the manner the four
      existing hand-authored timelines already use
- [ ] `Cuban Son Montuno`'s clave lane is no longer the Euclidean pattern
      `lockReferentLane` bakes
- [ ] A test asserts each shipped timeline equals its published pattern and
      differs from the Euclidean pattern of the same hit count and cycle length
- [ ] The guide's instruction to hand-build a true clave in timeline mode no
      longer describes the only way to obtain one

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Context every task needs

- **The patterns are documented in the repo.** Do not derive them:
  - son clave 3-2 — hits at 0, 3, 6, 10, 12 of 16
    (`site/src/content/docs/theory-afro-cuban.mdx`, construction step 1)
  - rumba clave 3-2 — hits at 0, 3, 7, 10, 12 of 16, gaps 3-4-3-2-4
    (`site/src/content/docs/03-afro-cuban.mdx`)
  - Clapping Music — `x x x . x x . x . x x .`, eight claps across twelve
    pulses (`site/src/content/docs/08-minimalism.mdx`)
- **How a hand-authored timeline is written**, per the four precedents:
  set `cfg.timeline = true`, `cfg.fixedPatternLength = <cycle>`, and
  `cfg.fixedPattern = {true, false, ...}` with one entry per step.
- **Why the correction is needed.** `lockPresetReferent` picks one lane per
  preset and calls `lockReferentLane`, which bakes `euclidean(hitCount, steps,
  rotation)` into the timeline slots. 39 of the 43 timeline lanes are baked this
  way, including `Cuban Son Montuno`'s clave. The comment above
  `lockPresetReferent` already states that presets shipping a hand-authored
  reference are left untouched — so authoring the pattern is also what stops it
  being re-baked. Verify that behaviour holds rather than assuming it.
- **Adding a preset touches five places**, and the build catches two of them:
  - `engine/include/poly/presets.h` — bump `kFactoryPresetCount`, declare the
    builder
  - `engine/src/presets.cpp` — the builder, the `{name, description, category}`
    registry entry, and the index dispatch that returns it
  - `plugin/source/webui/web_ui_view.cpp` — **no row, deliberately.**
    `kWebPresetLaneNames` is declared `[kFactoryPresetCount][kMaxLanes]` but is
    initialised sparsely: only the first 14 rows carry bespoke labels, and every
    preset from index 14 on zero-fills to null and falls back to default lane
    names at the `applyPreset` call site. Appending a row would give it index
    14 — `makeEweAgbekor` — and relabel that preset's lanes. The `static_assert`
    pins the array's extent to the preset count; it cannot detect a missing row.
  - `docs/preset-taxonomy.md` — two hardcoded counts. `count_drift` triggers on
    `factory presets` and `presets ... are grouped`, so a missed count fails
    `doc-discipline`.
- **`presets.json` is generated**, by building and running `poly_presets_emit`
  against an engine-only tree: `npm --prefix site run generate-presets`. No WASM
  is involved, so none of the artifact-churn discipline in `CLAUDE.md` applies
  to this slice. Regenerate it in the task that changes preset data, and commit
  the regenerated file with that task.

## Task 1 — Correct `Cuban Son Montuno`'s clave lane

**Files:** `engine/src/presets.cpp`, `tests/preset_tests.cpp`,
`site/src/generated/presets.json`

1. In `tests/preset_tests.cpp`, add a test asserting that the clave lane of
   `Cuban Son Montuno` has `timeline == true`, `fixedPatternLength == 16`, and
   hits at exactly {0, 3, 6, 10, 12} — and, separately, that this differs from
   `euclidean(5, 16, 0)`. Write the Euclidean comparison as a computed call, not
   as a copied literal, so the test states the distinction rather than asserting
   two hand-typed arrays.
2. Build and run `unit`. **Watch it fail**, and read the failure: it must report
   the lane carrying the Euclidean pattern, which is the defect. If it fails for
   any other reason, stop.
3. In `makeCubanSonMontuno`, set the clave lane's `timeline`,
   `fixedPatternLength = 16`, and `fixedPattern` to the son clave literal.
4. Build and run `unit`; watch it pass.
5. Confirm `lockPresetReferent` left the lane alone — the preset should still
   satisfy `isReferentLocked` through the hand-authored pattern rather than
   through a re-bake. Assert this in the same test if it is not already implied.
6. Regenerate `presets.json` and confirm the clave lane's data changed.
7. Run the full validation set. Append evidence to
   `docs/plans/engine-capability/evidence/M001-S02.md`, tick task 1, run
   `jk-standards ledger`, commit with the slice's trailers.

## Task 2 — Add the `Rumba Clave` preset

**Files:** `engine/include/poly/presets.h`, `engine/src/presets.cpp`,
`plugin/source/webui/web_ui_view.cpp`, `docs/preset-taxonomy.md`,
`tests/preset_tests.cpp`, `site/src/generated/presets.json`

1. Extend the test from task 1 to cover a preset named `Rumba Clave`, asserting
   its clave lane's hits are exactly {0, 3, 7, 10, 12} of 16 and that this
   differs from `euclidean(5, 16, 0)`. Run `unit` and watch it fail because no
   such preset exists.
2. Bump `kFactoryPresetCount` to 44 and declare `makeRumbaClave()` in
   `presets.h`.
3. Write `makeRumbaClave()` in `presets.cpp`: a clave lane carrying the
   hand-authored rumba pattern, plus supporting lanes consistent with the
   existing `Cuban Son Montuno` texture. Add the registry entry with category
   `Latin / Brazilian`, and the index dispatch case.
4. Add no `kWebPresetLaneNames` row — see the context note above. The preset
   takes default lane names, as every preset from index 14 on already does.
5. Update both hardcoded counts in `docs/preset-taxonomy.md` from 43 to 44, and
   place the preset in its category listing.
6. Build and run `unit`; watch it pass. Regenerate `presets.json`.
7. Full validation set, evidence, tick task 2, `jk-standards ledger`, commit.

## Task 3 — Add the `Clapping Music` preset

**Files:** as task 2.

1. Extend the test to cover a preset named `Clapping Music`, asserting a lane
   with `fixedPatternLength == 12` and hits at exactly {0, 1, 2, 4, 5, 7, 9, 10}
   — the twelve-pulse pattern `x x x . x x . x . x x .` — and that it differs
   from `euclidean(8, 12, 0)`. Run `unit` and watch it fail.
2. Bump `kFactoryPresetCount` to 45, declare `makeClappingMusic()`.
3. Write the builder: two lanes carrying the same hand-authored pattern, the
   second phasing against the first, which is what the piece does. Consult
   `makeReichPhasing` for how this repo already expresses a phasing voice, and
   follow it. Register with category `Minimalist / Compositional`.
4. Add the `kWebPresetLaneNames` row.
5. Update both counts in `docs/preset-taxonomy.md` from 44 to 45 and add the
   category placement.
6. Build and run `unit`; watch it pass. Regenerate `presets.json`.
7. Full validation set, evidence, tick task 3, `jk-standards ledger`, commit.

## Task 4 — Retire the workaround framing, and close the slice

`theory-afro-cuban.mdx` currently tells the reader the exact son and rumba
claves are non-Euclidean and to use the timeline-mode workflow from Chapter 3 to
get the true pattern. That instruction was true while every shipped clave was
Euclidean-baked. After tasks 1 and 2 it is no longer the only route, and the
sentence should say so without deleting the workflow — building a clave by hand
remains a legitimate thing a reader may want to do.

**Files:** `site/src/content/docs/theory-afro-cuban.mdx`,
`site/tests/theory-audit-claims.test.mjs`,
`docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M001-S02.md`

1. Add a case to `site/tests/theory-audit-claims.test.mjs` asserting the page
   names the shipped presets as a route to an exact clave. Run `site-unit` and
   watch it fail.
2. Amend the parenthetical so it states both routes: the presets ship the exact
   patterns, and the timeline-mode workflow remains available for building one
   by hand. Do not remove the non-Euclidean statement — it is the fact that
   makes both routes necessary, and it is correct.
3. Run `site-unit`; watch it pass.
4. **Mutation-prove it:** revert the sentence to its workaround-only wording,
   confirm the new case fails, restore, confirm `git diff --quiet`.
5. Run the full validation set. Append evidence, tick task 4, set row EC05 to
   `done`, tick all four definition-of-done boxes, set slice M001/S02 to `done`,
   run `jk-standards ledger`, commit with trailers.
