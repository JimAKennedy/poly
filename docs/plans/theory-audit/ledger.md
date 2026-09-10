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

**Row series.** `F01`–`F54` are the audit's findings, one row each, and that
range is closed. `H01`–`H04` are the harness this programme needed to lock its
own corrections. The `B` series is defects this programme found itself
rather than inheriting from the audit. `B01`–`B09` are bibliography defects, in
M006 — the audit drew its Tier-C list from the chapters it reviewed, so it never
saw the orphaned, duplicated and low-tier entries that M002's own work
surfaced. `B10` onward are other discovered defects, wherever the slice that
found one sits; a `B` row says only that the audit did not name it.

**Row vocabulary.** Beyond the required columns, each row carries a severity
(`P0` factually wrong · `P1` overclaimed or under-sourced · `P2` enrichment) and
a disposition (`correct` · `source` · `reframe` · `disclose` · `patch-align` ·
`enrich` · `verify`), and a `Lands in` pointer. All three are informational; the
`ledger` check ignores them, since the standard requires only `ID`, `Item`,
`Verification` and `Status`. `Lands in` is a forecast while a row is open and a
record once it closes, so the slice that closes a row corrects it to the files
actually modified — it is not a claim any test enforces.

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
**Status:** done
**Demo:** The audit's Section 4 Tier-C list is empty for inline citations, and
the new tier test fails if a Tier-B or Tier-C source is attached to a
named-theory claim.

### Slice M002/S01 — Reference [2] resolution

**Plan:** M002-S01-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S01.md
**Status:** done

**Definition of Done**

- [x] Reference [2]'s publication status is established from the publisher, and
      the finding records which it was
- [x] The appendix entry states the resolved status: real, replaced, or marked
      forthcoming

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F17 | Ref [2] cited Goldberg (2025), *Music Theory Online* 31(2) under the title "Resultant Patterns in Phase-Shifted Rhythmic Structures". Resolved 2026-09-01 against the publisher: the venue, volume, year, author and URL are real and correctly paired, and MTO is current through Vol 32 No 2 (June 2026) — so the audit's stated reason, a forward reference past Vol 30, does not hold. The title is what was wrong. MTO 31(2) carries Goldberg's "Music Theory as an Instrument of Nationalism: Notation, Identity, and Systemization in Dobri Hristov's Conception of Bulgarian Meter", and no MTO issue carries the printed title. A real article cited under a fabricated title, corrected in place. The entry stays uncited until M002/S05 draws on it for Ch 7 | `P0` | `verify` | `appendix-references.mdx` `ref-2` | Cases `S01-F17` and `S01-F17-tree` in `site/tests/citation-tier.test.mjs`: the entry carries the real title and the MTO 31(2) URL, and the fabricated title appears in no doc under the docs root | `done` |

### Slice M002/S02 — Chapter 3 clave citations

**Plan:** M002-S02-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S02.md
**Status:** done

**Definition of Done**

- [x] The clave-matrix and non-Euclidean-gap claims cite Peñalosa (2009) inline
- [x] Refs [10] and [11] no longer carry a named-theory claim in Chapter 3

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F18 | Refs [10] and [11] — YouTube videos — were the inline citations for the clave matrix, the most important theoretical claim in Ch 3, while Peñalosa (2009) sat unused in Further Reading. Resolved 2026-09-04: ref-10's three claim citations moved to Peñalosa, with Toussaint (ref-1) taking the Euclidean-gap half of the line 79 sentence, which Peñalosa does not support. Ref [11] carried no claim anywhere in the tree to begin with, so it needed no edit. The companion page keeps its Sources "See also refs [10]-[13]" listing, a bibliographic pointer rather than a named-theory claim | `P0` | `source` | `03-afro-cuban.mdx`, `theory-afro-cuban.mdx` | Cases `S02-F18` and `S02-F18-theory` in `site/tests/citation-tier.test.mjs` forbid the superscript claim form of ref-10 in `03-afro-cuban.mdx` and `theory-afro-cuban.mdx` and require `fr-penalosa-2009` in each. F23's Tier-A check, which M002/S06 builds, is not what proves this slice | `done` |

### Slice M002/S03 — Chapter 4 Afrobeat citations

**Plan:** M002-S03-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S03.md
**Status:** done

**Definition of Done**

- [x] Chapter 4's opening and phrase-gating claims cite Allen & Veal (2013) or
      Veal (2000) inline
- [x] Refs [14]–[17] no longer carry a named-theory claim in Chapter 4

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F19 | Refs [14]–[17] for Afrobeat were YouTube videos and production blogs, while Allen & Veal (2013) and Veal (2000) sat unused in Further Reading. Resolved 2026-09-04. Ch 4 carried only two of the four: ref-14 on the opening, whose sentence already cited Allen & Veal after M001/S06, so the superscript was simply dropped; and ref-17 on a sentence describing Poly lane behaviour rather than Afrobeat. That citation was removed with no replacement — a project-specific claim is declared as the project's own under the research-provenance classes, and ref-17 was an Afro House production guide, a different genre, so re-citing it to Veal would have swapped a wrong-tier citation for a wrong-claim one. The definition of done's phrase-gating claim was uncited altogether and gained Veal (2000). Refs [15] and [16] carried no claim anywhere and needed no edit | `P0` | `source` | `04-afrobeat.mdx` | Case `S03-F19` in `site/tests/citation-tier.test.mjs` forbids the superscript claim form of both ref-14 and ref-17 in `04-afrobeat.mdx` and requires `fr-allen-veal-2013` and `fr-veal-2000`. F23's Tier-A check, which M002/S06 builds, is not what proves this slice | `done` |

