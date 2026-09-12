# M007 — Named-rule coverage: review report

Every mechanically checkable rule a theory page states is checked against that
page's patch, so the guide's rules and its worked examples cannot drift apart
unnoticed.

Ledger: `docs/plans/theory-audit/ledger.md`
Branch: `milestone/M007-named-rule-coverage`
Generated at the `/jk:auto` review gate. `/jk:ship` is a separate, deliberate
act taken after reading this.

**This is the last milestone of the theory-audit programme.**

## The numbers

| | Before | After |
|---|---|---|
| Numbered rules with a recorded verdict | 0 of 92 | **92** |
| Rules checked against their patch | 16 | **42** |
| `patch-divergence-ok` markers | 1 | **0** |
| Site suite | 235 | **268** |

47 rules are checkable and all 47 are checked. 45 are not, and each carries a
specific reason for why not.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M007/S01 | Rule triage | B12 | done |
| M007/S02 | Roll out the checkable rules | B13, B15 | done |
| M007/S03 | Burn down the divergence markers | B14 | done |

## What the milestone actually established

**A rule whose predicate cannot be made to fail is not a checked rule.** Three
rules resolved that way, and all three were caught by the mutation round rather
than by reading the code:

- `theory-sub-saharan-africa` Rule 2 — every rotation in a static table is fixed
  by construction, so "every part has a fixed point of entry" cannot fail
- `theory-electronic-breakbeat` Rule 2 — forbids a layer *wandering between*
  territories, which is change over time; coincidence is not wandering
- `theory-gamelan` Rule 1 — under `Kotekan L`-mode the engine derives the
  complement, so the composite is complete by construction

Each reads as mechanical. Each would have shipped as a permanently green
assertion guarding nothing.

**No new patch contradiction surfaced across all 26 predicates.** M004/S05 found
one on the first rule it newly checked, so more were expected. The pages comply
with their own rules; what was missing was anything that would notice if they
stopped.

## Definition of done

All seven boxes across the three slices are checked.

## Validation

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 268 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0 |
| `gate` | `bash scripts/pre-push-check.sh` | exit 0, 7 stages, 589 ctest tests |
| `ledger` | `jk-standards ledger` | conforms with every slice `done` |

## Traceability

Ten commits, measured against `origin/main`. **Every one carries a `Slice:`
trailer; there is no untraced work.**

- `4bcad2f` plan(M007): plan the last milestone, scoped by a triage that does not exist yet — M007/S01
- `6e399c4` test(site): give every one of the 92 theory rules a verdict — M007/S01, rows B12
- `cf2df2b` docs(site): move the breakbeat kick off the snare instead of relaxing Rule 7 — M007/S02, rows B15
- `f5f44ac` test(site): check theory-afrobeat's five checkable rules — M007/S02
- `2a16749` test(site): stop an unregistered case passing as green, and re-triage against the tables — M007/S02
- `c9ff41e` test(site): check five more rules, and delete a sixth that misread its own rule — M007/S02
- `dab2a75` test(site): check balkan's cell grid, brazil's caixa, and minimalism's rotations — M007/S02
- `c43db24` test(site): finish the rollout, and drop a second predicate that could not fail — M007/S02, rows B13
- `200687c` docs(plans): close M007/S02 — every checkable rule is checked — M007/S02
- `f3fd1d6` test(site): forbid the untriaged placeholder, and close M007/S03 — M007/S03, rows B14

## What a reviewer should look at twice

- **A bug that nearly shipped inside the fix for itself.** The first page's
  block appended to `CHECKLIST` *after* the loop that registers a test per rule,
  so all five cases were silently skipped and the suite went 26 tests → 26,
  green. A rule that is not registered is indistinguishable from a rule that
  passes — precisely what this milestone exists to remove. A bidirectional guard
  now asserts every triage entry names a registered case and every registered
  case is named, and prints the remaining backlog.
- **The triage was re-checked against the tables, not just the rule text.** Six
  verdicts flipped because the rule names a value its own page's table does not
  carry — a Humanize bound where there is no Humanize column, a register
  comparison where there is no Note column.
