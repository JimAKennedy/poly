---
class: gated
---

# M002/S01 — Eleven formulae are presets

**Slice:** central-africa M002/S01 (rows CA04–CA18)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M002-decisions.md` (2026-10-15)

## Task status

- [x] 1. The grid harness proves Aka: Ngbolu, and the category exists
- [x] 2. The other ten formulae pass the harness
- [x] 3. Every count and name site agrees with 56 presets and 11 categories
- [x] 4. The WASM engine and generated data carry the new presets

## Definition of Done

Copied verbatim from the slice:

- [x] Eleven presets exist in a new "Central African" category, and the
      existing "African" category is named "West African"
- [x] Every preset's lanes, tiled to the formula's period, equal Arom's grid
      position by position at the preset's own macro settings, and the
      per-position count of marking parts equals his `(count)` row, asserted
      by a test that fails if one position moves
- [x] Every lane that carries marks has Mutation, Drift, Swing and Humanize at
      0, Probability 1 and no phrase gating
- [x] Every place that hard-codes the preset or category count agrees with 56
      and 11, and the WebUI lists all 56 presets with a lane name for each
      Central African lane
- [x] The committed WASM engine contains the new presets

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
| `site-unit` | `npm --prefix site test` |
| `webui-e2e` | `npm --prefix webui test` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `e2e` | `bash scripts/site-verify-local.sh` |

`e2e` rewrites `webui/poly_engine.js` and `webui/poly_engine.wasm`. Only task 4
commits them; every earlier task restores them with
`git checkout -- webui/poly_engine.js webui/poly_engine.wasm` and stages
explicit paths, never `git add -A`.

## The formulae

Grids are Arom's, transcribed in the owner's research note and re-checked
against Poly's Euclidean generator before the vision was written. `x` is a
mark; pulsation bars are for reading only. "E(k,n) rN" is a Euclidean lane;
"drawn" is a timeline lane whose `fixedPattern` is the row and whose
`hitCount` equals its marks.

| # | Index | Preset | Grid | Lanes (note, recipe) |
|---|---|---|---|---|
| 1 | 45 | Aka: Ngbolu | 24 at 1/8 | dikpakpa 37 E(1,3) r2 · ngue 64 drawn 12 · diketo 67 E(13,24) r4 |
| 2 | 46 | Aka: Mokondi | 24 at 1/8 | endomba 63 drawn 12 · dikpakpa 37 drawn 24 · ngue 64 drawn 12 · diketo 67 E(13,24) r13 |
| 3 | 47 | Aka: Yombe | 24 at 1/8 | endomba 63 E(1,3) r1 · dikpakpa 37 E(1,3) r2 · ngue 64 drawn 12 · diketo 67 E(13,24) r19 |
| 4 | 48 | Aka: Monzoli | 24 at 1/8 | endomba 63 drawn 4 · dikpakpa 37 E(1,3) r2 · ngue 64 drawn 12 · diketo 67 E(13,24) r22 |
| 5 | 49 | Aka: Mombenzele | 24 at 1/8 | endomba 63 drawn 4 · dikpakpa 37 E(1,3) r2 · ngue 64 drawn 24 · diketo 67 E(13,24) r10 |
| 6 | 50 | Aka: Zoboko | 24 at 1/8 | mokongo 76 drawn 24 · claps 39 E(1,3) r0 · diketo 67 E(13,24) r19 |
| 7 | 51 | Aka: Bondo | 12 at 1/8 | endomba 63, ngue 64, dikpakpa 37 each E(5,12) r7 · diketo 67 E(7,12) r0 · rattle 70 E(1,3) r0 AnchorPulse |
| 8 | 52 | Aka: Ngbolu Drums | 12 at 1/8 | dikpakpa 37 E(1,3) r2 · ngue 64 drawn 12 |
| 9 | 53 | Banda-Dakpa: Gaza | 16 at 1/16 | husband 77 drawn 16 · mother 76 E(3,16) r8 |
| 10 | 54 | Zande: Kponingbo | 8 at 1/16 | guru 64 E(3,8) r0 · bells 54 E(1,4) r2 AnchorPulse |
| 11 | 55 | Banda-Linda: Aga-torumo | 10 at 1/16, Tempo Mult 1.25 | akone 64 drawn 10 · kaminga 63 drawn 10 |

The exact fixtures — every row and every count row — are typed once, into
`tests/central_african_preset_tests.cpp`, from Part C of the owner's research
note (examples 1–11), whose grids were re-checked against Poly's Euclidean
generator before the vision was written. The fixture test that sums each
fixture's rows to its count row catches a transcription slip. The plan does not restate them, so
there is one copy to check against the note.

All lanes: probability 1, `mutationRate` 0, `driftRate` 0, `swingAmount` 0,
`humanizeMs` 0, `phraseLength` 0, `noteDuration` 0.1, `velocitySpread` 0,
`emphasisProb` 0. Macros: complexity 0.5, density 0.5, syncopation 0, swing 0,
tension 0, humanize 0. A fixed seed per preset. Velocities: diketo 100, ngue
110, endomba 95, dikpakpa 80, claps and rattle 75, mokongo 100, the gàzà,
kpóníngbó and àgà drums 105 and 95, the bells 75. Lane names are the part names
above, each ≤ 15 characters.

---

## Task 1 — The grid harness proves Aka: Ngbolu, and the category exists

**Rows:** CA04, CA15, CA16. **Consumes:** nothing. **Produces:** the harness
and fixture format tasks 2 reuses; the category the other presets join.

**Files:** `tests/central_african_preset_tests.cpp` (new),
`tests/CMakeLists.txt`, `engine/include/poly/presets.h`,
`engine/src/presets.cpp`, `tests/preset_conformance_tests.cpp`.

1. Write `tests/central_african_preset_tests.cpp`:
   - a fixture type: preset name, period in steps, one `{lane index, row}`
     per part, and Arom's count row
   - `renderedSteps(state, lane, periodSteps, stepPpq)`: resolve with
     `resolveConstraints(base, resolveMacros(base))`, render one period with
     `Engine::renderRange` (playing, tempo 120), and map each of that lane's
     note events to `lround(ppq / stepPpq)`
   - one test per fixture asserting each lane's rendered steps equal the row's
     marks, and the per-position count of sounding lanes equals the count row
   - one test that the fixture's own rows sum to its count row, so a typo in a
     fixture fails before any engine is involved
   - only the Ngbolu fixture for now
2. Add it to `poly_tests` in `tests/CMakeLists.txt`. Build and run: the Ngbolu
   case fails because no preset of that name exists.
3. In `presets.h` set `kFactoryPresetCount` to 46 and
   `kFactoryPresetCategoryCount` to 11, and declare `makeAkaNgbolu()`. In
   `presets.cpp` rename the "African" category and every `kInfos` use of it to
   "West African", append "Central African", write `makeAkaNgbolu()` from the
   table above, add `case 45`, and add its `kInfos` row with a one-line
   description citing Arom exx. 104–105.
4. In `preset_conformance_tests.cpp`:
   - `isMetronomicTradition` names "West African" and "Central African" in
     place of "African"
   - `EveryPresetHasExactlyOneLockedReferent` requires at least one locked lane
     for "Central African" and exactly one elsewhere, the reason in a comment:
     Arom's referent is the implicit pulsation (p. 230) and every part's marks
     are fixed (R10, R12)
5. Run `unit`. Ngbolu passes. Then flip one position in the Ngbolu fixture, run
   again, watch the case fail, and restore it; record both runs in the
   evidence.
6. Count sites and generated data now disagree with 46 presets, so run
   `site-unit` and note which tests fail. They are task 3's and task 4's to
   fix. Record them in the evidence. They do not block this commit, because
   `unit` is this task's token and the slice is validated whole at task 4.
7. Run `format` and `rt-safety`, append the evidence, tick this box, and
   commit with `Rows: CA04, CA15, CA16`.

## Task 2 — The other ten formulae pass the harness

**Rows:** CA05–CA14. **Consumes:** task 1's harness. **Produces:** all eleven
presets at indices 45–55.

**Files:** the same test file, `presets.h`, `presets.cpp`.

1. Add the ten remaining fixtures. Run `unit` and watch each fail on its
   missing preset.
2. Write the ten builders in table order, `case 46`–`case 55`, and their
   `kInfos` rows, and set `kFactoryPresetCount` to 56. Àgà-tɔ̀rúmɔ̃ sets
   `tempoMultiplier = 1.25f` on both lanes. Its fixture's step length is
   `0.25 / 1.25 = 0.2` beats, and a case asserts that value directly.
3. Run `unit` and watch all eleven pass, along with every existing preset test
   now applied to them.
4. Run `format` and `rt-safety`, append the evidence, tick this box, and
   commit with `Rows: CA05, CA06, CA07, CA08, CA09, CA10, CA11, CA12, CA13,
   CA14`.

## Task 3 — Every count and name site agrees with 56 presets and 11 categories

**Row:** CA17. **Consumes:** task 2's 56 presets. **Produces:** a repo with no
stale preset or category count.

**Files:**
- `plugin/source/webui/web_ui_view.cpp`
- `webui/mock-host.js`, `webui/tests/preset-selector.spec.mjs`,
  `webui/tests/startup.spec.mjs`
- `site/tests/presets-json-schema.test.mjs`
- `site/tests-e2e/modal-preset-categories.spec.ts`
- `docs/preset-taxonomy.md`, `site/tests/preset-taxonomy-conformance.test.mjs`
- `tests/cubase/e2e/preset-recall.spec.ts`,
  `tests/cubase/e2e/lib/preset-contract.ts`, `.github/workflows/cubase-nightly.yml`
- `engine/tools/emit_presets.cpp`, `CHANGELOG.md`, `.github/docs-drift-map.yml`

1. Regenerate the site data (`npm --prefix site run generate-presets` and
   `generate-counts`). Run `site-unit` and `webui-e2e` and record what fails.
2. `kWebPresetLaneNames`: fill rows 45–55 with the part names. Rows 14–44 stay
   zero-filled, as today.
3. `mock-host.js`: append presets 43–55 to `PRESETS`, `PRESET_NAMES`,
   `PRESET_MACROS` and `PRESET_SEEDS`; rename and append categories; extend
   `PRESET_CATEGORY_BY_INDEX`; correct the header comment. The two webui specs
   move from 43/44 to 56/57.
4. `presets-json-schema.test.mjs` and `modal-preset-categories.spec.ts`: the
   expected category lists rename "African" and append "Central African"; the
   e2e row counts become 57.
5. `preset-taxonomy.md`:
   - counts move to 56 and 11
   - the caps become 11 categories and 2–11 presets per category, with the
     reason (the eleven formulae are one tradition and one source)
   - the "## The 10 categories" heading becomes "## The 11 categories", in the
     conformance test too
   - mapping rows 45–55 are added, and the "African" rows become "West African"
   - the schemaVersion statement becomes the emitter's current value, read from
     `emit_presets.cpp`, not assumed
6. The Cubase wording: "45" becomes "56" in the spec's test name and comments,
   in the contract comment and in the nightly step name. The timeout minutes
   are not counts and do not change.
7. The `emit_presets.cpp` schema comment's `presetCount`, the changelog
   musician block's "45 presets", and the drift map's "10-category" reason.
8. Run `site-unit`, `webui-e2e`, `cubase-harness`, `doc-discipline`, `unit`,
   `format`. Search the tree for 43 and 45 used as preset counts, outside
   historical changelog entries and dated reviews. Append the evidence, tick,
   commit with `Rows: CA17`.

## Task 4 — The WASM engine and generated data carry the new presets

**Row:** CA18. **Consumes:** tasks 1–3. **Produces:** the slice's last row;
the slice closes here.

1. Run `e2e`, which rebuilds the WASM engine and runs the site's browser
   checks. It must pass.
2. Confirm the rebuilt `webui/poly_engine.wasm` contains "Aka: Ngbolu" and
   "Zande: Kponingbo" (`strings`), and that the WebUI's join of the WASM list
   to `presets.json` holds 56 entries in the same order (one e2e check, or a
   site test reading both).
3. Run the slice's whole validation set on this head. Every token passes.
4. Stage the two WASM artifacts, the generated JSON and the evidence by
   explicit path. Tick this box and the Definition of Done, set the slice and
   rows CA04–CA18 `done`, run `jk-standards ledger`, and commit with
   `Rows: CA18`.