### Slice M002/S04 — Chapter 6 Indian-classical citations

**Plan:** M002-S04-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S04.md
**Status:** done

**Definition of Done**

- [x] Chapter 6's tala, laya and theka claims cite Clayton (2000), Nelson
      (2008) or Kippen (1988) inline
- [x] Refs [21]–[25] no longer carry a named-theory claim in Chapter 6

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F20 | Refs [21]–[25] for Ch 6 included a commercial blog, a high-school textbook PDF and a YouTube konnakol video, while Clayton, Nelson and Kippen sat unused in Further Reading — the chapter cited none of the three even once. Resolved 2026-09-07. The tala and layakari claims moved to Clayton (2000), and the theka claim, which was uncited rather than mis-cited, gained Kippen (1988). ref-25 was off-topic as well as low-tier: a konnakol video standing in for layakari, where konnakol is Carnatic vocal percussion and layakari is augmentation ratios. Nelson (2008) is deliberately unused, its tihai and mora arithmetic being M004/S04's subject. The companion page keeps its Sources "See also refs [21]-[25]" listing, a bibliographic pointer rather than a named-theory claim | `P1` | `source` | `06-indian-classical.mdx`, `theory-indian-classical.mdx` | Cases `S04-F20` and `S04-F20-theory` in `site/tests/citation-tier.test.mjs` forbid any superscript containing a ref-21 to ref-25 link in either file and require `fr-clayton-2000`, plus `fr-kippen-1988` in the chapter. The pattern is generalised from the single-reference form S02 and S03 used, which cannot match this page's two-references-in-one-sup blocks. F23's Tier-A check, which M002/S06 builds, is not what proves this slice | `done` |

### Slice M002/S05 — Chapter 7 Balkan citations

**Depends:** M001/S04
**Plan:** M002-S05-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M002-S05.md
**Status:** done

**Definition of Done**

- [x] The aksak-definition and svatbarska-muzika claims cite Brăiloiu (1951),
      Rice (1994) or Goldberg (2015) inline
- [x] Refs [26] and [27] no longer carry a named-theory claim in Chapter 7
- [x] The `S04-F06` long-beat lock still passes after the citation edits

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F21 | Refs [26] (Fiveable) and [27] (Chromatone) for Balkan were educational aggregator pages, while Brăiloiu, Rice and Goldberg sat in Further Reading only. Resolved 2026-09-07. The aksak definition took Brăiloiu (1951), who coined the term and whose paper is the two/three-cell theory of additive meter; the svatbarska-muzika claim took Rice (1994), the standard ethnography of Bulgarian practice. Goldberg (2015) stayed where it already was, on the long-beat timing claim, being the authority for neither of these two. The companion page needed no edit and keeps its Sources listing, a plain-link bibliographic pointer that also stops both references orphaning once their claims moved | `P1` | `source` | `07-balkan.mdx` | Case `S05-F21` in `site/tests/citation-tier.test.mjs` forbids any superscript containing a ref-26 or ref-27 link in `07-balkan.mdx` and requires `fr-brailoiu-1951` and `fr-rice-1994`, and case `S04-F06` still passes — asserted by name, since the edits bracket the line it guards. F23's Tier-A check, which M002/S06 builds, is not what proves this slice | `done` |

### Slice M002/S06 — Reference tiers and the tier check

**Depends:** M002/S01, M002/S02, M002/S03, M002/S04, M002/S05
**Plan:** M002-S06-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline, gate
**Evidence:** evidence/M002-S06.md
**Status:** done

**Definition of Done**

- [x] Every entry in the reference appendix carries a declared tier
- [x] The "Spanish tinge" attribution cites Lomax's Morton interviews
- [x] A new check fails when a Tier-B or Tier-C source is the inline citation
      for a named-theory claim
- [x] The check is wired into `scripts/check-doc-conformance.sh` **and**
      added to the `REQUIRED` set in
      `site/tests/doc-conformance-wiring.test.mjs`. That file's own header
      states the contract — adding a guardrail means adding it to both — and
      the runner alone leaves the new check undefended against a later edit
      quietly dropping it

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F22 | Ch 3's "the habanera rhythm that Jelly Roll Morton called 'the Spanish tinge'" is accurate but was uncited; Lomax's Morton interviews (1950) are the primary source. Resolved 2026-09-08. No Lomax entry existed anywhere in the appendix, so this added a Further Reading entry as well as the inline citation — Mister Jelly Roll (1950) is the book built from the Library of Congress interviews in which Morton uses the phrase, and is tiered A as a primary source | `P1` | `source` | `03-afro-cuban.mdx`, `appendix-references.mdx` | Case `S06-F22` in `site/tests/citation-tier.test.mjs` requires `fr-lomax-1950` in the chapter. It carries no forbidden arm, because nothing wrong was removed — an uncited claim was sourced — so it can only fail on the present side | `done` |
| F23 | The reference list mixed peer-reviewed scholarship and hobbyist media at equal citation weight, with no mechanism preventing regression. Resolved 2026-09-08. All 99 entries — 100 after F22 added Lomax — declare a tier on the span they already carried, so the tier cannot drift from the entry, and a claim citation may not resolve below Tier A. The rule is mechanisable because the guide's own grammar separates claims from bibliography: a superscript marks a claim, a plain link marks a listing, so the Sources sections that deliberately point at low-tier refs are out of scope structurally rather than exempted one at a time. It ships with 10 live suppressions across 9 files covering 8 references, each carrying a reason read from the claim it sits on and naming the M006 row that owns the upgrade. M006/S01 is one row short: ref-42 has none, recorded in this slice's evidence | `P1` | `source` | `appendix-references.mdx`, `site/tests/citation-tier.test.mjs`, `site/tests/doc-conformance-wiring.test.mjs`, `scripts/check-doc-conformance.sh`, and the nine chapter and companion pages carrying a suppression | Cases `S06-tiers-declared` and `S06-claims-are-tier-a` in `site/tests/citation-tier.test.mjs`, both proved non-vacuous: the first catches a missing and an invalid tier by name, the second reports a citation again when its marker moves one line too far and fails 18 citations when Toussaint is flipped to Tier B. `doc-conformance-wiring.test.mjs` names the file in `REQUIRED`, which failed by name until the runner listed it too | `done` |

