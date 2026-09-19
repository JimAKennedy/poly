---
class: gated
---

# M004/S02 — All seven files convert, and ASCII cannot return

**Slice:** guide-parity M004/S02 (row GP10)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Depends:** M004/S01
**Decisions consumed:** `M004-decisions.md` (2026-09-19)

## Task status

- [x] 1. Convert the site appendix
- [ ] 2. Convert `ARCHITECTURE.md` and `docs/engine-spec.md`
- [ ] 3. Exempt `docs/euclidean-rhythm-guide.md`'s UI wireframe
- [ ] 4. `docs/testing-strategy.md`, `docs/webui-migration.md` convert; `docs/ui-guide.md` is exempted
- [ ] 5. The guard, wired and green, with the frozen audit record exempted
- [ ] 6. Mutation-prove the guard on every arm
- [ ] 7. Evidence and slice close-out

**Re-decomposed mid-slice.** The original order wrote the guard first, so it
could be seen failing against the un-converted tree. That order is impossible
here: `doc-conformance`'s own coverage check — M007/S03's *every repo guard is
reachable from a command a developer can run* — fails the moment a guard script
exists that no command runs, and wiring it before the conversions would make
`guards` red instead. Both routes commit a red gate, which `/jk:next` forbids.

The guard was still **written and run against the full ASCII tree** before any
conversion; that output is quoted in the evidence. What moved is where it is
committed, not whether it was seen failing. Task 6 then proves it bites by
reintroducing a character, which is what the definition of done actually asks
for and is the stronger claim.

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

## Task 1 — Convert the site appendix

**Files:** modify `site/src/content/docs/appendix-plugin-architecture.mdx`.

**Steps:**

1. The only block left is the `poly/` **directory tree**, not a diagram.
   Convert it to a nested Markdown list: path in code style, trailing comment as
   prose. Do not force it into a flowchart. M004/S01 already converted the
   System Overview; leave that as it is.
2. Preserve every label and arrow direction exactly. This is transcription. If
   an ASCII block is ambiguous about a relationship, express the ambiguity —
   point at a subgraph rather than one of its members — rather than inventing an
   edge to make the diagram render.
3. Build the site (`npm --prefix site run build`) and confirm every diagram
   renders, by finding its labels in the built HTML rather than trusting a green
   test. Record the build time with all diagrams present.
4. Run the parked guard from `scratchpad/check-ascii-diagrams.mjs` and confirm
   this file no longer appears.

**Check:** `format`, `site-unit`, `doc-conformance`, `doc-discipline`, `guards`
all pass; the site builds; the file is off the guard's list.

---

## Task 2 — Convert `ARCHITECTURE.md` and `docs/engine-spec.md`

**Files:** modify `ARCHITECTURE.md`, `docs/engine-spec.md`.

These render on GitHub, which draws ```mermaid fences at view time. No build
step applies to them.

**Steps:**

1. Convert `ARCHITECTURE.md`'s diagrams to ```mermaid fences.
2. Convert `docs/engine-spec.md`'s diagram. **Its `laneconfig` generated region
   must not be touched** — locate the `<!-- BEGIN GENERATED: laneconfig -->`
   marker and confirm the diagram sits outside it before editing.
3. Re-run `node scripts/generate-param-docs.mjs` and confirm it still finds its
   markers and leaves the generated region unchanged. A generator that can no
   longer find its region is the failure mode to watch for.
4. Run the parked guard; both files must drop off its list.

**Check:** the validation set passes; `generate-param-docs.mjs` runs clean;
`git diff` shows no change inside the generated region.

---

## Task 3 — Exempt `docs/euclidean-rhythm-guide.md`'s UI wireframe

**Files:** modify `docs/euclidean-rhythm-guide.md` (one added comment line).



**Steps:**

This file's single block is a 44-line **annotated wireframe of the plugin
window** — nested panels, knob positions, lane tabs, `← A:` callouts. It is not
an architecture diagram and no flowchart can express it.

1. Add the exemption marker at the top of the file, naming the reason:
   `<!-- boxdraw-ok: annotated UI mockup of the plugin window, not an
   architecture diagram; mermaid cannot express a panel layout -->`.
2. **Do not convert the wireframe.** Leave it exactly as it is.
3. Run the parked guard; the file must appear in the *exempted* list with its
   reason, not in the findings.

