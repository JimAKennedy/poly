# M004/S02 — One sentence says what Poly is

**Slice:** M004/S02 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS21 (the repository and the launch describe two different things)
**Depends:** nothing.
**Decisions consumed:** `M004-decisions.md`, 2026-09-29 — the sentence and
the alternatives not taken.

## Task status

- [ ] Task 1 — The sentence appears verbatim in the README, the site config,
      `CLAUDE.md` and the About panel, held by a test; the slice closes (OS21)

## Definition of Done

- [ ] The owner has chosen a one-sentence positioning statement and recorded it
      with the reasoning
- [ ] The README's opening, the site's meta description, the About description
      and `CLAUDE.md` carry the same statement

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `site-unit` | `npm --prefix site test` |

## Task 1 — The sentence appears verbatim in the README, the site config, `CLAUDE.md` and the About panel, held by a test; the slice closes (OS21)

**Files:** create `site/tests/positioning.test.mjs`; modify `README.md`,
`site/astro.config.mjs`, `CLAUDE.md`,
`docs/plans/open-source-launch/ledger.md`; create
`docs/plans/open-source-launch/evidence/M004-S02.md`.

1. Write `site/tests/positioning.test.mjs`: a `SENTENCE` constant equal to
   the decisions file's sentence; tests that `README.md`'s first
   non-heading, non-blank line is exactly the sentence; that
   `site/astro.config.mjs`'s `description:` value is exactly the sentence;
   that `CLAUDE.md`'s first non-heading, non-blank line is exactly the
   sentence. Run: three red.
2. `README.md`: replace line 3, "Polymetric drum pattern generator -- VST3
   instrument outputting MIDI.", with the sentence.
   `site/astro.config.mjs`: the `description:` string becomes the sentence.
   `CLAUDE.md`: its opening line under `# Poly`, "Polymetric drum pattern
   generator — VST3 instrument outputting MIDI.", becomes the sentence.
3. Run the test: green. Read the About description back with
   `gh api repos/JimAKennedy/poly --jq .description` and confirm it is the
   same string; record it in the evidence.
4. Run `format`, `doc-discipline`, `site-unit`. Tick the task box and both
   DoD boxes here and in the ledger; set the slice `done`; OS21 `done`.
   Evidence with the DoD-to-task table. Run `jk-standards ledger`. Commit
   with `Rows: OS21`.

## Self-review

| DoD | Task |
|---|---|
| The owner chose the statement and it is recorded with reasoning | the decisions file, 2026-09-29 |
| README opening, meta description, About and `CLAUDE.md` carry it | 1 — three held by the test, the fourth read back |

| Row | Task | Verification produced |
|---|---|---|
| OS21 | 1 | the sentence verbatim in all four surfaces; alternatives recorded |

No placeholders.
