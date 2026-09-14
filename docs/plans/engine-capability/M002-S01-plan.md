---
class: gated
---

# M002/S01 — The mode is something a patch can express

**Slice:** M002/S01 — `docs/plans/engine-capability/ledger.md`
**Rows:** EC06
**Design:** `M002-S01-design.md` — approved before this plan was written
**Classification:** architectural

## Task status

- [x] 1. `KotekanMode`, the overlap field, and the derivation
- [x] 2. Serialize both fields at state version 19
- [x] 3. Emit both into `presets.json` at schemaVersion 5
- [x] 4. Surface both in the WebUI
- [ ] 5. `Balinese Kotekan` adopts `Telu` with overlap 1, and close the slice

## Definition of Done

- [ ] A gamelan patch can name its interlock style, rather than only a source lane
- [ ] Polos–sangsih overlap is controllable, so the composite is no longer
      complete by construction
- [ ] The mode a preset uses reaches `site/src/generated/presets.json`, so a patch
      table can report it
- [ ] `renderRange()` gains no allocation, lock or blocking call
- [ ] The engine builds and passes its tests with no VST3 SDK present

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
| `webui-e2e` | `npm --prefix webui test` |

## Context every task needs

Read `M002-S01-design.md` first; it carries the measured constraints. The three
that bind hardest:

- **Both new fields become lane *core* parameters.** The design originally said
  state-only; that was corrected during task 4 — see the marked correction in
  `M002-S01-design.md`. The WebUI edit path resolves through `resolveParamId`,
  so a field with no parameter ID cannot be edited from the interface. Both
  per-lane families are full, so `kCoreParamsPerLane` goes 12 → 14. Presets and
  saved state are unaffected (the state is a blob); only host automation on
  per-lane core params for lanes 1–7 shifts, and Poly has never been released.
- **The default must reproduce today's output exactly.** `NyogCag` with overlap
  0 is the current strict complement. If any determinism golden moves before
  task 5, something is wrong — stop rather than regenerate it.
- **The derivation runs on the audio thread.** No allocation in
  `buildLanePattern`; the overlap pass writes into the existing `pattern` array.

The modes, stated mechanically — copy these, do not re-derive them:

| Mode | Rule |
|---|---|
| `NyogCag` | `pattern[s] = !src[s]` |
| `Telu` | `pattern[s] = !src[s % 3]` |
| `Empat` | `pattern[s] = !src[s % 4]` |

## Task 1 — `KotekanMode`, the overlap field, and the derivation

**Files:** `engine/include/poly/types.h`, `engine/src/engine.cpp`,
`engine/src/sanitize.cpp`, `tests/` (a new or existing engine test file)

1. Write the failing tests first, against a hand-built `GrooveState` with a
   two-lane interlocking pair:
   - `NyogCag` reproduces the strict complement for a known source pattern
   - `Telu` repeats on a three-step cell; `Empat` on four — assert the derived
     onsets explicitly, not "differs from strict"
   - `kotekanOverlap = 1` forces step 0 on in both lanes, so the intersection
     is non-empty
   - `kotekanOverlap = 0` leaves the intersection empty, byte-identical to
     `NyogCag` today
   Run `unit` and watch each fail for want of the fields.
2. Add `enum class KotekanMode : uint8_t { NyogCag = 0, Telu = 1, Empat = 2 };`
   and the two `LaneConfig` fields with the defaults the design names.
3. In `engine/src/engine.cpp`, inside `region:kotekan`, branch the complement on
   the mode. Keep the saturation fallback (`complementHits == 0`) applying to
   all three — it guards a real collapse, not a NyogCag quirk.
4. Apply overlap after the mode's pattern is derived: force `pattern[s] = true`
   at the first `kotekanOverlap` structural points, cycle boundary first, then
   the phrase join if the lane has phrase gating, then the midpoint.
5. In `sanitize.cpp`, clamp the mode to the enum's range and the overlap to
   `[0, cycle.steps]`, beside the existing `kotekanSourceLane` clamp.
6. Run `unit`, `engine-isolation` and `rt-safety`. **Confirm no determinism
   golden moved** — if one did, the default is not reproducing today's output
   and that is a defect, not a fixture to regenerate.
