---
class: gated
---

<!-- counts-ok: design doc — cites counts as point-in-time baselines; authoritative counts live in site/src/generated/counts.json -->

# SLM-First Architecture for the Groove-Spec Compiler

**Status:** Design document for ROADMAP.md milestone M-H, phases v1.5/v2.
Companion to `docs/groove-spec-design.md` (the language this architecture
compiles into). Adapted from a generic SLM/LLM engineering playbook
(workload-partitioning, escalation-over-overprovisioning, continuous
verification), tailored to where Poly's situation departs from the generic
case — which it does in Poly's favor at almost every point.

**Summary:** a locally-run small language model (SLM) compiles prose to groove
specs by default, with verified-failure fall-through to a hosted frontier LLM.
The LLM serves three roles: v1 product, permanent teacher (escalation traces
train the SLM), and permanent home for open-ended teaching conversation.

---

## 1. Why Poly is an unusually favorable SLM case

Generic playbooks hedge on SLM viability because most tasks lack bounded
scope, stable schemas, and cheap validation. The M-H compiler has all three:

1. **Owned output contract.** The target is `groove-spec.schema.json` — a
   schema we publish and version. The domain vocabulary is *closed*:
   descriptor names, archetype IDs, relation types, process types, sugar
   idioms are finite sets defined by our own artifacts.
2. **Objective, automatic, cheap validation.** Schema check → spec compile →
   satisfiability → headless render + descriptor assertions: deterministic,
   milliseconds, no LLM-as-judge anywhere. The playbook's largest cost center
   (building evaluators/guardrails) is a byproduct of M-A/M-C.
3. **Repair-loop tolerance.** The chat loop is propose → validate → repair
   (min-unsat clause reporting is the repair signal). A model that misses on
   first shot but converges in the loop matches a stronger single-shot model.
   Loop tolerance is precisely what makes small models viable.
4. **Local is a product feature.** Offline studios, no API key friction, no
   cloud dependency in a plugin workflow, privacy by default. For this
   audience local-first is a headline capability, not a cost optimization.
5. **Grammar-constrained decoding** (available only to local models): the
   JSON schema compiles to a decoding grammar (e.g. llama.cpp GBNF) that
   constrains generation token-by-token — the SLM *cannot* emit
   schema-invalid output. This eliminates the classic SLM failure mode
   (brittle formatting) by construction, a stronger guarantee than hosted
   structured outputs.

---

## 2. Workload partition — by function, not difficulty

| Function | Default model | Rationale |
| --- | --- | --- |
| Prose → spec compilation | **SLM** | Bounded, closed vocabulary, schema-constrained, loop-tolerant |
| Incremental spec edits ("sparser", "delay the break two bars") | **SLM** | Even more bounded: current spec + delta |
| Open-ended teaching ("why is this the son clave?", "explain aksak") | **LLM** | Open-ended synthesis; the SLM is never asked to do this. Offline degraded mode: templated answers assembled from archetype/descriptor `references` metadata |
| Novel / ambiguous / repeatedly-rejected requests | **LLM via escalation** | The high-variance tail |

A trivial intent gate (compilation vs. conversation) sits in front; it is a
keyword/heuristic classifier, not a model.

---

## 3. Routing: try→validate→escalate, not a request router

Generic architectures put a difficulty classifier in front because their
validation is expensive or subjective. Ours is cheap and objective, so
routing happens **after a verified attempt**, which cannot misroute:

```
utterance ──► intent gate ──(teaching)──► LLM (or templated offline answer)
                 │(compile)
                 ▼
            SLM attempt (grammar-constrained) ──► validate:
                 schema ✓ (by construction)
                 compile → satisfiable?
                 propose → assertions met?
                 │pass                │fail → 1 repair attempt (min-unsat fed back)
                 ▼                    │pass → serve   │fail
              serve                                   ▼
                                              escalate to LLM
                                              (same spec context)
```