- **`fs-the-one` needed a harder mutation than the rest**, and that is a fact
  about the patch. Rotating the kick off pulse 0 did not fail it, because the
  metronomic hat strikes all sixteen pulses and so always acknowledges the One.
- **B15 was a musical decision, taken by the user.** The breakbeat kick rotates
  from 3 to 13 — six rotations clear the snare, and r13 is the one that keeps the
  downbeat and leaves four of five onsets unmoved. Rule 7 was not relaxed to fit
  the patch; M004/S03's weakening of gamelan Rule 4 was allowed only because the
  engine genuinely could not express the strict reading.
- **The scope estimate was wrong twice, in both directions.** Reading headlines
  suggested 15–25 predicates; the triage read rule bodies and said 39; the table
  audit and three deletions brought it to 26. Both corrections are recorded in
  the decisions file.

## Decisions

Reproduced verbatim from `docs/plans/theory-audit/M007-decisions.md`.


Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

## 2026-09-12 — planning M007, front-loaded

- **Q:** B15 — `theory-electronic-breakbeat`'s kick is E(5,16) at rotation 3,
  landing on pulse 12, one of the backbeat snare's two slots, contradicting the
  page's own Rule 7. The row says the fix is a musical decision. Which? —
  **A:** Rotate the kick to r13.
- **Decision:** the kick becomes E(5,16) at rotation 13, onsets `{0,3,6,9,13}`
  — **Why:** six rotations clear both snare slots; r13 is the one that keeps an
  onset on pulse 0, so the pattern still starts on the downbeat a jungle kick
  usually does, and four of its five onsets are unchanged from the current
  spelling. Rule 7 stays as written and the page stops contradicting itself.
  Rejected: qualifying Rule 7 to permit the collision, which would weaken a rule
  to fit one patch. M004/S03 did weaken gamelan Rule 4, but only because Poly's
  Kotekan L-mode genuinely cannot express the strict reading; here the engine
  can, and a rotation is a one-cell edit.
- **Q:** B13 asks every checkable rule to get a checklist entry with a mutation
  proof. How deep should S02 go? — **A:** Every rule S01 triages as checkable.
- **Decision:** the rollout is bounded by S01's verdict rather than by a count
  chosen in advance — **Why:** a cap would leave a documented backlog inside the
  milestone whose purpose is closing one, and it is the triage that makes the
  scope defensible rather than arbitrary.

### Sizing, measured before planning

Eleven theory pages state 92 numbered rules. Reading the rule headlines, roughly
a third look settleable from a lane table — hit counts, rotations, velocities,
mutation, swing, subdivision, onset intersections. The rest are prose judgements
no predicate decides: "The One is sacred", "each part alone must be playable and
idiomatic", "declare the clave and its direction first", "arrangement is density
automation".

Fourteen rules are already covered — nine from the 2026-07-30 review's harness,
five added by M004/S05 — so S02's likely addition is fifteen to twenty-five.
That is an estimate from headlines; **S01's triage is the number that counts,**
and the plans are written to consume it rather than this paragraph.

## 2026-09-12 — executing M007

- **Q:** S01's triage found 39 predicates still to write, not the 15–25
  estimated when S02 was scoped as "every checkable rule". How should S02
  proceed? — **A:** All 39, committed page by page.
- **Decision:** the scope stands as decided; the rollout lands in page-sized
  commits so each is reviewable — **Why:** it is what B13 asks for and what makes
  the triage worth having, and a cap would leave a backlog inside the milestone
  meant to close the programme.
- **Why the estimate was low, recorded so the next estimate is better:** it came
  from reading rule headlines, and the triage read the bodies. Headlines mislead
  in both directions — `theory-balkan` Rule 7 reads "tight ensemble, minimal
  humanize" and ends "Humanize ≤ 0.15", while `theory-electronic-breakbeat` Rule
  5 reads "Arrangement is density automation" and concerns 8-, 16- and 32-bar
  boundaries no patch describes.
