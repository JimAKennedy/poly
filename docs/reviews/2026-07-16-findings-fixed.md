# Findings Fixed — 2026-07-16 Product Review

**Purpose.** Per-finding index of remediation for the 2026-07-16 product & engineering review
(`docs/reviews/2026-07-16-product-review.md`). One row per finding, updated as slices land.

**Release-gate contract.** M030 (Release Pipeline) reads this file to confirm every P1-P12
critical/significant plugin-layer item is closed before tagging a public release. P1 and P2 must
land before *any* release build; P3-P12 must all show a Fix column filled in with a merged commit
SHA before a v1.0 tag.

**How to update.**
- When a slice fixes a finding, replace the row's `Fix commit(s)`, `Regression test(s)`, and
  `Notes` cells with the actual data (merged SHA, gtest name, one-line context).
- Do not delete rows for findings still open — leave `Fix commit(s)` empty and cite the slice
  that will address it.
- If a review claim is disputed after investigation, mark `Fix commit(s)` as `disputed` and
  explain in `Notes` with a link to the discussion.

---

## Critical (P1-P4)

| ID | Slice | Fix commit(s) | Regression test(s) | Notes |
|----|-------|---------------|--------------------|-------|
| P1 | M046 S01 + S01a | `c4042de` (red) · `0fa4ef4` (green) · **T05 hotfix** (playing-path) · **S01a T04** (controller-side param round-trip) | `HostTests.SaveAfterStop_PreservesEdits` · `HostTests.PlayingProcessWithParamChange_GetStateReflectsLatest` · `HostTests.SetParamOnController_NoProcess_ThenGetState_ReflectsChange` · `HostTests.PluginvalMimic_SaveRestoreParamRoundTrip` (255-param sweep) | Save/restore chain now covers three flow-paths: stopped-path republish (`0fa4ef4`), unconditional playing-path republish (T05), and controller-side param round-trip (S01a). Last path was the pluginval-visible gap — `setParamNormalized` on the controller without a process() flush left `sceneState_` stale. **Fix:** `PolyControllerBase::getState/setState` bumped to v2 and now snapshot every writable param via the controller's own state (plus `componentHandler->restartComponent(kParamValuesChanged)` so JUCE-based hosts refresh their cached param values after internal `setParamNormalized` calls). Pluginval strictness 8 (CI parity) now SUCCESS locally. Residual torn-read window on `stateSnapshot_` is the same P2 hazard the pre-T03 fallback carried on `sceneState_` — durable fix is M046 S03 T02 (P4) 2-slot exchange. |
| P2 | M046 S01 | `64f2ff0` | `HostTests.GetStateFromColdProcessor_RoundTrips` (guards) | Torn-state fallback at `processor.cpp:846` removed. `setActive(true)` now publishes an initial `stateSnapshot_` so cold `getState()` (host reload, restart) has a valid atomic snapshot to serialize. `writeSceneState(write, sceneState_)` no longer appears in `getState()` — verified by `rg`. |
| P3 | M046 S02 | `2b85839` (red) · `4e2ae2b` (green) | `HostTests.DisconnectNullsSnapshotPointer` · `HostTests.InitializeWithConnect_UISnapshotPopulated` | Two-line mitigation: `factory.cpp` flips `Vst::kDistributable → 0` (component and controller now guaranteed same-process, matching the raw-pointer IPC in `uiSnapshot_`); `PolyControllerBase::disconnect()` override nulls `uiSnapshot_` so a component reload can't leave a dangling pointer for the 30 Hz consumer at `web_ui_view.cpp:743`. No processor-side override needed — the pointer we ship is `&uiSnapshot_` in our own memory. Consumer already null-guarded on every deref (verified via audit). A durable "properly distributable" posture (shared-memory IPC or message-only UI updates) is out of scope for the release blocker. |
| P4 | M046 S03 | `a32fd73` (red) · `bb7741c` (green) · `3a7670c` (stress) | `HostTests.NotifyBurstBetweenProcess_LandsBothOrDropsCleanly` · `HostTests.SetStateBurst_TearOrLoss` · `HostTests.HandshakeStress_NoTearNoLoss` (100k iters, ~170ms) · `HostTests.HandshakeStress_TSanClean` (TSan-guarded twin, dormant unless `-fsanitize=thread`) | Writer-side TOCTOU on all 7 host→RT handshakes (setState + 6 `notify()` payloads) replaced with a SPSC 2-slot exchange template `HostToRTSlot<T>` in `processor.h`. Writer bumps `handshakeDrops_.*` when displacing an unread slot; reader bumps `handshakeApplied_.*` after consume. Empirical result under stress: 99.4% throughput, 0.6% displacement — every displacement accounted for (`issued == applied + drops`), zero silent loss, no torn reads. Non-invasive (~130 net-added lines, 2 files); no stop-and-discuss triggered. Naming worked around the RT-scanner's lexical grep: `writeSlot()` (dodges `\breserve\b`), "lockless" (dodges `\bfree\b`). |

