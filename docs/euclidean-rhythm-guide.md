# Poly — Euclidean Rhythm Guide

A hands-on guide to every rhythmic feature in Poly, with UI references, parameter ranges, and sample values you can dial in to hear each concept.

## UI Overview

```
┌──────────────────────────────────────────────────────────────────────┐
│  POLY                    [Preset ▾]                        v1.0.0   │  ← A: Header / Preset Selector
├──────────────────────────────────────────────────────────────────────┤
│ LANES                                                               │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │  Kick      ████████████████████████████████░░░░░░░░░░░░░░░  ◎  │ │  ← B: Lane Grid
│ │  Snare     ██████████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │     (8 rows, one per lane)
│ │  HH Closed █████████████████████████████████████████████░░  ◎  │ │     Bar = probability
│ │  HH Open   ████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │     ◎ = phase indicator
│ │  Tom Hi     ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │
│ │  Tom Lo     ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │
│ │  Ride       ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │
│ │  Crash      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ◎  │ │
│ └──────────────────────────────────────────────────────────────────┘ │
│ LANE EDIT                                                           │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │ [1][2][3][4][5][6][7][8]   PATTERN             │ VOICE          │ │  ← C: Lane Edit
│ │  Lane tabs             (Steps)(Sub)(Hits)(Rot)(Note)│(Vel)(Gho)  │ │     Tabs select lane
│ │                                                │(Spr)(Swg)(Kot) │ │     10 rotary knobs
│ └──────────────────────────────────────────────────────────────────┘ │
│ PHRASE                                                              │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │ [1][2][3][4][5][6][7][8]  ┌─arc─┐    (Len)(Gap)(Ofs)           │ │  ← D: Phrase Editor
│ │  Lane tabs    Schematic   └─────┘    (Mut)(Drift)(Time)        │ │     6 rotary knobs
│ └──────────────────────────────────────────────────────────────────┘ │
│ MACROS                                                              │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │  (Complexity)  (Density)  (Syncopation)  (Swing)  (Tension)     │ │  ← E: Macro Knobs
│ │                                                       (Humanize)│ │     6 global knobs
│ └──────────────────────────────────────────────────────────────────┘ │
│ VELOCITY                                                            │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │  █  █  █  █  ░  ░  ░  ░                                        │ │  ← F: Velocity Monitor
│ │  L1 L2 L3 L4 L5 L6 L7 L8                                      │ │     Real-time per-lane bars
│ └──────────────────────────────────────────────────────────────────┘ │
│ VISUALIZATION                                                       │
│ ┌────────────────────────────────────────┐ ┌──────────────────────┐ │
│ │                                        │ │    ╭───╮             │ │
│ │   Envelope curves (all active lanes)   │ │  ╭─╯   ╰─╮   Phase  │ │  ← G: Envelope Curves
│ │   X: phase (0→1)  Y: value (0→1)      │ │  │  · ·   │  Rings  │ │  ← H: Phase Alignment
│ │                                        │ │  ╰─╮   ╭─╯         │ │     Concentric circles
│ │              Envelope                  │ │    ╰───╯             │ │     with phase dots
│ └────────────────────────────────────────┘ └──────────────────────┘ │
└──────────────────────────────────────────────────────────────────────┘
```

### Control Reference

