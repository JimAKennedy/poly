# Poly — Automation Mapping Specification

## Design Principle

Poly exposes a curated set of parameters to Cubase automation — those that are musically meaningful to automate in real time. Internal compound parameters and structural settings (cycle length, hit count, pattern rotation) are deliberately kept internal: automating them would create discontinuities that break determinism guarantees.

## ParamID Layout

ParamIDs are organized in ranges to avoid conflicts and allow future expansion.

| Range | Category | Description |
|-------|----------|-------------|
| 0–99 | Global | Seed, master controls |
| 100–199 | Macros | High-level compound controls |
| 1000–1799 | Lane 0 | Per-lane parameters (100 IDs per lane) |
| 1800–2599 | Lane 1 | |
| 2600–3399 | Lane 2 | |
| 3400–4199 | Lane 3 | |
| 4200–4999 | Lane 4 | |
| 5000–5799 | Lane 5 | |
| 5800–6599 | Lane 6 | |
| 6600–7399 | Lane 7 | |

### Lane Offset Formula

```
ParamID = kLaneBase + (laneIndex * kLaneStride) + paramOffset
```

Where `kLaneBase = 1000` and `kLaneStride = 800`.

## Exposed Parameters (Automatable)

### Global Parameters

| ParamID | Name | Normalized Range | Plain Range | Unit |
|---------|------|-----------------|-------------|------|
| 0 | Active Lane Count | 0.0–1.0 | 4–8 | count |

### Macro Parameters

| ParamID | Name | Normalized Range | Plain Range | Unit |
|---------|------|-----------------|-------------|------|
| 100 | Complexity | 0.0–1.0 | 0.0–1.0 | — |
| 101 | Density | 0.0–1.0 | 0.0–1.0 | — |
| 102 | Syncopation | 0.0–1.0 | 0.0–1.0 | — |
| 103 | Swing | 0.0–1.0 | 0.0–1.0 | — |
| 104 | Tension | 0.0–1.0 | 0.0–1.0 | — |
| 105 | Humanize | 0.0–1.0 | 0.0–1.0 | — |

### Per-Lane Parameters (offsets within lane range)

| Offset | Name | Normalized Range | Plain Range | Unit |
|--------|------|-----------------|-------------|------|
| 0 | Probability | 0.0–1.0 | 0.0–1.0 | — |
| 1 | Base Velocity | 0.0–1.0 | 0–127 | velocity |
| 2 | Emphasis Probability | 0.0–1.0 | 0.0–1.0 | — |
| 3 | Ghost Floor | 0.0–1.0 | 0–127 | velocity |
| 4 | Velocity Spread | 0.0–1.0 | 0.0–1.0 | — |
| 5 | Humanize | 0.0–1.0 | 0.0–50.0 | ms |
| 6 | Lane Active | 0.0–1.0 | off/on | toggle |

## Internal Parameters (Not Automatable)

These parameters are set via the editor UI or presets only. They are excluded from automation because changing them mid-playback would break deterministic output or create musical discontinuities.

| Parameter | Reason |
|-----------|--------|
| Cycle Steps | Changes pattern length mid-bar → broken cycles |
| Subdivision | Redefines step grid → note position discontinuity |
| Hit Count | Recomputes Euclidean pattern → all note positions shift |
| Rotation | Shifts pattern phase → note positions jump |
| MIDI Note | Drum voice change is a preset choice, not an automation target |
| Seed | Changing seed changes all probability/velocity rolls simultaneously |
| Envelope assignments | Structural change to modulation routing |
| Role | Semantic label, no runtime effect |

## Normalized ↔ Plain Conversion

All VST3 parameters use normalized `[0.0, 1.0]` range internally. Conversion:

```cpp
// Integer range (e.g., velocity 0–127)
plain = normalized * (max - min) + min;
normalized = (plain - min) / (max - min);

// Float range (e.g., humanize 0.0–50.0 ms)
plain = normalized * max;
normalized = plain / max;

// Toggle (e.g., lane active)
plain = normalized >= 0.5 ? 1 : 0;
normalized = plain ? 1.0 : 0.0;
```

## Automation Behavior

### Thread Safety

Parameter changes from automation arrive on the audio thread via `process()`. The processor reads normalized values from `IParameterChanges` and converts to plain values before passing to the engine's `GrooveState`.

### Sample-Accurate Automation

VST3 supports sample-accurate parameter changes within a block. Poly handles this by applying the latest parameter value for the entire block (block-level granularity). Sample-accurate splitting is unnecessary because Poly's output timing is PPQ-based, not sample-based.

### Default Values

All parameters have meaningful defaults that produce a playable groove without user intervention. The macro defaults (all 0.5 or 0.0) represent a neutral starting point.

## Future Considerations

- **Scene switching**: A dedicated scene/snapshot parameter may be added in Phase 2 for automating groove transitions
- **Envelope depth**: Per-lane envelope depth parameters may be exposed for real-time control of modulation intensity
- **Per-lane mute groups**: Automatable mute/solo per lane for arrangement-driven lane activation
