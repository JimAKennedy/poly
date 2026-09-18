---
class: gated
---

# Guide parity — vision

Status: current (2026-09-18)

Input document for `/jk:assess`. It proposes a decomposition; the ledger that
comes out of it is the plan of record, and the boundaries below are arguments,
not decisions.

## The observation

Poly's guide is ahead of Poly's engine, and the guide says so out loud.

Six of the seven feature issues gathered here are the same shape: **the guide
teaches a technique, and the engine either cannot express it or makes the user
perform it by hand.** This is not inference from the issues — it is what the
pages themselves say, and what the issues quote them saying:

| Issue | What the guide teaches | What the engine does |
|---|---|---|
| [#149](https://github.com/JimAKennedy/poly/issues/149) | `12-jazz.mdx`: "Jazz swing varies continuously with tempo, intensity, and style" | one tempo-invariant `swingAmount`, capped at exact triplet |
| [#152](https://github.com/JimAKennedy/poly/issues/152) | `03-afro-cuban.mdx`: clave is "a rhythmic matrix"; parts that ignore it "sound wrong" | nothing in the engine knows the timeline lane exists |
| [#154](https://github.com/JimAKennedy/poly/issues/154) | `06-indian-classical.mdx` § "Tihai: Landing on Sam" | the user does the arithmetic by hand |
| [#155](https://github.com/JimAKennedy/poly/issues/155) | `15-compositional-grammar.mdx`: "identical Length and Gap values but different Offsets" | a manual recipe with no structural relationship between lanes |
| [#158](https://github.com/JimAKennedy/poly/issues/158) | funk ghosting is grammatical, clustering around the backbeat | a flat per-step mutation roll, position-blind |
| [#151](https://github.com/JimAKennedy/poly/issues/151) | every chapter that recommends Humanize | white-noise jitter, which is what makes humanised MIDI sound fake |

**This programme's precedent is M003.** That milestone found the same shape in
`theory-brazilian` Rule 6 — a rule that ended "until subdivision profiles ship,
use light swing plus small per-lane offsets as an admitted approximation" — and
closed it by building the capability and deleting the admission. Three such
admissions came out of the guide in that one milestone. The work below is the
same move, six more times.

**Vision.** Where the guide teaches a technique, Poly performs it. The
workarounds the pages currently prescribe — do the tihai arithmetic yourself,
tune the offsets until the gates interleave, spend a lane on ghost notes — stop
being instructions and become things the engine does, and each page stops
prescribing its workaround.

Two of the ten items sit outside that vision and are called out as such below.

## What success looks like

A reader of the guide can do what the chapter describes by setting the thing the
chapter names, rather than by following a recipe that simulates it. Concretely,
for each item: the page's workaround prose is **gone**, not merely supplemented,
and a test fails if it comes back — the pattern
`site/tests/scope-framing.test.mjs` already enforces for M001–M003's corrections.

## Proposed milestones

### GP1 — Feel derives from tempo and from time

**Issues:** [#149](https://github.com/JimAKennedy/poly/issues/149) (tempo-adaptive, wider swing),
[#151](https://github.com/JimAKennedy/poly/issues/151) (correlated 1/f humanize)

Both change **how a hit is displaced in time**, both land in `applyTimingShifts`,
both cite measured-performance literature the guide already cites, and both need
a `kStateVersion` bump with defaults that leave existing presets byte-identical.

They belong together because they are the same edit to the same function with
the same back-compatibility obligation, and because M003 has just demonstrated
the whole pattern in that exact code — guarded field, normalisation decision,
golden proving 45 presets unmoved, `maxTimingShift` lookahead updated. #149's
own text names that last point: *"maxTimingShift lookahead must track any range
widening."*

**Why it is first.** It is the lowest-risk of the three engine milestones and
the one whose pattern is freshest. It also re-walks the code M003 changed, which
is where a mistake in M003 would surface.

### GP2 — Stochastic choices know where they are

**Issues:** [#158](https://github.com/JimAKennedy/poly/issues/158) (metric-position ghosts),
[#152](https://github.com/JimAKennedy/poly/issues/152) (timeline attraction/avoidance),
[#154](https://github.com/JimAKennedy/poly/issues/154) (phrase-position fills, tihai)

All three bias the **same seeded rolls** — mutation-add, ghost, drop, fill — by a
per-step weight derived from position, and all three describe the weight as
precomputable per lane in `prepareLaneContext`. #158 weights by metric position,
#152 by distance from a reference lane's onsets, #154 by proximity to a phrase
boundary. One weighting mechanism, three sources of weight.

Building them separately would mean three ways to bias one roll, which is the
outcome M003 explicitly rejected when it gave subdivision profiles precedence
over `cellSizes` rather than letting two non-isochronies combine silently.

**The tihai helper is the one piece that is not a weight.** #154 proposes it
either engine-side or as a UI calculator, and that is a genuine open question
for `/jk:assess` rather than something this document should settle.

### GP3 — Lanes can refer to each other

**Issue:** [#155](https://github.com/JimAKennedy/poly/issues/155) (`responseSourceLane`)

A lane names another lane and derives its phrase gate from that lane's gate,
mirroring `kotekanSourceLane` — including, as the issue says, the
mutual-reference guard that already exists beside it. Kotekan couples
*patterns*; this couples *phrasing*.

**It may belong inside GP2.** #152 also designates a reference lane, so the
"one lane names another, with a cycle guard" mechanism is shared between them,
and building it twice would be the same mistake GP2 exists to avoid. The counter
is that #152 reads a lane's *onsets* while #155 reads its *gate state*, which
may make the shared part thin. Worth deciding at assessment with the code open,
not here.

### GP4 — A lane can carry pitch

**Issue:** [#245](https://github.com/JimAKennedy/poly/issues/245) (per-lane note sequence)

The largest change and the only one that is not closing a guide gap: it opens
territory the guide does not yet describe. A lane emits one fixed pitch —
`ev.pitch = cfg.midiNote`, one line in `engine.cpp` — and an optional sequence of
up to N pitches with per-note durations would make every existing lane feature
apply to melodic material.

It is alone because it is genuinely separate work: a new array field, a state
version, a `presets.json` schema bump, and a WebUI editing question that
[#305](https://github.com/JimAKennedy/poly/issues/305) has already shown to be a
milestone of its own when array-valued lane state needs an editor.

**A prerequisite the issue names and this document should not gloss:** `phrase`
is already taken by `phraseLength`/`phraseGap`/`phraseOffset`, so the naming has
to be settled before the field exists, not after.

**The guide question GP4 raises.** If pitched lanes ship, the guide describes a
drum-pattern generator that also does something else. Whether the guide grows to
cover that, or the feature ships ahead of the guide with that stated, is a
decision for assessment. This programme's whole premise is that the two should
not silently diverge.

## Outside the vision, included because they were asked for

These three are not guide-parity work. They are named here so the assessment can
place them deliberately rather than have them arrive as an afterthought — and
the engine-capability ledger's own reasoning applies: *"they interlock with
nothing here and most are single fixes; a ledger's traceability costs more than
it returns on work that does not interlock."*

### GP5 — Diagrams are rendered, not drawn

**Issue:** [#111](https://github.com/JimAKennedy/poly/issues/111)

**Seven files** carry hand-drawn ASCII architecture diagrams, 108 marked-up
lines between them. An earlier draft of this document said "ten files",
conflating the four *diagrams* in `appendix-plugin-architecture.mdx` with a file
count; the measured inventory is below. `ARCHITECTURE.md` is at the repo root
rather than under `docs/`, which the issue's own list also places wrongly.

| File | ASCII lines |
|---|---|
| `docs/euclidean-rhythm-guide.md` | 44 |
| `site/src/content/docs/appendix-plugin-architecture.mdx` | 28 |
| `docs/testing-strategy.md` | 21 |
| `docs/engine-spec.md` | 9 |
| `ARCHITECTURE.md` | 8 |
| `docs/ui-guide.md` | 4 |
| `docs/webui-migration.md` | 2 |

They look markedly worse than the rest of a typography-first site, and they are
hand-maintained, which is the drift class this repo has spent several milestones
killing.

This one **does** have a coherent outcome and enough surface to be a milestone:
a build-time Mermaid pipeline, ten files converted, and the existing
`generated-freshness` discipline extended to cover them. Note that artifacts
rendered at build time interact with `wasm-freshness`-style reproducibility
concerns, and [#282](https://github.com/JimAKennedy/poly/issues/282) is the
standing example of what a non-reproducible build artifact does to commit
hygiene.

### GP6 — No signal stays red or flaky

**Issues:** [#142](https://github.com/JimAKennedy/poly/issues/142) (TSAN-PLUGIN nightly),
[#89](https://github.com/JimAKennedy/poly/issues/89) (flaky `reich-play.spec.ts`)

Both are cases where a signal is not telling the truth, which is the defect class
engine-capability M005–M007 spent three milestones on — a check that cannot run,
a check that passes without running, a check nobody runs. A check that is
chronically red, or red one run in five, is the same disease presenting
differently: it stops being read.

**Corrected at assessment, 2026-09-18.** An earlier draft called #142 "a
sanitizer finding from 2026-07-26 [that] nobody has triaged", implying something
stale. It is not stale — it is **auto-refiled**. The issue carries 22 comments,
one appended by the nightly per failure, the most recent on **2026-09-16**, and
that latest occurrence names **ASAN-PLUGIN** where the title says TSAN-PLUGIN.
The issue has accumulated more than one sanitizer.

Measured over the last 20 sanitizer nightlies: **1 failure, 19 successes.** So
it is intermittent at roughly 5%, spanning at least two sanitizers, in plugin
code. An intermittent ASAN/TSAN finding is a plausible real memory or threading
bug, and this repo's entire real-time-safety discipline rests on the plugin
being clean — it is not housekeeping.

#89 is better understood: a 2.5 s threshold against a 3 s Playwright wait,
verified still present at `site/tests-e2e/reich-play.spec.ts:43`, reproducing
locally and green in CI. It reads as a single fix and is sized as one.

## Sequencing

GP1 → GP2 → GP3 is a real ordering only in that each is a smaller edit than the
next is large; none blocks another. GP4 depends on nothing. GP5 and GP6 depend
on nothing and block nothing.

The one genuine dependency to look for during assessment is between **GP2 and
GP3**, and it is a design dependency rather than a scheduling one: if the
reference-lane mechanism is shared, whichever ships first builds it.

## What this programme does not claim

- **That the guide should grow to cover GP4.** Stated as an open question above.
- **That these are all the open issues.** [#266](https://github.com/JimAKennedy/poly/issues/266),
  [#267](https://github.com/JimAKennedy/poly/issues/267),
  [#274](https://github.com/JimAKennedy/poly/issues/274),
  [#282](https://github.com/JimAKennedy/poly/issues/282),
  [#305](https://github.com/JimAKennedy/poly/issues/305) and
  [#100](https://github.com/JimAKennedy/poly/issues/100) are open and deliberately
  absent. #305 is the WebUI editing milestone M003 deferred; #100 records that
  `appendix-presets.mdx` documents 14 of what was then 43 factory presets and is
  now 45, so that gap has widened since it was filed.
- **That every item here is the same size.** GP4 and GP5 are milestones. #89
  is a line. Saying so now is cheaper than discovering it during planning.

## Decisions taken at assessment

Recorded here rather than left in a conversation, so a re-run of `/jk:assess`
consumes them instead of re-asking.

- **Q:** Given #142 is intermittent across two sanitizers rather than chronically
  red, how should it be placed? — **A (2026-09-18):** Its own milestone,
  triage-first.
- **Decision:** GP6 covers #142 alone, and its first slice is triage —
  reproduce, classify, and only then size the fix. — **Why:** Pairing a possible
  real race in plugin code with a flaky test threshold would understate it, and
  sizing the fix before triage would be inventing a number.
- **Consequence:** #89 needs a home of its own. It is one line, which is not a
  milestone; where it lands is an open question for the decomposition.

- **Q:** How far should "the page stops prescribing the workaround" go? —
  **A (2026-09-18):** Engine and the page's workaround prose, in the same
  milestone.
- **Decision:** Each engine milestone deletes the workaround it obsoletes and
  locks the deletion with a `scope-framing` claim, exactly as M003 did for
  `theory-brazilian` Rule 6. — **Why:** Shipping capability first and sweeping
  the guide later leaves a window in which the engine can do things the guide
  still tells readers to fake — which is the drift this programme exists to
  close, running in the opposite direction.

- **Open, carried into the decomposition:** whether GP3 (#155) folds into GP2.
  Both designate a reference lane, but #152 reads that lane's *onsets* — which
  `kotekanSourceLane` already does at `engine.cpp:99-110` — while #155 reads its
  *phrase gate state*, computed at `:470-474` from `phraseCyclePpq` and
  `phraseOffPpq`. The shared part may be only the cycle guard.
