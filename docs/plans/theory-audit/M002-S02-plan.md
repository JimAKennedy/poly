# M002/S02 — Chapter 3 clave citations

**Slice:** M002/S02
**Ledger:** `docs/plans/theory-audit/ledger.md`

The clave matrix is the most important theoretical claim in Chapter 3, and it
was cited to a YouTube video while Peñalosa (2009) — the definitive treatment,
already in the guide's Further Reading — sat unused. This slice moves the claim
onto its real source.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [x] Task 1 — Chapter 3: re-cite the clave-matrix and gap claims
- [x] Task 2 — Companion page: drop the video citation from the clave claim (F18)

## Definition of Done

Copied verbatim from the slice. Both tasks below argue against *this* text.

- [x] The clave-matrix and non-Euclidean-gap claims cite Peñalosa (2009) inline
- [x] Refs [10] and [11] no longer carry a named-theory claim in Chapter 3

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/citation-tier.test.mjs` in the inner loop. Run all
three tokens before claiming a task done.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. So on the task that closes the
slice, the order is: `site-unit` and `doc-conformance` first, then write the
evidence file, then `format`. (M002/S01's plan had this backwards; its evidence
file records the order actually used.)

## Context

**The finding.** Ledger F18: refs [10] and [11] are YouTube videos carrying the
clave matrix, while `fr-penalosa-2009` sits unused in Further Reading.

**`ref-11` is already cited nowhere.** No `.mdx` under the docs root contains
`#ref-11`. Half of the second Definition-of-Done item is satisfied before this
slice starts. Do not invent work for it, and do not delete the entry — S06 owns
reference tiers.

**`ref-10` is cited four times, and only three are claims:**

| Site | Form | Nature |
|---|---|---|
| `03-afro-cuban.mdx:32` | `<sup>[10](…)</sup>` | clave-matrix claim — Task 1 |
| `03-afro-cuban.mdx:79` | `<sup>[10](…)</sup>` | gap / Euclidean claim — Task 1 |
| `theory-afro-cuban.mdx:13` | `<sup>[10](…)</sup>` | clave-matrix claim — Task 2 |
| `theory-afro-cuban.mdx:73` | `[10](…)–[13](…)` | "See also refs" in Sources — **leave alone** |

The guide uses two citation forms: `<sup>[N](#ref-N)</sup>` for a numbered
reference supporting a claim, and `([Author Year](#fr-…))` for Further Reading.
The Sources line uses neither — it is a plain-link bibliographic pointer, not a
named-theory claim, so it satisfies the Definition of Done as written. Leaving
it also stops `ref-10` becoming an orphan the moment its claims move away.

That form split is what the locks assert against: they forbid the **superscript
form** of `ref-10`, not every mention of it. Use `forbiddenRegex` /
`presentRegex`, which `assertClaim` matches against raw source; the plain
`forbidden` / `present` arms normalise prose and will not reliably see markup.

**Line 79 carries two different claims and needs two different sources.** The
sentence states the son clave's gap sequence (`3-3-4-2-4`), which is Peñalosa's
territory, and then that `E(k,n)` can only generate patterns whose gaps take one
of two consecutive values, which is not — that is Toussaint (2005),
`ref-1`, "The Euclidean Algorithm Generates Traditional Musical Rhythms".
Citing Peñalosa for the Euclidean property would replace a bad citation with a
wrong one, which is the defect class this milestone exists to remove. Cite each
half to the source that supports it.

**Existing tests read `03-afro-cuban.mdx`** — `chapter-euclidean-guardrail`,
`idiom-break-framing`, `prose-pattern-claims`, `prose-conformance-claims`. Line
79 is inside the Euclidean guardrail's subject matter, so Task 1 runs
`site-unit` immediately after that edit rather than at the end.

**The lock host.** `site/tests/citation-tier.test.mjs` exists — M002/S01 created
it. Append to it; do not create a second host, and do not add these cases to
`theory-audit-claims.test.mjs`, which is M001's.

## Task 1 — Chapter 3: re-cite the clave-matrix and gap claims

**Modifies:** `site/src/content/docs/03-afro-cuban.mdx`,
`site/tests/citation-tier.test.mjs`
**Rows:** none — F18 also covers the companion page and closes in Task 2

### Steps

1. **Write the failing case.** In `site/tests/citation-tier.test.mjs`, add to the
   `CLAIMS` array:

   - `id: 'S02-F18'`, `file: '03-afro-cuban.mdx'`
   - `rule`: ledger F18 — the clave matrix is Ch 3's central theoretical claim
     and was cited to a YouTube video while Peñalosa (2009) sat unused in
     Further Reading; the Euclidean-gap property is Toussaint (2005), not
     Peñalosa
   - `forbiddenRegex`: `[/<sup>\[10\]\(\/appendix-references\/#ref-10\)<\/sup>/]`
   - `presentRegex`: `[/#fr-penalosa-2009/, /#ref-1\)/]`

   `#ref-1\)` cannot match `#ref-10)` — the closing paren must follow the `1`.
   Both `presentRegex` arms are unsatisfied in `03-afro-cuban.mdx` today (it
   carries no `#ref-1)` and no `fr-penalosa-2009`), so each is a real lock this
   task establishes rather than a standing guard.

2. **Run it and watch it fail on the forbidden side.**
   `node --test site/tests/citation-tier.test.mjs` must report
   `forbidden pattern reappeared` for `S02-F18`. A `present`-side failure first
   would mean the superscript regex does not match the current markup: fix the
   regex, not the chapter.

