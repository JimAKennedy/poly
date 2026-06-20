# S03: Timing Determinism Prototype — UAT

**Milestone:** M001
**Written:** 2026-06-20T20:05:56.793Z

## UAT: Timing Determinism Prototype

### Automated Verification
- [x] Euclidean algorithm: 10 unit tests (distribution, rotation, edge cases, maximal evenness)
- [x] Block-size independence: identical sorted event sets across 6 different block sizes
- [x] Loop restart: sub-range from straight-through matches standalone render of same range
- [x] Position jump: jump-then-continue matches straight-through of target range
- [x] Tempo independence: PPQ positions identical at 120 BPM and 90 BPM
- [x] Seed determinism: same seed reproduces, different seed differs
- [x] Transport guard: not-playing produces zero events
- [x] Polymetric drift: 5/16 ghost lane shows bar-to-bar phase variation against 4/4

### Manual Verification
- [x] Harness output shows correct pitch mapping (36=kick, 38=snare, 42=HH, 45=ghost)
- [x] Velocity variation visible in output (spread applied per-event)
- [x] Probability filtering visibly drops ~10% of hi-hat hits (p=0.9) and ~30% of ghost hits (p=0.7)

