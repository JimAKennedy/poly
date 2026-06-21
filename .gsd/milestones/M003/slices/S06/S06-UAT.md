# S06: Phase and Envelope Visualization — UAT

**Milestone:** M003
**Written:** 2026-06-21T13:14:19.261Z

## UAT: Phase and Envelope Visualization

### Envelope Curve View
- [ ] Envelope curves render for all active lanes with distinct colors
- [ ] Phase marker (vertical line + dot) tracks current position during playback
- [ ] View updates smoothly as transport advances

### Phase Alignment View
- [ ] Concentric rings appear for each active lane
- [ ] Phase dots rotate around rings during playback
- [ ] Lane alignment/divergence is visually apparent when cycles have different lengths

### Lane Phase Indicators
- [ ] Each active lane in LaneGridView shows a small circular phase indicator
- [ ] Phase dot rotates within the circle during playback

### Visual Regression
- [x] EnvelopeCurveDefault baseline created and passes
- [x] EnvelopeCurveMidPhase baseline created and passes
- [x] PhaseAlignmentDefault baseline created and passes
- [x] PhaseAlignmentMultiPhase baseline created and passes
- [x] LaneGridView baselines updated and pass
