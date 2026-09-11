# M006/S02 — Resolve the orphaned references

**Slice:** M006/S02, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M006-decisions.md` — B06, B07 and B11 were researched before
planning, and three ledger amendments follow from what that found.

## Task status

- [ ] Task 1 — Assert every entry is cited, or says it is unread
- [ ] Task 2 — Delete the tier-C orphans
- [ ] Task 3 — Place or retire the tier-A and tier-B orphans
- [ ] Task 4 — Close the slice

## Definition of Done

- [ ] Every numbered reference is cited by at least one page, or is retired
      with the reason recorded
- [ ] A check fails when an appendix entry is neither cited by a page nor
      carries the `contents unverified` phrase, which is the one stated
      exception and is itself in-band, greppable and reasoned

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What was measured

109 appendix entries; **21 orphaned** — 18 numbered and 3 in Further Reading.
B08 says "eighteen numbered references and two Further Reading entries", which
was right when written: M005 made it three by adding `fr-peycheva-dimov-2002`
deliberately, and the stated exception covers that one.

| Tier | Count | Entries |
|---|---|---|
| A | 8 | `ref-2`, `ref-23`, `ref-28`, `ref-29`, `ref-40`, `ref-44`, `fr-toussaint-2013`, `fr-toussaint-2005` |
| B | 6 | `ref-7`, `ref-22`, `ref-24`, `ref-27`, `ref-30`, `fr-peycheva-dimov-2002` |
| C | 7 | `ref-8`, `ref-11`, `ref-12`, `ref-15`, `ref-16`, `ref-37`, `ref-45` |

**Re-measure before acting.** `fr-toussaint-2005` is also B09's duplicate, so if
M006/S03 has already run it is gone and this list is one shorter. Recompute the
orphan set from the tree rather than trusting this table.

## The policy, decided up front

- **Tier C orphans are deleted.** M002's argument was that such sources are not
  fit to carry a claim; an uncited one is pure dead weight.
- **Tier A and B orphans are cited where they genuinely support an existing
  claim**, and retired with the reason recorded where they have no honest home.
- **No passage is written to host a citation.** That is the rule M005/S04 halted
  rather than break, and it binds here too.

## Task 1 — Assert every entry is cited, or says it is unread

Produces the check. Closes no row.

**Files:** `site/tests/citation-tier.test.mjs`

1. **Write the case**: every `id="ref-*"` or `id="fr-*"` in
   `appendix-references.mdx` is either referenced by `#<anchor>` from some other
   page under `site/src/content/docs`, **or** its entry carries the phrase
   `contents unverified`. The failure message must list the orphans by anchor
   and tier, because the next two tasks work from that list.
2. **Run it and watch it fail**, naming all 21 — 20 once S03 has run.
3. **Check** the three tokens. The suite is red on this case by design until
   Task 3; that is why this task closes no row and why the slice's rows close at
   the end rather than here.

   **If the red case would block a commit**, land Tasks 1–3 as one commit
   instead of three. Say so in the message rather than committing a red suite:
   `/jk:next` forbids the latter and the plan must not ask for it.

## Task 2 — Delete the tier-C orphans

**Files:** `site/src/content/docs/appendix-references.mdx`

1. **Recompute the tier-C orphan set** from the tree.
2. **Delete those entries.** Record each in the evidence file with its anchor,
   tier and what it was — a deletion nobody can audit later is worse than the
   dead weight it removed.
3. **Confirm nothing cites them** — the recomputation already establishes this,
   but a `grep` for each anchor across `site/` is cheap and catches a citation
   in a file the check does not scan.
4. **Check** the three tokens.

## Task 3 — Place or retire the tier-A and tier-B orphans

Closes **B08**.

**Files:** `site/src/content/docs/appendix-references.mdx`, whichever chapter
files gain citations, the test host

For each tier-A and tier-B orphan except `fr-peycheva-dimov-2002`:

1. **Read the entry** and decide whether any existing claim in the guide is one
   it genuinely supports. `ref-2` is Goldberg on Bulgarian meter and nationalism
   — Chapter 7 discusses Bulgarian metre, so it plausibly has a home. `ref-44`
   is Gotham's review of Toussaint, and `fr-toussaint-2013` is Toussaint's book;
   Chapter 1 is where Toussaint's results are used.
2. **Cite it there** if so, binding the case to a phrase from the passage.
3. **Retire it** if not, deleting the entry and recording the reason.
4. **Do not write a passage to host a citation.** If an entry has no home, it is
   retired; that is what "retired with the reason recorded" means in the row.
5. **Run the Task 1 case and watch it pass**, then prove it bites by deleting a
   citation you just added.
6. **Check** the three tokens, and **commit** with `Rows: B08`.

## Task 4 — Close the slice

1. **Tick both definition-of-done boxes**, tick Task 4, confirm B08 is `done`,
   set the slice `Status` to `done`.
2. **Run every token**, then `jk-standards ledger`.
3. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (every numbered reference cited or retired with the
reason) → Tasks 2 and 3. Item 2 (a check fails when an entry is neither cited
nor marked `contents unverified`) → Task 1.

**Row coverage.** B08 → Task 3 step 6. Tasks 1, 2 and 4 close no row.

**Placeholder scan.** No TBDs. The orphan table is measured, and every task is
told to recompute rather than trust it.

**Name consistency.** `contents unverified` is the exact phrase throughout,
matching what M005 wrote into `fr-peycheva-dimov-2002`.

**Ordering.** Task 1 first so the list is machine-produced rather than
transcribed. Task 2 before Task 3 because deleting the tier-C entries shortens
the list Task 3 must judge. Task 4 last.
