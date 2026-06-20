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
