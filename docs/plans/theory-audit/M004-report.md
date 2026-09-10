# M004 — Patch and Theory Consistency: review report

Every chapter patch either follows its companion theory page's named rules or
carries an in-band note explaining why it deliberately does not, and CI enforces
the choice.

Ledger: `docs/plans/theory-audit/ledger.md`
Branch: `milestone/M004-patch-theory-consistency`
Generated at the `/jk:auto` review gate. `/jk:ship` is a separate, deliberate
act taken after reading this.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S05 | Named-rule conformance checklist | F42 | done |
| M004/S01 | Chapter 2 patch | F37 | done |
| M004/S02 | Chapter 3 patch | F38, F39, F40, B16 | done |
| M004/S03 | Chapter 5 patch | F41 | done |
| M004/S04 | Tihai worked example | F43 | done |

Listed in execution order, not slice number. **S05 was reordered to run first**
during planning: it builds the checklist and the divergence-marker contract that
F37, F40 and F41 all name as their verification, yet it declared `Depends` on
S01–S04. With S05 last, the escape hatch would have been reached for before it
was designed.

Eight rows: seven audit findings and one defect the programme found in itself
(B16). Every slice is `done`, which is the stricter ledger claim.

## Definition of done

All fourteen boxes across the five slices are checked.

- **S05** (5) — the checklist is declared data covering five patches; a marker
  satisfies one rule and nothing else; a silently dropped rule fails; every rule
  proved to fail when its lane is removed; M007 named as owner of the rest
- **S01** (2) — the Ewe patch carries the dance-beat lane; the choice is legible
  from the table
- **S02** (3) — the clave lane names its approximation and links the exact
  construction; the tumbao's onsets are rendered; the variation budget satisfies
  one-free-voice
- **S03** (2) — the Chapter 5 kotekan patch carries a pokok layer; it carries
  structural overlap at the cycle boundary
- **S04** (2) — the tihai formula is shown with real numbers; the arithmetic is
  checked, not asserted

## Validation

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 214 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 193 tests |
| `doc-discipline` | `jk-standards all` | exit 0 |
| `e2e` | `bash scripts/site-verify-local.sh` | exit 0, 13 stages, 27 chapters (S01, S02, S03) |
| `gate` | `bash scripts/pre-push-check.sh` | exit 0, 7 stages, 589 ctest tests (S05) |
| `ledger` | `jk-standards ledger` | conforms with every slice `done` |

The site suite grew from 198 to 214 and doc-conformance from 177 to 193.

## What the checklist found

The suite's nine pre-existing tests hand-write one predicate per known defect,
all passing — the 2026-07-30 conformance review's findings are remediated. The
checklist is declared data instead, so a page with no entry is a missing row
rather than an invisible absence.

**The first newly-checked rule found a tenth contradiction.**
`theory-electronic-breakbeat`'s own patch violates its Rule 7: the chopped kick
is E(5,16) at rotation 3, onsets `{3,6,9,12,15}` on a 16-pulse grid, and the
backbeat snare is `{4,12}`. Pulse 12 is shared, so the kick lands in a snare slot
on a page whose Rule 7 has it avoid them. Derived with the repo's own
`bjorklund` and `rotate`. It is row **B15** under M007/S02 and carries the one
remaining suppression.

Eleven theory pages state 92 numbered rules and nine were asserted. The other 83
have never been checked. **M007 — Named-rule coverage** now owns that, in three
slices: triage which of the 92 a lane table can settle (B12), roll out the
checkable ones (B13), and burn down the markers before they become permanent
(B14).

## Divergence markers

S05 created six. **Five were handoffs** — reasons naming the row that owed the
fix — and every one was removed by the slice that owned it, without anyone
hunting for stale ones. That is the property the marker was designed to have.

One remains: `ebb-kick-avoids-snare`, B15's genuine finding.

## Traceability

Twenty-three commits, measured against `origin/main`. **Every one carries a
`Slice:` trailer; there is no untraced work.**

