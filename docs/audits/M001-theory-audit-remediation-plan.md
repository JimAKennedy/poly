---
class: gated
---

# Music-Theory Audit Remediation Plan (M001–M005)

> **Status:** current (2026-08-23) — plan accepted, no slice executed yet.
> **Lifecycle:** Planning artifact. Drives milestones M001–M005. Rows move from
> `open` to `done` as slices land; the plan is archived when the ledger is clear.
> **Upstream input:** the August 2026 external music-theory audit of the Poly
> Guide (`poly_theory_audit.md`, held outside the repo in the owner's research
> archive). Every finding in that document — the twelve ranked corrective items
> in its Section 5, plus every sub-finding in Sections 1–4 and every enrichment
> source in Section 6 — is enumerated below as exactly one ledger row.
> **Numbering:** Milestone IDs here are the **GSD scheme** (`M001`–`M005`),
> assigned by `gsd_milestone_generate_id` against this project's GSD database.
> An earlier draft of this plan used the repo's legacy commit-message milestone
> numbering (`M074`–`M078`); those numbers are retired and appear nowhere in
> this document. Slice IDs (`S01`, `S02`, …) below are **provisional** — they
> reflect this plan's own decomposition and are reconciled against the real
> slice records when `gsd_plan_milestone` runs.
> **Completeness:** Mechanically enforced by
> `docs/audits/theory-audit-remediation.test.mjs` (`node --test`), built in
> M001/S01 — asserts every finding ID F01–F54 is present, each carries exactly
> one valid severity, disposition, and status token, each names a real slice
> that appears in that milestone's breakdown table below, and no `_pending_`
> placeholder survives.

## Why this plan exists

The audit's verdict is that the guide's **mathematics is sound** and its **CI
harness is the strongest thing about it**, but that three classes of problem
prevent it standing as a knowledge base: a handful of outright factual errors, a
numbered reference list that cites YouTube videos for the load-bearing
theoretical claims, and a set of chapter patches that do not follow the rules
their own companion theory pages state.

The remediation is therefore not "rewrite the guide." It is: fix what is wrong,
re-source what is weakly sourced, state plainly where the guide simplifies, and
align the patches with the rules — then extend the CI harness so none of it can
silently regress.

**Re-review contract.** Every row below closes with a *cited verification* — a
test name, a check, or a named artifact — not an assertion that it was done. A
re-read of the audit against the post-implementation site should find each
numbered item addressed at the location named in the "Fix lands in" column.

## Severity tokens

| Token | Meaning |
|---|---|
| `P0` | Factually wrong as written. A reader who trusts it is misinformed. Fix regardless of repositioning. |
| `P1` | Not wrong but overclaimed, under-sourced, or internally inconsistent. Credibility risk. |
| `P2` | Enrichment. The guide is accurate without it; the addition deepens it. |

## Disposition tokens

| Token | Meaning |
|---|---|
| `correct` | The claim is wrong. Change the claim. |
| `source` | The claim is right but cited to a weak source, or uncited. Attach the authority. |
| `reframe` | The claim is defensible but presented with more force or grounding than the source supports. Hedge and attribute. |
| `disclose` | A deliberate pedagogical simplification. Keep it; say plainly that it is one. |
| `patch-align` | A chapter patch does not follow its companion theory page's rules. Fix the patch or cross-reference the divergence. |
| `enrich` | Add a missing source or section. |
| `verify` | Confirm the underlying fact before deciding the action. |
| `accept` | Deliberate no-change. The repositioning statement covers it. Recorded so a future edit does not "discover" it again. |

## Status tokens

`open` · `in-progress` · `done` · `accepted`

---

## Finding ledger

### M001 — Theory Corrections (P0 factual errors and overclaims)