**Escalation triggers (all measurable, all logged with reason codes):**

- min-unsat after one SLM repair attempt;
- user rejects N consecutive proposal batches, or explicit "try harder"
  affordance;
- out-of-lexicon vocabulary in the utterance (cheap check against the closed
  descriptor/archetype/sugar lexicon);
- teaching-intent detection at the gate;
- SLM unavailable (model not downloaded) → LLM or retrieval fallback.

**Never-escalate mode** is a first-class user setting: fully local operation,
no network, escalation triggers become user-visible "couldn't fully satisfy
this — simplify or enable cloud assist" messages. This is the privacy
posture, not a failure state.

A failed SLM attempt costs well under a second locally; no upfront
difficulty prediction can beat that on cost or reliability.

---

## 4. The data flywheel

Escalation is not just capacity management — **every escalated request the
LLM resolves successfully becomes an SLM training example** (utterance →
validated spec, with validator verdicts and user accept/reject attached).
The fall-through funds its own decline: escalation rate is the KPI that
should fall across retraining cycles.

Training-data sources, in order of volume:

1. **Synthetic backtranslation** (bulk): sample valid specs (and factory
   presets / archetype compositions) → render → compute descriptors → have a
   frontier model write diverse prose descriptions → (prose → spec) pairs,
   guaranteed-valid by construction. Tens of thousands, near-free.
2. **v1 production traces**: real utterances with validated specs and user
   verdicts — the distribution that matters.
3. **Escalation traces** (highest value): exactly the cases the current SLM
   fails.

**Contamination discipline** (the one playbook warning that bites us): the
golden corpus (`groove-spec-design.md` §7, the M-H eval benchmark) and its
paraphrases must never enter training data. Enforced, not hoped: hold-out by
construction — corpus utterances get IDs, the synthetic generator is
prompted away from them, and a near-duplicate filter (embedding similarity
threshold) screens every training batch against the corpus. Train/val/test
splits are disjoint at the *spec* level, not just the utterance level (one
spec, many paraphrases — leakage travels through the spec).

Curation loop per retraining cycle: dedupe near-duplicates, drop low-signal
pairs, explicitly label edge cases, fold in the cycle's escalation traces.

---

## 5. Model and runtime selection

- **Base model:** 3–8B open-weights, instruction-tuned base, LoRA SFT.
  **Weights license is a selection criterion** for a GPLv3-distributed
  product: prefer Apache-2.0 / MIT-licensed weights (Qwen, Phi class) over
  community-licensed ones (Llama-style terms). Record the choice and license
  text in `THIRD_PARTY.md`. Re-survey candidates at v1.5 start — the small-
  model frontier moves fast; the architecture is model-agnostic by design.
- **Runtime:** llama.cpp (MIT) or equivalent, embedded in the companion
  service — never in the plugin process (same placement rule as the LLM
  path; the RT thread is untouched). GBNF grammar generated from
  `groove-spec.schema.json` at build time (drift-checked with the schema).
- **Artifact:** 4-bit quantized (~2 GB for 3B class), optional download,
  checksummed and signed; the plugin works without it (LLM or retrieval
  fallback).
- **Determinism note:** SLM inference is pinned (seeded sampling or greedy +
  grammar) so a logged trace is reproducible; but as with the LLM, the model
  only ever chooses the *spec* — accepted grooves replay deterministically
  with no model present at all.

---

## 6. Evaluation and gates

Offline evaluation is the release gate for every change to the (model,
prompt, grammar/schema) triple — the three are versioned and rolled back
**as a unit**.

**Golden benchmark:** the corpus (≥ 50 utterances, growing with production
failures) with per-utterance assertions on resulting descriptors/structure —
already specified as the M-H eval harness. Fixture-based in CI; live model
runs scheduled.

**Gates for promoting an SLM build (shadow → serving):**

