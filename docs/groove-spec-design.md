---
class: gated
---

<!-- counts-ok: design doc — cites current counts as a point-in-time baseline; authoritative counts live in site/src/generated/counts.json -->

# Groove Spec — Language Design

**Status:** Design document for ROADMAP.md milestone M-H (slice 14). This doc
fixes the language's semantics, strata, and method *before* the schema is
frozen; `groove-spec.schema.json` is generated from the kernel defined here.

The groove spec is the contract between three consumers:

1. the **LLM compiler** (M-H) — emits specs from prose via structured output;
2. the **proposal engine** (M-C) — searches patch space for states satisfying a
   spec;
3. the **eval harness** (M-H) — asserts spec clauses against analyzed output.

It is also usable with no LLM at all: a spec is an ordinary JSON document a
power user (or a test) can write by hand.

---

## 1. Semantic domain

A spec denotes **a region of patch space plus a preference ordering over it**:

```
⟦spec⟧ = ( H : (GrooveState, AnalyzedOutput) → bool,     -- hard predicate
           S : (GrooveState, AnalyzedOutput) → ℝ≥0,      -- soft objective (lower = better)
           G : generator hints )                          -- non-semantic search guidance
```

- **H (hard)** — locks, structural requirements (roles, relationships,
  archetype identity), and bound constraints. A candidate violating H is
  rejected, never ranked.
