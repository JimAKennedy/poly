# M005/S05 — Minimalism and electronic sources

**Slice:** M005/S05, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Cite Scherzinger in Chapter 8 itself
- [x] Task 2 — Add Born & Hesmondhalgh and cite them in Chapter 14

## Definition of Done

- [x] Scherzinger's critical account is cited in Chapter 8 itself, not only in
      the theory page and the appendix
- [x] Born & Hesmondhalgh are in the appendix with a declared tier and cited
      where the guide argues cross-cultural combination

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `gate` | not owed by this slice |

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

## Task 1 — Cite Scherzinger in Chapter 8 itself

Closes **F50**. **Adds no appendix entry:** `fr-scherzinger-2010` already
exists and is already cited from `theory-minimalism.mdx`. What is missing is a
citation from the chapter that makes the claim.

**Files:** `site/src/content/docs/08-minimalism.mdx`, the test host

1. **Write the failing case**: `fr-scherzinger-2010` cited from
   `08-minimalism.mdx` at the African–minimalist passage, bound to a phrase from
   it. M003/S05's F46 work puts Agawu (2003) at the same reframe, so check
   whether that citation already sits there and place Scherzinger beside it
   rather than replacing it — they are different arguments.
2. **Run it and watch it fail.**
3. **Cite it**, saying what Scherzinger contributes: a more critical account of
   the African–minimalist connection than Reich's own.
4. **Run, watch it pass, prove it bites**, restore by inverse edit.
5. **Check.** All three tokens.
6. **Commit** with `Rows: F50`, ticking Task 1, closing F50, appending to
   `docs/plans/theory-audit/evidence/M005-S05.md`.

## Task 2 — Add Born & Hesmondhalgh and cite them in Chapter 14

Closes **F51**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/14-synthesis.mdx`, the test host

**They are the volume's editors, not its authors.** The entry says `eds.`; F51's
`Item` records why.

1. **Write the failing case**: `fr-born-hesmondhalgh-2000` exists with a
   `data-tier` and is cited from `14-synthesis.mdx` where the guide argues about
   cross-cultural combination.
2. **Run it and watch it fail.**
3. **Add the entry** to `### Chapter 14: Synthesis` or its Further Reading
   equivalent — check which the appendix uses for that chapter — tier A: Born,
   G. & Hesmondhalgh, D., eds. (2000). *Western Music and Its Others:
   Difference, Representation, and Appropriation in Music*. University of
   California Press.
4. **Cite it** where Chapter 14 argues that cross-cultural combination works or
   does not. The volume is about musical borrowing and the representation of
   difference, which is that argument's subject; cite it for framing, not for a
   specific finding nobody here has read.
5. **Run, watch it pass, prove it bites**, restore by inverse edit.
6. **Check.** All three tokens.
7. **Commit** with `Rows: F51`, ticking Task 2, closing F51, appending the
   evidence, and setting the slice `done`.

## Self-review

**DoD coverage.** Item 1 (Scherzinger cited in Chapter 8 itself) → Task 1. Item
2 (Born & Hesmondhalgh in the appendix, cited at the cross-cultural argument) →
Task 2.

**Row coverage.** F50 → Task 1 step 6, F51 → Task 2 step 7, each with its own
case proved to bite.

**Placeholder scan.** No TBDs. Task 2 step 3 tells the executor to check which
appendix section Chapter 14 uses rather than assuming.

**Name consistency.** `fr-scherzinger-2010`, `fr-born-hesmondhalgh-2000`
throughout.
