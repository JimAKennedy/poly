# M002 UAT Script — Core Groove Engine MVP

**Date:** 2026-06-20
**Plugin:** Poly (poly_plugin.vst3)
**DAW:** Cubase
**Tester:** Jim

## Setup

1. Cubase project at 120 BPM (default)
2. Add an Instrument Track with Poly as the instrument
3. Route Poly's MIDI output to a drum VSTi (Groove Agent, Battery, or Cubase GM drums)
   - In the Inspector: set Poly's MIDI output routing to the drum instrument's MIDI input
   - **Important:** Poly generates MIDI — it makes no sound on its own. You need a receiving instrument.
4. Open Poly's editor window

---

## Known Default State

When Poly loads fresh, the default patch is:
- **4 active lanes** (L1-L4), L5-L8 inactive
- **All lanes output MIDI note 36** (GM Kick) — this is a known gap, not a bug per se, but it means the default patch won't produce a multi-instrument groove without manual MIDI note mapping
- **4-step cycle, 4 hits, quarter-note subdivision** — every lane fires on every beat (straight pulse)
- **Macros at midpoint:** Complexity 0.5, Density 0.5, Syncopation 0.0, Swing 0.0, Tension 0.0, Humanize 0.0
- **Probability 1.0** on all lanes (every hit fires)

---

## Test Cases

### TC01: Plugin Loads Without Crash
**Steps:**
1. Insert Poly on an Instrument Track
2. Open the editor window

**Expected:**
- No crash, no error dialog
- Editor shows 8 lane rows (Kick through Crash)
- 6 macro knobs visible at bottom
- 8 velocity bars visible below knobs

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC02: Transport — MIDI Output on Play
**Steps:**
1. Position cursor at bar 1
2. Press Play
3. Watch the MIDI activity indicator on the receiving drum instrument
4. Press Stop

**Expected:**
- MIDI activity appears on the drum instrument when transport plays
- You should hear a steady kick pulse (every beat at 120 BPM)
- Activity stops when transport stops
- No stuck notes after stopping

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC03: Transport — Loop Restart
**Steps:**
1. Set a 4-bar loop (bar 1 to bar 5)
2. Press Play and let it loop at least 3 times
3. Listen for timing glitches or stuck notes at the loop point

**Expected:**
- Smooth, continuous pattern across loop boundaries
- No double-triggers at the loop restart point
- No stuck notes

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC04: Transport — Jump/Locate
**Steps:**
1. While playing, click in the ruler to jump to a different position (e.g., bar 17)
2. Do this 3-4 times to different positions

**Expected:**
- Pattern restarts cleanly from the new position
- No stuck notes from the old position
- No crash

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC05: Macro — Complexity Knob
**Steps:**
1. While transport plays, slowly turn the Complexity knob from minimum to maximum
2. Listen for changes in the pattern

**Expected:**
- **Low complexity (left):** Sparse pattern, fewer hits per cycle, minimal rotation
- **Mid complexity (center):** Default patch behavior (passthrough)
- **High complexity (right):** Denser patterns, more hits per cycle, pattern rotation changes creating polymetric feel
- Change should be audible and smooth (no clicks/glitches)

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC06: Macro — Density Knob
**Steps:**
1. While playing, turn Density from min to max

**Expected:**
- **Low density:** Fewer hits fire (probability reduced, hitCount drops)
- **Mid density:** No change from default
- **High density:** More hits fire (probability approaches 1.0, hitCount increases toward max)
- Pattern should get noticeably busier as you turn up

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC07: Macro — Syncopation Knob
**Steps:**
1. While playing with Complexity at ~0.7, turn Syncopation from 0 to max

**Expected:**
- Pattern rotation shifts, creating off-beat emphasis
- Accent emphasis shifts (emphasis probability inverts)
- The groove should feel more syncopated/off-kilter at higher values

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC08: Macro — Swing Knob
**Steps:**
1. While playing, turn Swing from 0 to max

**Expected:**
- Off-beat hits (odd steps) shift later in time
- At moderate values: triplet-ish swing feel
- At maximum: extreme shuffle (may sound flammy)

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC09: Macro — Tension Knob
**Steps:**
1. While playing, turn Tension from 0 to max