| Zone | Control | Type | What It Does |
|------|---------|------|-------------|
| **A** | Preset dropdown | Selector | Load factory or Init preset; resets all parameters |
| **B** | Lane rows | Click + drag | Click to select lane; drag the bar to set probability |
| **C** | Lane tabs [1]-[8] | Click | Select which lane the 10 Lane Edit knobs edit |
| **C** | Steps | Rotary knob | Euclidean cycle length (1–64 steps) |
| **C** | Subdiv | Rotary knob | Grid resolution (1/1, 1/2, 1/4, 1/8, 1/16) |
| **C** | Hits | Rotary knob | Number of evenly-distributed hits (0–64) |
| **C** | Rot | Rotary knob | Cyclic rotation of the pattern (0–63) |
| **C** | Note | Rotary knob | MIDI note number (0–127, displayed as pitch e.g. "C2") |
| **C** | Vel | Rotary knob | Base velocity (0–127) |
| **C** | Ghost | Rotary knob | Minimum ghost note velocity (0–127) |
| **C** | Spread | Rotary knob | Random velocity range (0–100%) |
| **C** | Swing | Rotary knob | Per-lane swing amount (0–100%) |
| **C** | Kotek | Rotary knob | Kotekan source lane (off, L1–L8) |
| **D** | Lane tabs [1]-[8] | Click | Select which lane the 6 phrase knobs edit |
| **D** | Len / Gap / Ofs | Rotary knobs | Phrase length, gap, and offset (0–64 beats) |
| **D** | Mut / Drift / Time | Rotary knobs | Mutation rate (0–100%), drift (±4 st/bar), timing offset (±20ms) |
| **D** | Schematic | Display | Visual arc showing phrase on/off timing for selected lane |
| **E** | Complexity | Rotary knob | Scales hit count and rotation (0 = simple, 1 = complex) |
| **E** | Density | Rotary knob | Scales probability and hit count (0 = sparse, 1 = full) |
| **E** | Syncopation | Rotary knob | Rotates pattern, inverts emphasis (0 = on-beat, 1 = off-beat) |
| **E** | Swing | Rotary knob | Shifts odd steps forward for shuffle feel (0 = straight, 1 = max) |
| **E** | Tension | Rotary knob | Widens velocity spread, scales accent + envelope depth |
| **E** | Humanize | Rotary knob | Adds timing jitter (up to 10ms) and velocity randomness |
| **F** | Velocity bars | Display | Real-time velocity output per lane (read-only) |
| **G** | Envelope curves | Display | Sinusoidal envelope phase per lane over time |
| **H** | Phase rings | Display | Concentric polar view — dots show each lane's cycle position |

---

## 1. The Euclidean Algorithm

Poly distributes **k** hits across **n** steps using the Bjorklund/Euclidean algorithm. The result is the most evenly-spaced rhythm possible for a given density.

**How it works:** Step *i* is a hit when `(i × k) mod n < k`. The **rotation** parameter shifts the entire pattern cyclically.

### Key Parameters (Lane Edit → PATTERN group, zone C)

| Knob | Range | Description |
|------|-------|-------------|
| **Steps** | 1–64 | Total steps in the pattern (n) |
| **Subdiv** | 1/1 – 1/16 | Grid resolution: whole/half/quarter/8th/16th notes |
| **Hits** | 0–steps | Number of active hits distributed evenly (k) |
| **Rot** | 0–63 | Cyclic shift of the generated pattern |

### Try It: Basic Euclidean Patterns

Start from **Init** (preset dropdown → Init) so all parameters are at defaults.

Select a lane tab in zone C, then dial the PATTERN knobs:

| Pattern Name | Steps | Hits | Rot | Musical Effect |
|-------------|-------|------|-----|---------------|
| Tresillo | 8 | 3 | 0 | Classic Afro-Cuban cell: `x..x..x.` |
| Cinquillo | 8 | 5 | 0 | Dense Cuban rhythm: `x.xx.xx.` |
| Son clave 3-side | 8 | 3 | 3 | Rotated tresillo: `..x..x.x` |
| 7 in 12 | 12 | 7 | 0 | West African bell: `x.xx.x.xx.x.` |
| 5 in 16 | 16 | 5 | 0 | Bossa nova feel: `x..x..x..x..x..` |
| 4 in 4 | 4 | 4 | 0 | Four-on-the-floor |

*Use the **Complexity** and **Density** macros (zone E) to reshape hit counts and rotations in real time. At Complexity = 0.5 and Density = 0.5, the base values pass through unchanged.*

---

## 2. Polymetric Layering

Poly's power comes from running up to 8 independent Euclidean lanes at different cycle lengths simultaneously. When cycles share no common factor, the combined pattern takes many bars to repeat, creating evolving grooves.

### Try It: Prime-Number Polymetry

Load **Polymetric Drift** preset, or build from Init by selecting each lane tab (zone C) and setting:

| Lane | Instrument | Steps | Subdiv | Hits | Character |
|------|-----------|-------|--------|------|-----------|
| 1 | Kick | 3 | 1/4 | 2 | Anchor on 3-beat cycle |
| 2 | Rim | 5 | 1/16 | 3 | Off-grid accent layer |
| 3 | Tom | 7 | 1/16 | 4 | Ghost-note texture |
| 4 | HH Closed | 11 | 1/16 | 7 | Shimmering wash |

