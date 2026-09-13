---
class: gated
---

# M005/S02 — `presets.json` carries the pattern

**Slice:** M005/S02 — `docs/plans/engine-capability/ledger.md`
**Rows:** PIPE02
**Depends:** M005/S01
**Classification:** bounded. A field is added to an emitter that already
serialises fifteen per-lane fields, and a version it already guards is bumped.

## Task status

- [ ] 1. Emit onset positions for timeline lanes, and bump `schemaVersion` to 4
- [ ] 2. Answer the son-clave question from the JSON alone, and close the slice

## Definition of Done

- [ ] A lane running in timeline mode carries its step pattern in
      `site/src/generated/presets.json`
- [ ] `schemaVersion` is bumped, and the generator rejects a JSON written at the
      previous version rather than reading it as if the field were absent
- [ ] A site test answers, from `presets.json` alone, whether `Cuban Son
      Montuno`'s clave is the son clave or `E(5,16)` — the question M001 could
      not ask of that file
- [ ] Every existing consumer of the file still passes

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Context both tasks need

- **The shape is onset positions**, not a boolean array: `"onsets": [0, 3, 6,
  10, 12]`, emitted only when `timeline` is true. `fixedPatternLength` already
  carries the cycle length, so nothing is lost, and this is the form the guide
  itself uses when it states a pattern — `theory-afro-cuban.mdx` says "hits 0,
  3, 6, 10, 12 for 3-2" — so a test comparing the file against the prose
  compares like with like.
- **Why the field is needed at all.** M001/S02 gave four presets hand-authored
  patterns. The emitter writes `timeline` and `fixedPatternLength` but not the
  pattern, so an exact clave and a Euclidean bake of the same hit count and
  cycle serialise identically: the plugin plays the right thing and nothing
  rendered from `presets.json` can show it.
- **Every site that pins the version.** Two are functional and must change
  together, or the generator rejects its own output:
  - `engine/tools/emit_presets.cpp` — the emitted `"schemaVersion":3`
  - `site/scripts/generate-presets-json.mjs` — the `parsed.schemaVersion !== 3`
    guard
  - `site/tests/presets-json-schema.test.mjs` — asserts the version equals 3
  Three more name version 3 in comments only and should be corrected for
  accuracy: `site/tests/preset-taxonomy-conformance.test.mjs`,
  `site/src/audio/preset-patterns.ts`, `site/src/lib/preset-table-data.mjs`.
- **`webui/tests/startup.spec.mjs` is unrelated.** Its `schemaVersion` is the
  WebUI bridge's `POLY_SCHEMA_VERSION`, a different number. Do not touch it.
- **Regenerating is now safe.** M005/S01 removed the early return, so
  `npm --prefix site run generate-presets` rebuilds the emitter on its own. If
  this slice is somehow executed before S01 has landed, stop — that is the
  dependency, and regenerating against a stale emitter is exactly the failure
  S01 exists to remove.

## Task 1 — Emit onsets and bump the schema version

**Files:** `engine/tools/emit_presets.cpp`,
`site/scripts/generate-presets-json.mjs`,
`site/tests/presets-json-schema.test.mjs`,
`site/tests/preset-taxonomy-conformance.test.mjs`,
`site/src/audio/preset-patterns.ts`, `site/src/lib/preset-table-data.mjs`,
`site/src/generated/presets.json`

1. In `site/tests/presets-json-schema.test.mjs`, change the expected
   `schemaVersion` to 4 and add an assertion that every lane with
   `timeline === true` carries an `onsets` array whose length equals its
   `hits`, and whose every entry is an integer in `[0, fixedPatternLength)`.
   Run `site-unit` and **watch both fail** — the version is still 3 and no lane
   has the field. Confirm the failure names the version and the missing field,
   not something else.
2. In `engine/tools/emit_presets.cpp`, emit `,"onsets":[...]` immediately after
   `"fixedPatternLength"` when `lane.timeline` is true, listing the indices
   `s` in `[0, lane.fixedPatternLength)` where `lane.fixedPattern[s]` is set.
   Emit nothing for a non-timeline lane — the field's presence is what says the
   pattern is authored rather than derived.
3. In the same file, change the emitted `"schemaVersion":3` to `4`, and update
   the schema comment block at the top to describe the new field and name this
   slice, in the style the existing `schemaVersion 3 (M071 S04)` notes use.
4. In `site/scripts/generate-presets-json.mjs`, change the
   `parsed.schemaVersion !== 3` guard to `!== 4` and update its header comment.
5. Correct the three comment-only references to version 3 named in the context
   above. Do not change `webui/tests/startup.spec.mjs`.
6. Regenerate with `npm --prefix site run generate-presets` — no explicit build
   step, which also re-proves S01 — and confirm the run reports 45 presets.
7. Run `site-unit` and watch the two assertions from step 1 pass. Then run
   `unit`, `engine-isolation` and `doc-conformance`.
8. Append evidence to `docs/plans/engine-capability/evidence/M005-S02.md`, tick
   task 1, run `jk-standards ledger`, and commit with the slice's trailers.

## Task 2 — Answer the son-clave question from the JSON, and close the slice

**Files:** `site/tests/presets-json-schema.test.mjs` (or a new sibling test if
that file grows unwieldy — decide when you see it, and say which in the
evidence), `docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M005-S02.md`

1. Add a case that reads `Cuban Son Montuno` from `presets.json`, takes its
   clave lane's `onsets`, and asserts two things: that they equal
   `[0, 3, 6, 10, 12]`, the son clave as `theory-afro-cuban.mdx` states it; and
   that they differ from `bjorklund` for the same hit count and cycle. Import
   the Bjorklund implementation from `site/src/audio/bjorklund.ts`, which is the
   shared generator the site and engine agree on — do not hand-type the
   Euclidean pattern, because the point of the case is that the two are computed
   and compared, not asserted side by side.
2. Run `site-unit` and watch it pass.
3. **Mutation-prove it:** edit the clave's onsets in `presets.json` to the
   Euclidean `[0, 3, 6, 9, 12]`, re-run, and confirm the case fails on both
   arms — the equality and the differs-from-Euclidean assertion. Restore by
   regenerating rather than by hand-editing back, and confirm
   `git diff --quiet site/src/generated/presets.json`.
4. Run the full validation set. Append evidence, tick task 2, set row PIPE02 to
   `done`, tick all four definition-of-done boxes, set slice M005/S02 to `done`,
   run `jk-standards ledger`, and commit with the slice's trailers.
