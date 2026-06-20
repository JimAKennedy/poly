# S01: Dynamic Shaping — UAT

**Milestone:** M002
**Written:** 2026-06-20T20:54:40.005Z

## Dynamic Shaping UAT

### Accent Mask Boost
- [x] Steps marked in AccentMask receive +0.15 velocity boost
- [x] Steps not in AccentMask are unaffected
- [x] Multiple accent positions work independently

### Emphasis Probability
- [x] emphasisProb=0.0 suppresses all accents (base velocity only)
- [x] emphasisProb=1.0 always expresses accents
- [x] emphasisProb=0.5 produces a statistical mix of accented/unaccented

### Ghost Floor
- [x] Velocity never falls below ghostFloor/127.0f
- [x] ghostFloor=0 has no effect (no minimum)
- [x] High velocities are not reduced by the floor

### Pipeline Integration
- [x] Accent boost + ghost floor work together correctly
- [x] Velocity always stays in [0.0, 1.0] range
- [x] All features are deterministic (same input → same output)
- [x] Block-size independent with dynamic shaping enabled
- [x] Loop-restart deterministic with dynamic shaping enabled
