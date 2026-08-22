# Poly — Full Verification & Project Assessment (2026-08-22)

Assessed at `origin/main` @ `dbacc08`. Companion to `theory-conformance-review.md`
(the original findings) and `theory-remediation-plan.md` (the plan) on this branch.

**Method.** Ground truth first: full engine build + test run on origin/main
(**465/465 pass**, 2.1 s) and the site unit suite (**128/128 pass**, including the new
doc-conformance guardrails). Then seven parallel audits, each reading code and docs
side-by-side with all pattern arithmetic recomputed against the shipped Bjorklund
generator: (1) theory-page P0 fixes, (2) preset conformance P1/P6.1, (3) chapter prose
P3, (4) appendices/guardrails P4–P6, (5) a repo-wide doc-vs-code consistency sweep,
(6) code quality + release readiness, (7) feature completeness incl. bridge/WASM parity.

---

## 1. Executive summary

The remediation programme was executed **infrastructure-first, content-second** — and
that shows in the results:

- **Fully landed (excellent):** the Bjorklund generator switch (P2.0) with exhaustive
  equality tests, pinned Toussaint canon, and lossless state migration; the shared
  site generator module; generated preset tables replacing all hand-written ones
  (P4.1); preset-name resolution contracts (P4.2/P6.2); the 11-test doc-conformance
  suite wired into CI and pre-push (P6.4); taxonomy conformance (P4.5).
- **Half landed:** theory-page repairs (P0 — 1 clean, 6 partial, 2 not done, plus a new
  cross-page bell-rotation problem introduced by the generator switch itself); the
  preset lint (P6.1 — real, non-vacuous, but with scope gaps that let every remaining
  defect pass).
- **Mostly not landed:** the per-preset musical fixes (P1.2 — a generic
  `lockReferentLane()` post-processor delivered the *structural* lock on all 43 presets,
  but ~15 of the 21 prescribed content fixes were skipped, and in four presets the lock
  landed on the wrong lane); the chapter prose corrections (P3 — majority NOT DONE);
  idiom-break framing (P5 — 5 of ~20).

**The critical insight: both test suites are green, but green ≠ conformant.** The
guardrails were scoped around the remaining defects — a 7-entry saturation allowlist, a
hard-coded `FINDINGS.length === 5` framing contract, a backbeat-survival check limited
to one category, and appendix prose/diagrams explicitly excluded from arithmetic
verification. The machinery to enforce conformance exists and works; its coverage was
drawn to fit what was fixed, not what the theory requires.

Beyond the remediation scope, the project is in objectively strong shape: a genuinely
clean 535-line engine core behind a CI-enforced isolation gate, 111 test files /
~31.6k LOC across four layers, zero TODO/FIXME debt, disciplined dependency pinning,
and an unusually mature guard culture (CI checks that have their own red/green
behavior-proof tests). The debt is structural (three monolith files, one hand-forked
preset dataset) and editorial (docs lagging code), not architectural.

---

## 2. Remediation programme verification (per phase)

