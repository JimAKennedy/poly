# M002/S01 — Reference [2] resolution

**Slice:** M002/S01
**Ledger:** `docs/plans/theory-audit/ledger.md`

The audit flagged `ref-2` as a possible fabrication and gave a reason: MTO had
not reached Volume 31, so the URL had to be a forward reference. Checked against
the publisher on 2026-09-01, that reason does not hold — but the suspicion was
right for a different reason, and a worse one. The venue, volume, year, author
and URL are real and correctly paired; the **title** attached to them is not.

This slice records what the reference actually is and corrects it in place.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; the task's own commit ticks it.

- [x] Task 1 — Resolve `ref-2` against the publisher, correct it, and lock it (F17)

## Definition of Done

Copied verbatim from the slice. The task below argues against *this* text.

- [x] Reference [2]'s publication status is established from the publisher, and
      the finding records which it was
- [x] The appendix entry states the resolved status: real, replaced, or marked
      forthcoming

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`site-unit` runs `node --test tests/**/*.test.mjs` from `site/`, so a new file
at `site/tests/citation-tier.test.mjs` is picked up with no wiring. Run
`node --test site/tests/citation-tier.test.mjs` in the inner loop; run all three
tokens before claiming the task done.

`doc-conformance` is what runs `docs/audits/theory-audit-remediation.test.mjs`,
the guard that every F-row appears exactly once with valid tokens. It parses the
`Sev`, `Disp` and `Status` columns only — Item prose is free text — so
rewriting F17's Item cell is safe provided the cell contains no `|`.

## Context

**What was verified, and how.** Re-verify rather than trust this section; the
task's first step is to repeat these three checks.

1. `https://mtosmt.org/issues/issues.php` — MTO's issue index lists Volume 32
   Number 2 (June 2026) as current. Volume 31 ran through 2025. The audit's
   premise that MTO "is current through Vol 30 as of mid-2026" is wrong.
2. `https://mtosmt.org/issues/mto.25.31.2/mto.25.31.2.goldberg.pdf` — resolves
   to a real PDF, roughly 1.2 MB. The link in the guide is not broken.
3. `https://mtosmt.org/issues/mto.25.31.2/toc.31.2.html` — the issue's table of
   contents lists seven articles. Goldberg's is
   *"Music Theory as an Instrument of Nationalism: Notation, Identity, and
   Systemization in Dobri Hristov's Conception of Bulgarian Meter"*
   (Daniel Goldberg, University of Connecticut). No article in that issue, or
   anywhere a web search reaches, carries the title the guide prints.

**The entry today**, `site/src/content/docs/appendix-references.mdx` under the
`## Chapter 1: Foundations` heading:

> `<span id="ref-2">**[2]**</span> Goldberg, D. (2025). "Resultant Patterns in Phase-Shifted Rhythmic Structures." *Music Theory Online*, 31(2). [MTO](https://mtosmt.org/issues/mto.25.31.2/mto.25.31.2.goldberg.pdf)`

**`ref-2` is cited nowhere.** No `.mdx` under `site/src/content/docs` contains
`#ref-2`. It is an orphan entry, so correcting it changes no chapter prose and
breaks no inline citation. It stays an orphan after this slice — wiring the real
article into Chapter 7 belongs to M002/S05, which owns Balkan citations and
depends on M001/S04. Do not add an inline citation here.

**Why a new test file.** The slice's Verification column names
`citation-tier.test.mjs`, and M002/S06 (F23) extends that same file with the
tier machinery and wires it into `scripts/check-doc-conformance.sh`. Creating it
here with one case is what S06 expects to find. Do not add this case to
`theory-audit-claims.test.mjs`: that host's header declares itself the M001
remediation host, and its `FINDINGS` array is M001's.

**Claim helpers.** `site/tests/helpers/prose-claims.mjs` exports
`assertClaim(assert, claim, source)` and
`registerClaimTests({ test, assert, claims, loadSource, label })`. A claim is
`{ id, file, rule, forbidden, forbiddenRegex, present, presentRegex }`;
`forbidden`/`present` are matched under prose normalization, the `*Regex`
variants against raw source. See `site/tests/theory-audit-claims.test.mjs` for a
worked host.

## Task 1 — Resolve `ref-2` against the publisher, correct it, and lock it

**Creates:** `site/tests/citation-tier.test.mjs`,
`docs/plans/theory-audit/evidence/M002-S01.md`
**Modifies:** `site/src/content/docs/appendix-references.mdx`,
`docs/plans/theory-audit/ledger.md`
**Rows:** F17

### Steps

1. **Re-verify the three checks in Context.** Fetch the issue index, the PDF
   URL, and the issue table of contents. If any disagrees with what Context
   records — in particular if the TOC now shows an article matching the guide's
   printed title — **stop and report**. The correction below is only right
   while the finding holds.

