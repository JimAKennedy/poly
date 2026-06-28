# Poly UI Guide

Reference for the Poly VST3 plugin interface. Poly is a polymetric drum pattern generator that outputs MIDI.

## Plugin Window Layout

The plugin window is 600 x 550 pixels, laid out top to bottom:

```
+-----------------------------------------------+
|  POLY    [ Select Preset...  ▾ ]        1.0.0  |  Header
+-----------------------------------------------+
|  LANES                                         |
|  Kick       ██████████████████████████  ● L1   |
|  Snare      ████████████████████        ● L2   |  Lane Grid
|  HH Closed  ██████████████████████████  ● L3   |  (interactive)
|  ...                                           |
+-----------------------------------------------+
|  MACROS                                        |
|  ⟳    ⟳    ⟳    ⟳    ⟳    ⟳            |  Macro Knobs
|  Cmplx Dens Sync Swing Tens Human              |
+-----------------------------------------------+
|  VELOCITY                                      |
|  ██  ██  ██  ██                                |  Velocity Bars
|  L1  L2  L3  L4  L5  L6  L7  L8               |  (read-only)
+-----------------------------------------------+
|  VISUALIZATION                                 |
|  ┌─────────────────┐  ┌──────────┐             |
|  │ Envelope curves  │  │ Phase    │             |  Visualizations
|  │ with markers     │  │ circles  │             |  (read-only)
|  └─────────────────┘  └──────────┘             |
+-----------------------------------------------+
```

---

## Sections

### Header

Displays the plugin name **POLY** (blue), a **preset selector dropdown**, and the version number **1.0.0** (gray).

**Preset selector:** Click the dropdown in the center of the header bar. The menu offers:

- **Init (All Lanes)** — resets all parameters to defaults with all 8 lanes active. Use this to return to a clean state after loading a preset.
- **5 factory presets** — each loads lane configurations, macro settings, active lane count, and seed in one shot.

After loading, the preset name displays in the header. All knobs, bars, and visualizations update immediately.

### Lanes

Eight drum lanes, each representing an independent rhythmic voice. This is the primary interactive area.

| Lane | Name       | Default MIDI Note |
|------|------------|-------------------|
| L1   | Kick       | C1 (36)           |
| L2   | Snare      | D1 (38)           |
| L3   | HH Closed  | F#1 (42)          |
| L4   | HH Open    | A1 (45)           |
| L5   | Tom Hi     | A#1 (46)          |
| L6   | Tom Lo     | D#1 (39)          |
| L7   | Ride       | G#1 (43)          |
| L8   | Crash      | D2 (50)           |

**Controls per lane:**

- **Active toggle** - Click the lane name (left side) to enable/disable the lane. Active lanes show a blue-tinted background; inactive lanes are dark gray.
- **Probability slider** - Click or drag the horizontal blue bar to set the probability (0-100%) that each note in this lane's pattern will trigger. Full bar = always plays; shorter bar = notes are randomly skipped.
- **Phase indicator** - The small colored circle on the right (marked L1-L8) shows a rotating dot that indicates the lane's current position within its rhythmic cycle during playback. Each lane can have a different cycle length, so they rotate at different speeds.

**Hidden parameters** (not visible in the UI, but exposed for DAW automation):

| Parameter        | Range        | Purpose                                        |
|------------------|--------------|------------------------------------------------|
| Base Velocity    | 0-127        | Default velocity for notes in this lane         |
| Emphasis Prob    | 0-100%       | Probability of accented (emphasized) hits       |
| Ghost Floor      | 0-127        | Minimum velocity for ghost notes                |
| Velocity Spread  | 0-100%       | Random velocity variation around base            |
| Swing Amount     | 0-100%       | Swing feel (delays alternate notes)             |
| Humanize         | 0-50 ms      | Random timing offset for natural feel           |
| Note Duration    | 0-4 beats    | Length of MIDI notes                            |
| Tempo Mult       | 0.25x-4.0x   | Per-lane tempo scaling (see Metric Modulation)  |

These parameters can be automated from Cubase's automation lanes or adjusted via the VST3 generic editor.

### Macros

Six rotary knobs that shape the overall groove character. These are high-level controls that influence multiple engine parameters at once.

| Knob         | Default | Effect                                              |
|--------------|---------|-----------------------------------------------------|
| Complexity   | 50%     | Increases rhythmic intricacy across all lanes       |
| Density      | 50%     | Controls how many hits are active in the pattern    |
| Syncopation  | 0%      | Shifts emphasis away from strong beats              |
| Swing        | 0%      | Applies swing feel globally                         |
| Tension      | 0%      | Increases rhythmic unpredictability and conflict    |
| Humanize     | 0%      | Adds timing and velocity imperfections globally    |

**How to use:** Click and drag up/down on any knob to adjust. The blue arc shows the current value. These are the most impactful controls for quickly changing the feel of a pattern.

### Velocity

Eight vertical bar graphs (one per lane, labeled L1-L8) showing the real-time velocity of the most recent note triggered in each lane. Green bars pulse when notes fire during playback. Inactive lanes show no bars.

This section is **read-only** - it's a live monitor, not a control.

### Visualization

Two real-time displays at the bottom of the plugin:

#### Envelope (left panel)

Shows envelope/automation curves for all active lanes overlaid in a single plot. Each lane has a distinct color:

| Lane | Color       |
|------|-------------|
| L1   | Blue        |
| L2   | Orange      |
| L3   | Green       |
| L4   | Red         |
| L5   | Purple      |
| L6   | Teal        |
| L7   | Dark Orange |
| L8   | Light Blue  |

