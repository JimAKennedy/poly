<!-- counts-ok: roadmap doc — restates current counts as a point-in-time baseline; authoritative counts live in site/src/generated/counts.json and docs/preset-taxonomy.md -->

# Poly Roadmap — The Rhythm-Design-Space Milestones

**Status:** Active planning document (2026-07-23). Supersedes the archived
`IMPLEMENTATION_PLAN.md` for forward planning. Milestone/task tracking happens in
GitHub milestones and `.gsd/`; this doc is the design contract those milestones
implement.

**Sources synthesized here:**

1. *Layered Repetition as Sonic Architecture* (rhythmic-layering research paper) —
   cross-cultural grammar of layered ostinati; compositional principles in its §11.
2. Codebase assessment (2026-07-23, HEAD `cf73ec4`) — what the engine/plugin/WebUI
   actually implement today.
3. External review (Perplexity, no repo access) — the "descriptor-driven design
   space" reframing, corrected against the real codebase.

---

## 1. Vision

> Build a descriptor-driven rhythm design layer that analyzes multi-lane grooves,
> exposes interpretable music-theoretical characteristics graphically, and proposes
> coherent alternatives through constrained stochastic variation within archetype-
> and theory-based style spaces — making Poly both the definitive MIDI drum pattern
> creation tool and an instrument that *teaches* the mathematics and traditions
> behind the patterns it generates.

The strategic reframing: **from "generate more patterns" to "navigate a
rhythm-design space with explicit musical descriptors."** Poly's six macros
(complexity, density, syncopation, swing, tension, humanize) are today *open-loop*
heuristics — the syncopation macro pushes `syncopationOffset` into lanes, but
nothing measures the syncopation that results. This roadmap closes the loop:

1. **Measure** — compute descriptors of what the engine produced (M-A).
2. **Name** — recognize and retrieve documented patterns via an archetype library (M-B).
3. **Search** — propose ranked alternatives that hit descriptor targets (M-C).
4. **Steer** — manipulate descriptor targets graphically (M-D).
5. **Compose** — encode how layers relate (grammar), how timbre carries rhythm
   (articulation), and how structure evolves at scales above the cycle (M-E/F/G).

---

## 2. Baseline: what exists at `cf73ec4`

Implemented and tested (do **not** re-plan these):

| Area | Status |
| --- | --- |
| 8 lanes, independent cycles/subdivisions (true polymeter) | `types.h`, `engine.cpp` |
| Euclidean generation + rotation; additive/aksak cells | `euclidean.cpp`, `computeAdditiveCells` |
| Kotekan interlock (complement of source lane) | `buildLanePattern` |
| Per-lane tempo multipliers (Nancarrow) | `prepareLaneContext` |
| Probability, mutation, drift; swing/humanize/micro-timing | `engine.cpp` |
| Envelope superposition, 8 targets, per-lane + global | `envelope.cpp` |
| Constraints: anchor steps, backbeat protect, density bounds | `constraint.cpp` |
| Scenes A/B, full-state morph, scene chains | `scene.cpp`, `scene.h` |
| 6 macros mapped coherently onto lane params | `macro.cpp` |
| Determinism: position-hash RNG, absolute-PPQ phase, golden tests | `rng.h`, `tests/golden_tests.cpp` |
| RT-safe plugin: lock-free handshakes, versioned state, MIDI capture/SMF export | `processor.cpp`, `state_io.h` |
| WebUI editor (shipping), JSON bridge with published schema | `plugin/source/webui/`, `webui/` |
| 43 factory presets in 10 categories; generated docs/site pipeline | `presets.cpp`, `docs/preset-taxonomy.md` |

**The gaps this roadmap addresses:**

- **No characterization.** Nothing computes properties of generated output — no
  syncopation index, evenness, geometry, or distance metrics. Theory exists only
  as guide-site prose.
- **No pattern vocabulary.** Presets are compiled whole-patch C++ functions; there
  is no lane-level, user-extensible, metadata-carrying library of documented patterns.
- **No proposal/search.** Randomize is blind; there is no "sample near this target."
- **Lanes are peers, not roles.** Except kotekan, no typed relationships to an
  anchor (the single strongest generative principle in the research paper).
- **One timbre per lane.** No per-step articulation (dumm/takk, open/closed) —
  timbre-as-rhythmic-information is absent.
