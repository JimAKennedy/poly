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

- [ ] **S01: Extended Envelope System** `[sketch]` `risk:medium` `depends:[]`
  > After this: Envelopes target length, looseness, activation, and fill alongside velocity/density; 4/8/16/custom bar phrase lengths create section-scale evolution

- [ ] **S02: Scene and Snapshot System** `[sketch]` `risk:high` `depends:[S01]`
  > After this: User stores two scenes, switches instantly, and morphs between them with a crossfader producing smooth musical transitions

- [ ] **S03: Constraint Layer** `[sketch]` `risk:high` `depends:[]`
  > After this: Anchor kick remains stable even when density envelope bottoms out; backbeat survives high-syncopation macro settings

- [ ] **S04: Cubase Automation Polish** `[sketch]` `risk:low` `depends:[]`
  > After this: Cubase automation lanes show well-organized, clearly named parameters; high-value controls are easy to find and automate

- [ ] **S05: MIDI Export Workflow** `[sketch]` `risk:high` `depends:[S03]`
  > After this: User captures a generated groove as MIDI on a Cubase track for further editing

- [ ] **S06: Phase and Envelope Visualization** `[sketch]` `risk:medium` `depends:[S01]`
  > After this: UI shows how lane cycles align and diverge over time; envelope curves visible with their modulation targets

## Boundary Map

Not provided.
