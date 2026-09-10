# M004/S02 — Chapter 3 patch

**Slice:** M004/S02, in `docs/plans/theory-audit/ledger.md`
**Depends:** M004/S05, `done` — its checklist is this slice's oracle
**Decisions:** `M004-decisions.md`

Three rows in three different places: a chapter patch header (F38), a theory
page's lane (F39), and a row whose premise turned out not to exist (F40).

## Task status

- [ ] Task 1 — Name the clave lane an approximation and link the exact construction
- [ ] Task 2 — Render the theory tumbao's onset positions
- [ ] Task 3 — Close F40 on its lock, and close the slice

## Definition of Done

- [ ] The Chapter 3 clave lane header marks itself as the Euclidean
      approximation and links to the exact-timeline construction
- [ ] The theory-page tumbao lane's onset positions are rendered, not left to
      the reader to derive
- [ ] The conga and quinto mutation settings either satisfy the one-free-voice
      rule or carry a divergence note

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `e2e` | `bash scripts/site-verify-local.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What is already true

- **F40's premise does not exist.** The `Cuban Son Montuno` preset carries
  `mutationRate` 0.00 on all five lanes, the table has no Mutation column, and
  there is no quinto lane — quinto is a rumba voice and this is a son ensemble.
  M070 conformed the factory presets after the audit was written. S05 already
  built `ac-one-free-voice` against the preset and it **passes**, so F40 closes
  on a lock rather than on someone having looked. Task 3 does that and changes
  no patch.
- Chapter 3 already teaches the seed-and-adjust workflow, under
  `## Son Clave, Rumba Clave, and Their Euclidean Neighbour`, and already links
  Chapter 18's timeline-mode UI. F38's cross-reference has a real target: it
  does not need writing, only pointing at.
- One marker stands above the Chapter 3 patch, `ac-clave-approximation`, naming
  F38. **Task 1 removes it** — that is the red for the task, not a tidy-up.

## Task 1 — Name the clave lane an approximation and link the exact construction

Closes **F38**.

**Files:** `site/src/content/docs/03-afro-cuban.mdx`

The chapter's clave lane header reads `Clave`. The theory page's reads
`Clave (timeline, exact)` with rotation `—`. The chapter's is E(5,16) at
rotation 0, which the chapter's own prose says differs from the son clave in
the fifth hit's position, 13 against 12. A reader comparing the two tables
cannot see that one is an approximation of the other.

1. **Remove the `ac-clave-approximation` marker** and run
   `node --test site/tests/theory-patch-conformance.test.mjs`. It must fail
   naming both arms: the header not marking itself an approximation, and the
   patch carrying no link.

2. **Rename the lane header** to name the approximation — it must contain
   `approx` for the rule's first arm, and read naturally beside the theory
   page's `Clave (timeline, exact)`.

3. **Add the cross-reference inside the patch block**, an internal link of the
   form `](/...)`, pointing at the chapter's own
   `#son-clave-rumba-clave-and-their-euclidean-neighbour` section, which is
   where the two-step workflow already lives. Confirm that anchor resolves by
   deriving it from the heading text rather than trusting this plan.

4. **Run and watch the rule pass.**

5. **Prove it still bites, both arms separately.** Revert the header to `Clave`
   → fails on the header arm; restore. Remove the link → fails on the link arm;
   restore. Both by inverse edit, never `git checkout --`, since the file
   carries uncommitted work. Confirm with `git diff --stat`.

6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

7. **Commit** with `Rows: F38`, ticking Task 1, setting F38 `done` naming
   `ac-clave-approximation` in its `Verification`, appending to
   `docs/plans/theory-audit/evidence/M004-S02.md`.

## Task 2 — Render the theory tumbao's onset positions

Closes **F39**.

