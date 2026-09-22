# M003/S01 — The two mismatches are corrected

**Slice:** M003/S01 in `docs/plans/first-release/ledger.md`
**Rows:** FR16 (`ref-1`), FR17 (`ref-34`)
**Depends:** M002/S02 — done.
**Classification:** bounded. Two bibliography entries and two cases in
`site/tests/citation-tier.test.mjs`, which already carries `REF6-JONES`,
`REF22-CLAYTON` and `REF26-AKSAK` in exactly this shape from the
verifiable-references programme.

## Task status

- [x] 1. `ref-1` names and links the same work
- [x] 2. `ref-34` attributes the article to its author, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] `ref-1` names and links the same work
- [ ] `ref-34` attributes the article to its author
- [ ] Each correction is locked by a claim test seen red before the edit

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## What M002 of verifiable-references established

Both defects are diagnosed, not suspected. The evidence is in
`site/src/data/references.json` and was measured by reading the documents.

**`ref-1`** names Toussaint (2005), "The Euclidean Algorithm Generates
Traditional Musical Rhythms", *Proceedings of BRIDGES*, and links
`arxiv.org/pdf/0705.4085.pdf`. That URL returns **200** and serves "The Distance
Geometry of Music" by Demaine, Gomez-Martin, Meijer, Rappaport, Taslakian,
Toussaint, Winograd and Wood — the PDF's own margin stamp reads
`arXiv:0705.4085v1 [cs.CG] 28 May 2007`. Toussaint is one of eight authors. It is
the guide's most-cited reference, at six shipping chapters, and it underpins the
Euclidean claim the generator rests on.

**`ref-34`** cites Reich's own "Music as a Gradual Process, Part II". The PDF at
its URL is "Steve Reich: Music as a Gradual Process Part II" by **K. Robert
Schwarz**, *Perspectives of New Music* 20(1/2), 1981–82, pp. 225–286, JSTOR
stable URL 942414 — an article *about* Reich's process, not Reich's essay.

Neither URL is dead. That is why no link checker ever flagged either.

---

## Task 1 — `ref-1` names and links the same work

**Consumes:** nothing. **Produces:** the corrected foundational citation.

The 2005 BRIDGES paper is real and is the work the guide means. Its canonical
home is the BRIDGES proceedings archive; `arxiv.org/abs/0705.4085` is **not** it.
Find the paper's actual location before editing — the archive at
`archive.bridgesmathart.org` hosts the proceedings, and Toussaint's own pages
have historically carried a copy. **Do not invent a URL**: if no stable link can
be verified by fetching it, cite the paper by title, venue and year with no link
rather than a guessed one, and say so in the entry.

1. Add `REF1-TOUSSAINT` to the `CLAIMS` array in
   `site/tests/citation-tier.test.mjs`, beside the existing `REF6-JONES` and
   `REF26-AKSAK` cases:
   - `forbiddenRegex: [/0705\.4085/]` — tree-wide on the bibliography, so the
     wrong arXiv id cannot return under any entry number
   - `present: ['The Euclidean Algorithm Generates Traditional Musical Rhythms', 'Toussaint']`
   - `rule`: one line recording that the URL served a different paper by eight
     authors in 2007, that both works are real, and that the citation described
     the work it did not point at
2. Run `npm --prefix site test`. Watch it fail: `0705.4085` is still present.
3. Correct the entry in `appendix-references.mdx`. Keep `data-tier="A"` — the
   BRIDGES paper is peer-reviewed proceedings.
4. Re-run. Watch it pass.
5. **Prove it bites**, both arms, each reverted: restore the `0705.4085` URL and
   confirm the forbidden arm fails; remove the title and confirm the present arm
   fails.
6. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M003/S01`, `Rows: FR16`, and **close FR16 in the ledger in the same
   edit that ticks this box** — the two are one action, and splitting them is
   what left FR06 and FR10 open in M002.

## Task 2 — `ref-34` attributes the article to its author, and the slice closes

**Consumes:** task 1's harness pattern.

1. Add `REF34-SCHWARZ` to `CLAIMS`:
   - `forbidden: ['Reich, S. "Music as a Gradual Process, Part II."']` — the
     exact attribution being corrected
   - `present: ['Schwarz', 'Perspectives of New Music']`
   - `rule`: one line recording that the document is an article about Reich's
     process rather than Reich's essay, and that this is the same failure as
     `ref-6` before M001 of the verifiable-references programme corrected it
2. Run the suite. Watch it fail.
3. Correct the entry: Schwarz, K. R. (1981–82), "Steve Reich: Music as a Gradual
   Process Part II", *Perspectives of New Music* 20(1/2), 225–286, with the JSTOR
   stable URL. The chapter 8 sentence citing it reads "Reich framed his own
   discovery…" — read it and confirm whether it still holds with the attribution
   corrected, since a claim sourced to Reich's own words is not the same claim
   when the source is a commentator. Reword only if it no longer holds.
4. Re-run. Watch it pass. **Prove it bites**, both arms, each reverted.
5. Run `format`, `site-unit`, `doc-conformance`. Append the slice's evidence,
   tick every definition-of-done box, close FR17 **in the same edit**, set the
   slice `done`, run `jk-standards ledger`, and commit with `Slice: M003/S01`,
   `Rows: FR17`.
