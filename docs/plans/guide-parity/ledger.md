---
class: gated
---

# Guide parity — delivery ledger

Status: current (2026-09-18)

Assessed from `docs/plans/guide-parity/vision.md` on 2026-09-18. That document
is research; this file is the plan of record.

**Source:** —  <!-- ledger-ok: no spec system in this repo; the vision document is the input and `openspec/` is absent -->

## The programme

Poly's guide is ahead of Poly's engine, and the guide says so out loud. Six of
the seven feature issues assessed here are one shape: a page teaches a
technique, and the engine either cannot express it or makes the reader perform
it by hand. The pages' own words are the evidence, not an inference drawn from
the issues.

The precedent is engine-capability M003, which found the same shape in
`theory-brazilian` Rule 6 — a rule ending "until subdivision profiles ship, use
light swing plus small per-lane offsets as an admitted approximation" — built
the capability and deleted the admission. Three such admissions left the guide
in that one milestone.

**Every engine milestone here carries its own prose.** A milestone that ships
capability and leaves the page prescribing the workaround creates the drift this
programme exists to close, running the other way. The deletion is locked by a
`scope-framing` claim, as M001–M003's corrections are.

**Back-compatibility is a definition-of-done item, not an assumption.** Every
engine slice must show the 45 factory presets rendering byte-identically with
its feature off, proved by a golden rather than by inspection. M003/S01 set that
precedent and it is what made that milestone safe to merge without re-auditing
45 presets by hand.

## Reconciliation

Ten items assessed against the tree at `e3a2205`. None already satisfied, none
partly satisfied, all ten outstanding — each confirmed by absence in the code
rather than by reading the issue:

| Item | Confirmed outstanding by |
|---|---|
| #149 | `kSwingSyncopationDivisor = 3.0`, one fixed divisor, no tempo term |
| #151 | a single `deterministicRand(..., absStep, 3)` per step — white noise |
| #158 | `kMutationGhostThreshold` applied to a flat roll |
| #152 | `ConstraintConfig` is lane-local; no `timelineSourceLane` |
| #154 | fill roll on `absStep`; no phrase-position weighting |
| #155 | `kotekanSourceLane` exists, `responseSourceLane` does not |
| #245 | `ev.pitch = cfg.midiNote` — one assignment, no `noteSequence` |
| #111 | 7 files still ASCII, 108 marked-up lines, no mermaid dependency |
| #142 | open; 22 auto-filed occurrences; 1 failure in the last 20 nightlies |
| #89 | `toBeGreaterThan(2.5)` still present |

The vision document carried two wrong claims of its own — a file count for #111
and a stale framing for #142 — both corrected in it before this assessment, and
both recorded there rather than silently fixed.

## Milestone M001 — Feel derives from tempo and from time

**Vision:** Poly's swing and humanize behave as the measured literature the
guide already cites describes, and the pages stop prescribing a fixed amount.

**Branch:** milestone/M001-feel
**Status:** done
**Demo:** A jazz patch at 80 BPM swings wider than the same patch at 220, from
one setting; a humanized lane drifts rather than jitters, and the drift is
identical on every replay of the same seed.

**Why these two together.** Both change how a hit is displaced in time, both
land in `applyTimingShifts`, both need a `kStateVersion` bump with inert
defaults, and both must keep `maxTimingShift`'s lookahead correct — #149 names
that last point itself. M003/S01 walked exactly this ground, including the
lookahead bound, which is also why a mistake there would surface here first.

### Slice M001/S01 — Swing widens and tracks tempo