---

## Milestone M003 — Scope and Repositioning

**Vision:** The guide states plainly what it is, what it is not, and exactly
where it simplifies, so a reader can calibrate every claim it makes.
**Branch:** milestone/M003-scope-repositioning
**Status:** done
**Demo:** The audit's Section 5 repositioning statement is live on the site, and
each simplification it names is disclosed at the point of use.

### Slice M003/S01 — About This Guide

**Depends:** M001/S06
**Plan:** M003-S01-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S01.md
**Status:** done

**Definition of Done**

- [x] An "About This Guide" page carries the audit's repositioning statement
- [x] It is reachable from the introduction and from all twelve `theory-*.mdx`
      pages
- [x] It names the guide's deliberate scope exclusions, including the
      son-clave/rumba-clave precedence debate

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F24 | The guide had no statement of what it is and is not; the audit supplies a four-paragraph repositioning statement and judges it defensible. Resolved 2026-09-09. The statement was extracted from the audit by script rather than transcribed, so the four paragraphs are verbatim. The eleven companion pages take the link in their existing italic preamble; `theory-counterpoint-overview.mdx` has no preamble and instead links upward from its own "None of this is original research" paragraph, which is left intact so the local and global statements agree by reference rather than by copy | `P1` | `reframe` | `about-this-guide.mdx`, `introduction.mdx`, all twelve `theory-*.mdx`, `site/astro.config.mjs`, `.github/docs-drift-map.yml` | Case `S01-F24` in `site/tests/scope-framing.test.mjs` asserts the page exists and that all thirteen pages link to it. The theory set is discovered by glob and its count asserted at twelve, so a glob matching nothing cannot pass vacuously, and every missing file is named rather than the first | `done` |
| F36 | Whether rumba clave predates or postdates son clave is debated (Acosta 2004, Moore 2006); the audit judges the guide may legitimately sidestep it under the repositioning frame. Resolved 2026-09-09. The About page names it under "What this guide does not cover" and says why the sidestep is legitimate rather than merely convenient: both claves are given as they are played, and the rules for aligning parts to either are the same whichever came first, so nothing in the guide depends on the answer | `P1` | `accept` | `about-this-guide.mdx` | Case `S01-F36` in `site/tests/scope-framing.test.mjs` requires the debate, Acosta and Moore all named on the About page, so the sidestep cannot become silent again | `done` |

### Slice M003/S02 — Chapter 6 scope

**Plan:** M003-S02-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S02.md
**Status:** done

**Definition of Done**

- [x] Chapter 6's front-matter description and its in-page scope note both say
      Hindustani, and neither promises Carnatic coverage
- [x] The absence of the Carnatic tala system is stated rather than implied

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F25 | Ch 6 was described as covering "Hindustani and Carnatic" but the Carnatic tala system — solkattu/konnakol, different tala families, a different conceptual frame — is absent. Resolved 2026-09-09. The word Carnatic appeared exactly once in the chapter, in the description making the promise, so this removed a false promise rather than relabelling content. The same overclaim sat in the deep-dive index row of `theory-counterpoint-overview.mdx`, advertising "Hindustani and Carnatic rhythm" one click from the page that now says Carnatic is absent; it was corrected with it. The Scope note is about this chapter rather than the guide, because M004/S04 will add a tihai worked example from Nelson's Solkattu Manual to the companion page and the appendix already lists that source | `P1` | `correct` | `06-indian-classical.mdx`, `theory-counterpoint-overview.mdx` | Cases `S02-F25` and `S02-F25-overview` in `site/tests/scope-framing.test.mjs`. The first forbids the phrase and requires Carnatic, solkattu and konnakol to be named — the requiring arm is load-bearing, since deleting the word alone would pass a forbid-only check while leaving the absence unstated. The second guards the index row and was proved to bite independently | `done` |

### Slice M003/S03 — Simplification disclosures

**Plan:** M003-S03-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S03.md
**Status:** done

**Definition of Done**

- [x] The Manding same-cycle presentation is marked as a pedagogical flattening
      and cites Charry
- [x] Gamelan Rule 3's polos/sangsih assignment is marked style-dependent, with
      *norot* named as the reversing case
- [x] The layakari section says plainly that Poly's subdivision change is not
      what layakari does to hit density