- `16c99a6` docs(plans): run M004/S05 first and record the front-loaded decisions — M004/S05
- `74facd6` docs(plans): design M004/S05 and open M007 for the rule-coverage rollout — M004/S05
- `faed83d` plan(M004/S05): plan the named-rule conformance checklist — M004/S05
- `8d69ed7` test(site): address each PolyPatch by title, not by being first in the file — M004/S05
- `3095e55` test(site): add the named-rule checklist, and find a tenth rule contradiction — M004/S05
- `aa46845` test(site): check the Chapter 2, 3 and 5 patches against their pages' rules — M004/S05, rows F42
- `7388c24` docs(plans): close M004/S05 — the checklist its sibling slices are verified by — M004/S05
- `dd45673` plan(M004/S01): plan the Chapter 2 dance-beat lane — M004/S01
- `a9bb1d2` docs(site): lay the Ewe patch's dance beat and show the bell's timeline mode — M004/S01, rows F37
- `5b52506` docs(plans): close M004/S01 — Chapter 2's patch follows its page's construction — M004/S01
- `17d64ed` plan(M004/S02): plan the Chapter 3 patch's three rows — M004/S02
- `9bfa12f` revert(webui): drop the WASM artifacts M004/S01's close swept in — M004/S02
- `195a336` docs(site): name Chapter 3's clave lane the approximation it is — M004/S02, rows F38
- `df4090d` docs: record the e2e artifact hazard, and fix the guard's second [FH] blind spot — M004/S02, rows B16
- `d81661b` docs(site): print the tumbao's onsets instead of asking the reader to derive them — M004/S02, rows F39
- `c00dd2c` docs(site): give the son ensemble's variation budget to the quinto alone — M004/S02, rows F40
- `fe6cdaf` plan(M004/S03): plan Chapter 5's pokok layer, and correct the Rule 4 predicate — M004/S03
- `4b9d02d` test(site): check Rule 4 as the construction specifies it, not as it reads — M004/S03
- `9dee6ed` docs(site): give the Chapter 5 kotekan patch the pokok it elaborates — M004/S03, rows F41
- `7b9aa8f` plan(M004/S04): plan the tihai worked example — M004/S04
- `fa20bc0` docs(plans): close M004/S03 — Chapter 5's kotekan serves a melody — M004/S03
- `ae61738` docs(site): work the tihai's arithmetic instead of only stating it — M004/S04, rows F43
- `d01bf16` docs(plans): close M004/S04 — the tihai example is checked, not printed — M004/S04

## What a reviewer should look at twice

- **Two corrections were made to work already committed in this milestone.**
  F40's first amendment was wrong: it recorded the premise as non-existent after
  checking `03-afro-cuban.mdx`, the file the audit's `Lands in` named, when the
  finding lives on `theory-afro-cuban.mdx`. The audit was wrong about *where*,
  not about *what*. And `gam-structural-overlap`, a predicate S05 shipped, was
  weakened in S03 because it demanded pair-overlap that Poly's Kotekan L-mode
  cannot produce — it condemned `theory-gamelan`'s own reference patch. Both
  corrections are recorded in the rows and in the decisions file rather than
  quietly applied.
- **A rule was deliberately weakened.** `gam-structural-overlap` now checks
  construction step 4's remedy rather than Rule 4's headline. The argument is in
  `M004-S03-plan.md`; if it is wrong, that is where to argue with it.
- **The checklist does not guard cross-lane references.** Inserting Chapter 5's
  pokok lane renumbered polos, so the sangsih's `Kotekan L1` had to become `L2`.
  A stale lane number there would have pointed the strict complement at the
  pokok instead of the polos — a patch that still renders, still parses, and
  still passes every rule.
