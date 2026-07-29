---
class: gated
---

# M053 Cubase UAT Checklist — WebUI Parity Go-Signal

> **Status:** UAT script (human-run). Fill the Result/Notes columns in Cubase, then attach this file as the Integration + UAT verification-class evidence for M053 close.
> **Lifecycle:** Verification artifact — the manual go-signal the S05 decommission plan named. Only the milestone owner can run it (R10: the agent cannot drive Cubase).
> **Scope:** The 8 `close-in-webui` capability gaps closed in S03a–S03c, one representative Integration round-trip, and the G06 drag-export deferral note. The 7 cosmetic/viz-divergence rows (G09–G14, G16) are accepted documented divergences per the gap-closure plan — **not** tested here.

## How to run

1. Build/install the current `milestone/M053` WebUI-only plugin (ideally the **CI-produced binary** from a green cross-platform run, so you test the shipped artifact — see the "Preconditions" note).
2. Load Poly on an instrument track in Cubase.
3. Work each row below. Every check names the WebUI control, the gesture, and the pass condition. Mark **P** (pass) / **F** (fail) and jot anything surprising.
4. If any row fails, stop and report it — that flips M053 from evidence-pending to a real remediation slice.

## Preconditions

- [ ] Testing the **WebUI-only** build (no native VSTGUI editor present). The editor window is the web view.
- [ ] Ideally the binary under test is the CI cross-platform artifact from a green `milestone/M053` run (pluginval strictness 8 + full ctest + site e2e). If you test a local dev build instead, note that here.

---

## Part 1 — Capability-gap parity (the 8 closed gaps)

Each row: closed in the cited slice; native baseline is the behavior you had before, WebUI is what you're confirming now reaches parity.

| # | Gap | WebUI control & gesture | Pass condition (parity with old native) | Slice | Result | Notes |
|---|---|---|---|---|---|---|
| 1 | **G01 — cell size 1–16** | Open a lane's deep **pattern** pane; drag/stepper on a cell's size | Cell size sets any value **1–16** (not just the old 2→3→4 cycle); value persists on reopen | S03a | ☐ | |
| 2 | **G02 — inline lane rename** | Edit a lane's **name** field in the strip; type a new name, commit | Name round-trips: shows immediately, survives project **save → reopen** | S03a | ☐ | |
| 3 | **G03 — MIDI channel Auto/0** | Lane **channel** control; take it to its lowest position | Reaches **"Auto" / channel 0** (not just CH 1–16); persists | S03a | ☐ | |
| 4 | **G04 — envelope depth + reset** | Lane deep **env** pane; drag depth on an envelope; use the reset gesture | Depth edits **continuously** (not stuck at 0.3); reset gesture returns depth to its reset value | S03b | ☐ | |
| 5 | **G05 — GM-drum picker + note map** | Header **MAP** → note-map; use the GM-drum named picker + lane-scoped view | GM drum **names** are shown (not raw numbers only); lane-scoped rows selectable; remap round-trips | S03b | ☐ | |
| 6 | **G07 — capture length 1–32** | Export/capture cluster; **Bars** stepper | Sets any bar count **1–32** (not just the old {4,8,16,32} cycle) | S03c | ☐ | |
| 7 | **G08 — micro-timing double-click-zero** | Lane micro-timing (`.mtbars`); **double-click** a step | Double-click resets that step's offset to **0 ms** in one gesture | S03c | ☐ | |
| 8 | **G15 — phrase Gap/Offset gating** | `adv` pane; turn phrase **Length off** | **Gap** and **Offset** sliders become **disabled** while Length is off; re-enable when Length is on | S03c | ☐ | |

---

## Part 2 — Integration round-trip (representative workflow)

One end-to-end flow proving the WebUI-only build composes across load / edit / persist. This is the **Integration** verification class evidence.

| Step | Action | Pass condition | Result | Notes |
|---|---|---|---|---|
| 1 | Load a factory preset from the header dropdown | Preset applies; pattern audibly plays on transport | ☐ | |
| 2 | Edit a **macro** / scene morph slider | Change takes effect live | ☐ | |
| 3 | **Save** the Cubase project | Saves without error | ☐ | |
| 4 | **Close & reopen** the project | Poly reloads with **all edits intact** (preset, macro, plus any Part 1 edits you left in) | ☐ | |
| 5 | Confirm editor still renders after reopen | WebUI editor opens cleanly (no blank/white view) | ☐ | |

---

## Part 3 — G06 drag-export (deferred, spot-check only)

G06 is **built and merged** (S03d) but live drag-into-host is un-automatable and was verified only at compile/contract level. This is the **accepted deferral** — a spot-check is welcome but a failure here is a *known-limitation note*, not an M053 blocker.

- [ ] Trigger **Export** on a completed capture; a native drag-source window/affordance appears.
- [ ] Drag the `.mid` proxy into a Cubase track; a MIDI part lands with the captured notes.
- **If it doesn't drag cleanly:** record it as the deferred G06 live-verification item (already the documented divergence). Do **not** reopen the slice for this.

---

## Verdict

- [ ] **All Part 1 rows pass** → WebUI reaches capability parity with the retired native UI.
- [ ] **Part 2 round-trip passes** → Integration class satisfied.
- [ ] **Part 3** spot-checked (pass, or noted as deferred).

**Overall UAT verdict:** ☐ PASS (go-signal for M053 close) ☐ FAIL (name the failing row → remediation)

**Run by:** ___________  **Date:** ___________  **Build tested:** ☐ CI artifact ☐ local dev build

> On a clean PASS: this file becomes the Integration + UAT evidence. Combined with a green cross-platform CI run (Contract + Operational) and the G06 deferral note, M053 is clear for `gsd_complete_milestone`.