**Plan:** M001-S01-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] A lane can swing beyond the exact-triplet ceiling the fixed `/3` divisor imposes
- [x] With tempo-adaptive swing on, the effective ratio widens at slow tempi and narrows toward straight at fast ones, asserted at two tempi from one setting
- [x] With the feature off, all 45 factory presets render byte-identically, proved by a golden test
- [x] `maxTimingShift` covers the widened range, shown by a note near a block boundary still being emitted
- [x] `12-jazz` no longer tells the reader to pick a Swing value per tempo, and a `scope-framing` claim fails if that instruction returns
- [x] Both of this milestone's mode fields survive a save and reload, and a
      pre-bump state loads as the behaviour it played

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP01 | Swing is a fixed fraction of the step — `swingAmount * stepDurPpq / kSwingSyncopationDivisor` with the divisor at 3.0 — so the ratio is capped at exact triplet and is invariant with tempo. Measured jazz reaches ~3.5:1 at ballad tempi and narrows toward 1:1 near 300 BPM (Friberg & Sundström 2002), and `12-jazz` already teaches that swing "varies continuously with tempo" | `capability` | `engine/`, `12-jazz.mdx`, `site/tests/` | Engine tests assert a ratio beyond triplet is reachable and that one setting yields different ratios at two tempi; the golden asserts presets unmoved with the mode off; a site claim locks the prose deletion | `done` |

### Slice M001/S02 — Humanize drifts rather than jitters

**Plan:** M001-S02-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M001-S02.md
**Status:** done

**Definition of Done**

- [x] Successive humanize offsets on one lane are correlated rather than independent, asserted as a measurable property of the sequence rather than by eye
- [x] The offsets remain a pure function of absolute step index — a locate or loop reproduces them exactly
- [x] With the correlated mode off, all 45 factory presets render byte-identically, proved by a golden test
- [x] `renderRange()` gains no allocation, lock or blocking call
- [x] The pages recommending Humanize no longer describe it as jitter where they now mean drift, and a `scope-framing` claim locks the correction

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP02 | `humanizeMs` displaces each hit by an independent seeded value, which is white noise. Human timing fluctuation is long-range correlated (Hennig et al. 2011), and listeners distinguish the two — white-noise jitter is what makes humanized MIDI sound fake. Affects every chapter recommending Humanize, neo-soul at 0.4–0.5 most exposed | `capability` | `engine/`, `site/src/content/docs/`, `site/tests/` | Engine tests assert successive offsets correlate and that the sequence is reproduced exactly after a transport jump; the golden asserts presets unmoved with the mode off | `done` |

## Milestone M002 — Stochastic choices know where they are

**Vision:** Mutation, ghost, drop and fill decisions are weighted by where the
step sits — in the meter, against the timeline, and within the phrase — and the
chapters stop prescribing manual workarounds for them.

**Branch:** milestone/M002-position
**Status:** done
**Demo:** A batá or clave patch where added hits land with the timeline rather
than across it; a funk patch whose ghosts cluster before the backbeat without a
dedicated ghost lane; a tihai that lands on sam without the reader doing the
arithmetic.

**Why these four together.** All four bias the *same seeded rolls* — the
mutation, ghost, drop and fill decisions at `engine.cpp` — by a per-step weight
that is precomputable per lane. #158 weights by metric position, #152 by
distance from a reference lane's onsets, #154 by proximity to a phrase
boundary, and #155 couples one lane's phrase gate to another's. One weighting
mechanism, several sources of weight. Building them apart would mean several
ways to bias one roll, which is what M003 rejected when it gave subdivision
profiles explicit precedence over `cellSizes` rather than letting two mechanisms
combine silently.

**#155 is here rather than in a milestone of its own,** which the vision document
left open and said to decide with the code open. Decided: `kotekanSourceLane` at
`engine.cpp:99-103` already carries "one lane names another, guard against
mutual reference, read its state", and both #152 and #155 need exactly that.
They diverge in what they read — onsets versus gate state — so the shared part
is the reference and its guard, which is real but small. Small enough to build
once, not twice.

### Slice M002/S01 — A lane can weight by a reference lane's timeline

**Plan:** M002-S01-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S01.md
**Status:** done

**Definition of Done**

