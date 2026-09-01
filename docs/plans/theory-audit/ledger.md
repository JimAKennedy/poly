---
class: gated
---

# Theory Audit Remediation Ledger

Status: current (2026-08-28)

**Source:** `docs/audits/poly_theory_audit.md` — the August 2026 external
music-theory audit of the Poly Guide. Every finding in that document (the twelve
ranked corrective items in its Section 5, every sub-finding in Sections 1–4, and
every enrichment source in Section 6) appears below as exactly one row, F01–F54.
Rows H01–H04 are the harness this programme needs to lock its own corrections.

**Superseded:** `docs/audits/M001-theory-audit-remediation-plan.md` was this
programme's plan of record until this ledger existed. M001/S07 archives it and
repoints its completeness test at this file.

**Row vocabulary.** Beyond the required columns, each row carries a severity
(`P0` factually wrong · `P1` overclaimed or under-sourced · `P2` enrichment) and
a disposition (`correct` · `source` · `reframe` · `disclose` · `patch-align` ·
`enrich` · `verify`). Both are informational; the `ledger` check ignores them.

**Numbering.** Milestone IDs M001–M005 collide with Poly's legacy
commit-message milestones from the plugin work. The `Plan:` trailer
disambiguates: `git log --grep="Plan: docs/plans/theory-audit"` selects this
programme's commits and nothing else.

---

## Milestone M001 — Theory Corrections

**Vision:** Every claim in the guide that is factually wrong or overclaimed
relative to its source is corrected, and each correction is locked by a named
test case so it cannot silently regress.
**Branch:** milestone/M001-theory-corrections
**Status:** done
**Demo:** Re-read the audit's Section 5 items 1, 3, 4, 9, 10, 11 against the
site; each is addressed at the cited location, and `npm --prefix site test` is
green.

### Slice M001/S01 — Ledger and conformance harness

**Plan:** —  <!-- ledger-ok: landed as PR #256 before this ledger existed; the PR is the plan of record -->
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] Every audit finding is enumerated as exactly one row, and a mechanical
      check fails if one goes missing or gains a second home
- [x] Later slices have prose-claim assertion helpers to write their locks
      against, rather than each inventing its own matcher
- [x] The doc-conformance script runs the new harness

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| H01 | The audit's findings have no enumerated, mechanically checked home | `P1` | `enrich` | `docs/audits/theory-audit-remediation.test.mjs` | `node --test docs/audits/theory-audit-remediation.test.mjs` — 7 cases, asserting F01–F54 appear exactly once with one valid severity, disposition and status token each | `done` |
| H02 | Prose-claim locks would each hand-roll their own matching | `P1` | `enrich` | `site/tests/helpers/prose-claims.mjs` | `site/tests/prose-claim-helpers.test.mjs` exercises the section-scoping and proximity matchers the later slices use | `done` |

### Slice M001/S02 — Chapter 6 rupak, tintal analogy, appendix phase convention

**Plan:** —  <!-- ledger-ok: landed as PR #256 before this ledger existed; the PR is the plan of record -->
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S02.md
**Status:** done

**Definition of Done**

- [x] Chapter 6's rupak illustration and its companion patch no longer
      contradict each other
- [x] The E(7,16) tintal passage reads as an analogy, not as grounding
- [x] A reader can verify the appendix's E(3,16) row from the stated phase
      convention without re-deriving Bjorklund

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F01 | Ch 6 illustrates rupak's 3+2+2 with E(4,7), whose grouping is 2+2+2+1, while the chapter's own patch uses E(3,7) — prose and patch contradict | `P0` | `correct` | `06-indian-classical.mdx` | Case `S02-F01` in `site/tests/theory-audit-claims.test.mjs` forbids `E(4,7)` in the rupak illustration and pins the diagram to `steps={7} hits={3} rotation={3}`, checked against `site/src/audio/bjorklund.ts` | `done` |
| F12 | Ch 6 grounds E(7,16) as "the kind of structure a tabla player might outline during a slow theka"; thekas are fixed named bol sequences, not Euclidean distributions | `P1` | `reframe` | `06-indian-classical.mdx` | Case `S02-F12` forbids the pre-correction grounding phrase and requires the analogy framing ("not itself a tintal theka", "thekas are fixed, named bol sequences", "rough analogue") | `done` |
| F54 | The appendix E(3,16) row reads as wrong to a reader deriving Bjorklund from scratch because Poly's phase convention is nowhere stated (issue #91) | `P1` | `disclose` | `appendix-euclidean-reference.mdx` | Case `S02-F54` asserts the "Phase Convention" preamble (rotate-right, rotation-0 canonical, worked example "onsets at steps 0, 5, 10") and an arithmetic case re-derives the printed row from `bjorklund(16, 3)` | `done` |

### Slice M001/S03 — Chapter 8 Reich techniques