| Phase | Verdict | Detail |
|---|---|---|
| **P2.0** Bjorklund switch | ✅ **Implemented, exemplary** | RT-safe fixed-buffer Bjorklund in `engine/src/euclidean.cpp`; exhaustive engine-vs-reference equality test (all k≤n≤64 × all rotations, `tests/euclidean_tests.cpp:200`); pinned canon incl. bossa r10 / rupak r3 / Ewe bell r9 (`:241-251`); `euclideanMigrationDelta` + `legacyBresenham` wired into versioned state load (`state_io_read_lane.h:183`). One nit: the CHANGELOG's "migration is lossless" is stated unconditionally, but the code has two documented carve-outs (no-exact-shift fallback returns 0; timeline lanes skipped). |
| **P2.1/P2.2** one site generator | ✅ | `site/src/audio/bjorklund.ts` shared module; a test asserts the diagram and playback use *the same function object*. |
| **P0** theory-page repairs | 🟡 **6 of 9 partial, 1 clean, 2 not done** | P0.1 (Afro-Cuban tumbao) fully fixed and verified. P0.7 (tihai) still mathematically invalid for the periodic phrase gate: Length 5 / Gap 0.5 has period 5.5, which does not divide 16 — the third statement misses sam from cycle 2 onward, the exact failure the rule itself names. P0.4 lane 4 rotation still sub-optimal. 29 residual defects catalogued with corrected values (see §3). |
| **P1.1** referent locks | 🟡 **Structurally complete, musically partial** | `lockReferentLane()`/`lockPresetReferent()` (`presets.cpp:2473-2515`) gives all 43 presets exactly one timeline-locked referent (prob 1.0, mut 0, no gate) + `backbeatProtect` on backbeat lanes. But: it bakes whatever pattern was authored (Son Montuno's clave is locked to E(5,16)={0,3,6,9,12}, **not** son clave {0,3,6,10,12}); it does **not** clear swing/humanize/timing/spread (Deep House kick keeps 2 ms humanize; six locked referents keep swing 0.15–0.45); and in 4 presets it locked a different lane than the plan named (Tintal/Rupak/Carnatic theka still prob 0.95/0.9; Samba tamborim 0.95; Compositional Arc bell unlocked; Reich anchor prob 0.8 → 0.64 post-macro). |
| **P1.2** preset musical fixes | 🔴 **~6 of 21 done** | Done: Kotekan Interlock (complement non-empty), Agbekor de-saturation, Jungle/Classic-Funk/Balkan-Funk syncopation-macro zeroing (partially), Minimal Techno density, Afro-House phrase gating, tigun renaming. Not done: Neo-Soul 4-on-floor kick; backbeat mutation in 5 presets; the whole minimalism set (Reich prob 0.8, Riley 0.9/0.85/0.8/0.75, Nancarrow non-identical material); Bossa "clave" tamborim is literal son clave with ginga micro-timing on steps that never sound; Samba surdo {1,3} with no beat-1/2 dialogue; Latin Feel 5-sixteenth "clave"; Afrobeat 12/8 four-on-floor kick now *allowlisted* in the lint rather than fixed; Afro-Electronic kotekan still sourced from the clave lane; Pocket Groove one-beat kick cycle; Elvin cross-rhythms ungated. |
| **P3** chapter prose | 🔴 **Majority not done** | Fixed: 01 cinquillo-rotation claim, 04 preset names, 06 layakari naming, 07 rotation-variant table, 08 mutation-as-process, 09 density-exemption note, 10 E(5,16) spelling, 13 "avoids beat 1", 14 idiom-break framing. Not done: ~25 items including ch. 05's inverted kotekan/gong/colotomy claims, ch. 03's E(5,16) defence and dead workflow (§3.1), ch. 06 emergent-sam, ch. 07 "literally Euclidean" + wrong kopanitsa, ch. 11 backbeat rotation-4 bug (still in both patches), ch. 15 anchor-first recipe, guide's uncaveated "start with Density and Swing". |
| **P4** appendices | 🟡 | P4.1 ✅ (0 hand tables; emitter exports all fields; spot-checks exact). P4.2 ✅ (68 preset refs resolve or carry the `Custom:` contract). P4.3 partial (generated rows all verify; the two prose defects — cinquillo-as-complement, unscoped rotation advice — untouched). P4.4 ✗ (E(7,16) ghost claim + "1155 steps" both survive, now visibly contradicting the generated table on the same page). P4.5 ✅. |
| **P5** idiom framing | 🔴 **5 of ~20** | A well-formed `:::note[Bending the idiom]` convention exists and is test-pinned — to exactly 5 findings. ~7 flagged bullets remain unframed, several now arithmetically impossible against the fixed presets ("hits from 7 to 9" on a 4-hit lane; drift "+2" where the preset ships 0.25; Kotekan "L5" in a 4-lane patch; "[3+2+3] rupak (7 beats)" summing to 8). |
| **P6** guardrails | 🟡 **Real but scoped short** | P6.1 iterates all presets and asserts *post-resolveMacros* backbeat survival — but only for one category; no four-on-floor-post-macro check; no referent-survives-macros check; no prob/mutation invariant on timekeeping lanes. P6.2 ✅. P6.3 layered (generated claims + chapter/theory guardrails + prose fixtures) but appendix-presets prose and its 3 diagrams are excluded — and all 3 diagrams currently contradict the presets they illustrate. P6.4 ✅ wired into CI + pre-push with an anti-rot wiring test (which itself isn't in the runner it guards). |
| **D2–D6** decisions | 🔴 **Not recorded on main** | The plan, review, and decisions exist only on this branch. Test comments cite "D026/D027" with no document behind them. |

---

## 3. Correctness defects to fix first (new, prioritized)

### 3.1 The E(5,16)/son-clave fork (ship-blocker class)

Three mutually inconsistent claims about the same pattern:

1. **Chapter 03's exact-clave workflow is a no-op.** `03-afro-cuban.mdx:90-91` says
   rotation 0 gives {0,3,6,10,13} and tells the reader to toggle 13→off, 12→on. The
   engine actually plays {0,3,6,9,12}, where 13 is already off and 12 already on —
   the instructions change nothing and the result is not son clave (correct edit:
   9→off, 10→on). The rumba variant at `:93` is wrong the same way.
2. **The theory page itself** (`theory-afro-cuban.mdx:61,65`) still carries the
   pre-Bjorklund "fifth hit, 13 vs 12" framing; under the new generator the divergence
   is the fourth hit, 9 vs 10 (or cite bossa = r10 explicitly).
3. **The shipped Cuban Son Montuno preset is locked to the wrong clave** — the generic
   lock baked E(5,16) r0 instead of a hand-authored {0,3,6,10,12} `fixedPattern` (which
   is exactly how the Bossa preset does it — except Bossa's "telecoteco" is *itself*
   literal son clave, the mirror-image error).

Chapter 10 has the correct r0 spelling — so the site currently teaches both.

### 3.2 The Ewe bell rotation fork

The canon test pins the bell as E(7,12) **r9** = {0,2,4,5,7,9,11}. But: every chapter,
theory page, diagram, and the Ewe Polymetric preset use **r0**; the generated Euclidean
appendix prints **r2**; and the Agbekor/Afrobeat presets hand-code the true r9 pattern.
Three different "standard bells" ship simultaneously, and `01-foundations.mdx:59`
asserts "rotation 0 is the standard orientation." This needs one decision applied
everywhere at once (the plan's P2.0 step 6 warned exactly this would happen).

### 3.3 Kotekan source-lane bug, in doc and preset

`14-synthesis.mdx:42` and the Afro-Electronic preset (`presets.cpp:2292`) both pair the
sangsih with **lane 0/1 (the 8-step clave)** instead of the 12-step polos — a complement
across differing step counts. Chapter 02:65 has the same class of error (7-step lane
kotekan'd against a 12-step bell).

### 3.4 Bridge/WASM parity bugs (user-visible in the shipping plugin)

- The VST3 bridge never emits preset `category` → **the category filter is empty in the
  real plugin** while working in the mock and the site's Try It modal.
- `fillEveryN` is never serialized plugin→JS → the Fill slider always renders 0 in the
  plugin and cannot reflect loaded state.
- WASM host lacks handlers for `manualFill`, `armCapture`, `resetCapture`,
  `setLaneName`, `lane.N.fillEveryN`, `lane.N.seedLock` → those controls are silently
  dead in Try It (some warn, some drop silently).
- `bridge.schema.json` (the versioned wire contract) is missing 5 shipped actions and
  the `seedLocked` lane field.

### 3.5 Remaining arithmetic errors in docs (selection; full lists in agent evidence)

Funk backbeat "rotation 4" (identity → beats 1/3) in both ch. 11 patches; ch. 12 "Ride
(steady)" is E(3,4) = three consecutive quarters + rest, and its prose swaps the snare
and kick cycle lengths; ch. 05 complement diagram needs rotation 1 (as drawn every
"complement" hit collides with polos); ch. 06 rupak diagram/patch at r0 (needs r3);
ch. 07 kopanitsa still E(4,11); ch. 08 "no n=12 family has 3 consecutive onsets"
(E(9,12) does); ch. 09 "the only Euclidean pattern where k=n".

---

## 4. Doc-vs-code consistency (beyond the theory scope)

**Factually wrong (fix):** parameter-ID formula `800 + lane*10` vs actual stride 12
(`appendix-parameters.mdx:11` — wrong IDs for every lane ≥ 1); capture buffer "2048"
vs 8192 (`appendix-plugin-architecture.mdx:170`); ch. 16 documents the retired Type-0
SMF writer (both shipping paths use Format-1 multi-track); testing appendix names a
deleted `pluginval-linux` leg, "four" pre-push gates (now seven), "8 RT files" (now 12),
and calls the Cubase harness "deferred" (it runs nightly); ch. 17's "NoteMapView panel"
(class deleted in M053); CHANGELOG's envelope-target list names six targets that don't
exist; `CLAUDE.md` cites `kStateVersion` (actual: `kCurrentStateVersion`), lists VSTGUI
in the stack (compiled out), and a 5-step pre-push gate; taxonomy doc tells you to
commit a gitignored file.

**Convention drift:** six unannotated ownership-transfer `new` expressions in
`controller_base.cpp:108-113,182` — the repo's own scanner rule.

**Stale:** `ARCHITECTURE.md` predates the entire WebUI layer (the shipping editor);
`webui/README.md` still says "M028 experiment / Phase B is scaffolding"; dead
`BUILD_INTERACTION_TESTS` CMake option still force-reconfigured by the pre-push hook.

**Undocumented shipped features:** Groove Import / reverse-Euclid fitting (zero
mentions anywhere — the single largest doc gap, and it's user-facing: drop a `.mid`
on a lane); Fill (`fillEveryNBars` + Fill Now, empty description cell); Seed
dice/history/per-lane lock (empty description cell); the convergence meter; the
emission-classification layer; `anchorSteps`/`backbeatProtect`/density
constraints/global envelopes; state versions 17–18 and M053/M034/fitter absent from
CHANGELOG. Chapter 18 documents 3 of the 5 editors that exist (accent and envelope
editors omitted — while a theory page points readers to ch. 18 for the accent editor).

**Checked clean (notable):** all parameter ranges/defaults/IDs otherwise exact; all
timing-model formulas verified against the engine; engine isolation claim enforced by
CI; README build instructions valid; counts.json, midi-mapping, website-architecture
all accurate.

---

## 5. Quality assessment & refactoring priorities

**Strengths:** 535-line engine core decomposed into 13 single-purpose helpers; clean
layer separation with a CI job that fails if the engine build fetches the VST3 SDK;
465 C++ tests + 128 site tests + 32 webui specs + fuzz + benchmarks, mapped in a
written testing strategy; zero TODO debt; SHA-pinned dependencies; guards with their
own behavior-proof tests; honest engineering notes throughout.

**Refactor, in order of value:**
1. **`engine/src/presets.cpp` (2,735 lines) → data.** 43 near-identical factories + a
   93-line switch + a count constant; four coordinated edit sites per new preset; each
   edit requires regenerating presets.json *and* the committed WASM. The emitter already
   proves the data-first direction — invert it (checked-in data → generated header).
2. **Kill the `mock-host.js` preset fork (1,411 lines).** All 43 presets hand-coded in
   JS, consumed by ~30 of 32 webui specs, with only count-level parity assertions. Feed
   the mock from `presets.json` (the discipline already applied to `bjorklund.ts`).
3. **Split `webui/ui.js` (2,166-line single IIFE)** into modules along its existing
   comment banners; same for `wasm-host.js` (1,620).
4. **CI hygiene (cheap):** add `concurrency` cancellation + `timeout-minutes` to
   `ci.yml`; a PR currently triggers ~5 independent full plugin builds with no
   cancellation of superseded runs.
5. **State-version ladder:** name the 4 remaining magic-number version gates; write the
   compat policy (§6).
6. Smaller: `processor.cpp` (1,263), `web_ui_view.cpp` (1,254), `PolyPreviewCard.astro`
   (732 — 45% of the component tree); duplicated choc pin across two CMakeLists; two
   different pinned jk-standards versions in one repo.

**Preset conformance lint hardening** (turns "green" into "conformant"): add
four-on-floor-survives-macros; referent-survives-macros (post-`resolveMacros`);
backbeat check for all categories, not just Jazz/Funk/Soul; probability/mutation
invariants on timekeeping lanes; make `lockReferentLane` clear swing/humanize (or
assert on it); numeric-plausibility guard on Experiment bullets (hits ≤ steps, lane
indices exist, macro ranges); bring appendix-presets prose + diagrams into the
arithmetic guardrail.

---

## 6. Market position

Closest neighbours: **Audiomodern Playbeat 4** (algorithmic/AI drum-pattern generation,
strongest randomization in class), **XLN XO** (sample-map-first with generative
"playground" as a secondary feature), **Stepic**, **SEQUND**, **ADSR Orbit**, and
**HY-RPE2/HY-ESG** (Euclidean/polymetric step sequencers), plus free/modular options.

**Poly's genuine differentiators — no competitor has any of these:**
1. **Idiom-grounded generation.** Everyone else ships math + randomness; Poly ships
   math + an explicit, cited theory of what makes eleven traditions *sound right*, with
   presets and engine constraints (timeline lock, backbeat protect, kotekan, additive
   cells, tihai-aware gating) keyed to it. The 43-chapter site is a moat: it's
   marketing, documentation, and pedagogy in one, with a WASM try-before-install.
2. **Determinism as a feature.** Same patch/seed/transport → identical output, enforced
   by golden tests; seed history and per-lane seed locks. Playbeat's appeal is dice;
   Poly's is *reproducible* dice — much better suited to DAW production workflows.
3. **Open source (GPLv3), free.** Playbeat/XO/Stepic are $49–$179.

**Weaknesses vs the field:** MIDI-only (XO/Playbeat bundle sounds; Poly needs routing
knowledge — the guide covers it, but it's onboarding friction); no AU shipped (Logic
users — a large share of the mac market — currently can't load it, while the manual
says they can); no CLAP; no Linux binary; single-developer bus factor; and the
theory-conformance story — the actual differentiator — is the area §2 shows is only
half-true today. **The strategic risk is shipping a musicology-branded instrument whose
factory presets a musicologist can fault.** Closing §3 is therefore product work, not
polish.

---

## 7. Road to v1.0.0

Recommended: cut **v0.2.0 now** to exercise the release machinery, then gate v1.0.0 on
the list below. (No release has ever been cut — the only tag doesn't match the
workflow's `v*.*.*` trigger, so the sign/notarize/publish path is untested.)

**Gate A — release machinery proven (v0.2.0):**
1. Tag and ship v0.2.0; verify signing/notarization/stapling fire (make the secret-gated
   steps **fail loudly** when secrets are absent, or explicitly label artifacts
   unsigned); decide Windows code-signing.
2. Reconcile the manual with reality: platforms, bundle name (`poly_plugin.vst3`), and
   either ship the AU (it already builds and passes `auval` — it needs only a release
   asset) or delete the AU/Logic promises. Shipping it is strongly preferred: Logic is
   the AU's entire reason to exist.

**Gate B — correctness of the differentiator:**
3. Fix §3.1–§3.3 (clave fork incl. Son Montuno preset, bell-rotation canon decision
   applied site+presets-wide, kotekan source lanes).
4. Finish or consciously descope the remaining P1.2 preset fixes and P3/P5 content
   fixes — with decisions recorded (merge the D-series onto main as an ADR).
5. Harden the conformance lint (§5) so CI green certifies the theory claims.

**Gate C — product completeness:**
6. Bridge parity (§3.4): categories + fillEveryN in the plugin bridge; WASM handlers or
   capability-gating for dead Try-It controls; schema updated.
7. Document Groove Import, Fill, Seed, the accent/envelope editors; fill the two empty
   parameter descriptions; CHANGELOG entries for M034/M053/fitter and state v17–18.
8. Doc-truth pass over §4's factually-wrong list; rewrite ARCHITECTURE.md (WebUI layer);
   update CLAUDE.md.

**Gate D — trust & compat:**
9. Written state/preset compatibility policy (18 live state versions, zero deprecations,
   no stated promise); the M068 migration entry is the template.
10. `NOTICE`/third-party license inventory (VST3 SDK GPL-compatibility statement, choc,
    AudioUnitSDK, fonts w/ OFL distribution requirement) — the rigor already applied to
    audio samples, applied to code.
11. Minimal multi-DAW QA: a manual smoke checklist (load, play, capture, export, state
    round-trip) for Live/Logic(AU)/Reaper/Bitwig per release; Cubase stays the
    automated deep path. Resolve open P1 bugs (#172 Cubase nightly, #142 TSAN, #89
    flaky e2e).
12. CI concurrency/timeouts; consider flipping `POLY_WARNINGS_FATAL` on and starting
    the 1,400-warning clang-tidy burndown (post-1.0 acceptable if recorded).

Post-1.0 (correctly deferred): CLAP, Linux, the groove-spec/SLM roadmap branch, engine
musicality issues #149–#158.
