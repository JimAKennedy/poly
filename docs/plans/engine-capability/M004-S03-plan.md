---
class: gated
---

# M004/S03 — Transport motion

**Slice:** `M004/S03` in `docs/plans/engine-capability/ledger.md`

Group 1 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [x] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] The spec locates the transport backwards and forwards mid-playback, loops
      a range, and changes tempo, and asserts the emitted notes at those
      positions match the same positions played linearly
- [ ] The spec has been shown to fail against a lane whose phase is accumulated
      rather than derived from absolute PPQ
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — Locate, loop and tempo against the linear capture

Creates `tests/cubase/e2e/transport-motion.spec.ts` and
`tests/cubase/e2e/lib/transport-contract.ts`; modifies
`tests/cubase/driver/play_scenario.py` and
`.github/workflows/cubase-nightly.yml`.

1. Extend `play_scenario.py` with the three motions this slice needs — locate to
   a PPQ mid-playback, set a loop range and let it wrap, and change tempo —
   driven from a scenario file rather than hard-coded, so the spec describes the
   motion and the driver performs it.
2. Write `lib/transport-contract.ts`: the passage to play, the PPQ positions to
   compare, and the comparison itself — given a linear capture and a motion
   capture, return the positions whose emitted notes differ. Unit-test that
   comparison under `cubase-harness` against fixture captures, red and green.
   This is the slice's real logic and it does not need a DAW to be proved.
3. Write `transport-motion.spec.ts`: play the passage linearly, capture; play it
   with the motions, capture; compare at the contract's positions. A note whose
   phase was accumulated rather than derived from absolute PPQ diverges after
   the first locate, which is exactly what the comparison reports.
4. Implement `POLY_E2E_MUTATE=s03-accumulated-phase`: the spec shifts the motion
   capture's timestamps by the elapsed offset a naive accumulator would
   introduce, and the comparison must then fail. This proves the comparison
   catches the defect the row names without requiring an accumulating build.
5. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S03.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S03`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
