# M002/S06 — Reference tiers and the tier check

**Slice:** M002/S06
**Ledger:** `docs/plans/theory-audit/ledger.md`
**Design:** `M002-S06-design.md` — read it first; it settles the citation
grammar, the tier format, the escape hatch, and why three alternatives were
rejected.

The milestone's last slice, and the only one that builds a mechanism rather
than editing prose.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [x] Task 1 — Tier all 99 entries and assert every entry declares one
- [x] Task 2 — Enforce Tier A on claim citations, with the reasoned hatch
- [x] Task 3 — The Lomax attribution (F22)
- [x] Task 4 — Wire into the runner and REQUIRED, run `gate`, close the slice (F23)

## Definition of Done

Copied verbatim from the slice. Every task below argues against *this* text.

- [x] Every entry in the reference appendix carries a declared tier
- [x] The "Spanish tinge" attribution cites Lomax's Morton interviews
- [x] A new check fails when a Tier-B or Tier-C source is the inline citation
      for a named-theory claim
- [x] The check is wired into `scripts/check-doc-conformance.sh` **and**
      added to the `REQUIRED` set in
      `site/tests/doc-conformance-wiring.test.mjs`. That file's own header
      states the contract — adding a guardrail means adding it to both — and
      the runner alone leaves the new check undefended against a later edit
      quietly dropping it

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `jk-standards all` |
| `gate` | `bash scripts/pre-push-check.sh` |

`gate` is the full pre-push suite — format, RT safety, snippet regions, build,
tests and pluginval. No slice in this programme has owed it before. It belongs
to Task 4 only; Tasks 1–3 owe the first four tokens.

## Context

**Read the design doc.** It is the argument; this is the procedure. In
particular it settles that a `<sup>` is a claim citation and a plain link is
not, which is what makes the rule mechanisable.

**Always use the generalised superscript pattern**
`/<sup>[^<]*#ref-\d+[^<]*<\/sup>/`, never the single-reference form M002/S02
and S03 used. M002/S04 proved directly that the narrow form cannot match a
two-reference block.

**The lock host** is `site/tests/citation-tier.test.mjs`, which already carries
the S01–S05 claim cases. Add to it.

**M006 exists to receive what this slice defers.** Rows B01–B07 in
`docs/plans/theory-audit/ledger.md` own the low-tier citations Task 2
suppresses, one row per source. Suppression reasons name their B-row by ID, so
the marker points at the work rather than at a vague "later".

**Reverting a test mutation** while a file holds this task's uncommitted work:
use an inverse edit, not `git checkout --`. M002/S03 lost three edits that way.

## Task 1 — Tier all 99 entries and assert every entry declares one

**Modifies:** `site/src/content/docs/appendix-references.mdx`,
`site/tests/citation-tier.test.mjs`
**Produces:** the `data-tier` attribute every later task reads
**Rows:** none — F23 closes in Task 4

### Steps

1. **Write the failing assertion.** In `site/tests/citation-tier.test.mjs`, add
   a case `S06-tiers-declared` that reads `appendix-references.mdx`, finds every
   `<span id="ref-…">` and `<span id="fr-…">`, and asserts each carries
   `data-tier="A"`, `"B"` or `"C"`. On failure, name the offending anchor ids —
   a bare count is useless across 99 entries.

2. **Run it and watch it fail**, naming all 99 anchors, since none is tiered yet.

3. **Tier every entry.** Add `data-tier` to each span:

   ```html
   <span id="ref-1" data-tier="A">**[1]**</span> Toussaint, G. T. (2005). …
   - <span id="fr-clayton-2000" data-tier="A">Clayton, M. (2000). …</span>
   ```

   Apply the audit's definitions (`docs/audits/poly_theory_audit.md` §4):
   **A** peer-reviewed scholarship *or a primary source*; **B** secondary or
   educational but legitimate; **C** hobbyist media. The audit names about half
   the entries explicitly — use its classification where it gives one.

   Two judgement notes that matter, because they change Task 2's work:

   - **A primary source is Tier A even when the venue is informal.** `ref-34`
     is Reich's own "Music as a Gradual Process" and `ref-46` is Bjorklund's SNS
     technical note; both are Tier A. `ref-36` is an *interview with Roger Linn*
     hosted on a blog — as the authority for what Linn himself says about
     swing, that is a primary source, and it may well tier as A. Decide it by
     reading the page, not by matching the design doc's expected count.
   - **The design doc predicts seven suppressions in Task 2. That is a
     prediction, not a target.** If `ref-36` tiers as A the count is six and
     M006's B06 needs revisiting; if another entry tiers lower than expected it
     is eight. Record the actual set. Never force a tier to make a predicted
     count come out.

