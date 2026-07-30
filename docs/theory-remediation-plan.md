# Theory Conformance Remediation Plan

Companion to [`theory-conformance-review.md`](./theory-conformance-review.md) (same
branch), which catalogues ~120 verified discrepancies between the Theory Deep Dives
(PR #159) and the earlier chapters, examples, and factory presets. This document turns
those findings into an executable plan: the decisions to make first, seven phases of
work with concrete per-file tasks, ordering and dependencies, sizing, guardrails so the
site cannot drift again, acceptance criteria, and a risk register.

**How to read this.** Section 1 lists six gating decisions (D1–D6). **D1 is decided:
Bjorklund is canonical** and P2.0 contains the explicit engine-change instructions;
settle the remaining five before opening issues. Sections 2–8 are the phases (P0–P6); each task has a
size (S ≈ ≤half day, M ≈ a day, L ≈ multi-day) and references the review's finding IDs
(S1–S7 systemic causes, §2.x tradition tables, §4 authority defects). Section 9 maps
phases onto a suggested milestone/issue breakdown; sections 10–11 are acceptance criteria
and risks.

---

## 1. Decisions required before work starts

### D1. Which Euclidean convention is canonical? — **DECIDED: Bjorklund** *(gates P2, parts of P0/P1/P3)*

**Decision (2026-07-30): the engine switches to Bjorklund.** The site's
`EuclideanDiagram.astro:11-35` implementation is the normative reference; the engine's
Bresenham rule (`(i·k) mod n < k`) is retired.

Background: the two algorithms produce the same maximally-even pattern for any E(k,n) —
they differ only in which rotation each calls "rotation 0" (review S4). But the phase is
load-bearing: the scholarship the site cites (Toussaint), the Euclidean reference
appendix, and the rendered diagrams all use Bjorklund's phase, and under the engine's
phase the site's "E(5,8) is the cinquillo"-class claims are false in the shipped
instrument. Rationale for choosing Bjorklund over engine-canonical: Poly is pre-1.0 (the
compatibility argument protects users who barely exist yet, and this is the last cheap
moment to align), the pedagogy is the product (the instrument should play the pattern the
docs teach), and the migration cost is small because the change is a per-(k,n) phase
offset, not a new pattern (old saved states can be migrated losslessly by adjusting
stored rotations — see P2.0 step 4).

Reference phases under Bjorklund (rotation 0), for orientation:
E(3,8) = {0,3,6} (tresillo) · E(5,8) = {0,2,3,5,6} (cinquillo) ·
E(5,16) = {0,3,6,9,12} (bossa's necklace; the bossa figure {0,3,6,10,13} is rotation 10) ·
E(3,7) = {0,2,4} (2+2+3 rachenitsa; rupak's 3+2+2 is rotation 3) ·
E(4,9) = {0,2,4,6} (2+2+2+3 daichovo) · E(7,12) = {0,2,3,5,7,8,10} (the hard-coded Ewe
bell {0,2,4,5,7,9,11} is rotation 9, *not* rotation 0 — the theory-SSA construction step
needs updating accordingly in P0.8/P2.4).

Execution instructions are **P2.0** below. One generator, one rotation semantic,
mirrored exactly in the engine, the diagram component, and
`site/src/audio/preset-patterns.ts`. P2.0 runs **before** P1, so preset fixes are
authored once, under final semantics.

### D2. How do we lock a referent lane? *(gates P1)*

`timeline = true` does two things at once (`engine/include/poly/types.h:224-228`,
`engine/src/engine.cpp:83-86`, `engine/src/macro.cpp:20-21`): the lane plays its
hand-written `fixedPattern` array **and** becomes immune to all macros. There is no way
today to say "this Euclidean-parameterised lane is macro-immune."

- **Option A (recommended): bake the pattern at preset-build time.** In each preset
  factory, compute the referent's pattern with `euclidean()` into `fixedPattern`, set
  `timeline = true`. No engine change; matches how Agbekor/Bossa/Afrobeat/Balkan already
  lock their bells. Downside: the lane's steps/hits/rotation stop being live handles in
  the UI (which is exactly what the theory wants for a referent, but confirm the WebUI
  renders timeline lanes acceptably — see `webui/` preset-selector and lane editors).
- **Option B: add a `macroImmune` (or role-driven) flag** that `resolveMacros` honours
  independently of timeline mode, keeping the lane Euclidean-editable. Engine change +
  state/bridge surface change (`bridge_serialization`, `bridge.schema.json`,
  `emit_presets.cpp`), plus a `kStateVersion` bump if it serialises. More flexible,
  more churn.

Decide A vs B once; the P1.1 task list below is written to be valid under either.

### D3. Preset-change policy *(gates P1)*

Fixing shipped presets changes audible output for anyone who has used them, and will
churn golden determinism fixtures. Recommended: **fix in place, pre-1.0**, with one
deliberate golden-fixture regeneration commit per preset batch, each reviewed against the
intended musical change (not blind re-blessing). If any preset must stay bit-identical
for a release commitment, list it now — none is assumed here.

### D4. Generate the preset appendix from engine data *(gates P4)*

`site/src/generated/presets.json` is already emitted from the engine by
`poly_presets_emit` (see `site/scripts/generate-presets-json.mjs` — "engine-authored lane
data … instead of drifting hand-copies"). The hand-written tables in
`appendix-presets.mdx` are the last drifting hand-copy (review S6). Recommended: build a
`PresetTable.astro` component that renders a preset's lane table directly from
`presets.json` by preset name, and replace all 14 hand tables. Macros/experiments prose
stays hand-written; parameters become impossible to fake. (The emitter must then also
export the fields the tables show that it currently omits — swing, mutation, ghost floor,
timeline flag — extend `engine/tools/emit_presets.cpp` accordingly.)

### D5. Fix the authority first *(gates everything)*

The theory pages themselves have nine internal defects (review §4). Nothing should be
"conformed" to a page that contradicts itself. P0 lands before any content or preset PR
that cites the affected rules.

### D6. House style for deliberate idiom-breaking *(gates P5)*

~20 Experiment/ListenFor bullets instruct the reader to do something a theory page lists
under "What Breaks the Idiom" (review, class `idiom-breaking-suggestion`). Decide the
framing convention once — recommended: a standard Starlight aside, e.g.
`:::note[Leaving the idiom]` with a link to the relevant deep-dive rule — then apply it
mechanically. Bullets that can't be honestly framed that way get replaced with a
rule-conformant experiment.

---

## 2. P0 — Repair the theory pages (the authority)

All defects from review §4. One PR, one task per page. Size: **M** total.

| Task | File | Fix |
|---|---|---|
| P0.1 | `theory-afro-cuban.mdx:54` | Tumbao construction row: move to a 16-step lane (or rotate) so it hits bombo (pulse 3) and ponche (6) and avoids beat one, per its own Rule 3 / Constr. 2. |
| P0.2 | `theory-funk-soul.mdx:46-48` | Kick row: non-zero rotation + non-E(4,16) placement so it syncopates and dodges backbeat slots. Snare row: rotation 4 → **2**. Ghost row: rotation so gaps fall just *after* 2 and 4, not on them. Resolve the Rule 6 ("heavy Humanize = sloppiness") vs Rule 8 ("higher Humanize") tension with an explicit scope sentence. |
| P0.3 | `theory-brazilian.mdx:46` | Surdo row rotation 1 → **0** so hits land on the beats Constr. 1 names; state the beat-2 velocity weighting in the row. |
| P0.4 | `theory-balkan.mdx:49-56` | Replace lane 1's Euclidean params with the additive-cells/timeline spec its own text mandates ({0,2,4,7,9}); lane 4 rotation 2 → **4** (aligns 3 of 4 accents to cell heads). |
| P0.5 | `theory-jazz.mdx` | Reconcile Rule 3's "one to four attacks" with the patch's 4–5; differentiate lanes 2/4/5 swing per its own Rule 6 (0.05–0.1 deltas); add the per-lane ms offsets and form envelope its construction steps require. |
| P0.6 | `theory-gamelan.mdx:45-60` | Align Constr. 3's "5 hits" with the patch's 9 (pick one); supply the missing 4-step ketuk lane Constr. 1 promises; add the Rule-4 structural-overlap strokes the strict-`L5` patch lacks; fix the bell/shekere stratum poach vs its own Rule 2. |
| P0.7 | `theory-indian-classical.mdx:40` | Rewrite the tihai construction for the engine's *periodic* phrase gate (period = Length+Gap): either choose Length/Gap so the period divides the cycle, or state that the gate must be re-armed per phrase; use one unit (beats) consistently. |
| P0.8 | `theory-sub-saharan-africa.mdx:53-54` | Kidi rotation 1 → 2/5/8/11 (one collision, the sanctioned convergence); restate Rule 3 for the sogo case ("minimise beat coincidence; one crossing per cycle is structural") since full offbeat-ness is arithmetically unsatisfiable for 3-vs-4 in 12. |
| P0.9 | `theory-afrobeat.mdx:44-48` | De-conflict bell/shekere strata per its own Rule 2; add the Spread column Constr. 1 references; move the hat's 5% mutation to the colour voices per Rule 7. |

**Acceptance:** every theory-page construction patch passes its own numbered rules under
the D1 convention, verified by the P6.3 doc-arithmetic test.

---

## 3. P1 — Engine and preset fixes

### P1.1 Lock every referent (S1) — size **M** (mechanical once D2 is decided)

Apply the D2 mechanism + `probability = 1.0f`, `mutationRate = 0`, no phrase gating, to:

| Preset (`engine/src/presets.cpp`) | Referent lane |
|---|---|
| Cuban Son Montuno (`:1053`) | clave — and replace E(5,16) with the exact son clave pattern {0,3,6,10,12} (P1.2f) |
| Ewe Polymetric Ensemble (`:963`) | bell |
| Manding Djembe (`:1009`) | dunun timeline |
| Tintal Groove (`:1309`), Rupak Tal (`:1360s`), Carnatic Tala | theka (drop 0.95/0.9 probability) |
| Samba Batucada (`:1770s`) | tamborim telecoteco (drop 0.95 probability) |
| Bossa Nova Trio | tamborim already locked — verify ride/brush continuity fix in P1.2j |
| All House/Techno presets (Four on the Floor, Minimal Techno, Deep House, Afro-House Phrases) | anchor kick (and clap where present) |
| All Breaks/D&B presets (Breakbeat, Jungle Break, Liquid DnB) | backbeat snare |
| Classic Funk, Neo-Soul Pocket, Pocket Groove | backbeat snare (The One's kick stroke also protected from Syncopation — see P1.3) |
| Elvin Jones Cascade, Jazz Ride patch preset | ride lane |
| Reich Phasing (`:325`) | anchor pulse (drop 0.8 probability) |
| Compositional Arc (`:2350`) | "Bell timeline" lane |
| Synthesis presets (Afro-Electronic `:2219`, Balkan Funk `:2280s`) | their declared anchors |

### P1.2 Fix broken/non-conformant presets — size **L**

Each item is one reviewable change with a one-line musical intent:

a. **Kotekan Interlock** (`:365-420`): polos `{3,8}`/3 saturates → sangsih complement is
   silent (S5). Rebuild as a real pair — e.g. polos 8 steps E(5,8), sangsih kotekan-derived
   from it (complement is rot-3 under Bjorklund; verify under D1 winner), gong 4/1,
   restore the interlock its description claims. Align with the appendix table or update
   the table via P4.
b. **Sub-Saharan: Agbekor** (`:820-870`): kidi `{5,12}`/5 and sogo `{3,12}`/3 saturate
   into identical streams (S5). Give both 12-step cycles at the bell's subdivision with
   offbeat rotations per theory Constr. 3 (kidi rot 2, sogo minimised-collision rot).
c. **Jungle Break** (`:2100-2155`): remove/neutralise `macros.syncopation = 0.5` (it
   rotates the backbeat to beats 1/3); snare mutation 0.10 → 0 (Rule 6); keep instability
   in the ghost-snare layer.
d. **Classic Funk** (`:1870-1920`): `syncopation = 0.3` currently removes the One and the
   backbeat — restructure so the kick strikes the One natively and macros can't move the
   protected lanes; ghost lane MIDI note 37 → 38 (same drum as accent snare, Constr. 3).
e. **Minimal Techno** (`:1650-1700`): `density = 0.4` thins the four-on-floor kick;
   raise to ≥0.5 or rely on the locked anchor from P1.1.
f. **Cuban Son Montuno** (`:1050-1110`): exact clave {0,3,6,10,12} as fixed pattern;
   tumbao re-voiced onto a 16-pulse grid hitting bombo+ponche, no beat one; halve the
   swing stack (lane 0.2–0.3 **or** macro, not both — target effective 0.2–0.3).
g. **Afrobeat 12/8** (`:477-540`) and **Afrobeat Lagos** (`:1142`): kick 4-on-floor →
   3–4 hits/16 dodging beat one (Lagos: add rotation 3); snare velocity 95→~60 with
   ghost floor; shaker 12/12 → sparse offbeat pattern owning its stratum; drop
   `macros.swing = 0.1` (12-pulse frame takes no binary swing).
h. **Samba Batucada** (`:1750-1820`): surdo rotation 1 → 0 with beat-2 velocity
   weighting (accent mask or two-lane surdo pair); caixa probability 0.9 → 1.0;
   `macros.humanize` 0.25 → ≤0.2.
i. **Bossa Nova** (`:595-690`): surdo pattern beat-2-weighted (currently equal-velocity
   {0,2} — the "march" error); rename/re-voice the "clave" tamborim to the telecoteco
   contratempo pattern (it is literally son clave today); reshape `microTimingMs` from
   pair-shaped odd-step delays to the position-shaped LSSL profile (4th sixteenth early,
   not late).
j. **Bossa Nova Trio** (`:1826-1860`): ride/brush probability 0.9/0.8 → 1.0 (bossa keeps
   the continuity stratum); ride swing 0.10 → 0.15+.
k. **Minimalism set** — Reich Phasing (`:325`): anchor probability 0.8 → 1.0, spread → 0;
   Reich Phase Process (`:1495-1520`): velocities 90/85 → equal; Riley Layers
   (`:1540-1595`): probabilities 0.9/0.85/0.8/0.75 → 1.0 (layering comes from phrase
   gates, not dropout); Nancarrow Tempi (`:1600-1645`): probabilities → 1.0, identical
   material across canon voices, flat dynamics.
l. **Elvin Jones Cascade** (`:2035-2092`): ride hitCount 3 → 4, mutation 0.10 → 0;
   phrase-gate the 3/5/7 superimposition lanes to resolve at a form boundary (Constr. 6);
   bass drum velocity 85 → ~55-65 (below ride/hat); Jazz Ride preset kick 80 → ~55.
m. **Neo-Soul Pocket** (`:1925-1975`): kick E(4,16) → syncopated 3–5-hit shape dodging
   backbeat slots; snare probability 0.95 → 1.0, velocity 90 → 105+; humanize macro 0.4
   scoped away from the (now locked) backbeat.
n. **Pocket Groove** (`:420-470`): snare mutation 0.1 → 0; kick `{4,16}` one-beat cycle →
   a bar-length phrase (matches its own doc table 8/3 at 1/8, review S6).
o. **Tintal Groove / Rupak Tal** (`:1300-1400`): khali contour — accent-mask or velocity
   drop on the bayan lane's step 8 region (currently full 110 velocity on khali);
   rename/re-rate the "tigun" lane (currently 4× = chaugun): either 1/12 subdivision
   (true tigun) or rename; make theka and sam lanes agree on cycle span (theka currently
   states the tala twice per sam cycle).
p. **Four on the Floor** (`:5-53`): move the polymetric `{7,8}` lane off the open-hat
   note 46 (slot-owning timbre) onto neutral percussion, per Electronic Rule 2/3.
q. **Deep House** (`:1700-1750`): kick `humanizeMs = 2` → 0; macro swing 0.35/humanize
   0.1 are fine once the anchor is locked (P1.1) — verify resolved values post-lock.
r. **Liquid DnB** (`:2159-2211`): `density = 0.35` → ≥0.5 or rely on locked anchor/backbeat;
   snare mutation 0.05 → 0.
s. **Afro-House Phrases** (`:253-321`): anchored kick locked (P1.1); then swing
   0.15/humanize 0.15/density 0.45 macros act only on the phrase lanes, which is the
   design intent.
t. **Synthesis presets**: Afro-Electronic (`:2219-2276`) — kotekan source lane 0 (the
   8-step clave) → the 12-step polos lane; techno kick locked so `swing = 0.15` doesn't
   reach it; decide whether the "Cuban clave" lane should be a real 16-pulse clave or be
   relabelled tresillo (terminology, S7-adjacent). Balkan Funk (`:2280-2330`) —
   `syncopation = 0.3` displaces the aksak cell heads: lock the kick's cell-head lane;
   either remove the hi-hat's 0.15 swing or re-file the preset as Experimental/Fusion in
   `docs/preset-taxonomy.md` (currently filed as Balkan, which denies the fusion framing
   — pick one, review §2.6).
u. **Latin Feel** (`:200-250`): the lane labelled "clave" is E(3,5) on a 5-sixteenth
   cycle phasing against the bar. Either make it a bar-locked tresillo (rename label) or
   an exact clave; today it is neither.

### P1.3 Macro/emphasis semantics review — size **M**, optional but recommended

Syncopation's blanket `rotation += round(m.syncopation * steps/2)` is what un-anchors
funk/jungle (P1.2c/d). With referents locked (P1.1) the worst cases disappear, but
consider a `backbeatProtect`-style constraint (a field the review notes already exists in
`constraints`) or per-role syncopation weighting so the macro stays musical on the
remaining lanes. Keep any engine change behind golden-test review (D3).

### P1.4 Regenerate downstream artefacts — size **S** per batch

After each P1 batch: rebuild `poly_presets_emit` output → `site/src/generated/presets.json`
and `site/public/webui/presets.json` (the generator does both); update
`tests/preset_tests.cpp` expectations; regenerate golden determinism fixtures
deliberately (D3); check `webui/tests/fixtures/*` and `site/tests-e2e/preset-consistency`
still pass.

---

## 4. P2 — One Euclidean convention everywhere (D1: Bjorklund)

### P2.0 Switch the engine generator to Bjorklund — size **M**, runs before P1

Explicit instructions; the normative reference is the `bjorklund()` function in
`site/src/components/EuclideanDiagram.astro:11-35`.

1. **Replace the generator body** in `engine/src/euclidean.cpp`. Keep the public
   signature `euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>&)` and
   the existing guards unchanged (k≤0 or n≤0 → all false; n clamped to `kMaxSteps` = 64;
   k ≥ n → all true — the reference component has the same k≥n branch). Replace the
   Bresenham loop with Bjorklund's pairing/elimination algorithm: start with k groups
   `[1]` and n−k groups `[0]`; while more than one remainder group exists, append one
   remainder group to each of the first min(|pattern|, |remainder|) pattern groups, the
   longer list's tail becoming the new remainder; concatenate. Engine output at
   rotation 0 must equal the Astro function's output element-for-element for every
   0 < k < n ≤ 64.
   - **RT safety** (this runs on the audio path — `engine.cpp:99,106` — and must pass
     `scripts/check-realtime-safety.sh`): no heap, no exceptions. Do not port the JS
     array-of-arrays literally. Represent groups as counts/indices in fixed
     `std::array<..., kMaxSteps>` buffers, or use bounded recursion (Euclid's algorithm
     depth ≤ log₂ 64) with stack storage only.
   - **Rotation semantics stay as-is**: apply the existing right-shift *after* generating
     the base pattern (`out[i] = base[((i − rotation) % n + n) % n]`). Verified: the
     Astro component's slice-based rotation is the same direction (onset at index j moves
     to (j + r) mod n), so no doc or UI rotation value changes meaning beyond the phase
     shift itself.
   - The `// region:bjorklund` snippet marker in this file — currently a lie wrapping
     Bresenham code — becomes truthful; keep it, update the interior comment, and confirm
     `scripts/check-snippet-regions.sh` still passes (the region is embedded in site
     docs via CodeSnippet).
2. **Equality test (the contract):** add an engine test that reimplements the reference
   algorithm in the most literal form (tests may allocate) and asserts engine ==
   reference for **all** 1 ≤ k ≤ n ≤ 64 and every rotation 0…n−1. This is what makes
   "diagrams show what the engine plays" a checked invariant rather than a hope.
3. **Canon test (the literature):** pin Toussaint-known spellings at rotation 0 so a
   future "optimisation" cannot silently reintroduce a phase shift:
   E(3,8) = {0,3,6}; E(5,8) = {0,2,3,5,6}; E(5,16) = {0,3,6,9,12}; E(3,7) = {0,2,4};
   E(4,9) = {0,2,4,6}; E(7,12) = {0,2,3,5,7,8,10}. Also pin the named rotations the
   docs rely on: bossa = E(5,16) r10; rupak = E(3,7) r3; Ewe bell = E(7,12) r9.
4. **Saved-state migration (lossless):** bump the state version (per the CLAUDE.md
   serialization convention: version int first, branch in `setState`) and, when loading
   a pre-switch state, for each non-timeline lane compute the phase delta
   δ(k,n) = the shift for which `bjorklund(k,n) >> δ == bresenham(k,n)` (generate both,
   find δ by scan — n ≤ 64, trivial at load time; keep a private `legacyBresenham()`
   used only by the migration path), then store `rotation ← (rotation + δ) mod n`. Old
   projects then sound **identical** under the new generator. Mirror the same migration
   in the WebUI bridge path if states round-trip through
   `plugin/source/webui/bridge_serialization.*`. Presets need no migration — their
   rotations are re-authored in P1 under the new semantics.
5. **Rebuild downstream artefacts:** golden determinism fixtures (one dedicated,
   reviewed regeneration commit — D3); `presets.json` + `site/public/webui/presets.json`
   via `generate-presets-json.mjs`; the WASM engine build consumed by the Try-It modal
   (`webui/poly_engine.js`); re-run `site/tests-e2e/preset-consistency` and
   `webui/tests/*` fixtures. CHANGELOG entry stating the phase change and the migration
   guarantee.
6. **Caveat for the later sweeps:** the review's pattern arithmetic (§2 tables) was
   computed under the *engine's* old phase. After P2.0 those derivations flip in places —
   e.g. ch. 06:82's E(3,7) spelling becomes correct as written while its rupak
   attribution still needs r3, and theory-SSA's "E(7,12), rotation 0" construction step
   no longer matches the hard-coded bell (r9). P2.3/P2.4 and the P0 fixes must re-derive
   every value against Bjorklund — do not transcribe the review's onset lists.

| Task | Size | Detail |
|---|---|---|
| P2.1 | S | `EuclideanDiagram.astro` needs no algorithm change (it is the reference). Extract its `bjorklund()` into a small shared module (e.g. `site/src/audio/bjorklund.ts`) imported by both the component and `preset-patterns.ts`, so the site cannot fork internally; add a site test pinning the same canon table as P2.0 step 3. |
| P2.2 | S | Align `site/src/audio/preset-patterns.ts` (Try-It playback) with the shared module from P2.1 — one generator on the site, byte-equal to the engine's. |
| P2.3 | L | Sweep every printed pattern: all `x . x` spellings, interval strings, onset lists, and `<EuclideanDiagram>` props in chapters 01–15 and both appendices; re-derive from Bjorklund (per P2.0 step 6, not from the review's tables). Known-suspect list to start from: bell spellings (02:28), E(3,7)/E(4,9) rachenitsa/daichovo (07:33-45), E(5,16) spellings (03:90, 10:51-53), E(5,8)/E(3,8) complement demo (05:29-31), E(3,7) rupak spelling (06:82), E(9,12) claim (08:31). |
| P2.4 | M | Rotation-value sweep (S3): every patch-table rotation checked against the placement the prose claims, under Bjorklund + right-shift semantics. Known-suspect list (target values to be re-derived post-P2.0): funk snare backbeat rotations (ch. 11 ×2 and theory-funk patch), samba surdo, SSA support offsets and the bell's r9, afrobeat kick, electronic open-hat "&"s, D&B "avoids beat 1" prose (13:50), balkan cell-head alignments. |

---

## 5. P3 — Chapter prose corrections (contradictions)

One PR per chapter; each bullet is a prose rewrite bringing the claim in line with the
theory page (or an explicit, cited "the deep dive refines this" pointer). Size: **S–M**
per chapter.

- **Ch. 01 Foundations**: rotation≠cinquillo (`:53`); 84-steps convergence → 42 quarters
  / 10.5 bars (`:68`, match the timing appendix); "independent cycle lengths, exactly as
  in West Africa/gamelan" → reframe (those traditions share/nest cycles; free polymetry
  is Poly's generalisation) (`:45`); cinquillo-as-complement → superset (`:35`).
- **Ch. 02 Sub-Saharan**: bell spelling (P2.3); support-drums-different-cycles claim
  (`:42`) → supports share the timeline cycle, the *lead* stacks; lane-4 mutation 15–25%
  → 0–10% support / move budget to lead (`:65`); phrase-gate advice `:93` → cycle-multiple
  lengths (6/12 beats) and one-cycle gaps.
- **Ch. 03 Afro-Cuban**: rewrite the E(5,16) defence (`:86`) to teach the exact-clave
  timeline workflow as primary; fix 3-side/2-side rotation claims (`:66,70`); Syncopation
  macro claim (`:115`); quinto-less mutation advice (`:119,125`) → add a quinto lane or
  scope mutation to it.
- **Ch. 04 Afrobeat**: hat range 12–14 → 13–14 (`:37`); phrase-offset narrative in beats
  (`:72`); preset name (S7).
- **Ch. 05 Gamelan**: the big one — kotekan defined by empty intersection (`:23-25`),
  "neither part makes sense alone" (`:25,54`), gong as departure (`:93`), 4:8:16:64
  ratios (`:63-66`), "(or other integer multiples)" (`:70`), "never quite repeating"
  (`:95`), "ketuk strikes every beat" (`:85`). Rewrite the section around the theory
  page's Rules 1/2/4/7/8.
- **Ch. 06 Indian**: E(4,7)-rotation-to-3+2+2 claim (`:27`); E(3,7) spelling (`:82`);
  khali contour on the patch (`:51,60`); theka/sam cycle-span mismatch (`:52-53,72-74`);
  tigun/chaugun label (`:54`); emergent-sam framing (`:86-93`) → sam as fixed target.
- **Ch. 07 Balkan**: "literally Euclidean" framing (`:23,27,49`) → cells are ground
  truth, Euclidean often matches; kopanitsa section (`:45-49`) → 2+2+3+2+2 with the
  additive-cells workflow; rotation-variant table (`:33,37`) → correct rotations;
  continuous-pulse lanes in both patches.
- **Ch. 08 Minimalism**: Mutation-as-process (`:89`) → Complexity/Density unit steps;
  stacked processes praise (`:120`) → one process, framed as deliberate if kept; "drifts
  in and out of phase" hemiola (`:116`) → period-locked cross-rhythm; velocity-flat
  phasing pairs (`:47-48`); Nancarrow patch → identical material (`:99-108`); n=12
  consecutive-onsets claim (`:31`).
- **Ch. 10 Brazilian**: surdo patch + rotation prose (`:29,35`); humanize/swing ranges
  (`:45,85,87`); agogô-vs-tamborim timeline roles (`:23`); E(5,16) spelling (`:51-53,71`).
- **Ch. 11 Funk**: ghost-carpet definition (`:23,50`) → accent-directed phrases; backbeat
  rotation (`:42`); Deep Pocket patch kick/snare rows (`:68-69`); 5–10 ms late snare and
  every-lane Humanize (`:60,62`) → backbeat exempt; wandering rim-click "defines the
  genre" (`:77`) → feel-mutation framing.
- **Ch. 12 Jazz**: "zero or very low mutation" → zero on ride (`:89`); 420-step
  superimposition and LCM-convergence ListenFor (`:62,79`) → phrase-gated loans repaid at
  form boundaries (or explicit modern-free-practice framing); swing-flattening threshold
  200→~250 BPM (`:23`); kick velocities; missing hi-hat lane note (`:64-73`); ungated
  rim-click advice (`:93`).
- **Ch. 13 D&B**: backbeat mutation in both patches (`:44,69,75`); "avoids beat 1" prose
  (`:50`); liquid "swing more pronounced" (`:62`); neurofunk macro recipe (`:87`) →
  bar-two-scoped mutation, syncopation ceiling.
- **Ch. 14 Synthesis**: tresillo-as-"clave backbone" labelling (`:29,35`); kotekan lane
  pairing and hit counts (`:37-38`); swing-on-kotekan and swing-on-aksak passages
  (`:37-38,49,61,68`) → these are the overview's own non-interchangeability examples, so
  either remove the swing or make the idiom-break framing explicit and consistent with
  `:147`.
- **Ch. 15 Compositional Grammar**: recipe order (`:58-62`) → referent first, locked,
  kick relative to it; timeline "mutation low" → zero; Density-sweep-thins-the-bell
  passage (`:44,81`) → celebrate sweeps over *non-referent* lanes; foreign-cycle ornament
  advice (`:62,66`) → scope to traditions that allow it.
- **Guide** (`guide-using-poly.mdx`): "start with Density and Swing" (`:140`) → add
  per-tradition caveat or point at the deep dives; recipe walkthrough (`:371-373`) →
  include mode/mutation/swing columns in "set the lane parameters".

---

## 6. P4 — Appendix reconciliation

| Task | Size | Detail |
|---|---|---|
| P4.1 | M | Implement D4: `PresetTable.astro` rendering from `presets.json`; extend `emit_presets.cpp` to export swing/mutation/ghost/timeline fields; replace the 14 hand tables in `appendix-presets.mdx`. Macros lines become generated too (they're in `GrooveState.macros`). |
| P4.2 | S | Fix nonexistent preset names (S7): `04-afrobeat.mdx:51`, `11-funk-soul.mdx:37`, `07-balkan.mdx:51,70`, and the ch. 02 badge/table mismatch (`:52` — point badge and table at the same preset). Add a P6 check so names must resolve. |
| P4.3 | M | `appendix-euclidean-reference.mdx`: E(4,11) "kopanitsa" row and rotation-variants prose (`:97,103`) → correct attribution (E(4,11) is *a* Bulgarian-adjacent grouping, not kopanitsa; point at the additive-cells workflow); cinquillo "complement" prose (`:76`); "experiment with all rotations" advice (`:145`) → scoped away from timeline/clave lanes; re-derive all pattern rows under D1 (P2.3 covers the spellings). |
| P4.4 | S | `appendix-presets.mdx` narrative fixes that survive P4.1: "polymetric ghost at E(7,16)" (`:19`) → describe the real `{7,8}` lane; Polymetric Drift "1155 steps" → 1155 quarter notes (`:41`); Afrobeat scene-morph and gamelan L3/L5 experiments move to P5. |
| P4.5 | S | `docs/preset-taxonomy.md`: re-file Balkan Funk per P1.2t decision; sweep category assignments against which theory page governs each preset (the review used the taxonomy as the authority-selector). |

---

## 7. P5 — Experiments and suggestions (idiom-break framing, D6)

Apply the D6 convention to every `idiom-breaking-suggestion` finding. By file:

- `appendix-presets.mdx`: `:35` (swing 0.5 on 4-on-floor), `:102` (rotate kick through
  snare slots — add "check the snare slots" guidance), `:126-128` (conga mutation, E(5,16)
  "full son clave" — replace outright, it's factually wrong, plus swing 0.7), `:151`
  (density 0.3 build — note the locked anchor is exempt post-P1), `:172-174` (drift +2,
  humanize 0.3, pitched-pair split), `:194,196` (kotekan L3/L5 — fix L5 reference,
  frame L3), `:216,218` (humanize 0.5, ghost-layer churn), `:241` (afrobeat→funk morph),
  `:262-263` (zurna humanize, foreign-cycle lane), `:305,307` ([3+2+3] rupak — fix the
  arithmetic outright; Complexity-as-layakari — replace, the macro can't do it).
- Chapters: 09:52,73,81 (swing sweep, density sweep, mutation-for-interest), 12:93
  (ungated rim-click), 13:87 (neurofunk recipe), 02:65 (support mutation), 03:119,125
  (marcha/bass mutation), 14's swing-on-borrowed-idiom passages.

Size: **M** once D6 exists.

---

## 8. P6 — Guardrails (make drift impossible)

| Task | Size | Detail |
|---|---|---|
| P6.1 | M | **Preset conformance lint** (new `tests/preset_conformance_tests.cpp` or extend `preset_tests.cpp`): for every factory preset, assert (a) exactly one referent-role lane is locked per D2 with probability 1.0 / mutation 0 / no phrase gate; (b) no lane has `hitCount >= cycle.steps` unless annotated intentional; (c) kotekan-derived lanes produce a non-empty pattern; (d) category-scoped rules — no swing/humanize macros on Balkan/Gamelan/Indian/Sub-Saharan-category presets, backbeat lanes resolve to beats 2/4 *after* `resolveMacros` with the preset's own macro state, four-on-floor kicks survive their preset's macros intact. The post-macro assertions are the key novelty: they catch the Jungle Break class of bug where defaults undo the genre. |
| P6.2 | S | **Doc/preset name resolution test** (site): every `preset="…"` attribute in MDX must match a `presets.json` name (extend `site/tests/`). |
| P6.3 | M | **Doc arithmetic test**: extract `<EuclideanDiagram>` props and adjacent pattern spellings / `steps-hits-rotation` table rows from MDX and verify spellings against the canonical generator (a Node port already exists once P2.2 lands). Start with the theory pages' construction patches (protects P0), expand to chapters. |
| P6.4 | S | Wire P6.1–P6.3 into CI and the pre-push hook (`scripts/pre-push-check.sh` already runs build+tests; site tests run in the site job). |

---

## 9. Suggested sequencing and milestone breakdown

```
D2–D6 (decisions; D1 = Bjorklund, decided)
        │
        ├──►  P2.0 (engine → Bjorklund) ──►  P1 (presets, authored under final semantics) ── P1.4
        │              │                                                                      │
        └──►  P0 (authority — re-derive patterns post-P2.0)                                   │
                       │                                                                      ▼
                       └────────►  P2.1–P2.4 / P3 / P4 (content)  ◄── (P4.1 needs final presets)
P5 after P3/P4 text stabilises.  P6.1 lands with P1; P6.2/P6.3 land with P4/P2; P6.4 last.
```

- **Milestone A — "One generator"**: remaining decisions D2–D6 recorded (ADR-style note
  in `docs/`), then **P2.0** with its equality/canon tests and state migration. Small,
  self-contained, unblocks everything — do this first.
- **Milestone B — "Authority repaired"**: P0 complete, with all patterns re-derived
  under Bjorklund. ~1 week of part-time work.
- **Milestone C — "Presets conform"**: P1.1–P1.4 + P6.1. The largest engineering chunk;
  batch PRs by tradition (afrobeat+SSA, latin, minimalism, electronic+dnb, funk+jazz,
  indian+balkan+gamelan, synthesis).
- **Milestone D — "Content conforms"**: P2.1–P2.4, P3 (per-chapter PRs), P4, P5, P6.2,
  P6.3, P6.4.

Rough total: ~35–45 issue-sized tasks. The per-tradition P1/P3 pairs are independent of
each other and parallelise cleanly.

## 10. Acceptance criteria (definition of done)

1. Every theory-page construction patch passes its own rules (P6.3 green on theory pages).
2. Every factory preset passes the conformance lint **including post-macro assertions**
   with its shipped macro state (P6.1 green).
3. `appendix-presets.mdx` contains zero hand-written parameter tables (P4.1).
4. No MDX references a nonexistent preset name (P6.2 green).
5. Site diagrams, Try-It playback, and engine output agree on every E(k,n) — enforced
   exhaustively for all 1 ≤ k ≤ n ≤ 64 × all rotations by the P2.0 equality test, plus
   the pinned Toussaint canon table (tresillo/cinquillo/bossa/etc.) at rotation 0.
6. Zero remaining `contradiction`-class findings from the review; every remaining
   `idiom-breaking-suggestion` carries the D6 framing.
7. Golden determinism tests pass with deliberately-reviewed new fixtures (D3).

## 11. Risk register

| Risk | Impact | Mitigation |
|---|---|---|
| Preset fixes change audible output for existing users | User-facing surprise, golden churn | D3 policy: in-place pre-1.0, one reviewed fixture-regen commit per batch; CHANGELOG entries per preset |
| P2.0 phase switch mis-migrates saved states | Old projects change sound after update | Lossless δ-rotation migration (P2.0 step 4) + a round-trip test: load a pre-switch fixture state, assert identical NoteEvent output |
| Doc sweeps reuse the review's engine-phase arithmetic | Wrong "corrections" written under the retired convention | P2.0 step 6 caveat; P6.3 doc-arithmetic test validates against the live generator, so stale derivations fail CI |
| `timeline` lock hides lanes from WebUI editing | UX regression in Try-It modal and plugin UI | Verify WebUI rendering of timeline lanes before P1.1 batch 1; if poor, choose D2 Option B |
| Emitter field additions ripple through bridge schema | `bridge.schema.json`, wasm host, webui tests | P4.1 scoped to the *emitter* JSON only (site-side), not the live bridge; keep the two schemas separate |
| Post-macro lint (P6.1) is order-dependent on macro semantics | False confidence if `resolveMacros` changes later | Lint calls the real `resolveMacros`, not a re-implementation — it can't drift |
| Theory pages change again (new PRs) | Plan staleness | P6.3 covers theory pages too; re-run the review's method (12 parallel audits) after any theory-page rewrite |
| Chapter rewrites drift in tone from the guide's voice | Editorial churn | Keep P3 PRs per-chapter and small; prose fixes state the rule and cite the deep dive rather than re-deriving it |

---

*Prepared from the findings in `theory-conformance-review.md`. Engine facts cited here
(timeline/fixedPattern semantics, macro resolver behaviour, emitter pipeline) were
verified against `engine/include/poly/types.h`, `engine/src/engine.cpp`,
`engine/src/macro.cpp`, `engine/src/euclidean.cpp`, and
`site/scripts/generate-presets-json.mjs` at the commit this branch is based on.*