| ID | Audit § | Finding | Sev | Disp | Fix lands in | Slice | Verification | Status |
|---|---|---|---|---|---|---|---|---|
| F01 | 2.4.1, 3.4, §5.1 | Ch 6 uses E(4,7) (grouping 2+2+2+1) to illustrate rupak's 3+2+2. E(3,7) at rotation 3 is the correct illustration; the chapter's own patch already uses E(3,7), so prose and patch contradict each other. | `P0` | `correct` | `06-indian-classical.mdx` — the E(4,7) paragraph and the E(3,7) patch note | M001/S02 | New case S02-F01 in `site/tests/theory-audit-claims.test.mjs` forbids `E(4,7)` in the rupak illustration and pins the diagram to `<EuclideanDiagram steps={7} hits={3} rotation={3} />`, checked against `site/src/audio/bjorklund.ts` | `done` |
| F02 | 2.7.2, §5.3 | Ch 8 attributes gradual phase shifting to *Drumming* (1971). *Drumming* is an additive/subtractive process; gradual phasing is *Piano Phase* / *Violin Phase* (1967); *Clapping Music* (1972) uses discrete one-position jumps. Three techniques conflated. | `P0` | `correct` | `08-minimalism.mdx` — the *Drumming* and *Clapping Music* paragraphs | M001/S03 | Case S03-F02 in `site/tests/theory-audit-claims.test.mjs` forbids *Drumming* citations followed within ~120 chars by phase-shifting/gradual/continuous language, pinning each named Reich work to its own technique term | `done` |
| F03 | 2.7.3, §5.3 | Poly's Drift is described as modelling *Drumming*. Drift gradually shifts cycle position — that models *Piano Phase*. | `P0` | `correct` | `08-minimalism.mdx` Drift section | M001/S03 | Section-scoped case S03-F03 in `site/tests/theory-audit-claims.test.mjs` extracts the `## Drift as Phase Engine` section and asserts *Piano Phase* is cited while *Drumming* is not | `done` |
| F04 | 2.5.1, §5.4 | Ch 7: "Bjorklund's algorithm and Balkan folk musicians independently arrived at the same solution." Bjorklund (2003) derived it for neutron-beam timing; Toussaint (2004/2005) identified the musical connection. The relationship is mathematical, not historical convergence. | `P0` | `reframe` | `07-balkan.mdx` — the paragraph following the E(3,7) diagram | M001/S04 | New case S04-F04 in `site/tests/theory-audit-claims.test.mjs` forbids the independent-convergence phrasing and requires the Toussaint-identified (ref-1) and Bjorklund (ref-46) framing | `done` |
| F05 | 2.5.2, §5.10, §1 | Ch 7 presents E(4,11)=3+3+3+2 as the kopanitsa grouping while line 37 of the same chapter states the primary form is 2+2+3+2+2 (five cells, not four — not cleanly Euclidean at any rotation). Internal contradiction. | `P1` | `correct` | `07-balkan.mdx` — the aksak-definition paragraph and the "Daichovo and Kopanitsa" section | M001/S04 | Case S04-F05 in `site/tests/theory-audit-claims.test.mjs` asserts the Daichovo/Kopanitsa paragraph names 2+2+3+2+2 as the primary form with E(4,11) as a variant rendering, and arithmetic case S04-F05-arith re-derives the E(5,11) and E(4,11) spellings from `site/src/audio/bjorklund.ts` | `done` |
| F06 | 2.5.5 | Ch 7 implies aksak long beats are exactly 1.5× short beats. Goldberg (2015) shows performed long beats deviate systematically. Chapter cites Goldberg only in Further Reading. | `P1` | `disclose` | `07-balkan.mdx`, `theory-balkan.mdx` | M001/S04 | Case S04-F06 in `site/tests/theory-audit-claims.test.mjs` forbids assertive "exactly 1.5×" / "exactly 3:2" long-vs-short-beat phrasing in `07-balkan.mdx` and requires an inline fr-goldberg-2015 citation on the aksak long-beat caveat | `done` |
| F07 | 2.1.2, §5.9 | Ch 2: Ewe ensemble music has been organised this way "for centuries." Unsourced; fieldwork documentation begins mid-20th century. | `P1` | `reframe` | `02-sub-saharan-africa.mdx` — the chapter opening paragraph | M001/S05 | Prose-claim test forbidding unhedged multi-century temporal claims in Ch 2 | `open` |
| F08 | 2.1.1 | Ch 2 calls E(7,12) "the single most important timeline pattern in sub-Saharan music." Toussaint (2005) says "most commonly used" — descriptive, not evaluative. | `P1` | `reframe` | `02-sub-saharan-africa.mdx` — the gankogui / E(7,12) paragraph | M001/S05 | Prose-claim test asserting the descriptive phrasing | `open` |
| F09 | 2.6.1, §5.11 | Ch 4: Fela "built the genre… but it was Tony Allen who created its rhythmic vocabulary." Contentious; Allen himself co-attributes, and Fela's horn arranging shaped the rhythmic feel. | `P1` | `reframe` | `04-afrobeat.mdx` — the chapter opening paragraph | M001/S06 | Prose-claim test asserting co-attribution language | `open` |
| F10 | 2.6.3 | Ch 4: "Tony Allen's timing was precise but not quantised." Received wisdom presented as measured fact; no measurement study covers Allen recordings. | `P1` | `reframe` | `04-afrobeat.mdx` — the Humanize bullet in the macros section | M001/S06 | Prose-claim test asserting the claim is marked as characterisation, not measurement | `open` |
| F11 | 2.7.1 | Ch 8 adopts Reich's own framing that West African and Indonesian musicians "had known for centuries" that identical patterns at different rates generate complexity. African stacking uses fixed independent cycles; gamelan uses fixed hierarchical nesting. Neither is deliberate drift. Reich's analogy was criticised (Agawu 2003). | `P1` | `reframe` | `08-minimalism.mdx` — the chapter opening paragraph | M001/S06 | Prose-claim test asserting attribution to Reich's framing rather than assertion as fact | `open` |
| F12 | 2.4.2 | Ch 6: E(7,16) as "the kind of structure a tabla player might outline during a slow theka." Thekas are fixed named bol sequences, not Euclidean distributions. Fine as analogy, not as grounding. | `P1` | `reframe` | `06-indian-classical.mdx` — the E(7,16) tintal-skeleton paragraph | M001/S02 | Case S02-F12 in `site/tests/theory-audit-claims.test.mjs` forbids the pre-correction grounding phrase and requires the analogy framing ("not itself a tintal theka", "thekas are fixed, named bol sequences", "rough analogue") | `done` |
| F13 | 2.8 (Electronic) | Ch 9: "difference between techno and house is often reducible to one parameter: swing" cited to a Roger Linn blog. Contested (Butler 2006). Attribute to Linn rather than stating as fact. | `P1` | `source` | `09-electronic.mdx` | M001/S06 | Inline attribution present; Butler cross-reference added | `open` |
| F14 | 2.8 (D&B) | Ch 13: Amen break as "most sampled recording in music history" — disputed; needs "widely considered" hedge. | `P1` | `reframe` | `13-drum-and-bass.mdx` | M001/S06 | Prose-claim test asserting the hedge | `open` |
| F15 | 2.8 (Jazz) | Ch 12: Max Roach's polymetric independence claim is directionally right but uncited. Monson (1996) and Gridley are the authorities. | `P1` | `source` | `12-jazz.mdx` | M001/S06 | Monson citation resolves in the reference appendix | `open` |
| F16 | §1 (lcm) | The guide uses lcm convergence as "sam" (Indian) and "gong stroke" (gamelan). Legitimate informal translation, but neither tradition uses the term that way internally. Make the translation explicit. | `P1` | `disclose` | `01-foundations.mdx`, `06-indian-classical.mdx`, `05-gamelan.mdx` | M001/S06 | Translation caveat present at each of the three sites; asserted by prose-conformance test | `open` |
| F54 | issue [#91](https://github.com/JimAKennedy/poly/issues/91) | The E(3,16) row in the euclidean-reference appendix (`x . . . . x . . . . x . . . . .`, 5+5+6 at rotation 0) reads as wrong to a reader deriving Bjorklund from scratch, because Poly's phase convention (which rotation counts as canonical) is nowhere stated. The row is correct under that convention; the convention itself must be made explicit next to the table so a reader can verify without re-deriving. | `P1` | `disclose` | `appendix-euclidean-reference.mdx` — the table preamble and the E(3,16) row | M001/S02 | Case S02-F54 in `site/tests/theory-audit-claims.test.mjs` asserts the "Phase Convention" preamble is present (rotate-right / rotation-0 canonical / worked example "onsets at steps 0, 5, 10"), and an arithmetic case re-derives the printed E(3,16) row from `bjorklund(16, 3)` — closes [#91](https://github.com/JimAKennedy/poly/issues/91) | `done` |

### M002 — Citation Integrity (reference-list re-tiering)

| ID | Audit § | Finding | Sev | Disp | Fix lands in | Slice | Verification | Status |
|---|---|---|---|---|---|---|---|---|
| F17 | §4 "Ref 2" , §5.7 | Ref [2] cites Goldberg (2025), *Music Theory Online* 31(2). MTO is current through Vol 30 as of mid-2026; the URL is a forward reference. May be a citation error. | `P0` | `verify` | `appendix-references.mdx` ref-2 | M002/S01 | Resolution recorded in the ledger; if unpublished, replaced or marked forthcoming. Link-resolution check in CI | `open` |
| F18 | 2.2.1, §4 Tier C, §5.2 | Refs [10] and [11] — YouTube videos — are the inline citations for the clave matrix, the most important theoretical claim in Ch 3. Peñalosa (2009) is already in Further Reading. | `P0` | `source` | `03-afro-cuban.mdx` — the clave-matrix and non-Euclidean-gap paragraphs; `appendix-references.mdx` | M002/S02 | Tier-A citation check (F23) passes for Ch 3 | `open` |
| F19 | 2.6.2, §4 Tier C, §5.12 | Refs [14]–[17] for Afrobeat are YouTube videos and production blogs. Allen & Veal (2013) and Veal (2000) are already in Further Reading. | `P0` | `source` | `04-afrobeat.mdx` — the opening and phrase-gating paragraphs; `appendix-references.mdx` | M002/S03 | Tier-A citation check passes for Ch 4 | `open` |
| F20 | 2.4.7, §5.12 | Refs [21]–[25] for Ch 6 include a commercial blog, a high-school textbook PDF, and a YouTube konnakol video. Clayton (2000), Nelson (2008), Kippen (1988) are the authorities and are already in Further Reading. | `P1` | `source` | `06-indian-classical.mdx`; `appendix-references.mdx` | M002/S04 | Tier-A citation check passes for Ch 6 | `open` |
| F21 | 2.5, §4 Tier C, §5.12 | Refs [26], [27] for Balkan are educational aggregator pages. Brăiloiu (1951), Rice (1994), Goldberg (2015) are in Further Reading only. | `P1` | `source` | `07-balkan.mdx` — the aksak-definition and svatbarska-muzika paragraphs; `appendix-references.mdx` | M002/S05 | Tier-A citation check passes for Ch 7 | `open` |
| F22 | 2.2.2 | Ch 3: "the habanera rhythm that Jelly Roll Morton called 'the Spanish tinge'" — accurate but uncited. Lomax's Morton interviews (1950) are the primary source. | `P1` | `source` | `03-afro-cuban.mdx` — the tresillo / "Spanish tinge" paragraph | M002/S06 | Lomax citation resolves in the appendix | `open` |
| F23 | §4 (whole section) | The reference list mixes peer-reviewed scholarship and hobbyist media at equal citation weight, with no mechanism preventing regression. | `P1` | `source` | `appendix-references.mdx`; new `site/tests/citation-tier.test.mjs` | M002/S06 | New test: every reference carries a declared tier, and every inline citation attached to a named-theory claim resolves to a Tier-A source | `open` |

### M003 — Scope and Repositioning (honest-simplification framing)

| ID | Audit § | Finding | Sev | Disp | Fix lands in | Slice | Verification | Status |
|---|---|---|---|---|---|---|---|---|
| F24 | §5 (framework) | The guide has no statement of what it is and is not. The audit supplies a four-paragraph repositioning statement and judges it defensible. | `P1` | `reframe` | New "About This Guide" section; linked from `introduction.mdx` and every `theory-*.mdx` | M003/S01 | Test asserting the About page exists and is linked from the introduction and all 12 theory pages | `open` |
| F25 | 2.4.6, §5.8 | Ch 6 is titled/described as covering "Hindustani and Carnatic" but the Carnatic tala system (solkattu/konnakol, different tala families, different conceptual frame) is absent. | `P1` | `correct` | `06-indian-classical.mdx` — front-matter `description` and the page scope note | M003/S02 | Description front-matter and scope note agree on Hindustani-only; asserted by test | `open` |
| F26 | 2.1.3 | Ch 2 characterises Manding ensembles as all sharing one cycle length. Manding dunun ensembles do use distinct cycle lengths in many contexts. Pedagogical flattening, not error. | `P1` | `disclose` | `02-sub-saharan-africa.mdx` — the "Manding Traditions" section | M003/S03 | Simplification note present, Charry (2000) cited (see F44) | `open` |
| F27 | 2.3.2 | Gamelan Rule 3 states polos leans onbeat, sangsih offbeat. In *norot* the relationship is effectively reversed. Style-dependent, presented as general. | `P1` | `disclose` | `theory-gamelan.mdx` Rule 3 | M003/S03 | Style-dependence caveat asserted by theory-page conformance test | `open` |
| F28 | 2.4.5 | Ch 6 maps layakari ratios onto Poly's per-lane subdivision. True layakari is the same phrase performed at 2×/3× speed; changing subdivision changes hit density. Conceptual simplification. | `P1` | `disclose` | `06-indian-classical.mdx` layakari section | M003/S03 | Simplification note present | `open` |
| F29 | 2.3.5, 2.3.3 | Poly's Kotekan parameter implements strict complementation; Tenzer shows practice overlaps at structural tones. The guide already states this honestly — confirm it survives the M004 patch edits. | `P1` | `accept` | `theory-gamelan.mdx` Rule 4 | M003/S03 | Existing honesty statement asserted by test so a future edit cannot drop it | `open` |
| F30 | 2.3.5 (Rule 5) | Gamelan Rule 5 says to choose one interlock style and never mix mid-phrase. Stronger than Tenzer, who allows stylistic mixing within a kebyar performance. | `P1` | `reframe` | `theory-gamelan.mdx` Rule 5 | M003/S03 | Hedged phrasing asserted by test | `open` |
| F31 | 2.1.4 | Rule 8's Humanize approximation is honestly flagged, but the guide does not say *how* different it is: Humanize is random jitter; Polak (2010) documents systematic, style-specific subdivision profiles. | `P1` | `disclose` | `theory-sub-saharan-africa.mdx` Rule 8 | M003/S04 | Explicit random-vs-systematic contrast asserted by test | `open` |
| F32 | 2.5.5 | The same non-isochrony honesty is owed to Balkan aksak (Goldberg 2015) as is given to the Malian jembe (Polak). | `P1` | `disclose` | `theory-balkan.mdx` | M003/S04 | Paired with F06 | `open` |
| F33 | 2.3.4 | Gamelan Rule 5 lists five named interlock styles but omits *kotekan polos* (a third player on structural pokok tones) — directly relevant to Poly users adding a third melodic lane. | `P2` | `enrich` | `theory-gamelan.mdx` Rule 5 | M003/S05 | Style named and linked to the M004/S03 pokok lane | `open` |
| F34 | 2.5.4 | Ch 7's Rachenitsa patch names tupan and kaval — pitched folk instruments — without noting that the MIDI notes are GM drum stand-ins. Readers on a GM kit will not hear the named instruments. | `P1` | `patch-align` | `07-balkan.mdx` — the Rachenitsa Groove patch table | M003/S05 | Patch-table note present; asserted by patch-conformance test | `open` |
| F35 | 2.3.1 | Ch 5 opens "time is not a line — it is a circle." Legitimate (Geertz 1980, and standard in gamelan texts) but uncited, and slightly orientalist unsourced. Tenzer (2000)'s cyclic-structure discussion grounds it. | `P1` | `source` | `05-gamelan.mdx` — the chapter opening paragraph | M003/S05 | Citation resolves | `open` |
| F36 | 2.2.3 | Whether rumba clave predates or postdates son clave is debated (Acosta 2004, Moore 2006). The audit judges that the guide may legitimately sidestep this under the repositioning frame. | `P1` | `accept` | — | M003/S01 | Recorded as a deliberate scope exclusion in the About page | `open` |

### M004 — Patch ↔ Theory Consistency

| ID | Audit § | Finding | Sev | Disp | Fix lands in | Slice | Verification | Status |
|---|---|---|---|---|---|---|---|---|
| F37 | 3.1, §5.5 | Ch 2 patch omits the timeline-mode bell lane (Rule 1) and the dance-beat lane (construction Step 2) that `theory-sub-saharan-africa` requires. | `P1` | `patch-align` | `02-sub-saharan-africa.mdx` patch | M004/S01 | Rule-compliance assertions in the extended `theory-patch-conformance.test.mjs` (F42) | `open` |
| F38 | 3.2, 2.2.4 | Ch 3's clave lane is the E(5,16) approximation while the theory page's is an exact timeline pattern. The chapter explains this, but the patch table header says only "Clave" and there is no cross-reference between the two constructions. | `P1` | `patch-align` | `03-afro-cuban.mdx` patch header | M004/S02 | Header carries the approximation marker; cross-reference link asserted by test | `open` |
| F39 | 2.2.5 | The theory-page tumbao lane (16 steps, 6 hits, rotation 14) satisfies the beat-1-avoidance rule but is an unusual configuration presented without its onset positions. | `P1` | `patch-align` | `theory-afro-cuban.mdx` Lane 2 | M004/S02 | Onset list rendered and checked against `bjorklund.ts` | `open` |
| F40 | 3.2 (Rule 5 row) | Ch 3 patch has conga at 10% mutation *and* quinto at 30% against the theory page's "one free voice only" rule — borderline. | `P1` | `patch-align` | `03-afro-cuban.mdx` patch | M004/S02 | Rule-compliance assertion, or documented divergence | `open` |
| F41 | 3.3, 2.3.5, §5.6 | Ch 5 kotekan patch has no pokok (structural melody) layer — required by `theory-gamelan` Rule 6 — and no structural overlap at the cycle boundary (Rule 4). | `P1` | `patch-align` | `05-gamelan.mdx` patch | M004/S03 | Rule-compliance assertion | `open` |
| F42 | §3 (preamble) | The existing CI tests verify that theory-page patches carry *valid* Euclidean triples but never that a patch follows its page's own named rules. That is the gap every row F37–F41 lives in. | `P1` | `patch-align` | `site/tests/theory-patch-conformance.test.mjs` | M004/S05 | Test extended with a per-page named-rule checklist; each rule is either satisfied or carries an in-band documented-divergence marker | `open` |
| F43 | 2.4.4 | The tihai discussion gives the principle but no worked example. Nelson (2008)'s formula — (Length × 3) + (Gap × 2) = beats remaining to sam — should be shown with real numbers. | `P1` | `enrich` | `theory-indian-classical.mdx` Rule 6 | M004/S04 | Worked example present; its arithmetic asserted by test | `open` |

### M005 — Literature Enrichment (P2)

| ID | Audit § | Missing source | Adds | Slice | Verification | Status |
|---|---|---|---|---|---|---|
| F44 | §6, 2.1.3 | Charry, E. (2000). *Mande Music*. | Corrects the Manding same-cycle oversimplification (F26) | M005/S01 | Cited inline in Ch 2 | `open` |
| F45 | §6, 2.1 | Kubik, G. (1999). *Africa and the Blues*. | African rhythmic retentions in diaspora music; bridges Ch 2 and Ch 4 | M005/S01 | Cited | `open` |
| F46 | §6, 2.7.1 | Agawu, K. (2003). *Representing African Music*. | Methodological critique of Western analysis of African rhythm; grounds F11 | M005/S01 | Cited | `open` |
| F47 | §6, 2.2.3 | Acosta, L. (2004). *Cubano Be, Cubano Bop*. | Historical depth on clave evolution | M005/S02 | Cited | `open` |
| F48 | §6, 2.4.6 | Powers, H. (1980). "India", *New Grove*; Kippen (1988) inline. | Carnatic tala system; theka elaboration | M005/S03 | Cited | `open` |
| F49 | §6 | Peycheva, L. & Dimov, V. | Bulgarian wedding-music scholarship; enriches Ch 7 | M005/S04 | Cited | `open` |
| F50 | §6 | Scherzinger, M. (2010). | A more critical account of the African–minimalist connection than Reich's own | M005/S05 | Cited | `open` |
| F51 | §6 | Born, G. & Hesmondhalgh, D. (2000). *Western Music and Its Others*. | Frame for why cross-cultural combination works or does not | M005/S05 | Cited | `open` |
| F52 | §6, 2.1 | Arom's later methodological writings. | Strengthens the sub-Saharan methodology section | M005/S01 | Cited | `open` |
| F53 | 2.8 (Brazilian) | Maracatu treatment is thin (accurate, but minimal). | Richer maracatu section | M005/S06 | Section expanded | `open` |

---

## Milestone breakdown

### M001 — Theory Corrections

**Vision:** Every claim in the guide that is factually wrong or overclaimed
relative to its source is corrected, and each correction is locked by a prose
claim test so it cannot regress.

| Slice | Scope | Findings |
|---|---|---|
| S01 | Ledger + conformance harness. Land this plan doc and `theory-audit-remediation.test.mjs`; extend `prose-pattern-claims`/`prose-conformance-claims` with the assertion helpers the later slices need. | infrastructure |
| S02 | Ch 6 rupak: E(4,7) → E(3,7) at the rotation that yields 3+2+2; reconcile prose with the existing patch. Same Ch 6 slice also lands the tintal-skeleton-as-analogy reframe and, in the euclidean-reference appendix, an explicit phase-convention statement so the E(3,16) row can be verified without re-deriving Bjorklund (closes issue #91). | F01, F12, F54 |
| S03 | Ch 8 Reich: separate *Drumming* (additive), *Piano/Violin Phase* (gradual phasing), *Clapping Music* (discrete jumps); re-anchor Drift to *Piano Phase*. | F02, F03 |
| S04 | Ch 7 Balkan: drop the independent-convergence claim; reconcile the kopanitsa grouping; add the non-integer long-beat caveat with Goldberg inline. | F04, F05, F06 |
| S05 | Ch 2: hedge the "centuries" claim; restore Toussaint's descriptive phrasing for E(7,12). | F07, F08 |
| S06 | Attribution care and remaining analogy/hedge items: Allen/Fela co-attribution, Allen timing as characterisation, Reich's African framing attributed to Reich, Linn attribution for the techno-vs-house-swing claim, "widely considered" hedge on the Amen break, Monson/Gridley sourcing for Roach, and the lcm-as-sam/gong translation caveat at each of its three sites. | F09, F10, F11, F13, F14, F15, F16 |

**Demo:** re-read the audit's Section 5 items 1, 3, 4, 9, 10, 11 against the
site; each is addressed at the cited line, and `node --test site/tests/` is green.

### M002 — Citation Integrity

**Vision:** No load-bearing theoretical claim in the guide is cited to a YouTube
video or a hobbyist blog, and a CI check keeps it that way.

| Slice | Scope | Findings |
|---|---|---|
| S01 | Verify ref [2] (Goldberg 2025 / MTO 31(2)); correct, replace, or mark forthcoming. | F17 |
| S02 | Ch 3: swap refs [10]/[11] for Peñalosa (2009). | F18 |
| S03 | Ch 4: swap refs [14]–[17] for Allen & Veal (2013), Veal (2000). | F19 |
| S04 | Ch 6: promote Clayton, Nelson, Kippen into the inline refs. | F20 |
| S05 | Ch 7: promote Brăiloiu, Rice, Goldberg into the inline refs. | F21 |
| S06 | Lomax citation for the Spanish-tinge claim; declare a tier on every reference; add `citation-tier.test.mjs`. | F22, F23 |

**Demo:** the audit's Section 4 Tier-C list is empty for inline citations; the
new tier test fails if a Tier-B/C source is attached to a named-theory claim.

### M003 — Scope and Repositioning

**Vision:** The guide states plainly what it is, what it is not, and exactly
where it simplifies — so a reader can calibrate every claim it makes.

| Slice | Scope | Findings |
|---|---|---|
| S01 | "About This Guide" page carrying the audit's repositioning statement; linked from the introduction and all 12 theory pages; records deliberate scope exclusions. | F24, F36 |
| S02 | Ch 6 scope limited to Hindustani practice, with the Carnatic absence stated. | F25 |
| S03 | Simplification disclosures: Manding cycles, polos/sangsih style-dependence, layakari vs. subdivision, kotekan strict complementation, interlock-style mixing. | F26–F30 |
| S04 | Non-isochrony honesty: how Humanize differs from Polak's systematic profiles; the same for aksak long beats. | F31, F32 |
| S05 | Remaining framing items: kotekan polos, GM stand-in note for tupan/kaval, Tenzer citation for the cyclic-time opening. | F33, F34, F35 |

**Demo:** the audit's Section 5 repositioning statement is live on the site and
each simplification it names is disclosed at the point of use.

### M004 — Patch ↔ Theory Consistency

**Vision:** Every chapter patch either follows its companion theory page's named
rules or carries an in-band note explaining why it deliberately does not — and
CI enforces the choice.

| Slice | Scope | Findings |
|---|---|---|
| S01 | Ch 2 patch: timeline-mode bell lane and dance-beat lane, or a documented cross-reference to the fuller theory-page construction. | F37 |
| S02 | Ch 3 patch: approximation marker on the clave header, cross-reference to the exact-timeline construction, tumbao onset positions, one-free-voice reconciliation. | F38, F39, F40 |
| S03 | Ch 5 patch: pokok layer and structural overlap at the cycle boundary. | F41 |
| S04 | Worked tihai example using Nelson's formula. | F43 |
| S05 | Extend `theory-patch-conformance.test.mjs` from "valid triples" to a per-page named-rule checklist with an in-band documented-divergence escape hatch. | F42 |

**Demo:** re-run the audit's Section 3 tables against the site; every ❌ and ⚠️
row is either ✅ or carries a documented divergence.

### M005 — Literature Enrichment (P2, deferrable)

**Vision:** The bibliography carries the sources the audit identifies as the
highest-value additions per tradition.

| Slice | Scope | Findings |
|---|---|---|
| S01 | Sub-Saharan: Charry, Kubik, Agawu, later Arom. | F44, F45, F46, F52 |
| S02 | Afro-Cuban: Acosta. | F47 |
| S03 | Indian: Powers (New Grove), Kippen inline. | F48 |
| S04 | Balkan: Peycheva & Dimov. | F49 |
| S05 | Minimalism / Electronic: Scherzinger, Born & Hesmondhalgh. | F50, F51 |
| S06 | Brazilian: richer maracatu treatment. | F53 |

**Demo:** the audit's Section 6 table has no unaddressed row.

---

## Sequencing and dependencies

```
M001 (P0 corrections) ──┬──> M003 (repositioning; cites the corrected claims)
                        │
M002 (citations) ───────┴──> M004 (patch alignment; asserts against corrected rules)
                                        │
                                        └──> M005 (enrichment; optional)
```

- **M001/S01 is the hard prerequisite for everything** — it lands the ledger and
  the conformance harness that every later slice writes its verification into.
- **M001 and M002 are independent of each other** and can run in parallel, except
  that F06/F21 (Goldberg inline for Balkan) and F11/F46 (Agawu) touch the same
  paragraphs; sequence M001/S04 before M002/S05.
- **M003 depends on M001** — the repositioning statement should describe a guide
  whose factual errors are already fixed, not promise them.
- **M004/S05 depends on M003/S03**, because the named-rule checklist encodes the
  rules as reworded there.
- **M005 is deferrable** without leaving the audit unaddressed: every P0 and P1
  row closes in M001–M004. M005 closes the P2 enrichment rows.

## What "clean sheet" means at re-review

The audit is re-runnable. After M001–M004 land, a re-read should find:

| Audit artifact | Expected post-implementation state |
|---|---|
| §5 numbered items 1–12 | All twelve addressed at the cited locations |
| §2 per-tradition issue lists | Every issue either corrected, sourced, or explicitly disclosed as a simplification |
| §3 consistency tables | No ❌ rows; ⚠️ rows resolved or documented as intentional divergence |
| §4 Tier C list | No Tier-C source cited inline for a named-theory claim; ref [2] resolved |
| §6 enrichment table | Closed by M005; explicitly deferred (not silently dropped) if M005 is postponed |
| §7 score table | Both 🔴 rows (Indian Classical, Minimalism) → 🟢; 🔴 reference list → 🟢; 🟡 rows → 🟢 or 🟡-by-declared-scope |

The mechanical half of that verdict is `docs/audits/theory-audit-remediation.test.mjs`
plus the per-claim tests each slice adds. The editorial half is a human re-read —
which is why every row names a *location*, not just a fix.

## Related open issues

Every currently-open GitHub issue that overlaps this plan's remit is enumerated
here so the two views cannot silently drift. A row is either **closed by** a
ledger finding (the finding's verification, when it lands, is what will close
the issue) or **not closed here** — retained on the issue tracker because it is
an engine or preset change that this documentation-and-tests plan deliberately
does not build.

| Issue | Ledger row | Relationship |
|---|---|---|
| [#91](https://github.com/JimAKennedy/poly/issues/91) | F54 | Closed by F54. The E(3,16) appendix row is correct under Poly's phase convention; F54 makes that convention explicit next to the table so a reader can verify without re-deriving Bjorklund. |
| [#156](https://github.com/JimAKennedy/poly/issues/156) | F31 | Not closed here. F31 discloses that Humanize applies random jitter rather than the systematic, style-specific subdivision profiles Polak (2010) documents; the exact non-Euclidean-timeline preset work #156 tracks stays on the issue tracker. |
| [#157](https://github.com/JimAKennedy/poly/issues/157) | F32 | Not closed here. F32 pairs the Balkan long-beat honesty note with F06 (Goldberg inline); cell-aware swing for aksak — a real engine change — stays on the issue tracker as #157. |

## Out of scope

- **Engine or plugin behaviour changes.** Every row is documentation, website,
  or test. Two findings (F31 Humanize-vs-systematic-microtiming, F34 GM stand-in
  notes) point at real engine limitations; this plan *discloses* them rather than
  building features. Existing enhancement issues #156 (exact non-Euclidean
  timelines as presets) and #157 (cell-aware swing for aksak) are where those
  would land, and are unchanged by this plan.
- **A comprehensive ethnomusicology review.** The audit's own conclusion is that
  the guide should not be positioned as one. M003/S01 states that in the guide.
