# Poly — Cubase Workflow Guide

## Overview

Poly is a VST3 instrument that generates MIDI note events. It loads on a Cubase instrument track and outputs MIDI to drum instruments via Cubase's routing system. This document covers the canonical workflow for setup, monitoring, recording, and export.

## Setup

### 1. Load Poly on an Instrument Track

1. **Add Instrument Track**: Project → Add Track → Instrument → select "Poly"
2. Poly loads as a VST3 instrument with no audio output — it generates MIDI only
3. The track appears in the arrangement and mixer like any instrument track

### 2. Route MIDI Output to a Drum Instrument

Poly generates MIDI that needs to reach a drum instrument (Battery, Groove Agent, Kontakt, etc.):

**Option A — MIDI Send (recommended)**
1. Select the Poly instrument track
2. In the Inspector, open MIDI Sends
3. Enable a send and route it to the target drum instrument's MIDI input
4. The drum instrument receives Poly's generated notes in real time

**Option B — Record then route**
1. Record Poly's output to a MIDI part (see Recording below)
2. Move or copy the MIDI part to the drum instrument track
3. This gives full editing control before the notes reach the instrument

### 3. Monitor Configuration

- Enable **Monitor** on the Poly track to hear generated output during playback
- If using MIDI sends, monitor the drum instrument track instead
- Set MIDI Thru in Preferences → MIDI if needed for real-time monitoring

## Playback

### Transport Behavior

Poly derives all timing from Cubase's transport:

- **Play**: Poly generates notes based on current PPQ position
- **Stop**: Output ceases immediately
- **Loop**: Seamless — Poly recalculates from the loop-start PPQ position with no drift
- **Tempo changes**: Take effect immediately since timing is PPQ-based, not sample-based
- **Position jumps**: Correct output from the new position (no accumulated state to reset)

### Determinism

Identical transport playback produces identical MIDI output every time. Poly's output is a pure function of the patch settings and transport position. Changing the block size or buffer settings does not affect the musical result.

## Recording

### Record MIDI Output

1. Arm the Poly track for recording (enable Record on the track)
2. Press Record in the transport
3. Poly's generated MIDI events are captured as a standard MIDI part on the track
4. Stop recording — the MIDI part appears in the arrangement

### Record to a Separate Track

1. Create an empty MIDI track
2. Set its input to "Poly" (the instrument)
3. Arm the MIDI track for recording
4. Play — Poly's output is recorded on the MIDI track
5. This keeps the generated and recorded MIDI separate

### Loop Recording

Cubase's loop recording modes work with Poly:
- **Mix**: Each pass adds to the same MIDI part (layered takes)
- **Replace**: Each pass overwrites the previous

Since Poly is deterministic, loop recording in Replace mode produces identical content each pass unless you change parameters between passes.

## Editing Recorded MIDI

After recording, the MIDI part is a standard Cubase MIDI object:

- Open in the **Key Editor** for note-level editing
- Open in the **Drum Editor** for drum-map-oriented editing
- Quantize, transpose, or process with any standard MIDI function
- The recorded MIDI is fully independent of Poly — editing does not affect the generator

## Freeze and Export

### Freeze

Freeze is primarily useful for audio instruments. Since Poly outputs MIDI only, freezing the Poly track itself is not meaningful. Instead:

1. Record Poly's output as MIDI
2. Freeze the target drum instrument track (which renders the MIDI to audio)

### MIDI Export

1. Record Poly's output as a MIDI part
2. Select the part in the arrangement
3. File → Export → MIDI File
4. Choose Type 0 (single track) or Type 1 (multi-track)

### Render in Place

1. Select the drum instrument track receiving Poly's MIDI
2. Edit → Render in Place → Render Settings
3. This renders the drum instrument's audio output, not Poly's MIDI

## Scene Chaining

Scene chaining automates transitions between Scene A and Scene B at bar boundaries, removing the need to manually automate the Scene Select parameter.

### Setup

1. Open Cubase's generic editor for the Poly track (or use automation lanes)
2. Set **Chain Enable** to On
3. Set **Chain Mode**: OneShot (play once and stop), Loop (repeat), or PingPong (reverse at boundaries)
4. Set **Chain Length** to the number of entries (1-16)
5. For each entry, set the **Scene** (A, B, or Morph) and **Bars** (how long to hold it)

### Example: A→B→A Structure

A simple three-entry chain for verse→chorus→verse:

| Entry | Scene | Bars |
|-------|-------|------|
| 1     | A     | 8    |
| 2     | B     | 8    |
| 3     | A     | 8    |

With **OneShot** mode, the chain plays through all three entries (24 bars total) and holds the last entry. With **Loop**, it repeats indefinitely. With **PingPong**, it reverses at entry 3 and walks back: A→B→A→B→A→...

### Transport Behavior

- Chain position resets on transport jump (seek to a new position)
- Chain advances only at bar boundaries — mid-bar transport starts wait for the next bar line
- Recording captures the chain-driven scene changes as normal MIDI output

## Metric Modulation (Per-Lane Tempo)

Each lane has a **Tempo Mult** parameter that scales its step grid independently from the host tempo. At 2.0x, a lane runs at double speed. At 0.5x, half speed. This creates Nancarrow-style polymetric independence where lanes operate at different effective tempi.

### Setup

1. In Cubase's automation lanes, find the Tempo Mult parameter for the desired lane
2. Set the value (0.25x to 4.0x, default 1.0x)
3. Lanes at different multipliers produce cross-tempo polymetric interactions

### Usage Tips

- Start with simple ratios: 2.0x against 1.0x creates a 2:1 tempo relationship
- 1.5x against 1.0x creates a 3:2 hemiola
- Phrase gating boundaries stay bar-aligned (unscaled) regardless of tempo multiplier
- Scene morphing interpolates tempo multiplier smoothly — automate Scene Morph for gradual metric modulation transitions
- Works with both standard Euclidean and additive/aksak cycles

## Parameter Automation

Poly exposes selected parameters to Cubase's automation system:

1. Show automation lanes on the Poly track
2. Select the parameter to automate from the dropdown
3. Draw automation curves or record automation moves

See [automation-mapping.md](automation-mapping.md) for which parameters are automatable.

## Preset Management

### Save Presets

1. Open Poly's editor window
2. Use the VST3 preset manager (top bar of the plugin window)
3. Save as a VST3 preset (`.vstpreset` file)

### Load Presets

1. Use the preset browser in the plugin window
2. Or use Cubase's MediaBay to browse and load `.vstpreset` files

### State Versioning

Poly uses versioned state serialization. Presets saved with older versions will load correctly in newer versions — the plugin reads the version number and migrates the state forward.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| No sound | MIDI not routed to drum instrument | Check MIDI send or recording workflow |
| Notes cut off at loop point | Expected — Poly recalculates from loop start | This is correct deterministic behavior |
| Different output after restart | Patch seed changed | Verify seed in patch settings |
| High CPU | Many active lanes with short subdivisions | Reduce active lane count or increase subdivision |
| Missing notes | `kMaxEventsPerBlock` exceeded | Reduce lanes/density for very dense passages |
| Chain not advancing | Chain Enable is Off | Set Chain Enable to On in generic editor |
| Chain resets on seek | Expected behavior | Chain position resets when transport jumps |
| Lane too fast/slow | Tempo Mult not at 1.0x | Check per-lane Tempo Mult in automation |
