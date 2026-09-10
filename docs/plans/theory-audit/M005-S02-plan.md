# M005/S02 — Afro-Cuban sources

**Slice:** M005/S02, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Add Acosta and cite him at the clave-evolution discussion

## Definition of Done

- [x] Acosta is in the reference appendix with a declared tier and cited at the
      clave-evolution discussion

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

## Task 1 — Add Acosta and cite him at the clave-evolution discussion

Closes **F47**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/03-afro-cuban.mdx`, the test host

**The year is 2003, not the 2004 the audit gives.** F47's `Item` records the
correction; take the details from `M005-decisions.md`.

1. **Write the failing case**: `fr-acosta-2003` exists with a `data-tier`, and
   is cited from `03-afro-cuban.mdx` at the clave-evolution material — bound to
   a phrase from that passage rather than to the page as a whole.
2. **Run it and watch it fail** on the missing anchor.
3. **Add the entry** to `### Afro-Cuban (Chapter 3)`, tier A: Acosta, L. (2003).
   *Cubano Be, Cubano Bop: One Hundred Years of Jazz in Cuba*. Smithsonian
   Books.
4. **Cite it** where the chapter discusses how clave practice developed. The
   book also covers the "Spanish tinge" in early New Orleans jazz, which M001/S05
   attributed to Lomax — if the chapter has such a passage, that is the more
   precise home, but do not manufacture one.
5. **Run and watch it pass**, then prove it bites by removing the citation.
   Restore by inverse edit.
6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.
7. **Commit** with `Rows: F47`, ticking Task 1, closing F47, appending to
   `docs/plans/theory-audit/evidence/M005-S02.md`, and setting the slice `done`
   — one task, one row, so the slice closes here.

## Self-review

**DoD coverage.** The single item (Acosta in the appendix with a tier, cited at
the clave-evolution discussion) → Task 1 steps 3 and 4.

**Row coverage.** F47 → Task 1 step 7; its case is written at step 1 and proved
to bite at step 5.

**Placeholder scan.** No TBDs. Step 4 explicitly forbids manufacturing a passage
to cite into.

**Name consistency.** `fr-acosta-2003` throughout — note the year, which differs
from the audit's.