- [x] Gamelan Rule 5's "choose one and keep it" is hedged to match Tenzer
- [x] Gamelan Rule 4's existing strict-complementation honesty is locked by a
      test, so a later edit cannot drop it

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F26 | Ch 2 characterised Manding ensembles as all sharing one cycle length; Manding dunun ensembles do use distinct lengths in many contexts. Pedagogical flattening, not error. Resolved 2026-09-09. The chapter now marks it as a pedagogical simplification, says the dunun, sangban and kenkeni relationship varies by repertoire and region, and gives the reason the shared cycle is used — the shortest route to a playable patch that sounds idiomatic. Charry (2000) is named as plain text with no anchor, deliberately: the appendix entry is F44's in M005/S01, whose verification reads "cited inline at the F26 disclosure", so linking now would point at nothing and fail research-provenance. F44 converts the name to a citation | `P1` | `disclose` | `02-sub-saharan-africa.mdx` | Case `S03-F26` in `site/tests/scope-framing.test.mjs` requires "pedagogical simplification", "Charry" and "distinct cycle lengths" in the chapter. It carries no forbidden arm — the original sentence is not wrong and was not removed, only qualified | `done` |
| F27 | Gamelan Rule 3 stated polos leans onbeat and sangsih offbeat; in *norot* the relationship is effectively reversed. Style-dependent, presented as general. Resolved 2026-09-09. Rule 3 now marks the division style-dependent, names norot as the reversing case with sangsih taking the beat-coincident tones, and asks the reader to treat it as the common case rather than the general one | `P1` | `disclose` | `theory-gamelan.mdx` | Case `S03-F27` in `site/tests/scope-framing.test.mjs` requires "style-dependent" and "in norot the relationship is effectively reversed". It deliberately does not assert bare norot, which Rule 5 already names and which would therefore have passed before the edit | `done` |
| F28 | Ch 6 mapped layakari ratios onto Poly's per-lane subdivision; true layakari is the same phrase at 2×/3× speed, and changing subdivision changes hit density. Resolved 2026-09-09. The mapping stays — it is a good way to get the sound in Poly — but is now marked a practical approximation, with the distinction stated plainly: layakari moves the rate of unchanged material, while a subdivision change alters how many onsets fall in the cycle, so the lane no longer plays the pattern it played before. A tabla player performing chaugun is not adding hits | `P1` | `disclose` | `06-indian-classical.mdx` | Case `S03-F28` in `site/tests/scope-framing.test.mjs` requires "hit density", "the same phrase" and "simplification". It carries no forbidden arm — the workflow is legitimate and was not removed, only qualified | `done` |
| F29 | `theory-gamelan.mdx` Rule 4 already stated honestly that strict complementation is only the textbook case; nothing stopped a later edit removing it. Resolved 2026-09-09. No prose changed — this row added only the lock, which is what an accept disposition means here | `P1` | `accept` | `theory-gamelan.mdx` | Case `S03-F29` requires "Strict complementation is only the textbook case", "the overlap marks structure" and the `fr-tenzer-2000` anchor. It passed on the day it was written, so it was proved non-vacuous by deleting the first sentence and watching it fail by name, recorded in the evidence | `done` |
| F30 | Gamelan Rule 5 said to choose one interlock style and never mix mid-phrase — stronger than Tenzer, who allows stylistic mixing within a kebyar performance. Resolved 2026-09-09. The practical advice survives as the reliable default and safest starting discipline, but is now marked a discipline rather than a rule of the tradition, citing Tenzer for the mixing he documents within a single kebyar performance | `P1` | `reframe` | `theory-gamelan.mdx` | Case `S03-F30` forbids "mixing interlock styles mid-phrase is not idiomatic" and requires "reliable default" and "stylistic mixing". It deliberately does not assert Tenzer or the tenzer anchor, both of which Rule 4 already carries | `done` |

### Slice M003/S04 — Non-isochrony honesty

**Plan:** M003-S04-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S04.md
**Status:** done

**Definition of Done**

- [x] `theory-sub-saharan-africa.mdx` states explicitly that Humanize is random
      jitter, against Polak's systematic style-specific profiles — not merely
      that it is an approximation
- [x] Both non-isochrony disclosures are locked by tests, so the Balkan one
      already in the tree cannot be dropped

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F31 | Rule 8's Humanize approximation was flagged as an approximation but the guide never said *how* it differs: Humanize is random jitter, Polak (2010) documents systematic style-specific subdivision profiles. Resolved 2026-09-09. Verified in `engine/src/engine.cpp` rather than taken on trust — `applyTimingShifts` derives `jitterPpq` from `deterministicRand`, so Humanize is seeded and reproducible but carries no per-position structure. Construction step 5 now says jitter reproduces the presence of non-isochrony and not its shape, and that a straight grid plus noise is still not the feel. Rule 8 is unchanged: it states what the scholarship found, and the contrast belongs where Humanize is named | `P1` | `disclose` | `theory-sub-saharan-africa.mdx` construction step 5 | Case `S04-F31` in `site/tests/scope-framing.test.mjs` requires "random jitter" and "systematic profile". It deliberately does not assert bare "systematic", which the page already contained inside "systematically" | `done` |
| B10 | `theory-sub-saharan-africa.mdx` construction step 5 said "until Poly ships subdivision-profile support", which understated the engine: `microTimingMs` is a per-step timing array, exposed to the WebUI as the micro-timing bars through the `setMicroTiming` bridge action and clamped to ±20 ms. What Poly lacks is a jembe *profile* to load, not the mechanism to express one. Found while planning F31, not named by the audit. Resolved 2026-09-09. The step now names per-step micro-timing as where such a profile would be entered, one step at a time, and says the missing piece is the measured data | `P2` | `correct` | `theory-sub-saharan-africa.mdx` construction step 5 | Case `S04-B10` forbids the old phrasing and requires micro-timing to be named | `done` |
| F32 | The audit asked that Balkan aksak be given the same non-isochrony honesty as the Malian jembe; `theory-balkan.mdx` Rule 8 already gave it, citing Goldberg (2015), but nothing locked it. Resolved 2026-09-09. **No prose changed** — `git diff` on the page is empty, and the row added only the lock, which is what the audit's own judgement of this item amounted to | `P1` | `disclose` | `theory-balkan.mdx` | Case `S04-F32` in `site/tests/scope-framing.test.mjs` requires "systematic, style-defining tendency", "the grid version is the" and the `fr-goldberg-2015` anchor. It passed on the day it was written, so it was proved non-vacuous by deleting the style-defining phrase and watching it fail by name, then restoring to a byte-identical file | `done` |

