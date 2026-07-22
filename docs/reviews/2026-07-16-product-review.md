---
class: archived
---

# Poly — Product & Engineering Review

**Date:** 2026-07-16
**Scope:** engine, plugin layer, web UI, docs/training site, engineering process, product vision/design, market landscape
**Method:** four parallel deep-dive reviews (engine internals with a full standalone build + test run; VST3 plugin layer; site/webui/CI; market research with cited sources), plus direct review of vision docs, changelog, git cadence, and GitHub project hygiene. File:line references are against commit `93385db` on `main`.

---

## Executive summary

Poly is an unusually well-engineered late-alpha/early-beta product with a genuinely differentiated concept and a 95th-percentile verification culture — but it currently **cannot be obtained by anyone who isn't a C++ developer**. There are no releases, tags, installers, or signed binaries. The engine is near production-grade; the plugin layer has a handful of real correctness bugs (including a save-after-stop data-loss path); the training site is near-commercial quality but contains music-theory errors that undermine its credibility claim. The most urgent work is distribution, the five plugin-layer bugs, and a theory-accuracy pass — roughly in that order.

Structural observation: the project's execution speed is its biggest risk multiplier. The same velocity that produced ~26 milestones in a few weeks also produced stale specs, contradictory docs, and half-finished handshake patterns declared "eliminated" in migration notes. The verification culture is superb at catching *engine* regressions; the failure modes now live in the seams — threads, docs, distribution — where the gates don't yet reach. See the appendix for a concrete strategy to make non-engine claims testable.

---

## 1. Technical implementation maturity

### 1.1 Engine (`poly_engine`) — verdict: late beta, production-grade core

Built standalone (`POLY_ENGINE_ONLY=ON`): **294 tests, 100% pass, 0.7s wall time.**

**Strengths**

- `Engine::renderRange` (`engine/src/engine.cpp:373`) is a pure function — the `Engine` class holds no state. All timing (step positions, envelope phase, drift, phrase gating) derives from absolute PPQ; the "no accumulation" promise was verified true in every path:
  - Step positions: `absStep * stepPpq` from `floor/ceil(ppq / stepPpq)` (`engine.cpp:304-312, 322-334`)
  - Envelope phase: `fmod(ppq / periodPpq + offset, 1.0)` with negative correction (`envelope.cpp:51-59`)
  - Drift: `floor(ppq/4 * driftRate)` — not an accumulator despite the region name (`engine.cpp:346-357`)
- RNG is a stateless splitmix64-style hash keyed `(seed, laneId, absStep, channel)` with purpose-separated channels (`rng.h:10-23`). No `rand()`, no stateful PRNG anywhere.
- Serialization discipline is exemplary: state version 15 (`state_io.h:13`), per-version gated reads, sanitization structurally fused into deserialization (NaN-aware clamps annotated with the division each protects, `sanitize.cpp`), a libFuzzer harness that renders fuzzed state (`tests/fuzz/fuzz_state_io.cpp`), backward-compat regression tests at v1/v2/v11/v12/v13/v14.
- Golden-test methodology is exactly right for a generative engine: byte-identical output asserted across three block sizes (0.05/0.5/2.0 PPQ), per feature, plus loop-restart, 1000-bar transport-jump, tempo-independence, seed-sensitivity, and buffer-overflow tests. `Scene.MorphAtZeroPreservesAllLaneFields` guards the classic "forgot to interpolate the new field" bug.
- RT design: no heap/locks/exceptions/I-O in the render path; fixed-capacity buffers with drop counters; block-boundary safety-margin window math that provably emits each shifted note exactly once (`kTimingSafetyMarginMs = 20.0` exactly covers the sanitized ±20ms microTiming range — correct, but zero headroom if the range ever widens).

**Weaknesses / bugs**