**Plan:** —  <!-- ledger-ok: landed as PR #256 before this ledger existed; the PR is the plan of record -->
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S03.md
**Status:** done

**Definition of Done**

- [x] *Drumming*, *Piano Phase* and *Clapping Music* are each named with their
      own technique, and no longer conflated
- [x] Poly's Drift is anchored to the work whose technique it actually models

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F02 | Ch 8 attributes gradual phase shifting to *Drumming* (1971), an additive/subtractive process; gradual phasing is *Piano Phase* / *Violin Phase* (1967) and *Clapping Music* (1972) uses discrete jumps | `P0` | `correct` | `08-minimalism.mdx` | Case `S03-F02` forbids *Drumming* citations followed within ~120 chars by phase-shifting language, pinning each named Reich work to its own technique term | `done` |
| F03 | Poly's Drift is described as modelling *Drumming*; Drift gradually shifts cycle position, which models *Piano Phase* | `P0` | `correct` | `08-minimalism.mdx` Drift section | Section-scoped case `S03-F03` extracts `## Drift as Phase Engine` and asserts *Piano Phase* is cited while *Drumming* is not | `done` |

### Slice M001/S04 — Chapter 7 Balkan corrections

**Plan:** —  <!-- ledger-ok: landed as PR #256 before this ledger existed; the PR is the plan of record -->
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S04.md
**Status:** done

**Definition of Done**

- [x] The Bjorklund/Balkan relationship is stated as mathematical, with
      Toussaint credited for identifying it
- [x] The chapter names one primary kopanitsa grouping and does not contradict
      itself between paragraphs
- [x] The aksak long beat is no longer asserted as an exact ratio

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F04 | Ch 7 says Bjorklund and Balkan musicians "independently arrived at the same solution"; Bjorklund (2003) derived it for neutron-beam timing and Toussaint (2004/2005) identified the musical connection | `P0` | `reframe` | `07-balkan.mdx` | Case `S04-F04` forbids the independent-convergence phrasing and requires the Toussaint-identified (ref-1) and Bjorklund (ref-46) framing | `done` |
| F05 | Ch 7 presents E(4,11)=3+3+3+2 as the kopanitsa grouping while the same chapter states the primary form is 2+2+3+2+2 | `P1` | `correct` | `07-balkan.mdx` | Case `S04-F05` asserts the Daichovo/Kopanitsa paragraph names 2+2+3+2+2 as primary with E(4,11) as a variant, and `S04-F05-arith` re-derives the E(5,11) and E(4,11) spellings from `bjorklund.ts` | `done` |
| F06 | Ch 7 implies aksak long beats are exactly 1.5× short beats; Goldberg (2015) shows performed long beats deviate systematically | `P1` | `disclose` | `07-balkan.mdx` | Case `S04-F06` forbids assertive "exactly 1.5×" / "exactly 3:2" phrasing and requires an inline `fr-goldberg-2015` citation on the long-beat caveat | `done` |

### Slice M001/S05 — Chapter 2 hedges

**Depends:** M001/S01
**Plan:** —  <!-- ledger-ok: landed as PR #258 before this ledger existed; the PR is the plan of record -->
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S05.md
**Status:** done

**Definition of Done**

- [x] Chapter 2's opening no longer asserts multi-century continuity in the
      guide's own voice
- [x] The gankogui claim uses the source's descriptive phrasing
- [x] Both are locked by named test cases so an edit cannot silently regress

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F07 | Ch 2 states Ewe ensemble music has been organised this way "for centuries", unsourced; fieldwork documentation begins mid-twentieth century | `P1` | `reframe` | `02-sub-saharan-africa.mdx` | Case `S05-F07` forbids unhedged multi-century phrasing in the chapter opening and requires the documentation-since framing with Jones (ref-6) and Locke | `done` |
| F08 | Ch 2 calls E(7,12) "the single most important timeline pattern"; Toussaint (2005) says "most commonly used" — descriptive, not evaluative | `P1` | `reframe` | `02-sub-saharan-africa.mdx`, `appendix-references.mdx` | Cases `S05-F08` and `S05-F08-appendix` assert the descriptive phrasing and that the claim resolves to Toussaint (ref-1) rather than the retired Wikipedia ref-6 | `done` |

### Slice M001/S06 — Attribution care and remaining hedges

**Depends:** M001/S01
**Plan:** M001-S06-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S06.md
**Status:** done

**Definition of Done**

- [x] Ch 4 credits Fela and Allen jointly rather than assigning the rhythmic
      vocabulary to one of them
- [x] Ch 4's Allen timing claim reads as characterisation, not measurement
- [x] Ch 8 attributes the "known for centuries" framing to Reich rather than
      asserting it in the guide's own voice
- [x] Ch 9's techno-vs-house swing claim is attributed to Linn and cross-refers
      to Butler's contrary reading
