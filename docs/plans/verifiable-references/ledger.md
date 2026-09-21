---
class: gated
---

# Verifiable references — delivery ledger

Status: current (2026-09-20)

Assessed from `docs/plans/verifiable-references/vision.md` on 2026-09-20. That
document is research; this file is the plan of record.

**Source:** —  <!-- ledger-ok: no spec system in this repo; the vision document is the input and `openspec/` is absent -->

## The programme

The guide cites 107 sources and has no systematic answer to *how do I read
this*. The appendix holds two bibliographies with opposite problems: 43
numbered entries, every one clickable and a third of them Tier C — five YouTube
videos, three Wikipedia articles, a Scribd upload, blog and course-marketing
pages; and 64 Further Reading entries, almost all Tier A scholarship, **none
carrying a URL or any other indication of where to get it.**

That split is not neglect. Theory-audit M005 added the 64 good sources and did
exactly what it promised; it never promised to clean the numbered list. The
unreformed list is simply the one at the top of the page.

**The principle is obtainability plus accuracy.** A reference is good when the
text can be obtained and read, *and* the citation describes the work it points
at. The second half was added after the first draft of the vision, because the
first reference examined closely failed only the second test: `ref-9` was
freely downloadable, correctly attributed, and named a dissertation that does
not exist — printed as being about West African drumming when it is about
African pianism.

**It was the third such defect, not the first.** Of the numbered references
anyone has ever checked against their actual source, all three misdescribed what
they cite: `ref-2` carried a fabricated title over a real author, journal and
URL (theory-audit F17); `ref-6` cites Jones (1959) and links to a Cambridge
review *of* the book; `ref-9` as above. Three is not a rate to extrapolate from,
and the sample was not adversarially chosen — `ref-9` was opened because its
link was dead.

**Rows are defect classes; the manifest tracks entries.** Per-reference
state — obtainability, verified description, archive filename — lives in
`site/src/data/references.json`, a hand-recorded data file, not in
`site/src/generated/`: these are human verdicts, not emitter output. The ledger
stays a plan; the manifest is where progress across 107 entries is visible.

## Milestone M001 — The known defects are fixed

**Vision:** The references this audit already proved broken — two dead links and
one citing a review instead of the work — are corrected and locked.

**Branch:** milestone/M001-known-defects
**Status:** done
**Demo:** Every URL in the numbered bibliography either resolves or is recorded
as deliberately unresolvable, and no entry links to a commentary on itself.

**Why this is not folded into M004.** All three were measured before the
programme began and none needs the 107-entry audit to fix. Leaving a dead link
live behind weeks of classification work is over-serialising for tidiness.

### Slice M001/S01 — The three measured defects are corrected

**Plan:** M001-S01-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] `ref-26` and `ref-22` each resolve, or are replaced, or are recorded as
      deliberately removed with the claim they supported rewritten
- [x] `ref-6` links to Jones (1959) itself, or names plainly that the link is to
      a review and cites the book by ISBN
- [x] Each correction is locked by a claim test that fails if the old form
      returns

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR01 | `ref-26` (`fiveable.me`, Tier B) returns 404. It is a course-marketing study guide, so under the obtainability principle this is a replacement rather than a repair | `defect` | `appendix-references.mdx` | The URL resolves or the entry is gone; a claim test pins the outcome | `done` |
| VR02 | `ref-22` (`nios.ac.in`, Tier B) does not respond. NIOS is India's National Institute of Open Schooling and legitimate — the PDF moved rather than the source being bad | `defect` | `appendix-references.mdx` | The URL resolves or the entry is gone; a claim test pins the outcome | `done` |
| VR03 | `ref-6` cites Jones (1959) *Studies in African Music* and links to a Cambridge **review** of the book. A reader reaches someone else's two-page opinion of a work they still cannot read | `defect` | `appendix-references.mdx` | The entry links to the work or names the surrogate explicitly; a claim test pins it | `done` |

## Milestone M002 — Every reference has a verdict

**Vision:** Each of the 107 references carries a recorded obtainability status
and a description verified against the source itself.

**Branch:** milestone/M002-verdicts
**Status:** in-progress
**Demo:** `site/src/data/references.json` holds one record per bibliography
entry, and a test fails if an entry exists without one.

