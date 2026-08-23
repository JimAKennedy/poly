---
class: gated
---

# M053 Cubase UAT Plan — WebUI Parity Go-Signal

> **Milestone numbering (legacy scheme):** the `M0xx` identifiers in this
> document — `M053` and any others it cites — belong to the repo's earlier
> commit-message milestone numbering. They do **not** correspond to the GSD
> milestone scheme (`M001` onwards) now used for planned work: a GSD `M001` is
> a different milestone from anything named here. Read these references as
> historical labels, not as pointers into the current roadmap.

> **Status (2026-07-28):** UAT script (human-run). Fill the Result/Notes columns in Cubase, then attach this file as the Integration + UAT verification-class evidence for M053 close.
> **Lifecycle:** Verification artifact — the manual go-signal the S05 decommission plan named. Only the milestone owner can run it (R10: the agent cannot drive Cubase).
> **Scope:** The 8 `close-in-webui` capability gaps closed in S03a–S03c, one representative Integration round-trip, and the G06 drag-export deferral spot-check. The 7 cosmetic/viz-divergence rows (G09–G14, G16) are accepted documented divergences per the gap-closure plan — **not** tested here.

This is the detailed procedural version. Each gap gives you: where to find the control, the exact gesture, what a **PASS** looks like, and the **fail signal** to watch for. Work top to bottom — Part 1 gaps are ordered so edits you make survive into the Part 2 save/reopen round-trip.

---

## Preconditions

- [ ] **Binary under test is the CI cross-platform artifact** from a green `milestone/M053` run (pluginval strictness 8 + full ctest + site e2e on macOS/Linux/Windows), OR a fresh local `scripts/build.sh --clean` build. Record which in the verdict block — a CI artifact is the stronger evidence.
- [ ] This is the **WebUI-only** build: the editor window *is* the web view. There is no native VSTGUI editor anymore (S05 deleted it). If you see the old fixed 600×870 native layout, you loaded a stale binary — reinstall.
- [ ] Latest install path (macOS): `~/Library/Audio/Plug-Ins/VST3/poly_plugin.vst3`. Rescan plugins in Cubase if the timestamp changed since your last session.
- [ ] Poly loaded on an **instrument track**, transport ready, a metronome or audible monitor available so pattern changes are audible.

## UI orientation primer (read once before Part 1)

The WebUI is a horizontal row of **lane strips**. Key affordances you'll use repeatedly:

- **Expand a lane** — each strip has an expand (`.ex`) button that opens the **deep pane** for that lane. The deep pane has tabs: **pattern**, **expr**, **env**, **adv**. Most Part 1 checks live in a deep pane.
- **Deep-pane tabs** — `pattern` (cells/steps/timeline), `expr` (velocity/ghost/spread/probability sliders), `env` (envelope manager), `adv` (phrase Length/Gap/Offset/Mutation/Drift).
- **Header** — preset dropdown (with category filter chips), **MAP** button (opens the note-map modal), scene A/B/Morph + morph slider + CHAIN.
- **Capture/Export cluster** — Bars stepper, Arm/Reset, Export (Save-As) chip, and the drag-to-DAW affordance (G06).
- **Gesture vocabulary:** single-drag = set value; double-click = reset-to-zero (micro-timing); right-click = reset (envelope depth). Values commit live to the engine via the bridge.

---

## Part 1 — Capability-gap parity (the 8 closed gaps)

Native baseline is the behavior you had before decommission; the WebUI column is what you're confirming now reaches parity. Leave your edits in place where noted — Part 2's save/reopen re-checks them.

### G01 — Cell size 1–16 (S03a)

- **Locate:** Expand a lane → **pattern** tab. Find a cell's size control.
- **Gesture:** Drag the cell size (or use its stepper) across its full range.
- **PASS:** Cell size reaches **any integer 1–16** — no longer the old 2→3→4 three-value cycle. Set one cell to an odd value (e.g. **7**) and leave it there for the Part 2 reopen check.
- **Fail signal:** The control still only cycles 2/3/4, or clamps below 16 / above 1, or won't hold a non-{2,3,4} value.

### G02 — Inline lane rename (S03a)

- **Locate:** The lane **name** field on the strip (previously display-only).
- **Gesture:** Click into it, type a new name (e.g. `KICK-A`), commit (Enter / blur).
- **PASS:** Name updates immediately on the strip and anywhere the lane label is echoed. Leave the rename in place for Part 2.
- **Fail signal:** Field is not editable, reverts on blur, or the typed name doesn't persist to the strip.

### G03 — MIDI channel Auto / 0 (S03a)