- [x] Ch 13's Amen-break superlative is hedged
- [x] Ch 12's Roach independence claim carries a scholarly citation
- [x] The lcm-as-sam / lcm-as-gong translation is marked as the guide's own
      informal translation at all three sites
- [x] Each of the seven is locked by a named case in `theory-audit-claims.test.mjs`

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F09 | Ch 4: "Fela built the genre… but it was Tony Allen who created its rhythmic vocabulary" — contentious; Allen himself co-attributes | `P1` | `reframe` | `04-afrobeat.mdx` chapter opening | Case `S06-F09` asserts co-attribution language and forbids the sole-creator phrasing | `done` |
| F10 | Ch 4: "Tony Allen's timing was precise but not quantised" — received wisdom presented as measured fact; no measurement study covers Allen recordings | `P1` | `reframe` | `04-afrobeat.mdx` Humanize bullet | Case `S06-F10` asserts the claim is marked as characterisation and forbids measurement-asserting phrasing | `done` |
| F11 | Ch 8 asserts in its own voice that West African and Indonesian musicians "had known for centuries" what the minimalists discovered; Reich's analogy was criticised (Agawu 2003) | `P1` | `reframe` | `08-minimalism.mdx` chapter opening | Case `S06-F11` asserts the framing is attributed to Reich and that the African-stacking / gamelan-nesting distinction is stated | `done` |
| F13 | Ch 9 states the techno-vs-house difference "is often reducible to one parameter: swing" as fact, cited to a Roger Linn blog; contested by Butler (2006) | `P1` | `source` | `09-electronic.mdx` | Case `S06-F13` asserts inline attribution to Linn and the presence of a Butler cross-reference | `done` |
| F14 | Ch 13 calls the Amen break "the most sampled recording in music history" — disputed | `P1` | `reframe` | `13-drum-and-bass.mdx` | Case `S06-F14` asserts a "widely considered"-class hedge on the superlative | `done` |
| F15 | Ch 12's Max Roach polymetric-independence claim is directionally right but uncited; Monson (1996) and Gridley are the authorities | `P1` | `source` | `12-jazz.mdx` | Case `S06-F15` asserts the Roach passage carries an inline `fr-monson-1996` citation that resolves in the reference appendix | `done` |
| F16 | The guide translates lcm convergence as "sam" (Indian) and "gong stroke" (gamelan); neither tradition uses the term that way internally | `P1` | `disclose` | `01-foundations.mdx`, `06-indian-classical.mdx`, `05-gamelan.mdx` | Case `S06-F16` asserts a translation caveat is present at each of the three sites | `done` |

### Slice M001/S07 — Harness migration to the ledger

**Plan:** M001-S07-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M001-S07.md
**Status:** done

**Definition of Done**

- [x] `theory-audit-remediation.test.mjs` reads this ledger, not the retired
      plan doc, and still fails if any of F01–F54 goes missing or gains a
      second home
- [x] The test asserts rows sit under a real slice by nesting, with no
      cross-referencing slice column
- [x] The three Related-issues guards survive the migration, rewritten against
      this ledger's list form: every issue cites a row that resolves, no issue
      is both listed and declared deliberately absent, and "Out of scope" cites
      only issues the list carries
- [x] `docs/audits/M001-theory-audit-remediation-plan.md` is `class: archived`
      and names this ledger as its successor
- [x] `jk-standards ledger` and `jk-standards doc-taxonomy` both pass

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| H03 | The completeness test reads the retired plan doc, so the ledger and the plan doc are two sources of truth for the same 54 findings | `P1` | `correct` | `docs/audits/theory-audit-remediation.test.mjs` | The test's fixture path is `docs/plans/theory-audit/ledger.md` and all 11 of its cases still run — 8 finding-completeness and 3 Related-issues guards; deleting any F-row from the ledger fails it | `done` |
| H04 | The retired plan doc still presents as current, so a reader may act on its stale statuses | `P1` | `disclose` | `docs/audits/M001-theory-audit-remediation-plan.md` | `jk-standards doc-taxonomy` accepts the `archived` class and the doc names its successor | `done` |

---

## Milestone M002 — Citation Integrity

**Vision:** No load-bearing theoretical claim in the guide is cited to a video
or a hobbyist blog, and a check keeps it that way.
**Branch:** milestone/M002-citation-integrity
**Status:** in-progress
**Demo:** The audit's Section 4 Tier-C list is empty for inline citations, and
the new tier test fails if a Tier-B or Tier-C source is attached to a
named-theory claim.

### Slice M002/S01 — Reference [2] resolution

**Plan:** M002-S01-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S01.md
**Status:** in-progress

**Definition of Done**

- [ ] Reference [2]'s publication status is established from the publisher, and
      the finding records which it was