**The accuracy half is the expensive one and does not parallelise onto a
script.** Title and year usually come off a landing page; subject fitness
sometimes cannot — establishing that Oluranti (2012) is about pianism took
opening the PDF. Expect a long tail where the only way to answer is to look.

### Slice M002/S01 — The manifest exists and cannot drift from the bibliography

**Plan:** M002-S01-plan.md
**Validation:** format, site-unit, guards
**Evidence:** evidence/M002-S01.md
**Status:** done

**Definition of Done**

- [x] `site/src/data/references.json` has a declared shape carrying, per entry:
      anchor id, obtainability, description verdict, archive filename, and
      ISBN/DOI where one exists
- [x] A test fails when a bibliography anchor has no manifest record, and when a
      manifest record names an anchor that does not exist
- [x] The test is shown to fail in both directions before being trusted

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR04 | Nothing records obtainability or accuracy anywhere, so a later pass cannot tell a verified entry from an unexamined one. Silence currently implies "checked" | `tooling` | `site/src/data/references.json`, `site/tests/` | Both arms mutation-proved: an unrecorded anchor fails, and an orphan record fails | `done` |
| VR05 | The archive root differs per machine and is a personal path, which `check-personal-paths` rejects in tracked files — it caught the absolute form in the vision's own first draft | `tooling` | `site/src/data/references.json`, `scripts/` | Filenames are recorded relative to a root supplied by the environment; `guards` stays green | `done` |

### Slice M002/S02 — All 107 carry both verdicts

**Depends:** M002/S01
**Validation:** format, site-unit
**Evidence:** evidence/M002-S02.md
**Status:** open

**Definition of Done**

- [ ] Every one of the 107 entries has an obtainability verdict with its
      evidence, and a price where the verdict is "purchasable"
- [ ] Every entry has a description verdict — verified, mismatch, or
      **unverified** — reached against the source, not against the guide's entry
- [ ] Every mismatch is recorded with what the source actually is, so M004 can
      act on it without repeating the work

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR06 | 107 references with no recorded availability. "Purchasable" without a price is not a decision anyone can make, so the verdict carries one | `docs` | `site/src/data/references.json` | The manifest is complete; the completeness test from M002/S01 passes | `open` |
| VR07 | A citation can name a work that does not exist while every mechanical check passes — a link checker sees 200, a tier check sees the venue, the anchor check sees a defined id, and none sees the pairing. Three of three examined entries failed this, and a fourth (`ref-23`) surfaced during M001 without being looked for | `defect` | `site/src/data/references.json` | Each entry carries a description verdict; "unverified" is recorded rather than assumed | `open` |
| VR08 | The 64 Further Reading entries carry no URL, ISBN or DOI, so a reader who wants one has no route at all | `docs` | `site/src/data/references.json` | Each carries an obtainability verdict and an identifier where one exists | `open` |
| VR17 | `ref-23` is suspected of naming both the wrong title and the wrong journal: the article at its URL appears to be "Indian Rhythmic Systems as Sources of Inspiration for…" in *Analytical Approaches to World Music* 11(2), while the entry says "…in Comparative Perspective" in the *Journal of the International Folk Art and World Music Society* — a name that looks like a guessed expansion of "iftawm". Seeded from M001 so this pass does not rediscover it | `defect` | `site/src/data/references.json`, `appendix-references.mdx` | The real title and journal are read off the source and recorded; the entry matches them | `open` |

## Milestone M003 — The archive exists

**Vision:** Every source that can be obtained is in the archive, and the
repository records what it holds without holding it.

**Branch:** milestone/M003-archive
**Status:** planned
**Demo:** The archive holds every source classified as retrievable, and the
manifest names each one.

**The archive lives outside the repository.** These are copyrighted works, the
repository is public, and a gitignored directory is one `git add -f` from
redistributing them. The repository holds the manifest; the files live in a
Dropbox research folder addressed through an environment variable.

### Slice M003/S01 — Scripted retrieval fills what it can

**Depends:** M002/S02
**Validation:** format, site-unit, guards
**Evidence:** evidence/M003-S01.md
**Status:** open

**Definition of Done**

- [ ] Every source classified as a plain fetch is in the archive under the
      `NN - Name.pdf` convention, with the manifest updated to match
- [ ] The retrieval reads its destination from the environment and fails with a
      clear message when unset, rather than writing somewhere arbitrary
