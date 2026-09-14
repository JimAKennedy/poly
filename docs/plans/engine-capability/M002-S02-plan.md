---
class: gated
---

# M002/S02 — The guide catches up

**Slice:** M002/S02 — `docs/plans/engine-capability/ledger.md`
**Rows:** EC07
**Depends:** M002/S01
**Classification:** bounded. Prose edits and triage flips, against a capability
S01 has already shipped.

## Task status

- [x] 1. Rule 4: drop the parenthetical and check it as written
- [ ] 2. Rule 1: flip to checkable with a predicate that can fail
- [ ] 3. Rule 5 and the disclosures, and close the slice

## Definition of Done

- [ ] Rule 4 no longer carries the "until kotekan modes ship" parenthetical
- [ ] Rule 4 is checked as written — pair-overlap — rather than as construction
      step 4 specifies
- [ ] Rule 1 reads `checkable`, with a predicate that has been shown to fail
- [ ] This ledger records that the theory-audit ledger's F41 weakening no longer
      describes the shipped behaviour
- [ ] The page states that Poly models only the rhythmic dimension of *telu* and
      *empat* — the cell length the interlock repeats on, not their pitch
      content — and directs the reader to Tenzer (2000) and Vitale (1990) for
      that dimension; *norot* is named as not expressible, and why

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Context every task needs

- **Do not start before M002/S01 has landed.** Every predicate here needs the
  `Kotekan` column to carry a mode, and the patch to carry an overlap. If S01 is
  not done, stop — that is the dependency.
- **What M007 of the theory-audit programme found**, and what this slice
  reverses: Rule 4's strict predicate could not be made to fail under `L`-mode,
  and Rule 1 was marked not checkable for the same cause — the engine derived
  the complement, so the composite was complete by construction. With `Telu` and
  a non-zero overlap, neither is true any more.
- **The patch table lives in `theory-gamelan.mdx`** under
  `<PolyPatch title="Rule-Checked Kotekan Over Colotomy">`, and already carries a
  `Kotekan` column reading `off` / `L6`. M001/S01 added its `Note` column; the
  registration rules from that slice apply here unchanged — a `CHECKLIST.push`
  must land above `let liveMarkers = 0;` or it is silently never registered.

## Task 1 — Rule 4: drop the parenthetical and check it as written

**Files:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. Register a predicate for Rule 4 asserting the pair's intersection is
   non-empty: the polos and sangsih lanes strike together at at least one step.
   Derive both patterns from the table's own columns rather than asserting a
   literal. Run `site-unit` and watch it fail — the shipped patch is still
   strictly complementary until its `Kotekan` cell names an overlap.
2. Update the patch table so the sangsih lane names its mode and overlap, and
   the values agree with what `Balinese Kotekan` now ships.
3. Remove Rule 4's parenthetical — the clause from "(Poly's Kotekan `L1`
   implements the strict case" to "until kotekan modes ship.)" — and replace the
   sentence it qualified with one that states the overlap is a setting.
4. Run `site-unit` and watch the predicate pass.
5. **Mutation-prove it:** set the overlap cell to 0, re-run, confirm the case
   fails naming the empty intersection. Restore, `git diff --quiet`.
6. Update `RULE_TRIAGE` for `theory-gamelan.mdx` rule 4 so its `why` no longer
   cites construction step 4 as the reason it is checked loosely.
7. Run the full validation set. Evidence, tick task 1, ledger, commit.

## Task 2 — Rule 1: flip to checkable

**Files:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. Read Rule 1 verbatim before writing anything. M007 deleted three predicates
   that could not fail; this one was one of them, for a reason that S01 has now
   removed. If after reading it the rule still cannot be made to fail against a
   static patch, **stop and say so** — flipping it to `checkable` with an
   unfalsifiable predicate would be worse than leaving it.
2. Register the predicate and flip the triage entry, naming the case.
3. Run `site-unit`, watch it pass.
4. **Mutation-prove it** against the specific property the rule names, and
   record in the evidence what mutation fired it — this is the rule M007 could
   not falsify, so the proof is the whole point.
5. Full validation set. Evidence, tick task 2, ledger, commit.

## Task 3 — Rule 5, the disclosures, and the slice close

The disclosure is a definition-of-done item, not a nicety: the design keeps the
traditional names *telu* and *empat* for modes that model only their rhythm, and
that is defensible only if the page says so.

**Files:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/scope-framing.test.mjs` or
`site/tests/theory-audit-claims.test.mjs` — whichever the neighbouring
disclosures use; say which in the evidence,
`docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M002-S02.md`

1. Add a claim case requiring three things on the page, and run it first to
   watch it fail:
   - that only the **rhythmic** dimension of *telu* and *empat* is modelled —
     the cell length the interlock repeats on, not pitch content
   - that [Tenzer 2000](/appendix-references/#fr-tenzer-2000) and
     [Vitale 1990](/appendix-references/#fr-vitale-1990) are named as where to
     read about the pitch dimension
   - that *norot* is named as not expressible in Poly, because it is defined by
     pitch oscillation around neighbouring tones and a Poly lane carries one note
2. Write the disclosure into Rule 5's paragraph — at the point of use, in the
   voice M003 used for its five simplification disclosures, not as a footnote.
   Rule 5 also gains that three of its four styles are now settings.
3. Run `site-unit`, watch it pass. **Mutation-prove each of the three arms
   separately** by removing one requirement at a time; a disclosure case that
   fires only on total deletion is not guarding the parts.
4. Record in this ledger that theory-audit row F41's weakening no longer
   describes the shipped behaviour — in M002's own prose, not by editing the
   theory-audit ledger, which is a closed programme's record.
5. Run the full validation set. Append evidence, tick task 3, set row EC07 to
   `done`, tick all five definition-of-done boxes, set slice M002/S02 to `done`,
   run `jk-standards ledger`, commit with trailers.
