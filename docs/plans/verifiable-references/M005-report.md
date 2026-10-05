# M005 — One bibliography, one standard

**Review-gate report.** Generated from the ledger, `git log` and
`M005-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** The numbered list and Further Reading are a single bibliography
under a single standard.

**Branch:** `milestone/M005-one-bibliography` · **Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | Further Reading entries become citable | VR14 | accepted (not done, by decision) |
| M005/S02 | The two lists become one | VR15 | done |

## Definition of done

**S01** — none claimed; accepted as not done at planning (the reasons are in
`M005-S01-plan.md`):

- [ ] Every Further Reading entry carries a citation number, so the archive's
      `NN - Name.pdf` convention has a prefix to use for all 107
- [ ] Existing `fr-` anchors still resolve, or every citation of them is updated
      in the same change
- [ ] No numbered entry is renumbered — the ledger standard's rule that anchors
      are never reassigned applies to the guide's own citations too

**S02**, all satisfied:

- [x] The appendix presents one bibliography, ordered so a reader can find an
      entry from a citation without knowing which list it used to be in
- [x] Every entry carries the same fields: tier, obtainability, and an
      identifier or URL
- [x] No citation anywhere in the guide is broken by the merge

## The headline

**Each bibliography is now one list a reader can use without knowing its
history.** The shipping appendix (40 entries) and the theory bundle (73) each
lost their split into a numbered section and a Further Reading section:
every source sits under its chapter's group, numbered entries first, and
every entry is a list item. Entry text was not changed by the merge — the
sorted entry lines are byte-identical before and after in both files — and
every citation still resolves.

**Every entry now says how to get it.** Each ends with its route (its own
link, a DOI link, or an ISBN) and an access label from the manifest. In the
shipping appendix: 7 open access, 5 free online, 11 borrowable, 17
purchasable. One entry has no route by recorded exception: Novotney (1998),
an unpublished doctoral thesis with no public identifier, which says so on
the page.

**Numbering was not extended.** S01 and VR14 are accepted as not done: M003's
`FR - Surname Year` archive naming had already removed VR14's reason, and the
owner kept both labels — `[N]` for numbered entries, author–year for Further
Reading.

## Validation

| Slice | Token | Result |
|---|---|---|
| S01 | — | accepted at planning; no task ran |
| S02 | `site-unit` | exit 0, 398 tests (task 4) |
| S02 | `format` | exit 0 |
| S02 | `doc-conformance` | exit 0 |
| S02 | `doc-discipline` | exit 0; `research-provenance: all citations resolve` |

## Traceability

Every commit on the branch carries `Plan:` and `Slice:` lines.

- `dc24661` M005 planning — one bibliography: decisions, S01 accepted, and S02's plan — Slice M005/S02, Rows VR14
- `78f73e9` M005/S02 task 1 — the shipping appendix is one list — Slice M005/S02
- `d283350` M005/S02 task 2 — the theory bundle is one list — Slice M005/S02
- `08abc78` M005/S02 task 3 — every entry shows its route and access — Slice M005/S02
- `a0602de` M005/S02 task 4 — Novotney is cited as an unpublished doctoral thesis — Slice M005/S02, Rows VR15
- the report itself, the commit that carries this file

Hashes regenerated after the branch was rebased onto `main` at #355 (the
macOS deployment-target fix), which rewrote every commit on the branch.

**Untraced commits:** none.

## What a reviewer should look at twice

- **The shipping appendix looks different.** It is retitled "Appendix:
  References", grouped by chapter, and every entry now ends with a route and
  an access label such as *(Purchasable)*. Read it rendered before shipping.
- **S01 is accepted, not done.** The milestone's Vision is met by S02's
  single list; numbering every Further Reading entry was declined.
- **Novotney is the one exception** to every entry showing a route, by the
  owner's decision, held by a test that names it and requires the entry to
  say why.
- **Two manifest corrections:** Novotney's obtainability became
  `library-only` and Collins's `open-access`, matching what M003 found.
- **One judgment call:** an M003 naming test's "entry with no link" example
  (Anku) became a fixed line, because every entry now has a link.

## Decisions

## 2026-10-04 — planning M005/S01 and M005/S02

Measured before asking, on `main` at `51c4405`. The shipping appendix holds 40
entries (8 numbered, 32 Further Reading) and the theory bundle 73 (21
numbered, 52 Further Reading), each split into a numbered section and a
Further Reading section with its own chapter grouping. The shipping chapters
cite with 33 author–year links and 12 numbered superscripts, and
`citation-tier.test.mjs` uses `<sup>` as the marker of a named-theory claim
that must be Tier A, so the citation style is load-bearing for a test. Most
entries carry no link in their text, but the manifest holds a DOI or ISBN for
all but three: `fr-novotney-1998`, `fr-vitale-1990`, `fr-collins-2001`.
Vitale and Collins have known free URLs (M003); Novotney has none that could
be verified. M003's archive naming (`FR - Surname Year.pdf`) already gives
every Further Reading entry an archive name, which was VR14's stated reason
for numbering them.

- **Q:** What single citation standard should the merged bibliography use?
  — **A:** merge, keep both labels.
- **Decision:** one list per bibliography; numbered entries keep `[N]` and
  Further Reading entries stay author–year; nothing is renumbered. **M005/S01
  and VR14 are accepted as not done** — **Why:** VR14's reason (an archive
  prefix) was removed by M003's naming, author–year citation already lets a
  reader cite a Further Reading entry, and numbering 84 more entries would
  change every one of them for no reader-facing gain.
- **Q:** Which bibliographies does M005 cover? — **A:** both.
- **Decision:** the shipping appendix and the theory bundle, so the deep dives
  are ready to republish under the same standard (M004's rule).
- **Q:** How is the single list ordered? — **A:** by chapter.
- **Q:** How should a reader see each entry's fields? — **A:** a visible
  route and access label.
- **Decision:** every entry shows a route (its link, or a DOI link, or an
  ISBN) and an access label taken from the manifest; tier stays the existing
  `data-tier` attribute. A test checks every entry against the manifest.
- **Q:** `fr-novotney-1998` has no verifiable identifier or URL; how is it
  handled? — **A:** the owner finds a record.
- **Decision:** a **planned pause** at the end of M005/S02 (task 4). Until
  then the fields test names Novotney as the one pending entry, with the
  reason beside it; the pause removes the exemption.

### Taken on the owner's behalf

- **Headings are the Further Reading groups, promoted to the one list.** Each
  bibliography's sections become `##` headings named as its Further Reading
  groups already are — "Foundations and Cross-Cultural Rhythm Theory
  (Chapter 1)", "Electronic and Drum & Bass (Chapters 9, 13)", and so on —
  because those names already cover multi-chapter and cross-chapter sources.
  A numbered entry joins the group of its current chapter heading; within a
  group, numbered entries come first in number order, then Further Reading in
  its current order.