- [ ] Sources that resist scripted fetching are recorded as such, not retried
      silently

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR09 | Four sources are already collected by hand under an `NN - Name.pdf` convention. Nothing else is archived, and nothing records the convention | `tooling` | `scripts/`, the archive | Scripted-class sources are present and named; the manifest lists them | `open` |

### Slice M003/S02 — The browser worklist finishes the archive

**Depends:** M003/S01
**Validation:** format, site-unit
**Evidence:** evidence/M003-S02.md
**Status:** open

**Definition of Done**

- [ ] A worklist names every browser-only source: URL, what to save, and the
      exact filename to save it as
- [ ] The worklist has been worked and the archive contains its results
- [ ] Sources that are free but only as unsearchable scans, or free only to an
      institution, are recorded as what they are rather than as archived

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR10 | Scholarly hosts routinely refuse scripts — five entries return 403 and one 406 to automation. One is confirmed to load fine by hand: the owner reached the D-Scholarship@Pitt record behind `ref-9` in a browser and downloaded the PDF, from the same URL that 403s to every script tried. The rest are inferred from the response class, not measured. Treating "a script cannot fetch it" as "unobtainable" would wrongly condemn good sources | `docs` | `docs/plans/verifiable-references/`, the archive | The worklist exists and its entries are archived; the manifest distinguishes retrieved from unretrievable | `open` |

## Milestone M004 — Nothing unreviewable is cited

**Vision:** No claim in the guide rests on a source with no text to review.

**Branch:** milestone/M004-reviewable
**Status:** planned
**Demo:** No YouTube link remains in the bibliography, and every entry that
stays is either reviewable text or recorded as a primary artefact with its
reason.

**This is the milestone that needs judgement rather than tooling.** Sixteen
editorial decisions plus one policy call, and no amount of scripting makes them
faster.

### Slice M004/S01 — The Wikipedia policy is decided and applied

**Depends:** M002/S02
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M004-S01.md
**Status:** open

**Definition of Done**

- [ ] The policy is written down: whether Wikipedia may be cited at all, and if
      so for what — with its reason, once, rather than per entry
- [ ] The three Wikipedia entries conform to it
- [ ] Any claim that loses its citation is rewritten to need none, not left
      uncited

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR11 | Three Wikipedia entries are cited from the guide. Wikipedia is reviewable text and often a fair summary, and is also not a source a scholarly guide should rest a claim on. Deciding "never" and deciding "orientation only" are both defensible; deciding per entry is not | `docs` | `appendix-references.mdx`, citing chapters | The policy is stated once and the three entries match it; a claim test pins the outcome | `open` |

### Slice M004/S02 — The remaining unreviewable entries go

**Depends:** M004/S01
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M004-S02.md
**Status:** open

**Definition of Done**

- [ ] No YouTube entry remains in the bibliography
- [ ] Every remaining Tier-C entry is either reviewable text, or the primary
      artefact rather than a commentary on one, with that reason recorded
- [ ] Every entry M002/S02 marked as a description mismatch is corrected or
      replaced

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR12 | Five YouTube entries carry no text to review, no page to cite, and no way to check that they say what the guide claims. A video may be excellent and still fail this test | `docs` | `appendix-references.mdx`, citing chapters | No `youtube.com` URL remains; a claim test forbids their return | `open` |
| VR13 | Eleven further Tier-C entries — Scribd, blogs, course-marketing pages — are cited from the guide. Some are primary artefacts and should stay; the rest are commentary that a better source says properly | `docs` | `appendix-references.mdx`, citing chapters | Each is replaced, kept with a recorded reason, or dropped with its claim rewritten | `open` |

## Milestone M005 — One bibliography, one standard

**Vision:** The numbered list and Further Reading are a single bibliography
under a single standard.

**Branch:** milestone/M005-one-bibliography
**Status:** planned
**Demo:** A reader reaching the references appendix sees one list, and the
scholarship M005 added is as reachable as the numbered entries.

**Deliberately last.** Merging before M004 mixes good sources into a list still
carrying unreviewable ones; merging before M002 means renumbering twice.

### Slice M005/S01 — Further Reading entries become citable

**Depends:** M004/S02
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M005-S01.md
**Status:** open

**Definition of Done**

- [ ] Every Further Reading entry carries a citation number, so the archive's
      `NN - Name.pdf` convention has a prefix to use for all 107