**Files:** `site/src/content/docs/theory-afro-cuban.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

`theory-afro-cuban` Lane 2 is the tumbao at 16 steps, 6 hits, rotation 14. That
satisfies Rule 3 — the existing test `theory-afro-cuban: tumbao dodges beat one
and reaches bombo(3) and ponche(6)` already proves it — but rotation 14 is an
unusual spelling and the page asks the reader to take it on trust.

Derive the onsets rather than copying them: run `bjorklund(16, 6)` then
`rotate(..., 14)` from `site/src/lib/euclidean-claims.mjs`, the same shared
verifier the suite uses. As measured while planning they are `{1,3,6,9,11,14}`
— no onset at 0, and both bombo (3) and ponche (6) present — but re-derive and
use what the code returns.

1. **Write the failing case** as a new checklist rule on
   `theory-afro-cuban.mdx`, id `ac-tumbao-onsets-rendered`, asserting the
   patch's source window prints the tumbao's onset positions and that every
   number printed matches the derived set. Comparing against the derivation,
   not against a literal, is what stops the prose and the spelling drifting
   apart later.

2. **Run it and watch it fail** — nothing is printed yet.

3. **Render the onsets** in the page, beside the patch, in the page's existing
   voice. State that they avoid pulse 0 and land on bombo and ponche, which is
   Rule 3's content and is what makes rotation 14 legible rather than magic.

4. **Run and watch it pass.**

5. **Prove it bites.** Change one printed number, watch the case fail naming
   the mismatch, restore by inverse edit, confirm with `git diff --stat`.

6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

7. **Commit** with `Rows: F39`, ticking Task 2, setting F39 `done` naming
   `ac-tumbao-onsets-rendered` in its `Verification`, appending the evidence.

## Task 3 — Close F40 on its lock, and close the slice

Closes **F40**; closes the slice. **Changes no patch.**

**Files:** `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M004-S02.md`, this plan

1. **Confirm `ac-one-free-voice` passes** and carries no marker. Then prove it
   bites, because a row closing on a lock that cannot fail closes on nothing:
   raise two lanes' `mutationRate` in `site/src/generated/presets.json`, watch
   the rule fail naming them, and restore. That file is generated and carries
   no uncommitted work at this point, so `git checkout HEAD --` is safe for it
   — confirm with `git status --porcelain` afterwards.

2. **Close F40** as `done`, naming `ac-one-free-voice` in its `Verification`.
   Its `Item` already records that the premise does not exist; do not restate
   that in the commit as though it were discovered here.

3. **Confirm no marker names F38, F39 or F40.**
   `grep -rn patch-divergence-ok site/` must show the count down to three:
   `gam-pokok-layer`, `gam-structural-overlap`, `ebb-kick-avoids-snare`.

4. **Tick all three definition-of-done boxes** in the slice and this plan, tick
   Task 3, confirm F38, F39 and F40 are `done`, set the slice `Status` to
   `done`.

5. **Run every token**: `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, then
   `bash scripts/site-verify-local.sh` for `e2e`. Read exit codes.

6. **Run `jk-standards ledger`** with the slice `done`.

7. **Append the evidence** and **commit** with `Rows: F40`.

## Self-review

**DoD coverage.** Item 1 (clave header and link) → Task 1. Item 2 (tumbao
onsets rendered) → Task 2. Item 3 (one-free-voice satisfied or noted) → Task 3,
satisfied rather than noted, because the preset already complies.

**Row coverage.** F38 → Task 1 step 7, verification `ac-clave-approximation`,
which S05 built and step 5 proves still bites on both arms. F39 → Task 2 step
7, verification `ac-tumbao-onsets-rendered`, which Task 2 step 1 creates. F40 →
Task 3 step 2, verification `ac-one-free-voice`, proved to bite at step 1.

**Placeholder scan.** No TBDs. The onset set is named as measured but every
step says to re-derive it from `euclidean-claims.mjs` rather than copy it.

**Name consistency.** `ac-clave-approximation`, `ac-one-free-voice` and
`ac-tumbao-onsets-rendered` throughout; the first two are S05's ids, the third
is new in Task 2.

**Ordering.** Tasks 1 and 2 are independent and could swap. Task 3 must be last:
it closes the slice and runs `e2e` over the finished tree.
