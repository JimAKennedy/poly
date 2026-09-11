# M005/S03 — Indian-classical sources

**Slice:** M005/S03, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M005-decisions.md` — every source was verified online before
planning, and six of M005's ten rows were amended because verification
contradicted them.

## Task status

- [x] Task 1 — Add Powers, cite him, and lock Kippen's existing citation

## Definition of Done

- [x] Powers's *New Grove* "India" article is in the appendix with a declared
      tier and cited for Indian art-music theory, its coverage of the Carnatic
      tala system specifically being unconfirmed
- [x] Kippen's existing inline citation in Chapter 6 is confirmed and locked,
      the row's claim that he is in Further Reading only being stale

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

## Task 1 — Add Powers, cite him, and lock Kippen's existing citation

Closes **F48**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/06-indian-classical.mdx`, the test host

Two corrections from verification, both recorded in F48's `Item`:

- **Kippen's half is already done.** `06-indian-classical.mdx` cites
  `fr-kippen-1988` inline — M002/S04 added it, at the theka claim. Nothing to
  write; the work is to lock it so it cannot be dropped.
- **Powers on Carnatic tala is unconfirmed.** The *New Grove* "India,
  subcontinent of" article exists and is his. That it covers the Carnatic tala
  system specifically was not established, so cite it for Indian art-music
  theory generally. Do not attach it to a Carnatic claim.

1. **Write the failing cases**: `fr-powers-1980` exists with a `data-tier` and
   is cited from `06-indian-classical.mdx`; and a second case asserting
   `fr-kippen-1988` is still cited inline there. The Kippen case will pass on
   creation — it guards work that already landed — so its non-vacuity comes from
   the deletion test at step 5, exactly as `S04-F32` did in M003.
2. **Run them**: Powers fails, Kippen passes.
3. **Add the entry** to `### Indian Classical (Chapter 6)`, tier A: Powers, H.
   (1980). "India, subcontinent of". *The New Grove Dictionary of Music and
   Musicians*, vol. 9, ed. Stanley Sadie. Macmillan.
4. **Cite it** where the chapter frames Indian art-music theory.
5. **Prove both bite.** Remove the Powers citation, watch its case fail; restore.
   Remove Kippen's existing citation, watch its case fail; restore. Both by
   inverse edit, confirmed with `git diff --stat`.
6. **Check.** All three tokens.
7. **Commit** with `Rows: F48`, ticking Task 1, closing F48, appending the
   evidence, and setting the slice `done`.

## Self-review

**DoD coverage.** Item 1 (Powers in the appendix, cited for Indian art-music
theory rather than Carnatic tala) → Task 1 steps 3 and 4. Item 2 (Kippen's
existing citation confirmed and locked) → Task 1 steps 1 and 5.

**Row coverage.** F48 → Task 1 step 7.

**Placeholder scan.** No TBDs. The plan states outright what Powers may not be
cited for.

**Name consistency.** `fr-powers-1980`, `fr-kippen-1988` throughout.