- [ ] Existing `fr-` anchors still resolve, or every citation of them is updated
      in the same change
- [ ] No numbered entry is renumbered — the ledger standard's rule that anchors
      are never reassigned applies to the guide's own citations too

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR14 | The 64 Further Reading entries have no citation number, so the archive naming convention has no prefix for them and a reader cannot cite one the way they cite `[9]` | `docs` | `appendix-references.mdx`, citing chapters | Every entry has a number; `research-provenance` reports every citation resolving | `open` |

### Slice M005/S02 — The two lists become one

**Depends:** M005/S01
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M005-S02.md
**Status:** open

**Definition of Done**

- [ ] The appendix presents one bibliography, ordered so a reader can find an
      entry from a citation without knowing which list it used to be in
- [ ] Every entry carries the same fields: tier, obtainability, and an
      identifier or URL
- [ ] No citation anywhere in the guide is broken by the merge

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR15 | Two bibliographies in one appendix, with opposite problems and no stated relationship. The good scholarship sits below a section a reader may never reach | `docs` | `appendix-references.mdx` | One list; `research-provenance` and the manifest completeness test both pass | `open` |

## Milestone M006 — A dead reference is found by a check

**Vision:** Link rot is reported by a scheduled job rather than discovered by a
reader.

**Branch:** milestone/M006-liveness
**Status:** planned
**Demo:** A deliberately broken URL appears in the next scheduled report.

**Advisory, never a gate.** A hard gate on third-party availability makes every
unrelated pull request hostage to somebody else's web server, and a flaky gate
gets disabled — which is worse than not having one.

### Slice M006/S01 — The scheduled check reports

**Validation:** format, guards
**Evidence:** evidence/M006-S01.md
**Status:** open

**Definition of Done**

- [ ] A scheduled job checks every URL in the bibliography and reports what it
      found, without failing a pull request
- [ ] It does not report the browser-only class as dead: 403 and 406 are
      distinguished from 404 and no-response
- [ ] It is shown to detect a genuinely dead URL, by introducing one

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| VR16 | Nothing fetches anything. `research-provenance` checks that a citation resolves to an anchor and `citation-tier` checks the declared tier; a Tier-A source rotted to a 404 with every gate green | `tooling` | `.github/workflows/`, `scripts/` | The job runs and reports; mutation-proved by breaking a URL and seeing it named | `open` |

## Sequencing

**M001 and M006 block nothing and are blocked by nothing.** M001 fixes three
defects already measured; M006 builds the reporting. Either can land first.

**M002 blocks M003 and M004.** You cannot archive what you have not classified,
and you cannot decide which entries to replace without knowing which
misdescribe their source.

**M003 and M004 are independent of each other.** One is retrieval, the other is
editorial judgement. They touch the manifest and the prose respectively, and can
run in parallel.

**M005 depends on M004**, because merging a list you are still editing means
merging twice.

**The bottleneck is judgement, not scripting**, and M002 carries more of it than
M004. M002/S02 is 107 paired verdicts with a tail that can only be settled by
opening the source. M004 is 16 editorial decisions plus a policy call. Neither
gets faster with better tooling.

## Related issues

- [#100](https://github.com/JimAKennedy/poly/issues/100) — `appendix-presets.mdx`
  documents 14 of 43 presets. Adjacent but not this programme: it is about the
  preset appendix, not the bibliography. Its title also carries a stale count —
  the engine ships 45.

## Out of scope

Recorded so a later pass does not rediscover them as omissions.

- **Re-reviewing whether each claim is true.** That was the theory-audit
  programme. The boundary is fine but real: this programme asks whether a
  citation names the work it links to and whether that work is in the right
  subject area. Whether the work *supports* the claim stays the theory audit's
  question — `ref-9` failed this programme's test without anyone re-reading the
  argument it was cited for.
- **Redistributing the archive.** The PDFs stay local and the repository holds a
  manifest. Hosting them is a licensing question, not a tooling one.
- **Citations outside the guide.** `docs/` and code comments cite things too;
  this programme is the site's bibliography.
- **The tier scheme.** Obtainability and accuracy are recorded alongside it, not
  instead of it, unless M005 finds the two genuinely conflict.
- **Automated title matching.** Confirming a title exists means fetching, and
  scholarly hosts refuse scripts. jk-standards' `research-provenance` skill
  carries this reasoning as portable guidance after this audit surfaced it.
