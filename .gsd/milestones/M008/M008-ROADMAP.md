# M008: Rhythmic Grammar Extensions

**Vision:** Unlock non-Western and additive rhythmic structures. Builds on M007 phrase and evolution features. Adds aksak/additive meters from unequal cell sequences, a timeline mode for invariant reference lanes, extended per-step micro-timing maps, genre-specific preset packs (Afro-House, Balkan, Carnatic-inspired, IDM, Afrobeat), and cross-rhythm visualization showing how lanes align and diverge.

## Success Criteria

- Additive cycle mode supports unequal cell sequences (e.g. 2+2+3 for 7/8 aksak)
- Timeline lane mode provides invariant rhythmic reference immune to macro changes
- Per-step micro-timing maps enable groove templates beyond swing
- Genre preset packs cover at least 5 distinct rhythmic traditions
- Cross-rhythm visualization shows lane alignment in the UI

## Slices

- [ ] **S01: Additive and Aksak Cycle Mode** `[sketch]` `risk:high` `depends:[]`
  > After this: Define a lane cycle as [2,2,3] for 7/8 aksak — Euclidean distribution operates over the unequal cells

- [ ] **S02: Timeline Mode** `[sketch]` `risk:medium` `depends:[]`
  > After this: Mark a lane as timeline — it becomes immune to macro changes and serves as the rhythmic skeleton

- [ ] **S03: Extended Micro-timing Maps** `[sketch]` `risk:medium` `depends:[]`
  > After this: Apply a per-step timing map (not just swing) — e.g. Brazilian ginga feel or J Dilla pocket

- [ ] **S04: Genre Preset Packs** `[sketch]` `risk:low` `depends:[S01,S02,S03]`
  > After this: Load genre-specific presets — Afro-House, Balkan, Carnatic-inspired, IDM, Afrobeat patterns using M007+M008 features

- [ ] **S05: Cross-rhythm Visualization** `[sketch]` `risk:medium` `depends:[S01]`
  > After this: UI view showing how all lanes align and cross over time — polyrhythmic convergence points visible

## Boundary Map

Not provided.