### Slice M003/S05 — Remaining framing items

**Validation:** format, site-unit, doc-conformance, gate
**Plan:** M003-S05-plan.md
**Evidence:** evidence/M003-S05.md
**Status:** done

**Definition of Done**

- [x] Gamelan Rule 5 says a third part may double the pokok tones instead of
      interlocking, cross-referenced to Rule 6 and to Construction's Pokok lane
- [x] The Rachenitsa patch table carries a `Note` column matching
      `presets.json`, and a line naming the GM sounds and flagging that kaval
      and gadulka have no GM drum equivalent
- [x] Chapter 5's cyclic-time opening carries a citation

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F33 | Gamelan Rule 5's named interlock styles omit a third part that doubles the structural pokok tones rather than interlocking with the pair — directly relevant to a user adding a third melodic lane. Amended during planning: the audit (§125) named this practice *kotekan polos* and cited nothing for the term, and the guide's own Rule 3 already uses *polos* for one of the interlocking pair, so adopting it as a style name would collide with the page's established usage. Amended again during execution: no source in this repo attests the practice either, so the sentence carries no citation — it asserts nothing the page does not already contain, since Rule 6 makes pokok what the interlock elaborates and Construction step 2 and patch Lane 5 already build a Pokok lane. Both the term and its attribution are M006/S04 row B11. The audit also miscounted the rule as listing five styles; it lists four | `P2` | `enrich` | `theory-gamelan.mdx` Rule 5 | Case `S05-F33` asserts a third part doubling the pokok is offered and cross-referenced to Rule 6, and forbids the unattested label | `done` |
| F34 | Ch 7's Rachenitsa patch table carries no note information at all, so a reader cannot tell what any lane will sound like on a GM kit. Amended during planning: the audit (§176) asked the "Note" column to mark tupan and kaval as stand-ins, but the table has no `Note` column and the premise is backwards. The tupan is a double-headed bass drum, so its two lanes map to GM kick and side stick — a drum standing in for a drum — which the Role column already says by naming them "Tupan bass" and "Tupan rim". The real stand-ins are lane 3, a woodblock for the kaval (an end-blown flute), and lane 4, a hi-hat for the gadulka (a bowed fiddle), which the audit does not mention. Note numbers and GM names are taken from the `Rachenitsa 7/8` record in `presets.json` | `P1` | `patch-align` | `07-balkan.mdx` Rachenitsa patch table | Case `S05-F34` asserts the table's `Note` column matches every lane's `noteNumber` in `presets.json`, and that the line beneath names each `roleLabel` and flags kaval and gadulka as having no GM drum equivalent | `done` |
| F35 | Ch 5 opens "time is not a line — it is a circle": legitimate but uncited, and unsourced it reads as orientalist; Tenzer (2000) grounds it | `P1` | `source` | `05-gamelan.mdx` chapter opening | Case `S05-F35` asserts the opening carries a citation that resolves in the appendix | `done` |

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

**Depends:** M004/S05
**Plan:** M004-S01-plan.md
**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S01.md
**Status:** done

**Definition of Done**

- [x] The Chapter 2 patch either carries the timeline-mode bell lane and the
      dance-beat lane its theory page requires, or cross-references the fuller
      theory-page construction in band
- [x] The choice is legible from the patch table alone

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F37 | The Ch 2 Ewe patch omits the dance-beat lane that `theory-sub-saharan-africa` construction step 2 requires — a low drum at 12 steps, 4 hits, E(4,12). Amended during planning: the other half of the audit's finding is already satisfied, since the `Ewe Polymetric Ensemble` preset carries `timeline: true` on its bell lane (Rule 1). What is missing is the dance-beat lane, and the fact that the patch table has no `Timeline` column, so the bell's mode is invisible to a reader — which is what the slice's second definition-of-done item is about | `P1` | `patch-align` | `02-sub-saharan-africa.mdx` Ewe patch | Cases `ssa-dance-beat` and `ssa-timeline-legible` in the named-rule checklist, both passing with no divergence marker — the two markers S05 left naming this row are removed | `done` |

### Slice M004/S02 — Chapter 3 patch

**Depends:** M004/S05
**Plan:** M004-S02-plan.md
**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S02.md
**Status:** done

**Definition of Done**

- [x] The Chapter 3 clave lane header marks itself as the Euclidean
      approximation and links to the exact-timeline construction
- [x] The theory-page tumbao lane's onset positions are rendered, not left to
      the reader to derive
