---
class: gated
---

# M004/S02 — All seven files convert, and ASCII cannot return

**Slice:** guide-parity M004/S02 (row GP10)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Depends:** M004/S01
**Decisions consumed:** `M004-decisions.md` (2026-09-19)

## Task status

- [ ] 1. The guard, written against the tree as it is, failing on all seven files
- [ ] 2. Convert the site appendix
- [ ] 3. Convert `ARCHITECTURE.md` and `docs/engine-spec.md`
- [ ] 4. Convert `docs/euclidean-rhythm-guide.md`
- [ ] 5. Convert `docs/testing-strategy.md`, `docs/ui-guide.md` and `docs/webui-migration.md`
- [ ] 6. Exempt the frozen audit record and prove the hatch is honoured
- [ ] 7. Wire the guard in, and mutation-prove it
- [ ] 8. Evidence and slice close-out

## Definition of Done

Copied verbatim from the slice:

- [ ] No box-drawing characters remain in the seven files, replaced by Mermaid source, proved by the guard
- [ ] A check fails when ASCII box-drawing characters appear in a diagram position in any governed doc
- [ ] That check has been shown to fail by reintroducing one

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

---

## Task 1 — The guard, written against the tree as it is, failing on all seven files

**Produces:** `scripts/check-ascii-diagrams.mjs`, wired into `check-guards.sh`.

The guard is written **first**, while the ASCII is still there, so its initial
state is a genuine failure listing real files rather than a green check over an
already-clean tree. A guard first seen passing has never been seen working.

**Files:** create `scripts/check-ascii-diagrams.mjs`; modify
`scripts/check-guards.sh`, `scripts/README.md`.

**Steps:**

1. Write the guard. It walks the doc roots declared in `jk-standards.yaml`
   (`docs/`, `site/src/content/docs/`) plus `ARCHITECTURE.md`, skips the
   `exempt_dirs` (`docs/reviews/`, `docs/plans/`), and reports every line
   containing a character from the U+2500 box-drawing block.
2. Support one escape hatch: a line `<!-- boxdraw-ok: <reason> -->` anywhere in
   a file exempts that whole file, and the reason must be non-empty. Print the
   count of exempted files in the summary, so a rising count is visible.
3. Make the failure message teach the hatch — name the file, the line, and both
   remedies (convert it, or add the marker with a reason).
4. Run it. **Watch it fail**, listing the seven files. Record the exact count it
   reports; that number is the inventory this slice is measured against.
5. Add it to `scripts/README.md` — `check-scripts-readme.mjs` enforces that
   every script is documented, and M006 shipped red in CI on exactly this.
6. **Do not wire it into `scripts/check-guards.sh` yet.** Task 7 does that, once
   the tree is clean. Wiring it here would commit a red `guards` token, and
   `/jk:next` forbids a commit carrying a red gate. The guard being *seen*
   failing is the evidence that matters, and running it directly gives that
   without making the branch red.

**Check:** running `node scripts/check-ascii-diagrams.mjs` directly **fails**,
naming the seven files plus the audit record handled in task 6 — that failure,
with its file list and count, is what the evidence records. The `guards` token
itself stays green, because the script is not yet wired in.

---

## Task 2 — Convert the site appendix

**Consumes:** the guard from task 1 and the pipeline from S01.

**Files:** modify `site/src/content/docs/appendix-plugin-architecture.mdx`.

**Steps:**

1. Convert every remaining ASCII diagram in the file to a ```mermaid fence. S01
   already converted one; leave it as it is.
2. Preserve every label and arrow direction exactly. This is transcription. If
   an ASCII block is ambiguous about a relationship, **stop and ask** — do not
   invent an edge to make the diagram render.
3. Build the site (`npm --prefix site run build`) and confirm every diagram
   renders. Record the build-time delta with all diagrams present; S01's design
   deferred that measurement to this task rather than estimating it.
4. Run the guard and confirm this file no longer appears.

**Check:** `site-unit`, `doc-conformance` pass; the site builds; the guard's
remaining list is the six non-site files.

---

## Task 3 — Convert `ARCHITECTURE.md` and `docs/engine-spec.md`

**Files:** modify `ARCHITECTURE.md`, `docs/engine-spec.md`.

These render on GitHub, which draws ```mermaid fences at view time. No build
step applies to them.

**Steps:**

