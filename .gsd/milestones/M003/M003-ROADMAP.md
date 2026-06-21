# M003: Musical Refinement and Cubase Polish

**Vision:** Turn the groove generator into a composition tool. Extended envelope system with full target set, scene/snapshot switching with morph, structural constraints that protect musical anchors, polished Cubase automation mapping, MIDI export workflow for composition integration, and phase/envelope interaction visualizations. The user can write a full rhythmic section while keeping structural anchors intact.

## Success Criteria

- User can write a full rhythmic section while keeping structural anchors intact
- Multiple simultaneous envelopes per lane with full target set functional
- Scene A/B switching and morph produces smooth musical transitions
- Constraints protect structural elements (kick anchor, backbeat, density ranges)
- MIDI capture/drag-to-track/freeze workflow functional in Cubase
- Phase and envelope visualizations help users understand evolving polymetric structure

## Slices

- [x] **S01: S01** `risk:medium` `depends:[]`
  > After this: Envelopes target length, looseness, activation, and fill alongside velocity/density; 4/8/16/custom bar phrase lengths create section-scale evolution

- [x] **S02: S02** `risk:high` `depends:[]`
  > After this: User stores two scenes, switches instantly, and morphs between them with a crossfader producing smooth musical transitions

- [x] **S03: S03** `risk:high` `depends:[]`
  > After this: Anchor kick remains stable even when density envelope bottoms out; backbeat survives high-syncopation macro settings

- [x] **S04: S04** `risk:low` `depends:[]`
  > After this: Cubase automation lanes show well-organized, clearly named parameters; high-value controls are easy to find and automate

- [x] **S05: S05** `risk:high` `depends:[]`
  > After this: User captures a generated groove as MIDI on a Cubase track for further editing

- [x] **S06: S06** `risk:medium` `depends:[]`
  > After this: UI shows how lane cycles align and diverge over time; envelope curves visible with their modulation targets

## Boundary Map

Not provided.
