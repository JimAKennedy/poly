# M005/S02 — The two lists become one

**Slice:** M005/S02 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR15 (two bibliographies in one appendix, no stated relationship)
**Classification:** bounded. A restructure of two `.mdx` files under tests
that already parse them, plus one fields test against the manifest. No claim
text changes; every anchor stays. Decisions are in `M005-decisions.md`.

## Task status

- [x] 1. The shipping appendix is one list
- [x] 2. The theory bundle is one list
- [x] 3. Every entry shows its route and access
- [ ] 4. Novotney's record (planned pause)

## Definition of Done

Copied verbatim from the slice:

- [ ] The appendix presents one bibliography, ordered so a reader can find an
      entry from a citation without knowing which list it used to be in
- [ ] Every entry carries the same fields: tier, obtainability, and an
      identifier or URL
- [ ] No citation anywhere in the guide is broken by the merge

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

VR15's verification names `research-provenance` (run by `doc-discipline`)
and the manifest completeness test (run by `site-unit`); both must pass after
every task. "No citation is broken" is proved by those two plus
`theory-bundle-references.test.mjs`, which checks every deep-dive anchor
resolves.

## The shape

Both files take the same shape, built by the same rule:

```
## Foundations and Cross-Cultural Rhythm Theory (Chapter 1)

- <span id="ref-1" …>**[1]**</span> Toussaint, … *(Open access)*
- <span id="ref-46" …>**[46]**</span> Bjorklund, … *(Open access)*
- <span id="fr-toussaint-2013" …>Toussaint, G. T. (2013). … — gloss</span> ISBN … *(Purchasable)*
```

The headings are the Further Reading group names, in their current order. A
numbered entry joins the group of its current chapter heading (appendix "Chapter 1:
Foundations" and "Chapter 1: Foundations (additional)" both go to the Chapter
1 group; "Chapter 9: Electronic" to "Electronic and Drum & Bass (Chapters 9,
13)"). Within a group: numbered entries in number order, then Further Reading
in current order. The `---` separator and the numbered section's headings go.

## Task 1 — The shipping appendix is one list

**Files:** `site/tests/references-manifest.test.mjs`,
`site/src/content/docs/appendix-references.mdx`; the evidence file.

1. Add a test `each bibliography is one list, grouped once` over both files
   (a list of paths; this task asserts the appendix, task 2 adds the bundle):
   no line matches `^### `; no line matches `^---$` after the frontmatter;
   no heading text repeats; every line carrying `id="(ref|fr)-` starts with
   `- <span id=`; and every such line sits after the first `## ` heading.
   Run it: red on the appendix.
2. Restructure `appendix-references.mdx` by the rule above: move each
   numbered entry into its group as a list item, promote `###` group headings
   to `##`, delete the numbered section's headings and the `---`. Retitle the
   frontmatter `title` to "Appendix: References". Change no entry's text.
3. Prove nothing moved but its position: the multiset of entry lines (with
   any leading `- ` stripped) is identical before and after — compare sorted
   lists with a one-off command and record the result in the evidence.
4. Run the test (green), `site-unit`, `format`, `doc-conformance`,
   `doc-discipline`. Commit with `Rows:` empty.

## Task 2 — The theory bundle is one list

**Files:** `site/tests/references-manifest.test.mjs`,
`site/src/content/theory/theory-references.mdx`; the evidence file.

1. Add the bundle's path to the one-list test. Run it: red.
2. Restructure `theory-references.mdx` the same way. Its extra group,
   "Microtiming and Groove Science (cross-chapter)", stays last.
3. The same before/after multiset proof, recorded.
4. Run the test, `site-unit` (including `theory-bundle-references.test.mjs`
   both ways), `format`, `doc-conformance`, `doc-discipline`. Commit.

## Task 3 — Every entry shows its route and access

**Files:** `site/tests/references-manifest.test.mjs`, both bibliographies,
`site/src/data/references.json`; the evidence file.

1. Add a test `every entry shows its route and access`: for each entry line
   in both files, take the anchor's manifest record and require (a) exactly
   one access label from the mapping in the decisions file, written
   `*(Label)*`, matching its `obtainability`; and (b) a route — a markdown
   link, or `ISBN ` followed by digits. `fr-novotney-1998` is listed in a
   `PENDING_ROUTE` array with the reason ("the owner is finding its record,
   M005/S02 task 4"); it must still carry its access label, and the test
   fails if `PENDING_ROUTE` names an anchor that already has a route. Run
   it: red on every entry.
2. Manifest corrections: `fr-novotney-1998` obtainability → `library-only`;
   `fr-collins-2001` → `open-access`. Each gets a dated `note` line.
3. For each entry, append `*(Label)*` at the end of the line. Where the
   entry has no markdown link: add `[DOI](https://doi.org/<doi>)` from the
   manifest, else `ISBN <isbn>`, placed before the label. Collins gains
   `[PDF](https://composerprogrammer.com/research/acmethodsforbbsci.pdf)`;
   Vitale gains its `gamelan.org` PDF URL from the manifest's evidence.
   Never invent a route: an entry with none of these is a halt.
4. Run the fields test (green), the whole suite, `format`,
   `doc-conformance`, `doc-discipline`. Read a sample of five rendered lines
   in the diff from each file before committing.
5. Commit with `Rows:` empty.

## Task 4 — Novotney's record (planned pause)

**The executor stops before this task** and asks the owner for a ProQuest
publication number, an IDEALS handle, or a WorldCat OCLC number, and whether
the degree was a PhD or a DMA. On resumption:

1. Append the owner's answer verbatim to `M005-decisions.md`.
2. Remove `fr-novotney-1998` from `PENDING_ROUTE`; run the fields test: red.
3. Add the route to the entry in `theory-references.mdx` (and correct
   "PhD dissertation" if the owner found a DMA); record the identifier in the
   manifest (`identifier` or `note`) and set `checked`.
4. Run the fields test (green), the whole suite, `format`,
   `doc-conformance`, `doc-discipline`.
5. Evidence; tick the DoD; set VR15 and the slice `done`; run
   `jk-standards ledger`. Commit with `Rows: VR15`.

If the owner finds no record, that is a halt, not an exception taken here:
the owner then chooses between a recorded exception and retiring the entry.

## Self-review

- **DoD 1** (one bibliography, findable from a citation) — tasks 1–2; a
  citation is an anchor link, so findability is unchanged and the grouping is
  by chapter, where a reader comes from.
- **DoD 2** (same fields) — task 3 (tier attribute, access label, route),
  completed by task 4.
- **DoD 3** (no citation broken) — every task runs `research-provenance`,
  the manifest completeness test and the bundle test; the multiset proofs show
  no entry text changed in tasks 1–2.
- **VR15** — "One list; research-provenance and the manifest completeness
  test both pass": tasks 1–2 and every gate run.
- **Placeholders** — none; the label mapping and the route rule are in the
  decisions file, and task 4's content is the owner's by design.
