# First release — delivery ledger

**Source:** docs/plans/first-release/vision.md
**Slug:** first-release

Three subtractions and a docs pass. Nothing here needs designing; it needs
deciding and removing. The companion programme
[`docs/plans/installers/vision.md`](../installers/vision.md) covers how a
release is built and delivered — this one covers what is in it.

## Reconciliation, before any structure

Read the vision: 22 actionable items, plus 2 findings the vision does not
mention.

| | |
|---|---|
| Already satisfied | 0 — nothing in this vision has been built |
| Partly satisfied | 2 — `ref-1` is diagnosed but not corrected; the theory bibliography's data exists in M002's manifest but the split does not |
| Contradicted by the tree | 2 — the vision says eleven chapter callouts and eleven test files; there are **12** of each |
| Outstanding | 18 |
| Missing from the vision | 2 — the docs drift map and the doc-conformance runner both name theory paths |

The two contradictions are errors in the vision, not in the tree. The counts
below are measured.

### What the measurement changed

The vision proposed cutting the bibliography to 10–20. That target is
**declined** and recorded as row FR23, because the measurement that would have
implemented it disproved it: all 40 surviving citations sit in running prose
attached to a claim, and **zero** appear in bibliographic list lines. The
shipping chapters carry no decorative citations, so there is no fat to trim —
reaching 10–20 would mean removing claims or leaving claims unsourced. The
programme keeps 34 instead: every load-bearing citation that a reader can
obtain without an institution.

## Milestone M001 — Desk is the only view

**Vision:** The shipped plugin presents one main view, and everything a user can
do is reachable from it.

**Branch:** milestone/M001-desk-only
**Status:** done
**Demo:** A shipped build offers no mode chip, and MIDI capture is reachable
from the toolbar beside Export.

**Why capture moves first.** `webui/ui.js` gates the capture controls to Cloth
(`capCtl.classList.toggle('show', m === 'cloth')`). Removing Cloth before
ungating them would lose a shipped feature for the length of a commit, and the
engine capability was never Cloth-bound — chapter 16 documents capture as VST3
parameters 600 and 601.

### Slice M001/S01 — Capture reaches the toolbar

**Plan:** M001-S01-plan.md
**Validation:** format, webui-e2e
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] The capture bars control and Arm are visible without entering Cloth
- [x] They sit beside Export, which is already toolbar-level and unconditional
- [x] A test fails if either control becomes mode-dependent again

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR01 | The capture controls are gated to Cloth, so hiding Cloth would hide MIDI capture with it — the premise "all functionality is reachable from Desk" does not hold today | `defect` | `webui/ui.js`, `webui/index.html` | `capCtl` carries no mode class; a test asserts the controls are present with the mode set to desk | `done` |

### Slice M001/S02 — Cloth leaves the shipped build

**Depends:** M001/S01
**Plan:** M001-S02-plan.md
**Validation:** format, webui-e2e, doc-conformance, site-unit
**Evidence:** evidence/M001-S02.md
**Status:** done

**Definition of Done**

- [x] A shipped build contains no Cloth chip, no `#cloth` node, no loom canvas
      and no draw loop
- [x] Learn is gone, because its annotations only ever described the Cloth
      visualisation
- [x] `guide-using-poly.mdx` no longer describes a Cloth/Desk toggle
- [x] A test fails if a mode chip returns to the shipped UI

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR02 | Cloth is a second main view that is not finished, and a first release should not have to defend it | `docs` | `webui/index.html`, `webui/ui.js` | No mode chip and no `#cloth` node in the built UI; the source is retained in git history | `done` |
| FR03 | `learnBtn` reveals three annotations that exist only inside `#cloth`, so it has nothing to say once Cloth is gone | `defect` | `webui/index.html`, `webui/ui.js` | The chip and its handler are gone; no annotation text remains unreferenced | `done` |
| FR04 | `guide-using-poly.mdx` documents the Cloth/Desk toggle in prose, so the guide would describe a control the user cannot find | `docs` | `site/src/content/docs/guide-using-poly.mdx` | No mention of Cloth or of a mode toggle; the existing Desk screenshot still matches | `done` |
| FR05 | Nothing prevents a mode chip returning — the removal is reversible by accident as well as on purpose | `tooling` | `webui/`, `site/tests/` or `webui/` specs | A test fails when a mode chip is reintroduced, proved by adding one and watching it go red | `done` |

## Milestone M002 — The deep dives stop shipping

**Vision:** No theory deep dive renders, nothing links to one, and every guard
that asserts their content still runs.

**Branch:** milestone/M002-unpublish-theory
**Status:** planned
**Demo:** The site builds with no `theory-*` route and no link to one, and
`doc-conformance` still runs every guard that asserts a deep dive's content.

**Why the pointers go before the pages.** Moving the pages first would leave 12
chapter callouts and a nav group pointing at routes that no longer exist — a
commit where the published site is visibly broken. Removing the pointers first
leaves the pages live but unreferenced, which is harmless.

