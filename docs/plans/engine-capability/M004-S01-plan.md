---
class: gated
---

# M004/S01 — Session recall

**Slice:** `M004/S01` in `docs/plans/engine-capability/ledger.md`

Group 2 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [ ] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] A Cubase project saved with a non-default Poly patch reopens carrying that
      patch — edited steps, selected preset, and per-step micro-timing
- [ ] The spec has been shown to fail when the saved state is perturbed before
      reopening, so it is a round-trip check rather than a "did it load" check
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — The save/reopen spec, and a second Cubase launch

Creates `tests/cubase/e2e/session-recall.spec.ts` and
`tests/cubase/e2e/lib/session-contract.ts`; modifies
`.github/workflows/cubase-nightly.yml`.

1. Write `lib/session-contract.ts` first, with the round-trip's shape as data: the
   step index to toggle, the preset index to select, the per-step micro-timing
   value to set, and the path the spec writes its expected-state JSON to. Give it
   unit coverage under `cubase-harness` for the comparison function — the part
   that can be tested without a DAW is the part that decides pass or fail.
2. Write `session-recall.spec.ts`. It attaches over CDP exactly as
   `toggle-step.spec.ts` does, applies the three edits, writes the expected state
   to disk, and triggers Cubase's save. Reuse the CDP boilerplate from
   `toggle-step.spec.ts` rather than reinventing it; note in a comment that
   `127.0.0.1` is deliberate, as that file records.
3. Add the reopen arm to `.github/workflows/cubase-nightly.yml`: after the
   existing quit step, relaunch Cubase on the same fixture and run the spec a
   second time in assert mode, comparing the reloaded patch against the JSON the
   first pass wrote. The two passes are the same spec under a mode variable, so
   the contract cannot drift between them.
4. Implement `POLY_E2E_MUTATE=s01-perturb-state`: before the reopen, the spec
   rewrites one field of the *expected-state JSON* it wrote. The assertion must
   then fail. Perturb the expectation file, never the plugin's saved state — the
   knob must not be able to change what Poly does.
5. Run `format` and `cubase-harness`. Commit; the row stays open until a nightly
   names it.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S01.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S01`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
