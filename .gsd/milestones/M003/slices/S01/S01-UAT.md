# S01: Extended Envelope System — UAT

**Milestone:** M003
**Written:** 2026-06-21T01:28:20.122Z

## UAT: Extended Envelope System (M003/S01)

### Envelope Target Coverage
- [ ] Create a patch with a Velocity envelope (Sine, 4 bars) — velocity should cycle smoothly
- [ ] Add a Probability envelope (Ramp, 2 bars) — note density should increase over the ramp
- [ ] Add an AccentBias envelope — accented steps should fire more/less based on envelope position
- [ ] Add a NoteLength envelope — note durations should modulate with the envelope
- [ ] Add a TimingLooseness envelope — timing jitter should increase at envelope peaks
- [ ] Add an ActivationWeight envelope with negative bias — lane should drop out at envelope troughs
- [ ] Add a FillLikelihood envelope — extra notes should appear on non-pattern steps at envelope peaks

### New Shapes
- [ ] Set envelope shape to Curve with positive curvature — should ease-in (slow start, fast end)
- [ ] Set Curve with negative curvature — should ease-out (fast start, slow end)
- [ ] Set shape to StepList with 4 entries at [0, 0.5, 1.0, 0.25] — modulation should step discretely

### Preset Compatibility
- [ ] Load a v1 preset — should load without errors, new fields default to neutral values
- [ ] Save and reload a v2 preset — all envelope parameters including curvature and step values should round-trip

### Determinism
- [ ] Play a patch with multiple extended envelopes, stop and replay from same position — output must be identical