**Expected:**
- Velocity spread widens (hits vary more in loudness)
- Accent emphasis increases
- Pattern should feel more "dramatic" with louder accents and softer ghosts

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC10: Macro — Humanize Knob
**Steps:**
1. While playing, turn Humanize from 0 to max

**Expected:**
- Timing becomes slightly loose (hits shift slightly early/late)
- Velocity variation increases slightly
- At maximum: noticeable timing sloppiness (up to 10ms jitter)
- At minimum: quantized, machine-like

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC11: UI — Lane Grid Probability Bars
**Steps:**
1. Look at the lane grid in the editor while transport is stopped and while playing
2. Note which lanes show activity (blue probability bars)

**Expected:**
- L1-L4 should show as active (full probability bars)
- L5-L8 should show as inactive (dimmer/shorter bars or different appearance)
- The probability bars should reflect the probability parameter for each lane

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC12: UI — Velocity Bars
**Steps:**
1. Watch the velocity bars (L1-L8) at the bottom while transport plays

**Known Issue:** Currently, all lanes output MIDI on channel 0, so the velocity tracking only updates L1's velocity bar. L2-L8 velocity bars will likely remain dark even when those lanes are active. This is a known wiring gap — note it but don't block on it.

**Expected (current behavior):**
- L1 shows green velocity activity when transport plays
- L2-L8 may not show activity (known issue)

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC13: State Save/Load
**Steps:**
1. Move several macro knobs to non-default positions (e.g., Complexity to max, Swing to ~50%)
2. Save the Cubase project
3. Close the project
4. Reopen the project
5. Open Poly's editor

**Expected:**
- Macro knobs return to the saved positions
- Pattern output matches what was playing before save
- No crash on load

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC14: Preset Save/Load (if applicable)
**Steps:**
1. Set distinctive knob positions
2. Save as a VST3 preset via Cubase's preset menu
3. Reset knobs to default (reload plugin or use a different preset)
4. Load the saved preset

**Expected:**
- Knob positions restored
- Pattern output matches saved state

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC15: Stress — Rapid Knob Movement
**Steps:**
1. While playing, rapidly move multiple macro knobs simultaneously
2. Automate knobs via Cubase automation lanes with fast-moving ramps

**Expected:**
- No crash, no audio glitch
- Pattern responds to changing parameters
- CPU usage remains reasonable

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC16: Editor Open/Close Cycles
**Steps:**
1. While transport plays, close the editor window
2. Reopen the editor
3. Repeat 5 times

**Expected:**
- No crash
- Pattern continues playing when editor is closed
- Editor shows current state when reopened

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

### TC17: Multiple Instances
**Steps:**
1. Add a second instance of Poly on another track
2. Play both simultaneously

**Expected:**
- Both instances produce MIDI output independently
- No cross-talk between instances
- No crash

**Result:** ☐ Pass ☐ Fail
**Notes:**

---

## Known Gaps (Not Bugs for M002)

These are expected limitations of the MVP:

1. **All lanes default to MIDI note 36** — No per-lane note differentiation in the default patch. A proper default patch with Kick=36, Snare=38, HH=42, etc. is needed for M003.
2. **Velocity bars only show L1** — Engine outputs all notes on channel 0, so per-lane velocity tracking doesn't work correctly yet.
3. **No per-lane parameter controls in the editor** — The uidesc only has macro knobs, not per-lane probability/velocity/swing sliders.
4. **Lane active/inactive state not user-controllable** — activeLaneCount param exists but has no UI control.
5. **No visual pattern display** — The lane grid shows probability bars but not the actual Euclidean hit pattern.

## Summary

| # | Test | Result |
|---|------|--------|
| TC01 | Plugin loads | ☐ |
| TC02 | MIDI on play | ☐ |
| TC03 | Loop restart | ☐ |
| TC04 | Jump/locate | ☐ |
| TC05 | Complexity | ☐ |
| TC06 | Density | ☐ |
| TC07 | Syncopation | ☐ |
| TC08 | Swing | ☐ |
| TC09 | Tension | ☐ |
| TC10 | Humanize | ☐ |
| TC11 | Lane grid | ☐ |
| TC12 | Velocity bars | ☐ |
| TC13 | State save/load | ☐ |
| TC14 | Preset save/load | ☐ |
| TC15 | Rapid knobs | ☐ |
| TC16 | Editor open/close | ☐ |
| TC17 | Multiple instances | ☐ |
