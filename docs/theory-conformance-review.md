# Theory Conformance Review — Counterpoint Deep Dives vs. Chapters, Examples, and Presets

**Scope.** The Theory Deep Dives (PR #159, commit `377d062`) state explicit, numbered
counterpoint rules per tradition. This review treats those pages (plus
`theory-counterpoint-overview.mdx`) as the authority and audits everything older against
them: chapters 01–15, the user guide, the Euclidean/timing/preset appendices, and the
factory presets in `engine/src/presets.cpp` / `site/src/generated/presets.json` /
`docs/preset-taxonomy.md`. Every finding below was verified against the exact lines cited,
and all pattern arithmetic was recomputed with the engine's actual generator
(`engine/src/euclidean.cpp`) and macro resolver (`engine/src/macro.cpp`).

**Classification key.**
- `contradiction` — prose states the opposite of an explicit theory rule or factual claim.
- `rule-violation` — an example patch or shipped preset whose parameter values break a numbered rule.
- `idiom-breaking-suggestion` — an Experiment/ListenFor instructs the reader to do something the theory page lists under "What Breaks the Idiom", without framing it as a deliberate departure.
- `terminology` — naming/labelling conflict (wrong pattern name, nonexistent preset name, unit confusion).

Omissions are not findings (chapters are intentionally "broad and shallow" per the
overview). External real-world accuracy was not the test — conformance to the theory
section was.

---

## 1. Systemic findings (root causes affecting many traditions)

These seven mechanisms generate most of the individual findings. Remedial planning should
start here: fixing one systemic cause clears dozens of line-item violations at once.

### S1. Referent lanes are not locked (`timeline` flag absent)

The overview (§1, §3) and every tradition's Rule 1 require the referent locked: timeline
mode, zero mutation, probability 1.0, no phrase gating. Only a handful of presets set
`lane.timeline = true` (Afrobeat 12/8 bell at `presets.cpp:491`, Bossa Nova tamborim at
`:624`, Agbekor bell at `:837`, Balkan Aksak at `:1127`). Everything else leaves the
referent as an ordinary macro-responsive lane:

- Cuban Son Montuno clave (`presets.cpp:1053-1061`) — E(5,16), un-locked, no `timeline`.
- Ewe Polymetric Ensemble bell (`presets.cpp:963-971`) — un-locked, despite chapter 02:34 telling readers to lock bells.
- Manding Djembe (`presets.cpp:1009-1045`) — no lane locked at all.
- Tintal Groove / Rupak Tal / Carnatic Tala — no theka lane locked; theka lanes carry probability 0.95/0.9 (`presets.cpp:1326,1375`).
- Samba Batucada tamborim — probability 0.95 (`presets.cpp:1776`) on the "never mutates" telecoteco stratum.
- Every House/Techno/D&B preset — no anchor kick or backbeat snare is `timeline`-flagged.
- Compositional Arc "Bell timeline" lane (`presets.cpp:2350-2356`) — un-locked, and chapter 15:81 celebrates the resulting Density thinning of the timeline.

### S2. The macro system reaches referents, and the docs tell readers to sweep macros

`resolveMacros` (`engine/src/macro.cpp`) skips only `timeline` lanes. For all others:
Swing is **additive** (`:89`, `base.swingAmount + m.swing`); Density < 0.5 halves
probability and pulls hitCount toward 1 (`:61-66`); Complexity re-hit-counts and rotates
(`:32-50`); Syncopation adds `round(m.syncopation * steps/2)` to rotation (`:79-80`);
Humanize adds up to 25 ms jitter (`:116`). Combined with S1, every documented macro sweep
or macro default violates an anchor rule:

- "Four on the Floor" documented macros (Swing 0.3, Humanize 0.15, `appendix-presets.mdx:32`) swing/humanize the E(4,4) kick — the verbatim "What Breaks the Idiom" item of theory-electronic-breakbeat Rule 1. The engine actually ships neutral macros (`presets.cpp:52-53`), so the docs invent the violation.
- "Push the Swing macro to 0.5 for a house feel" (`appendix-presets.mdx:35`; same advice `09-electronic.mdx:52`) swings the kick — theory Rule 4 requires kick/clap straight while only hats shuffle.
- Deep House preset ships `swing = 0.35` + `humanize = 0.1` macros (`presets.cpp:1746-1747`) → resolved kick swing 0.35, contradicting chapter 09:67's own "the kick stays unswung".
- Minimal Techno ships `density = 0.4` (`presets.cpp:1691`) → its "four-on-the-floor" kick resolves to E(3,4) at 0.9 probability, contradicting its own catalogue description (`:2574`).
- Classic Funk ships `syncopation = 0.3` (`presets.cpp:1916`) → kick rotates off the One entirely, snare rotates off 2-and-4. Rule 1 ("The One is sacred") and Rule 2 both broken by the preset's own default macro state.
- Jungle Break ships `syncopation = 0.5` (`presets.cpp:2150`) → the backbeat snare resolves to beats **1 and 3**. The shipped preset does not play a backbeat (theory Rule 6).
- Liquid DnB ships `density = 0.35` (`presets.cpp:2209`) → kick and backbeat drop ~15 % of hits.
- Cuban Son Montuno: lane swings 0.20–0.30 **plus** `macros.swing = 0.25` (`presets.cpp:1061-1108`) → effective 0.45–0.55, double theory-afro-cuban Rule 7's 0.2–0.3 lilt zone. Chapter 03:115's "Syncopation macro at 0.4–0.6 … without overriding the lane-level settings" is false — it rotates the clave lane 3–5 pulses.
- Guide (`guide-using-poly.mdx:140`): "Start with Density and Swing — they have the most immediate impact" is offered directly above a preset list containing Balkan Aksak, Carnatic Tala, Gamelan Colotomic, and Agbekor — four traditions whose theory pages all say **no swing**.

### S3. Rotation arithmetic is wrong throughout

Recurring error class: a rotation value is asserted to land hits somewhere it does not
(engine semantics: right-shift; rotating a set cannot reorder its interval sequence).

- **Backbeat rot 4 ≠ beats 2/4.** E(2,8) rot 4 = {0,4} = beats 1 and 3; rot 2 is required. Present in `11-funk-soul.mdx:42` ("rotated to land on the backbeat"), `:69`, and **the funk theory page's own patch** (`theory-funk-soul.mdx:47`). The shipped Classic Funk preset uses the correct rot 2 (`presets.cpp:1886`).
- **Breakbeat snare rot 1 = the "&"s of 1 and 3**, not the claimed backbeat (`appendix-presets.mdx:84,95`); engine version differs again (`{4,4}` rot 0 = beats 1 and 3, accent mask on silent steps, `presets.cpp:166-175`).
- **Samba surdo rot 1 = beats 2/4**, so no beat-1 answer exists — chapter patch (`10-brazilian.mdx:35`), shipped Samba Batucada (`presets.cpp:1758-1764`), and **the Brazilian theory page's own patch** (`theory-brazilian.mdx:46`: E(2,8) rot 1 = {1,5}, no beat at all).
- **"Rotation changes the grouping" claims are impossible.** `07-balkan.mdx:33,37` (E(3,7) rotations mapped to wrong variants; correct rotations are 3 and 5), `06-indian-classical.mdx:27` (no rotation of E(4,7) can yield rupak's three-onset 3+2+2), `01-foundations.mdx:53` (a rotation of E(7,12) "becomes the cinquillo" — impossible, different onset/step counts), `appendix-euclidean-reference.mdx:103` ("rotation shifts through kopanitsa variants").
- **Offbeat placements that hit the beat.** `02-sub-saharan-africa.mdx:56-59` support lanes at rot 0 land on the dance beat; `13-drum-and-bass.mdx:50` "avoids beat 1" but E(5,16) rot 3 = {0,3,7,10,13} hits step 0; `04-afrobeat.mdx:57` kick rot 0 = {0,6,11} hits beat one (theory's own patch uses rot 3); `09-electronic.mdx:59` open hat rot 3 puts only 2 of 6 hits on the "&"s; `11-funk-soul.mdx` and `theory-funk-soul.mdx:48` ghost-lane gaps land **on** the backbeat instead of after it; `theory-balkan.mdx:52` lane 4 rot 2 aligns only 2 of 4 accents to cell heads (rot 4 aligns 3); `theory-sub-saharan-africa.mdx:53-54` kidi rot 1 collides with the dance beat on 2 of 5 hits (rot 2/5/8/11 reduce to 1).

### S4. Two different Euclidean algorithms render the same E(k,n) differently

The engine (`euclidean.cpp:23`, Bresenham `(i·k) mod n < k`) and the site diagram
component (`EuclideanDiagram.astro`, true Bjorklund) produce **different onset sets for
the same parameters**, and chapter prose sometimes matches neither:

- Standard bell: chapter 02:28 spells 2-2-1-2-2-1-2 ({0,2,4,5,7,9,10}); the adjacent diagram renders 2-1-2-2-1-2-2; the engine generates 2-2-2-1-2-2-1; the hard-coded preset pattern is 2-2-1-2-2-2-1 ({0,2,4,5,7,9,11}). Four spellings of the referent, no two identical.
- Rachenitsa: printed params (7/3/rot 0) produce 3+2+2 in the engine but 2+2+3 in the diagram; prose promises 2+2+3. Same divergence for E(4,9) daichovo.
- E(5,16): chapter 10:51-53 spells {0,3,6,10,13}; diagram renders {0,3,6,9,12}; engine plays {0,4,7,10,13}.
- Chapter 03:90-91's "click position 13 off, 12 on" workflow assumes seed {0,3,6,10,13}; the engine's E(5,16) is {0,4,7,10,13}, so the workflow yields the wrong clave.
- Gamelan complement demo (05:29-31): E(3,8) unrotated is **not** the complement of E(5,8) under either algorithm (rot 3 is required); the diagram shows the wrong pattern.

### S5. Degenerate lanes: `hitCount >= steps` saturates, and complements go silent

`euclidean.cpp:13-17` fills every step when k ≥ n; the kotekan complement
(`engine.cpp:97-103`) then inverts to all-silent:

- **Kotekan Interlock** (`presets.cpp:371-390`): polos `{3,8}`/3 hits fills all steps → the sangsih lane is **completely silent**. The preset's own description ("polos and sangsih fill each other's gaps") and `appendix-presets.mdx:178` are false, and theory-gamelan Rules 1/2 are unmet. The documented appendix table (8/5 + 8/3 + 16-step gong) matches nothing in the shipped preset.
- **Sub-Saharan: Agbekor** (`presets.cpp:845-857`): kidi `{5,12}`/5 and sogo `{3,12}`/3 both saturate into identical continuous 1/12 streams — the "stratum collision"/doubling that theory-sub-saharan Rule 7 forbids, and systematic offbeat placement (Rule 3) becomes impossible.

### S6. `appendix-presets.mdx` tables diverge from the shipped engine presets

Beyond rule conformance, the documented parameter tables repeatedly describe patches the
engine does not ship, so a reader checking a preset against the theory rules cannot trust
either source:

- Four on the Floor: doc hi-hat 16/8, ghost note 42 vs engine hi-hat `{8,8}`/8, ghost note 46 on `{7,8}`; doc macros non-neutral, engine neutral.
- Reich Phasing: doc "two lanes at E(5,12), drift +1" vs engine 5-step 1/12 cycle with 3 hits, drift 0.25 (doc drift is at/over the theory's ≥1 step/bar "no longer Reich" threshold; engine value is compliant).
- Pocket Groove kick: doc 8/3 at 1/8 vs engine `{4,16}`/3 (a one-beat cycle ≈ 12 hits/bar, breaking funk Rule 3's 3–5 hits).
- Kotekan Interlock: doc four-lane 8/16-step patch vs engine 3/3/4/7-step patch (see S5).
- Breakbeat snare: doc 8/2 rot 1 vs engine `{4,4}`/2 rot 0 — differently wrong (S3).
- Gamelan doc macros include Humanize 0.10 (theory: "no swing, no humanize"); the engine ships humanize 0.

### S7. Preset names cited in chapters that do not exist

- `04-afrobeat.mdx:51` → `preset="Afrobeat: Lagos '72"` (shipped: "Afrobeat Lagos").
- `11-funk-soul.mdx:37` → `preset="Funk: JB Pocket"` (shipped: "Classic Funk").
- `07-balkan.mdx:51,70` → `preset="7/8 Aksak"`, `"11/8 Aksak"` (shipped: "Rachenitsa 7/8", "Kopanitsa 11/8").
- `02-sub-saharan-africa.mdx:52` → badge says "Sub-Saharan: Agbekor" but the table's values are the "Ewe Polymetric Ensemble" preset; loading the named preset yields a materially different patch.

---

## 2. Per-tradition findings

### 2.1 Sub-Saharan Africa (`theory-sub-saharan-africa.mdx` vs ch. 02 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `02-sub-saharan-africa.mdx:28` | Bell spelled 2-2-1-2-2-1-2 — matches no implementation on the site (see S4) | Constr. 1, Rule 2 | contradiction |
| `02-sub-saharan-africa.mdx:42` | "support drums often run at different cycle lengths from the bell" presented as the defining Ewe feature | Constr. 3 (supports share the timeline's 12; only the lead stacks cycles) | contradiction |
| `02-sub-saharan-africa.mdx:56-59` | Support lanes rot 0 land on the dance beat (E(3,12)={0,4,8} vs dance {0,3,6,9} converge at 0) | Rule 3 ("lives offbeat; does not visit") | rule-violation |
| `02-sub-saharan-africa.mdx:65` | "mutation 15–25 % on Lane 4" (a support lane) | Rule 5 / Constr. 3 (supports 0–10 %; 15–30 % is the lead's budget) | rule-violation |
| `02-sub-saharan-africa.mdx:93` | Lead phrase "4–8 beats, gap 2–4" — only 6 beats lands on a cycle boundary (12-step 1/8 cycle = 6 beats); all gaps < 1 cycle | Rule 6 / Constr. 4 | contradiction |
| `presets.cpp:963-971` | Ewe Polymetric Ensemble bell not `timeline`-locked | Rule 1 ("single most damaging error") | rule-violation |
| `presets.cpp:845-857` | Agbekor kidi/sogo saturated identical streams (S5) | Rules 3, 7 | rule-violation |
| `presets.cpp:1009-1045` | Manding Djembe: no locked timeline; "soloist" djembe plays 7/8 steps continuously with zero variation budget — inverts bursty-lead/steady-support | Rules 1, 5, 7 | rule-violation |
| `presets.cpp:535` / `appendix-presets.mdx:236` | Afrobeat 12/8 applies Swing 0.1 to the 12-pulse frame | Sub-Saharan "What Breaks": Swing; Constr. 5 "No swing" | rule-violation |
| `appendix-presets.mdx:222,229` | 4-on-floor kick at 112 velocity slamming pulse 0 of the 12/8 cycle | "Western downbeat accenting" break item | rule-violation |
| `appendix-presets.mdx:228` vs `presets.cpp:493` | Row says "12/7/rot 0 + Fixed pattern" but E(7,12,0) and the hard-coded `fixedPattern` are different rotations of the bell | Constr. 1, Rule 2 | contradiction |

### 2.2 Afro-Cuban (`theory-afro-cuban.mdx` vs ch. 03 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `03-afro-cuban.mdx:86` | Chapter *defends* E(5,16)-as-clave and macro participation of the clave lane | Rule 1; "The approximate clave" break item ("the most common generator error") | contradiction |
| `03-afro-cuban.mdx:105,119` | Patch clave lane is E(5,16) rot 0 (fifth stroke at 13, not 12), labelled "Clave" | Rule 1 (timeline-mode exact clave) | rule-violation |
| `03-afro-cuban.mdx:107,119` | Tumbao E(3,8) rot 0 = pulses {0,6,12}: beat-one bass, no bombo (1/8 grid cannot reach pulse 3) | Rule 3; "Beat-one bass. Instantly un-Cuban" | rule-violation |
| `03-afro-cuban.mdx:115` | "Syncopation 0.4–0.6 … without overriding lane settings" — actually rotates the clave 3–5 pulses (S2) | Rule 1 | contradiction |
| `03-afro-cuban.mdx:119,125` | Mutation pushed onto marcha and bass; two simultaneous free voices | Rules 4, 5, 8; "A second free voice" break item | idiom-breaking-suggestion |
| `03-afro-cuban.mdx:66,70` | 3-side/2-side rotation claims arithmetically false (S3); theory says tresillo *is* the three-side | Rule spelling `:17-18` | contradiction |
| `appendix-presets.mdx:108,116` | Latin Feel's E(3,8) tresillo called "the son clave" | `:12` (clave = 5 strokes / 16 pulses) | terminology |
| `appendix-presets.mdx:127` | "Change Lane 1 to E(5,16) for the full son clave" | `:65` break item | idiom-breaking-suggestion |
| `appendix-presets.mdx:123,128` | Doc macros Swing 0.5 (+0.25 lane) ≈ 0.75; "Push Swing to 0.7" ≈ 0.95 | Rule 7 (0.2–0.3 zone) | rule-violation |
| `presets.cpp:1053-1061` | Son Montuno clave: E(5,16), un-locked (S1) | Rule 1, overview §1 | rule-violation |
| `presets.cpp:1075-1083` | Son Montuno tumbao rot 0: beat-one bass, no bombo | Rule 3 | rule-violation |
| `presets.cpp:208-215` | Latin Feel lane labelled "clave" is E(3,5) on a 5-sixteenth cycle — phases against the bar continuously | Rules 1, 8 | rule-violation |
| `presets.cpp:2219-2224,2593` | Afro-Electronic "Cuban clave" lane is an 8-step tresillo (fusion preset — naming only) | `:12` | terminology |
| `theory-afro-cuban.mdx:54` (self) | Theory's own tumbao row (8/3/rot 0 at 1/8) hits beat one and misses the bombo its Constr. 2 requires | Rule 3 | rule-violation (self) |

### 2.3 Afrobeat (`theory-afrobeat.mdx` vs ch. 04 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `appendix-presets.mdx:222,229` / `presets.cpp:499-501,2536` | Afrobeat 12/8 ships and advertises a four-on-the-floor kick — the page's named idiom-breaker | Rule 3; break item 2 | contradiction / rule-violation |
| `appendix-presets.mdx:230` / `presets.cpp:511` | Snare velocity 95, no ghost floor — second-loudest lane | Rule 5 ("the snare whispers"); Constr. 2 (vel ~60, ghost-heavy) | rule-violation |
| `appendix-presets.mdx:231` / `presets.cpp:518-519` | Shaker 12/12 saturates its stratum, poaching every lane's positions | Rule 2 (one voice, one territory) | rule-violation |
| `appendix-presets.mdx:241` | Scene-morph into the straight-16 funk preset framed as "classic Afrobeat" | Rule 1 (preserve ternary/binary friction); "Squaring the bell" break item | idiom-breaking-suggestion |
| `04-afrobeat.mdx:57` / `presets.cpp:1142-1150` | Kick rot 0 lands on beat one (theory's own patch uses rot 3); shipped in Afrobeat Lagos | Rule 3, Constr. 1 | rule-violation |
| `04-afrobeat.mdx:58` / `presets.cpp:1158-1159` | Cross-stick at velocity 90 — louder than hat and shekere | Rules 5, 8 | rule-violation |
| `04-afrobeat.mdx:37` | Hat range "12–14 hits" dips below theory's 13-hit floor | Constr. 1 | contradiction (minor) |
| `04-afrobeat.mdx:72` | Phrase-offset narrative ("bar 5 … a beat later … bar 9") misreads beat-denominated offsets (`types.h:213-215`) | Rule 6 | terminology |
| `04-afrobeat.mdx:51` | Nonexistent preset name (S7) | — | terminology |

### 2.4 Gamelan (`theory-gamelan.mdx` vs ch. 05 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `05-gamelan.mdx:29-31` | E(5,8)+E(3,8) unrotated presented as perfect complement — actually doubles step 0 and gaps step 1; rot 3 required; diagram shows the wrong pattern (S4) | Rule 1 | rule-violation |
| `05-gamelan.mdx:23-25` | "their intersection is empty" as the *definition* of kotekan | Rule 4 (overlap at structural tones; empty intersection everywhere = mechanical) | contradiction |
| `05-gamelan.mdx:25,54` | "Neither part makes musical sense alone" | Rule 2 (each part alone must be playable and idiomatic) | contradiction |
| `05-gamelan.mdx:63-66` | Colotomic ratios 4:8:16:64 — kenong→gong is a 4:1 jump; contradicts its own line 13 and its own patch | Rule 7 (rates halve) | contradiction |
| `05-gamelan.mdx:70` | "powers of two (or other integer multiples)" sanctions non-binary ratios | Rule 7; break item | contradiction |
| `05-gamelan.mdx:93` | Gong as "return to the beginning, not an arrival" — inverts end-weighting | Rule 8 ("arrival, not departure") | contradiction |
| `05-gamelan.mdx:95` | "without ever quite repeating" — impossible under the binary ratios both patches use | Rule 7 | contradiction |
| `05-gamelan.mdx:85` | "ketuk strikes every beat" vs tabled 4-steps/1-hit lane | Rule 7 unit definition | terminology |
| `presets.cpp:371-390` | Kotekan Interlock: sangsih silent (S5); doc table matches nothing shipped (S6) | Rules 1, 2 | rule-violation |
| `appendix-presets.mdx:184-187` | "Gong cycle" lane E(3,16) = 3 uneven strokes, a non-binary layer | Rules 7, 8 | rule-violation |
| `appendix-presets.mdx:191` | Humanize 0.10 documented on a Balinese preset (engine ships 0) | Constr. 5 ("no swing, no humanize"); break item | rule-violation |
| `appendix-presets.mdx:194` | "Change Kotekan from L1 to L3 or L5" — L3 is the low colotomic lane; L5 doesn't exist | Rules 1, 9 | idiom-breaking-suggestion |
| Clean | `Gamelan: Colotomic`, `Javanese Colotomic`, `Balinese Kotekan` presets fully conform (nesting, end-weight, no swing/humanize) | — | — |

### 2.5 Indian Classical (`theory-indian-classical.mdx` vs ch. 06 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `06-indian-classical.mdx:27` | "rotation … to match the traditional 3+2+2" — no rotation of E(4,7) can produce a three-onset partition | Rule 8; ch. 06:21 | contradiction |
| `06-indian-classical.mdx:82` | E(3,7) spelled `x . x . x . .` (2+2+3) — engine actually yields {0,3,5} = 3+2+2, the correct rupak shape | Rule 8, Constr. 1 | contradiction |
| `06-indian-classical.mdx:51,60` / `presets.cpp:1309-1317` | Low-drum "Sam accent" lane strikes khali (beat 9) at full 110 velocity — flat cycle weight | Rule 3; "Uniform cycle weight" break item | rule-violation |
| `06-indian-classical.mdx:52-53,72-74` | Theka lane spans half the sam lane's cycle (2:1 disagreement about the tala length); rupak version can't articulate 3+2+2 | Rules 1, 4 | rule-violation |
| `06-indian-classical.mdx:54` | 1/16 lane labelled *tigun* is 4× = chaugun (chapter's own line 45 agrees) | Rule 4 | terminology |
| `06-indian-classical.mdx:86-93` | Sam redefined as an emergent lcm(7,16)=112-step coincidence rather than the fixed target phrases aim at | Rules 2, 5; overview §4 | contradiction |
| `appendix-presets.mdx:305` | "[3+2+3] for rupak (7 beats)" — sums to 8; rupak is 3+2+2 | Constr. 1 | contradiction |
| `appendix-presets.mdx:307` | "Complexity 0.8 for layakari" — Complexity never changes subdivision (`macro.cpp:26-50`); it rotates the anchor off sam | Rule 4; "A missed sam" | idiom-breaking-suggestion |
| `presets.cpp:1326,1375` | Theka probability 0.95/0.9; no Indian preset locks any lane (S1) | Rule 1; overview §1/§3 | rule-violation |
| `presets.cpp:1338-1346,2561` | "Tigun" lane/description is 4× (chaugun); tigun absent | Rule 4 | terminology |
| `theory-indian-classical.mdx:40` (self) | Tihai arithmetic correct once, but the phrase gate is periodic (period 5.5) so only the first tihai lands on sam; matra/beat units conflated | Rule 6 | self-inconsistency |

### 2.6 Balkan (`theory-balkan.mdx` vs ch. 07 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `07-balkan.mdx:23,27,49` | "These are literally Euclidean rhythms … not a coincidence" | "Euclidean-only thinking" break item (cell sequence is ground truth) | contradiction |
| `07-balkan.mdx:45-49` | Kopanitsa presented as E(4,11)=3+3+3+2 with rotation variants — not a rotation of the five-cell 2+2+3+2+2 | `:13`, Constr. 2, `:56` | contradiction |
| `07-balkan.mdx:70-79` / `presets.cpp:1447-1452` | Kopanitsa kick E(4,11) rot 0 = {0,3,6,9}: only pulse 0 is a cell head | Rule 2 | rule-violation |
| `07-balkan.mdx:76,58` | "Pulse" lanes are gapped Euclidean patterns; nothing articulates the continuous quick pulse | Rule 3; Constr. 3; "Missing quick pulse" | rule-violation |
| `07-balkan.mdx:33,37` | E(3,7) rotation→variant mapping arithmetically false (S3); rotations 1–2 leave no downbeat onset | Rule 1 | contradiction |
| `07-balkan.mdx:51-58` / `presets.cpp:1398-1406` | Printed params yield 3+2+2 in the engine while prose/diagram promise 2+2+3 (S4) | Rules 2, 4 | rule-violation |
| `appendix-presets.mdx:263` | "Add a 5th lane at E(4,11) … 11/8 against the 7/8 foundation" | Rule 5 ("never on foreign cycle lengths"); break item | idiom-breaking-suggestion |
| `appendix-presets.mdx:245` | "E(7,8) zurna/darbuka polymetric tension" — lanes are same-grid E(4,7)/E(3,7); no polymeter; E-notation misused | Rules 1, 5 | contradiction / terminology |
| `appendix-presets.mdx:262` | "humanize … looser zurna … wedding band style" — inverts the stated near-mechanical wedding-band feel | Rule 7 | contradiction |
| `presets.cpp:2317` + `preset-taxonomy.md:88,96` | Balkan Funk hi-hat swing 0.15 while the taxonomy files it as Balkan-tradition (denying the fusion framing ch. 14 gives it) | Rule 6 ("No swing") | rule-violation |
| `theory-balkan.mdx:49,52` (self) | Own patch: lane 1 printed as Euclidean params its own text says are wrong; lane 4 rot 2 aligns only 2/4 accents to cell heads | Rules 2, 4 | self-inconsistency |

### 2.7 Minimalism (`theory-minimalism.mdx` vs ch. 08 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `08-minimalism.mdx:89` | Mutation recommended as the additive-process tool | Rule 8 (no mutation/probability in a process patch); Rule 6; "Stochastic parameters" break item | contradiction |
| `08-minimalism.mdx:120` | Tempo-ratio + drift simultaneously endorsed as "particularly powerful" | Rule 1 (one process at a time); break item | idiom-breaking-suggestion |
| `08-minimalism.mdx:116` | "hemiola drifts in and out of phase" — recomputed: it re-converges every bar (period-locked 3:2) | Rule 7 | contradiction |
| `08-minimalism.mdx:47-48,99-108` | Phasing pair at 90/85 velocity; "Nancarrow" canon uses 4 different patterns at 4 dynamic levels | Rule 4 (flat dynamics), Constr. (identical material) | rule-violation |
| `08-minimalism.mdx:31` | "no E(k,n) family with n=12 produces three consecutive onsets" — false (E(9,12) does); the theory page cites this passage | Rule 2's cited basis | contradiction |
| `appendix-presets.mdx:155-174` | Doc table E(5,12) drift +1 (at the "no longer Reich" threshold; engine ships 0.25 and a different cycle entirely — S6); doc macros Humanize 0.05; experiments push drift +2, Humanize 0.3, timbral separation of the pair | Rules 3, 4, 8; Constr. 2, 4 | rule-violation / idiom-breaking-suggestion |
| `presets.cpp:358-359` | Reich Phasing anchor: probability 0.8 — the ground drops 20 % of pulses | Rules 5, 8; Constr. 4 | rule-violation |
| `presets.cpp:1504,1514` | Reich Phase Process: phasing pair at 90 vs 85 velocity | Rule 4 | rule-violation |
| `presets.cpp:1553-1589` | Riley Layers: all four layers stochastically thinned (prob 0.9/0.85/0.8/0.75), invisible in the chapter's table | Rule 8 | rule-violation |
| `presets.cpp:1620,1640` | Nancarrow Tempi: probability 0.9/0.85 blurs the convergences that are the form | Rules 4, 7, 8 | rule-violation |
| Clean | Theory page's own construction patch fully self-consistent (verified arithmetic) | — | — |

### 2.8 Electronic & Breakbeat (`theory-electronic-breakbeat.mdx` vs ch. 09, 13 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `09-electronic.mdx:58` | Deep House patch kick carries Humanize 0.05 | Rule 1 (verbatim break item) | rule-violation |
| `09-electronic.mdx:67` vs `presets.cpp:1709,1746-1747` | "kick stays unswung" but preset ships kick humanize 2 ms + macros swing 0.35 / humanize 0.1 → resolved kick swing 0.35 (S2) | Rules 1, 4 | contradiction |
| `09-electronic.mdx:52` | Global Swing sweep as the techno↔house move — swings the kick (S2) | Rule 4 | idiom-breaking-suggestion |
| `09-electronic.mdx:59` | Open hat 16/6 rot 3: only 2 of 6 hits on the "&"s | Rule 2; "Slot promiscuity" | rule-violation |
| `09-electronic.mdx:73` | Density 0.2–0.9 sweep incl. the anchor (kick resolves to 2 hits @ 0.7 prob) | Rule 1 vs Rule 5 | contradiction |
| `09-electronic.mdx:81` | Mutation 5–15 % "keeps a loop interesting" — continuous novelty | Rule 5; "Continuous novelty" break item | idiom-breaking-suggestion |
| `13-drum-and-bass.mdx:44,69` / `presets.cpp:2123,2182` | Backbeat snare mutation 10 % / 5 % in both patches and both shipped presets | Rule 6; "Backbeat-snare mutation" break item | rule-violation |
| `13-drum-and-bass.mdx:50` | "avoids beat 1" — E(5,16) rot 3 hits step 0 (S3) | — (prose vs own patch) | contradiction |
| `13-drum-and-bass.mdx:62` | "swing more pronounced" — liquid patch and preset contain no swing anywhere | chapter-internal | contradiction |
| `13-drum-and-bass.mdx:87` | Neurofunk recipe: Syncopation 0.7–0.9 rotates the backbeat off 2/4; uniform mutation | Rules 6, 9 | idiom-breaking-suggestion |
| `presets.cpp:2150` | Jungle Break `syncopation = 0.5` → snare on beats 1 & 3: shipped preset has no backbeat (S2) | Rule 6 | rule-violation |
| `presets.cpp:1691` | Minimal Techno `density = 0.4` → kick E(3,4) @ 0.9 prob, contradicting its own description | Rule 1 | rule-violation |
| `presets.cpp:2209` | Liquid DnB `density = 0.35` → kick/backbeat ~15 % dropout | Rules 1, 6 | rule-violation |
| `presets.cpp:319-321` | Afro-House Phrases: swing 0.15 + humanize 0.15 + density 0.45 on an un-locked anchor kick | Rule 1 (×3) | rule-violation |
| `presets.cpp:44-45,2523` | Four-on-the-Floor's polymetric `{7,8}` lane is an **open hat** — slot class migrates every bar for 7 bars | Rule 2; "Slot promiscuity" | rule-violation |
| `appendix-presets.mdx:19,28` | "polymetric ghost layer at E(7,16)" — 16 steps at 1/16 = exactly one bar; no polymeter (page's own experiment at :34 concedes it) | Rule 3 | terminology |
| `appendix-presets.mdx:84,95` | Breakbeat "backbeat" snare on the "&"s of 1 and 3 (S3); engine differently wrong | Rules 2, 6 | rule-violation |
| `appendix-presets.mdx:102` | "Rotate Lane 1 through 0–5" — rotations 2 and 4 put kicks on the snare slots | Rule 7; "Kick/snare collisions" | idiom-breaking-suggestion |
| `appendix-presets.mdx:151` | "Automate Density from 0.3" on a documented 4-hit 1/4 kick | Rule 1 | idiom-breaking-suggestion |
| Clean | Theory page's own construction patch verified correct; ch. 09 Minimal Techno patch table itself clean | — | — |

### 2.9 Brazilian (`theory-brazilian.mdx` vs ch. 10 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `10-brazilian.mdx:35` / `presets.cpp:1758-1764` | Surdo E(2,4) rot 1 = beats 2 & 4 uniform — no 1↔2 dialogue, no beat-2 weight; contradicts chapter's own line 23 | Rule 1 | rule-violation |
| `presets.cpp:1776` | Samba Batucada tamborim probability 0.95 on the "never mutates" telecoteco stratum (S1) | Rule 2; overview §1 | rule-violation |
| `presets.cpp:1810` | Caixa probability 0.9 — random holes in the carpet | Rule 3 ("caixa never stops"); "A gappy caixa" | rule-violation |
| `10-brazilian.mdx:29` | "rotation shifts the accent away from beat 1" — recomputed rot 2 still strikes beats 1 and 4 | Rule 2 | contradiction |
| `10-brazilian.mdx:45,87` | Humanize 0.2–0.3 recommended (theory cap ≤ 0.2); `presets.cpp:1815` ships 0.25 | Constr. 6 | contradiction / rule-violation |
| `10-brazilian.mdx:85` | Swing 0.2–0.3 (theory window 0.15–0.25) | Rule 6 | contradiction |
| `appendix-presets.mdx:267-274` / `presets.cpp:601-608` | Bossa surdo on beats "1 and 3" with equal velocity — beat-one weighting, "instantly a march" | Rules 1, 7; break item | contradiction |
| `appendix-presets.mdx:267,285` / `presets.json:713` | Tamborim timeline named "clave" and *is* Cuban son clave {0,3,6,10,12} — not the telecoteco; strikes beats 1 and 4 against Rule 2's contratempo | Rule 2 | terminology |
| `presets.cpp:610-671` | Micro-timing profile is pair-shaped (odd steps only, 4th sixteenth pushed **late**) — the shape Rule 6 explicitly says fails to reproduce LSSL | Rule 6; break item | rule-violation |
| `presets.cpp:1842,1854` | Bossa Trio ride/brush probability 0.9/0.8 — dropout on the one stratum bossa must keep continuous | Rule 7 | rule-violation |
| `10-brazilian.mdx:51-53,71` | E(5,16) spelled {0,3,6,10,13} — matches neither the adjacent diagram nor the engine (S4) | — | contradiction |
| `theory-brazilian.mdx:46` (self) | Own patch surdo 8/2 rot 1 = {1,5}: lands on no beat; Constr. 1 demands beats 1 and 2 (rot 0) | Rule 5, Constr. 1 | rule-violation (self) |

### 2.10 Funk & Soul (`theory-funk-soul.mdx` vs ch. 11 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `11-funk-soul.mdx:23,50` | Even sixteenth ghost carpet presented as the definition ("the ghosts are the groove"); E(11,16) rests uniform, with a rest **on** beat 2 | Rule 4; "Uniform ghosts" break item | contradiction / rule-violation |
| `11-funk-soul.mdx:42,50` | Snare "rotated to land on the backbeat" — rot 4 = beats 1 & 3 (S3); preset uses correct rot 2 | Rule 2 | rule-violation |
| `11-funk-soul.mdx:68` | Kick E(4,16) rot 0 = four-on-the-floor hitting both backbeat slots, zero syncopation | Rule 3 | rule-violation |
| `11-funk-soul.mdx:69` | Backbeat at velocity 90/ghost 35/spread 45 % — can drop to ghost level (theory: 105+, full velocity) | Rules 2, 8 | rule-violation |
| `11-funk-soul.mdx:60,62` | Snare "5–10 ms late"; Humanize randomising "every lane" incl. the backbeat | Rules 6, 8 (neo-soul loosens hats/kick, never the backbeat) | contradiction |
| `11-funk-soul.mdx:77` | 12-step rim lane "never landing in the same place twice … defines the genre" — pattern freedom sold as feel | System ("near-total fidelity"); Rule 8 | contradiction |
| `presets.cpp:1916` | Classic Funk `syncopation = 0.3` → nothing strikes the One; backbeat rotates off 2/4 (S2) | Rules 1, 2 | rule-violation |
| `presets.cpp:1895` | Ghost lane on a different MIDI note (37) than the accent snare (38) — breaks one-drum-two-dynamics | Constr. 3 | rule-violation |
| `presets.cpp:1930,1944,1974` | Neo-Soul Pocket: 4-on-floor kick; backbeat prob 0.95 / vel 90; humanize macro 0.4 jitters the backbeat ±10 ms | Rules 2, 3, 8 | rule-violation |
| `presets.cpp:443` / `appendix-presets.mdx:207` | Pocket Groove snare mutation 10 % (engine) / 5 (docs) on the backbeat | Rule 2 ("zero mutation") | rule-violation |
| `presets.cpp:427-428` vs `appendix-presets.mdx:206` | Kick `{4,16}` = one-beat cycle ≈ 12 hits/bar (docs claim 8/3 — S6) | Rule 3 (3–5 hits) | rule-violation |
| `appendix-presets.mdx:216,218` | "Humanize 0.5 … almost drunk feel"; "mutation 30 + gating on the ghost layer" | Rule 6 ("Random pocket"), Rule 7 ("Pattern churn") | idiom-breaking-suggestion |
| `theory-funk-soul.mdx:46-48` (self) | Own patch: kick E(4,16) rot 0 (no syncopes, hits backbeat slots); snare rot 4 (beats 1/3); ghost gaps land **on** 2 and 4, not after | Rules 2, 3; Constr. 1–3 | rule-violation (self) |

### 2.11 Jazz (`theory-jazz.mdx` vs ch. 12 + presets)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `12-jazz.mdx:68` / `presets.cpp:2045,2049` | "Ride (steady)" with 10 % mutation and 3-of-4 quarters — the "varying ride" failure, shipped in Elvin Jones Cascade | Rule 1; Constr. 1 ("mutation 0") | rule-violation |
| `12-jazz.mdx:89` | "zero **or very low** mutation" on timekeeping lanes — licenses what the theory forbids | Rule 1 | contradiction |
| `12-jazz.mdx:62,79` | Never-resolving lcm(3,4,5,7)=420 superimposition presented as the goal; convergence relocated to accidental LCM events | Rules 5, 7 ("loans repaid at form boundaries") | idiom-breaking-suggestion / contradiction |
| `presets.cpp:2035-2092` | Elvin Cascade: all three cross-rhythm lanes run ungated forever (`phraseLength` 0) | Rule 7; Constr. 6; "Perpetual superimposition" | rule-violation |
| `12-jazz.mdx:49,71` / `presets.cpp:2011,2083` | Kick at velocity 80/85 — above hat and snare, inverting the dynamic hierarchy ("feathered", theory vel 55) | Rule 8; Constr. 4 | rule-violation |
| `12-jazz.mdx:25,50` | Ride↔snare swing gap 0.15–0.20 (budget: 0.05–0.1); shipped at `presets.cpp:1992,2027` | Rule 6 | rule-violation |
| `12-jazz.mdx:23` | Swing flattens "at 200+ BPM" vs theory's ~250 threshold (Friberg & Sundström) | Rule 6 | terminology |
| `12-jazz.mdx:93` | Ungated 7/11-step rim-click lanes recommended | Rule 7 | idiom-breaking-suggestion |
| `12-jazz.mdx:64-73` | Core-jazz patch silently drops the planted 2-and-4 hi-hat | Rule 2 | rule-violation |
| `theory-jazz.mdx` (self) | Rule 3 "one to four attacks" vs its own patch's 4–5; lanes 2/4/5 share identical swing 0.40 (own Rule 6 wants 0.05–0.1 diffs); missing per-lane ms offsets and form envelope its own construction requires | Rules 3, 6, 8 | self-inconsistency |

---

## 3. Cross-cutting pages (foundations, synthesis, grammar, guide, appendices)

| Location | Finding | Rule | Class |
|---|---|---|---|
| `01-foundations.mdx:53` | Rotation of the Ewe bell "becomes the Cuban cinquillo" — impossible (7 onsets/12 steps vs 5/8) | Euclidean ref.:66; Afro-Cuban vocabulary | terminology |
| `01-foundations.mdx:68` | "converge every 84 steps (7 bars of 12)" — it's 42 quarters = 10.5 bars (timing appendix computes this same pair correctly) | overview §4 | terminology |
| `01-foundations.mdx:45` | Free coprime cycle lengths attributed to West African & gamelan practice — both theory pages say shared/binary-nested cycles | theory-SSA Constr. 3; gamelan Rule 7 | contradiction |
| `01-foundations.mdx:35` + `appendix-euclidean-reference.mdx:76` | Cinquillo called the tresillo's "complement … cover nearly every pulse" — it's a strict superset; union covers 5/8 pulses | overview §2 | terminology |
| `14-synthesis.mdx:29,35` | 8-step tresillo shipped as "the clave backbone/matrix", un-locked (`presets.cpp:2220-2228`) | Afro-Cuban `:13`, `:65`; overview §1 | contradiction |
| `14-synthesis.mdx:37-38` / `presets.cpp:2260` | Kotekan L1 pairs sangsih with the 8-step clave (not the 12-step polos): 9 hits, doubles 6 of 7 polos strokes, gaps at 3 and 6; table claims 5 hits | Gamelan Rules 1, 2; Constr. | rule-violation |
| `14-synthesis.mdx:37-38` | Swing 0.10 on lanes labelled polos/sangsih | Gamelan "Swing or humanize" break; overview §5 | idiom-breaking-suggestion |
| `14-synthesis.mdx:49,61,68` | Swing applied to the rachenitsa hybrid while claiming the aksak survives; contradicts its own line 147 | Balkan Rules 6, 7; overview §5's own example | idiom-breaking-suggestion |
| `presets.cpp:2330,2597` | Balkan Funk `syncopation = 0.3` displaces the advertised 2+2+3 cell-head accents | Balkan Rules 1, 2 | rule-violation |
| `presets.cpp:2275` | Afro-Electronic `swing = 0.15` reaches the E(4,4) techno kick (table says Swing 0) | Electronic Rule 1 | rule-violation |
| `15-compositional-grammar.mdx:58-62` | Generic recipe adds the anchor kick **first** and the timeline second, "mutation low" (not zero) — inverts referent-first-and-locked | overview §1; SSA Rule 1 | contradiction |
| `15-compositional-grammar.mdx:44,81` | Density sweeps praised for thinning "the bell timeline" (un-locked in Compositional Arc) | SSA "A varying timeline" ("single most damaging error") | contradiction |
| `15-compositional-grammar.mdx:62,66` | Foreign-cycle timeline recommended even for the aksak anchor; variation budget spread across 3+ ornament lanes | Balkan `:60`; overview §3 | idiom-breaking-suggestion |
| `guide-using-poly.mdx:140` | "Start with Density and Swing" above a list of no-swing tradition presets (S2) | overview §5; four tradition pages | idiom-breaking-suggestion |
| `guide-using-poly.mdx:371-373` | Walkthrough promises "the rhythmic tradition … plays" from steps/hits/rotation/subdivision/note alone — omits mode/mutation/swing, the locking columns | overview §1, `:15` (validity ≠ idiom) | contradiction |
| `appendix-euclidean-reference.mdx:97,103` | E(4,11) labelled kopanitsa with rotation "variants" | Balkan `:13`, `:56`, `:64` | contradiction |
| `appendix-euclidean-reference.mdx:145` | "Experiment with all available rotations before settling" — for timeline/clave lanes rotation is identity/direction, not feel | SSA `:21`; Afro-Cuban `:42`, `:66` | idiom-breaking-suggestion |
| `appendix-euclidean-reference.mdx:114` vs `08-minimalism.mdx:31` | Reference's own E(9,12) row disproves the chapter claim the theory page cites | Minimalism Rule 2's basis | contradiction |
| `appendix-presets.mdx:41` | Polymetric Drift "does not repeat for 1155 **steps**" — lanes on different grids; correct figure is 1155 quarter notes | overview §4; timing appendix | terminology |
| Clean | `appendix-timing-model.mdx` (all worked values recomputed correct), `introduction.mdx`, `index.mdx`, Sparse Pulse, IDM Glitch (deliberately abstract) | — | — |

---

## 4. Theory-page self-consistency defects (fix the authority too)

Remedial work must also patch the deep dives themselves, or conformance can never be
achieved:

1. **theory-afro-cuban.mdx:54** — its own tumbao row hits beat one and cannot reach the bombo (needs 16-step lane or rotation).
2. **theory-funk-soul.mdx:46-48** — kick rot 0 (four-on-floor), snare rot 4 (beats 1/3), ghost gaps on the backbeat. Three of five construction rows contradict the page's own rules. Also Rule 6 vs Rule 8 tension on Humanize.
3. **theory-brazilian.mdx:46** — surdo rot 1 lands on no beat; Constr. 1 requires rot 0.
4. **theory-balkan.mdx:49-56** — patch table prints Euclidean params the page's own text says produce the wrong spacing; lane 4 rotation misaligns cell heads.
5. **theory-jazz.mdx** — comping attack count, uniform 0.40 swing across lanes vs its own 0.05–0.1 rule, missing per-lane offsets/form envelope.
6. **theory-gamelan.mdx:45-60** — construction text says 5 polos hits, patch uses 9; four colotomic lanes prescribed, three supplied; strict-complement patch violates its own Rule 4 overlap requirement; bell/shekere strata poach under its own Rule 2.
7. **theory-indian-classical.mdx:40** — tihai lands on sam only on the first phrase repetition (periodic gate, period 5.5 beats); matra/beat units conflated.
8. **theory-sub-saharan-africa.mdx:53-54** — kidi rotation collides with the dance beat twice; sogo offbeat requirement is unsatisfiable as stated (a 3-hit lane must intersect a 4-hit dance beat once per 12-pulse cycle).
9. **theory-afrobeat.mdx:44-48** — bell and shekere share a stratum against its own Rule 2; construction demands a Spread column the patch omits; hat carries mutation the rules assign to colour voices.

---

## 5. Suggested remediation workstreams

1. **Engine: lock every referent.** Add `timeline = true` (or a macro-exemption role) to the referent lane of every tradition preset; set referent probability to 1.0 and mutation to 0. Re-derive `presets.json`. This clears the largest single class of violations (S1) and makes S2's documented macro advice safe for referents.
2. **Engine: fix broken presets.** Kotekan Interlock (silent sangsih), Agbekor (saturated supports), Jungle Break / Classic Funk / Minimal Techno / Liquid DnB (macro defaults that undo the genre), Samba Batucada surdo rotation, Son Montuno tumbao, Afrobeat 12/8 kick, stochastic thinning on all minimalism presets, synthesis-preset kotekan source lane.
3. **Unify Euclidean semantics** (S4): make `EuclideanDiagram.astro` use the engine's Bresenham convention (or vice-versa), then re-verify every printed pattern spelling and rotation in chapters 01–15 and both appendices.
4. **Sweep rotation values** (S3): audit every patch table rotation against intended beat placement using the engine convention; fix the four theory-page construction patches first (they are the templates readers copy).
5. **Rewrite contradicting chapter prose**: the highest-priority contradictions are ch. 05 (kotekan definition inverted ×3), ch. 03 (E(5,16) defence), ch. 07 (Euclidean-only framing), ch. 08 (mutation-as-process), ch. 12 (unresolved superimposition as goal), ch. 15 (anchor-first recipe), ch. 06 (emergent sam).
6. **Re-frame or replace idiom-breaking Experiments**: either add explicit "this deliberately leaves the idiom — see the deep dive" framing, or substitute rule-conformant experiments. Roughly 20 experiment bullets are affected across the presets appendix and chapters.
7. **Reconcile `appendix-presets.mdx` with the engine** (S6) and fix nonexistent preset names (S7). Consider generating the appendix tables from `presets.json` the way `counts.json` already is, so they cannot drift.
8. **Patch the theory pages' own defects** (§4 above) so the authority is internally consistent before enforcing it.

---

*Method note: findings were produced by twelve parallel audits (one per tradition, one for
cross-cutting pages), each reading the theory page, companion chapter, and preset sources
in full, with all pattern/rotation/LCM arithmetic recomputed against
`engine/src/euclidean.cpp`, `engine/src/macro.cpp`, and `engine/src/engine.cpp` semantics.
Engine-level mechanisms cited in the systemic findings were independently re-verified.*