- [x] The conga and quinto mutation settings either satisfy the one-free-voice
      rule or carry a divergence note

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F38 | Ch 3's clave lane is the E(5,16) approximation while the theory page's is an exact timeline pattern; the patch header says only "Clave" with no cross-reference | `P1` | `patch-align` | `03-afro-cuban.mdx` patch header | Case `ac-clave-approximation` in the named-rule checklist, passing with no divergence marker; both arms — the header naming the approximation and the link to the exact construction — proved to fail separately | `done` |
| F39 | The theory-page tumbao lane (16 steps, 6 hits, rotation 14) satisfies the beat-1-avoidance rule but is an unusual configuration presented without its onset positions | `P1` | `patch-align` | `theory-afro-cuban.mdx` Lane 2 | Case `ac-tumbao-onsets-rendered` asserts the printed set equals the derivation from the shared verifier, so the prose and the (steps, hits, rotation) spelling cannot drift apart | `done` |
| F40 | The son-ensemble patch carried three lanes with a variation budget — Cáscara 5%, Conga marcha 10%, Quinto 30% — against Rule 5, "the variation budget belongs to one voice at a time", whose construction names the quinto as that voice. **Twice amended, and the first amendment was wrong.** The audit's `Lands in` said `03-afro-cuban.mdx`; planning checked that file, found `mutationRate` 0.00 across the `Cuban Son Montuno` preset and no quinto lane at all, and recorded the premise as non-existent. The premise is real and lives on `theory-afro-cuban.mdx`, whose patch is the one with a quinto. The oddity that should have caught it — the audit naming a quinto in a son ensemble, when the theory page says the quinto is the free voice *in rumba* — was read as evidence the audit was wrong rather than as evidence the wrong table was being read. Cáscara and Conga marcha are now 0%, leaving the quinto the sole free voice | `P1` | `patch-align` | `theory-afro-cuban.mdx` patch | Case `ac-theory-one-free-voice` asserts at most one lane carries a non-zero Mutation, and names the offenders when more do | `done` |
| B16 | The `e2e` token runs `scripts/site-verify-local.sh`, which rebuilds the WASM engine and copies `poly_engine.js` and `poly_engine.wasm` over the committed artifacts, so every run dirties the tree. The rebuild is not byte-reproducible: during M004, which changes no engine source, the `.wasm` moved 142014 → 142151 bytes and the `.js` gained a trailing-whitespace line that `pre-commit` then strips, so `e2e` and `format` interfere. M004/S01's close swept both files in via `git add -A` and they were reverted. Found while running M004/S02 | `P2` | `accept` | `scripts/site-verify-local.sh`, `webui/poly_engine.{js,wasm}` | Recorded and owned by [#282](https://github.com/JimAKennedy/poly/issues/282); this programme reverts the artifacts after each `e2e` run and stages explicit paths rather than `git add -A` | `accepted` |

### Slice M004/S03 — Chapter 5 patch

**Depends:** M004/S05
**Plan:** M004-S03-plan.md
**Validation:** format, site-unit, doc-conformance, e2e
**Evidence:** evidence/M004-S03.md
**Status:** done

**Definition of Done**

- [x] The Chapter 5 kotekan patch carries a pokok layer, as `theory-gamelan`
      Rule 6 requires
- [x] It carries structural overlap at the cycle boundary, as Rule 4 requires

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F41 | The Ch 5 kotekan patch has no pokok (structural melody) layer, required by `theory-gamelan` Rule 6, and no structural overlap at the cycle boundary, required by Rule 4 | `P1` | `patch-align` | `05-gamelan.mdx` patch | Cases `gam-pokok-layer` and `gam-structural-overlap` in the named-rule checklist, both passing with no divergence marker. Rule 4 is checked as construction step 4 specifies it — a lane outside the kotekan pair sounding at the cycle boundary — because Rule 4 as written demands pair-overlap that Poly's Kotekan L-mode cannot produce, as its own parenthetical says | `done` |

### Slice M004/S04 — Tihai worked example

**Plan:** M004-S04-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M004-S04.md
**Status:** done

**Definition of Done**

- [x] The tihai discussion shows Nelson's formula with real numbers, not just
      the principle
- [x] The arithmetic in the example is checked, not asserted

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F43 | The tihai discussion gives the principle but no worked example; Nelson (2008)'s formula — (Length × 3) + (Gap × 2) = beats remaining to sam — should be shown with real numbers | `P1` | `enrich` | `theory-indian-classical.mdx` Rule 6 | Case `ind-tihai-worked` reads Phrase Len and Gap from the lane, derives 3 × phrase + 2 × gap, and asserts the prose prints both operands and the product and that the product closes the lane's cycle — so changing the lane fails the case, not just changing the prose | `done` |

### Slice M004/S05 — Named-rule conformance checklist

**Depends:** M003/S03
**Plan:** M004-S05-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline, gate
**Evidence:** evidence/M004-S05.md
**Status:** done

**Definition of Done**

- [x] `theory-patch-conformance.test.mjs` carries a per-page named-rule
      checklist declared as data, covering the Chapter 2, 3 and 5 patches its
      sibling slices need and the two theory pages that carry no assertion at
      all today, `theory-electronic-breakbeat` and `theory-minimalism`
- [x] A rule a patch deliberately breaks is satisfied by an in-band divergence
      marker carrying a written reason, and by nothing else
- [x] A patch that silently drops a rule fails the suite
- [x] Every rule in the checklist is proved to fail when its lane is removed
- [x] The remaining rule coverage is owned by M007, named here, not left
      implicit

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| F42 | The CI tests verify theory-page patches carry valid Euclidean triples but never that a patch follows its page's own named rules — the gap every one of F37–F41 lives in. Closed by a per-page checklist declared as data and iterated, so a page with no entry is a missing row rather than an invisible absence, together with the `patch-divergence-ok` marker contract. Coverage is this slice's: the three chapter patches S01–S03 need, plus `theory-minimalism` and `theory-electronic-breakbeat`, the only two theory pages that carried no assertion at all. The remaining rules across the other nine theory pages are M007's. The first newly-checked rule found a tenth contradiction of the class the 2026-07-30 review found nine of — see B15 | `P1` | `patch-align` | `site/tests/theory-patch-conformance.test.mjs` | The checklist carries eleven rules across five patches; each was watched to fail on its own terms, a marker satisfies only the rule it names, a marker on a passing rule fails, and the live suppression count is printed | `done` |

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

## Milestone M006 — Bibliography Hygiene

**Vision:** Every reference in the guide is cited by something, cited at a tier
that supports the claim it carries, and appears exactly once.
**Branch:** milestone/M006-bibliography-hygiene
**Status:** planned
**Demo:** `grep -rn citation-tier-ok site/src/content/docs` returns nothing, the
appendix has no entry that no page cites, and no two entries name the same work.

