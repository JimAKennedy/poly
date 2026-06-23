# S07: Phrase UI Controls — UAT

**Milestone:** M007
**Written:** 2026-06-22T19:41:58.898Z

## Phrase UI Controls UAT

### Setup
1. Build and deploy Poly VST3
2. Load in Cubase, open plugin editor

### Test Cases

- [ ] **Lane tabs**: Click tabs 1-8, each highlights in lane color
- [ ] **Knob drag**: Vertical drag on Len/Gap/Ofs changes values with 200px sensitivity
- [ ] **Value readout**: All knobs show 1-decimal format (e.g. '12.0 bt', 'off')
- [ ] **Dimming**: When Len=0, Gap and Ofs knobs dim to ~25% opacity and show '--'
- [ ] **Schematic bar**: Shows scaled play/gap pattern in lane color
- [ ] **Beat labels**: '0' at start, len value at gap start, len+gap at end; no overlaps
- [ ] **Continuous mode**: Schematic shows faint full bar when Len=0
