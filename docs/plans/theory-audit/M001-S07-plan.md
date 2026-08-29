# M001/S07 — Harness migration to the ledger

**Slice:** M001/S07
**Ledger:** `docs/plans/theory-audit/ledger.md`

The programme's completeness gate currently reads
`docs/audits/M001-theory-audit-remediation-plan.md`. That doc is no longer the
plan of record — this ledger is — so the gate is guarding a file nobody will
maintain, and the two disagree about which slices exist. This slice moves the
gate onto the ledger and retires the doc.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [x] Task 1 — Repoint the parser at the ledger
- [x] Task 2 — Related-issues guards against the list form
- [ ] Task 3 — Archive the plan doc and repoint its neighbours
- [ ] Task 4 — Evidence and ledger close-out

## Definition of Done

Copied verbatim from the slice. Every task below argues against *this* text.

- [ ] `theory-audit-remediation.test.mjs` reads this ledger, not the retired
      plan doc, and still fails if any of F01–F54 goes missing or gains a
      second home
- [ ] The test asserts rows sit under a real slice by nesting, with no
      cross-referencing slice column
- [ ] The three Related-issues guards survive the migration, rewritten against
      this ledger's list form: every issue cites a row that resolves, no issue
      is both listed and declared deliberately absent, and "Out of scope" cites
      only issues the list carries
- [ ] `docs/audits/M001-theory-audit-remediation-plan.md` is `class: archived`
      and names this ledger as its successor
- [ ] `jk-standards ledger` and `jk-standards doc-taxonomy` both pass

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `jk-standards all` |

`doc-conformance` is the token that actually exercises the rewritten test:
`scripts/check-doc-conformance.sh` runs `docs/audits/*.test.mjs`, while
`site-unit` covers only `site/tests`. Run `node --test` directly on the file
during the inner loop; run `doc-conformance` before claiming a task done.

## Context

**The test stays where it is.** `docs/audits/theory-audit-remediation.test.mjs`
is named by ledger row H03, by `scripts/check-doc-conformance.sh` (in the list
of audit tests it runs), and by `site/tests/doc-conformance-wiring.test.mjs`
(in its expected-tests array). Moving it would mean editing both of those for
no benefit. Only the fixture it reads changes.

**What this test uniquely covers.** `jk-standards ledger` already checks ID
shape, status vocabulary, DoD presence, validation tokens, and placeholders. It
has no idea that this programme owes exactly 54 findings, and it does not look
at the Related-issues list at all. Those are this test's job, and the overlap on
status tokens and placeholders is deliberate belt-and-braces.

**Shape differences the parser must absorb.**

| | Plan doc (old fixture) | Ledger (new fixture) |
|---|---|---|
| Milestone heading | `### M001 — Theory Corrections (…)` | `## Milestone M001 — Theory Corrections` |
| Slice heading | none; slices lived in a "Milestone breakdown" table | `### Slice M001/S07 — Harness migration to the ledger` |
| Row → slice link | a `Slice` column holding `M001/S07` | nesting under the slice heading |
| Row columns | `ID │ Audit § │ Finding │ Sev │ Disp │ Fix lands in │ Slice │ Verification │ Status` | `ID │ Item │ Sev │ Disp │ Lands in │ Verification │ Status` |
| Row IDs | `F01`–`F54` | `F01`–`F54` plus `H01`–`H04` |
| M005 rows | no `Sev`/`Disp` columns; implicitly P2/`enrich` | explicit `Sev`/`Disp` like every other row |
| Related issues | a markdown table | a bullet list |

The ledger contains exactly 29 markdown tables and they all have the identical
header `| ID | Item | Sev | Disp | Lands in | Verification | Status |` — one per
slice, and nothing else. The parser can therefore treat "any table" as "a slice
row table" without disambiguating, which is simpler than the old two-mode
parser.

---

## Task 1 — Repoint the parser at the ledger