- [ ] The appendix entry states the resolved status: real, replaced, or marked
      forthcoming

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F17 | Ref [2] cites Goldberg (2025), *Music Theory Online* 31(2); MTO is current through Vol 30 as of mid-2026, so the URL is a forward reference and may be a citation error | `P0` | `verify` | `appendix-references.mdx` `ref-2` | Resolution recorded in this row; the appendix entry either cites a real issue or is marked forthcoming, asserted by a case in `citation-tier.test.mjs` | `open` |

### Slice M002/S02 — Chapter 3 clave citations

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S02.md
**Status:** open

**Definition of Done**

- [ ] The clave-matrix and non-Euclidean-gap claims cite Peñalosa (2009) inline
- [ ] Refs [10] and [11] no longer carry a named-theory claim in Chapter 3

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F18 | Refs [10] and [11] — YouTube videos — are the inline citations for the clave matrix, the most important theoretical claim in Ch 3, while Peñalosa (2009) sits unused in Further Reading | `P0` | `source` | `03-afro-cuban.mdx`, `appendix-references.mdx` | The Tier-A citation check (F23) passes for Ch 3 | `open` |

### Slice M002/S03 — Chapter 4 Afrobeat citations

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S03.md
**Status:** open

**Definition of Done**

- [ ] Chapter 4's opening and phrase-gating claims cite Allen & Veal (2013) or
      Veal (2000) inline
- [ ] Refs [14]–[17] no longer carry a named-theory claim in Chapter 4

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F19 | Refs [14]–[17] for Afrobeat are YouTube videos and production blogs, while Allen & Veal (2013) and Veal (2000) sit unused in Further Reading | `P0` | `source` | `04-afrobeat.mdx`, `appendix-references.mdx` | The Tier-A citation check passes for Ch 4 | `open` |

### Slice M002/S04 — Chapter 6 Indian-classical citations

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S04.md
**Status:** open

**Definition of Done**

- [ ] Chapter 6's tala, laya and theka claims cite Clayton (2000), Nelson
      (2008) or Kippen (1988) inline
- [ ] Refs [21]–[25] no longer carry a named-theory claim in Chapter 6

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F20 | Refs [21]–[25] for Ch 6 include a commercial blog, a high-school textbook PDF and a YouTube konnakol video, while Clayton, Nelson and Kippen sit unused in Further Reading | `P1` | `source` | `06-indian-classical.mdx`, `appendix-references.mdx` | The Tier-A citation check passes for Ch 6 | `open` |

### Slice M002/S05 — Chapter 7 Balkan citations

**Depends:** M001/S04
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S05.md
**Status:** open

**Definition of Done**

- [ ] The aksak-definition and svatbarska-muzika claims cite Brăiloiu (1951),
      Rice (1994) or Goldberg (2015) inline
- [ ] Refs [26] and [27] no longer carry a named-theory claim in Chapter 7
- [ ] The `S04-F06` long-beat lock still passes after the citation edits

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F21 | Refs [26] and [27] for Balkan are educational aggregator pages, while Brăiloiu, Rice and Goldberg sit in Further Reading only | `P1` | `source` | `07-balkan.mdx`, `appendix-references.mdx` | The Tier-A citation check passes for Ch 7, and case `S04-F06` still passes | `open` |

### Slice M002/S06 — Reference tiers and the tier check

**Depends:** M002/S01, M002/S02, M002/S03, M002/S04, M002/S05
**Validation:** format, site-unit, doc-conformance, doc-discipline, gate
**Evidence:** evidence/M002-S06.md
**Status:** open

**Definition of Done**

- [ ] Every entry in the reference appendix carries a declared tier
- [ ] The "Spanish tinge" attribution cites Lomax's Morton interviews
- [ ] A new check fails when a Tier-B or Tier-C source is the inline citation
      for a named-theory claim
- [ ] The check is wired into `scripts/check-doc-conformance.sh`

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F22 | Ch 3's "the habanera rhythm that Jelly Roll Morton called 'the Spanish tinge'" is accurate but uncited; Lomax's Morton interviews (1950) are the primary source | `P1` | `source` | `03-afro-cuban.mdx` | The Lomax citation resolves in the appendix, asserted by `citation-tier.test.mjs` | `open` |
| F23 | The reference list mixes peer-reviewed scholarship and hobbyist media at equal citation weight, with no mechanism preventing regression | `P1` | `source` | `appendix-references.mdx`, `site/tests/citation-tier.test.mjs` | `citation-tier.test.mjs` asserts every reference carries a declared tier and every inline citation on a named-theory claim resolves to a Tier-A source; `doc-conformance-wiring.test.mjs` asserts the script runs it | `open` |

---

## Milestone M003 — Scope and Repositioning

**Vision:** The guide states plainly what it is, what it is not, and exactly
where it simplifies, so a reader can calibrate every claim it makes.
**Branch:** milestone/M003-scope-repositioning
**Status:** planned
**Demo:** The audit's Section 5 repositioning statement is live on the site, and
each simplification it names is disclosed at the point of use.

### Slice M003/S01 — About This Guide

