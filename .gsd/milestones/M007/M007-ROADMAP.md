# M007: Phrase Architecture and Pattern Evolution

**Vision:** Make patterns breathe and evolve — add phrase-level structure (lengths, gaps, offsets), per-cycle mutation, gradual phase drift, per-voice micro-timing, and complementary kotekan pairs. Extends the engine from continuous Euclidean loops into musically phrased, evolving grooves that cover a much wider genre range.

## Success Criteria

- Lanes can be configured with independent phrase lengths and gap durations
- Patterns mutate subtly across cycles when mutation rate is non-zero
- Phase drift creates gradual Reich-style phasing between lanes
- Per-lane timing offset enables pocket/groove feel beyond swing
- Kotekan pair mode generates complementary interlocking patterns
- All new features are deterministic given same (patch, seed, transport)
- Golden tests updated to cover phrase/mutation/drift scenarios
- Factory presets updated to demonstrate new capabilities
- No RT safety regressions — all new code allocation-free in renderRange()

## Slices

- [x] **S01: S01** `risk:medium` `depends:[]`
  > After this: Load Poly in Cubase — lanes play for N bars then rest for M bars, with different phrase lengths per lane creating offset polyrhythmic phrases

- [x] **S02: S02** `risk:medium` `depends:[]`
  > After this: With mutation rate > 0, each cycle of a pattern has subtle variations — displaced hits, ghost notes, occasional substitutions

- [x] **S03: S03** `risk:high` `depends:[]`
  > After this: Two lanes with identical patterns gradually phase against each other over bars, creating Reich-style emergent rhythms

- [x] **S04: S04** `risk:low` `depends:[]`
  > After this: Kick channel offset +5ms, hi-hat -3ms creating a human pocket feel that swing alone cannot achieve

- [x] **S05: S05** `risk:medium` `depends:[]`
  > After this: Mark two lanes as a kotekan pair — one gets the Euclidean hits, the other fills the gaps, creating interlocking Balinese-style patterns on two MIDI notes

- [x] **S06: S06** `risk:low` `depends:[]`
  > After this: New preset bank demonstrating phrase breathing, mutation, drift, kotekan, and groove pocket across genres

- [x] **S07: S07** `risk:low` `depends:[]`
  > After this: 

## Boundary Map

Not provided.