**The deep dives are deferred, not retired.** They are intended for publication
in a later release, so they move rather than being deleted, and the destination
is decided by their imports rather than by preference: every deep dive opens
with `import PolyPatch from '../../components/PolyPatch.astro'`, which resolves
to `site/src/components` from `site/src/content/theory/` and to a nonexistent
root-level `components/` from `docs/theory/`. A sibling under
`site/src/content/` keeps the imports byte-identical, and
`site/src/content.config.ts` defines only the `docs` collection, so a sibling
directory renders nothing.

### Slice M002/S01 — The site stops pointing at them

**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S01.md
**Status:** open

**Definition of Done**

- [ ] No navigation entry offers a theory deep dive
- [ ] No chapter carries a `:::note[Theory deep dive]` callout
- [ ] No prose anywhere on the site tells a reader the deep dives exist
- [ ] The site builds with no broken internal link

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR06 | `site/astro.config.mjs` presents a `Theory Deep Dives` nav group with 12 entries | `docs` | `site/astro.config.mjs` | The group is absent and the sidebar has no theory entry | `open` |
| FR07 | **12** chapters carry a `:::note[Theory deep dive]` callout linking to a page that will not exist | `docs` | `site/src/content/docs/0*.mdx`, `1*.mdx` | No callout remains; the count is read from the tree, not from this row | `open` |
| FR08 | `about-this-guide.mdx` explains the deep dives as part of how the guide works | `docs` | `site/src/content/docs/about-this-guide.mdx` | The passage is gone and what remains describes the guide that ships | `open` |
| FR09 | `appendix-references.mdx` names the deep dives in its opening sentence | `docs` | `site/src/content/docs/appendix-references.mdx` | The opening describes the shipping bibliography only | `open` |

### Slice M002/S02 — The pages move, and the guards follow

**Depends:** M002/S01
**Validation:** format, site-unit, doc-conformance, doc-discipline, guards
**Evidence:** evidence/M002-S02.md
**Status:** open

**Definition of Done**

- [ ] No `theory-*` page is a published route
- [ ] Every guard that asserted a deep dive's content still runs, against the
      new path
- [ ] The moved bundle carries the references only it cites, so republishing
      needs no citation repair
- [ ] `jk-standards.yaml` records why a sibling directory falls outside its
      roots, so a later widening is deliberate

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR10 | 12 `theory-*.mdx` files are published routes inside the Starlight docs collection | `docs` | `site/src/content/theory/` | The build emits no theory route; the files exist at the new path with their imports unchanged | `open` |
| FR11 | **12** `.mjs` test files assert deep-dive content by path — `theory-patch-conformance` alone references them 71 times. Deleting the pages would retire guards a whole milestone built | `tooling` | `site/tests/` | Every one of the 12 runs green against the new path; none is deleted or skipped | `open` |
| FR12 | `scripts/check-doc-conformance.sh` names 3 theory test files by path, so the conformance runner stops finding them after a move. Not named in the vision | `tooling` | `scripts/check-doc-conformance.sh` | The runner executes the same test set it did before the move | `open` |
| FR13 | `.github/docs-drift-map.yml` maps 12 theory docs by full path, so `doc-drift` breaks on the move. Not named in the vision | `tooling` | `.github/docs-drift-map.yml` | `doc-discipline` passes, including the `doc-drift` arm | `open` |
| FR14 | 66 anchors are cited only by the deep dives. If the shipping bibliography shrinks while they still point at it, those citations dangle the day someone republishes — and nothing warns, because the provenance check will no longer scan them | `defect` | `site/src/content/theory/theory-references.mdx` | Every anchor the moved pages cite resolves within the moved bundle, proved by a check that reads both | `open` |
| FR15 | The scoping that makes the move work — `doc_roots` and `research_provenance.doc_roots` both limited to `site/src/content/docs` — is a consequence of a path list rather than an expressed intent | `tooling` | `jk-standards.yaml` | A comment beside both roots states that theory content is deliberately outside them | `open` |

## Milestone M003 — A bibliography a reader can finish

**Vision:** Every reference in the shipping guide supports a claim, is Tier A,
and can be obtained without an institution.

**Branch:** milestone/M003-bibliography
**Status:** planned
**Demo:** The appendix holds 34 entries, every one cited from a claim in prose,
and no entry requires an institutional subscription.

**Why this waits for M002/S02.** The deep dives cite 66 anchors that nothing
else cites. Until they and their bibliography have moved, removing an entry from
the appendix breaks a citation in a page that still scans.

### Slice M003/S01 — The two mismatches are corrected

**Depends:** M002/S02
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S01.md
**Status:** open

**Definition of Done**

- [ ] `ref-1` names and links the same work
- [ ] `ref-34` attributes the article to its author
- [ ] Each correction is locked by a claim test seen red before the edit

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR16 | `ref-1` names Toussaint (2005), "The Euclidean Algorithm Generates Traditional Musical Rhythms", and links arXiv:0705.4085, which is "The Distance Geometry of Music" by eight authors in 2007. It is the guide's most-cited reference at six chapters and underpins the Euclidean claim the generator rests on | `defect` | `site/src/content/docs/appendix-references.mdx`, `site/tests/citation-tier.test.mjs` | The entry points at the 2005 paper; a claim test forbids the old pairing and is mutation-proved | `open` |
| FR17 | `ref-34` attributes to Reich an article *about* Reich by K. Robert Schwarz, *Perspectives of New Music* 20(1/2) | `defect` | `appendix-references.mdx`, `site/tests/citation-tier.test.mjs` | The entry names Schwarz; a claim test pins it | `open` |

