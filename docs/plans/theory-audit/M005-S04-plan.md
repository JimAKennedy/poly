# M005/S04 — Balkan sources

**Slice:** M005/S04, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Add Silverman and cite the svatbarska-muzika claim
- [x] Task 2 — Add Peycheva & Dimov, stating what is unverified

## Definition of Done

- [x] Silverman (2007) is in the appendix with a declared tier and cited at the
      svatbarska-muzika discussion
- [x] Peycheva & Dimov are in the appendix with a declared tier, cited where the
      chapter touches Romani musicianship, and their entry states plainly that
      the work's contents are unverified pending translation

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

## Task 1 — Add Silverman and cite the svatbarska-muzika claim

Produces the entry that carries F49's main claim.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/07-balkan.mdx`, the test host

`07-balkan.mdx` line ~96 says the wedding-music tradition pushes aksak metres to
extraordinary tempos, citing Rice (1994) — M002/S05's work. Silverman is the
source for the tradition's own history and politics, which Rice is not being
cited for.

1. **Write the failing case**: `fr-silverman-2007` exists with a `data-tier` and
   is cited from the svatbarska-muzika passage, bound to a phrase from it.
2. **Run it and watch it fail.**
3. **Add the entry** to `### Balkan (Chapter 7)`, tier A: Silverman, C. (2007).
   "Bulgarian Wedding Music between Folk and Chalga: Politics, Markets and
   Current Directions". *Muzikologija*.
4. **Cite it** at that passage, alongside the existing Rice citation rather than
   replacing it — they support different claims.
5. **Run, watch it pass, prove it bites**, restore by inverse edit.
6. **Check.** All three tokens.
7. **Commit** with `Rows: —`, ticking Task 1, appending to
   `docs/plans/theory-audit/evidence/M005-S04.md`. F49 closes in Task 2, when
   both its entries exist.

## Task 2 — Add Peycheva & Dimov, stating what is unverified

Closes **F49**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/07-balkan.mdx`, the test host

The audit named these scholars for wedding music. Their verifiable work is *The
Zurna Tradition in Southwest Bulgaria: Romani Musicians in Practice* (2002),
published in Bulgarian in the Bŭlgarsko muzikoznanie series — a different
subject, and one whose contents cannot be read here.

**The entry must say so.** Not a downgraded tier standing in for a caveat
nobody wrote: the annotation states what was confirmed — that the work exists,
its authors, year, title and series — and what was not, using the fixed phrase
**`contents unverified`** so the set is greppable, and naming the reason as a
translation being sought.

1. **Write the failing cases**: `fr-peycheva-dimov-2002` exists with a
   `data-tier`; its entry contains `contents unverified`; and it is cited from
   `07-balkan.mdx` where the chapter touches Romani musicianship.
2. **Run them and watch all three fail.**
3. **Add the entry** to `### Balkan (Chapter 7)`, **tier B**: Peycheva, L. &
   Dimov, V. (2002). *The Zurna Tradition in Southwest Bulgaria: Romani
   Musicians in Practice*. Bŭlgarsko muzikoznanie. Annotate it with what is
   confirmed, the `contents unverified` phrase, and the translation being
   sought.
4. **Cite it** where the chapter touches Romani musicianship in Bulgarian
   wedding bands. If the chapter has no such passage, **stop and report** rather
   than writing one to hang a citation on — inventing a claim to justify a
   reference is the failure this whole milestone is guarding against.
5. **Run, watch them pass, prove each bites**, restore by inverse edit.
6. **Check.** All three tokens.
7. **Commit** with `Rows: F49`, ticking Task 2, closing F49, appending the
   evidence, and setting the slice `done`.

## Self-review

**DoD coverage.** Item 1 (Silverman in the appendix, cited at the
svatbarska-muzika discussion) → Task 1. Item 2 (Peycheva & Dimov in the appendix
with a tier, cited at Romani musicianship, contents stated unverified) → Task 2.

**Row coverage.** F49 → Task 2 step 7; both its entries exist by then, and each
case is proved to bite independently.

**Placeholder scan.** No TBDs. Task 2 step 4 names the halt condition explicitly
rather than leaving the executor to improvise a passage.

**Name consistency.** `fr-silverman-2007`, `fr-peycheva-dimov-2002`, and the
phrase `contents unverified`, throughout.

**Ordering.** Task 1 first because Silverman carries the row's main claim; if
Task 2 halts at step 4, the slice still has the citation the chapter most needs.