4. **Run the case and watch it pass.**

5. **Run** `format`, `site-unit`, `doc-conformance` and `doc-discipline`, and
   read each exit code.

6. **Append to** `docs/plans/theory-audit/evidence/M002-S06.md`, creating it.
   Record the tier distribution — how many A, B, C — and any entry whose tier
   was a judgement call rather than the audit's. Name no commit SHA.

7. **Commit** with trailers `Plan: docs/plans/theory-audit/ledger.md` and
   `Slice: M002/S06`. No `Rows:` trailer.

## Task 2 — Enforce Tier A on claim citations, with the reasoned hatch

**Modifies:** `site/tests/citation-tier.test.mjs`, and each `.mdx` needing a
suppression marker
**Consumes:** Task 1's `data-tier` attributes
**Rows:** none — F23 closes in Task 4

### Steps

1. **Write the failing assertion.** Add a case `S06-claims-are-tier-a` that,
   for every `.mdx` under `site/src/content/docs`, finds every superscript block
   matching `/<sup>[^<]*#ref-\d+[^<]*<\/sup>/`, resolves each `#ref-N` it
   contains against Task 1's tier map, and fails unless that entry's tier is
   exactly `A`.

   Implement the hatch in the same case: a citation is exempt when the citing
   line, or the line immediately above it, contains the literal token
   `citation-tier-ok`. No file-level or global form. Count the live suppressions
   and print the count in the assertion's output, so a rising number is visible.

   Make the failure message teach the hatch — it is the documentation people
   actually read: name the file, the line, the ref, its tier, and say
   *"cite a Tier-A source, or add `{/* citation-tier-ok: <reason> */}` on the
   line above"*.

2. **Run it and watch it fail**, listing every non-Tier-A claim citation. The
   expected set is `ref-3` (`01-foundations.mdx`), `ref-13`
   (`03-afro-cuban.mdx`), `ref-20` (`05-gamelan.mdx`), `ref-32` and `ref-33`
   (`08-minimalism.mdx`), `ref-36` (`09-electronic.mdx` and
   `theory-electronic-breakbeat.mdx`), and `ref-38` (`13-drum-and-bass.mdx`) —
   but take the *actual* list from the failure, per Task 1 step 3.

3. **Add one suppression per citation**, as an MDX comment on the line above,
   using the repo's `{/* … */}` comment form:

   ```mdx
   {/* citation-tier-ok: <why this source is acceptable here, or what should
       replace it and which row owns that> — M006/B0N */}
   ```

   Write each reason by **reading the claim it sits on**. Where the honest
   answer is that the claim should cite scholarship and does not, say exactly
   that and name the B-row — do not invent a justification for Wikipedia. A
   marker whose reason would not satisfy a reviewer is a finding in itself.

   The B-rows are: B01 `ref-3`, B02 `ref-13`, B03 `ref-20`, B04 `ref-32`,
   B05 `ref-33`, B06 `ref-36` (both sites), B07 `ref-38`.

4. **Run the case and watch it pass**, and read the printed suppression count.
   It must equal the number of markers added — if it is lower, a marker is
   mis-placed and is suppressing nothing.

5. **Prove the hatch is narrow.** Move one marker two lines above its citation,
   confirm the case fails again, then restore it with an inverse edit. A hatch
   that works from anywhere in the file is not the hatch this design specifies.

6. **Prove the rule bites on a Tier-A entry too.** Temporarily change one
   `data-tier="A"` to `"B"` on an entry cited in a superscript, confirm the case
   fails for that citation, and restore with an inverse edit. This shows the
   rule reads the tier rather than a hard-coded list of bad refs.

