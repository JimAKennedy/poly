# M002: Core Groove Engine MVP

**Vision:** A musically useful polymetric groove generator with no ML. Complete the velocity pipeline (accents, emphasis, ghosts), add superimposed multi-period envelopes, swing and humanization, coherent macro controls, real-time-safe VST3 plugin integration with state serialization, and a minimal VSTGUI editor. Stable under start/stop, loop, and tempo changes in Cubase.

## Success Criteria

- Stable under start/stop, loop, and tempo change in Cubase
- Repeatable output when randomness disabled (seed-deterministic)
- Visible lane-to-audio correspondence in the UI
- Audible velocity and emphasis variation across lanes
- Patch save/load round-trips correctly with version stamp
- No heap allocation, locks, exceptions, or I/O in process()
- Macro controls coherently affect multiple lane parameters
- Envelope modulation creates evolving multi-bar patterns

## Slices

- [x] **S01: S01** `risk:low` `depends:[]`
  > After this: Lane output shows natural velocity variation with distinct accents, ghost notes, and controlled emphasis probability

- [x] **S02: S02** `risk:high` `depends:[]`
  > After this: Velocity and density evolve over multi-bar periods; overlapping envelopes with non-dividing periods create emergent evolving patterns

- [x] **S03: S03** `risk:low` `depends:[]`
  > After this: Notes shift timing on even subdivisions (swing), jitter slightly (humanize), and have controllable durations

- [x] **S04: S04** `risk:medium` `depends:[]`
  > After this: Each macro visibly and audibly affects multiple lane parameters in a musically coherent way across its full range

- [x] **S05: S05** `risk:high` `depends:[]`
  > After this: Plugin generates polymetric rhythms in Cubase; save project and reload preserves complete patch; loop and transport jumps handled correctly

- [x] **S06: S06** `risk:medium` `depends:[]`
  > After this: UI shows lane activity in real time, macro knobs control all macros, velocity view updates during playback

## Boundary Map

Not provided.