**Check:** the validation set passes.

---

## Task 4 — The last three files, each treated on its merits

**Files:** modify `docs/testing-strategy.md`, `docs/ui-guide.md`,
`docs/webui-migration.md`.

**Steps:**

These three differ, and each needs its own treatment:

1. `docs/testing-strategy.md` holds **two** blocks: the L1–L4 test pyramid,
   which is a genuine diagram and becomes a ```mermaid fence; and the
   `webui/tests/fixtures/` **directory tree**, which becomes a nested Markdown
   list as in task 1.
2. `docs/webui-migration.md` holds a **dependency graph** (W1 → W2 → W3 → W6
   with two branches). Convert to a ```mermaid fence, preserving both branches.
3. `docs/ui-guide.md` is a **UI wireframe** drawn mostly in plain `+--+` ASCII,
   with 5 lines using box-drawing for nested visualization panels. Exempt it
   with the same reason as task 3; do not convert it.
4. Run the parked guard; `testing-strategy.md` and `webui-migration.md` must
   drop off the findings, and `ui-guide.md` must appear as exempted.

**Check:** the validation set passes.

---

## Task 5 — The guard, wired and green, with the frozen audit record exempted

**Consumes:** a tree with no ASCII diagrams left in the seven files.
**Produces:** `scripts/check-ascii-diagrams.mjs`, wired into
`scripts/check-guards.sh` and documented in `scripts/README.md`.

The guard, its wiring, its README entry and the one exemption land together.
They have to: `doc-conformance`'s coverage check fails on a guard script no
command runs, and `guards` fails on a wired guard with work left to do, so any
smaller commit carries a red gate.

**Files:** create `scripts/check-ascii-diagrams.mjs` (restore from
`scratchpad/check-ascii-diagrams.mjs`); modify `scripts/check-guards.sh`,
`scripts/README.md`, `docs/audits/M001-theory-audit-remediation-plan.md`.

**Steps:**

1. Restore the guard script and add its `scripts/README.md` entry —
   `check-scripts-readme.mjs` enforces that every script is documented, and M006
   shipped red in CI on exactly this.
2. Add `<!-- boxdraw-ok: dated remediation record, frozen; the same class as
   docs/reviews/, which is exempt structurally -->` to the audit file.
3. Wire the guard into `scripts/check-guards.sh` alongside the existing guards.
4. Run `bash scripts/check-guards.sh`. It must be **green**, and the guard's own
   output must report exactly **three** exempted files — the two UI wireframes
   and the frozen audit record — each named with its reason. If it is
   red, a conversion was missed — fix the conversion, not the guard.
5. Run the full validation set, including `doc-conformance`, whose coverage
   check is what forced this ordering.

**Check:** the whole validation set passes with the guard wired in.

---

## Task 6 — Mutation-prove the guard on every arm

**Files:** none permanently. This task's product is evidence.

Four arms, each reverted by an **explicit edit** rather than `git checkout` —
the tree carries uncommitted work and a checkout would discard it.

**Steps:**

1. **It detects ASCII.** Insert a single box-drawing character into one
   converted file. `guards` must fail, naming that file and line. Remove it.
2. **The hatch works.** Confirm the exempted audit file is reported as exempted
   and counted, not passing silently.
3. **An empty reason fails.** Empty the hatch's reason in the audit file.
   `guards` must fail saying the marker carries no reason. Restore the reason.
4. **It cannot go vacuous.** Break the config parse so no doc roots are found.
   The guard must refuse to report a pass rather than scanning nothing — the
   failure mode `CLAUDE.md` records for the jk-standards rules that passed while
   matching nothing. Restore the parse.
5. After each restoration confirm `git diff` is clean for the touched file.

**Check:** all four arms fail under mutation and pass after restoration.

---

## Task 7 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M004-S02.md`; modify the
ledger.

**Steps:**

1. Append one entry per task: token, exit code, headline counts, date. No commit
   SHAs.
2. Record the **measured** inventory: the guard's count before any conversion
   (from the first run, quoted) and after, and the number of exempted files.
3. Record the build-time delta from task 1.
4. Record any block left as ASCII under a hatch, and why.
5. Set row GP10 to `done` and slice M004/S02 to `done`, ticking all three DoD
   boxes.
6. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done`.