- **Locate:** The lane **channel** control (in the lane's deep pane / expr cluster).
- **Gesture:** Take the channel control to its **lowest** position, below CH 1.
- **PASS:** Reaches an **"Auto" / channel 0** position — not just CH 1–16. (Native mapped index 0 = Auto; the WebUI slider now includes it rather than mapping `(ch-1)/15` = 1–16 only.)
- **Fail signal:** Lowest position is CH 1; no Auto/0 reachable.

### G04 — Envelope depth editing + reset (S03b)

- **Locate:** Expand a lane → **env** tab. Add an envelope if none exists, then find its **depth**.
- **Gesture:** **Drag** the depth up and down; then **right-click** the depth to reset it.
- **PASS:** Depth edits **continuously** across its range (not frozen at the old fixed 0.3 on add); the **right-click reset** returns depth to its reset value in one gesture.
- **Fail signal:** Depth is display-only / stuck at 0.3, drag does nothing, or no reset gesture works.

### G05 — GM-drum named picker + lane-scoped note map (S03b)

- **Locate:** Header **MAP** button → note-map modal.
- **Gesture:** Open the drum picker for a mapping; use the lane-scoped view.
- **PASS:** GM drum **names** are shown (e.g. "Acoustic Snare", not just note number); the picker offers the named GM set; a lane-scoped row view is selectable; a remap round-trips (change one, confirm it sticks). WebUI still covers all 128 notes — that's a superset of native's 8, not a regression.
- **Fail signal:** Only raw note numbers, no GM names, or no lane-scoped view; remap doesn't apply.

### G06-capture-length note → **G07 — Capture length 1–32 (S03c)**

- **Locate:** The **Bars** control in the capture/export cluster.
- **Gesture:** Step/drag Bars across its range.
- **PASS:** Sets **any bar count 1–32** — no longer the old fixed {4, 8, 16, 32} cycle. Set an off-cycle value (e.g. **12** or **7**) and confirm it holds.
- **Fail signal:** Only cycles 4/8/16/32; won't set an intermediate value. (Note: this control was a silent no-op in shipping hosts before S03c — verify it *actually* changes the captured length, not just the label.)

### G08 — Micro-timing double-click-to-zero (S03c)

- **Locate:** A lane's micro-timing bars (`.mtbars`) — the per-step ±20 ms offset row.
- **Gesture:** Drag a step off-center to a non-zero offset, then **double-click** that step.
- **PASS:** Double-click snaps that step's offset back to **0 ms** in a single gesture (matching native's double-click reset), with the live ms readout confirming 0.
- **Fail signal:** Double-click does nothing; only a manual drag-to-center is possible.

### G15 — Phrase Gap/Offset gating (S03c)

- **Locate:** Expand a lane → **adv** tab. Find phrase **Length**, **Gap**, **Offset**.
- **Gesture:** Turn phrase **Length off**; observe Gap and Offset. Turn Length back on.
- **PASS:** With Length **off**, **Gap** and **Offset** sliders are **disabled** (greyed / non-interactive), matching native gating; they **re-enable** when Length is on.
- **Fail signal:** Gap/Offset stay active and editable while Length is off (the pre-fix behavior — editing inert params).

---

## Part 2 — Integration round-trip (representative workflow)

One end-to-end flow proving the WebUI-only build composes across load / edit / persist. This is the **Integration** verification-class evidence. Run it **after** Part 1 so your Part 1 edits (the cell size 7, the `KICK-A` rename, the capture length 12) are the state that must survive.

| Step | Action | Pass condition | Result | Notes |
|---|---|---|---|---|
| 1 | Load a factory preset from the header dropdown | Preset applies; pattern audibly plays on transport | ☐ | |
| 2 | Edit a **macro** / scene morph slider | Change takes effect live (audible / visible) | ☐ | |
| 3 | Re-apply your Part 1 edits if the preset load cleared them (cell=7, rename, capture=12) | Edits present in the session | ☐ | |
| 4 | **Save** the Cubase project | Saves without error | ☐ | |
| 5 | **Close & reopen** the project | Poly reloads with **all edits intact** (preset, macro, cell size, lane name, capture length) | ☐ | |
| 6 | Confirm editor still renders after reopen | WebUI editor opens cleanly — no blank/white view, no console-dead pane | ☐ | |

**Fail signal for the round-trip:** any Part 1 edit that silently resets on reopen (state-serialization gap), or a blank editor after reopen (webview lifecycle bug).

---

## Part 3 — G06 drag-export (deferred, spot-check only)

G06 is **built and merged** (S03d) and its Windows source now compiles cross-platform (confirmed in CI on `cd9a8f6`), but **live drag-into-host is un-automatable** and was verified only at compile/contract level. This is the **accepted deferral** — a spot-check is welcome, but a failure here is a *known-limitation note*, not an M053 blocker.

- [ ] Arm and complete a capture (capState reaches complete), then trigger **Export** → a native drag-source window/affordance appears.
- [ ] Drag the `.mid` proxy from that window into a Cubase track → a MIDI part lands with the captured notes.
- **If it doesn't drag cleanly:** record it as the deferred G06 live-verification item (already the documented divergence). Do **not** reopen the slice for this. The Save-As export path (the Export chip writing a `.mid` via the save dialog) is the always-available fallback and *should* work — test that too.

---

## Verdict

- [ ] **All 8 Part 1 gaps pass** → WebUI reaches capability parity with the retired native UI.
- [ ] **Part 2 round-trip passes** → Integration class satisfied.
- [ ] **Part 3** spot-checked (pass, or noted as deferred; Save-As fallback confirmed).

**Overall UAT verdict:** ☐ PASS (go-signal for M053 close) ☐ FAIL (name the failing gap → remediation)

**Run by:** ___________  **Date:** ___________  **Build tested:** ☐ CI artifact ☐ local dev build

**Failing gaps (if any):** ___________________________________________

> On a clean PASS: this file becomes the Integration + UAT evidence. Combined with the green cross-platform CI run on `cd9a8f6` (Contract + Operational) and the G06 deferral note, M053 is clear for `gsd_complete_milestone`.