**Macro settings (zone E):** Complexity = 0.7, Density = 0.4, Tension = 0.3

**What to listen for:** The four cycles (3, 5, 7, 11) are coprime. The combined pattern has a period of 3×5×7×11 = 1,155 steps before it repeats exactly. Watch the **Phase Alignment rings** (zone H) — the dots orbit at different speeds and rarely align.

**Experiment:** Slowly raise **Density** from 0.4 → 0.8. The sparse interplay fills in. Lower it to 0.2 and the pattern becomes skeletal.

---

## 3. Probability & Ghost Notes

Each lane has a **probability** (0–100%) that gates whether a step actually fires. Combined with **ghost floor** and **velocity spread**, this creates varying loudness and absent beats.

### Parameters

| Control | Zone | Range | Effect |
|---------|------|-------|--------|
| Probability bar | B (drag) | 0–100% | Chance each Euclidean hit actually plays |
| Vel | C (knob) | 0–127 | Base note velocity before spread/accents |
| Ghost | C (knob) | 0–127 | Minimum velocity for quiet "ghost" hits |
| Spread | C (knob) | 0–100% | Random ± range around base velocity |
| Emphasis Prob | (DAW automation) | 0–100% | Accent likelihood (no on-screen knob) |

### Try It: Ghost Note Texture

Load **Sparse Pulse** preset. Select Lane 3 tab in zone C (ghost layer):

- **Probability (zone B):** 60% — many hits are skipped, creating gaps
- **Ghost (zone C):** 20 — surviving quiet hits are barely audible
- **Spread (zone C):** 20% — wide dynamic range
- **Vel (zone C):** 45 — quiet to begin with

**Macro settings (zone E):** Density = 0.25, Humanize = 0.3

**What to listen for:** The ghost layer appears and disappears unpredictably, with widely varying velocities. The **Velocity bars** (zone F) for Lane 3 flicker between near-zero and mid-range. Raise **Density** macro to bring more ghost hits in; raise **Tension** to widen the velocity spread further.

---

## 4. Syncopation & Rotation

The **Syncopation** macro (zone E) shifts all patterns away from their natural downbeat positions and inverts the emphasis probability — accents fall on weak beats instead of strong ones. For per-lane control, use the **Rot** knob in zone C to set a static rotation offset.

### How It Works

| Syncopation Value | Rotation Effect | Emphasis Effect |
|-------------------|----------------|-----------------|
| 0.0 | No shift | Normal accent probability |
| 0.5 | Half-cycle rotation | 50/50 emphasis inversion |
| 1.0 | Full half-cycle offset | Emphasis completely inverted |

### Try It: Off-Beat Grooves

Load **Breakbeat** preset:

- **Starting macros (zone E):** Syncopation = 0.5, Tension = 0.4, Complexity = 0.6
- Select Lane 1 (zone C) and check Rot to see the kick's rotation
- The kick (5 in 16, 3 hits) sits in unusual positions
- Snare accents on steps 1 and 3 (emphasisProb = 0.8)

**Experiment:**
1. Set Syncopation = 0.0 — the pattern snaps to predictable downbeats
2. Sweep Syncopation slowly from 0.0 → 1.0 — hear the groove shift off-center
3. At Syncopation = 1.0 with Tension = 0.4, accents now favor weak beats while velocity spread is wide — the groove sounds "inside out"
4. Select a lane tab and try increasing its **Rot** knob independently — per-lane rotation stacks with the global Syncopation macro

---

## 5. Swing

**Swing** (zone E) shifts every other 16th-note step later in time, creating a shuffle or triplet feel. The global macro adds to each lane's individual swing amount.

### Parameters

| Control | Zone | Range | Effect |
|---------|------|-------|--------|
| Swing (per-lane) | C (knob) | 0–100% | Lane-specific swing amount |
| Swing macro | E | 0–1.0 | Added to all lanes (clamped at 1.0) |

### Try It: Shuffle Feels

Load **Latin Feel** preset:

