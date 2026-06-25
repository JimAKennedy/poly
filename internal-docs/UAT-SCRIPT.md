# Poly UAT Script

Manual acceptance test script for the Poly VST3 plugin UI.

**Setup:** Load Poly on a MIDI/Instrument track in Cubase. Route the track's output to a drum VSTi (e.g. Groove Agent, Battery) or leave unrouted if you only need to verify MIDI output in the event display.

---

## 1. Plugin Opens

- [ ] Open the plugin editor from the track inspector
- [ ] Window shows five sections top-to-bottom: Header, Lanes, Macros, Velocity, Visualization
- [ ] Header shows "POLY" in blue, "Select Preset..." in the dropdown, "1.0.0" on the right
- [ ] All 8 lanes visible (Kick, Snare, HH Closed, HH Open, Tom Hi, Tom Lo, Ride, Crash)
- [ ] All 8 lanes show colored backgrounds and probability bars (all active by default)
- [ ] Six macro knobs visible below the lanes (Complexity, Density, Syncopation, Swing, Tension, Humanize)
- [ ] Velocity section shows 8 bar slots labeled L1-L8
- [ ] Visualization section shows Envelope plot (left) and Phase Alignment circle (right)

## 2. Preset Dropdown

- [ ] Click the preset dropdown in the header — menu appears
- [ ] First item is "Init (All Lanes)", followed by a separator, then 5 factory presets
- [ ] Select "Four on the Floor" — header shows "Four on the Floor"
- [ ] Lanes update: L1-L4 active (colored), L5-L8 inactive (dark gray)
- [ ] Macro knobs update (Complexity and Density should show non-zero arcs)
- [ ] Select "Polymetric Drift" — header updates, lanes and macros change
- [ ] Select "Sparse Pulse" — only 3 lanes active (L1-L3)
- [ ] Select "Breakbeat" — 4 lanes active
- [ ] Select "Latin Feel" — 4 lanes active

### 2a. Init Reset

- [ ] After selecting any preset, open dropdown and select "Init (All Lanes)"
- [ ] Header shows "Init (All Lanes)"
- [ ] All 8 lanes become active again (colored backgrounds, probability bars visible)
- [ ] Macro knobs reset: Complexity and Density at 50%, Syncopation/Swing/Tension/Humanize at 0%
- [ ] This confirms you can get back to the default state after any preset

## 3. Lane Interaction

### 3a. Toggle Active

- [ ] Click the lane name text (e.g. "Tom Hi") on any active lane — it goes inactive (dark gray, dimmed text)
- [ ] Click it again — it reactivates (colored background returns)
- [ ] Toggling a lane off/on is reflected in velocity bars and visualization sections

### 3b. Probability Drag

- [ ] Click in the colored probability bar area (to the right of the lane name) on any lane
- [ ] Drag left/right — the bar width changes (shorter = lower probability, longer = higher)
- [ ] During playback, a lane with ~50% probability should audibly skip roughly half its hits
- [ ] Setting probability to near-zero should silence the lane almost entirely

## 4. Macro Knobs

- [ ] Click and drag up on the Complexity knob — arc grows clockwise
- [ ] Drag down — arc shrinks
- [ ] Each of the 6 knobs is independently adjustable
- [ ] Macro changes take effect immediately during playback (no need to restart transport)
- [ ] Verify Swing knob: at 0% the pattern is straight; increase Swing and listen for shuffle feel
- [ ] Verify Humanize knob: at 0% timing is mechanical; increase to hear looseness in timing

## 5. Playback — Velocity Display

- [ ] Press Play in Cubase transport
- [ ] Velocity bars pulse for active lanes when notes trigger
- [ ] Bar height corresponds to note velocity (loud = tall, quiet = short)
- [ ] Bar colors match lane colors (L1 blue, L2 orange, L3 green, L4 red, etc.)
- [ ] Inactive lanes show no velocity bars (dark gray slots)
- [ ] Select "Init (All Lanes)" and play — all 8 velocity bars should show activity
- [ ] Select a 4-lane preset and play — only L1-L4 should pulse

## 6. Playback — Envelope Visualization

- [ ] During playback, the Envelope panel (bottom-left) shows sine curves
- [ ] Colored vertical lines and dots move across the plot for each active lane
- [ ] Each active lane has a distinct color matching the lane color table
- [ ] Dots travel along the curves showing the current envelope value
- [ ] With 4 lanes active, you see 4 overlaid curves; with 8, you see 8

## 7. Playback — Phase Alignment

- [ ] During playback, the Phase Alignment panel (bottom-right) shows concentric orbit rings
- [ ] Each active lane has a ring with a colored dot orbiting at the lane's cycle rate
- [ ] Dots orbit at different speeds due to different cycle lengths (polymetric!)
- [ ] Labels (L1, L2, etc.) follow their dots around the rings
- [ ] When multiple dots align at the top of their rings, lanes are "in phase" (downbeats coincide)
- [ ] With "Polymetric Drift" preset: orbits should be noticeably different speeds (prime-number cycles)

## 8. Cross-Section Color Consistency

- [ ] Verify that lane colors are consistent across all sections:
  - L1 (Kick) = blue everywhere (lane bar, velocity bar, envelope curve, phase dot)
  - L2 (Snare) = orange everywhere
  - L3 (HH Closed) = green everywhere
  - L4 (HH Open) = red everywhere
  - L5 (Tom Hi) = purple (visible when active via Init preset)
  - L6 (Tom Lo) = teal
  - L7 (Ride) = dark orange
  - L8 (Crash) = light blue

## 9. MIDI Output

- [ ] With the track routed to a drum VSTi, press Play — you should hear drums
- [ ] Stop playback — sound stops, velocity bars go still
- [ ] Resume playback — pattern picks up from the new transport position (no stuck notes)
- [ ] Enable loop in Cubase and play through a loop boundary — pattern continues without glitches
- [ ] Change tempo mid-playback — pattern adjusts timing immediately

## 10. State Persistence

- [ ] Set up a custom configuration: toggle some lanes off, drag probability bars, adjust macros
- [ ] Save the Cubase project
- [ ] Close and reopen the project
- [ ] Open the plugin editor — verify your custom lane/macro settings are restored
- [ ] Preset dropdown should show "Select Preset..." (since the state was manually configured, not loaded from a preset)

---

## Known Limitations

- Macro knobs, velocity bars, and visualization sections are **not clickable for fine-tuning** — macros use drag gestures; velocity and visualization are read-only monitors
- Lane parameters beyond probability (velocity, swing, humanize, etc.) are only adjustable via Cubase's automation lanes or generic editor — no in-plugin sliders yet
- Active Lane Count is controlled implicitly by presets or the Init option; there's no dedicated slider in the UI (available via DAW automation at param ID 300)
- Factory presets don't appear in Cubase's own preset browser — use the in-plugin dropdown