1. Convert `ARCHITECTURE.md`'s diagrams to ```mermaid fences.
2. Convert `docs/engine-spec.md`'s diagram. **Its `laneconfig` generated region
   spans a later part of the file and must not be touched** — confirm by
   locating the `<!-- BEGIN GENERATED: laneconfig -->` marker and checking the
   diagram sits outside it before editing.
3. Re-run `node scripts/generate-param-docs.mjs` and confirm it still finds its
   markers and produces no diff beyond what it always regenerates. A generator
   that can no longer find its region is the failure mode to watch for here.
4. Run the guard; these two files must drop off its list.

**Check:** `doc-conformance`, `doc-discipline` pass; `generate-param-docs.mjs`
runs clean; `git diff` shows no change inside the generated region.

---

## Task 4 — Convert `docs/euclidean-rhythm-guide.md`

**Files:** modify `docs/euclidean-rhythm-guide.md`.

The largest single file in the inventory. It gets its own task because it is
roughly a third of the total and a reviewer should be able to read its diff
alone.

**Steps:**

1. Convert its diagrams to ```mermaid fences.
2. Some of this file's box-drawing may be **rhythm-grid illustration rather than
   an architecture diagram** — a step grid drawn with `│` separators is not a
   flowchart and Mermaid is the wrong tool for it. Read each block before
   converting. If a block is a grid rather than a diagram, **stop and ask**:
   converting it to Mermaid would be worse than leaving it, and exempting it
   with the hatch may be the honest answer.
3. Run the guard; the file must drop off its list, or its remaining lines must
   be covered by a hatch with a stated reason.

**Check:** `doc-conformance`, `doc-discipline` pass.

---

## Task 5 — Convert the last three files

**Files:** modify `docs/testing-strategy.md`, `docs/ui-guide.md`,
`docs/webui-migration.md`.

**Steps:**

1. Convert each file's diagrams to ```mermaid fences, applying task 4's
   grid-versus-diagram judgement to each block.
2. Run the guard; all three must drop off its list.

**Check:** `doc-conformance`, `doc-discipline` pass.

---

## Task 6 — Exempt the frozen audit record and prove the hatch is honoured

**Files:** modify `docs/audits/M001-theory-audit-remediation-plan.md`; modify
`scripts/check-ascii-diagrams.mjs` only if the hatch does not already work.

**Steps:**

1. Add `<!-- boxdraw-ok: dated remediation record, frozen; the same class as
   docs/reviews/, which is exempt structurally -->` to the audit file.
2. Run the guard and confirm the file is reported as exempted, not as passing
   silently — the summary must count it.
3. **Prove the reason field is load-bearing:** empty the reason, run the guard,
   and confirm it fails; restore the reason by an explicit edit, not by
   `git checkout`, and confirm it passes. Record both outcomes.

**Check:** `guards` passes with exactly one exempted file, counted in the output.

---

## Task 7 — Wire the guard in, and mutation-prove it

**Files:** modify `scripts/check-guards.sh`. Nothing else permanently — the rest
of this task's product is evidence.

**Steps:**

0. With every file converted and the guard passing when run directly, wire it
   into `scripts/check-guards.sh` alongside the existing guards. `guards` must
   be green immediately after wiring; if it is not, a conversion was missed and
   that is the thing to fix, not the wiring.
1. Insert a single `┌` into one converted file.
2. Run `bash scripts/check-guards.sh` and confirm it **fails**, naming that file
   and that line.
3. Remove the character **by an explicit edit**, not `git checkout` — the tree
   contains uncommitted work from earlier tasks and a checkout would discard it.
4. Confirm `guards` is green again and `git diff` is empty for that file.
5. Record the mutation, the failure message, and the restoration in the evidence.

**Check:** `guards` fails under mutation and passes after restoration. A guard
that cannot be shown to fail is not evidence of anything.

---

## Task 8 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M004-S02.md`; modify the
ledger.

**Steps:**

1. Append one entry per task: token, exit code, headline counts, date. No commit
   SHAs.
2. Record the **measured** final inventory: the guard's count before task 2 and
   after task 5, and the number of exempted files.
3. Record the build-time delta from task 2.
4. Record any block left as ASCII with a hatch, and why — if task 4 or 5 found a
   rhythm grid, that decision belongs here and in `M004-decisions.md`.
5. Set row GP10 to `done` and slice M004/S02 to `done`, ticking all three DoD
   boxes.
6. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done`.
