# M010: Expose Core Euclidean Parameters in UI

**Vision:** Add VST3 parameters and UI controls for the 5 core Euclidean fields (steps, subdivision, hits, rotation, MIDI note) that are currently hardcoded, plus surface the 5 existing per-lane params (velocity, ghost floor, spread, swing, kotekan) that have no UI knobs. Creates a new LaneEditView with 10 knobs, shrinks velocity view to compensate for height.

## Success Criteria

- All 5 core Euclidean params (steps, subdivision, hits, rotation, midiNote) exposed as VST3 parameters
- LaneEditView shows 10 knobs per lane covering pattern + voice params
- Presets fully reset all parameters including new ones
- All tests pass, plugin deploys and runs in Cubase

## Slices

- [x] **S01: S01** `risk:high` `depends:[]`
  > After this: Change steps/hits/subdivision/rotation/note from DAW automation and hear the pattern change

- [x] **S02: S02** `risk:medium` `depends:[]`
  > After this: Open plugin UI, see new Lane Edit section with 10 knobs for pattern and voice params per lane

- [x] **S03: Golden Test and Preset Update** `risk:low` `depends:[S01]`
  > After this: Golden test passes with updated baseline reflecting new default params; presets load with correct Euclidean config

## Boundary Map

Not provided.
