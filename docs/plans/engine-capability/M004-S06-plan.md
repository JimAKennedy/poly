---
class: gated
---

# M004/S06 — Offline bounce equivalence

**Slice:** `M004/S06` in `docs/plans/engine-capability/ledger.md`

Group 2 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [ ] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] A bounced or offline-rendered passage matches the realtime capture of the
      same passage, note for note and position for position
- [ ] The spec has been shown to fail when the two diverge
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — Bounce and realtime, compared

Creates `tests/cubase/e2e/offline-bounce.spec.ts` and
`tests/cubase/e2e/lib/bounce-contract.ts`; modifies
`.github/workflows/cubase-nightly.yml`.

1. Write `lib/bounce-contract.ts`: the passage, and the comparison between two
   captures — realtime and offline — as a pure function returning the positions
   that differ. Unit-test it under `cubase-harness` against fixture captures,
   red and green. As in S03, the comparison is the logic and it needs no DAW.
2. Write the spec: play the passage in realtime and capture; trigger Cubase's
   offline export of the same range and capture that; compare. The existing
   `export-midi.spec.ts` already drives an export from the WebUI — reuse its
   route to the export control rather than finding a second one.
3. Note in a comment what the comparison tolerates and why. A bounce and a
   realtime pass can differ in absolute start offset without differing
   musically; a tolerance that is not written down becomes a tolerance nobody
   can audit later.
4. Implement `POLY_E2E_MUTATE=s06-perturb-capture`: shift one note in the
   offline capture, and the comparison must fail naming its position.
5. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S06.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S06`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
