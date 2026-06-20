# M002: Core Groove Engine MVP

**Vision:** A musically useful polymetric groove generator with no ML. Lane-independent Euclidean rhythms with dynamics-first velocity shaping, superimposed multi-period envelopes, coherent macro controls, real-time-safe VST3 plugin integration, and a minimal but legible VSTGUI editor. The plugin is stable under start/stop, loop, and tempo changes in Cubase.

## Success Criteria

- Stable under start/stop, loop, and tempo change in Cubase
- Repeatable output when randomness disabled (seed-deterministic)
- Visible lane-to-audio correspondence in the UI
- Audible velocity and emphasis variation across lanes
- Patch save/load round-trips correctly with version stamp
- No heap allocation, locks, exceptions, or I/O in process()

## Slices

- [ ] **S01: Lane Generator Core** `risk:medium` `depends:[]`
  > After this: Engine produces correct Euclidean rhythms for various hitCount/cycle configs; identical seed produces identical output across runs

- [ ] **S02: Dynamic Shaping** `risk:low` `depends:[S01]`
  > After this: Lane output shows natural velocity variation with distinct accents, ghost notes, and controlled randomization

- [ ] **S03: Envelope Superposition v1** `risk:high` `depends:[S01]`
  > After this: Velocity and density evolve over multi-bar periods; overlapping envelopes with non-dividing periods create emergent evolving patterns

- [ ] **S04: Macro Controls** `risk:medium` `depends:[S01,S02,S03]`
  > After this: Each macro visibly and audibly affects multiple lane parameters in a musically coherent way across its full range

- [ ] **S05: Plugin Integration and State** `risk:high` `depends:[S01,S02,S03,S04]`
  > After this: Plugin generates polymetric rhythms in Cubase; save project and reload preserves complete patch; loop and transport jumps handled correctly

- [ ] **S06: Minimal VSTGUI Editor** `risk:medium` `depends:[S05]`
  > After this: UI shows lane activity in real time, macro knobs control all 5 macros, velocity view updates during playback

## Boundary Map

Not provided.
