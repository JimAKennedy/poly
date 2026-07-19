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
| P4 | M046 S02 (pending) | — | — | Writer-side TOCTOU across all six `notify()` handlers + `setState()`. Migration notes overclaim "atomic handshakes" — half the pattern is missing on the writer side. |

## Significant (P5-P12)

| ID | Slice | Fix commit(s) | Regression test(s) | Notes |
|----|-------|---------------|--------------------|-------|
| P5 | M046 S03 (pending) | — | — | Stuck-note path 1: `flushDue` lower bound stranding note-offs in inter-block crack under tempo ramps. |
| P6 | M046 S03 (pending) | — | — | Stuck-note path 2: `pendingNoteOffs_.push` return ignored at >512 pending. |
| P7 | M046 S04 (pending) | — | — | Controller reflects only Scene A on `setComponentState`; Scene B project loads show wrong knobs. |
| P8 | Deferred to M051 | — | — | Automation scene-addressing under chain playback — routed to M051 as a semantic/scope decision, not a bug fix. |
| P9 | M046 S05 (pending) | — | — | Loop-wrap treated as generic jump; wipes `captureBuffer_` every cycle. |
| P10 | M046 S06 (pending) | — | — | `kExportTrigger` never resets — dedupe-aware hosts drop the second click. |
| P11 | M048 (pending) | — | — | Parameter scaling math duplicated in 4-5 places. Anti-staleness milestone owns the registry that eliminates this. |
| P12 | M046 S07 (pending) | — | — | `IEventList` not sample-sorted. Cubase-tolerant but not host-portable. |

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