- [x] A lane can name a reference lane, and a mutual reference is refused rather than followed
- [x] Mutation-adds are biased toward or away from the reference lane's onsets by a signed per-lane strength, shown by the distribution of added steps changing with the sign
- [x] Drops are less likely on high-weight steps than on low-weight ones
- [x] With no reference lane named, all 45 factory presets render byte-identically, proved by a golden test
- [x] `03-afro-cuban` stops describing clave alignment as something the reader maintains by hand, and a `scope-framing` claim locks it

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP03 | Nothing in the engine knows the timeline lane exists. `ConstraintConfig` offers `anchorSteps`, `backbeatProtect` and density bounds, all lane-local, so mutation-adds and fill-adds can place hits that cross the clave and probability culls can drop clave-confirming ones. `03-afro-cuban` teaches that parts ignoring the clave "sound wrong"; the same gap covers the Ewe bell and tala accent structure | `capability` | `engine/src/constraint.cpp`, `engine/`, `03-afro-cuban.mdx` | Engine tests assert the added-step distribution shifts with the weight's sign and that drops avoid high-weight steps; a mutual reference is asserted refused; the golden asserts presets unmoved | `done` |

### Slice M002/S02 — Ghosts cluster where funk puts them

**Plan:** M002-S02-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S02.md
**Status:** done

**Definition of Done**

- [x] Ghost-add probability is higher on weak subdivisions preceding an accent than on those following one, asserted as a distribution over many seeds rather than a single roll
- [x] The weighting scales with the Complexity macro, so low Complexity keeps grooves clean
- [x] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [x] `11-funk`'s dedicated ghost lane is no longer the recipe the page prescribes, and a `scope-framing` claim locks the change

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP04 | Ghost notes come from a flat per-step mutation roll: any mutated step has equal chance of becoming a ghost, independent of where it sits in the meter. Funk ghosting is grammatical — ghosts concentrate on the weak subdivisions around the backbeat and fill toward the next accent (Danielsen 2006; Stewart 2000). Chapter 11 works around this with a dedicated high-hit-count ghost lane, which costs a lane and cannot respond to where the accents are | `capability` | `engine/`, `11-funk` chapter and its theory page | Engine tests assert the ghost distribution differs before and after an accent across many seeds, and that Complexity scales it; the golden asserts presets unmoved | `done` |

### Slice M002/S03 — Fills resolve onto the phrase boundary, and a tihai lands

**Plan:** M002-S03-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S03.md
**Status:** done

**Definition of Done**

- [x] Fill probability rises toward the end of a phrase cycle, with a shape parameter controlling how sharply, asserted as a distribution across the cycle
- [x] For an ungated lane the boundary used is the composite convergence point, not silence
- [x] A tihai of a given phrase length lands its final onset exactly on the target, asserted arithmetically rather than by ear
- [x] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [x] `06-indian-classical` no longer asks the reader to do the tihai arithmetic by hand, and a `scope-framing` claim locks it

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP05 | `FillLikelihood` is an envelope target with no knowledge of phrase position: a fill-add is as likely at beat 2 of bar 1 as at the end of an 8-bar phrase. Idiomatic fills cluster at phrase boundaries and resolve onto the downbeat, most explicitly the tihai — a phrase repeated three times to land on sam (Nelson 2008; Clayton 2000). Chapter 6 asks the reader to solve `3×P + 2×gap` by hand | `capability` | `engine/`, `06-indian-classical.mdx` | Engine tests assert the fill distribution concentrates toward the boundary and that a tihai's final onset equals the target; the golden asserts presets unmoved | `done` |

### Slice M002/S04 — A response lane answers its call

**Plan:** M002-S04-plan.md
**Validation:** format, unit, engine-isolation, rt-safety, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S04.md
**Status:** done
**Depends:** M002/S01

**Definition of Done**

- [x] A lane's phrase gate can be defined as open exactly when a named source lane's gate is closed, with an optional lead-in or overlap in beats
- [x] Changing the source lane's phrase settings keeps the antiphony intact, which is the failure the manual recipe has
- [x] A mutual reference between two response lanes is refused rather than followed
- [x] With no response lane named, all 45 factory presets render byte-identically, proved by a golden test
- [x] `15-compositional-grammar` no longer gives interleaving offsets as the recipe for antiphony, and a `scope-framing` claim locks it

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP06 | Call-and-response is achieved by hand-tuning `phraseLength`/`phraseGap`/`phraseOffset` until gates happen to interleave — `15-compositional-grammar` describes the recipe explicitly. It is fragile: change one lane's phrase settings and the antiphony breaks silently, because there is no structural relationship between the lanes. Kotekan couples patterns; this couples phrasing | `capability` | `engine/`, `15-compositional-grammar.mdx` | Engine tests assert the response gate is the complement of the source's, that it survives a change to the source's phrase length, and that a mutual reference is refused; the golden asserts presets unmoved | `done` |

