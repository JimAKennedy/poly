# M004/S01 — The repository accepts a stranger

**Slice:** M004/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS18 (already done), OS19 (empty About panel), OS20 (roadmap
enumerates closed issue numbers)
**Depends:** nothing.
**Decisions consumed:** `M004-decisions.md`, 2026-09-29 — the positioning
sentence; API access to the About panel; label queries and ledgers in place
of milestone queries; the `check-front-door.mjs` guard.

## Task status

- [x] Task 1 — The About panel carries the sentence, the site URL and the
      topics (OS19)
- [ ] Task 2 — The roadmap names themes and links queries, with no issue
      number left to go stale (OS20); the slice closes

## Definition of Done

- [ ] A non-collaborator account can open an issue from each template
- [ ] The About panel carries a description, the site URL and topics
- [ ] `ROADMAP.md` links to issues by label and milestone rather than
      enumerating numbers, so closing an issue cannot make it stale

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

Also run, as the tasks' own checks: `node --test scripts/check-front-door.mjs`
and `bash scripts/check-guards.sh`.

## Task 1 — The About panel carries the sentence, the site URL and the topics (OS19)

**Files:** create `docs/plans/open-source-launch/evidence/M004-S01.md`.
Repository settings, through the API.

1. Read the panel: `gh api repos/JimAKennedy/poly --jq '{description, homepage, topics}'`.
   Record the empty state.
2. Set it:
   ```bash
   gh api -X PATCH repos/JimAKennedy/poly \
     -f description="Poly is a free, open-source polymetric drum sequencer for your DAW: grooves grounded in real drumming traditions, a guide that cites where every preset comes from, deterministic output, and an engine that runs in your browser." \
     -f homepage="https://poly.jk.digital"
   gh api -X PUT repos/JimAKennedy/poly/topics \
     -f 'names[]=vst3' -f 'names[]=midi' -f 'names[]=euclidean-rhythm' \
     -f 'names[]=polyrhythm' -f 'names[]=drum-machine' -f 'names[]=audio-plugin'
   ```
3. Read the panel again and record the three values verbatim in the
   evidence. The sentence must match the decisions file character for
   character.
4. Run `format` and `doc-discipline` (the evidence file is the only tree
   change). Tick the box. Commit with `Rows: OS19`; OS19 `done`.

## Task 2 — The roadmap names themes and links queries, with no issue number left to go stale (OS20); the slice closes

**Files:** modify `ROADMAP.md`, `scripts/check-guards.sh`, `scripts/README.md`,
`docs/plans/open-source-launch/ledger.md`; create
`scripts/check-front-door.mjs`.

1. Write `scripts/check-front-door.mjs` in the style of
   `check-version-source.mjs`, with one test for now:
   `ROADMAP.md enumerates no issue numbers` — `/\[#\d+\]|\bissues\/\d+/`
   must not match. Run it: red (four numbers today).
2. Rewrite `ROADMAP.md`. Keep the opening, the "How this fits together"
   list and the closing "Finding something to work on" section, then three
   theme sections with no tables:
   - **Keep the build green** — links the open issues labelled `bug`,
     `sanitizer-failure` and `cubase-nightly-failure`, each as a query URL of
     the form `https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A<label>`.
   - **Documentation and onboarding** — links `documentation` and
     `good first issue`.
   - **Rhythm engine and musicality** — links `enhancement`, and says the
     design-led work is planned in the delivery ledgers under `docs/plans/`,
     linking that directory.
   Replace "tracked as GitHub milestones" in the opening list with: release
   work is planned in the delivery ledgers under `docs/plans/`, each slice
   with its definition of done, and shipped work is in the changelog. No
   `#NNN` anywhere.
3. Run the guard: green. Wire it: `run_guard "front-door contract" node --test scripts/check-front-door.mjs`
   after the `workflow-hygiene contract` line in `check-guards.sh`, with a
   comment naming OS20 and OS22; a bullet in `scripts/README.md`. Run
   `bash scripts/check-scripts-readme.sh` and `bash scripts/check-guards.sh`:
   green.
4. `CLAUDE.md` and `README.md` say the active roadmap is GitHub milestones
   plus the changelog; leave them — `README.md` is OS22's and `CLAUDE.md`
   OS23's, both in S03, which correct the sentence there.
5. Run `format` and `doc-discipline`. Tick every DoD box here and in the
   ledger (OS18's box was satisfied on 2026-09-23 per the row); set the
   slice `done`; OS20 `done`. Append evidence with the DoD-to-task table.
   Run `jk-standards ledger`. Commit with `Rows: OS20`.

## Self-review

| DoD | Task |
|---|---|
| A non-collaborator can open an issue from each template | OS18, closed on 2026-09-23 in the ledger: the setting reads "All users" |
| The About panel carries a description, the site URL and topics | 1 |
| `ROADMAP.md` links by label and milestone rather than enumerating numbers | 2 — labels and the ledgers; no milestone exists to query, recorded in the decisions file |

| Row | Task | Verification produced |
|---|---|---|
| OS19 | 1 | the read-back in the evidence: the sentence, `poly.jk.digital`, the six topics |
| OS20 | 2 | no `#NNN` remains, held by the guard; themes link label queries |

No placeholders.