2. **Write the failing test.** Create `site/tests/citation-tier.test.mjs`. Give
   it a header comment stating that it hosts citation-integrity cases for M002,
   that S01 creates it with the F17 case, and that S06 extends it with the
   reference-tier assertions and wires it into `check-doc-conformance.sh`.

   Import `test` from `node:test`, `assert` from `node:assert/strict`,
   `readFile` and `readdir` from `node:fs/promises`, `dirname`/`join` from
   `node:path`, `fileURLToPath` from `node:url`, and `registerClaimTests` from
   `./helpers/prose-claims.mjs`. Resolve `DOCS` as
   `join(dirname(fileURLToPath(import.meta.url)), '..', 'src', 'content', 'docs')`
   and define `loadSource = (file) => readFile(join(DOCS, file), 'utf8')`.

   Define a `CLAIMS` array holding one claim, and register it with
   `registerClaimTests({ test, assert, claims: CLAIMS, loadSource })`:

   - `id: 'S01-F17'`, `file: 'appendix-references.mdx'`
   - `rule`: audit §5 item on ref [2]; MTO 31(2) verified against the
     publisher's table of contents on 2026-09-01 — real article, fabricated
     title
   - `forbidden`: `['Resultant Patterns in Phase-Shifted Rhythmic Structures']`
   - `present`: `['Music Theory as an Instrument of Nationalism', 'Dobri Hristov']`
   - `presentRegex`: `[/mto\.25\.31\.2\.goldberg\.pdf/]`

   Then add a second case as a plain `node:test` — **not** a `CLAIMS` entry,
   because it spans the tree rather than one file. Name it `S01-F17-tree`, have
   it `readdir(DOCS)`, read every entry ending `.mdx`, and assert the string
   `Resultant Patterns in Phase-Shifted Rhythmic Structures` appears in none of
   them; on failure, name the offending file. This is the regression lock that
   matters: the per-file case guards the appendix, this one stops the title
   reappearing anywhere else.

3. **Run it and watch it fail for the right reason.**
   `node --test site/tests/citation-tier.test.mjs`. Both cases must fail on the
   *forbidden* side — the fabricated title is present in the appendix today. A
   failure on `present` instead would mean the corrective phrases are already
   there, which they are not; if you see that, the claim is mis-written, so fix
   the claim rather than the appendix.

4. **Correct the entry.** In `appendix-references.mdx`, replace the `ref-2`
   line with:

   > `<span id="ref-2">**[2]**</span> Goldberg, D. (2025). "Music Theory as an Instrument of Nationalism: Notation, Identity, and Systemization in Dobri Hristov's Conception of Bulgarian Meter." *Music Theory Online*, 31(2). [MTO](https://mtosmt.org/issues/mto.25.31.2/mto.25.31.2.goldberg.pdf)`

   Keep the straight double quotes, the `*Music Theory Online*` emphasis and
   the `[MTO](…)` link text, matching the surrounding entries. The `id`, the
   number, the author, the year, the volume and the URL are all unchanged —
   only the title moves.

   This is what satisfies the second Definition-of-Done item. Of its three
   offered outcomes — real, replaced, or marked forthcoming — the resolution is
   **real**: the cited work exists and is now named correctly. Nothing is
   marked forthcoming, because nothing is awaiting publication.

5. **Run it and watch both cases pass.**

6. **Record the resolution in F17.** In `docs/plans/theory-audit/ledger.md`,
   rewrite the F17 row's `Item` cell so it states what was found rather than
   what was suspected: that the venue, volume, year, author and URL are real
   and correctly paired; that the printed title is fabricated and MTO 31(2)
   instead carries Goldberg's Hristov/Bulgarian-meter article; that the audit's
   stated reason — a forward reference past Vol 30 — does not hold because MTO
   reached Vol 32 No 2 in June 2026; and that it was resolved on 2026-09-01 by
   correcting the title in place. Use no `|` in the cell. Update the row's
   `Verification` cell to name case `S01-F17`. Set the row `Status` to `done`.

   Then tick both Definition of Done boxes in the slice and in this plan's
   copy, tick this plan's Task status box, and set the slice `Status` to `done`.

7. **Write the evidence file**,
   `docs/plans/theory-audit/evidence/M002-S01.md`, in the format `/jk:next`
   section 4 gives. Name no commit SHA.

8. **Run the gates**, in this order, and read each exit code:
   `node --test site/tests/citation-tier.test.mjs`, then `pre-commit run --all-files`,
   `npm --prefix site test`, `bash scripts/check-doc-conformance.sh`, and
   `jk-standards ledger`.

9. **Commit** the test, the appendix, the ledger, the plan and the evidence as
   one unit, with trailers `Plan: docs/plans/theory-audit/ledger.md`,
   `Slice: M002/S01`, `Rows: F17`.