## Milestone M003 — A lane can carry pitch

**Vision:** A lane emits a sequence of pitches with their own durations, so
Poly's polymetric machinery applies to melodic material and not only to
percussion.

**Branch:** milestone/M003-pitch
**Status:** done
**Demo:** One lane playing a five-note sequence against a seven-step cycle,
phasing, with drift and kotekan complement applying to it unchanged.

**The only milestone here not closing a guide gap.** It opens territory the
guide does not describe. Whether the guide grows to cover it, or the capability
ships ahead of the guide with that stated, is recorded as an open question
rather than settled — but the two must not diverge silently, which is this
programme's whole premise.

### Slice M003/S01 — A lane emits a sequence of pitches

**Plan:** M003-S01-plan.md
**Validation:** format, unit, engine-isolation, rt-safety
**Evidence:** evidence/M003-S01.md
**Status:** done

**Definition of Done**

- [x] A lane can carry an optional sequence of pitches with per-note durations, supplying successive hits' pitch instead of the single `midiNote`
- [x] The field's name does not collide with the existing `phrase*` fields, and the chosen name is recorded with its reason
- [x] Every existing lane feature — drift, kotekan complement, tempo multiplier, additive cells — applies unchanged with a sequence set, asserted for at least two of them
- [x] With no sequence set, all 45 factory presets render byte-identically, proved by a golden test
- [x] A pre-bump state loads as the single-pitch behaviour it played

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP07 | A lane emits one fixed pitch: `ev.pitch = cfg.midiNote`, a single assignment. Hand-drum traditions are one voice with several strokes — djembe bass/tone/slap, tabla bols, conga open/muted/slap — so each articulation currently needs its own lane, competing for the 8-lane budget and unable to share a pattern. A sequence also makes a lane a pitched voice, so the polymetric machinery applies to melodic material | `capability` | `engine/include/poly/types.h`, `engine/src/engine.cpp`, `engine/include/poly/state_io_*.h` | Engine tests assert successive hits take successive sequence pitches, that a named existing feature still applies, and that a pre-bump state loads as single-pitch; the golden asserts presets unmoved | `done` |

### Slice M003/S02 — The sequence reaches a factory preset

**Plan:** M003-S02-plan.md
**Validation:** format, unit, engine-isolation, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M003-S02.md
**Status:** done
**Depends:** M003/S01

**Definition of Done**

- [x] A lane's note sequence is expressible in a preset and reaches `site/src/generated/presets.json` under a raised schema version
- [x] At least one factory preset uses a sequence, and its lanes' pitches are asserted against the generated data
- [x] The preset count and any per-lane field-count guards are updated rather than bypassed
- [x] The guide names the capability and says the traditions chapters do not
      yet use it, locked by a `scope-framing` claim *(added at planning — see
      `M003-decisions.md`)*

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP08 | A capability reachable only from hand-written state is a capability users do not have. M003/S01 established the path: the emitter carries the field, the generator's schema gate rises, and a factory preset demonstrates it | `pipeline` | `engine/src/presets.cpp`, `engine/tools/emit_presets.cpp`, `site/scripts/generate-presets-json.mjs` | The generated `presets.json` carries the sequence at the raised schema version and a site test asserts the preset's pitches against it | `done` |

## Milestone M004 — Diagrams are rendered, not drawn

**Vision:** Every architecture diagram is Mermaid source rendered at build time,
and none is hand-maintained ASCII art.

**Branch:** milestone/M004-diagrams
**Status:** done
**Demo:** The plugin-architecture appendix renders vector diagrams that match
the site's typography, and a check fails if ASCII art returns.