- **Every entry becomes a list item.** Numbered entries gain the leading
  `- ` the Further Reading entries already have; their `<span>` shape is
  unchanged, so every existing claim scoped to an entry's line still holds.
- **The appendix is retitled "Appendix: References".** "and Further Reading"
  names the split this milestone removes. The route (`/appendix-references/`)
  and every anchor are unchanged.
- **Access labels**, from the manifest's `obtainability`: `open-access` →
  "Open access"; `browser-only` → "Free online"; `borrowable` →
  "Borrowable"; `purchasable` → "Purchasable"; `library-only` → "Library".
- **Two manifest corrections the labels depend on.** `fr-novotney-1998` is
  `browser-only` but M003 recorded it `institution-only` (no free copy), so
  its obtainability becomes `library-only`; `fr-collins-2001` is free on the
  author's own site (M003), so it becomes `open-access` and gains that link.
- **A route is the entry's own link when it has one;** otherwise a DOI link
  from the manifest, else the ISBN as text. No entry is given a route the
  manifest or a recorded reading does not already hold.

### Deferred

- **The Novotney record**, waiting at M005/S02 task 4: the owner looks up
  ProQuest Dissertations or Illinois's IDEALS repository for a publication
  number, handle or OCLC number, and checks whether the degree was a PhD or a
  DMA. Planned pause, not a failure.

## 2026-10-04 — judgment calls during M005/S02 task 3

- **A naming test's "entry with no link" example became a fixed line.**
  M003's test used `fr-anku-2000` as an entry with no link; task 3 gave it a
  DOI route, as it gives every entry one, so the null-URL case now parses a
  fixed line through `parseLine` and Anku keeps the no-markup check.
  Obviously right: the same move M004 made for `ref-43`, for the same reason.

## 2026-10-05 — the Novotney record (the deferred question, answered)

> From a 2025 article in Empirical Musicology Review, I have the following citation: "Novotney, E. D. (1998). The 3:2 relationship as the foundation of timelines in West African musics. Unpublished doctoral thesis, University of Illinois at Urbana-Champaign." I think we should go with this since it's basically impossible to get hold of it, so logging as unpublished is likely as correct as you are going to get without access to a university library (which I do not have)

- **Q:** a ProQuest, IDEALS or WorldCat record for Novotney (1998), and PhD
  or DMA? — **A:** (above) no record is reachable without a university
  library; cite it as an unpublished doctoral thesis, as a 2025 *Empirical
  Musicology Review* article does.
- **Decision:** `fr-novotney-1998` becomes the bibliography's one recorded
  exception to "every entry shows a route": cited as "Novotney, E. D. (1998).
  … Unpublished doctoral thesis, University of Illinois at Urbana-Champaign",
  access label "Library", no identifier. The fields test's pending list
  becomes a named exception with this reason, and requires the entry to say
  "Unpublished doctoral thesis" so a reader sees why there is no route. "PhD
  dissertation" goes, because the degree was not established — **Why:** the
  owner's call; the plan named this case a halt for the owner to decide, and
  the owner decided it. The DoD's "identifier or URL" box is ticked with this
  one exception recorded, not silently.