**Depends:** M001/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S01.md
**Status:** open

**Definition of Done**

- [ ] An "About This Guide" page carries the audit's repositioning statement
- [ ] It is reachable from the introduction and from all twelve `theory-*.mdx`
      pages
- [ ] It names the guide's deliberate scope exclusions, including the
      son-clave/rumba-clave precedence debate

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F24 | The guide has no statement of what it is and is not; the audit supplies a four-paragraph repositioning statement and judges it defensible | `P1` | `reframe` | New About page, `introduction.mdx`, every `theory-*.mdx` | Case `S01-F24` asserts the About page exists and is linked from the introduction and all twelve theory pages | `open` |
| F36 | Whether rumba clave predates or postdates son clave is debated (Acosta 2004, Moore 2006); the audit judges the guide may legitimately sidestep it under the repositioning frame | `P1` | `accept` | New About page | Case `S01-F36` asserts the exclusion is named on the About page, so the sidestep is declared rather than silent | `open` |

### Slice M003/S02 — Chapter 6 scope

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S02.md
**Status:** open

**Definition of Done**

- [ ] Chapter 6's front-matter description and its in-page scope note both say
      Hindustani, and neither promises Carnatic coverage
- [ ] The absence of the Carnatic tala system is stated rather than implied

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F25 | Ch 6 is described as covering "Hindustani and Carnatic" but the Carnatic tala system — solkattu/konnakol, different tala families, a different conceptual frame — is absent | `P1` | `correct` | `06-indian-classical.mdx` front matter and scope note | Case `S02-F25` asserts the description and scope note agree on Hindustani-only and that the Carnatic absence is stated | `open` |

### Slice M003/S03 — Simplification disclosures

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S03.md
**Status:** open

**Definition of Done**

- [ ] The Manding same-cycle presentation is marked as a pedagogical flattening
      and cites Charry
- [ ] Gamelan Rule 3's polos/sangsih assignment is marked style-dependent, with
      *norot* named as the reversing case
- [ ] The layakari section says plainly that Poly's subdivision change is not
      what layakari does to hit density
- [ ] Gamelan Rule 5's "choose one and keep it" is hedged to match Tenzer
- [ ] Gamelan Rule 4's existing strict-complementation honesty is locked by a
      test, so a later edit cannot drop it

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F26 | Ch 2 characterises Manding ensembles as all sharing one cycle length; Manding dunun ensembles do use distinct lengths in many contexts. Pedagogical flattening, not error | `P1` | `disclose` | `02-sub-saharan-africa.mdx` "Manding Traditions" | Case `S03-F26` asserts the simplification note is present and cites Charry (2000), which F44 adds | `open` |
| F27 | Gamelan Rule 3 states polos leans onbeat and sangsih offbeat; in *norot* the relationship is effectively reversed. Style-dependent, presented as general | `P1` | `disclose` | `theory-gamelan.mdx` Rule 3 | Case `S03-F27` asserts the style-dependence caveat naming *norot* | `open` |
| F28 | Ch 6 maps layakari ratios onto Poly's per-lane subdivision; true layakari is the same phrase at 2×/3× speed, and changing subdivision changes hit density | `P1` | `disclose` | `06-indian-classical.mdx` layakari section | Case `S03-F28` asserts the simplification note distinguishing phrase-speed from hit density | `open` |
| F29 | `theory-gamelan.mdx` Rule 4 already states honestly that strict complementation is only the textbook case; nothing stops a later edit removing it | `P1` | `accept` | `theory-gamelan.mdx` Rule 4 | Case `S03-F29` extracts Rule 4 and asserts the existing overlap-at-structural-tones honesty statement and its Tenzer citation survive | `open` |
| F30 | Gamelan Rule 5 says to choose one interlock style and never mix mid-phrase — stronger than Tenzer, who allows stylistic mixing within a kebyar performance | `P1` | `reframe` | `theory-gamelan.mdx` Rule 5 | Case `S03-F30` asserts hedged phrasing and forbids the absolute prohibition | `open` |

### Slice M003/S04 — Non-isochrony honesty

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S04.md
**Status:** open

**Definition of Done**

- [ ] `theory-sub-saharan-africa.mdx` states explicitly that Humanize is random
      jitter, against Polak's systematic style-specific profiles — not merely
      that it is an approximation
- [ ] Both non-isochrony disclosures are locked by tests, so the Balkan one
      already in the tree cannot be dropped

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F31 | Rule 8's Humanize approximation is flagged as an approximation but the guide never says *how* it differs: Humanize is random jitter, Polak (2010) documents systematic style-specific subdivision profiles | `P1` | `disclose` | `theory-sub-saharan-africa.mdx` Rule 8 and construction step 5 | Case `S04-F31` asserts the explicit random-versus-systematic contrast, not just the word "approximation" | `open` |
| F32 | The audit asks that Balkan aksak be given the same non-isochrony honesty as the Malian jembe; `theory-balkan.mdx` Rule 8 already gives it, citing Goldberg (2015), but nothing locks it | `P1` | `disclose` | `theory-balkan.mdx` Rule 8 | Case `S04-F32` extracts Rule 8 and asserts the sub-3:2 statement, its `fr-goldberg-2015` citation, and the notated-versus-played disclosure survive | `open` |