- First-shot compile success (schema-valid ∧ satisfiable) ≥ target on the
  full benchmark, with **no critical slice below bar** — slices by stratum
  (pattern / structure / goal / temporal clauses), by kernel-vs-sugar, by
  utterance length, and by fresh-vs-corpus;
- Post-repair success within a small delta of the v1 LLM baseline on the
  same benchmark (the LLM's measured quality *is* the bar);
- Assertion pass rate on served proposals ≥ v1 baseline;
- No regression on the previous build's worst slices;
- Latency budget met on reference hardware (mid-range laptop, CPU-only).

**Shadow mode (v1.5):** the SLM runs on every real request — compile,
validate, log, never serve — while the LLM serves. Shadow metrics accumulate
the promotion evidence on real traffic before the flip; this replaces the
playbook's "benchmark both at prototype time" with a benchmark on *observed*
traffic.

**Online metrics (v2):** escalation rate (with reason-code breakdown),
first-shot compile success, repair iterations per request, proposal
acceptance rate, teaching-turn share, latency percentiles, and — per the
playbook's best line — **cost per accepted groove, not cost per call**.
Slice-based monitoring so rare-but-severe failures aren't hidden by healthy
averages.

---

## 7. Observability

Every request logs a structured trace: utterance (hash-linkable, locally
stored), intent-gate verdict, model+prompt+schema versions, emitted spec,
validator outcomes, repair iterations, escalation reason code, proposal IDs
served, user verdict (accept/reject/audition-only), end-to-end latency.
Traces are local-first (they are also the training corpus); any telemetry
upload is opt-in and stripped to aggregates. This is the same log the
deferred learned-ranking work consumes — one pipeline, three consumers
(debugging, retraining, ranking).

---

## 8. Governance residue

Most generic governance apparatus does not apply: specs contain no PII, the
tool surface has no dangerous side effects (casting is user-confirmed and
undoable), and there is no meaningful misuse surface. What remains:

- **API key handling** — user-supplied, stored outside plugin state, never
  serialized into patches (already an M-H invariant);
- **SLM artifact integrity** — checksummed, signed download; no code
  execution from the artifact (weights only);
- **Prompt-injection posture** — spec text and archetype metadata rendered
  in the UI are data, never instructions; the compiler service's tools have
  no side effects beyond proposing patches the user must accept;
- **Documentation of known weaknesses** — the model card for each SLM build
  records worst slices and known failure vocabularies (kept honest by the
  gate reports).

Ownership collapses to the maintainer, with the checklist above standing in
for the playbook's multi-team operating model.

---

## 9. Phasing and exit criteria

| Phase | Ships | Exit criterion |
| --- | --- | --- |
| **v1 — LLM-only** | Compiler service, validators, eval harness, trace logging, chat panel | Benchmark green; traces accumulating |
| **v1.5 — SLM shadow** | Training pipeline (synthetic + traces, contamination-guarded), shadow deployment, model card | All §6 promotion gates pass on shadowed real traffic |
| **v2 — SLM-first** | Escalation routing, never-escalate mode, escalation telemetry, retraining cadence | Escalation rate stable/falling across two retraining cycles; no online-metric regression vs. v1 |

Rollback at every phase: the (model, prompt, schema) triple reverts as a
unit; v2 → v1 is a routing flag, not a rebuild.

---

## 10. Open questions

1. **SLM for spec *edits* first?** Incremental edits are the most bounded
   sub-task; a v1.5a that serves only edits locally (full compiles still
   LLM) could flip part of the traffic earlier at lower risk.
2. **Teaching-turn offline quality** — how far do templated answers from
   archetype/descriptor metadata get before they feel canned? May motivate a
   second, tiny model or better templates; decide from v2 telemetry.
3. **On-device tiering** — one 3B model for all hardware, or a 7–8B option
   for capable machines? Decide from shadow-mode latency slices.
4. **Retraining cadence** — fixed (per release) vs. triggered (escalation
   volume threshold); start fixed, revisit with data.