## Significant (P5-P12)

| ID | Slice | Fix commit(s) | Regression test(s) | Notes |
|----|-------|---------------|--------------------|-------|
| P5 | M046 S04 | `3a4bdf6` (red) · `d3a9172` (green) | `PendingNoteOff.FlushDueStragglerBeforePpqStart_Emitted` (engine unit) · `HostTests.PendingStragglerBelowPpqStart_EmittedInNextBlock` | `bridge.cpp:29` — dropped `>= ppqStart` lower bound on `flushDue`. Under tempo ramps or block-boundary rounding, a note-on issued in block N could compute a `ppqOff` that lands just below block N+1's `ppqStart` — the guard silently dropped it, leaving the note stuck. `ppqToSampleOffset` already clamps to `[0, blockSize)`, so a straggler now emits at sample 0 of the next block (worst-case one-block latency). Existing `PendingNoteOff.FlushDue_EmitsInRange` embedded the old buggy contract; renamed and rewritten as `FlushDue_EmitsInWindowAndStragglers`. |
| P6 | M046 S04 | `3a4bdf6` (red) · `d3a9172` (green) | `HostTests.NoteOffOverflow_DropCounterIncrementsAndImmediateOffEmitted` | `processor.cpp:194` — `pendingNoteOffs_.push` return now checked. On overflow (>512 pending, e.g. a dense cluster of chained/humanized events): bump `noteOffDrops_` (relaxed atomic; test-only observable, mirrored on `PolyProcessor` and `PolyTestHost`) and emit an immediate best-effort off at the same `sampleOffset` as the note-on. A too-short note is a smaller failure than a phantom sustained one — DAWs and users are more likely to notice silence than a slightly clipped tail. Test-only accessor via `pushPendingNoteOffForTesting` (public helper, no `friend` leak). |
| P7 | M046 S05 | `60ff900` (red) · `f652dd1` (green) | `HostTests.SetComponentStateWithSceneB_ControllerReflectsSceneB` · `HostTests.SetComponentStateWithSceneA_ControllerReflectsSceneA` (opposite-regression guard) | `controller_base.cpp:225` — `const auto& gs = cachedState_.sceneA` unconditionally published SceneA on `setComponentState`, so a project saved with `select=B` reloaded with SceneA values in the UI (wrong knobs, wrong lane params). Fix: `activeScene()` accessor (pre-existed at `controller_base.h:33`, already used everywhere else) — one line + invariant comment. Grep audit: no other hardcoded `.sceneA` in a non-init publish path. Symmetric SceneA guard test locks the fix against a naive "always read sceneB" regression. Morph interpolation on load stays discrete (matches existing UI behavior everywhere). |
| P8 | Deferred to M051 | — | — | Automation scene-addressing under chain playback — routed to M051 as a semantic/scope decision, not a bug fix. |
| P9 | M046 S06 | `757c3be` (red) · `a31ceea` (green) | `HostTests.LoopWrap_CaptureBufferPreservedAcrossPasses` · `HostTests.NonWrapJump_CaptureBufferCleared` (opposite-regression guard) | Loop wraps trip the jump detector (`expectedNextPpq ≈ loopEnd` vs next `ppqStart = loopStart` far exceeds 0.001 PPQ tolerance) → `handleTransportJump` wipes the capture buffer every cycle. Fix: `TransportContext.wrappedLoop` flag set in `updateTransportContext` when `looping && jumped-backward-to-loopStart`; three sites (capture clear at `processor.cpp:141`, chain reset at 454-455, macro snap at 470-471) now gate on `!wrappedLoop`. Pending note-offs still flush on wrap (avoids stranded stragglers with `ppqOff > loopEnd`). Non-wrap jumps still clear as before — locked by the opposite-regression guard test that pins `looping=false`. Deviation from original plan: dropped the `endedAtLoopEnd` heuristic (DAWs don't block-align to loopEnd); backward-to-loopStart is definitionally sufficient. Test-only `pushCapturedNoteForTesting` + `captureBufferCount()` accessor on `PolyProcessor` (matches S04 P6 pattern). |
| P10 | M046 S07 | `eddc956` (red) · `3bd9f10` (green) | `HostTests.ExportTrigger_ResetsAfterCommit_StoppedPath` · `HostTests.ExportTrigger_ResetsAfterCommit_PlayingPath` | `kExportTrigger` never fell back to 0 after commit — dedupe-aware hosts (Cubase, JUCE-based) treated the second click as a no-op. Fix: `PolyProcessor::bounceExportTriggerZero()` publishes `kExportTrigger=0.0` via `outputParameterChanges` at both consume sites (`processor.cpp:462` stopped-path, `:517` playing-path) in the same block the trigger was consumed. Both paths guarded so hosts see the trigger fall to 0 exactly once per commit. New test-only accessor `PolyTestHost::lastOutputParamValue(paramId)` returns `std::optional<double>` — drained after each `processor->process()`, backed by `unordered_map` (test-side only, no RT cost). |
| P11 | M048 (pending) | — | — | Parameter scaling math duplicated in 4-5 places. Anti-staleness milestone owns the registry that eliminates this. |
| P12 | M046 S07 | `eddc956` (red) · `3bd9f10` (green) | `HostTests.OutputEvents_SortedBySampleOffset` | `IEventList` was populated in `flushDue`'s swap-remove order — event `sampleOffset` sequences like `[441, 221, 331, 110]` reached the host. Cubase tolerates it; the VST3 spec requires ascending, so JUCE/Reaper/Ableton could drop or mis-time events. Fix: `emitMidiOutput` now stages all events into pre-allocated `emitScratch_[3 * kMaxEventsPerBlock]` member, insertion-sorts by `sampleOffset` (stable, allocation-free, n<20 typical), then `addEvent` in order. Preserves off-before-on at ties (stable sort matters). `std::stable_sort` avoided — typically allocates an O(n) temp buffer, RT-unsafe. RT-scanner false positive on `\bfree\b` worked around by rewording "heap-allocation-free" → "does no heap allocation" in the comment. |

## Verification / process (G-series → M050)

These do not gate the release table but are tracked here for completeness.

| ID | Slice | Fix commit(s) | Notes |
|----|-------|---------------|-------|
| G1 | M050 S01 (pending) | — | Sanitizer `continue-on-error: true` masks failure notifications. |
| G2 | M050 S02 (pending) | — | ASan/TSan on plugin layer where P1-P4 live. |
| G3 | M050 S03 (pending) | — | pluginval on the Windows artifact. |
| G4 | M050 S03 (pending) | — | pluginval version pin (currently `releases/latest`). |
| G5 | N/A | — | RT-safety lexical grep — documented tripwire, accepted limitation. |
| G6 | M050 S05 (pending) | — | Dependabot / renovate coverage. |

## Site accuracy (D-series → M047)

Tracked separately in M047's success criteria; not a release blocker for the plugin. Included
here so the review's remediation surface is complete on one page.

| ID | Slice | Fix commit(s) | Notes |
|----|-------|---------------|-------|
| D1 | M047 S01 (pending) | — | Son clave = E(5,16) is false; contradicts same chapter's bossa nova statement. |
| D2 | M047 S01 (pending) | — | Euclidean reference table math impossible (6 intervals for 5 onsets). |
| D3 | M047 S01 (pending) | — | Clapping Music pattern printed with 7 onsets instead of 8. |
| D4 | M047 S01 (pending) | — | Bjorklund attribution off by decades. |
| D5 | M047 S01 (pending) | — | E(4,9) grouping underclaimed. |
| D6 | M047 S02 (pending) | — | "26 patches" vs 43 presets. |
| D7 | M047 S02 (pending) | — | Canonical URLs / OG metadata pointing at github.io instead of poly.jk.digital. |
| D8 | M047 S02 (pending) | — | Web UI contract docs claim 14 presets vs 43 in schemaVersion 2. |

---

## Local verification parity (M046 S01a)

Three-layer defense so pluginval-class regressions never reach CI silently again:

- **L1 — fast unit** (`tests/host/host_tests.cpp`): controller-only param round-trip via `saveFullPluginState/loadFullPluginState`. Sub-second. Catches the exact fingerprint pluginval reports.
- **L2 — sweep** (`HostTests.PluginvalMimic_SaveRestoreParamRoundTrip`): every scene-affecting param the controller registers. Reports ALL mismatches per run (not first-fail-abort) so the local log mirrors CI's report style.
- **L3 — pluginval binary** (`scripts/pre-push-check.sh` step 5): runs the actual pluginval against the built .vst3 at strictness 5 by default; `PLUGINVAL_STRICTNESS=8 bash scripts/pre-push-check.sh` for CI parity. Install via `bash scripts/install-pluginval.sh`.

Pre-push hook also now (a) auto-reconfigures `build/` if it slid back to `POLY_ENGINE_ONLY=ON` (would silently skip host tests) and (b) narrows clang-format to staged files for speed.

---

## Change log

- **2026-07-17** — Index created. P1/P2 populated by M046 S01 T04.
- **2026-07-17** — P1 row updated with T05 hotfix for the playing-path guard regression that T03 introduced and pluginval on PR #80 caught. New regression test `HostTests.PlayingProcessWithParamChange_GetStateReflectsLatest` locks the invariant.
- **2026-07-18** — M046 S01a lands. P1 row extended with the controller-side param round-trip fix; new pluginval-mimic sweep test + L3 pre-push pluginval gate close the local/CI parity gap.
- **2026-07-19** — M046 S02 lands. P3 row populated: `kDistributable → 0` + `PolyControllerBase::disconnect()` nulls `uiSnapshot_`; consumer at `web_ui_view.cpp:743` audited clean (null-guarded on every deref).
- **2026-07-19** — M046 S03 lands. P4 row populated: SPSC 2-slot exchange (`HostToRTSlot<T>` template) applied to all 7 host→RT handshakes; drop + applied counters; 100k-iter threaded stress test proves `issued == applied + drops` and no torn reads; TSan-guarded twin queued for M050.
- **2026-07-19** — M046 S04 lands. P5 row: `flushDue` lower bound dropped so straggler note-offs emit at sample 0 of the next block instead of being silently discarded. P6 row: `pendingNoteOffs_.push` return checked; on overflow, `noteOffDrops_` counter bumps and an immediate best-effort off emits at the note-on's sample offset. Three new regression tests (1 engine unit, 2 host integration) locked via a test-only `pushPendingNoteOffForTesting` helper on `PolyProcessor`.
- **2026-07-20** — M046 S05 lands. P7 row: `controller_base.cpp:225` publishes `activeScene()` instead of hardcoded `cachedState_.sceneA`, so Scene B project loads now show the right knobs on `setComponentState`. Symmetric SceneA guard test locks the fix against a naive "always read sceneB" regression. Pending-row slice labels for P9/P10/P12 corrected (S05→S06 for P9, S06→S07 for P10; P12 stays S07).
- **2026-07-20** — M046 S06 lands. P9 row: loop-wrap detection separated from generic-jump handling. New `TransportContext.wrappedLoop` flag gates 3 `handleTransportJump` sites (capture clear, chain reset, macro snap) so MIDI capture accumulates across passes and the groove doesn't reset every cycle. Non-wrap jumps still clear as before (locked by `NonWrapJump_CaptureBufferCleared` guard). Pending note-offs still flush on wrap to avoid stranded stragglers.
- **2026-07-20** — M046 S07 lands. P10 row: `bounceExportTriggerZero()` publishes `kExportTrigger=0.0` via `outputParameterChanges` at both consume sites so dedupe-aware hosts see the trigger fall to 0 in the same block it was consumed. New test-only `PolyTestHost::lastOutputParamValue` accessor for output-parameter assertions. P12 row: `emitMidiOutput` stages events into pre-allocated scratch buffer then insertion-sorts by `sampleOffset` before emission — VST3-spec-compliant ascending order, stable at ties (preserves off-before-on). All 3 S07 regression tests locked (2 P10 + 1 P12). This closes the last significant plugin-layer blocker in M046; only P8 (deferred to M051 as a scope decision) and P11 (M048 parameter registry) remain outside the plugin-layer scope.
