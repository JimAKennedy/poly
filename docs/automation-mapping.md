# Poly — Automation Mapping Specification

## Design Principle

Poly exposes a curated set of parameters to Cubase automation — those that are musically meaningful to automate in real time. Internal compound parameters and structural settings (cycle length, hit count, pattern rotation) are deliberately kept internal: automating them would create discontinuities that break determinism guarantees.

## VST3 Unit Hierarchy

Parameters are organized into named VST3 Units for clean grouping in Cubase's automation lanes.

| UnitID | Name | Parent | Contents |
|--------|------|--------|----------|
| 0 | Root | — | (container) |
| 1–8 | Lane 1–8 | Root | Per-lane parameters |
| 9 | Macros | Root | 6 macro controls |
| 10 | Global | Root | Active Lanes, Seed |
| 11 | Scene | Root | Select, Morph |
| 12 | Output | Root | Per-lane velocity readouts |

In Cubase, automation lane names appear as `Unit / Parameter` (e.g., `Lane 1 / Probability`).

## ParamID Layout

| Range | Category | Description |
|-------|----------|-------------|
| 0–127 | Lanes | Per-lane parameters (16 IDs per lane × 8 lanes) |
| 200–205 | Macros | High-level compound controls |
| 300–301 | Global | Active Lanes, Seed |
| 400–407 | Output | Per-lane velocity readouts (read-only) |
| 500–501 | Scene | Scene Select, Scene Morph |

### Lane Offset Formula

```
ParamID = lane * 16 + offset
```

Where `lane` is 0–7 and `offset` is the parameter within the lane.

## Exposed Parameters (Automatable)

### Per-Lane Parameters (offsets within lane range)

| Offset | Name | Unit | Steps | Default | Plain Range |
|--------|------|------|-------|---------|-------------|
| 0 | Probability | % | 0 | 1.0 | 0–100% |
| 1 | Base Velocity | — | 127 | 100 | 0–127 |
| 2 | Emphasis | % | 0 | 0.5 | 0–100% |
| 3 | Ghost Floor | — | 127 | 30 | 0–127 |
| 4 | Spread | % | 0 | 0.05 | 0–100% |
| 5 | Swing | % | 0 | 0.0 | 0–100% |
| 6 | Humanize | ms | 0 | 0.0 | 0–50 ms |
| 7 | Duration | beats | 0 | 0.0 | 0–4 beats |
| 8 | Active | — | 1 | 1.0 | Off/On |

### Macro Parameters

| ParamID | Name | Unit | Default |
|---------|------|------|---------|
| 200 | Complexity | % | 0.5 |
| 201 | Density | % | 0.5 |
| 202 | Syncopation | % | 0.0 |
| 203 | Swing | % | 0.0 |
| 204 | Tension | % | 0.0 |
| 205 | Humanize | % | 0.0 |

### Global Parameters

| ParamID | Name | Steps | Default | Plain Range |
|---------|------|-------|---------|-------------|
| 300 | Active Lanes | 7 | 4 | 1–8 |
| 301 | Seed | 0 | 0 | 0–999999 |

### Scene Parameters

| ParamID | Name | Unit | Steps | Default |
|---------|------|------|-------|---------|
| 500 | Select | — | 2 | A (0) |
| 501 | Morph | % | 0 | 0.0 |

### Output Parameters (Read-Only)

| ParamID | Name | Description |
|---------|------|-------------|
| 400–407 | Lane 1–8 | Last triggered velocity per lane |

## Internal Parameters (Not Automatable)

These are set via the editor UI or presets only. They are excluded from automation because changing them mid-playback would break deterministic output or create musical discontinuities.

| Parameter | Reason |
|-----------|--------|
| Cycle Steps | Changes pattern length mid-bar — broken cycles |
| Subdivision | Redefines step grid — note position discontinuity |
| Hit Count | Recomputes Euclidean pattern — all note positions shift |
| Rotation | Shifts pattern phase — note positions jump |
| MIDI Note | Drum voice change is a preset choice, not an automation target |
| Envelope assignments | Structural change to modulation routing |
| Role | Semantic label, no runtime effect |
| Constraint config | Structural guardrails, not real-time controls |

## Normalized / Plain Conversion

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
