---
class: gated
---

# M006/S01 — Edits reach the playing engine

**Slice:** central-africa M006/S01 (rows CA38–CA40)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [x] 1. The playback context is visible to tests, and a spec proves edits do not reach it
- [x] 2. Every edit and lane action reaches the playing engine
- [x] 3. Choosing a preset while playing plays it
- [ ] 4. The control audit asserts against what plays

## Definition of Done

Copied verbatim from the slice:

- [ ] Every control that changes the engine changes what is playing, without
      Stop and Play
- [ ] Choosing a preset while playing plays the new preset
- [ ] A test edits controls during playback and fails if the sound does not
      follow

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |
| `site-unit` | `npm --prefix site test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

`e2e` rewrites `webui/poly_engine.{js,wasm}`. This slice changes no engine
source, so after every `e2e` run restore them with
`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm` and stage
explicit paths only (CLAUDE.md).

## Background

`webui/wasm-host.js` keeps `engineCtx`, which the UI reads and edits, and
`playbackCtx`, created on Play by `_poly_copy_scenes(playbackCtx, engineCtx)`
and the only context `schedule()` renders. Macros, morph, chain, mute
(`active`) and scene select already write both. Everything else in `edit()`
(`INT_EDITS`, `FLOAT_EDITS`, `seed`) and in `action()` (`toggleStep`,
`setEuclid`, `setCells`, `setFixedStep`, `setMicroTiming`, `setEnvelope`,
`setAccent`, `fitMidi`, `revertImport`) writes `engineCtx` only, and
`applyPreset` writes `engineCtx` only. Re-copying scenes on each edit is
ruled out: `poly_copy_scenes` resets the chain to bar 0.

## Task 1 — The playback context is visible to tests, and a spec proves edits do not reach it

Rows: CA40 (exposure).

**Files**

- Modify `webui/wasm-host.js`: after `window.__polyEngineCtx = engineCtx;`,
  define `window.__polyPlaybackCtx` with `Object.defineProperty(window,
  '__polyPlaybackCtx', { get: () => playbackCtx, configurable: true })`, so
  tests see the context created on each Play.
- Create `webui/tests/playback-follows-edits.spec.mjs`
- Create `docs/plans/central-africa/evidence/M006-S01.md`

**Consumes:** nothing. **Produces:** the spec and its helpers, which Tasks 2
and 3 extend.

**Steps**

1. Write the spec on the WASM host, with the per-suite `python3 -m
   http.server` pattern of `scene-isolation.spec.mjs`. Load
   `index.html?host=wasm&preset=Deep%20House`, wait for `window.__polyModule`
   and a non-zero `window.__polyEngineCtx`, click Play and wait for a
   non-zero `__polyPlaybackCtx`. Helpers, evaluated in the page:
   - `laneConfig(ctx, lane)`: every `LaneFieldInt` (indices 0–14) and
     `LaneFieldFloat` (0–13) through `_poly_lane_int` / `_poly_lane_float`,
     plus `_poly_lane_pattern` for 64 steps.
   - `renderSignature(ctx)`: create a scratch context, `_poly_copy_scenes`
     from `ctx`, copy the six macros, render 16 quarters in
     `_poly_render` calls of 1 quarter at 44100 Hz / 512, collect
     `(ppq, note, velocity)` from `_poly_event_buffer`, destroy the scratch,
     and return the list as a string.
2. Cases, each asserting `laneConfig(playbackCtx, 0)` equals
   `laneConfig(engineCtx, 0)` after the edit:
   - edits through `PolyHost.edit`: `lane.0.velocity` 0.2, `lane.0.probability`
     0.3, `lane.0.hits` 0.1, `lane.0.steps` 0.2, `lane.0.rotation` 0.3,
     `lane.0.ghostFloor` 0.5, `lane.0.spread` 0.6, `lane.0.swing` 0.4,
     `lane.0.humanize` 0.3, `lane.0.note` 0.4, `lane.0.timeline` 1,
     `lane.0.mutationRate` 0.5, `lane.0.driftRate` 0.7, `lane.0.timingOffset`
     0.8, `lane.0.phraseLength` 0.5, `lane.0.tempoMult` 0.5,
     `lane.0.emphasisProb` 0.5, `lane.0.subdivision` 0.75;
   - actions through `PolyHost.action`: `toggleStep {lane:0, step:1}`,
     `setEuclid {lane:0, steps:12, hits:5, rotation:2}`,
     `setMicroTiming {lane:0, step:0, ms:8}`, `setEnvelope` adding a
     Velocity envelope at index 0, `setCells {lane:0, cells:[2,2,3]}`;
   - `seed` 0.42: `_poly_seed(playbackCtx) === _poly_seed(engineCtx)`.
   And two sound cases: VEL 0.2 and probability 0.3 each change
   `renderSignature(playbackCtx)`.
3. Run `npm --prefix webui test -- playback-follows-edits` and watch every
   case fail except any already mirrored; record which fail. VEL must fail:
   that is the reported defect.
4. Run the four tokens (restoring the artifacts after `e2e`) and confirm the
   only failures are this spec's. Do not commit a red `webui-e2e`: mark the
   spec's cases `test.fail()` with the comment "red until Task 2", so the
   suite is green and the expected-failure markers are what Task 2 removes.
5. Write the evidence with the red list. Tick this task. Commit with
   `Rows: CA40`.

## Task 2 — Every edit and lane action reaches the playing engine

Rows: CA38.

**Files**

- Modify `webui/wasm-host.js`
- Modify `webui/tests/playback-follows-edits.spec.mjs` (remove `test.fail()`)
- Append to the evidence

**Consumes:** Task 1's spec. **Produces:** the `forBoth` helper Task 3 uses.

**Steps**

1. Add, near `edit()`: `function forBoth(fn) { fn(engineCtx); if
   (playbackCtx) fn(playbackCtx); }`.
2. Rewrite `INT_EDITS` and `FLOAT_EDITS` entries to take the context:
   each `v => Module._poly_edit_lane_int(engineCtx, …)` becomes
   `v => forBoth((c) => Module._poly_edit_lane_int(c, …))`. Leave the
   `active` entry's behaviour unchanged, now through `forBoth`. Do the same
   for `seed` (`_poly_set_seed`).
3. In `action()`, route through `forBoth`: `toggleStep`, `setEuclid`,
   `setCells` (allocate the buffer once, call both, free once),
   `setFixedStep`, `setMicroTiming`, `setEnvelope`, `setAccent`. For
   `fitMidi`, after a successful `importMidi` on `engineCtx`, call
   `Module._poly_import_midi(playbackCtx, …)` with the same bytes when
   `playbackCtx` exists; for `revertImport`, call `_poly_revert_import` on
   both. Leave every existing dual-write as it is.
4. Remove the `test.fail()` markers and run the spec: every case passes.
5. Mutate: change `forBoth` to call only `engineCtx`, run, watch VEL and
   probability fail; restore. Record.
6. Run the four tokens. Append the evidence. Tick this task. Commit with
   `Rows: CA38`.

## Task 3 — Choosing a preset while playing plays it

Rows: CA39.

**Files**

- Modify `webui/wasm-host.js` (`applyPreset`)
- Modify `webui/tests/playback-follows-edits.spec.mjs`
- Append to the evidence

**Consumes:** `forBoth` from Task 2.

**Steps**

1. Add a case: while playing Deep House, `PolyHost.action('applyPreset',
   {index})` with the index of "Aka: Ngbolu" from `getState().presets`;
   assert `laneConfig(playbackCtx, 2)` equals `laneConfig(engineCtx, 2)`,
   `_poly_active_lane_count(playbackCtx) === 3`, and that
   `renderSignature(playbackCtx)` equals a signature rendered from a fresh
   context loaded with that preset alone.
2. Run it and watch it fail: the playing context still holds Deep House.
3. In `applyPreset`, apply `_poly_action_apply_preset_to_scene` through
   `forBoth` in both branches (index −1 and a real index). If the preset
   path sets macros on `engineCtx`, mirror them into `playbackCtx` the way
   `togglePlay` does.
4. Run and watch it pass. Mutate: drop `playbackCtx` from the preset apply,
   watch the case fail; restore. Record.
5. Run the four tokens. Append the evidence. Tick this task. Commit with
   `Rows: CA39`.

## Task 4 — The control audit asserts against what plays

Rows: CA40 (audit).

**Files**

- Modify `site/tests-e2e/control-audit.spec.ts`
- Append to the evidence
- Modify this plan and the ledger (DoD boxes, rows, slice status)

**Consumes:** `__polyPlaybackCtx` from Task 1.

**Steps**

1. In the control audit's label→effect section, for each engine-changing
   control it already exercises, add a check after the edit that the iframe's
   `__polyPlaybackCtx` lane config equals `__polyEngineCtx`'s (the
   `laneConfig` helper from Task 1, inlined in the spec), with a message
   naming the control. Add an explicit VEL check that the playing context's
   rendered velocities change.
2. Check the red: temporarily revert `forBoth` to `engineCtx` only, run
   `e2e`, and confirm the audit fails naming VEL; restore. Record.
3. Run the four tokens. Tick the DoD boxes here and in the ledger, set
   CA38–CA40 and the slice to `done`, run `jk-standards ledger`. Append the
   evidence. Tick this task. Commit with `Rows: CA40`.

## Self-review

- **DoD.** Controls follow playback → Task 2. Preset while playing → Task 3.
  A test that fails if sound does not follow → Tasks 1, 2 (spec) and 4
  (audit).
- **Rows.** CA38 → Task 2, verified by the spec's VEL/probability sound
  cases, red in Task 1. CA39 → Task 3. CA40 → Tasks 1 and 4, red on the
  pre-fix host in step 2 of Task 4.
- **Placeholders.** None.
- **Names.** `forBoth`, `laneConfig`, `renderSignature`,
  `__polyPlaybackCtx`, `playback-follows-edits.spec.mjs` throughout.