7. **Run** `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Append
   evidence, recording the final suppression count and the list. **Commit** with
   `Plan:` and `Slice:` trailers, no `Rows:`.

## Task 3 — The Lomax attribution (F22)

**Modifies:** `site/src/content/docs/appendix-references.mdx`,
`site/src/content/docs/03-afro-cuban.mdx`,
`site/tests/citation-tier.test.mjs`
**Rows:** F22

`03-afro-cuban.mdx` line 48 says the habanera is what "Jelly Roll Morton called
'the Spanish tinge'". The claim is accurate and uncited, and **no Lomax entry
exists anywhere in the appendix**, so this adds an entry as well as a citation.

### Steps

1. **Write the failing case.** Add `S06-F22` to the `CLAIMS` array:
   `id: 'S06-F22'`, `file: '03-afro-cuban.mdx'`,
   `presentRegex: [/#fr-lomax-1950/]`, with a `rule` recording that the
   attribution is accurate but was uncited and that Lomax's Morton interviews
   are the primary source.

2. **Run it and watch it fail on the `present` side** — there is no forbidden
   phrase here, because nothing wrong is being removed; a claim is being
   sourced. This is the one case in the milestone whose first failure is
   *expected* to be a missing corrective rather than a surviving forbidden one.

3. **Add the appendix entry** to the Further Reading list, in the Chapter 3
   grouping, tiered `A` — it is a primary source:

   > `- <span id="fr-lomax-1950" data-tier="A">Lomax, A. (1950). *Mister Jelly Roll: The Fortunes of Jelly Roll Morton, New Orleans Creole and "Inventor of Jazz"*. Duell, Sloan and Pearce. — The Library of Congress interviews in which Morton describes the "Spanish tinge".</span>`

4. **Cite it** at line 48, after the closing quotation mark of "the Spanish
   tinge," using the parenthetical Further Reading form:
   `([Lomax 1950](/appendix-references/#fr-lomax-1950))`.

5. **Run the case and watch it pass.** Confirm Task 1's tier assertion still
   passes — the new entry must carry `data-tier`, and it is the first test of
   whether that assertion actually catches an untiered arrival. If it does not
   fail when you temporarily drop the attribute, the assertion is vacuous: fix
   it before continuing.

6. **Run** `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Append
   evidence. **Commit** with `Rows: F22`.

## Task 4 — Wire into the runner and REQUIRED, run `gate`, close the slice

**Modifies:** `scripts/check-doc-conformance.sh`,
`site/tests/doc-conformance-wiring.test.mjs`,
`docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/M002-S06-plan.md`,
`docs/plans/theory-audit/evidence/M002-S06.md`
**Consumes:** the check built by Tasks 1–3
**Rows:** F23

### Steps

1. **Add `site/tests/citation-tier.test.mjs` to the `REQUIRED` array** in
   `site/tests/doc-conformance-wiring.test.mjs`, in the M002 grouping with a
   comment naming the slice.

2. **Run the wiring test and watch it fail** — `REQUIRED` now names a file the
   runner does not, which is exactly the coverage-drop assertion. That failure
   is the proof the wiring test actually defends the new check.

3. **Add the same path to the `TESTS` array** in
   `scripts/check-doc-conformance.sh`.

4. **Run the wiring test and watch it pass**, then run `doc-conformance` and
   confirm the runner's own output now includes the citation-tier cases.

5. **Run every token the slice declares**, in order:
   `format`, `site-unit`, `doc-conformance`, `doc-discipline`, then `gate`
   (`bash scripts/pre-push-check.sh`). Read each exit code. `gate` builds the
   plugin and runs ctest and pluginval, so allow it minutes rather than seconds.
   If `gate` fails for a reason this slice did not cause, stop and report rather
   than fixing unrelated breakage inside this commit.

6. **Close the rows and the slice.** In `docs/plans/theory-audit/ledger.md`, set
   F22 and F23 to `done`. Rewrite F23's `Verification` cell to describe what was
   actually built — the tier attribute, the two assertions, the hatch and its
   suppression count — and correct both rows' `Lands in` cells to the files this
   slice modified. Record in F23's `Item` cell that the check ships with N live
   suppressions owned by M006/S01, giving the real N. Use no `|` in any cell.

   Then tick all four Definition-of-Done boxes in the slice and in this plan's
   copy, tick this plan's Task 4 box, and set the slice `Status` to `done`.

7. **Append the evidence**, recording the `gate` result explicitly since no
   earlier slice has run it. Then run `format` last, per the gate ordering, and
   `jk-standards ledger`.

8. **Commit** as one unit with trailers
   `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M002/S06`,
   `Rows: F22, F23`.