**The inventory, re-measured 2026-09-19.** Seven files, **117** lines carrying
a box-drawing character: `docs/euclidean-rhythm-guide.md` (44), the appendix
(28), `docs/testing-strategy.md` (21), `docs/engine-spec.md` (9),
`ARCHITECTURE.md` (8), `docs/ui-guide.md` (4), `docs/webui-migration.md` (3).
`ARCHITECTURE.md` is at the repo root, which the issue's own list places under
`docs/`.

The figure stated at assess time was 108, which contradicted its own breakdown:
those per-file numbers summed to 116, and `docs/webui-migration.md` measures 3
rather than 2. Corrected here rather than left standing, because the sentence
claims the inventory was measured. No definition-of-done item now carries the
count — S02 asserts the property instead, so this figure is information and
cannot go stale into a gate.

**What the inventory turned out to be, measured at execution.** Only **32** of
the 106 lines are architecture diagrams. 20 are directory trees, which become
nested Markdown lists — a file listing is not a flowchart and Mermaid has no
representation for one. **49 are UI wireframes**, chiefly a 44-line annotated
mockup of the plugin window in `docs/euclidean-rhythm-guide.md`; those stay,
under a stated exemption, because no flowchart can express a panel layout and
forcing one would produce worse documentation than it replaced. The remaining 5
are the frozen audit record.

That is why the definition of done asks for *no ASCII architecture diagram*
rather than *no box-drawing character*: the original wording would have been
satisfiable only by mangling content the milestone was never aimed at.

### Slice M004/S01 — One diagram renders from Mermaid at build time

**Plan:** M004-S01-plan.md
**Design:** M004-S01-design.md
**Validation:** format, site-unit, doc-conformance, doc-discipline, guards
**Evidence:** evidence/M004-S01.md
**Status:** done

**Definition of Done**

- [x] A Mermaid source block in a site page renders to vector output at build time, not at page load
- [x] The rendering is deterministic: an unchanged source produces byte-identical output across two builds
- [x] One existing diagram is converted and renders correctly, with the ASCII original removed

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP09 | Establishing the pipeline is a different risk from converting content, and a reviewer could reasonably accept one and reject the other. Build-time rendering also raises the reproducibility question #282 records for the WASM artifacts: output that churns without a source change makes commit hygiene undecidable | `tooling` | `site/`, `site/package.json` | A build produces the vector output; a second build over unchanged source produces it byte-identically | `done` |

### Slice M004/S02 — All seven files convert, and ASCII cannot return

**Plan:** M004-S02-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline, guards
**Evidence:** evidence/M004-S02.md
**Status:** done
**Depends:** M004/S01

**Definition of Done**

- [x] No ASCII architecture diagram remains in the seven files, replaced by
      Mermaid source; any file still carrying box-drawing characters does so
      under a stated, greppable exemption — all proved by the guard
- [x] A check fails when ASCII box-drawing characters appear in a diagram position in any governed doc
- [x] That check has been shown to fail by reintroducing one
- [x] Every Mermaid fence in a governed doc is proved to render, including the
      four in `.md` files that no build touches

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP10 | Architecture diagrams live as ASCII art in seven files. They render as monospace blocks markedly unlike the rest of a typography-first site, and they are hand-drawn — the drift class M048 was built to kill | `docs` | `ARCHITECTURE.md`, `docs/`, `site/src/content/docs/appendix-plugin-architecture.mdx`, `scripts/` | The seven files carry no box-drawing characters; the guard is mutation-proved by reintroducing one and watching it fail | `done` |
| GP11 | Four of the five Mermaid fences live in `.md` files — `ARCHITECTURE.md` among them — which Astro never builds and GitHub renders at view time. `astro build` exits 1 on a bad fence, so the site-side one is covered twice over; nothing at all covers the other four. Found while converting them, not in the source issues | `tooling` | `site/tests/` | Every fence in a governed doc renders through the pinned renderer; the test refuses to pass if it finds no fences | `done` |

## Milestone M005 — The sanitizer findings are understood

**Vision:** Every sanitizer finding the nightly has filed is reproduced and
classified, and each is either fixed or recorded as benign with its reason.

**Branch:** milestone/M005-sanitizers
**Status:** in-progress
**Demo:** A local command runs the same five sanitizer variants the nightly
does, and the programme can say what each filed occurrence was.

