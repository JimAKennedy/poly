---
class: gated
---

# M006/S02 — design: the engine features the browser host drops

Approved by the owner on 2026-10-18 (`M006-decisions.md`).

## Problem

Four controls reach the plugin's engine but not the WASM engine:

| Control | WebUI sends | Plugin path | WASM host today |
|---|---|---|---|
| Fill Every N | `edit lane.N.fillEveryN` | `kCoreFillEveryN` → `cfg.fillEveryNBars` | dropped: no `fillEveryN` in `INT_EDITS` |
| Fill Now | `action manualFill` | `kFillManualTrigger` latches `fillManualTriggered_`, consumed by one render | unknown action |
| Seed Lock | `edit lane.N.seedLock` | `kCoreSeedLock`: false→true captures `gs.seed` into `cfg.laneSeed`, then sets `cfg.seedLocked` | dropped |
| − Lane / + Lane | `edit activeLaneCount` | `gs.activeLaneCount = 1 + round(norm * 7)` | dropped |

## Interface

Appended to `engine/src/wasm_api.cpp`, never inserted, because the JS side
addresses lane fields by index:

- `LaneFieldInt::FillEveryNBars` and `LaneFieldInt::SeedLocked`, after
  `KotekanOverlap`. `poly_edit_lane_int` on `FillEveryNBars` clamps to
  0–1024 as `sanitize.cpp` does. On `SeedLocked`, a false→true edge copies the
  scene's `seed` into the lane's `laneSeed` before setting the flag, exactly as
  `processor.cpp` does; unlock keeps `laneSeed`. `poly_lane_int` reads both.
- `void poly_set_active_lane_count(PolyContext, int count)`: clamps to
  1–`kMaxLanes` and writes the current scene's `activeLaneCount`.
- `void poly_trigger_manual_fill(PolyContext)`: sets a `Context` latch that
  the next `poly_render` copies into `resolved.fillManualTrigger` and clears,
  so the fill covers exactly one render pass, as the plugin's does.

## Host

`webui/wasm-host.js` maps `fillEveryN` (`round(v * 64)`, matching the UI's
`/ 64` norm), `seedLock`, `activeLaneCount` and the `manualFill` action to
these, on both contexts per S01's pattern. `readLaneFromWasm` reports
`fillEveryN` and `seedLocked` so the UI's chips show real state.

## Verification

Engine unit tests cover each entry point (clamp, edge capture, one-pass
latch). A webui-e2e spec on the WASM host asserts each control changes the
rendered output of the playing context, and that a locked lane's output
survives a seed change. `wasm-freshness` passes on the rebuilt artifacts.