- **S (soft)** — a weighted sum of distances from descriptor targets. M-C ranks
  surviving candidates by S (plus a diversity term that is *not* part of the
  spec's meaning).
- **G (hints)** — seeds, style-space narrowing, iteration budgets. Hints may
  change *which* satisfying states are found, never *whether* a state
  satisfies the spec. Two specs differing only in G are semantically equal.

Everything below is surface syntax for denoting (H, S, G). The
algebra-vs-calculus question is resolved per stratum (§3); the semantics is
sets and orderings throughout.

**Determinism:** ⟦spec⟧ is a pure value. A proposal batch is
`f(spec, basePatch, batchSeed)`; re-running with identical inputs yields
identical candidates. Nothing in the language may depend on wall-clock, RNG
outside the seed, or LLM state.

---

## 2. Design principles

1. **Declarative only.** A spec states what must/should be true. The "how"
   belongs to M-C. No construct in the language sequences engine operations.
2. **Hard/soft is first-class.** Every clause is explicitly a `require`
   (contributes to H) or a `prefer` (contributes to S, with a weight). The
   same comparison syntax works in both.
3. **Flat carrier, bounded expression depth.** The wire format is a flat JSON
   statement list. Pattern expressions (the one genuinely recursive stratum)
   are depth-bounded (≤ 4). Rationale: LLM structured-output schemas do not
   support recursion, and flat statement lists diff/inspect/version better
   than nested trees.
4. **Defunctionalized processes.** No lambdas. Temporal processes are a
   closed, versioned enum of parameterized record types. Higher-order
   composition lives in the compiler, not the wire format.
5. **Kernel + sugar.** The kernel is minimal; idioms ("shimmer", "backbeat",
   named ensemble templates) are derived forms that desugar to kernel
   statements. The LLM emits sugar freely; only the kernel is consumed by M-C
   and golden-tested. The spec inspector displays desugared form — this is an
   educational surface ("here is what *shimmer* means").
6. **The descriptor spec is the standard library.** Goal-stratum clauses name
   descriptors defined in `docs/descriptor-spec.md` and nothing else.
   Descriptor names are API names; choose them accordingly.
7. **Adjoint to analysis.** For any reachable `GrooveState`, analysis +
   recognition must be able to produce a spec that the state satisfies
   (decompilation, possibly lossy). Gaps in either direction are defects:
   an indescribable state is a language gap; an unsatisfiable-by-construction
   spec shape is an engine gap.
8. **Versioned like every other contract.** `schemaVersion` at top level;
   kernel changes are breaking (major), new sugar is additive (minor); drift
   checks generated alongside `bridge.schema.json` and friends.

---

## 3. The four strata

The domain has four natural levels; each gets the paradigm that fits it.
Mixing them is the design error this section exists to prevent.

### 3.1 Pattern stratum — a term algebra

Patterns are values: onset sets on an *n*-step cycle (binary necklaces).
Closed operator set, each mapping 1:1 onto an engine primitive:

| Expression | Meaning | Engine primitive |
| --- | --- | --- |
| `{"euclid": [k, n]}` | E(k,n), Bjorklund distribution | `euclidean()` |
| `{"rotate": [P, r]}` | rotate pattern P by r steps | `rotation` |
| `{"complement": P}` | onsets ↔ rests of P | kotekan derivation |
| `{"cells": [2, 2, 3]}` | additive/aksak cell concatenation | `cellSizes` |
| `{"fixed": [0, 3, 6, 10, 12], "steps": 16}` | literal onset list | `fixedPattern` |
| `{"archetype": "son-clave-16"}` | library reference (M-B), resolves to its pattern | archetype cast |

Laws the compiler may exploit (and tests must hold): rotation forms a cyclic
group (`rotate(rotate(P,a),b) = rotate(P,a+b mod n)`); complement is an
involution; `euclid` is invariant under the Bjorklund/necklace equivalences
documented in the Euclidean appendix. Expression depth ≤ 4 (sufficient for
every corpus utterance found so far, e.g. `rotate(complement(euclid(5,16)),2)`).

Pattern expressions appear only in *pattern positions* of statements — they
are not themselves statements.

### 3.2 Structure stratum — relations

A typed graph over lanes. Facts, not functions:

```json
{"role": {"lane": "kick", "is": "anchor"}}
{"relation": {"type": "answer", "from": "toms", "to": "kick"}}
{"relation": {"type": "interlock", "between": ["cym_a", "cym_b"]}}
```

- **Roles** (one per lane, optional): `anchor`, `backbeat`,
  `subdivision-carrier`, `cross-rhythm`, `response`, `fill`, `bass-support`.
- **Relationships** (directed unless symmetric): `reinforce`, `interlock`
  (symmetric), `displace`, `shadow`, `anticipate`, `answer`, `phase-against`,
  `avoid-overlap` (symmetric).

Each relationship type is *defined* by a predicate over M-A relational
descriptors (e.g. `interlock` ⇔ bidirectional complementarity ≥ θ;
`avoid-overlap` ⇔ overlap ≤ θ). The predicate definitions live in
`docs/descriptor-spec.md` §relationships and are shared verbatim with M-E —
the spec language and the layering grammar are the same vocabulary.

Lanes are referenced by stable symbolic names declared in the spec's
`ensemble` block (mapped to lane indices at cast time), never by raw index —
specs survive lane reordering.

### 3.3 Goal stratum — constraints over descriptors

Comparisons over named descriptors, in `require` (→ H) or `prefer` (→ S)
position:

```json
{"require": {"descriptor": "supercycle.bars", "in": [12, 36]}}
{"require": {"descriptor": "lane.kick.pattern", "is": {"archetype": "four-on-floor"}}}
{"prefer":  {"descriptor": "composite.syncopation.lhl", "target": 0.65, "weight": 1.0}}
{"prefer":  {"descriptor": "composite.density", "target": 0.4, "weight": 0.5, "tolerance": 0.1}}
```

- Comparison forms: `in: [lo, hi]`, `min:`, `max:`, `eq:`, `is:` (pattern/
  archetype identity, exact or rotation-equivalent), `target:` (+ optional
  `tolerance`, soft only).
- Descriptor paths: `lane.<name>.<metric>`, `pair.<a>.<b>.<metric>`,
  `composite.<metric>`, `supercycle.<metric>`, `evolution.<metric>` — the
  five tiers of `docs/descriptor-spec.md`. Only descriptors flagged
  `targetable: yes` there are legal in `prefer` clauses.
- Weights are relative within one spec; M-C normalizes.

Locks are hard constraints with dedicated sugar:
`{"lock": {"lane": "kick"}}` ≙ require lane state equal to base patch.

### 3.4 Temporal stratum — defunctionalized processes

Time-directed change as a closed enum of record types over bar spans (M-G's
process objects, referenced — not redefined — here):

```json
{"process": {"type": "construction", "lane": "toms", "span": [0, 8]}}
{"process": {"type": "phase-shift-lock", "lane": "cym_b", "span": [8, 24], "targetOffsetSteps": 1}}
{"process": {"type": "break", "lanes": ["toms", "cymbals"], "every": {"bars": 8}, "length": {"bars": 1}}}
{"section": {"name": "build", "span": [0, 16], "prefer": [{"descriptor": "composite.density", "target": 0.7}]}}
```

- Process types v1: `construction`, `reduction`, `phase-shift-lock`,
  `konnakol-transform`, `korvai-cadence`, `break` (compiles to phrase
  gating / FillLikelihood until M-G lands, to a process object after).
- `section` scopes goal-stratum clauses to a span — the bridge between the
  goal and temporal strata, and the spec-side view of the M-G arrangement
  timeline.
- Spans are in bars relative to spec start; the compiler resolves to PPQ.
  All processes are PPQ-derived and deterministic, per engine invariants.

**Compilation note (pre-M-G):** until the arrangement timeline exists,
`section`/`process` statements compile to the nearest existing mechanism
(envelopes, phrase gating, scene chains) with documented fidelity limits.
The spec is stable; the compilation target improves.

---

## 4. Top-level spec shape

```json
{
  "schemaVersion": 1,
  "ensemble": {
    "lanes": {
      "kick":  {"stratum": "low",  "articulation": "default"},
      "snare": {"stratum": "mid"},
      "toms":  {"stratum": "mid"},
      "cym_a": {"stratum": "high"},
      "cym_b": {"stratum": "high"}
    },
    "template": null
  },
  "style": {"space": "house-techno"},
  "statements": [ /* role | relation | require | prefer | lock | process | section */ ],
  "hints": {"batchSeed": 41, "candidates": 8}
}
```

- `ensemble` declares symbolic lane names (+ optional stratum/articulation and
  an optional M-B ensemble template to start from).
- `style.space` narrows archetype retrieval and mutation norms (M-B `norms`) —
  a generator hint with soft influence on S via style-typicality, never a
  hard filter unless a `require` says so.
- `statements` is the flat list; order is not semantically significant
  (H is a conjunction; S is a sum). Conflicting hard constraints make the
  spec unsatisfiable — M-C reports *which* clauses conflict (min-unsat
  reporting is a v1 deliverable; it is the chat loop's repair signal).

---

## 5. Worked example

Prose: *"an EDM bass and snare combination with toms that support a
call-and-response interaction with other instruments over a 4-bar period, with
other layers that combine to create a shimmering cymbals effect with phase
effects over a 12–36 bar period, and breaks built into the tom and cymbal
cycles to create interest and variety."*

Kernel transcription (post-desugar):

```json
{
  "schemaVersion": 1,
  "ensemble": {
    "lanes": {
      "kick": {"stratum": "low"}, "snare": {"stratum": "mid"},
      "toms": {"stratum": "mid"},
      "cym_a": {"stratum": "high"}, "cym_b": {"stratum": "high"}
    }
  },
  "style": {"space": "house-techno"},
  "statements": [
    {"role": {"lane": "kick", "is": "anchor"}},
    {"require": {"descriptor": "lane.kick.pattern", "is": {"euclid": [4, 16]}}},
    {"role": {"lane": "snare", "is": "backbeat"}},
    {"require": {"descriptor": "lane.snare.backbeatStrength", "min": 0.8}},

    {"role": {"lane": "toms", "is": "response"}},
    {"relation": {"type": "answer", "from": "toms", "to": "kick"}},
    {"require": {"descriptor": "lane.toms.phraseBars", "eq": 4}},

    {"relation": {"type": "interlock", "between": ["cym_a", "cym_b"]}},
    {"prefer": {"descriptor": "pair.cym_a.cym_b.complementarity", "target": 0.9, "weight": 1.0}},
    {"prefer": {"descriptor": "lane.cym_a.density", "target": 0.7, "weight": 0.5}},
    {"relation": {"type": "phase-against", "from": "cym_b", "to": "cym_a"}},
    {"require": {"descriptor": "supercycle.bars", "in": [12, 36]}},

    {"process": {"type": "break", "lanes": ["toms"], "every": {"bars": 8}, "length": {"bars": 1}}},
    {"process": {"type": "break", "lanes": ["cym_a", "cym_b"], "every": {"bars": 12}, "length": {"bars": 2}}},

    {"prefer": {"descriptor": "evolution.novelty", "target": 0.5, "weight": 0.7}}
  ],
  "hints": {"batchSeed": 41, "candidates": 8}
}
```

The LLM would more likely emit the sugared form —
`{"idiom": {"shimmer": {"lanes": ["cym_a", "cym_b"]}}}` expanding to the
interlock + complementarity + density + phase-against block above — but the
kernel is what M-C consumes and what this doc specifies.

---

## 6. Sugar layer (v1 candidates)

| Idiom | Expansion sketch |
| --- | --- |
| `shimmer(lanes)` | interlock + high complementarity target + high-stratum density floor + phase-against + micro-timing spread |
| `four-on-floor(lane)` | anchor role + `euclid(4,16)` identity + downbeat-weight floor |
| `backbeat(lane)` | backbeat role + backbeatStrength floor + protect constraint |
| `call-response(a, b, bars)` | response role + `answer` relation + phrase length + anti-coincidence preference |
| `groove-template(name)` | ensemble block + per-lane archetype identities from an M-B ensemble template |

Sugar definitions are data (JSON expansion rules with parameter slots), not
code, so the library can grow without schema changes; each carries a
`references` field like archetypes (the education hook).

---

## 7. Method (how this design gets validated before freezing)

1. **Corpus.** ≥ 50 utterances: user prompts (real and constructed), the
   research paper's tradition descriptions, all 43 preset descriptions,
   refinement turns ("sparser", "delay the break two bars", "swap toms and
   snare roles"). Hand-transcribe each; log every awkward or impossible
   transcription as a candidate construct, every ambiguous pair as a missing
   distinction. The corpus becomes the M-H eval benchmark seed.
2. **Denotation check.** Every construct must denote into (H, S, G). Anything
   that wants to sequence engine operations is rejected or reformulated.
3. **Kernel/sugar split.** Kernel admission test: needed by ≥ 3 corpus
   utterances *and* not expressible as sugar over existing kernel.
4. **Round-trip audit.** Decompile all 43 factory presets via analysis +
   recognition into specs; re-run M-C against each spec and confirm the
   original preset satisfies it. Gaps → language or engine defects (§2.7).
5. **Unsatisfiability behavior.** Author deliberately conflicting specs;
   confirm min-unsat clause reporting is actionable.
6. **Freeze** kernel as `groove-spec.schema.json` v1 + goldens.

---

## 8. Concept → parameter concordance (first cut)

Building the ontology exposes where the current engine/UI distributes one
concept across several parameters (or one parameter serves several concepts).
This table is simultaneously the compiler's mapping spec and the M-D UI
reorganization backlog. ⚠ marks a mismatch to resolve.

| Spec concept | Engine parameter(s) today | UI surface today | Notes |
| --- | --- | --- | --- |
| Pattern identity | `euclidean(k,n)`, `fixedPattern`, `cellSizes`, `kotekanSourceLane` | Lane edit, cell editor, timeline editor | ⚠ Four entry modes presented as unrelated features; spec unifies as one pattern expression. `kotekanSourceLane` is a hard-coded relation living among pattern params — migrates to structure stratum (M-E). |
| Displacement | `rotation` (steps), envelope `phaseOffset` (fraction), `phraseOffset` (bars), `driftRate` (rate) | Lane edit, envelope view, phrase edit, lane edit | ⚠ One concept, four params, three units, four panels. Spec: static displacement (pattern `rotate`) vs evolving displacement (`phase-shift-lock` / drift process). UI should co-locate. |
| Density | `density` macro, per-step `probability`, Density envelope target, constraint min/max | Macro row, lane edit, envelope view, (constraints not surfaced) | ⚠ Four altitudes of one concept. Spec: density is a *descriptor* with soft target + hard bounds. Macro row becomes the target editor (M-D); probability remains mechanism, not user concept. |
| Syncopation | `syncopation` macro → `syncopationOffset` | Macro row | ⚠ Control exists, measurement doesn't (until M-A). Same word, opposite direction — the open-loop gap. Spec side is measurement-only. |
| Swing / feel | `swingAmount`, `humanizeMs`, `timingOffsetMs`, `microTimingMs[]`, TimingLooseness envelope | Lane edit, micro-timing editor, envelope view | Mostly coherent, but spec needs one `feel` descriptor cluster; `swing` macro overlaps `swingAmount` per-lane. ⚠ macro-vs-param precedence undocumented. |
| Accent / dynamics | `baseVelocity`, `AccentMask`, `emphasisProb`, `ghostFloor`, `velocitySpread`, Velocity/AccentBias envelopes | Velocity view, lane edit, envelope view | ⚠ Binary accent mask vs continuous cycle-weight profile (M-G) — spec assumes weight profiles; mask becomes a special case. |
| Phrase / pauses | `phraseLength`, `phraseGap`, `phraseOffset` | Phrase edit | Clean. Spec `break` process compiles here pre-M-G. |
| Evolution | `mutationRate`, `driftRate`, FillLikelihood envelope, scene chain | Lane edit, envelope view, chain popover | ⚠ Three time-structure mechanisms (envelopes, chains, future timeline). Spec temporal stratum treats timeline as canonical; envelopes/chains become compilation targets. |
| Interlock / relations | `kotekanSourceLane` only | Lane edit dropdown | ⚠ One special case of an eight-relation vocabulary; generalized by M-E. |
| Structure anchor | timeline mode + macro immunity, anchor steps, backbeat protect | Timeline editor, (constraints not surfaced) | ⚠ "Anchor" exists as three unrelated features (fixed lane, protected steps, protected backbeat); spec unifies under role `anchor` + protect constraints. |
| Identity / seed | `seed`, scene A/B | Header, scene bar | Clean; spec `hints.batchSeed` is separate from patch seed by design. |

Resolution of ⚠ rows is *not* a prerequisite for M-H (the compiler can map
around them) — but each row should get an issue, and M-D should adopt the
spec-side concept names as its UI vocabulary.

---

## 9. Prior art and licensing

Poly is GPLv3, so GPL-compatible copyleft prior art is reusable as code, not
just as reference. Survey summary (verified 2026-07-23; licenses checked at
source — re-verify before vendoring anything):

### 9.1 Pattern algebra / Toussaint formalizations

*Findings to be inserted from research pass — see §9.4 verdicts.*

### 9.2 Envelope / modulation / timeline formats

*Findings to be inserted from research pass — see §9.4 verdicts.*

### 9.3 Constraint and analysis systems

*Findings to be inserted from research pass — see §9.4 verdicts.*

### 9.4 Verdicts

*Findings to be inserted from research pass.*

---

## 10. Open questions

1. **Probability in the pattern algebra?** v1 keeps patterns binary; per-step
   probability stays a mechanism below the spec. Revisit if corpus utterances
   demand "sometimes" semantics ("occasionally drop the third hit").
2. **Articulation in specs** — blocked on M-F; the `ensemble.lanes.*.
   articulation` slot is reserved but unspecified.
3. **Negative relations** ("nothing on the anchor's downbeats except kick") —
   expressible as `avoid-overlap` + protect today; decide whether a dedicated
   exclusion construct earns kernel status via the corpus.
4. **Tempo/meter** — the spec deliberately does not set host tempo or
   signature (host-owned); confirm no corpus utterance needs it beyond
   cycle-length selection.