**What the assessment found, correcting the input.** #142 is not a stale finding
nobody triaged. It is auto-refiled: 22 comments, one per nightly failure, the
most recent naming `ASAN-PLUGIN` where the title says `TSAN-PLUGIN`.

**Corrected again at execution, 2026-09-19, by reading the run history rather
than the last 20 results.** The assessment called this "an intermittent finding
at roughly 5%, across two sanitizers". It is not intermittent and it is not two
findings:

| Window | Nights | Result |
|---|---|---|
| 2026-07-22 → 07-25 | 4 | success |
| 2026-07-26 → 08-16 | **22** | failure, every night, `TSAN-PLUGIN` |
| 2026-08-17 → 09-15 | 30 | success |
| 2026-09-16 | 1 | failure, `ASAN-PLUGIN` |

Every occurrence is the same test, `HostTests.HandshakeStress_NoTearNoLoss`, and
the same defect. Under TSan it is reported deterministically —
`WARNING: ThreadSanitizer: data race`, `SUMMARY: … in memcpy`. Without TSan the
same race only occasionally produces an observable torn read, which is what the
2026-09-16 run caught through the test's own assertion
(`torn-read: final noteMap[1]=127 but map[0] implies 126`). AddressSanitizer
reported no memory error at all; the test found it.

Three consequences. `.github/tsan.supp` contains no suppressions, so nothing was
silenced. The 2026-09-16 torn read proves the race is **still live** despite 30
quiet nights. And what stopped TSan reporting on 2026-08-17 is unknown — the
`HandshakeStress_TSanClean` variant predates the streak — which matters, because
a race whose window merely narrowed looks exactly like a fixed one.

**A token was added for this milestone.** `.jk/validations.yml` gained
`sanitizers`, running all five variants the nightly runs, because nothing ran
them locally and a 5%-intermittent failure cannot be investigated through
dispatches alone.

### Slice M005/S01 — Every filed finding is reproduced or its resistance recorded

**Plan:** M005-S01-plan.md
**Validation:** format, sanitizers
**Evidence:** evidence/M005-S01.md
**Status:** done

**Definition of Done**

- [x] The `sanitizers` token runs all five variants locally and its result is recorded
- [x] Each of the filed occurrences is classified by sanitizer, stack and date
- [x] Either a finding reproduces locally, with the exact invocation and iteration count that produced it recorded — or the attempts are recorded with what was tried and what the filed occurrences' logs show

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP14 | Nothing runs the sanitizers outside CI, so an intermittent finding can only be observed through dispatches. Both outcomes of this slice are results: a reproduction gives the fix something to verify against, and a recorded failure to reproduce is what makes the next attempt cheaper rather than identical | `tooling` | `.jk/validations.yml`, `docs/plans/guide-parity/evidence/` | The token runs; the evidence names either the reproducing invocation or the attempts and the log analysis | `done` |

### Slice M005/S02 — Each finding is fixed or recorded benign

**Plan:** M005-S02-plan.md
**Validation:** format, unit, rt-safety, sanitizers
**Evidence:** evidence/M005-S02.md
**Status:** in-progress
**Depends:** M005/S01

**Definition of Done**

- [ ] Every finding classified in S01 is either fixed, or recorded as benign with the reason and a suppression entry naming it
- [ ] A fixed finding is shown gone by the means S01 established — the reproducing invocation, or a named nightly run if it never reproduced locally
- [ ] No suppression is added without a written reason

