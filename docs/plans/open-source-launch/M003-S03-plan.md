# M003/S03 — Logic is supported or declined on purpose

**Slice:** M003/S03 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS17 (the AU is `aumu`, built on every PR, shipped by no release,
and Logic routes MIDI only from an `aumi`)
**Depends:** nothing.
**Decisions consumed:** `M003-decisions.md`, 2026-09-29 — the feasibility
check is recorded there; the owner declined Logic for the first release; S03
touches Logic's lines only, and the guide's AU install prose stays for OS41.

## Task status

- [x] Task 1 — Logic is declined in the README, the guide and the AU build's
      own comments, with the feasibility finding recorded; the slice closes
      (OS17)

## Definition of Done

- [x] The owner's decision is recorded in this milestone's decisions file and
      cited by M007/S01
- [x] If supported: an `aumi` unit passes `auval`, drives an instrument track in
      Logic (evidence), carries the real version, and the release builds and
      ships it — n/a, declined
- [x] If declined: the README and guide say Logic is not supported, and
      `build-au-macos` carries a comment saying it exists for validation only

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

The task's own check, run first: `node --test site/tests/host-table.test.mjs`
(the Logic assertions are added to it).

## Task 1 — Logic is declined in the README, the guide and the AU build's own comments, with the feasibility finding recorded; the slice closes (OS17)

**Files:** modify `README.md`, `site/src/content/docs/guide-using-poly.mdx`,
`site/tests/host-table.test.mjs`, `.github/workflows/ci.yml`,
`plugin/CMakeLists.txt`, `docs/plans/open-source-launch/ledger.md`; create
`docs/plans/open-source-launch/evidence/M003-S03.md`.

1. Add two tests to `host-table.test.mjs`:
   - `the guide does not list Logic as a host that loads Poly`: in
     `guide-using-poly.mdx`, no line under the `### Load Poly` list begins
     `- **Logic Pro:**`, and no `**Logic Pro:**` recording heading remains.
   - `README and guide say Logic is not supported`: `README.md` matches
     `/Logic Pro[^.\n]*not supported/i` and `guide-using-poly.mdx` matches
     the same.
2. Run them: both red.
3. `README.md`: below the host table's sentence, add: "**Logic Pro is not
   supported.** Logic routes generated MIDI only from a MIDI-FX Audio Unit,
   and Poly's Audio Unit is an instrument built by the VST3 SDK's wrapper,
   which cannot produce one; the Audio Unit is built in CI for validation and
   is not part of any release."
4. `guide-using-poly.mdx`: in `### Load Poly`, replace the Logic Pro bullet
   with `- **Logic Pro:** not supported — see the note below the list`, and
   add that note after the list: the same two sentences as the README, plus
   that a future milestone revisits Logic with a MIDI-FX wrapper. Under the
   recording steps, remove the `**Logic Pro:**` block and its steps; do not
   touch the "two formats", `Poly.component` copy or `auval` prose, which
   OS41 rewrites.
5. `.github/workflows/ci.yml`, above `build-au-macos:`: a comment that the
   AU exists for validation only (OS17, declined 2026-09-29), is shipped by
   no release, and that M007 packages the VST3 alone.
   `plugin/CMakeLists.txt`: extend the AU target's header comment with the
   same sentence and a pointer to `M003-decisions.md`.
6. Run the two tests: green. Run
   `node --test scripts/check-release-workflow.mjs`: still 32 of 32.
7. Run `format`, `doc-discipline`, `guards`. Write the evidence: the
   feasibility finding (the wrapper's base class, the allowed types, the
   MIDI output callback), the decision, what each file now says, and the
   DoD-to-task table with the "if supported" arm marked not applicable. Tick
   the task box; tick the first and third DoD boxes and, for the second,
   tick it with "n/a — declined" appended, here and in the ledger; set the
   slice `done`; OS17 `done`. Run `jk-standards ledger`. Commit with
   `Rows: OS17`.

## Self-review

| DoD | Task |
|---|---|
| The decision is recorded and M007/S01 cites it | 1 — `M003-decisions.md`, 2026-09-29 entry; M007/S01's plan will cite it when planned |
| If supported … | not applicable: declined |
| If declined: README and guide say so; `build-au-macos` says validation only | 1 |

| Row | Task | Verification produced |
|---|---|---|
| OS17 | 1 | feasibility recorded first; the declined arm's DoD, not both; the comment and the two docs, held by tests |

Names used throughout: `host-table.test.mjs`, `### Load Poly`. No placeholders.