3. **Edit line 32**, the clave-matrix claim. Replace

   > `whether or not anyone is playing it aloud.<sup>[10](/appendix-references/#ref-10)</sup>`

   with

   > `whether or not anyone is playing it aloud ([Peñalosa 2009](/appendix-references/#fr-penalosa-2009)).`

   This is the parenthetical form the companion page already uses for Peñalosa.

4. **Edit line 79**, the gap claim, giving each half its own source. Replace

   > `Inter-onset gaps: 3-3-4-2-4.`

   with

   > `Inter-onset gaps: 3-3-4-2-4 ([Peñalosa 2009](/appendix-references/#fr-penalosa-2009)).`

   and replace the trailing

   > `(here, 3s and 4s).<sup>[10](/appendix-references/#ref-10)</sup>`

   with

   > `(here, 3s and 4s).<sup>[1](/appendix-references/#ref-1)</sup>`

   The superscript-after-the-period placement matches `02-sub-saharan-africa.mdx`,
   which cites `ref-1` the same way.

5. **Run the case and watch it pass**, then **run `site-unit` immediately** —
   `npm --prefix site test`. Four other suites read this file and line 79 is in
   the Euclidean guardrail's subject matter. If one goes red, stop and report
   which, rather than editing it: a guardrail objecting to this change is
   information, not an obstacle.

6. **Run the gates**: `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`. Read each exit code.

7. **Append to** `docs/plans/theory-audit/evidence/M002-S02.md`, creating it,
   in the format `/jk:next` section 4 gives. Name no commit SHA.

8. **Commit** the chapter, the test, the plan's ticked box and the evidence as
   one unit, with trailers `Plan: docs/plans/theory-audit/ledger.md` and
   `Slice: M002/S02`. No `Rows:` trailer — this task closes no row.

## Task 2 — Companion page: drop the video citation from the clave claim

**Modifies:** `site/src/content/docs/theory-afro-cuban.mdx`,
`site/tests/citation-tier.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Consumes:** Task 1's `S02-F18` case, as the pattern to copy
**Rows:** F18

`theory-afro-cuban.mdx` is Chapter 3's companion page — "The Clave Matrix:
Afro-Cuban". Its line 13 cites the YouTube video **and** Peñalosa for the same
sentence, which is the least defensible instance of F18 in the tree: the real
source is already there, next to the video.

### Steps

1. **Write the failing case.** Add to `CLAIMS` in
   `site/tests/citation-tier.test.mjs`:

   - `id: 'S02-F18-theory'`, `file: 'theory-afro-cuban.mdx'`
   - `rule`: ledger F18 — the companion page cited the same video for the same
     clave-matrix claim, on a sentence already carrying Peñalosa; the Sources
     "See also" listing is a bibliographic pointer and stays
   - `forbiddenRegex`: `[/<sup>\[10\]\(\/appendix-references\/#ref-10\)<\/sup>/]`
   - `presentRegex`: `[/#fr-penalosa-2009/]`

   Do **not** forbid the plain `[10](/appendix-references/#ref-10)` form. Line
   73's Sources listing uses it and must survive.

   The `presentRegex` arm is already satisfied — the page carries four
   `fr-penalosa-2009` citations today — so it locks nothing this task
   establishes; it is a standing guard against a later edit stripping Peñalosa
   out. The forbidden arm is therefore the only way this case can fail, which
   is what step 2 expects to see.

2. **Run it and watch it fail on the forbidden side**, for the reason above.

3. **Edit line 13.** Replace

   > `functions as a matrix rather than a part <sup>[10](/appendix-references/#ref-10)</sup>.`

   with

   > `functions as a matrix rather than a part.`

   Delete the space before the superscript along with it. The Peñalosa citation
   later in the same sentence already sources the claim, so nothing is added.

4. **Run the case and watch it pass**, then confirm line 73 still cites
   `ref-10`: `grep -n 'See also refs' site/src/content/docs/theory-afro-cuban.mdx`
   must still show the `#ref-10` link. If it does not, the edit took too much.

5. **Prove the lock bites**, as M001's evidence files do: re-insert
   `<sup>[10](/appendix-references/#ref-10)</sup>` into `03-afro-cuban.mdx`,
   confirm `S02-F18` fails while `S02-F18-theory` still passes, then revert and
   confirm with `git diff --quiet`. This shows the two cases are independent
   rather than one guard counted twice.

6. **Run `site-unit` and `doc-conformance`** and read their exit codes.

7. **Close the row and the slice.** In `docs/plans/theory-audit/ledger.md`, set
   F18's `Status` to `done`, and rewrite its `Verification` cell to name cases
   `S02-F18` and `S02-F18-theory` in `site/tests/citation-tier.test.mjs` — the
   cell currently forward-references F23's Tier-A check, which S06 builds and
   which cannot be what proves this slice. Record in the `Item` cell that
   `ref-11` carried no claim to begin with and that the Sources "See also"
   listing was deliberately left. Use no `|` in either cell.

   Then tick both Definition-of-Done boxes in the slice and in this plan's copy,
   tick this plan's Task 2 box, and set the slice `Status` to `done`.

8. **Write the evidence**, appending a second block to
   `docs/plans/theory-audit/evidence/M002-S02.md`. Then run `format`
   (`pre-commit run --all-files`) — last, per the gate ordering above — and
   `jk-standards ledger`.

9. **Commit** as one unit with trailers `Plan: docs/plans/theory-audit/ledger.md`,
   `Slice: M002/S02`, `Rows: F18`.
