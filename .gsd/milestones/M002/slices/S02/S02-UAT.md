# S02: Envelope Superposition v1 — UAT

**Milestone:** M002
**Written:** 2026-06-20T21:27:34.825Z

## Envelope Superposition v1 UAT

### Shape Functions
- [x] Sine produces 0.5 at phase 0, 1.0 at 0.25, 0.5 at 0.5, 0.0 at 0.75
- [x] Ramp produces linear 0→1 sawtooth
- [x] Triangle produces 0→1→0 symmetric triangle
- [x] Curve and StepList return 0.5 placeholder

### Phase Calculation
- [x] Phase derived from absolute PPQ, never accumulated
- [x] Phase offset shifts correctly
- [x] Non-dividing periods wrap cleanly
- [x] Large PPQ values wrap without drift

### Modulation
- [x] Velocity envelopes modulate note velocity multiplicatively
- [x] Density envelopes modulate probability threshold additively
- [x] Zero depth has no effect
- [x] Inactive envelopes are skipped
- [x] Multiple envelopes compound correctly
- [x] Global envelopes apply across all lanes

### Determinism
- [x] Identical output across block sizes (0.05/0.5/2.0 PPQ)
- [x] Loop restart produces identical sub-range
- [x] Non-dividing periods (3/5/7 bar) create deterministic emergent patterns