**Modifies:** `docs/audits/theory-audit-remediation.test.mjs`
**Consumes:** nothing. **Produces:** a parser the Task 2 guards read
`rowsById` from.

### Steps

1. Change the fixture constant. Replace

   ```js
   const DOC = join(HERE, 'M001-theory-audit-remediation-plan.md');
   const md = readFileSync(DOC, 'utf8');
   ```

   with

   ```js
   const LEDGER = join(HERE, '..', 'plans', 'theory-audit', 'ledger.md');
   const md = readFileSync(LEDGER, 'utf8');
   ```

2. Run `node --test docs/audits/theory-audit-remediation.test.mjs` and **watch
   it fail**. The expected failure is specific: the old header detector requires
   a `slice` column, which the ledger's tables do not have, so no rows parse at
   all. You should see `every finding ID F01–F54 appears exactly once` fail
   listing all 54 as missing, and `Related issues: the table is non-empty …`
   fail with zero rows. If you see a different failure, the fixture path is
   wrong — fix that before continuing.

3. Replace `parseDoc()` with `parseLedger()`. It returns
   `{ rows, sliceIds, sliceColumnSeen }`:

   - Track headings with
     `const MILESTONE_H2 = /^##\s+Milestone\s+(M\d{3})\b/;` and
     `const SLICE_H3 = /^###\s+Slice\s+(M\d{3})\/(S\d{2})\b/;`.
     A milestone heading sets `currentMilestone` and clears `currentSlice`;
     a slice heading sets `currentSlice` to the full `M001/S07` string and adds
     it to `sliceIds`.
   - A table header row is recognised when its lower-cased cells contain `id`,
     `item` and `status`. Capture `idxId`, `idxSev`, `idxDisp`, `idxStatus`.
     If the same header also contains a cell equal to `slice`, set
     `sliceColumnSeen = true` — that flag is what step 4's nesting case asserts
     against.
   - A row is collected when `c[idxId]` matches `/^[FH]\d{2}$/`. Push
     `{ id, sev, disp, status, milestone: currentMilestone,
     slice: currentSlice, row: line, rowNum: i + 1 }`, reading `sev`, `disp`
     and `status` through the existing `backtickToken()` helper. A row whose
     `currentSlice` is `null` keeps `slice: null` so step 4 can name it.
   - Delete the `breakdown` mode, the `breakdowns` map, the `MILESTONES`
     constant, and the `sliceRefs()` helper. Nothing reads them once the
     nesting case replaces the slice-resolution case.
   - Keep `cells()`, `isTableRow()`, `isSeparator()` and `backtickToken()`
     unchanged.
   - Delete the `sevExplicit` / `dispExplicit` fields and the
     `table.idxSev !== -1 ? … : 'P2'` fallbacks. The ledger carries explicit
     `Sev` and `Disp` on every row, so an absent token must now fail rather
     than default.

4. Rewrite the seven finding cases. Keep them in this order:

   - `ledger exists and is substantial` — `assert.ok(md.length > 5000)`.
   - `every finding ID F01–F54 appears exactly once` — filter `rows` to
     `id.startsWith('F')` before comparing against `EXPECTED_IDS`. Keep all
     three existing assertions (missing, duplicate, unexpected). `EXPECTED_IDS`
     stays the hard-coded F01–F54 array; do not scrape it from the ledger.
   - `every harness row carries a well-formed ID` — assert every row's `id`
     matches `/^(F\d{2}|H\d{2})$/`. This is a new case that replaces nothing;
     it exists so an `H`-prefixed typo cannot slip past the F-only case above.
   - `every row carries exactly one valid severity token` — over **all** rows,
     F and H alike.
   - `every row carries exactly one valid disposition token` — over all rows.
   - `every row carries exactly one valid status token` — over all rows.
   - `every row sits under a real slice, and no row table declares a Slice
     column` — assert `sliceColumnSeen === false` with the message
     `row tables must link rows to slices by nesting, not by a Slice column`,
     and assert no row has `slice === null`, naming any that do. Also assert
     `sliceIds.size >= 1` so a parser that silently matches no slice headings
     fails loudly instead of passing vacuously.
   - `no _pending_ placeholder survives in any row` — unchanged except that it
     iterates all rows.

   That is eight cases where there were seven; the count in ledger row H03 says
   ten total, counting the three Related-issues guards Task 2 rewrites. Update
   H03's cell to say eleven in Task 4 if you keep the new well-formed-ID case,
   or fold that assertion into the F01–F54 case and leave the count at ten.
   Either is fine; the ledger and the file must agree.