- **No explicit multi-scale model.** Envelopes + scene chains approximate macro
  structure; no arrangement timeline, nested/colotomic hierarchy, cycle-position
  weight profiles, or directed processes (construction/reduction, phase-lock, korvai).

---

## 3. Invariants (apply to every milestone)

These are non-negotiable constraints inherited from the existing architecture:

1. **Engine purity.** `poly_engine` (and the new `poly_analysis`) stay pure C++
   static libraries: no VST3, no I/O, no globals, compile and pass tests without
   the SDK. Analysis is *not* RT code — it runs on the UI thread — but purity keeps
   it testable and WASM-buildable.
2. **Determinism.** Same `(patch, seed, transport)` → identical output. Descriptor
   computation and proposal generation must be seeded and reproducible. Never use
   wall-clock or nondeterministic iteration order. Extend the golden-test discipline
   to descriptor values.
3. **RT safety.** Nothing in this roadmap touches the audio thread's character.
   Casting archetypes, computing descriptors, and generating proposals happen on
   the UI/controller side and reach the processor through the existing lock-free
   single-slot handshakes. No allocation/locks/exceptions in `process()`/`renderRange()`.
4. **State versioning.** Any `GrooveState` extension bumps `kStateVersion`,
   branches in `setState()`, and adds round-trip + fuzz coverage (`fuzz_state_io.cpp`).
5. **Generated-artifact discipline.** New schemas/data follow the existing pattern:
   single source of truth, generated JSON, drift-check script wired into CI
   (like `bridge.schema.json`, `presets.json`, the Euclidean appendix).
6. **Graceful degradation.** Serialized patches must load correctly when a user
   archetype library is absent: store *resolved* pattern data plus optional
   archetype ID references, never bare references.
7. **Attribution.** Factory archetype metadata (tradition names, lineage,
   references) must be sourced — generate from / cross-check against the guide
   site's researched content. Sourced `references` are mandatory for factory
   entries.

---

## 4. Milestones

Dependency graph:

```
M-A (descriptors) ──► M-B (archetypes + recognition) ──► M-C (proposals) ──► M-D (UI)
        │                        │                            ▲
        │                        └── M-E (grammar) ───────────┘
        └──────────► (site/WASM educational surfacing, ongoing)
M-F (articulation)  — independent, any time after M-A
M-G (multi-scale)   — after M-C; uses descriptors for arc/novelty measurement
M-H (NL groove spec / chat) — after M-A+B+C minimum; gains M-E (relationships)
                              and M-G (processes/arrangement) as they land
```

---

### M-A — Descriptor engine (`poly_analysis`)

**Goal:** a pure static library computing interpretable, referenced,
music-theoretical descriptors of lanes, composites, lane relationships, and
temporal evolution. This is the foundation everything else scores against.

**New code:** `analysis/` (mirror `engine/` layout: `analysis/include/poly/`,
`analysis/src/`, own CMake target `poly_analysis`, links only against `poly_engine`
headers/types).

**Descriptor tiers and metrics (v1):**

1. **Lane (single cyclic pattern).** Input: onset set + optional velocities on an
   n-step cycle.
   - Onset count, density ratio.
   - Inter-onset-interval (IOI) histogram and gap variance.
   - Evenness / maximal-evenness distance (vs the Euclidean pattern with same k,n —
     Toussaint, *The Geometry of Musical Rhythm*, 2013).
   - Balance: magnitude of the onset centroid on the unit circle (Milne et al.).
   - Syncopation: Longuet-Higgins & Lee (1984) metric-weight model; plus
     Toussaint off-beatness as a cheap second opinion.
   - Erdős-deepness, palindromicity (circular), rhythmic oddity.
   - Accent descriptors (velocity-aware): accent contrast, ghost-note ratio,
     downbeat weight, backbeat strength, offbeat bias.
