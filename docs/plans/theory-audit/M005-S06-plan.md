# M005/S06 — Brazilian maracatu

**Slice:** M005/S06, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Name the maracatu ensemble's parts and cite Crook
- [ ] Task 2 — Close the slice and run the shipping gate

## Definition of Done

- [ ] The maracatu section describes the ensemble's named parts and their
      rhythmic relationship, not only its density and weight
- [ ] Its claims carry citations that resolve

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `gate` | `bash scripts/pre-push-check.sh` |

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

## Task 1 — Name the maracatu ensemble's parts and cite Crook

Closes **F53**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/10-brazilian.mdx`, the test host

`10-brazilian.mdx` line ~82, "Maracatu: Weight and Repetition", covers density
and dynamics accurately but names no ensemble parts and carries no citation.

The scope was fixed during planning, because "the treatment is thin" is not one.
The section names the baque virado ensemble's parts — **caixa, alfaia, mineiro,
agbê and gonguê** — and describes their rhythmic relationship, citing Crook
(2009), whose Afro-Brazilian traditions chapter covers maracatu directly.

Write what the sources support and no more. The instrument names and their roles
are what verification established; a detailed account of each part's pattern is
not, and inventing one would be the failure this milestone exists to avoid.

1. **Write the failing cases**: `fr-crook-2009` exists with a `data-tier` and is
   cited from the maracatu section; and a prose case asserting the section names
   all five parts. Assert the names as a set read from the case, so a later edit
   dropping one fails by name.
2. **Run them and watch both fail.**
3. **Add the entry** to `### Brazilian (Chapter 10)`, tier A: Crook, L. (2009).
   *Focus: Music of Northeast Brazil*, 2nd ed. Routledge.
4. **Write the section's addition** in the chapter's voice: the parts, what each
   does, and how they relate — the alfaia carrying the weight the section already
   describes, the gonguê a bell, the caixa introducing the ensemble. Cite Crook.
5. **Run, watch both pass, prove each bites** — remove the citation, then remove
   one instrument name — restoring by inverse edit each time.
6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.
7. **Commit** with `Rows: F53`, ticking Task 1, closing F53, appending to
   `docs/plans/theory-audit/evidence/M005-S06.md`.

## Task 2 — Close the slice and run the shipping gate

Closes no row; closes the slice and the milestone's execution.

1. **Confirm every M005 row is `done`** — F44 through F53 — and that
   `grep -rn "contents unverified" site/` lists exactly the Peycheva & Dimov
   entry.
2. **Tick both definition-of-done boxes** in the slice and this plan, tick Task
   2, set the slice `Status` to `done`.
3. **Run every token**: `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, then
   `bash scripts/pre-push-check.sh` for `gate`. Read exit codes.
4. **Run `jk-standards ledger`** with the slice `done`.
5. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (the section names the ensemble's parts and their
relationship) → Task 1 step 4. Item 2 (its claims carry resolving citations) →
Task 1 steps 1 and 3.

**Row coverage.** F53 → Task 1 step 7.

**Placeholder scan.** No TBDs. Step 4 states the limit — the parts and roles are
verified, per-part patterns are not — rather than leaving the executor to fill
the gap.

**Name consistency.** `fr-crook-2009` and the five part names — caixa, alfaia,
mineiro, agbê, gonguê — throughout.

**Ordering.** Task 2 last: it runs `gate` over the finished tree, and this is the
last slice of M005.
