---
class: gated
---

# M006/S02 — The engine features the browser host drops

**Slice:** central-africa M006/S02 (rows CA41–CA43)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Design:** `M006-S02-design.md` (approved 2026-10-18)
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [ ] 1. The WASM API carries fill, seed lock, lane count and manual fill
- [ ] 2. The browser host uses them, and the rebuilt engine ships

## Definition of Done

Copied verbatim from the slice:

- [ ] Fill Every N and Fill Now fill in the browser as they do in the plugin
- [ ] Seed Lock holds the seed in the browser
- [ ] − Lane and + Lane change the number of lanes that play

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `wasm-freshness` | `bash scripts/check-wasm-freshness.sh` (needs a deployed URL; see `M006-decisions.md`) |
| `webui-e2e` | `npm --prefix webui test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

This slice changes engine source, so the `.wasm` and `.js` that `e2e`
rebuilds are committed in Task 2, and only there.

## Task 1 — The WASM API carries fill, seed lock, lane count and manual fill

Rows: CA41, CA42, CA43 (engine side).

**Files**

- Modify `engine/src/wasm_api.cpp`, and its header if it declares the API
- Create `tests/wasm_api_m006_tests.cpp`; add it to `tests/CMakeLists.txt`
  beside `wasm_api_envelope_gap_slot_tests.cpp`
- Create `docs/plans/central-africa/evidence/M006-S02.md`

**Consumes:** nothing. **Produces:** the four entry points Task 2 calls.

**Steps**

1. Write the tests, in the style of `wasm_api_envelope_gap_slot_tests.cpp`:
   - `FillEveryNBars` (enum value 15) round-trips through
     `poly_edit_lane_int`/`poly_lane_int`, and clamps 2000 to 1024 and −3 to 0.
   - `SeedLocked` (16): with scene seed 111, setting it to 1 makes
     `poly_lane_int` read 1 and captures 111 as the lane seed; changing the
     seed to 222 then leaves that lane's rendered events identical to before
     the change, while an unlocked lane's change. Setting it to 0 and back to
     1 after seed 222 captures 222.
   - `poly_set_active_lane_count(ctx, 2)` makes `poly_active_lane_count`
     read 2; 0 clamps to 1, 99 to `kMaxLanes`; rendering then emits notes
     only on lanes 0 and 1.
   - `poly_trigger_manual_fill`: with a lane whose fill weights change the
     output, a render after the trigger differs from the same render without
     it, and the render after that matches the untriggered one (one pass).
2. Build and run them; watch them fail to compile, since none of the symbols
   exist.
3. Implement per the design: append `FillEveryNBars` and `SeedLocked` to
   `LaneFieldInt` after `KotekanOverlap`, with read and write cases (clamp to
   0–1024; the seed-lock edge copies `c->state().seed` into `laneSeed`);
   add `poly_set_active_lane_count`; add a `bool manualFillPending` to
   `Context`, set by `poly_trigger_manual_fill`, copied into
   `resolved.fillManualTrigger` in `poly_render` and cleared there. Export
   the two new functions wherever the build lists exported functions
   (`CMakeLists.txt` `poly_wasm` target).
4. Run `unit` and watch the new tests pass. Mutate: skip the seed capture on
   the edge, watch the seed-lock case fail; restore. Record.
5. Run `format` and `unit`. Write the evidence. Tick this task. Commit with
   `Rows: CA41, CA42, CA43`.

## Task 2 — The browser host uses them, and the rebuilt engine ships

Rows: CA41, CA42, CA43 (host side).

**Files**

- Modify `webui/wasm-host.js`
- Create `webui/tests/engine-features.spec.mjs`
- Commit `webui/poly_engine.js`, `webui/poly_engine.wasm` as rebuilt
- Append to the evidence; modify this plan and the ledger

**Consumes:** Task 1's entry points; S01's `forBoth`, `laneConfig` and
`renderSignature` (copy the helpers into the new spec).

**Steps**

1. Write the spec on the WASM host, playing Deep House:
   - `lane.0.fillEveryN` at 1/64 (every bar) changes
     `renderSignature(playbackCtx)`.
   - `manualFill` reaches the playing context: wrap
     `Module._poly_trigger_manual_fill` in the page with a spy that records
     its argument and calls through, send the action, and assert the spy saw
     `__polyPlaybackCtx`. The latch lives on the Context and is consumed by
     the next render, so its effect on the sound is proved by Task 1's unit
     test, not here.
   - `lane.0.seedLock` 1, then `seed` 0.9: lane 0's rendered events from the
     playing context are unchanged, lane 1's change.
   - `activeLaneCount` at the norm for 2 lanes: the playing context renders
     notes on lanes 0 and 1 only.
2. Rebuild the engine (`bash scripts/build-wasm.sh`) and run the spec; watch
   each case fail: the host drops all four.
3. Implement in `webui/wasm-host.js`: add `fillEveryN` (`round(v * 64)`,
   `LaneFieldInt.FillEveryNBars`) and `seedLock` (`v >= 0.5 ? 1 : 0`,
   `LaneFieldInt.SeedLocked`) to `INT_EDITS` through `forBoth`; handle
   `paramId === 'activeLaneCount'` with `1 + round(v * 7)` through `forBoth`;
   add `case 'manualFill'` calling `_poly_trigger_manual_fill` on
   `playbackCtx` (or `engineCtx` when stopped). Add both new enum entries to
   the host's `LaneFieldInt` table, and report `fillEveryN` and `seedLocked`
   from `readLaneFromWasm`.
4. Run the spec and watch it pass. Mutate: drop the `seedLock` entry, watch
   that case fail; restore. Record.
5. Run all five tokens. `e2e` rebuilds the artifacts: keep them and stage
   them by path. Record `wasm-freshness` per the decision. Tick the DoD boxes
   here and in the ledger, set CA41–CA43 and the slice to `done`, run
   `jk-standards ledger`. Append the evidence. Tick this task. Commit with
   `Rows: CA41, CA42, CA43`.

## Self-review

- **DoD.** Fill → Tasks 1 and 2 (fill cases). Seed lock → seed-lock cases.
  Lanes → lane-count cases.
- **Rows.** CA41 → spec's fill cases (the row's "each changes the rendered
  output"); CA42 → seed-lock case; CA43 → lane-count case. Entry points
  appended at the end of their enums → Task 1 step 3.
- **Placeholders.** None.
- **Names.** `FillEveryNBars`, `SeedLocked`, `poly_set_active_lane_count`,
  `poly_trigger_manual_fill`, `manualFillPending`, `engine-features.spec.mjs`.