2. **Composite (union of all active lanes at a common resolution).**
   - The resultant pattern itself (Reich's "resultant") at LCM resolution.
   - All lane metrics applied to the composite.
   - Composite density and per-pulse coverage profile.
3. **Relational (per lane pair, and lane-vs-anchor).**
   - Coincidence rate / anti-coincidence rate (Arom's "balance of coincidences").
   - Complementarity (fraction of one lane's onsets falling in the other's gaps);
     interlock score (bidirectional complementarity — kotekan detector).
   - Overlap (shared onsets at common resolution).
   - Phase relation: cycle-length ratio (reduced fraction), current phase offset,
     super-cycle (LCM) length in bars.
   - Distances between two patterns of equal length: swap distance, edit distance,
     chronotonic (Gustafson) distance — also used for A/B scene distance and
     mutation-drift measurement.
4. **Temporal evolution (over a rendered span).**
   - Repetition stability (self-similarity of successive cycles).
   - Novelty curve across the super-cycle (composite change per cycle).
   - Envelope divergence / phrase variance (how much shaping departs from the
     static pattern).
   - Fill emergence rate.

**API sketch:**

```cpp
namespace poly::analysis {
struct LaneDescriptors { /* metrics above, all doubles/ints */ };
struct RelationalDescriptors { /* pairwise metrics */ };
struct CompositeDescriptors { /* union metrics + coverage */ };
struct GrooveDescriptors {
    std::array<LaneDescriptors, kMaxLanes> lanes;
    CompositeDescriptors composite;
    // pairwise, indexed (i,j) i<j, active lanes only
    std::vector<RelationalDescriptors> relations;  // pre-sized, non-RT use only
};
LaneDescriptors analyzeLane(const StepPattern& p, const VelocityInfo* v = nullptr);
GrooveDescriptors analyzeGroove(const GrooveState& s);
GrooveDescriptors analyzeRendered(std::span<const NoteEvent> events, /* span info */);
}
```

`analyzeGroove` characterizes the *specified* pattern; `analyzeRendered`
characterizes what the engine actually emitted (post probability/mutation/
envelopes) by binning a headless render — both matter, and their divergence is
itself informative (it *is* the temporal-evolution tier).

**Deliverables:**

- [ ] `poly_analysis` library + unit tests per metric against hand-computed
      values for canonical rhythms (son clave E(5,16), rumba clave, tresillo
      E(3,8), standard bell E(7,12), gahu, bossa, shiko, soukous — the Toussaint
      six-plus set makes ideal fixtures).
- [ ] Descriptor golden tests: compute `GrooveDescriptors` for all 43 factory
      presets, check in as golden files, assert byte-stable in CI (extend
      `tests/golden/` conventions).
- [ ] Separation sanity test: assert that descriptor vectors *distinguish*
      musically distinct presets (e.g. pairwise distance above a floor for
      presets in different categories). If Bossa Nova ≈ Minimal Techno in
      descriptor space, the descriptors are wrong.
- [ ] WASM build of `poly_analysis` wired into the site build (reuse the existing
      wasm freshness check pattern) — enables interactive descriptor exploration
      on the guide site later. May land as a stub target if site work is deferred.
- [ ] `docs/descriptor-spec.md`: every metric with formal definition, literature
      reference, value range, and worked example. This doc is the contract for
      M-C ranking and M-D UI.
- [ ] Minimal read-only surfacing: descriptor values for the current groove
      exposed over the WebUI bridge (extend `bridge.schema.json`) and shown in a
      basic panel. Full UI is M-D; this proves the plumbing.

**Acceptance:** all metrics unit-tested against literature examples; preset golden
descriptors stable across platforms in CI; bridge exposes descriptors; no engine
RT-path changes.

---

### M-B — Archetype library + recognition

**Goal:** a lane-level, user-extensible, metadata-rich library of documented
patterns — pattern + role + meaning — used for construction (browse/cast/suggest)
and education (provenance, lineage, recognition). Distinct from presets: presets
are full starting points; archetypes are building blocks.

**Schema (`archetype.schema.json`, published like `bridge.schema.json`):**

```json
{
  "id": "std-bell-12",
  "schemaVersion": 1,
  "name": "Standard Bell Pattern",
  "pattern": { "steps": 12, "onsets": [0, 2, 4, 5, 7, 9, 11], "subdivision": 8 },
  "euclidean": { "k": 7, "n": 12, "rotation": 0 },
  "accents": { "profile": [0], "ghosts": [] },
  "role": "anchor",
  "densityStratum": "medium",
  "tradition": ["Ewe / Agbekor", "pan-West-African"],
  "aliases": ["bembe bell", "6/8 clave"],
  "geometry": { "computed": "by poly_analysis at build time — do not hand-author" },
  "norms": {
    "syncopationRange": [0.3, 0.6],
    "densityRange": [0.5, 0.65],
    "mutationBounds": { "maxSwapDistance": 1, "protectedOnsets": [0] }
  },
  "companions": [
    { "id": "sogo-displaced", "relationship": "displace" },
    { "id": "hemiola-3-over-2", "relationship": "phase-against" }
  ],
  "notes": "Temporal reference for the whole ensemble; played without variation.",
  "references": ["Toussaint 2013 ch.7", "site:/traditions/ewe"]
}
```

Key schema decisions:

- `geometry` is **generated** by running `poly_analysis` over the pattern at
  build time (factory) or import time (user) — never hand-authored. One more
  drift-checked generated artifact.
- `norms` and `mutationBounds` are what make M-C proposals *stylistic*: they
  define the neighborhood an archetype may be mutated within (typical
  syncopation/density ranges, protected onsets, max swap distance, forbidden
  zones). Optional in v1 user files; required for factory entries.
- `companions` + `relationship` use the M-E relationship vocabulary (see below);
  in M-B they power *suggestions only* (UI hints), not constraints.
- Ensemble templates: a second, thin schema (`ensemble.schema.json`) listing
  archetype IDs + lane assignments + relationships ("Agbekor ensemble",
  "salsa rhythm section"). Factory presets progressively refactor into these.

**Library mechanics:**

- Factory library: one generated, checked-in JSON (seeded from the research
  paper's documented patterns — standard bell, son/rumba clave, tresillo, cinquillo,
  gahu, tumbao, montuno accent pattern, gonguê, caixa, zabumba pair, polos/sangsih
  kotekan pair, colotomic strata set, teentaal/rupak clap-wave profiles, aksak
  2+2+3 / 2+3+2 / 3+2+2 / 2+2+2+3 / kopanitsa cells, Amen kick/snare figures,
  four-on-floor + offbeat-hat set, Reich Clapping Music cell). Target ≥ 40
  entries at launch; every entry carries sourced `references`.
- User libraries: JSON files in a watched per-user folder (platform-standard
  location alongside DAW user presets), validated against the schema on load;
  invalid files reported, never crash. Multiple library files supported.
- "Save lane as archetype" from the UI: analysis auto-fills `geometry`; user
  supplies name/notes/tradition. Round-trip is the user-extensibility on-ramp.
- **Recognition:** nearest-match of any lane (or user-drawn pattern) against the
  library in descriptor space + exact/rotation match on onset sets. Surfaces
  "this is son clave (rotation 0)" or "2 swaps from rumba clave." Shared code
  with M-A distances.

**Placement:** library loading/validation/recognition is UI-/controller-side
(non-RT). A small pure `poly_library` target (JSON parsing may use a vendored
header-only lib — allowed because it is *not* part of `poly_engine`) keeps it
testable headlessly. Casting an archetype = producing ordinary lane state through
existing parameter paths; the engine never knows archetypes exist. Serialized
state stores resolved patterns + optional archetype IDs (invariant #6).

**Deliverables:**

- [ ] `archetype.schema.json` + `ensemble.schema.json`, published + drift-checked.
- [ ] `poly_library` target: load, validate, index, recognize; unit tests
      including malformed-file handling.
- [ ] Factory library JSON (≥ 40 sourced entries) generated with computed geometry.
- [ ] WebUI: archetype browser (filter by tradition / role / cycle length /
      geometric property), cast-to-lane, lane provenance badge, companion
      suggestions, save-lane-as-archetype.
- [ ] Recognition wired into the descriptor panel from M-A.
- [ ] Refactor at least 5 factory presets as ensemble templates to prove the
      composition model (full refactor of `presets.cpp` can be incremental).
- [ ] Docs: `docs/archetype-guide.md` (authoring user libraries) + site page
      linking archetypes ↔ existing tradition chapters.

**Acceptance:** cast → edit → save → re-recognize round-trips; user library
folder works with graceful failure; state saved with a missing user library
loads correctly; schema drift check green.

---

### M-C — Proposal engine (constrained variation + ranking)

**Goal:** "randomize" becomes "sample around this target." Generate ranked,
coherent alternatives that move measured descriptors toward user targets while
respecting constraints and archetype norms.

**Design — generate-and-test, exploiting existing architecture:**

1. **Search state:** current `GrooveState` (or a subset of unlocked lanes).
2. **Move set (per candidate, seeded):** onset swap/add/remove within
   `mutationBounds`; rotation; accent/ghost reassignment; velocity-spread nudge;
   swing/syncopation-offset nudge; phase/rotation offset change; density change
   via Euclidean k±1; envelope shape/amount nudge. Anchor steps, backbeat
   protection, and locked lanes are **hard** constraints (reuse `constraint.cpp`
   semantics); archetype norms and style forbidden zones are hard bounds when a
   lane carries an archetype identity.
3. **Score:** render candidate headlessly over one super-cycle (the engine is a
   cheap pure function — reuse the `tools/harness/` path), run `poly_analysis`,
   rank by weighted distance to the target descriptor vector; tie-break with a
   diversity term so the top-N aren't near-duplicates.
4. **Reproducibility:** proposal batch = f(patch, targets, locks, batchSeed).
   Same inputs → same candidates, always. Golden-testable.

All UI-thread/offline; results become ordinary state updates through existing
handshakes.

**Deliverables:**

- [ ] `poly_propose` module (may live inside `analysis/` or its own target):
      move set, constraint filter, scorer, ranker; deterministic under batchSeed.
- [ ] Descriptor-target model: which descriptors are steerable in v1 (suggest:
      composite syncopation, composite density, lane evenness, lane-pair overlap/
      complementarity, phase interest = super-cycle novelty) — defined in
      `docs/descriptor-spec.md` as "targetable: yes/no."
- [ ] Golden tests: fixed (patch, targets, seed) → fixed ranked candidate list.
- [ ] Behavioral tests: proposals measurably move the targeted descriptor toward
      target without violating locks/anchors/norms.
- [ ] Minimal UI: "propose alternatives" on the descriptor panel with lane locks
      and accept/audition/reject (full descriptor-first surface is M-D).
- [ ] Accept/reject logging (local, private, versioned format) — data collection
      only; no learning loop yet.

**Acceptance:** proposals deterministic, constraint-respecting, and
descriptor-effective; audition/accept round-trip works in the WebUI; RT path
untouched.

---

### M-D — Descriptor-first UI

**Goal:** graphical manipulation of rhythm characteristics — the descriptor layer
becomes the primary creative surface, evolving (not replacing) the existing
macro row and sequencer views.

**Components (WebUI; extend `groove-math.js` + bridge):**

- [ ] Radar/profile chart of the composite descriptor vector: current value vs
      draggable target; drag → M-C proposal batch.
- [ ] Lane-relationship matrix (overlap / complementarity / coincidence per pair,
      color-coded; cells click through to the pair's phase detail).
- [ ] Rhythm-circle view per lane: onset polygon on the circle (Toussaint
      geometry made visible), with recognition label and archetype provenance;
      extends the existing phase-wheel/cross-rhythm view.
- [ ] Super-cycle strip: LCM length, per-cycle novelty curve, current position —
      the "why it evolves without repeating" view.
- [ ] Candidate tray: ranked proposals with per-candidate descriptor deltas,
      audition (temporary cast), accept, reject, lock-lane toggles.
- [ ] Educational hooks on every metric: tooltip definition + link to
      site chapter (reuse `docs/descriptor-spec.md` content — single source).

**Deliverables also include:** bridge schema extensions (drift-checked), Playwright
specs for each new panel (existing `webui/tests/` patterns), visual-regression
baselines, and a `docs/ui-guide.md` update.

**Acceptance:** a user can go archetype → analyze → drag target → audition →
accept without touching a raw parameter; all specs green.

---

### M-E — Layering grammar (roles & relationships)

**Goal:** formalize "each layer has a unique relationship to the anchor" — the
research paper's strongest generative principle — as data the proposal engine
enforces and the UI explains.

**Vocabulary (v1):**

- Roles: `anchor`, `backbeat`, `subdivision-carrier`, `cross-rhythm`,
  `response/ornament`, `fill/disruption`, `bass-support` (rhythmic only).
- Relationships (directed, lane → reference lane): `reinforce`, `interlock`,
  `displace`, `shadow`, `anticipate`, `answer`, `phase-against`, `avoid-overlap`.

Each relationship maps to (a) a *descriptor predicate* used for scoring/validation
(e.g. `interlock` → complementarity ≥ threshold; `displace` → constant non-zero
onset offset; `avoid-overlap` → overlap ≤ threshold) and (b) a *constraint* the
M-C move set must respect. Kotekan's existing `kotekanSourceLane` becomes the
first migrated relationship (`interlock`, engine-enforced variant).

**State:** per-lane optional `role` + optional `(relationship, targetLane)` —
`GrooveState` extension ⇒ `kStateVersion` bump, serialization + fuzz tests.
Engine may remain unaware of most relationships (they constrain *generation*, not
rendering); only engine-enforced ones (kotekan-style derivation) touch
`renderRange`, and those follow existing patterns.

**Deliverables:**

- [ ] Vocabulary + predicate definitions appended to `docs/descriptor-spec.md`.
- [ ] State model + versioned serialization + tests.
- [ ] M-C integration: relationships as hard constraints and score terms.
- [ ] M-B integration: companion suggestions become one-click "add companion
      lane with relationship."
- [ ] UI: role badges on lanes; relationship edges drawn in the lane-relationship
      matrix; violation warnings (advisory, never blocking manual editing).
- [ ] Ensemble templates upgraded to carry roles/relationships.

**Acceptance:** an "anchor + 3 typed companions" groove survives proposal
batches with all relationship predicates holding; templates instantiate with
correct roles.

---

### M-F — Per-step articulation (timbre as rhythmic information)

**Goal:** let a lane speak with 2–3 timbres (dumm/takk, open/closed, low/high —
the iqa'/zabumba/conga model), because in most documented traditions the timbral
sequence is constitutive of the pattern, not decoration.

**Design:** per-step articulation index (0 = default) into a per-lane articulation
table mapping index → (MIDI note, channel, velocity scale). Default table has one
entry ⇒ existing behavior preserved bit-for-bit (golden tests must not change for
untouched patches). Archetype patterns gain optional per-onset articulation
(schema addition, versioned). Mutation/probability treat articulation as
mutable-within-bounds (a dumm must not silently become a takk if norms forbid it).

**Touchpoints:** `types.h` (+`kStateVersion`), `engine.cpp` emission path
(RT-safe, fixed-size), `state_io` + fuzz, NoteMap interaction, WebUI step editor,
archetype schema v2, new golden tests for articulated presets.

**Acceptance:** existing golden refs unchanged; new articulated fixtures golden-
tested; at least the iqa' dumm/takk and zabumba archetypes re-expressed with
articulation.

---

### M-G — Multi-scale structure & process generators

**Goal:** explicit design above the cycle: nested hierarchy, cycle-position
weights, and *directed processes* — the difference between "play B after A" and
"arrive at B from A by a perceptible process" (Reich).

**Components (sequence within the milestone as separate PRs):**

1. **Cycle-weight profiles:** per-lane (or global) weight per cycle position —
   sam emphasis, khali emptiness, usul front-heaviness — richer than binary
   accent masks; feeds velocity and the LHL syncopation weights used in M-A.
2. **Colotomic scaffold generator:** one gesture creates a nested marker set
   across lanes (every 2 / 8 / 16 / 32 with offsets — gamelan levels), as an
   ensemble-template special case.
3. **Process objects:** parameterized, deterministic, PPQ-driven transformations
   with a defined start/end and duration:
   - Rhythmic construction/reduction (beat-for-rest substitution, Reich *Drumming*).
   - Phase-shift-and-lock (continuous offset ramp → lock at target step, *Piano Phase*).
   - Konnakol transformations (double/halve/augment a cell across repetitions).
   - Korvai cadence: generate a phrase engineered to resolve on the downbeat of a
     chosen future cycle (constraint-solved; small search space).
4. **Arrangement timeline:** ordered sections over bars/super-cycles, each
   holding scene + active processes + macro/descriptor targets — replacing
   envelope workarounds for macro form. Scene chains become a special case.
   Novelty/tension curve from M-A visualized along the timeline.

All PPQ-derived (no accumulation), deterministic, golden-tested. Timeline and
processes are `GrooveState`/session-state extensions with version bumps.

**Acceptance:** a patch can specify "8 bars construction → 16 bars phase-shift →
korvai resolving on bar 32" and render it deterministically; timeline UI shows
measured novelty against the plan.

---

### M-H — Natural-language groove specification (chat interface)

**Goal:** a chat interface where a user describes a groove in prose — *"an EDM
bass and snare combination with toms supporting a call-and-response interaction
over a 4-bar period, layers combining into a shimmering cymbal effect with phase
effects over a 12–36 bar period, and breaks built into the tom and cymbal cycles"*
— and Poly proposes configurations that meet the specification.

**The architectural principle: the LLM is a compiler, not a generator.** It
translates prose into a structured *groove spec* — descriptor targets (M-A) +
archetype/template selections (M-B) + roles and relationships (M-E) + process/
arrangement directives (M-G) — validated against a published JSON schema. The
deterministic, seeded M-C proposal engine then does the actual generation and
ranking. The LLM never emits a note. This preserves determinism, theory
grounding, explainability, and testability. Every clause of the example prompt
maps onto roadmap constructs:

| Prose clause | Spec construct |
| --- | --- |
| "EDM bass and snare combination" | Ensemble template / archetypes, House–Techno style space (M-B) |
| "toms… call and response over 4 bars" | Role `response`, relationship `answer` to anchor (M-E) + 4-bar phrase length |
| "shimmering cymbals effect" | High-density stratum + kotekan interlock pair (engine) |
| "phase effects over a 12–36 bar period" | Cycle lengths constrained so super-cycle LCM ∈ [12, 36] bars + drift/offsets (M-A super-cycle descriptor) |
| "breaks in the tom and cymbal cycles" | Phrase gating + FillLikelihood envelopes (engine); construction/reduction processes (M-G) |

**Approach options (decision record):**

1. **Frontier LLM, no training — chosen for v1.** Claude API with structured
   outputs (`output_config.format` forcing a schema-valid spec) plus a small
   agentic tool-use loop (SDK tool runner). Costs cents per chat turn with
   prompt caching on the static system prompt.
2. **Fine-tuned small open-weights model** — only if offline/embedded operation
   becomes a requirement. Training data via synthetic backtranslation: sample
   valid specs / presets / archetypes → render → compute descriptors → have a
   frontier model write diverse prose descriptions → SFT (LoRA) on the
   resulting (prose → spec) pairs; validate against the eval harness below.
3. **End-to-end symbolic MIDI model — rejected.** Bypasses the engine, destroys
   determinism and the theory/education layer, needs licensed MIDI corpora,
   cannot explain itself.
4. **Embedding retrieval only** — offline fallback path: embed template/
   archetype descriptions, NL query → nearest template + macro nudge. Cannot
   handle compositional requests; keep as degraded mode, not the product.

**v1 design (option 1):**

- **Groove-spec contract:** `groove-spec.schema.json`, published and
  drift-checked like `bridge.schema.json`. One artifact, three consumers: the
  LLM's output contract, the proposal engine's input contract, and the eval
  harness's assertion vocabulary. Vocabulary drawn from `docs/descriptor-spec.md`
  (targetable descriptors), the archetype/ensemble schemas, the M-E role/
  relationship enums, and M-G process types.
- **Tool loop:** the model converses with the user and calls a small tool set —
  `analyze_current_groove` (returns `poly_analysis` descriptors),
  `search_archetypes` (library queries), `propose_candidates` (invokes M-C,
  returns ranked candidates *with their measured descriptors*), `cast_to_lanes`.
  The measured-descriptor feedback is the grounding loop: the model verifies
  that proposals actually hit the targets and iterates when they don't.
  Multi-turn refinement ("sparser", "more tension in bars 9–16") edits the
  persistent spec object incrementally.
- **Model:** default `claude-opus-4-8`; a cheaper tier (e.g. `claude-haiku-4-5`)
  as a budget option. Model IDs current as of 2026-07; re-check at build time.
  System prompt (schemas + descriptor definitions + archetype catalog summary)
  is static → prompt-cache it.
- **Placement:** never in the plugin RT process, and preferably not in the VST
  binary at all. Chat panel lives in the WebUI; API calls go through the WebUI
  backing layer or a small companion service, reaching the processor via the
  existing bridge/handshakes. User-supplied API key, stored outside the plugin
  state (never serialized into patches).
- **Determinism:** a proposal batch remains `f(spec, patch, batchSeed)` — the
  LLM chooses the spec, not the notes. Accepted results are ordinary state
  updates, reproducible without the LLM present.

**Evaluation harness (CI-testable because generation is deterministic):**
a benchmark file of prose prompts, each with assertions on the *resulting
descriptors and structure* — e.g. "super-cycle ∈ [12, 36] bars", "tom lane has
`answer` relationship to anchor", "composite density < 0.5". Pipeline: prompt →
spec → propose → analyze → assert. Schema-validity rate and assertion pass rate
are the regression metrics; run against recorded LLM outputs (fixtures) in CI,
against the live API in a scheduled job.

**Deliverables:**

- [ ] `groove-spec.schema.json` + generator/drift check + spec→engine-state
      compiler (pure, unit-tested, LLM-free — usable and testable on its own).
- [ ] Compiler service: Claude API integration, structured output, tool loop,
      prompt caching, spec persistence per session.
- [ ] WebUI chat panel: conversation, spec inspector (show the user the
      compiled spec — this is itself an educational surface), candidate
      audition/accept/reject wired to the M-C tray.
- [ ] Eval harness: benchmark prompts + descriptor assertions; fixture-based CI
      job; scheduled live-API job.
- [ ] Accept/reject logging extended with the associated prose intent (feeds
      the deferred learned-ranking work).
- [ ] Offline fallback: embedding-retrieval template match (option 4) when no
      API access.

**Sequencing:** requires M-A + M-B + M-C; substantially better after M-E
(relationship vocabulary) and M-G (processes/arrangement). A thin demo (chat →
macro settings + ensemble-template selection) is possible right after M-B.

**Acceptance:** the example prompt above compiles to a valid spec, proposals
satisfy the eval assertions, refinement turns edit the spec incrementally, and
an accepted groove reloads deterministically with the LLM disconnected.

---

## 5. Explicitly deferred (decisions needed before planning)

- **Pitched basslines / harmony coupling.** Poly has no pitch model; chord-tone
  descriptors need harmonic context a MIDI-out instrument can't infer. Rhythmic
  kick/bass coupling is covered by M-E relationships. Pitched generation is a
  product-direction fork — decide separately.
- **Learned proposal ranking.** Deferred by design (as in the archived plan).
  M-C's accept/reject log accumulates the data — enriched by M-H with the prose
  intent behind each accept/reject; revisit only after M-D ships and real usage
  exists. Likely first form: preference-tuning a lightweight candidate
  re-ranker, not the generator.
- **Fine-tuned local NL model.** M-H ships on a hosted frontier LLM; a local
  fine-tuned compiler (M-H option 2) is worth building only if offline/embedded
  operation becomes a product requirement.
- **VSTGUI parity for new surfaces.** New UI lands in the WebUI only; the legacy
  editor stays frozen pending its existing decommission plan.

---

## 6. Suggested implementation order & session sizing

Each checkbox above is roughly one focused implementation session (branch → PR).
Recommended PR-sized slices, in order:

1. M-A: analysis scaffolding + lane metrics (evenness, balance, IOI) + fixtures.
2. M-A: syncopation (LHL, off-beatness) + accent/velocity metrics.
3. M-A: relational metrics + distances; composite; super-cycle/LCM.
4. M-A: rendered-span evolution metrics; preset descriptor goldens; separation test.
5. M-A: bridge exposure + minimal WebUI panel; `docs/descriptor-spec.md`.
6. M-B: schemas + `poly_library` + factory library seed (patterns only).
7. M-B: recognition; browser UI + cast; provenance badges.
8. M-B: user libraries + save-as-archetype; ensemble templates + preset refactor start.
9. M-C: move set + constraints + scorer; golden proposals; minimal propose UI.
10. M-D: radar + candidate tray; relationship matrix; rhythm circles; super-cycle strip.
11. M-E: state model + predicates; proposal integration; UI edges/badges.
12. M-F: articulation engine change; schema v2; UI editor.
13. M-G: weights → scaffold → processes → timeline (4+ PRs).
14. M-H: `groove-spec.schema.json` + spec→state compiler + eval harness
    (LLM-free, fully unit-tested).
15. M-H: compiler service (Claude API, structured output, tool loop) +
    fixture-based CI eval.
16. M-H: WebUI chat panel + spec inspector + candidate tray wiring +
    offline fallback.

Every PR: follows invariants §3; updates goldens intentionally (never
incidentally); adds Playwright coverage for UI; keeps the drift checks green;
bumps `kStateVersion` at most once per PR that touches state.

---

## 7. References

- Toussaint, G. *The Geometry of Musical Rhythm* (2nd ed., 2020) — evenness,
  balance, oddity, deepness, distances, the six distinguished timelines.
- Longuet-Higgins, H. C. & Lee, C. S. "The Rhythmic Interpretation of Monophonic
  Music" (1984) — syncopation model.
- Milne, A. et al. — perfect balance / rhythm centroid measures.
- Gustafson, K. — chronotonic distance.
- Arom, S. *African Polyphony and Polyrhythm* (1991) — coincidence balance,
  interlocking analysis.
- Reich, S. "Music as a Gradual Process" (1968) — process transparency (M-G).
- *Layered Repetition as Sonic Architecture* (project research paper, 2026) —
  cross-cultural grammar; §11 compositional principles; archetype seed list.
- London, J. *Hearing in Time* (2012) — metric hierarchy and well-formedness.
