# M006/S01 — Upgrade the suppressed claim citations

**Slice:** M006/S01, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M006-decisions.md` — B06, B07 and B11 were researched before
planning, and three ledger amendments follow from what that found.

## Task status

- [x] Task 1 — Re-point the six citations whose replacements are already here
- [x] Task 2 — Add the Linn interview and cite it in both places (B06)
- [ ] Task 3 — Add Harrison and cite the Amen-break claim (B07)
- [ ] Task 4 — Close the slice on a zero suppression count

## Definition of Done

- [ ] Each of the eight claims either cites a Tier-A source, cites the primary
      source it was standing in for, or no longer makes a claim requiring one
- [ ] `grep -rn citation-tier-ok site/src/content/docs` returns nothing, and the
      tier check's live suppression count is zero

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What the tree already tells you

`grep -rn citation-tier-ok site/src/content/docs` returns **ten** markers across
nine files, and each names the row that owns it. Eight rows cover them: B06
covers two (`09-electronic.mdx` and `theory-electronic-breakbeat.mdx` are the
same source and defect), and **B17 was added during planning** for the two that
cite `ref-42` — their markers say outright that S01 carried seven rows and this
was the eighth.

Every marker names its replacement. Six point at entries already in this repo:

| Row | File | Replacement, already present |
|---|---|---|
| B01 | `01-foundations.mdx` | `ref-1` / `ref-46` (Toussaint) |
| B02 | `03-afro-cuban.mdx` | `fr-mauleon-1993` |
| B03 | `05-gamelan.mdx` | `fr-vitale-1990` |
| B04 | `08-minimalism.mdx` | `fr-cohn-1992` |
| B05 | `08-minimalism.mdx` | `fr-potter-2000` |
| B17 | `10-brazilian.mdx`, `appendix-euclidean-reference.mdx` | `fr-sandroni-2001`, `fr-fryer-2000`, `ref-1` |

Read each marker in full before touching its citation. They say precisely what
is wrong with the current source and what should replace it, and they are the
most reliable statement of intent in the tree — better than this plan.

## Task 1 — Re-point the six citations whose replacements are already here

Closes **B01, B02, B03, B04, B05, B17**.

**Files:** the six chapter files above, plus `site/tests/citation-tier.test.mjs`

1. **Write the failing case.** Add to `citation-tier.test.mjs` — or to
   `literature-enrichment.test.mjs` if that is the better host; check which one
   already asserts suppression counts — a case asserting
   `grep`-equivalent: no `citation-tier-ok` marker naming B01, B02, B03, B04,
   B05 or B17 remains in `site/src/content/docs`.
2. **Run it and watch it fail**, naming all six.
3. **For each row in turn**: read its marker, re-point the citation at the
   replacement the marker names, remove the marker. Do not batch this blindly —
   B01's marker says only *half* the sentence is mis-cited, and B04 and B05 are
   two different claims in the same file.
4. **Run and watch it pass**, and confirm the tier check's live suppression
   count has dropped from 10 to 4 — the two B06 markers, B07's, and the one in
   `05-gamelan.mdx` if B03's turns out to need more than a re-point.
5. **Prove it bites.** Restore one marker, watch the case name that row, remove
   it again by inverse edit.
6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`, and
   `npm --prefix site run build`, since these are inline MDX links.
7. **Commit** with `Rows: B01, B02, B03, B04, B05, B17`, ticking Task 1, closing
   all six, appending to `docs/plans/theory-audit/evidence/M006-S01.md`.

## Task 2 — Add the Linn interview and cite it in both places (B06)

Closes **B06**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`09-electronic.mdx`, `theory-electronic-breakbeat.mdx`, the test host

The marker says: *"not Linn speaking. This is Thomas Brett's commentary, quoting
Linn from an Attack Magazine interview published elsewhere; that interview is
the primary source this should cite."* Verification found it —
"Roger Linn On Swing, Groove & The Magic Of The MPC's Timing", *Attack
Magazine*, July 2020 — and it carries Linn describing the LM-1 mechanism
directly.

**Tier B, not A.** A professional publication interviewing the primary actor is
not scholarship. The upgrade is from commentary-quoting-an-interview to the
interview itself, which is what the row asks for; claiming tier A would
overstate it.

1. **Write the failing cases**: `fr-linn-attack-2020` exists with a
   `data-tier`, and is cited from both files at the swing claim.
2. **Run and watch them fail.**
3. **Add the entry**, and re-point both citations. Both markers go.
4. **Run, watch them pass, prove each bites**, restoring by inverse edit.
5. **Check.** All three tokens plus the site build.
6. **Commit** with `Rows: B06`, ticking Task 2, closing B06, appending evidence.

## Task 3 — Add Harrison and cite the Amen-break claim (B07)

Closes **B07**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`13-drum-and-bass.mdx`, the test host

B07's marker expects only reception to be witnessable: *"a Tier-A source for the
reception claim would still be better."* Verification found a better one —
Harrison, N. "Reflections on the Amen Break: A Continued History, an Unsettled
Ethics", in *The Routledge Companion to Remix Studies*, 2nd ed., eds. Navas,
Gallagher & burrough, Routledge, 2025 — by the person whose 2004 work first
documented the break's history.

**Keep the hedge.** The chapter says "widely described as" the most sampled
recording, and that hedge is M001/S06's work. Harrison grounds the history; he
does not turn a reception claim into a counted fact. Do not un-hedge the
sentence while adding him.

1. **Write the failing cases**: `fr-harrison-2025` exists with a `data-tier`,
   and is cited at the Amen-break claim.
2. **Run and watch them fail.**
3. **Add the entry** at tier A and re-point the citation; the marker goes.
4. **Run, watch it pass, prove it bites**, restoring by inverse edit.
5. **Check.** All three tokens plus the site build.
6. **Commit** with `Rows: B07`, ticking Task 3, closing B07, appending evidence.

## Task 4 — Close the slice on a zero suppression count

1. **`grep -rn citation-tier-ok site/src/content/docs` must return nothing**,
   and the tier check's printed live-suppression count must be zero. That is the
   slice's second definition-of-done item and the only one that cannot be
   satisfied by any individual row.
2. **Tick both boxes** in the slice and this plan, tick Task 4, confirm all
   eight rows are `done`, set the slice `Status` to `done`.
3. **Run every token**, then `jk-standards ledger`.
4. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (each of the eight claims cites a Tier-A source, cites
the primary source it stood in for, or no longer makes such a claim) → Tasks 1,
2 and 3. Item 2 (zero suppressions) → Task 4 step 1.

**Row coverage.** B01–B05 and B17 → Task 1; B06 → Task 2; B07 → Task 3. Task 4
closes no row, which is why it carries `Rows: —`.

**Placeholder scan.** No TBDs. Every replacement is either named in the tree's
own markers or recorded in `M006-decisions.md` with what verification
established.

**Name consistency.** `fr-linn-attack-2020` and `fr-harrison-2025` are the two
new anchors, used identically throughout.

**Ordering.** Task 1 first because it is the bulk and needs no new entries.
Tasks 2 and 3 are independent of each other. Task 4 must be last: the zero count
is only true once all three have run.