5. Update the file's header comment block (lines 1–41) so the numbered contract
   describes the ledger: the fixture path, nesting instead of a `Slice` column,
   `H`-rows alongside `F`-rows, and the removal of the M005 implicit-P2 case.
   The comment is the contract a reader trusts; leaving it describing the plan
   doc is the same drift this slice exists to remove.

6. Run `node --test docs/audits/theory-audit-remediation.test.mjs`. Expect the
   finding cases to pass and the three Related-issues cases to still fail —
   Task 2 owns those. Do not proceed until the only failures are the three
   named `Related issues: …` / `Out of scope …` cases.

### Verification

Prove the gate still bites, with two mutations of the ledger:

1. Delete the `| F31 | …` row from `docs/plans/theory-audit/ledger.md`. Re-run
   `node --test docs/audits/theory-audit-remediation.test.mjs`. The
   `every finding ID F01–F54 appears exactly once` case must fail with
   `missing finding IDs: F31`. Restore the row.
2. In the `H03` row, change `` `open` `` to `` `pending` ``. Re-run. The status
   case must fail naming `H03`. Restore.

Then `git diff --quiet docs/plans/theory-audit/ledger.md` — it must exit 0,
proving both mutations were undone.

---

## Task 2 — Related-issues guards against the list form

**Modifies:** `docs/audits/theory-audit-remediation.test.mjs`
**Consumes:** `rows` / `byId` from Task 1. **Produces:** a green file.

The ledger's `## Related issues` section is a bullet list, not a table:

```markdown
- [#91](https://github.com/JimAKennedy/poly/issues/91) — **closed by F54**. The
  E(3,16) appendix row is correct under Poly's phase convention; …
```

### Steps

1. Keep `sectionLines()`, `issueNumbers()`, `ISSUES_HEADING` and
   `OUT_OF_SCOPE_HEADING` exactly as they are — they are shape-agnostic.

2. Replace the `issueRows` table-scraping loop with bullet parsing. A bullet
   starts a new entry when the line matches `/^\s*-\s+/`; continuation lines are
   indented and belong to the entry above, so accumulate text until the next
   bullet or the end of the section:

   ```js
   const issueEntries = [];
   for (const line of issuesSection) {
     if (/^\s*-\s+/.test(line)) {
       issueEntries.push(line);
     } else if (issueEntries.length && /^\s+\S/.test(line)) {
       issueEntries[issueEntries.length - 1] += ' ' + line.trim();
     }
   }
   const issueBullets = issueEntries
     .map((text) => {
       const num = text.match(/#(\d+)\b/);
       return num ? { issue: num[1], rowRefs: text.match(/[FH]\d{2}/g) || [], text } : null;
     })
     .filter(Boolean);
   ```

   Accumulating continuation lines matters: the F-reference and the issue number
   can land on different physical lines once the paragraph wraps.

3. Recompute the deliberately-absent set from the section's non-bullet prose:

   ```js
   const absentProse = issuesSection
     .filter((l) => !/^\s*-\s+/.test(l) && !/^\s+\S/.test(l))
     .join('\n');
   const absentIssues = issueNumbers(absentProse);
   ```