M002 fixed every reference the audit flagged. Doing so surfaced three classes of
defect the audit did not see, because its Tier-C list was drawn from the
chapters it reviewed rather than from the bibliography as a whole. This
milestone is that follow-on work, and its rows carry the `B` prefix to say
plainly that they are ours rather than the audit's.

### Slice M006/S01 — Upgrade the seven suppressed claim citations

**Depends:** M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M006-S01.md
**Status:** open

**Definition of Done**

- [ ] Each of the seven claims either cites a Tier-A source or no longer makes
      a claim requiring one
- [ ] `grep -rn citation-tier-ok site/src/content/docs` returns nothing, and the
      tier check's live suppression count is zero

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B01 | Ch 1 cites Wikipedia's "Euclidean Rhythm" for a named-theory claim, where Toussaint (ref-1) is cited in the same chapter | `P1` | `source` | `01-foundations.mdx` | The `citation-tier-ok` suppression on this citation is removed and `citation-tier.test.mjs` still passes | `open` |
| B02 | Ch 3 cites Sher Music publisher sample pages for a named-theory claim, where Mauleón (1993) sits in Further Reading | `P1` | `source` | `03-afro-cuban.mdx` | As B01, for this citation | `open` |
| B03 | Ch 5 cites a Gamelan New Zealand community PDF for a named-theory claim, where Tenzer, Vitale and Sumarsam sit in Further Reading | `P1` | `source` | `05-gamelan.mdx` | As B01, for this citation | `open` |
| B04 | Ch 8 cites Wikipedia's "Steve Reich" for a named-theory claim, where Reich (2002), Potter and Gann sit in Further Reading | `P1` | `source` | `08-minimalism.mdx` | As B01, for this citation | `open` |
| B05 | Ch 8 cites an All Classical Portland radio article for a named-theory claim | `P1` | `source` | `08-minimalism.mdx` | As B01, for this citation | `open` |
| B06 | Ch 9 and its companion cite the Brettworks blog for the Linn swing claim; M001/S06 attributed the claim to Linn in prose but left the reference | `P1` | `source` | `09-electronic.mdx`, `theory-electronic-breakbeat.mdx` | As B01, for both citations | `open` |
| B07 | Ch 13 cites the Ethan Hein blog for a named-theory claim about the Amen break | `P1` | `source` | `13-drum-and-bass.mdx` | As B01, for this citation | `open` |

### Slice M006/S02 — Resolve the orphaned references

**Depends:** M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M006-S02.md
**Status:** open

**Definition of Done**

- [ ] Every numbered reference is cited by at least one page, or is retired
      with the reason recorded
- [ ] A check fails when an appendix entry is cited by nothing

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B08 | Eighteen numbered references and two Further Reading entries are cited by no page. Four were orphaned by M002 moving claims onto scholarship; the rest predate it. An uncited entry is either dead weight or a source nobody checked — ref-2 was the fabricated-title citation S01 found, and nothing cited it | `P1` | `correct` | `appendix-references.mdx`, `site/tests/citation-tier.test.mjs` | `citation-tier.test.mjs` asserts every entry is cited by at least one page, with retired entries deleted rather than exempted | `open` |

### Slice M006/S03 — De-duplicate the appendix

**Depends:** M002/S06
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M006-S03.md
**Status:** open

**Definition of Done**

- [ ] No two appendix entries name the same work
- [ ] A check fails when two entries share a title and year

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B09 | `ref-1` and `fr-toussaint-2005` are the same 2005 BRIDGES paper listed twice, at different weights in the same appendix. ref-1 is cited in six files and the duplicate in none, so the two cannot disagree today, but nothing stops a later citation picking the wrong one | `P2` | `correct` | `appendix-references.mdx`, `site/tests/citation-tier.test.mjs` | `citation-tier.test.mjs` asserts no two entries share a normalised title and year, and the sweep that finds them is recorded in this row | `open` |

### Slice M006/S04 — Unattested terms

**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M006-S04.md
**Status:** open

**Definition of Done**

- [ ] *Kotekan polos*, and the practice Rule 5 describes without it, are either
      cited to a tier-A or tier-B source, or recorded here as unverifiable and
      deliberately left uncited

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B11 | The audit (§125) asserts *kotekan polos* — "a third player playing only the structural pokok tones" — as a named interlock style, citing nothing for the term. Nothing in the repo attests either the term or the practice: `fr-tenzer-2000` is annotated "the authoritative analysis of kotekan varieties", which argues Tenzer could cover it but is not evidence that he does. M003/S05 wrote the practice into Rule 5 with no citation and no label, forbidding the label in case `S05-F33`, on the grounds that Rule 6 and Construction's Pokok lane already ground it internally. This row owns both open questions: whether the term has an attestation, and whether any source documents the practice so Rule 5 can cite it. Found while planning and executing M003/S05, not named by the audit as a defect in itself | `P2` | `source` | `theory-gamelan.mdx` Rule 5 | Either Rule 5 names the term and/or cites the practice with a resolving tier-A/B reference, or this row is `accepted` with the reason recorded | `open` |

## Milestone M007 — Named-rule coverage

**Vision:** Every mechanically checkable rule a theory page states is checked
against that page's patch, so the guide's rules and its worked examples cannot
drift apart unnoticed.
**Branch:** milestone/M007-named-rule-coverage
**Status:** planned
**Demo:** Count the checklist's rule entries against the pages' numbered rules;
every checkable rule is present, and every omission carries a recorded verdict.