- **`e2e` rebuilds committed WASM artifacts non-reproducibly.** M004/S01's close
  swept `webui/poly_engine.{js,wasm}` in via `git add -A`; the `.wasm` moved 137
  bytes with no source change. Reverted, recorded as B16, opened as
  [#282](https://github.com/JimAKennedy/poly/issues/282), and documented in
  CLAUDE.md. Explicit path staging was used for the rest of the run.
- **A second `[FH]` blind spot** was found in
  `docs/audits/theory-audit-remediation.test.mjs`: Related-issues entries matched
  row references with `/[FH]\d{2}/`, so an entry citing a `B` row read as citing
  none. M003/S04 fixed the same defect in the table-row parser and did not reach
  this one.

## Left deliberately open

- **B15** — `theory-electronic-breakbeat`'s Rule 7 violation, under M007/S02.
  The fix is a rotation that clears the snare or a qualification to the rule, and
  that is a musical decision.
- **B16** — the WASM reproducibility, `accepted` here and owned by #282.
- **Chapter 2's table and its named preset agree on lane 1 only.**
  `polypatch-preset-resolution` establishes that the `preset=` attribute is a
  display caption resolving nothing at runtime, so it is not a defect in this
  milestone's path — but a reader loading `Sub-Saharan: Agbekor` gets something
  different from the printed table. Recorded in the decisions file, no row
  opened.
- **The milestone's own `Status`** is still `planned` while every slice is
  `done`. `/jk:close` sets it after the pull request merges.

## Decisions

Reproduced verbatim from `docs/plans/theory-audit/M004-decisions.md`.


Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

## 2026-09-10 — planning M004, front-loaded

- **Q:** M004/S05 builds the rule-conformance suite and the divergence-marker
  contract that S01–S03 name as their verification, but it declares `Depends` on
  S01–S04. Which way should the milestone run? — **A:** S05 first, machinery
  then chapters.
- **Decision:** S05's `Depends` drops the sibling slices and keeps M003/S03;
  S01–S03 gain `Depends: M004/S05`; S04 stays independent because the tihai
  worked example needs neither the checklist nor the marker — **Why:** with S05
  last, the escape hatch would be reached for before it was designed, and each
  chapter slice would assert ad hoc against an oracle that did not exist yet.
- **Q:** F40 says Chapter 3's patch has "conga at 10% mutation and quinto at
  30%" against the one-free-voice rule. The preset has `mutationRate` 0.00 on
  all five lanes and there is no quinto lane. How should the row be handled? —
  **A:** Close as already-satisfied, with evidence.
- **Decision:** F40's disposition changes from `patch-align` to `verify`; its
  `Item` records that the premise does not exist and names M070's preset
  conformance as the likely cause; it closes on a lock asserting the
  one-free-voice rule still holds — **Why:** a row that closes because someone
  looked and saw nothing wrong is indistinguishable from one nobody checked; a
  lock makes the claim falsifiable and catches a future regression.
- **Q:** F37's definition of done permits either carrying Chapter 2's missing
  dance-beat lane or cross-referencing the theory page's fuller construction in
  band. Which? — **A:** Add the dance-beat lane.
- **Decision:** The Ewe patch gains a low drum at 12 steps, 4 hits — E(4,12),
  every third pulse — and the table gains a `Timeline` column — **Why:** the
  milestone's Vision is that a patch follows its page's rules or says why not,
  and there is no reason this one should not follow them. The column is what
  makes the second definition-of-done item true, since the bell's timeline mode
  is already `true` in the preset but invisible in the table.
- **Decision:** F37's `Item` records that the audit's Rule 1 half is already
  satisfied — the `Ewe Polymetric Ensemble` preset carries `timeline: true` on
  its bell lane — **Why:** the row would otherwise licence work that is already
  done, and the remaining gap is narrower than the audit describes.
- **Q:** With the nine review defects already fixed, S05's real cost is how many
  of the 92 numbered rules the checklist covers. Which scope? — **A:** Create a
  new milestone for the comprehensive option, then proceed with the narrow scope
  under this slice.
- **Decision:** M007 — Named-rule coverage created, three slices, rows B12–B14,
  depending on M004/S05. S05's definition of done narrowed to the mechanism plus
  the five patches its siblings need and the two theory pages with no assertion,
  and gains an item naming M007 as the owner of the rest — **Why:** the DoD's
  literal reading is a tenfold expansion over 83 never-checked rules, which is a
  milestone rather than a slice; naming the owner in the DoD keeps the boundary
  explicit instead of implied by silence.
- **Q:** When the checklist finds a patch breaking a rule that no F-row covers,
  what should the unattended run do? — **A:** Mark it, open a B row, keep moving.
- **Decision:** an untriaged finding gets a `patch-divergence-ok` marker with the
  reason "found by the checklist, not yet triaged" and a B row naming the page
  and rule; M007/S03's B14 fails on any marker still carrying that placeholder —
  **Why:** the suite stays green and the finding stays greppable and tracked,
  without a slice widening to chase it; the placeholder check stops the backlog
  becoming permanent by inattention.
- **Finding, recorded rather than acted on:** the 2026-07-30 conformance
  review's nine defects are all fixed — the harness is green on all nine. The
  concern that S05 would open red on M069's work was wrong. The real exposure is
  the 83 rules never checked at all, which is what M007 owns.
- **Finding:** `theory-electronic-breakbeat` (9 rules) and `theory-minimalism`
  (8 rules) carry no assertion whatsoever. Folded into S05's scope rather than
  left for M007, because they are the only two pages with zero coverage.

## 2026-09-10 — executing M004/S05

- **Decision, under the pre-agreed policy:** `theory-electronic-breakbeat`'s
  patch violates its own Rule 7 — the kick's E(5,16) at rotation 3 puts an onset
  on pulse 12, one of the snare's two slots. Marked `patch-divergence-ok` with
  the untriaged reason and opened as ledger row B15 under M007/S02, rather than
  fixed here — **Why:** this is the "checklist finds something no F-row covers"
  case, and the fix is a musical choice between moving the rotation and
  qualifying the rule.
- **Judgment call:** Task 2's cases were written with the checklist data and its
  iteration together, so no case was watched to fail for lack of the mechanism —
  the plan's step 2 asked for that red. The mutation round is what establishes
  non-vacuity instead, and every rule was watched to fail on its own terms —
  **Why:** the ordering would have proved the harness ran, and the mutations
  prove each predicate bites, which is the property the definition of done
  actually names. Recorded rather than glossed.
- **Correction:** the plan's step 2 asserts the five seeded rules "hold today".
  Four do. `ebb-kick-avoids-snare` does not, which is how B15 was found.
- **Finding, recorded rather than chased:** the Chapter 2 Ewe table binds
  `preset="Sub-Saharan: Agbekor"`, but the preset's lanes (cowbell 12/7, conga
  5/4, tom 3/2, snare 7/4) agree with the table on lane 1 only — the table reads
  bell 12/7, support 12/3, responding 5/3, lead 7/5. `polypatch-preset-
  resolution.test.mjs` says the attribute "renders only as an italic display
  sub-title; it resolves nothing at runtime", so the contract is that the name
  resolves, not that the lanes match, and S01 is a doc edit rather than an
  engine change — **Why recorded:** a reader who loads the named preset gets
  something different from the printed table, which is a real conformance
  question. It is outside M004's rows and outside the checklist's remit, so it
  is noted here for a human rather than turned into work nobody asked for.