| # | Finding | Location |
|---|---|---|
| E1 | `poly_action_set_envelope` gap slots: setting index 2 with `envelopeCount==0` exposes slots 0-1 as default-constructed `EnvelopeAssign` whose default is `active=true` — two phantom full-depth velocity LFOs | `engine/src/wasm_api.cpp:686-700`, `types.h:159` |
| E2 | `fixedPatternLength` semantic inconsistency: pattern built at that length, but cycle wrap uses `cfg.cycle.steps` — a `fixedPatternLength=10, steps=16` lane plays 16 steps with 6 silent, contradicting the field's own comment and the UI's assumption | `engine.cpp:84-86` vs `:144, :348`; `types.h:204`; `timeline_step_editor_view.cpp:67` |
| E3 | `writeSMF` has no `tempo > 0` guard → inf → UB in `uint32_t` cast (bridge guards the same case; SMF doesn't) | `smf_writer.cpp:68` |
| E4 | Lane `id` (not index) keys the RNG; sanitize doesn't enforce uniqueness → duplicate ids silently correlate two lanes' rolls | `rng.h`, `sanitize.cpp` |
| E5 | `writeGrooveStateBody(bodyVersion)` honors old versions for lanes but `writeEnvelope` always writes v2 format — latent trap if old-version writes are ever requested | `state_io.h:18`, `state_io_write_lane.h` |
| E6 | **Hardcoded 4/4** across engine, capture, and scene chain; no time-signature input on `TransportContext`. A scope decision, but undocumented — and ironic for a polymeter tool courting aksak/tala audiences | `engine.cpp:12`, `midi_capture.h:14`, `scene.h:53` |
| E7 | `docs/engine-spec.md` is dangerously stale: lists accent masks, envelopes, humanize, macros as "not implemented" (all shipped and tested); LaneConfig table missing ~18 shipped fields; `renderRange` signature omits `EmissionEventBuffer*`; describes ~v1 while code is at v15 | `docs/engine-spec.md:177-234` |
| E8 | RT-safety scanner covers only `engine.cpp`, `euclidean.cpp`, `rng.h`, `types.h` — `scene.cpp`, `macro.cpp`, `constraint.cpp`, `envelope.cpp` all run on the audio thread every block and are unscanned (currently clean by inspection) | `scripts/check-realtime-safety.sh:13-22` |
| E9 | Header hygiene wart: `state_io.h` includes lane read/write headers mid-file inside the namespace; those headers call functions they don't declare — compiles only via inclusion order | `state_io.h:71-72` |
| E10 | Perf note (not a violation): `GrooveState` ≈ 13.6 KB; per-block pipeline copies it by value 3-4× (~40-55 KB memcpy/block). Deterministic and alloc-free; worth profiling eventually | `engine.cpp`, pipeline call sites |

Test gaps: no negative-PPQ transport tests, no `EmissionEventBuffer` overflow test, fuzzing opt-in and apparently not run in CI, no performance benchmark.

### 1.2 Plugin layer (VST3) — verdict: late alpha / early beta

RT discipline, state versioning, VSTGUI ownership hygiene (zero leaks found across 15 custom views; consistent `// ownership-transfer` annotations), and host-level test coverage (stop-flush, jump note-offs, determinism, buffer-size invariance, golden 4-bar output, dedicated `midi_probe` companion plugin) are well above solo-project standard. The cross-thread machinery, however, has real holes.

**Critical**

| # | Finding | Location |
|---|---|---|
| P1 | **Save-after-stop drops edits (data loss).** The `!tc_.playing` path never refreshes `stateSnapshot_`; play → stop → tweak → save serializes the last *playing* state. Second save falls through to live `sceneState_` — see P2 | `processor.cpp:383-415, 467-470, 834-839` |
| P2 | `getState()` fallback serializes `sceneState_` on the host thread while `process()` may be mutating it — torn state can be written into the project | `processor.cpp:839` |
| P3 | **`kDistributable` contradicted by its own IPC.** Factory declares the component distributable, but the processor sends the raw address of its `uiSnapshot_` via `IMessage` and the controller dereferences it at 30 Hz — garbage pointer in any bridged host; dangling pointer on component reload even in-process (no `disconnect()` override clears it) | `factory.cpp:14`, `processor.cpp:83-92`, `controller.cpp:439-446`, `web_ui_view.cpp:743-763` |
| P4 | Writer-side TOCTOU in every UI→audio handshake: `setState()` and all six `notify()` handlers write pending structs without checking the ready flag is false → tearing under rapid edits, plus silent lost updates. Reader side is correct; migration notes claiming "race elimination (atomic handshakes)" describe only half the pattern | `processor.cpp:696-796, 851-853` vs `:344-379` |

**Significant**

| # | Finding | Location |
|---|---|---|
| P5 | Stuck-note path 1: `flushDue` lower bound (`ppqOff >= ppqStart`) can strand note-offs in the inter-block "crack" during tempo ramps (ppqEnd extrapolated with block-start tempo; jump detector tolerates 0.001 PPQ). Lower bound should not exist | `engine/src/bridge.cpp:25-37`, `processor.cpp:107-112` |
| P6 | Stuck-note path 2: `pendingNoteOffs_.push` return ignored — silent off-drops at >512 pending | `processor.cpp:184` |
| P7 | Controller reflects only Scene A into parameter values on `setComponentState` — load a project saved on Scene B and the knobs display A while the processor plays B | `controller.cpp:299-355` |
| P8 | Automation scene-addressing is non-deterministic under chain playback: `applyParameter` routes to the *currently selected* scene, which chain playback rewrites every block | `processor.cpp:420-424, 650` |
| P9 | Loop wrap treated as generic jump → wipes `captureBuffer_` every cycle pass — "capture while jamming with Cycle on" (the primary documented Cubase workflow) can never accumulate more than one pass. Undocumented | `processor.cpp:131` |
| P10 | `kExportTrigger` never resets to 0; hosts that dedupe unchanged values won't deliver the second export click | `processor.cpp:680-682`, `export_controls_view.cpp:301` |
| P11 | **Parameter scaling math hand-copied in four places** (processor, controller `setComponentState`, `WebUIView::applyEditToCache`, preset pushers) — the largest standing defect-injection risk; any range change must be made 4-5× | `processor.cpp:527-690`, `controller.cpp:302-354`, `web_ui_view.cpp:541-718`, `header_view.cpp:299-360`, `web_ui_view.cpp:399-460` |
| P12 | `IEventList` output not sample-sorted (all offs, then all ons); Cubase tolerates, pedantic hosts may not | `processor.cpp:151-191` |

Minor: stereo-bus assumptions in silence flags (`processor.cpp:319-327`); block-start tempo used for sample-offset conversion (slight skew under ramps); web UI `editCooldown_` ≈ 660 ms echo-suppression heuristic papers over the snapshot round-trip race rather than fixing it (`web_ui_view.cpp:191-197`); `transport_frame_exchange.h` is dead code contradicting `ui_snapshot.h`'s multi-instance guarantee — delete it; `web_ui_view.cpp:73` leaves `enableDebugMode = true`; serializer emits `"tempo":0` (`bridge_serialization.cpp:242`); event count display hardcodes capture capacity 2048 (`export_controls_view.cpp:206`).

**Dual UI status:** deliberate, documented migration (VSTGUI shipping; WebView experimental behind `POLY_WEB_UI=OFF`, `plugin/CMakeLists.txt:74-108`) — not sprawl. But `docs/webui-migration.md` has drifted stale in both directions (claims 6 open `TODO(spike)` markers — there are zero; claims bridge params missing that now exist).

### 1.3 Engineering process — verdict: very high rigor, one dead spot

- CI: 3-platform matrix (macOS-14 arm64 / ubuntu / windows MSVC), VST3 validator, **engine-isolation job that fails if the engine build fetched the VST3 SDK** (`ci.yml:83-88`), gitleaks, 60% coverage gate + Codecov, webui Playwright, pluginval strictness 8, WASM build with size reporting, sample-manifest strict validation, full local site build + audio-gate e2e, aggregate `ci-complete` gate. All actions SHA-pinned.
- `deploy-site.yml` does **post-deploy verification against the live URL** including audio-correctness gates and a sha256 WASM freshness handshake. Rare anywhere.
- Local gates mirror CI (pre-commit + pre-push: format, RT scan, snippet regions, build, tests, main-push block).

Gaps:

| # | Finding | Location |
|---|---|---|
| G1 | Sanitizer jobs set job-level `continue-on-error: true` → job conclusion is success → the `needs.*.result == "failure"` notify/issue-filing path is likely dead code. Verify with a forced failure | `sanitizers.yml:12,27,42` |
| G2 | Sanitizers and coverage are engine-only — plugin threading/VST3 lifecycle code (where P1-P4 live) never runs under ASan/TSan and is outside the coverage gate | `sanitizers.yml`, `ci.yml` |
| G3 | pluginval not run on the Windows artifact | `ci.yml:196-201` |
| G4 | pluginval fetched from `releases/latest` (unpinned) — nondeterministic CI risk | `ci.yml:218-220` |
| G5 | RT-safety check is a lexical denylist grep; indirect allocation passes silently. Fine as tripwire; know its limits | `scripts/check-realtime-safety.sh` |
| G6 | No dependabot/renovate; single CODEOWNER; no branch protection (known Pro limitation, pre-push hook is the substitute) | — |

---

## 2. Documentation / training website — verdict: near-commercial polish, credibility at risk from accuracy slips

**Strengths**

- Complete arc: 18 chapters (foundations → 12 traditions/genres → synthesis → practical workflow) + 10 appendices + a real 351-line user manual (`guide-using-poly.mdx`) with per-DAW routing instructions and troubleshooting.
- Interactive machinery is a standout: the **real C++ engine compiled to WASM plays in-page** (`PolyPreviewCard.astro` — shared AudioContext, per-lane mute chips, RMS probe consumed by CI); "Try It" opens the full plugin web UI in a sandboxed iframe modal; CI enforces **byte-level SMF parity** between site play cards and the Try It modal (`site/tests-e2e/equivalence.spec.ts`).
- Sample licensing is production-grade: per-file SPDX + source URL manifest, credits page rendered *from* the manifest (attribution cannot drift), license texts committed, strict CI validation with coverage.
- Code snippets live-extracted from real source via `// region:` markers and build-verified (`scripts/check-snippet-regions.sh`).
- Site has 5 unit-test suites and a 15-spec e2e battery including console-error and audible-floor RMS gates.

**Weaknesses — accuracy (most damaging for an education-anchored product)**

| # | Claim | Problem | Location |
|---|---|---|---|
| D1 | Son clave 3-2 "is exactly E(5,16)" | False — son clave (gaps 3,3,4,2,4) contains a 2-gap; E(5,16) gaps are 3,3,3,3,4. Toussaint identifies E(5,16) with the bossa-nova cell — which `10-brazilian.mdx:48,66` states *correctly*, so the site contradicts itself. Same chapter correctly notes rumba clave is non-Euclidean while resting on a false premise for son | `03-afro-cuban.mdx:36,52-53` |
| D2 | Euclidean reference labels E(5,16) "Bossa nova bass / son clave" with gap math "3+3+2+4+3+1" | Six intervals for five onsets — arithmetically impossible; repeats the son-clave-at-rotation-0 claim | `appendix-euclidean-reference.mdx:114,120` |
| D3 | Clapping Music printed as `x x x . x x . x . x . .` (7 onsets), called "E(8,12), eight claps" | Reich's pattern is `xxx.xx.x.xx.` (8 claps) and is not E(8,12) | `08-minimalism.mdx:18` |
| D4 | Bjorklund's algorithm from "1960s nuclear physics" | SNS timing-system work is ~1999-2003 | `01-foundations.mdx:12` |
| D5 | E(4,9) given as "2+2+3+2, close to standard daichovo" | Canonical E(4,9) is 2+2+2+3, which *is* the standard daichovo grouping — underclaims its own best example | `07-balkan.mdx:40` |
| D6 | "26 patches" | 43 presets exist in generated `presets.json` | `index.mdx:29` |
| D7 | Canonical URLs / OG metadata point at `jimakennedy.github.io/poly` while branding says poly.jk.digital | `astro.config.mjs` |
| D8 | Web UI contract docs claim "14 factory presets (index 0-13)" vs 43 presets / 10 categories in schemaVersion 2; `bridge.schema.json`'s `applyPreset` payload is untyped so the schema can't catch it | `host-iface.js:35,53`, `bridge-schema.md:20,42-43` |

References appendix mixes real scholarship (Toussaint, Arom, Holzapfel) with YouTube/Fiveable/Scribd links — fine for a guide, thin for the "ethnomusicological scholarship" framing.

---

## 3. Product vision & execution

**Vision: coherent and unusually disciplined.** The IMPLEMENTATION_PLAN's core bets — engine purity, determinism as a product principle, Cubase-first, dynamics-first velocity modeling, education paired with the tool — are all visible in the shipped code. Determinism isn't marketing: it's enforced by golden tests in CI. Phases 0→2 executed essentially in order; milestone discipline (M019→M045 in the visible ~2.5-week window, everything through PRs, Keep-a-Changelog) shows very high tempo.

**Execution gaps**

- The roadmap source (`internal-docs/roadmap.md`) referenced by IMPLEMENTATION_PLAN.md is not in the repo; the PRD (Phase 0.5 deliverable) doesn't exist; the plan doc still ends with a stale day-one "tell me to proceed" prompt.
- **Zero GitHub issues, zero releases, zero tags.** Planning lives outside the repo — contributor-hostile for an open-source project: no public backlog, no "good first issue," no visible direction. CHANGELOG says 0.1.0 shipped 2026-06-27; that version exists only as text.
- Docs describe the product's past, not its present (engine-spec, webui-migration, plan doc all drifted).

---

## 4. Product design maturity

**Strengths:** lanes-as-cycles rather than bar-grid as the core model; macro morphing; scene chaining; seeded reproducibility with explicit randomness controls; progressive disclosure (overview → lane → envelope). "Learn the tradition in the browser, then open the same preset in your DAW" is a first-run story none of the competition has. The tradition-grounded preset taxonomy (aksak, kotekan, clave) is pedagogically honest and a natural browse structure.

**Weaknesses:**

- **The first-run experience doesn't exist yet.** A MIDI-only instrument that makes no sound is the classic support nightmare; the manual addresses routing, but the product has no onboarding, no built-in preview voice, and no way to install it at all.
- The manual's Logic instructions describe loading an AU that isn't built — documentation ahead of product.
- Automation semantics under scene chaining (P8) is a design ambiguity needing a decision, not just a patch: what *should* an automation lane mean when scenes chain?
- The dual-UI migration splits design effort across two implementations of every surface.
- Hardcoded 4/4 caps the design ceiling for exactly the audiences (Balkan aksak, Indian tala) the education content courts.

---

## 5. Market comparison

Verified current as of mid-2026 (sources in the market appendix of the review conversation; key URLs inline).

| Offering | Price | Relation to Poly |
|---|---|---|
| HY-Plugins HY-RPE2 | $48 | Closest commercial: 8-track grid+Euclidean sequencer, macros, LFOs. Mature, cheap, cluttered; no multi-bar envelopes, no determinism, no education. |
| Audiomodern Playbeat 4 | $89 | Volume leader in "generative drums": ships 1,500 kits, iOS, drag-MIDI. Sells surprise — explicitly *not* reproducible. |
| Sugar Bytes DrumComputer | $129 | Drum synth + sequencer; 16-step bar-grid mindset, sounds-bound. |
| Devicemeister Stepic | €39 | Polymetric mod-lane sequencer, Ableton favorite; melodic-leaning, no Euclidean generators. |
| WaveDNA Liquid Rhythm | $79 | The cautionary tale: deep rhythm theory + steep learning curve → commercially stalled. |
| XLN XO / Algonaut Atlas 2 | $99-148 | Sample-space beatmakers; compete on sound content, not concept. |
| Toontrack EZdrummer 3 / UJAM | $25-179 | Human-groove browsing paradigm; defines the mainstream expectation Poly is not chasing. |
| Five12 Numerology | $125-199 | The deep-sequencer benchmark; **Mac only** — leaves Poly's Cubase/Windows target unserved. |
| Ableton Live 12 MIDI Generators / Bitwig Note Grid / M4L Euclidean devices | free w/ DAW | The free competition inside other hosts. **Cubase has no Euclidean/polymetric generator** — Poly fills a real gap in exactly its target DAW (Beat Designer + Pattern Editor only). |
| XronoMorph | free | Strongest conceptual kin (geometry of rhythm, NIME pedigree); standalone-only, effectively unmaintained. |
| Factor8 Constellations (2026) | free, GPLv3 | Newest direct free competitor: Euclidean polyrhythmic, 16-32 channels, VST3/AU/standalone. Proves the niche and license model; no curriculum, envelopes, scenes, or determinism story. |
| Stochas (Surge Synth Team) | free, OSS | Best-run precedent for Poly's model: open-source sequencer with CI, installers, docs. Probabilistic, not polymetric. |
| TidalCycles / Strudel | free, OSS | Strongest free competitor for deep polyrhythmic expression; requires code, not a plugin. |
| Groove Pizza / Ableton Learning Music / Melodics | free-$15/mo | Pedagogy benchmarks; none has depth + cultural curriculum + a plugin. |

**White space (real and defensible):**
1. Polymeter-first architecture as the *core model* (everything commercial is bar-grid-first with polymeter bolted on).
2. Determinism/reproducibility as a feature no generative competitor offers (film/game composers, collaboration, version-control mindsets).
3. Cubase-first in a market where every generative sequencer chases Ableton — and Cubase has no native equivalent.
4. Curriculum + tool as one artifact — the clearest moat; each competitor holds at most one piece.
5. GPLv3 with real engineering discipline (Stochas/Surge-tier credibility, above hobby freeware).

**Table stakes currently missing:** AU format (Logic users are a large share of this audience; the manual already references it); drag-MIDI-from-plugin-to-track (native-UI-only today; every competitor has it); preset volume expectations (43 vs Playbeat's 1,500 — the tradition taxonomy is a quality answer to a quantity question, but the bank should grow); one-click "new seed" randomize with history; per-DAW routing quirk handling beyond Cubase; sound content is deliberately absent — mitigate with first-run routing UX, possibly a trivial bundled preview voice.

**Realistic positioning:** don't fight the $39-129 discount-driven creative-sequencer tier on content/marketing. As a **free, GPLv3, education-anchored niche instrument**, Poly competes only with Constellations (younger, shallower) and Stochas (different concept), and the site is a discovery engine none of them has. The WaveDNA lesson applies: keep the immediate-gratification path short or depth becomes a liability. At launch, BPB/KVR/Rekkerd coverage matters more than features.

---

## 6. Most urgent next steps (ordered)

1. **Ship something installable.** Tag v0.1.x; add a release workflow producing signed/notarized macOS + Windows binaries with installers; put a Download button on the site. Everything else is ahead of this gap. (Stochas's release pipeline is a ready-made model.)
2. **Fix the plugin-layer correctness bugs before release:** P1+P2 (refresh snapshot in the stopped path / handshake-request getState), P3 (declare `0` instead of `kDistributable` as the one-line mitigation; add `disconnect()` overrides nulling the snapshot pointer), P4 (write-only-when-flag-clear or 2-slot/seqlock exchange), P5+P6 (drop `flushDue` lower bound; check `push` return). These surface the week strangers start using it, and they live in code the sanitizers never see.
3. **Theory-accuracy pass on the site** (D1-D6). Cheap to fix; the education moat is worthless if percussionists catch the flagship chapter being wrong about clave.
4. **Kill the doc drift** — see the appendix for the systematic version. Immediate: rewrite/generate `engine-spec.md` from the v15 model; refresh `webui-migration.md`; resolve the `internal-docs/roadmap.md` reference; open a public backlog (issues/milestones).
5. **Consolidate parameter scaling into one shared definition** used by processor, controller, web bridge, and preset pushers (P11) — this also enables generating the parameter docs (appendix).
6. **Close verification blind spots:** ASan/TSan on the plugin, fix the `continue-on-error` dead-notification path in `sanitizers.yml`, add the four missing audio-thread files to the RT scanner, pluginval on Windows, pin pluginval.
7. **Then market-facing gaps:** AU build → drag-MIDI-to-track in the web UI → decide automation-vs-scene-chain semantics → document (or lift) the 4/4 limitation.

Engine fixes E1-E5 can ride along with any of the above; none blocks release by itself, but E1 and E2 are user-visible once the web UI ships.

---

## Appendix: Making non-engine claims testable (anti-staleness strategy)

The engine stays honest because its claims are executable (golden tests). Everything that went stale — specs, counts, contract docs, migration status, theory claims — is prose restating facts that live somewhere else. The fix is not more review; it's removing the second copy or gating it. The repo already invented all the right patterns locally (snippet regions build-verified from source; credits rendered from the sample manifest; `presets.json` generated by `emit_presets`); the strategy is to universalize them.

**Principle: every fact appears in exactly one machine-readable place. Docs either render it (generated), assert it (tested), or declare themselves historical (archived). Prose may explain; it may not be the only carrier of a checkable fact.**

### A. Generate what can be generated (kills the biggest classes)

1. **Parameter registry as single source of truth.** Move every parameter's id/range/scaling/format into one X-macro or constexpr table (`params.def`). Derive from it: processor apply-code, controller reflection, web-bridge cache math, preset pushers (fixes P11's 4-way duplication), *and* `appendix-parameters.mdx` + the engine-spec LaneConfig table (fixes the worst of E7). One change site; docs cannot drift because they are output.
2. **Counts and inventories are always rendered, never typed.** "26 patches" (D6) happened because a number was hand-typed. Emit preset/chapter/lane counts into a generated JSON the site imports; add a lint that greps `.mdx` for hard-coded numerals adjacent to words like "presets/patches/chapters" and fails.
3. **Contract doc from schema.** Type the `bridge.schema.json` payloads fully (e.g. `applyPreset` index bounded by generated preset count), generate the human-readable `bridge-schema.md` tables from the schema, and keep the existing two-sided contract tests. D8 becomes impossible.

### B. Assert what can be asserted (theory and behavior claims)

4. **Music-theory claims as fixtures.** Every pattern the site displays or names is machine-checkable against the same `euclid()` the product ships. Add a `claims` fixture per chapter (already have `PolyPatch` blocks as anchors): `{"pattern": "x.x.x..x", "isEuclidean": {"k":5,"n":16,"rotation":r}}` or `"isEuclidean": false`, onset counts, gap sequences. A site unit test evaluates each claim. D1-D3 and D5 would all have been red in CI. Claims that aren't formalizable (historical dating, attribution) get an explicit `"verify": "human"` tag with a `verifiedOn` date — visible debt instead of silent rot.
5. **Behavioral doc claims → golden tests.** Any doc sentence of the form "X produces Y" (e.g. capture-during-loop semantics, P9) either has a test with the doc citing the test name, or the sentence carries a `⚠ unverified` marker the linter enforces. If writing the test is too expensive, the marker is the honest state.

### C. Gate what can only be drift-detected (prose that explains)

6. **Doc taxonomy in front-matter, enforced by CI.** Every file under `docs/` and `site/src/content/docs/` declares `class: generated | gated | archived` (+ `sources:` globs for gated docs). A check fails on undeclared docs. `IMPLEMENTATION_PLAN.md` and `webui-migration.md` become `archived` with a banner ("historical; see X for current state") — archived docs are exempt from truth expectations, which is most of the battle: the current problem is stale docs *pretending to be current*.
7. **Drift map (CODEOWNERS-for-docs).** A small YAML mapping source globs → gated docs: `engine/include/poly/types.h → docs/engine-spec.md`, `plugin/source/webui/** → webui/bridge-schema.md`, `engine/src/presets.cpp → site preset pages`. CI check: a PR touching a mapped glob must either touch the mapped doc or carry a `docs-not-affected` trailer/label in the PR. This is deterministic, cheap, and forces the decision at the moment the knowledge is fresh — the only time doc updates are cheap.
8. **Ban status prose.** "6 TODO(spike) markers remain," "not implemented yet," "Status: Phase B" — status claims in prose are stale the day after writing. Policy + lint: progress lives in CHANGELOG/issues/generated dashboards; docs describe contracts and rationale only. Any `Status:` section must carry a date and is flagged after N days.
9. **Symbolic references, not line numbers.** Comments and docs that cite `processor.cpp:429` rot on every edit. The `// region:` marker system already exists and is verified — extend it: docs/comments reference region names or symbols; `check-snippet-regions.sh` (or a sibling) verifies referenced anchors exist. Allow `file:line` only in dated review documents like this one.

### D. Process backstops

10. **Public backlog as the living roadmap.** Issues/milestones replace roadmap prose; the plan doc archives. A roadmap that is a query ("open milestones") cannot go stale.
11. **Release notes generated from Keep-a-Changelog + tags**, so version claims ("0.1.0 shipped") are backed by artifacts.
12. **Scheduled AI review stays useful but becomes the *last* line**, scoped to exactly the residue the above can't check: prose quality, pedagogy, "does this explanation still match the design intent." Everything mechanical should be red/green before an LLM ever reads it.

**Suggested order:** (7) drift map + (6) taxonomy first — one afternoon, immediately stops the bleeding for every doc; then (1) parameter registry (biggest dual payoff: doc truth + P11); then (4) theory claim fixtures before the next content push; then (2)(3)(8)(9) as ratchets. Target state: `main` cannot merge with a checkable claim that is false, and every uncheckable claim is visibly labeled as such.