7. Append evidence, tick task 1, run `jk-standards ledger`, commit with trailers.

## Task 2 — Serialize both fields at state version 19

**Files:** `engine/include/poly/state_io_envelope.h`,
`engine/include/poly/state_io_write_lane.h`,
`engine/include/poly/state_io_read_lane.h`, `tests/`

1. Write a failing round-trip test: a state carrying `Telu` and overlap 2
   survives write-then-read. Run `unit` and watch it fail.
2. Add `static constexpr int32_t kKotekanModeStateVersion = 19;` beside the
   existing version constants, and raise `kCurrentStateVersion` to 19.
3. Write both fields in the lane writer; read them in the lane reader guarded by
   `if (version >= kKotekanModeStateVersion)`, exactly as
   `kLaneSeedLockStateVersion` already guards its field.
4. Write a second test proving the migration is lossless: a state written at
   version 18 reads back with `NyogCag` and overlap 0 — which is what it played
   before — rather than failing or reading garbage.
5. Run `unit` and `engine-isolation`. Evidence, tick task 2, ledger, commit.

## Task 3 — Emit both into `presets.json` at schemaVersion 5

**Files:** `engine/tools/emit_presets.cpp`,
`site/scripts/generate-presets-json.mjs`,
`site/tests/presets-json-schema.test.mjs`, `docs/preset-taxonomy.md`,
`site/src/generated/presets.json`

1. In the schema test, expect `schemaVersion` 5 and assert every lane with
   `kotekanSourceLane >= 0` carries a `kotekanMode` string and a numeric
   `kotekanOverlap`. Run `site-unit`, watch it fail.
2. Emit both fields — the mode as its name (`"nyogcag"`, `"telu"`, `"empat"`),
   not its integer, so the JSON reads without a lookup table. Bump the emitted
   `schemaVersion` to 5 and update the emitter's schema comment.
3. Update the generator's version guard 4 → 5 and its header comment.
4. Add a row to `docs/preset-taxonomy.md`'s **JSON schema version** table for
   version 5. That table exists because M005's doc-drift failure put it there,
   and `emit_presets.cpp` is a mapped source — a change here without it fails
   `doc-discipline` locally now, which is what M006 bought.
5. Regenerate, run `site-unit` and `doc-conformance`. Evidence, tick, commit.

## Task 4 — Surface both in the WebUI

**Files:** `plugin/source/webui/bridge_serialization.cpp`,
`plugin/source/webui/web_ui_view.cpp`, `webui/`, `webui/bridge-schema.md`

1. Extend the bridge payload with both fields, following `kotekanSourceLane`'s
   existing path on both sides.
2. Add a mode selector and an overlap stepper to the lane pane, beside the
   existing kotekan source control. Match the surrounding controls' idiom rather
   than inventing one.
3. Regenerate `webui/bridge-schema.md` with
   `node scripts/generate-bridge-schema-doc.mjs` — it is a declared generated
   doc, so a stale copy fails `generated-freshness`.
4. Run `webui-e2e` and `unit`. Evidence, tick task 4, ledger, commit.

## Task 5 — `Balinese Kotekan` adopts the mode, and close the slice

**Files:** `engine/src/presets.cpp`, `site/src/generated/presets.json`,
`tests/`, `docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M002-S01.md`

1. Write a failing test asserting `Balinese Kotekan`'s guiro lane has mode
   `Telu` and overlap 1, and that its composite with the polos lane has a
   non-empty intersection — the property Rule 4 is about. Run `unit`, watch it
   fail.
2. Set those values in `makeBaliKotekan`. Leave `Kotekan Interlock`,
   `Ewe Polymetric Ensemble` and `Afro-Electronic Fusion` on `NyogCag`.
3. Run `unit`. **One determinism golden is expected to move** — this preset's.
   Confirm that exactly one moved, read the diff rather than regenerating
   blindly, and record in the evidence what changed and why it is right. If more
   than one moved, stop: the default is leaking.
4. Regenerate `presets.json` and confirm the mode reaches it.
5. Run the full validation set. Append evidence, tick task 5, set row EC06 to
   `done`, tick all five definition-of-done boxes, set slice M002/S01 to `done`,
   run `jk-standards ledger`, commit with trailers.