- **Swing macro (zone E):** 0.3 (adds to all lanes)
- Select Lane 3 tab (zone C) — check the per-lane Swing knob to see its base value
- HH Closed (lane 3): base swing 0%, effective swing 30% (0% + macro 30%)
- Conga (lane 2): humanize 2ms + swing creates a loose, lilting feel

**Experiment:**
1. Set Swing macro = 0.0 — hear the straight, grid-locked pattern
2. Set Swing macro = 0.3 — gentle shuffle, especially audible on the shaker's 16th notes
3. Set Swing macro = 0.7 — heavy swing approaching a 12/8 triplet feel
4. Select a lane tab (zone C) and raise its per-lane **Swing** knob independently — give one layer a different shuffle amount from the rest
5. Combine with Humanize = 0.3 — swing + jitter creates an organic, "played" quality

---

## 6. Tension & Dynamic Range

**Tension** (zone E) controls how wide the dynamic range is. At 0 it compresses everything; at 1 it exaggerates velocity differences, boosts accents, and deepens envelope modulation.

### How It Works

| Tension Value | Velocity Spread | Emphasis Prob | Envelope Depth |
|--------------|----------------|---------------|----------------|
| 0.0 | Halved | → 0 (no accents) | Halved |
| 0.5 | Unchanged | Unchanged | Unchanged |
| 1.0 | Doubled (max 0.5) | → 1 (always accent) | Doubled |

### Try It: Compression vs. Explosion

Load **Four on the Floor** preset:

1. Set Tension = 0.0 — the groove sounds flat and mechanical; all notes are similar velocity
2. Set Tension = 0.5 — natural dynamics return
3. Set Tension = 1.0 — accents punch hard, ghost notes are very quiet, envelope modulation is exaggerated
4. Watch the **Velocity bars** (zone F) — at Tension = 0 they're uniform height; at Tension = 1 they jump between extremes

---

## 7. Humanize & Micro-Timing

**Humanize** (zone E) adds timing jitter and a small velocity spread bump to all lanes, simulating an imperfect human player. Per-lane humanize is available via DAW automation for fine-grained control.

### Parameters

| Control | Zone | Range | Effect |
|---------|------|-------|--------|
| Per-lane humanize | (DAW automation) | 0–20ms | Lane-specific timing randomness |
| Humanize macro | E | 0–1.0 | Adds up to 10ms jitter + velocity spread to all lanes |

### Try It: Machine vs. Human

Load **Four on the Floor** preset. Set all macros to 0 first.

1. Play the pattern — perfectly quantized, robotic
2. Set Humanize = 0.3 — subtle looseness, notes land slightly early/late
3. Set Humanize = 0.7 — noticeable drift, feels hand-played
4. Set Humanize = 1.0 — maximum 10ms jitter per unit — at 120 BPM this is about 2.4% of a 16th note, enough to sound loose without sounding sloppy

**Combine with Swing:** Humanize = 0.3, Swing = 0.2 creates a warm, natural shuffle feel.

---

## 8. Phrase Gating

Phrases create breathing patterns — a lane plays for a set duration, then goes silent for a gap, then repeats. Each lane can have its own phrase length, gap, and offset, so lanes fade in and out at different rates.

### Parameters (Zone D Knobs)

| Knob | Label | Range | Effect |
|------|-------|-------|--------|
| 1 | Len | 0–64 beats | Duration of the "play" window (0 = always on) |
| 2 | Gap | 0–64 beats | Duration of silence between phrases |
| 3 | Ofs | 0–64 beats | Shifts the phrase cycle start point |

### Try It: Staggered Breathing

Load **Afro-House Phrases** preset (5 lanes active):

Set phrase parameters per lane using the lane tabs in zone D:

| Lane | Instrument | Phrase Len | Gap | Offset | Behavior |
|------|-----------|-----------|-----|--------|----------|
| 1 | Kick | 16 beats | 0 | 0 | Always playing (16-beat cycle, no gap) |
| 2 | Shaker | 0 (off) | 0 | 0 | Continuous — no phrase gating |
| 3 | Conga | 8 beats | 2 beats | 0 | Plays 8 beats, rests 2, repeats |
| 4 | Djembe | 12 beats | 4 beats | 4 beats | Offset start — enters 4 beats after conga |
| 5 | Perc | 4 beats | 4 beats | 8 beats | Short bursts, enters 8 beats late |