### Slice M003/S05 — Remaining framing items

**Validation:** format, site-unit, doc-conformance, gate
**Evidence:** evidence/M003-S05.md
**Status:** open

**Definition of Done**

- [ ] Gamelan Rule 5 names *kotekan polos* among its interlock styles and links
      it to the pokok lane
- [ ] The Rachenitsa patch table says its tupan and kaval notes are GM drum
      stand-ins, not the named instruments
- [ ] Chapter 5's cyclic-time opening carries a citation

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F33 | Gamelan Rule 5's named interlock styles omit *kotekan polos* — a third player on structural pokok tones, directly relevant to a user adding a third melodic lane | `P2` | `enrich` | `theory-gamelan.mdx` Rule 5 | Case `S05-F33` asserts the style is named and cross-links to the M004/S03 pokok lane | `open` |
| F34 | Ch 7's Rachenitsa patch names tupan and kaval — pitched folk instruments — without noting the MIDI notes are GM drum stand-ins | `P1` | `patch-align` | `07-balkan.mdx` Rachenitsa patch table | The patch-conformance suite asserts the stand-in note is present on the patch table | `open` |
| F35 | Ch 5 opens "time is not a line — it is a circle": legitimate but uncited, and unsourced it reads as orientalist; Tenzer (2000) grounds it | `P1` | `source` | `05-gamelan.mdx` chapter opening | Case `S05-F35` asserts the opening carries a citation that resolves in the appendix | `open` |

---

## Milestone M004 — Patch and Theory Consistency

**Vision:** Every chapter patch either follows its companion theory page's named
rules or carries an in-band note explaining why it deliberately does not, and CI
enforces the choice.
**Branch:** milestone/M004-patch-theory-consistency
**Status:** planned
**Demo:** Re-run the audit's Section 3 tables against the site; every ❌ and ⚠️
row is either ✅ or carries a documented divergence.

### Slice M004/S01 — Chapter 2 patch

**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S01.md
**Status:** open

**Definition of Done**

- [ ] The Chapter 2 patch either carries the timeline-mode bell lane and the
      dance-beat lane its theory page requires, or cross-references the fuller
      theory-page construction in band
- [ ] The choice is legible from the patch table alone

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F37 | The Ch 2 patch omits the timeline-mode bell lane (Rule 1) and the dance-beat lane (construction Step 2) that `theory-sub-saharan-africa` requires | `P1` | `patch-align` | `02-sub-saharan-africa.mdx` patch | Rule-compliance assertion in the extended `theory-patch-conformance.test.mjs` (F42), or a documented-divergence marker it accepts | `open` |

### Slice M004/S02 — Chapter 3 patch

**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S02.md
**Status:** open

**Definition of Done**

- [ ] The Chapter 3 clave lane header marks itself as the Euclidean
      approximation and links to the exact-timeline construction
- [ ] The theory-page tumbao lane's onset positions are rendered, not left to
      the reader to derive
- [ ] The conga and quinto mutation settings either satisfy the one-free-voice
      rule or carry a divergence note

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F38 | Ch 3's clave lane is the E(5,16) approximation while the theory page's is an exact timeline pattern; the patch header says only "Clave" with no cross-reference | `P1` | `patch-align` | `03-afro-cuban.mdx` patch header | The header carries the approximation marker and the cross-reference link, asserted by the patch-conformance suite | `open` |
| F39 | The theory-page tumbao lane (16 steps, 6 hits, rotation 14) satisfies the beat-1-avoidance rule but is an unusual configuration presented without its onset positions | `P1` | `patch-align` | `theory-afro-cuban.mdx` Lane 2 | The onset list is rendered and checked against `site/src/audio/bjorklund.ts` | `open` |
| F40 | Ch 3's patch has conga at 10% mutation and quinto at 30% against the theory page's one-free-voice rule | `P1` | `patch-align` | `03-afro-cuban.mdx` patch | Rule-compliance assertion in the patch-conformance suite, or a documented divergence it accepts | `open` |

### Slice M004/S03 — Chapter 5 patch

**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S03.md
**Status:** open

**Definition of Done**

- [ ] The Chapter 5 kotekan patch carries a pokok layer, as `theory-gamelan`
      Rule 6 requires
- [ ] It carries structural overlap at the cycle boundary, as Rule 4 requires

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F41 | The Ch 5 kotekan patch has no pokok (structural melody) layer, required by `theory-gamelan` Rule 6, and no structural overlap at the cycle boundary, required by Rule 4 | `P1` | `patch-align` | `05-gamelan.mdx` patch | Rule-compliance assertion for Rules 4 and 6 in the patch-conformance suite | `open` |

