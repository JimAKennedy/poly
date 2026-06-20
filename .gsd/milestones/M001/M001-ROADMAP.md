# M001: Definition and Architecture De-risking

**Vision:** Lock the Cubase-first VST3 model, establish poly_engine isolation boundary, and prove deterministic timing before any feature work. An empty instrument loads in Cubase, the engine compiles without the VST3 SDK, and golden tests verify byte-identical output under transport stress.

## Success Criteria

- Empty VST3 instrument instantiates in Cubase without error
- poly_engine compiles and passes tests with no VST3 SDK dependency
- Engine harness produces deterministic event stream that is byte-identical under loop restart, tempo change, and position jump
- Repository layout matches planned structure with engine/plugin/tests separation
- Phase 0 documentation (PRD, engine-spec, Cubase workflow) complete

## Slices

- [x] **S01: S01** `risk:medium` `depends:[]`
  > After this: Empty VST3 plugin instantiates in Cubase without error; clean build with all warnings enabled

- [x] **S02: S02** `risk:low` `depends:[]`
  > After this: Harness accepts a patch and transport config, outputs structured note events to stdout; engine compiles without VST3 SDK

- [x] **S03: S03** `risk:high` `depends:[]`
  > After this: Golden test suite passes: byte-identical event stream under loop, tempo, and jump stress scenarios

- [x] **S04: S04** `risk:low` `depends:[]`
  > After this: Complete docs/ directory with PRD, engine-spec, Cubase workflow guide, wireframe images, and automation mapping specification

- [x] **S05: S05** `risk:low` `depends:[]`
  > After this: Comprehensive review of M001 deliverables: architecture decisions, test coverage analysis, engine API surface audit, and milestone retrospective

## Boundary Map

Not provided.