During playback:
- A **vertical line** moves across the plot showing the current phase position for each lane
- A **colored dot** follows along the curve showing the current envelope value
- Multiple lanes overlay in the same plot, each cycling at its own rate

This visualizes how each lane's velocity shaping evolves over its cycle.

#### Phase Alignment (right panel)

A circular display showing concentric rings, one per active lane. A colored dot orbits each ring at the lane's cycle rate.

- Lanes with **shorter cycles** orbit faster
- Lanes with **longer cycles** orbit slower
- When dots align vertically (top), those lanes are "in phase" - their downbeats coincide
- When dots scatter around their rings, the polymetric pattern is at maximum divergence

This is the key visualization for understanding Poly's core concept: independent rhythmic cycles that drift in and out of alignment over time.

---

## Parameters Not in the UI

The following parameters exist and can be automated from Cubase but have no dedicated UI controls in the plugin window:

### Global

| Parameter         | ID  | Range     | Purpose                                |
|-------------------|-----|-----------|----------------------------------------|
| Active Lane Count | 300 | 1-8       | Number of active lanes                 |
| Seed              | 301 | 0-999999  | Random seed for pattern generation     |

### Scenes

| Parameter    | ID  | Range           | Purpose                              |
|--------------|-----|-----------------|--------------------------------------|
| Scene Select | 500 | A / B / Morph   | Choose between two groove snapshots  |
| Scene Morph  | 501 | 0-100%          | Crossfade between Scene A and B      |

The scene system lets you save two different groove configurations and crossfade between them. Accessible via Cubase's generic editor or automation.

### Scene Chaining

| Parameter         | ID       | Range                     | Purpose                                   |
|-------------------|----------|---------------------------|-------------------------------------------|
| Chain Enable      | 510      | Off / On                  | Enable automatic scene sequencing         |
| Chain Mode        | 511      | OneShot / Loop / PingPong | How the chain behaves at its end          |
| Chain Length       | 512      | 1-16                      | Number of active entries in the chain     |
| Entry N Scene     | 520+     | A / B / Morph             | Scene for chain entry N                   |
| Entry N Bars      | 521+     | 1-32                      | Duration in bars for chain entry N        |

Scene chaining automates transitions between scenes at bar boundaries. Configure a sequence of up to 16 entries, each specifying which scene (A, B, or Morph) to use and how many bars to hold it. Three modes control what happens when the chain reaches its end:

- **OneShot** — stops at the last entry and holds it
- **Loop** — wraps back to the first entry and repeats
- **PingPong** — reverses direction at each boundary (A→B→A→B...)

Chain parameters are per-entry, with IDs starting at 520. Each entry uses 2 parameter slots (scene, bars), so entry 0 is IDs 520-521, entry 1 is 522-523, etc.

### Metric Modulation (Per-Lane Tempo)

Each lane has a **Tempo Mult** parameter (range 0.25x to 4.0x, default 1.0x) that scales the lane's step grid independently from the host tempo. This is a per-lane core parameter, accessible via DAW automation.

| Multiplier | Effect                                                    |
|------------|-----------------------------------------------------------|
| 0.25x      | Quarter speed — 4x longer between hits                   |
| 0.5x       | Half speed — notes at double the PPQ spacing              |
| 1.0x       | Normal — follows host tempo directly                      |
| 2.0x       | Double speed — hits at half the PPQ spacing               |
| 4.0x       | Quadruple speed — 4x faster than host tempo               |

At 2.0x, a lane plays its pattern in half the time, effectively doubling its rate. At 0.5x, the pattern stretches to double length. Phrase gating stays in absolute PPQ (unscaled) so gating boundaries remain bar-aligned regardless of tempo multiplier. Scene interpolation lerps the multiplier smoothly when morphing between scenes.

### MIDI Export

| Parameter      | ID  | Range    | Purpose                              |
|----------------|-----|----------|--------------------------------------|
| Export Trigger | 600 | Button   | Captures current pattern to MIDI     |
| Capture Length | 601 | 1-32 bars| Length of MIDI capture               |
| Capture Ready  | 602 | Read-only| Indicates capture is complete        |

---

## Presets

Poly has **5 factory presets** built into the engine:

| #  | Name              | Style                                              | Active Lanes |
|----|-------------------|----------------------------------------------------|--------------|
| —  | Init (All Lanes)  | Default state, all 8 lanes active                  | 8            |
| 1  | Four on the Floor | Classic club groove with straight hats              | 4            |
| 2  | Polymetric Drift  | Prime-number cycles (3,5,7,11) creating drift       | 4            |
| 3  | Sparse Pulse      | Minimal, spacious groove with gentle ghosts         | 3            |
| 4  | Breakbeat         | Syncopated kick with punchy snare and fast hats     | 4            |
| 5  | Latin Feel        | Clave-inspired with conga, shaker, cowbell          | 4            |

**How to access:** Click the preset selector dropdown in the header bar. "Init (All Lanes)" is at the top, separated from the factory presets. Selecting any entry immediately loads all its parameters. The name stays displayed in the header until you select a different one.

**Note:** These presets are not yet registered as a VST3 Program List, so they do not appear in Cubase's own preset browser — use the in-plugin dropdown instead.

---

## Playback

Poly generates MIDI output synchronized to your DAW's transport:

1. **Press Play** in Cubase to start pattern generation
2. The lane phase indicators, velocity bars, and visualizations animate during playback
3. MIDI notes are sent to the track's output - route to a drum VSTi or sampler
4. Patterns are **deterministic**: the same settings + seed + tempo always produce the same output
5. Patterns respond to **tempo changes** and **loop points** in real time

Poly is a MIDI instrument - it generates note data but produces no audio on its own. You need a drum sound source on the same or a linked track.