- **Decision:** S01 adds the dance-beat lane as lane 2 rather than appending it,
  renumbering the lanes below — **Why:** the theory page's construction lays
  bell, then dance beat, then supports, then lead, and a table that reads in
  construction order is the thing the reader is meant to follow.
- **Q:** The `e2e` token rebuilds the WASM and dirties `webui/poly_engine.{js,wasm}`
  on every run, non-reproducibly, and S02 and S03 both owe `e2e`. How should the
  rest of the run handle it? — **A:** Revert the artifacts after each `e2e` run.
- **Decision:** after each `e2e` run, restore both files from `origin/main` and
  stage explicit paths rather than `git add -A` — **Why:** a milestone that
  changes no engine source should not ship a 137-byte binary delta, and a
  reviewer cannot tell toolchain churn from a real change in a `.wasm` diff.
- **Q:** Should the non-reproducibility and the `git add -A` hazard be recorded
  beyond this conversation? — **A:** All three — a poly issue, a CLAUDE.md note,
  and a B row.
- **Decision:** issue [#282](https://github.com/JimAKennedy/poly/issues/282)
  opened, a "The e2e Gate Rewrites Committed Artifacts" section added to
  CLAUDE.md, and row B16 added to M004/S02 as `accepted` with #282 named as its
  owner — **Why:** the ledger records the finding, the issue owns the fix, and
  CLAUDE.md is where the next person will actually trip over it.
- **Mistake, recorded:** M004/S01's close commit swept `webui/poly_engine.{js,wasm}`
  in via `git add -A` after the `e2e` run. Reverted in its own commit. This is
  the second time `git add -A` has pulled unrelated files into a commit in this
  programme; explicit staging is now the rule for the rest of the run.
- **Finding:** `docs/audits/theory-audit-remediation.test.mjs` matched ledger row
  references in the Related-issues section with `/[FH]\d{2}/`, so an entry citing
  a `B` row read as citing no row at all. This is the same `[FH]` versus `[FHB]`
  defect M003/S04 fixed in the table-row parser, in a second place the earlier
  fix did not reach. Corrected to `/[FHB]\d{2}/` and proved both ways: the #282
  entry now resolves, and changing its citation to a non-existent `B99` still
  fails.
- **Correction to an earlier decision, and it was my error.** F40's premise is
  real. The first amendment recorded it as non-existent after checking
  `03-afro-cuban.mdx`, the file the audit's `Lands in` named. The finding lives
  on `theory-afro-cuban.mdx`, whose patch carries Cáscara 5%, Conga marcha 10%
  and Quinto 30% — three lanes against Rule 5's one. The audit was wrong about
  *where*, not about *what*.
- **Q:** How should Task 3 handle it? — **A:** Re-amend F40 to the theory page
  and fix the patch.
- **Q:** My earlier amendment is committed and wrong — how should the record
  handle it? — **A:** Correct the Item cell and say so in the commit.
- **Decision:** F40's `Lands in` moves to `theory-afro-cuban.mdx`, its `Item`
  states both the real finding and that the first amendment checked the wrong
  file, a new rule `ac-theory-one-free-voice` asserts at most one lane carries a
  non-zero Mutation, and the Cáscara and Conga marcha budgets are zeroed so the
  quinto alone holds it — **Why:** the ledger should end up true with the error
  still visible, rather than quietly right.
- **What should have caught it sooner:** the audit named a quinto in a *son*
  ensemble, and the theory page itself says the quinto is the free voice *in
  rumba*. That oddity was noticed and read as evidence the audit was wrong,
  when it was evidence the wrong table was being read. Trusting a row's
  `Lands in` over the rule it cites is the same defect class the audit keeps
  exhibiting, now reproduced from the other side.
- **Decision:** `ac-one-free-voice` on Chapter 3's preset is kept alongside the
  new theory-page rule — **Why:** it is correct as written and locks a side that
  currently complies only through M070's preset conformance.
- **Q:** `gam-structural-overlap` demands the kotekan pair share an onset, which
  Poly's Kotekan L-mode cannot produce, and Rule 4's own parenthetical says so.
  How should S03 resolve it? — **A:** Redefine it to match construction step 4.
- **Decision:** the rule becomes "a lane outside the kotekan pair strikes at the
  cycle boundary together with a pair lane" — **Why:** the predicate S05 shipped
  condemned `theory-gamelan`'s own reference patch, which uses `L6`, and Rule 4
  already discloses that L-mode implements the strict case "until kotekan modes
  ship". A rule that demands the impossible earns a suppression that can never
  be burned down, which is worse than an honest weaker rule.
- **Q:** Should the uncontested pokok half proceed regardless? — **A:** Yes, add
  the pokok lane.
- **Decision:** Chapter 5 gains a mid-register pokok lane, 8 steps, 4 hits, at
  1/4 against the pair's 1/16 — **Why:** it matches construction step 2 and the
  theory page's own `Pokok melody` lane, and it gives gamelan Rule 5's
  cross-reference a real pokok lane to point at, which M003/S05 deliberately
  left pointing at Construction because none existed.
