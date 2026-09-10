# M005/S01 — Sub-Saharan sources

**Slice:** M005/S01, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Add Charry, Kubik and Agawu (2003) to the appendix
- [ ] Task 2 — Cite each at the claim it supports, and cite Arom's 1991

## Definition of Done

- [ ] Charry, Kubik and Agawu (2003) are in the reference appendix with
      declared tiers
- [ ] Each is cited at the claim it supports, not only listed
- [ ] The existing `fr-arom-1991` entry is cited from the sub-Saharan
      methodology material, no later Arom work having been found

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## How an entry is written

Every new appendix entry follows the file's existing shape, in the Further
Reading subsection for its chapter:

```mdx
- <span id="fr-author-year" data-tier="A">Author, A. (Year). *Title*. Publisher. — What it is cited for.</span>
```

**Every one of these works was verified bibliographically — author, exact title,
year, publisher — and its subject matter from publishers' and reviewers'
descriptions. None was read.** That distinction is recorded in
`M005-decisions.md` with what each search established, and it is why the
annotation says what the work is cited *for* rather than paraphrasing an
argument nobody here has followed.

Where even the description is out of reach, the entry says so in the phrase
**`contents unverified`**, which is fixed so `grep -rn "contents unverified"
site/` enumerates every such entry. That applies to Peycheva & Dimov, published
in Bulgarian.

Re-derive the bibliographic details from `M005-decisions.md` rather than from
memory; do not invent a page range, an edition, or a subtitle that file does not
carry.

## Task 1 — Add Charry, Kubik and Agawu (2003) to the appendix

Produces the three anchors Task 2 cites. Closes no row on its own: a listed
entry nobody cites is exactly what F44, F45 and F46 complain about.

**Files:** `site/src/content/docs/appendix-references.mdx`

1. **Write the failing case** in `site/tests/citation-tier.test.mjs`'s sibling
   host or a new `site/tests/literature-enrichment.test.mjs` — check which host
   `scripts/check-doc-conformance.sh` and `doc-conformance-wiring.test.mjs`'s
   `REQUIRED` array already name, and follow that wiring, because a lock outside
   the runner never runs on CI (poly issue #272). The case asserts the three
   anchors `fr-charry-2000`, `fr-kubik-1999` and `fr-agawu-2003` exist, each
   carrying a `data-tier`.
2. **Run it and watch it fail**, naming all three as missing.
3. **Add the entries** to `### Sub-Saharan Africa (Chapter 2)`, tier A:
   - Charry, E. (2000). *Mande Music: Traditional and Modern Music of the
     Maninka and Mandinka of Western Africa*. University of Chicago Press.
   - Kubik, G. (1999). *Africa and the Blues*. University Press of Mississippi.
   - Agawu, K. (2003). *Representing African Music: Postcolonial Notes, Queries,
     Positions*. Routledge.

   Agawu (2003) sits beside the existing `fr-agawu-2006`, which F46 says is a
   different argument. Both stay; the annotations must make the difference
   legible, or a later reader will think one is a typo for the other.
4. **Run and watch it pass.**
5. **Prove it bites.** Remove one entry, watch the case name that anchor,
   restore by inverse edit, confirm with `git diff --stat`.
6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.
7. **Commit** with `Rows: —`, ticking Task 1, appending to
   `docs/plans/theory-audit/evidence/M005-S01.md`.

## Task 2 — Cite each at the claim it supports, and cite Arom's 1991

Closes **F44**, **F45**, **F46** and **F52**.

**Files:** `site/src/content/docs/02-sub-saharan-africa.mdx`,
`site/src/content/docs/08-minimalism.mdx`,
`site/src/content/docs/theory-sub-saharan-africa.mdx`, the test host

1. **Write the failing cases**, one per row, each asserting the citation appears
   at the right place rather than anywhere on the page — bind each to a phrase
   from its target passage, the way `S05-F35` binds Tenzer to Chapter 5's
   opening sentence:
   - **F44** — `fr-charry-2000` cited inside the "A simplification" note at
     `02-sub-saharan-africa.mdx` line ~84, which is M003/S03's F26 disclosure.
     Charry is what corrects the same-cycle oversimplification, so it belongs in
     the note that admits it.
   - **F45** — `fr-kubik-1999` cited in `02-sub-saharan-africa.mdx` where the
     chapter reaches toward the African retentions that bridge it to Chapter 4.
   - **F46** — `fr-agawu-2003` cited in `08-minimalism.mdx` at the F11 reframe,
     the passage about the African–minimalist connection.
   - **F52** — `fr-arom-1991` cited from `theory-sub-saharan-africa.mdx`'s
     methodology material.
2. **Run them and watch all four fail.**
3. **Write the citations**, in each page's existing inline form —
   `([Author Year](/appendix-references/#fr-anchor))`. Say what the source
   contributes; do not paraphrase an argument from a book nobody here has read.
4. **Run and watch them pass.**
5. **Prove each bites**, one at a time, by removing that citation and watching
   only its own case fail. Restore by inverse edit.
6. **Check.** All three tokens.
7. **Commit** with `Rows: F44, F45, F46, F52`, ticking Task 2, closing all four
   rows with their case ids in `Verification`, and appending the evidence. F52's
   `Item` already records that no later Arom work was found — do not restate it
   in the commit as though discovered now.

## Self-review

**DoD coverage.** Item 1 (Charry, Kubik, Agawu in the appendix with tiers) →
Task 1. Item 2 (each cited at its claim) → Task 2. Item 3 (`fr-arom-1991` cited
from the methodology material) → Task 2's F52 case.

**Row coverage.** F44, F45, F46, F52 → Task 2 step 7, each with its own case
proved to bite independently at step 5.

**Placeholder scan.** No TBDs. Every bibliographic detail is in
`M005-decisions.md`; the plan names no page number or edition that file does not
carry.

**Name consistency.** `fr-charry-2000`, `fr-kubik-1999`, `fr-agawu-2003`,
`fr-arom-1991` throughout.

**Ordering.** Task 1 must precede Task 2: a citation to an anchor that does not
exist fails `research_provenance`, so the entries land first.