4. Rewrite the three cases against `issueBullets`, renaming "table" to "list"
   in both the case names and the failure messages:

   - `Related issues: the list is non-empty and every entry cites resolvable
     ledger rows` — assert `issueBullets.length > 0`; for each entry assert
     `rowRefs.length >= 1` and that every ref is in `byId`.
   - `Related issues: no issue is both listed and declared deliberately absent`
     — unchanged logic against `issueBullets`.
   - `Out of scope cites only issues the Related-issues list carries` —
     unchanged logic against `issueBullets`.

5. Update the comment block above these cases (currently lines 294–303) to
   describe the list form. Record plainly that the second guard is **currently
   vacuous** — the ledger has no absent-prose today, so `absentIssues` is empty
   — and that it is retained to bite the day one is added. An unexplained
   always-passing test is worse than no test.

### Verification

1. `node --test docs/audits/theory-audit-remediation.test.mjs` — all cases pass.
2. Mutation: in the ledger's `#91` bullet, change `F54` to `F99`. Re-run; the
   resolvable-rows case must fail naming `F99`. Restore.
3. Mutation: in the ledger's `## Out of scope` section, change `#157` to
   `#999`. Re-run; the Out-of-scope case must fail naming `#999`. Restore.
4. `git diff --quiet docs/plans/theory-audit/ledger.md` exits 0.
5. `bash scripts/check-doc-conformance.sh` exits 0.
6. `npm --prefix site test` exits 0 — this is what proves
   `doc-conformance-wiring.test.mjs` still recognises the file.

---

## Task 3 — Archive the plan doc and repoint its neighbours

**Modifies:** `docs/audits/M001-theory-audit-remediation-plan.md`,
`docs/audits/poly_theory_audit.md`, `.github/docs-drift-map.yml`
**Consumes:** nothing. **Produces:** the archived state Task 4 records.

### Steps

1. In `docs/audits/M001-theory-audit-remediation-plan.md`, change the front
   matter `class: gated` to `class: archived`.

2. Replace the whole opening blockquote — the `**Status:**`, `**Lifecycle:**`,
   `**Upstream input:**`, `**Numbering:**` and `**Completeness:**` lines — with
   an archival note in the same shape `poly_theory_audit.md` already uses:

   ```markdown
   > **Archived (2026-08-28)** — superseded by
   > [the theory-audit delivery ledger](../plans/theory-audit/ledger.md), which
   > is now the plan of record for milestones M001–M005 and the file
   > `theory-audit-remediation.test.mjs` guards. The finding tables below are
   > kept as the historical decomposition and as the mapping from each finding
   > to its "Audit §" citation in
   > [poly_theory_audit.md](poly_theory_audit.md), which the ledger's rows do
   > not carry. Do not edit them to reflect later work: their `Status` column
   > froze on 2026-08-28 and the ledger is the live record.
   ```

   The "Audit §" sentence is load-bearing. The ledger deliberately dropped that
   column, so this doc remains the only place a finding ID maps back to a
   section of the external audit. Saying so is why the doc is archived rather
   than deleted.

3. In `docs/audits/poly_theory_audit.md`, the archival note currently ends
   "…and the remediation plan's ledger — not this file — is the live record of
   what has since been corrected." It points at the doc being archived in step
   1. Repoint it at `../plans/theory-audit/ledger.md`, while keeping its
   existing statement that this file's §1–§7 numbering is what the plan doc's
   tables cite. Leaving it is exactly the stale cross-reference row H04 exists
   to prevent.

4. In `.github/docs-drift-map.yml`, delete the
   `- doc: "docs/audits/M001-theory-audit-remediation-plan.md"` entry together
   with its `reason:` block. The file's own header says "Do NOT map: Archived
   docs (they are frozen by definition — the taxonomy check gates them)", and
   an archived doc is exempt by class in `doc-completeness`, so the declaration
   is not merely redundant but contradicts the map's stated policy.

### Verification

Each command must exit 0:

1. `jk-standards doc-taxonomy` — 68 docs carry a valid class.
2. `jk-standards doc-completeness` — reports one more doc exempt by class than
   before this task (8 rather than 7), and no unmapped doc.
