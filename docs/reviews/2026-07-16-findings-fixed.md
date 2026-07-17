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
| P1 | M046 S01 | `c4042de` (red) · `0fa4ef4` (green) | `HostTests.SaveAfterStop_PreservesEdits` | `!tc_.playing` path in `getState()` early-return now republishes `stateSnapshot_` unconditionally before serializing. Red test was 0/128 slot match against the injected note map; post-fix 128/128. Snapshot publish is safe because the audio thread is idle in this path. |
| P2 | M046 S01 | `bc1afd1` | `HostTests.GetStateFromColdProcessor_RoundTrips` (guards) | Torn-state fallback at `processor.cpp:846` removed. `setActive(true)` now publishes an initial `stateSnapshot_` so cold `getState()` (host reload, restart) has a valid atomic snapshot to serialize. `writeSceneState(write, sceneState_)` no longer appears in `getState()` — verified by `rg`. |
| P3 | M046 S02 (pending) | — | — | `kDistributable` vs raw-pointer IPC in `uiSnapshot_`. Blocks any bridged-host use. |
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

## Change log

- **2026-07-17** — Index created. P1/P2 populated by M046 S01 T04.
