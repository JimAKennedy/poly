---
class: gated
---

# M004/S07 — Host parameter automation

**Slice:** `M004/S07` in `docs/plans/engine-capability/ledger.md`

Group 1 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [ ] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] A host automation lane driving a Poly parameter changes the emitted MIDI
      at the automated positions
- [ ] The spec has been shown to fail when automation is ignored, and when it is
      applied at the wrong position
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — An automation lane that changes the output

Creates `tests/cubase/e2e/host-automation.spec.ts` and
`tests/cubase/e2e/lib/automation-contract.ts`; modifies
`tests/cubase/driver/play_scenario.py` and
`.github/workflows/cubase-nightly.yml`.

1. Choose the parameter and record why in the contract file: it must be one
   whose effect on emitted MIDI is unambiguous at a known PPQ position, so the
   assertion can be "the output changed here and not before" rather than "the
   output changed somewhere".
2. Extend `play_scenario.py` to write an automation lane for a named parameter
   over a PPQ range, then play it. The driver performs; the contract describes.
3. Write `lib/automation-contract.ts`: the parameter, the ramp, the position the
   change must first appear at, and the comparison. Unit-test the comparison
   under `cubase-harness` — including the case the definition of done names
   second, automation applied too early, which a naive "did anything change"
   assertion would pass.
4. Write the spec: play with the lane, capture, and assert the output differs
   from the un-automated capture at and after the position, and matches it
   before. Both halves are required; the row's verification says so.
5. Implement `POLY_E2E_MUTATE=s07-flatten-lane`: the spec writes a flat lane, so
   the "output changed at the position" assertion fails.
6. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S07.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S07`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
