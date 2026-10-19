---
class: gated
---

# M006/S05 — The playhead and the sound agree

**Slice:** central-africa M006/S05 (row CA49)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [x] 1. One step length for every factory preset
- [ ] 2. The WASM host's playhead lands on the sounding step

## Definition of Done

Copied verbatim from the slice:

- [ ] The playhead shows the step that is sounding, for every factory preset

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

After every `e2e` run restore `webui/poly_engine.{js,wasm}` with
`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm`.

## Background

`readLaneFromWasm` sets `stepLen = Math.round(8 / subdivision)` eighths and
ignores `tempoMultiplier`, so a 1/16 lane gets 1 instead of 0.5, a 1/12 lane
1 instead of 2/3, and a Tempo Mult lane its unscaled step. `cyc8`, `pump()`'s
step, the preview's `laneHitAt` and the step glyph all read `stepLen`. The
engine's step is `4 / subdivision / tempoMultiplier` quarter notes, which is
`8 / subdivision / tempoMultiplier` eighths.

## Task 1 — One step length for every factory preset

Rows: CA49 (formula).

**Files**

- Modify `webui/groove-math.js`: add `stepEighths(subdivision,
  tempoMultiplier = 1)` returning `8 / subdivision / (tempoMultiplier || 1)`,
  and export it on `window.PolyGrooveMath`.
- Modify `webui/wasm-host.js`: `stepLen = stepEighths(subdivision,
  tempoMultiplier)` in `readLaneFromWasm` (read the Tempo Mult float first).
- Create `webui/tests/playhead-cycle.spec.mjs`
- Create `docs/plans/central-africa/evidence/M006-S05.md`

**Steps**

1. Write the spec on the WASM host: for every preset in `getState().presets`,
   apply it and, for each lane of `getState().lanes` without cells, assert
   `cyc8(lane)` equals `steps × 8 / subdivision / tempoMultiplier`, where
   `subdivision` and `tempoMultiplier` are read straight from the engine
   (`_poly_lane_int` / `_poly_lane_float` on `__polyEngineCtx`), within
   1e-9. Collect failures by preset name and assert the list is empty.
2. Run it and watch it fail; record the number of presets that fail. The
   ledger expects 35.
3. Add `stepEighths` and use it in `readLaneFromWasm`.
4. Run and watch it pass. Mutate: restore `Math.round`, watch it fail again;
   restore. Record.
5. Run the three tokens. Write the evidence. Tick this task. Commit with
   `Rows: CA49`.

## Task 2 — The WASM host's playhead lands on the sounding step

Rows: CA49 (playhead, preview, glyph).

**Files**

- Modify `webui/wasm-host.js` (`pump()`: hold-the-clock test hook only if
  needed), `webui/groove-math.js` (`laneHitAt`), `webui/ui.js` (step glyph)
- Modify `webui/tests/playhead-cycle.spec.mjs`
- Append to the evidence; modify this plan and the ledger

**Steps**

1. Add cases:
   - Zande: Kponingbo on the WASM host: hold the host clock at four eighths
     (override `now8` through a test hook `window.__polyHoldClock8 = 4`, read
     by `now8` when set) and assert the next frame's lane 1 step is 0 (an
     8-step 1/16 lane completes one cycle every four eighths).
   - `laneHitAt` on a 16-step 1/16 lane with hits on every step reports a
     hit at tick 0 and at the tick of every step, not only every other one:
     walk the cycle in step-sized ticks.
   - The step glyph for a 1/16 lane does not read "♪" (eighth); it reads the
     sixteenth glyph or the subdivision as text.
2. Run and watch them fail.
3. Implement: add the `__polyHoldClock8` hook to `now8`; change `laneHitAt`
   to compute `step = Math.floor(tick / l.stepLen + 1e-9)` with the hit test
   `Math.abs(tick - step * l.stepLen) < 1e-6`, and make its callers in the
   WASM and mock hosts walk ticks of `min(1, stepLen)`; render the glyph from
   `l.subdivision` (`4` → ♩, `8` → ♪, `16` → 𝅘𝅥𝅯, `12` → "♪³").
4. Run and watch them pass, and run the existing preview and drift specs
   (`accent-preview`, `drift-math`, `drift-subdivision-viz`). Mutate: revert
   `laneHitAt`, watch its case fail; restore. Record.
5. Run the three tokens. Tick the DoD boxes here and in the ledger, set CA49
   and the slice to `done`, run `jk-standards ledger`. Append the evidence.
   Tick this task. Commit with `Rows: CA49`.

## Self-review

- **DoD.** Every factory preset → Task 1's walk; the sounding step →
  Task 2's held-clock case.
- **Rows.** CA49's two verifications → Task 1 (all presets, red on 35) and
  Task 2 (Kponingbo at four eighths); the preview's `laneHitAt` → Task 2.
- **Placeholders.** None.
- **Names.** `stepEighths`, `__polyHoldClock8`, `playhead-cycle.spec.mjs`.