M004/S05 builds the checklist mechanism and proves it on five chapter patches
and the two theory pages that had no assertion. This milestone carries it to
the rest. The sizing was measured while planning M004/S05: eleven theory pages
state 92 numbered rules between them, of which nine were asserted — one
predicate per page, each encoding one defect from the 2026-07-30 conformance
review. Those nine all pass. The other 83 have never been checked against a
patch, which is where this milestone's findings will come from.

Not every numbered rule is mechanically checkable. "The timeline never varies"
and "each part alone must be playable and idiomatic" are prose judgments a lane
table cannot settle, so the first slice triages the 92 before the second
asserts any of them.

### Slice M007/S01 — Rule triage

**Depends:** M004/S05
**Validation:** format, doc-conformance
**Evidence:** evidence/M007-S01.md
**Status:** open

**Definition of Done**

- [ ] Every numbered rule on every theory page is classified checkable or not,
      with a one-line reason recorded for each not-checkable verdict
- [ ] The triage lives beside the checklist, so a rule added to a theory page
      without a verdict is visible

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B12 | The eleven theory pages state 92 numbered rules and nine are asserted, but which of the remaining 83 a lane table can settle has never been decided. Until that verdict is recorded, "this rule is not checked" and "this rule is not checkable" are indistinguishable | `P2` | `verify` | `site/tests/theory-patch-conformance.test.mjs` triage table | A case asserts every numbered rule on every theory page carries a triage verdict, and fails when a page gains a rule that has none | `open` |

### Slice M007/S02 — Roll out the checkable rules

**Depends:** M007/S01
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M007-S02.md
**Status:** open

**Definition of Done**

- [ ] Every rule triaged checkable carries a checklist entry
- [ ] Each entry is proved to fail when the lane it guards is removed
- [ ] A patch violating a rule either carries a divergence marker with a written
      reason or is corrected

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B13 | 83 numbered rules across nine theory pages have never been checked against the patch meant to demonstrate them. The 2026-07-30 review found nine such contradictions by hand and all nine are now fixed, which is evidence the class is real rather than that it is exhausted | `P1` | `patch-align` | `site/tests/theory-patch-conformance.test.mjs` | Every checkable rule has a checklist entry, each with a mutation proof recorded in the evidence file | `open` |
| B15 | `theory-electronic-breakbeat`'s own patch contradicts its Rule 7, "the kick syncopates against the snare, avoiding its slots": the chopped kick is E(5,16) at rotation 3, whose onsets on a 16-pulse grid are {3,6,9,12,15}, and the backbeat snare's are {4,12}. Pulse 12 is shared. Found by the M004/S05 checklist on the first newly-checked rule, and suppressed there with a `patch-divergence-ok` marker so the slice could close; the fix is either a rotation that clears the snare or a qualification to Rule 7, and that is a musical decision this row exists to put to a human | `P1` | `patch-align` | `theory-electronic-breakbeat.mdx` patch, Rule 7 | Case `ebb-kick-avoids-snare` passes with no divergence marker | `open` |

### Slice M007/S03 — Burn down the divergence markers

**Depends:** M007/S02
**Validation:** format, site-unit, doc-conformance, gate
**Evidence:** evidence/M007-S03.md
**Status:** open

**Definition of Done**

- [ ] Every `patch-divergence-ok` marker either is replaced by a corrected patch
      or carries a reason a reviewer has accepted
- [ ] No marker carries the untriaged placeholder reason

| ID | Item | Sev | Disp | Lands in | Verification | Status |
|---|---|---|---|---|---|---|
| B14 | Markers opened during M004 and M007/S02 with the reason "found by the checklist, not yet triaged" are a backlog, not a decision. A suppression nobody has read since it was written is indistinguishable from a defect | `P2` | `verify` | theory and chapter pages carrying `patch-divergence-ok` | A case fails on any marker still carrying the untriaged placeholder reason, and the suite prints the live marker count | `open` |

## Sequencing

```
M001 (corrections) ──┬──> M003 (repositioning) ──┐
                     │                            ├──> M004 (patch alignment)
M002 (citations) ────┴────────────────────────────┘
                     │
                     ├──> M005 (enrichment)
                     │
                     └──> M006 (bibliography hygiene)
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
  gamelan rules as reworded there. It runs **first** within its milestone, and
  M004/S01–S03 depend on it. The original order had S05 last, on the M002/S06
  precedent that a burn-down slice follows the work it burns down — but the two
  are not alike. S05 builds the machinery its siblings are verified by: F37,
  F40 and F41 all name the extended suite or the divergence marker as their
  verification, so with S05 last the escape hatch would be reached for before it
  was designed, and each chapter slice would assert ad hoc against an oracle
  that did not exist yet. S04 needs neither and stays independent.
- **M006 depends on M002/S06** throughout, and is the milestone that burns down
  what M002/S06 suppresses. Its S01 removes the seven `citation-tier-ok` markers
  one claim at a time, so the tier check only becomes unconditionally true when
  M006/S01 closes. Its S02 and S03 add the uncited-entry and duplicate-entry
  assertions to the same host, which is why they wait for the host to exist
  rather than racing it.
- **M005 depends on M002/S06** throughout: a reference added after the tier
  check exists must arrive carrying a tier. M005 is genuinely last, but it is
  queued rather than written off — every P0 and P1 row closes in M001–M004.

## Related issues

Every open issue overlapping this programme's remit is enumerated here, so the
two views cannot silently drift.

- [#282](https://github.com/JimAKennedy/poly/issues/282) — **not closed here**.
  B16 records that `e2e` rebuilds the committed WASM artifacts non-reproducibly
  and that this programme reverts them after each run; making the build
  reproducible is tracker work, not audit remediation.
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
