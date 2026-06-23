# S05: Kotekan Pairs — UAT

**Milestone:** M007
**Written:** 2026-06-22T21:17:16.876Z

## UAT: Kotekan Pairs

### Setup
1. Load Poly in Cubase, set Active Lanes to at least 2
2. Open Cubase's generic parameter editor for Poly

### Test 1: Basic Kotekan Pair
1. Set Lane 2's "Kotekan Source" parameter to lane 1 (value 1/8)
2. Hit play — Lane 1 plays its normal Euclidean pattern, Lane 2 fills the gaps
3. **Expected:** Together they cover every step position with no overlaps

### Test 2: Different Voices
1. Assign different MIDI notes to lane 1 (e.g. kick) and lane 2 (e.g. hi-hat)
2. Set different velocities — lane 1 loud, lane 2 soft
3. **Expected:** Interlocking pattern on two distinct voices with different dynamics

### Test 3: Visual Indicator
1. Check lane grid — lane 2 should show "L2 ← L1" in purple
2. **Expected:** Link relationship visible at a glance

### Test 4: Timing Offset Knob
1. Select a lane in the phrase editor
2. The 6th knob "Time" should appear after Drift
3. Drag it to adjust per-lane timing offset (-20 to +20 ms)
4. **Expected:** Knob shows "off" at center, "+Nms" / "-Nms" when adjusted