### Slice M004/S04 — Tihai worked example

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M004-S04.md
**Status:** open

**Definition of Done**

- [ ] The tihai discussion shows Nelson's formula with real numbers, not just
      the principle
- [ ] The arithmetic in the example is checked, not asserted

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F43 | The tihai discussion gives the principle but no worked example; Nelson (2008)'s formula — (Length × 3) + (Gap × 2) = beats remaining to sam — should be shown with real numbers | `P1` | `enrich` | `theory-indian-classical.mdx` Rule 6 | An arithmetic case re-derives the printed worked example from the formula, alongside the existing tihai case in `theory-patch-conformance.test.mjs` | `open` |

### Slice M004/S05 — Named-rule conformance checklist

**Depends:** M003/S03, M004/S01, M004/S02, M004/S03, M004/S04
**Validation:** format, site-unit, doc-conformance, doc-discipline, gate
**Evidence:** evidence/M004-S05.md
**Status:** open

**Definition of Done**

- [ ] `theory-patch-conformance.test.mjs` checks each theory page's patches
      against that page's own named rules, not only that their Euclidean
      triples are valid
- [ ] A rule a patch deliberately breaks is satisfied by an in-band divergence
      marker carrying a written reason, and by nothing else
- [ ] A patch that silently drops a rule fails the suite

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F42 | The CI tests verify theory-page patches carry valid Euclidean triples but never that a patch follows its page's own named rules — the gap every one of F37–F41 lives in | `P1` | `patch-align` | `site/tests/theory-patch-conformance.test.mjs` | The suite carries a per-page named-rule checklist; removing a required lane from any chapter patch fails it, and adding the divergence marker passes it | `open` |

---

## Milestone M005 — Literature Enrichment

**Vision:** The bibliography carries the sources the audit identifies as the
highest-value additions per tradition, each cited at the claim it supports.
**Branch:** milestone/M005-literature-enrichment
**Status:** planned
**Demo:** The audit's Section 6 table has no unaddressed row.

### Slice M005/S01 — Sub-Saharan sources

**Depends:** M002/S06, M003/S03
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S01.md
**Status:** open

**Definition of Done**

- [ ] Charry, Kubik, Agawu (2003) and Arom's later methodological writings are
      in the reference appendix with declared tiers
- [ ] Each is cited at the claim it supports, not only listed

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F44 | Charry, E. (2000), *Mande Music*, is absent; it is what corrects the Manding same-cycle oversimplification | `P2` | `enrich` | `appendix-references.mdx`, `02-sub-saharan-africa.mdx` | Cited inline at the F26 disclosure; the entry resolves and carries a tier | `open` |
| F45 | Kubik, G. (1999), *Africa and the Blues*, is absent; it covers the African rhythmic retentions that bridge Ch 2 and Ch 4 | `P2` | `enrich` | `appendix-references.mdx`, `02-sub-saharan-africa.mdx` | Cited inline; the entry resolves and carries a tier | `open` |
| F46 | Agawu, K. (2003), *Representing African Music*, is absent; the appendix carries Agawu (2006) instead, which is a different argument and does not ground F11 | `P2` | `enrich` | `appendix-references.mdx`, `08-minimalism.mdx` | Cited inline at the F11 reframe; the entry resolves and carries a tier | `open` |
| F52 | Arom's later methodological writings are absent; the appendix carries Arom (1991) only | `P2` | `enrich` | `appendix-references.mdx`, `theory-sub-saharan-africa.mdx` | Cited in the sub-Saharan methodology section; the entry resolves and carries a tier | `open` |

### Slice M005/S02 — Afro-Cuban sources

**Depends:** M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S02.md
**Status:** open

**Definition of Done**

- [ ] Acosta is in the reference appendix with a declared tier and cited at the
      clave-evolution discussion

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F47 | Acosta, L. (2004), *Cubano Be, Cubano Bop*, is absent; it is the historical depth behind clave evolution | `P2` | `enrich` | `appendix-references.mdx`, `03-afro-cuban.mdx` | Cited inline; the entry resolves and carries a tier | `open` |

### Slice M005/S03 — Indian-classical sources

**Depends:** M002/S04, M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S03.md
**Status:** open

**Definition of Done**

- [ ] Powers's *New Grove* "India" article is in the appendix with a declared
      tier and cited where the Carnatic absence is stated
- [ ] Kippen is cited inline at the theka discussion, not only in Further Reading

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F48 | Powers, H. (1980), "India", *New Grove*, is absent, and Kippen (1988) is in Further Reading only; between them they cover the Carnatic tala system and theka elaboration | `P2` | `enrich` | `appendix-references.mdx`, `06-indian-classical.mdx` | Both cited inline; the entries resolve and carry tiers | `open` |