**What to listen for:** The kick and shaker provide a constant foundation while conga, djembe, and percussion weave in and out on staggered cycles. Watch the **Phase Alignment rings** (zone H) — phrase arcs show the play/gap windows rotating at different speeds.

**Experiment:**
1. Select Lane 3 tab (zone D), increase Gap from 2 → 8 — the conga becomes more sparse
2. Select Lane 5, change Offset from 8 → 0 — the percussion now aligns with the conga instead of offsetting
3. Set all lanes to Len = 4, Gap = 4, Offset = 0 — they all breathe in unison (a very different feel)

---

## 9. Mutation

**Mutation** introduces per-cycle random changes to the Euclidean pattern. Each cycle, every step has a chance (equal to the mutation rate) of being flipped, ghosted, or added.

### Mutation Types

| Type | What Happens |
|------|-------------|
| Delete | An existing hit is removed for that cycle |
| Ghost | An existing hit becomes a quiet ghost note |
| Add | A gap gains a new hit for that cycle |

### Parameters (Zone D)

| Knob | Label | Range | Effect |
|------|-------|-------|--------|
| 4 | Mut | 0–100% | Per-step probability of mutation each cycle |

### Try It: Evolving Patterns

Load **Pocket Groove** preset:

| Lane | Mutation Rate | Effect |
|------|-------------|--------|
| Kick | 0% | Rock-solid foundation |
| Snare | 10% | Occasional ghost note or skip |
| HH | 15% | Gentle variation in hat pattern |
| Ghost | 20% | Constantly shifting ghost layer |

**What to listen for:** The kick stays locked while the hat and ghost layers shift subtly each bar. The groove never exactly repeats but stays recognizable.

**Experiment:**
1. Set all lanes to Mutation = 0 — the pattern is static and deterministic
2. Set Lane 2 (Snare) Mutation = 50% — half the snare hits change each cycle; the backbeat becomes unreliable
3. Set Lane 3 (HH) Mutation = 80% — the hat pattern is nearly random while maintaining the Euclidean density structure

---

## 10. Drift

**Drift** gradually rotates a lane's Euclidean pattern over time, derived from absolute PPQ position. This creates phasing effects where two similar patterns slowly separate and reconverge.

### Parameters (Zone D)

| Knob | Label | Range | Effect |
|------|-------|-------|--------|
| 5 | Drift | ±4 steps/bar | Speed and direction of pattern rotation |

Positive drift rotates the pattern forward; negative drift rotates backward. The rotation is continuous and derived from transport position, so it's deterministic and reproducible.

### Try It: Steve Reich Phasing

Load **Reich Phasing** preset (3 lanes). Select each lane tab (zone C) to see the Euclidean parameters:

| Lane | Instrument | Steps | Hits | Drift (zone D) | Role |
|------|-----------|-------|------|------|------|
| 1 | Fixed (Note: E4) | 12 | 5 | 0.0 | Reference pattern |
| 2 | Drifting (Note: E4) | 12 | 5 | +0.25 st/bar | Slowly phasing copy |
| 3 | Pulse (HH, Note: F#2) | 4 | 4 | 0.0 | Steady timekeeping |

**What to listen for:** Lanes 1 and 2 start in unison but lane 2 gradually shifts ahead. After several bars, hits that were simultaneous become offset, creating new resultant rhythms. Eventually (after many bars) they realign.

**Watch the Phase Alignment rings** (zone H) — the two inner dots orbit at slightly different speeds. The drift trail arc on lane 2 shows the direction of rotation.

**Experiment:**
1. Select Lane 2 tab (zone D), set Drift = +1.0 — faster phasing, audible separation within 2–3 bars
2. Set Lane 2 Drift = -0.25 — reverse direction; the pattern rotates the other way
3. Add Drift = +0.5 to Lane 3 (HH) — even the timekeeping layer drifts, dissolving all reference points

---

## 11. Kotekan Pairs (Interlocking)

Kotekan is a Balinese gamelan technique where two players create a single composite melody by filling each other's gaps. In Poly, setting a lane's **Kotek** knob (zone C, VOICE group) makes it play the *complement* of the source lane's pattern — it hits exactly where the source doesn't.

### Parameters (Zone C → VOICE group)

| Knob | Label | Range | Effect |
|------|-------|-------|--------|
| 10 | Kotek | off / L1–L8 | off = independent; L1–L8 = complement of that lane |

### Try It: Interlocking Pair

Load **Kotekan Interlock** preset (4 lanes). Select each lane tab (zone C) to view:

| Lane | Name | Steps/Hits | Kotek | Result |
|------|------|-----------|-------|--------|
| 1 | Polos | 8 steps, 3 hits | off | Plays its own pattern |
| 2 | Sangsih | 8 steps, 3 hits | L1 | Plays the *inverse* of Lane 1 |
| 3 | Gong | 4 steps, 1 hit | off | Single anchor beat |
| 4 | Shimmer | 16 steps, 4 hits | off | Ghost texture |

**What to listen for:** Polos and Sangsih together fill every step in their 8-step cycle. Where Polos plays, Sangsih rests; where Polos rests, Sangsih plays. The gong marks the downbeat. Together they create a continuous stream of interlocking notes at two different pitches — check the **Note** knobs in zone C to see each lane's MIDI assignment.

**Experiment:**
1. Mute Lane 1 (click its probability bar in zone B down to 0) — Sangsih now plays the complement of silence, which is *every* step
2. Restore Lane 1. Select Lane 1 tab (zone C) and change its **Hits** knob — Sangsih's pattern changes accordingly to maintain the complement
3. Raise Complexity macro (zone E) to 0.8 — more hits on Polos means fewer on Sangsih; the balance shifts

---

## 12. Timing Offset (Push/Pull)

**Timing Offset** shifts a lane's notes slightly early or late relative to the grid, measured in milliseconds. This is distinct from Humanize (which is random) — offset is a fixed, consistent shift.

### Parameters (Zone D)

| Knob | Label | Range | Effect |
|------|-------|-------|--------|
| 6 | Time | ±20ms | Positive = late (behind the beat), Negative = early (ahead) |

### Try It: Pocket Feel

Load **Pocket Groove** preset. Select each lane tab (zone D) to see timing offsets:

| Lane | Instrument | Timing Offset | Feel |
|------|-----------|--------------|------|
| Kick | Kick | +3ms | Slightly behind — lazy, heavy |
| Snare | Snare | -2ms | Slightly ahead — pushes the groove forward |
| HH | HH Closed | +1ms | Barely late — creates drag |
| Ghost | Ghost perc | -1.5ms | Slightly early — adds tension |

**What to listen for:** The kick sits slightly behind the grid while the snare pushes slightly ahead. This push-pull creates the "pocket" feel associated with J Dilla, Questlove, and other groove-centric drummers. The effect is subtle — you feel it more than hear it.

**Experiment:**
1. Set all timing offsets to 0 — the groove snaps to grid, losing its character
2. Set Kick = +10ms, Snare = -10ms — exaggerated push-pull, clearly audible
3. Set all lanes to +5ms — everything is uniformly late (laid-back) rather than having internal tension

---

## 13. Complexity & Density Macros

**Complexity** and **Density** (zone E) are the primary groove-shaping macros. They scale the underlying Euclidean parameters across all lanes simultaneously.

### Complexity (default: 0.5)

| Value | Hit Count | Rotation | Envelope Depth |
|-------|----------|----------|----------------|
| 0.0 | → 1 (minimal) | → 0 (no rotation) | Halved |
| 0.5 | Unchanged | Unchanged | Unchanged |
| 1.0 | → max steps (saturated) | → half-cycle | Doubled |

### Density (default: 0.5)

| Value | Probability | Hit Count |
|-------|------------|-----------|
| 0.0 | Halved | → 1 |
| 0.5 | Unchanged | Unchanged |
| 1.0 | → 1.0 (always plays) | → max |

### Try It: Macro Sweep

Load **Four on the Floor** preset. Leave Complexity at 0.3 (preset default).

1. **Density sweep (0.1 → 0.9):** At 0.1, only the kick survives; ghost lanes disappear entirely. At 0.9, every possible step fires on every lane — a wall of notes.
2. **Complexity sweep (0.1 → 0.9):** At 0.1, all patterns collapse to a single hit with no rotation — minimal. At 0.9, every lane has maximum hits with half-cycle rotation, and envelopes become dramatic.
3. **Sweet spots:** Complexity = 0.4, Density = 0.5 — a balanced groove with moderate movement. Complexity = 0.8, Density = 0.3 — complex patterns but sparse triggering (sophisticated minimalism).

---

## 14. Envelopes

Global envelopes modulate lane parameters over time using shapes that cycle at configurable periods.

### Envelope Shapes

| Shape | Description |
|-------|-------------|
| Ramp | Linear 0 → 1 |
| Sine | Half-cycle sinusoidal |
| Triangle | Linear up then down |
| Curve | Exponential (curvature parameter) |
| StepList | Stepped values via a lookup table |

### Envelope Targets

| Target | What Gets Modulated |
|--------|-------------------|
| Velocity | Note velocity scaling |
| Density | Hit probability over time |
| Probability | Play chance fluctuation |
| AccentBias | Emphasis probability shift |
| NoteLength | Gate duration |
| TimingLooseness | Humanize depth |
| ActivationWeight | Lane activation |
| FillLikelihood | Fill insertion rate |

**Watch zone G** (Envelope Curves) to see the current envelope phase and value for each active lane. The marker dots show where each lane sits in its cycle.

**Experiment with Tension:** Tension = 0 compresses envelope depth to half; Tension = 1 doubles it. With envelopes targeting Velocity, high Tension creates dramatic swells and drops.

---

## 15. Scene Morphing

Poly stores two complete groove configurations (Scene A and Scene B) and can blend between them in real time.

| Parameter | Range | Effect |
|-----------|-------|--------|
| Scene Select | A / B / Morph | Which scene is active |
| Scene Morph | 0–1.0 | Blend factor (0 = pure A, 1 = pure B) |

Float parameters (probability, velocity, swing, timing) crossfade smoothly. Discrete parameters (step count, MIDI note) snap at the midpoint (0.5).

**Use case:** Set up a sparse verse groove in Scene A and a dense chorus groove in Scene B. Automate the Morph knob to transition between them over 4 bars.

---

## 16. Combining Features — Recipe Card

Here are starting-point combinations that showcase Poly's features working together. For each recipe, select lane tabs in zone C to set up the PATTERN and VOICE parameters, zone D for phrase parameters, and zone E for the macros.

### Minimal Techno

| Parameter | Value | Why |
|-----------|-------|-----|
| Lanes active | 3 | Kick, HH, Ghost |
| Kick (zone C): Steps 4, Subdiv 1/4, Hits 4 | Standard four-on-the-floor | Anchor |
| HH (zone C): Steps 16, Subdiv 1/16, Hits 8 | Half density | Space |
| Ghost (zone C): Steps 7, Subdiv 1/16, Hits 3 | Polymetric | Movement |
| Density (zone E) | 0.3 | Sparse |
| Tension (zone E) | 0.2 | Tight dynamics |
| Humanize (zone E) | 0.1 | Barely loose |

### Afro-Cuban Fusion

| Parameter | Value | Why |
|-----------|-------|-----|
| Lanes active | 5 | Clave, Conga, Shaker, Bell, Ghost |
| Lane 1 (zone C): Steps 16, Subdiv 1/16, Hits 3 | Tresillo base | |
| Lane 2 (zone C): Steps 8, Subdiv 1/8, Hits 4 | Off-grid | Polymetric tension |
| Lane 3 (zone C): Steps 16, Subdiv 1/16, Hits 12 | Continuous | Textural wash |
| Swing (zone E) | 0.3 | Shuffle |
| Humanize (zone E) | 0.2 | Organic |
| Complexity (zone E) | 0.4 | Moderate rotation |
| Lane 3 Mutation (zone D) | 15% | Evolving texture |

### Generative Ambient Percussion

| Parameter | Value | Why |
|-----------|-------|-----|
| Lanes active | 6 | All different cycle lengths |
| Steps (zone C, per lane) | 3, 5, 7, 11, 13, 17 | Coprime — never repeats |
| Probability (zone B, per lane) | 50–70% | Sparse, unpredictable |
| Mutation (zone D) | 20–40% per lane | Constant evolution |
| Drift (zone D) | +0.1 to +0.5 per lane | Slow phasing |
| Humanize (zone E) | 0.5 | Very loose |
| Tension (zone E) | 0.7 | Wide dynamics |
| Phrase (zone D): staggered offsets | Varying | Breathing in/out |

---

## Quick Reference: All Parameters at a Glance

### Per-Lane Parameters

| Parameter | Range | Default | UI Control |
|-----------|-------|---------|-----------|
| Steps | 1–64 | varies | Steps knob (zone C, PATTERN) |
| Subdivision | 1/1–1/16 | varies | Subdiv knob (zone C, PATTERN) |
| Hits | 0–steps | varies | Hits knob (zone C, PATTERN) |
| Rotation | 0–63 | 0 | Rot knob (zone C, PATTERN) |
| MIDI Note | 0–127 | varies | Note knob (zone C, PATTERN) |
| Probability | 0–100% | 100% | Lane bar (zone B) |
| Base Velocity | 0–127 | 100 | Vel knob (zone C, VOICE) |
| Ghost Floor | 0–127 | 30 | Ghost knob (zone C, VOICE) |
| Velocity Spread | 0–100% | 5% | Spread knob (zone C, VOICE) |
| Per-lane Swing | 0–100% | 0% | Swing knob (zone C, VOICE) |
| Kotekan Source | off / L1–L8 | off | Kotek knob (zone C, VOICE) |
| Emphasis Prob | 0–100% | 50% | DAW automation only |
| Per-lane Humanize | 0–20ms | 0ms | DAW automation only |
| Note Duration | 0–4 beats | 0 | DAW automation only |
| Active | on/off | on | DAW automation only |
| Phrase Length | 0–64 beats | 0 | Len knob (zone D) |
| Phrase Gap | 0–64 beats | 0 | Gap knob (zone D) |
| Phrase Offset | 0–64 beats | 0 | Ofs knob (zone D) |
| Mutation Rate | 0–100% | 0% | Mut knob (zone D) |
| Drift Rate | ±4 st/bar | 0 | Drift knob (zone D) |
| Timing Offset | ±20ms | 0ms | Time knob (zone D) |

### Global Macro Parameters

| Macro | Range | Default | Zone E Position |
|-------|-------|---------|----------------|
| Complexity | 0–1.0 | 0.5 | 1st knob (left) |
| Density | 0–1.0 | 0.5 | 2nd knob |
| Syncopation | 0–1.0 | 0.0 | 3rd knob |
| Swing | 0–1.0 | 0.0 | 4th knob |
| Tension | 0–1.0 | 0.0 | 5th knob |
| Humanize | 0–1.0 | 0.0 | 6th knob (right) |

### Global Parameters

| Parameter | Range | Default |
|-----------|-------|---------|
| Active Lane Count | 1–8 | 4 |
| Seed | 0–999999 | 0 |
| Scene Select | A/B/Morph | A |
| Scene Morph | 0–1.0 | 0 |

---

## Factory Preset Summary

| # | Name | Lanes | Key Feature | Suggested Macro Experiments |
|---|------|-------|------------|---------------------------|
| 1 | Four on the Floor | 4 | Straight kick, polymetric ghost layer (7/8) | Sweep Density 0→1 |
| 2 | Polymetric Drift | 4 | Prime-number cycles (3, 5, 7, 11) | Sweep Complexity; watch phase rings |
| 3 | Sparse Pulse | 3 | Minimal, ghost-heavy, wide spread | Add Humanize; raise Density slowly |
| 4 | Breakbeat | 4 | Syncopated kick (5/16), accented snare | Sweep Syncopation 0→1 |
| 5 | Latin Feel | 4 | Clave pattern, conga/shaker/cowbell | Adjust Swing; add Tension |
| 6 | Afro-House Phrases | 5 | Staggered phrase gating on 3 layers | Change phrase offsets; try unison gaps |
| 7 | Reich Phasing | 3 | Two identical patterns, one drifting | Increase drift rate; add mutation |
| 8 | Kotekan Interlock | 4 | Interlocking complementary pair | Mute/unmute polos; sweep Complexity |
| 9 | Pocket Groove | 4 | Micro-timing offsets, gentle mutation | Zero all timing → hear the difference |
