# S02: Pattern Mutation — UAT

**Milestone:** M007
**Written:** 2026-06-22T19:48:56.174Z

## UAT: S02 Pattern Mutation

### Setup
1. Build and deploy: `cmake --build build`
2. Copy VST3 to plugin folder
3. Rescan plugins in Cubase

### Test Cases

#### TC1: Zero mutation = no change
1. Load Poly with default preset
2. Set Mutation to 0% on all lanes
3. **Verify:** Pattern sounds identical to before — no variation between cycles

#### TC2: Low mutation adds subtle variation
1. Set hi-hat lane mutation to ~20%
2. Play 8+ bars
3. **Verify:** Some cycles have slightly different hi-hat patterns — occasional missing hits, ghost notes, or extra hits. Pattern still feels coherent.

#### TC3: High mutation = heavy variation
1. Set kick lane mutation to ~80%
2. Play 4+ bars
3. **Verify:** Every cycle sounds noticeably different from the last. Pattern becomes chaotic but deterministic (same position = same output on replay).

#### TC4: Anchor protection
1. Set mutation to 100% on a lane
2. Mark steps 0 and 2 as anchors in the constraint system
3. **Verify:** Steps 0 and 2 always play at normal velocity regardless of mutation rate.

#### TC5: Determinism
1. Set mutation to 30% on any lane
2. Play from bar 1, note the pattern
3. Stop and restart from bar 1
4. **Verify:** Identical pattern on replay — mutation is position-based, not random.