3. `jk-standards all`.
4. `pre-commit run --all-files`.
5. `bash scripts/check-doc-conformance.sh` — confirms the class flip did not
   disturb the gate, since the test no longer reads this doc at all.

Commit at this point, with the trailers:

```
Plan: docs/plans/theory-audit/ledger.md
Slice: M001/S07
Rows: H03,H04
```

---

## Task 4 — Evidence and ledger close-out

**Modifies:** `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M001-S07.md`
**Consumes:** the commit SHA produced at the end of Task 3.

### Steps

1. Create `docs/plans/theory-audit/evidence/M001-S07.md` recording, for each of
   the slice's four validation tokens, the command run and what it returned,
   plus the Task 3 commit SHA and the date. Follow the terse shape the
   backfilled files in that directory already use — command, result, commit,
   date. Record the two Task 1 mutations and the two Task 2 mutations as the
   evidence that the gate bites, naming the case each one tripped: this is the
   part a reviewer cannot reconstruct from the diff.

2. In `docs/plans/theory-audit/ledger.md`, in the M001/S07 slice:

   - tick all five `Definition of Done` boxes
   - change the `H03` row's `Status` from `` `open` `` to `` `done` ``
   - change the `H04` row's `Status` from `` `open` `` to `` `done` ``
   - change the slice's `**Status:**` from `in-progress` to `done`
   - if Task 1 step 4 kept the extra well-formed-ID case, change `all 10 of its
     cases` in H03's `Verification` cell to `all 11 of its cases`

3. Run `jk-standards ledger`. It enforces that a `done` slice has every DoD box
   ticked, an evidence file on disk, and no row left `open` — so this is the
   command that proves step 2 is internally consistent, not just edited.

### Verification

1. `jk-standards ledger` — 1 ledger conforms.
2. `jk-standards all` — exit 0.
3. `pre-commit run --all-files` — exit 0.
4. Commit with the same trailers as Task 3.

---

## Self-review

**Definition of Done → task.**

| DoD item | Satisfied by |
|---|---|
| Test reads the ledger and still fails on a missing or duplicated F-row | Task 1 steps 1–4; proved by Task 1 verification mutation 1 |
| Rows sit under a real slice by nesting, no cross-referencing slice column | Task 1 step 4, the `every row sits under a real slice…` case |
| Three Related-issues guards survive against the list form | Task 2 steps 2–4; proved by Task 2 verification mutations 2 and 3 |
| Plan doc is `class: archived` and names this ledger as successor | Task 3 steps 1–2; proved by Task 3 verification 1 |
| `jk-standards ledger` and `doc-taxonomy` pass | Task 3 verification 1, Task 4 verification 1 |

**Row → task.**

| Row | Closed by | Its `Verification` produced by |
|---|---|---|
| H03 | Tasks 1 and 2 | Task 1 verification (fixture path, mutation proof) and Task 2 verification 1 (full case count green) |
| H04 | Task 3 | Task 3 verification 1 (`doc-taxonomy` accepts `archived`) and step 2 (successor named) |

**Placeholder scan.** No "TBD", no "add appropriate error handling", no "similar
to task N", no reference to a helper no task defines. Every function this plan
names — `parseLedger`, `cells`, `isTableRow`, `isSeparator`, `backtickToken`,
`sectionLines`, `issueNumbers` — is either defined in Task 1 step 3 or already
exists in the file at the paths given.

**Name consistency.** The parser is `parseLedger()` throughout; its outputs are
`rows`, `sliceIds`, `sliceColumnSeen`; the derived index is `byId`; the parsed
Related-issues entries are `issueBullets` in every task that mentions them.

**One decision left to the executor, deliberately.** Task 1 step 4 allows either
eight finding cases plus three guards (eleven) or folding the well-formed-ID
assertion into the F01–F54 case (ten). Both satisfy the DoD; Task 4 step 2
requires whichever is chosen to be reflected in H03's cell, so the ledger cannot
end up disagreeing with the file.