`GP13` (#89) rides in this slice's row table because the format has no
milestone-level row, and is `accepted` rather than sliced — see its row.

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GP12 | #142's 23 nightly failures are two groups, established by M005/S01. **Group A**, 22 occurrences 2026-07-26 → 08-16, was a real TSan data race on the two-slot host→RT exchange, already fixed by `076f545` on 2026-08-16. **Group B**, the single 2026-09-16 occurrence, is a defect in the test: assertion 3 encodes `noteMap` values up to 32767 while `readSceneState` clamps them to `[0,127]`, so the invariant it checks is destroyed by sanitize rather than by tearing — reproduced single-threaded, failing for 99.61% of writeIds. No outstanding product defect | `defect` | `tests/host/host_tests.cpp` | The invariant is re-encoded to survive the round trip, proved to accept clean maps and still detect a genuine two-writeId tear | `open` |
| GP13 | `tests-e2e/reich-play.spec.ts` asserts `lastFireTime > 2.5` against a 3 s Playwright wait, which is too tight and flakes locally while CI stays green. Carried in this slice's table because the ledger format has no home for a milestone-level row, but deliberately **not sliced**: it is a one-line threshold change, and a definition of done would be more ceremony than the change earns. To be landed as an ordinary pull request referencing #89 | `defect` | `site/tests-e2e/reich-play.spec.ts` | Accepted without a slice; the fix is an ordinary PR | `accepted` |

## Sequencing

The graph is almost flat. M001, M002, M003, M004 and M005 are mutually independent:
they touch different code, and none produces anything another needs.

Three slice-level dependencies are real:

- **M002/S04 depends on M002/S01.** #155 reuses the reference-lane-and-guard
  mechanism #152 builds. This is the one dependency that came out of reading the
  code rather than the issues, and it is why the two are in one milestone.
- **M003/S02 depends on M003/S01.** A preset cannot carry a field that does not
  exist.
- **M004/S02 depends on M004/S01.** Converting seven files onto a pipeline that
  does not render yet would be seven files of unverifiable work.

**M001 is worth taking first, for a reason the graph cannot express.** It
re-walks `applyTimingShifts` and `maxTimingShift`, which engine-capability M003
changed most recently — so a defect introduced there surfaces under M001's tests
rather than later, when it would be harder to attribute.

**M005 blocks nothing and is blocked by nothing,** but it is the only milestone
investigating a possible live defect. The others add capability; this one asks
whether something is already wrong.

## Related issues

- [#149](https://github.com/JimAKennedy/poly/issues/149) — closed by GP01.
- [#151](https://github.com/JimAKennedy/poly/issues/151) — closed by GP02.
- [#152](https://github.com/JimAKennedy/poly/issues/152) — closed by GP03.
- [#158](https://github.com/JimAKennedy/poly/issues/158) — closed by GP04.
- [#154](https://github.com/JimAKennedy/poly/issues/154) — closed by GP05.
- [#155](https://github.com/JimAKennedy/poly/issues/155) — closed by GP06.
- [#245](https://github.com/JimAKennedy/poly/issues/245) — closed by GP07, with
  GP08 the pipeline consequence.
- [#111](https://github.com/JimAKennedy/poly/issues/111) — closed by GP10, with
  GP09 the pipeline it needs.
- [#142](https://github.com/JimAKennedy/poly/issues/142) — closed by GP12, with
  GP14 the triage it depends on.
- [#89](https://github.com/JimAKennedy/poly/issues/89) — GP13, accepted without
  a slice.

## Out of scope

Recorded so a later pass does not rediscover them as omissions.

- **Per-step subdivision profile editing in the WebUI** —
  [#305](https://github.com/JimAKennedy/poly/issues/305). Deferred by
  engine-capability M003 with its reason; M003 raises the same question for note
  sequences and defers to the same issue rather than solving it twice.
- **CI and tooling debt other than #142** —
  [#266](https://github.com/JimAKennedy/poly/issues/266),
  [#267](https://github.com/JimAKennedy/poly/issues/267),
  [#274](https://github.com/JimAKennedy/poly/issues/274),
  [#282](https://github.com/JimAKennedy/poly/issues/282). #274 is adjacent —
  it names `HostTests.HandshakeStress_NoTearNoLoss`, and the TSan plugin job
  runs that same suite — so M005/S01's triage may touch it. If it does, that is a
  finding to record, not a row to add here.
- **`appendix-presets.mdx` coverage** —
  [#100](https://github.com/JimAKennedy/poly/issues/100). Filed when the engine
  shipped 43 presets and the appendix documented 14; it now ships 45, so the gap
  has widened. Unrelated to guide parity as defined here.
- **Whether the guide grows to describe pitched lanes.** M003's open question,
  recorded in its milestone rather than resolved at assessment.