### Slice M003/S02 — Nothing needs an institution

**Depends:** M003/S01
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M003-S02.md
**Status:** open

**Definition of Done**

- [ ] No entry in the shipping appendix is `library-only`
- [ ] Every claim whose source left carries a source a reader can obtain, or no
      longer asserts something that needs one
- [ ] The appendix contains only entries cited from a shipping page
- [ ] The verifiable-references ledger records what this programme changed about
      its remaining milestones

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR18 | 6 load-bearing entries are `library-only` — obtainable only through an institution, which fails the programme's own test of "a reader can get it" | `docs` | `appendix-references.mdx`, citing chapters | None remain; each affected claim either cites an obtainable source or no longer needs one | `open` |
| FR19 | The appendix carries 107 entries while the shipping pages cite 40. After the deep dives move, the remainder are cited by nothing | `docs` | `appendix-references.mdx` | The appendix holds 34 entries and every one is cited from a shipping page | `open` |
| FR24 | The verifiable-references ledger's M003, M004 and M005 are scoped to 107 entries and a two-list bibliography that this programme removes. M004 is largely solved by unpublishing the deep dives, since every Tier C entry is cited only from them | `docs` | `docs/plans/verifiable-references/ledger.md` | Those milestones record what changed and what remains, rather than being silently overtaken | `open` |

### Slice M003/S03 — A guard keeps it true

**Depends:** M003/S02
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M003-S03.md
**Status:** open

**Definition of Done**

- [ ] A test fails if an appendix entry is cited from nowhere
- [ ] A test fails if an entry is not Tier A, or is `library-only`
- [ ] Both arms are shown to fail before being trusted

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR20 | Nothing prevents an uncited, lower-tier or institution-only entry returning to the appendix; the 34-entry state would hold only until the next edit | `tooling` | `site/tests/`, `site/src/data/references.json` | Both arms mutation-proved: an uncited entry fails, and a `library-only` entry fails | `open` |
| FR23 | The vision asked for 10–20 references. **Declined.** All 40 surviving citations sit in running prose attached to a claim and none appears in a bibliographic list, so the shipping chapters carry no decorative citations. Reaching 10–20 would mean removing claims or leaving claims unsourced; the programme keeps 34 — every load-bearing citation a reader can obtain without an institution | `docs` | — | Recorded so a later pass does not rediscover the target as an omission | `accepted` |

## Milestone M004 — The docs describe what ships

**Vision:** Someone reading the guide finds the product they downloaded.

**Branch:** milestone/M004-docs-match
**Status:** planned
**Demo:** Every screenshot, control description and chapter in the published
guide matches a one-view build with no theory section.

**Deliberately narrow.** Tagging, signing, notarisation and installers belong to
the installers programme. This milestone only closes the gap between what the
guide says and what a downloader gets.

### Slice M004/S01 — The guide matches the build

**Depends:** M001/S02, M002/S01
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M004-S01.md
**Status:** open

**Definition of Done**

- [ ] No screenshot shows a control the shipped UI does not have
- [ ] Chapter 18 describes the views that exist
- [ ] `CHANGELOG.md` records the removals under the right heading

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| FR21 | `guide-using-poly.mdx` carries a screenshot and control walkthrough taken against a two-view UI | `docs` | `site/src/content/docs/guide-using-poly.mdx` | Screenshot and prose match a build with one view | `open` |
| FR22 | Chapter 18 and the changelog describe the editor surface without accounting for a removed view or a removed documentation section | `docs` | `site/src/content/docs/18-editors-and-views.mdx`, `CHANGELOG.md` | Both describe what ships; `doc-discipline` passes | `open` |

## Sequencing

**M001 and M002 are independent.** One is the plugin UI, the other the published
site; they share no file.

**M003 depends on M002/S02** and not merely on M002. Until the deep dives and
their bibliography have moved, the appendix is still scanned on their behalf, so
removing an entry breaks a citation in a page the provenance check still reads.

**M004 depends on M001/S02 and M002/S01** — it cannot describe what ships until
the view is gone and the section is unlinked. It does not wait for M003, because
a bibliography's size is not something a screenshot shows.

Within M002 the order is reversed from the obvious one on purpose: pointers
first, pages second, so no commit leaves a live link pointing at a missing
route.

## Related issues

None yet. Issues raised during execution are added here with the row they
belong to.

## Out of scope

- Installer, signing and notarisation — the installers programme owns these, and
  together they are the other half of "first release"
- Any change to the engine, the preset set, or the parameter surface
- Rewriting chapter prose beyond what removing a callout or resourcing a claim
  requires
- Republishing the deep dives. M002 leaves them buildable, not built
