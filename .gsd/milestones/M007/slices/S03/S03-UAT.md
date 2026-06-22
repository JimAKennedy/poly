# S03: Phase Drift — UAT

**Milestone:** M007
**Written:** 2026-06-22T19:55:46.135Z

## S03 Phase Drift UAT

### Prerequisites
- Build and rescan Poly VST3 in Cubase

### Test Cases

1. **Drift = 0 (default)**
   - Load Poly, confirm all lanes play their normal patterns
   - Drift Rate parameter should be at center (0.0 steps/bar)
   - Expected: No change from previous behavior

2. **Single lane drift**
   - Set Lane 2 (Snare) drift rate to 1.0 steps/bar
   - Play for 8+ bars
   - Expected: Snare pattern gradually shifts relative to kick — hits arrive at different beat positions each bar

3. **Reich-style phasing**
   - Set Lane 1 and Lane 2 to identical patterns (same steps, hits, subdivision)
   - Set Lane 1 drift = 0, Lane 2 drift = 0.25
   - Play for 16+ bars
   - Expected: Patterns start in unison, gradually diverge, creating emerging rhythmic combinations

4. **Transport jump preserves drift state**
   - Set drift rate = 0.5 on all lanes, play from bar 0
   - Jump transport to bar 8
   - Expected: Pattern immediately snaps to correct drifted position (no glitch, no gradual catch-up)

5. **Phase alignment view**
   - Set non-zero drift rates on several lanes
   - Expected: Arc trails visible from each lane's phase dot, indicating drift direction and rate
   - Positive drift shows clockwise trail, negative shows counter-clockwise

6. **Preset save/load**
   - Configure drift rates, save preset
   - Reload preset
   - Expected: Drift rates restored correctly (state version 7)