### Slice M005/S04 — Balkan sources

**Depends:** M002/S05, M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S04.md
**Status:** open

**Definition of Done**

- [ ] Peycheva & Dimov are in the appendix with a declared tier and cited at
      the svatbarska-muzika discussion

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F49 | Peycheva, L. & Dimov, V. — Bulgarian wedding-music scholarship — are absent, and Ch 7's svatbarska-muzika material has no scholarly citation | `P2` | `enrich` | `appendix-references.mdx`, `07-balkan.mdx` | Cited inline; the entry resolves and carries a tier | `open` |

### Slice M005/S05 — Minimalism and electronic sources

**Depends:** M001/S06, M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S05.md
**Status:** open

**Definition of Done**

- [ ] Scherzinger's critical account is cited in Chapter 8 itself, not only in
      the theory page and the appendix
- [ ] Born & Hesmondhalgh are in the appendix with a declared tier and cited
      where the guide argues cross-cultural combination

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F50 | Scherzinger (2010) gives a more critical account of the African–minimalist connection than Reich's own; it is in the appendix and cited from `theory-minimalism.mdx`, but the chapter that makes the claim does not cite it | `P2` | `enrich` | `08-minimalism.mdx` | Cited inline alongside the F11 reframe; the citation resolves | `open` |
| F51 | Born, G. & Hesmondhalgh, D. (2000), *Western Music and Its Others*, is absent; it frames why cross-cultural combination works or does not | `P2` | `enrich` | `appendix-references.mdx`, `14-synthesis.mdx` | Cited inline; the entry resolves and carries a tier | `open` |

### Slice M005/S06 — Brazilian maracatu

**Depends:** M002/S06
**Validation:** format, site-unit, doc-conformance, gate
**Evidence:** evidence/M005-S06.md
**Status:** open

**Definition of Done**

- [ ] The maracatu section describes the ensemble's named parts and their
      rhythmic relationship, not only its density and weight
- [ ] Its claims carry citations that resolve

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F53 | The maracatu treatment is thin — accurate, but three paragraphs covering density and dynamics with no named ensemble parts and no citations | `P2` | `enrich` | `10-brazilian.mdx` "Maracatu" section | The section names the ensemble's parts and carries resolving citations, asserted by a prose-conformance case | `open` |

---

## Sequencing

```
M001 (corrections) ──┬──> M003 (repositioning) ──┐
                     │                            ├──> M004 (patch alignment)
M002 (citations) ────┴────────────────────────────┘
                     │
                     └──> M005 (enrichment)
```

- **M001/S01 was the hard prerequisite for everything** — it landed the harness
  every later slice writes its locks against. M001/S05–S06 and every M002 slice
  carry it as a dependency, directly or transitively.
- **M002/S05 depends on M001/S04** because the Goldberg inline citation and the
  long-beat caveat touch the same paragraphs of `07-balkan.mdx`; running them in
  the other order means one rewrites the other's lock.
- **M002/S06 depends on M002/S01–S05** because the tier check fails while any
  Tier-C source is still the inline citation for a named-theory claim. Landing
  the check first means shipping a red gate.
- **M003/S01 depends on M001/S06** because a repositioning statement should
  describe a guide whose overclaims are already hedged, not promise it.
- **M004/S05 depends on M003/S03** because the named-rule checklist encodes the
  gamelan rules as reworded there, and on M004/S01–S04 for the same reason
  M002/S06 depends on its milestone: the checklist fails while any patch still
  diverges silently.
- **M005 depends on M002/S06** throughout: a reference added after the tier
  check exists must arrive carrying a tier. M005 is genuinely last, but it is
  queued rather than written off — every P0 and P1 row closes in M001–M004.

## Related issues

Every open issue overlapping this programme's remit is enumerated here, so the
two views cannot silently drift.

- [#91](https://github.com/JimAKennedy/poly/issues/91) — **closed by F54**. The
  E(3,16) appendix row is correct under Poly's phase convention; F54 makes that
  convention explicit next to the table.
- [#156](https://github.com/JimAKennedy/poly/issues/156) — **not closed here**.
  F31 discloses that Humanize applies random jitter rather than Polak's
  systematic profiles; the exact non-Euclidean-timeline preset work stays on
  the tracker.
- [#157](https://github.com/JimAKennedy/poly/issues/157) — **not closed here**.
  F32 locks the Balkan long-beat honesty note; cell-aware swing for aksak is a
  real engine change and stays on the tracker.

## Out of scope

- **Engine or plugin behaviour changes.** Every row is documentation, website,
  or test. F31 and F34 point at real engine limitations; this programme
  discloses them rather than building features. Issues #156 and #157 are where
  those would land, and are unchanged by this ledger.
- **A comprehensive ethnomusicology review.** The audit's own conclusion is that
  the guide should not be positioned as one. M003/S01 states that in the guide.
