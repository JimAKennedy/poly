---
class: gated
---

# M004/S02 — Preset recall across all 45

**Slice:** `M004/S02` in `docs/plans/engine-capability/ledger.md`

Group 1 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [x] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] Every one of the 45 factory presets is selected in a running Cubase
      instance, and each loads without crashing the host
- [ ] For each preset the spec asserts the lane count and note numbers against
      `site/src/generated/presets.json`, so a preset that loads wrongly fails
      rather than merely not crashing
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — Iterate every preset and assert against `presets.json`

Creates `tests/cubase/e2e/preset-recall.spec.ts` and
`tests/cubase/e2e/lib/preset-contract.ts`; modifies
`.github/workflows/cubase-nightly.yml`.

1. Write `lib/preset-contract.ts` first: load `site/src/generated/presets.json`,
   and expose for each preset its index, name, active lane count, and the lane
   note numbers. Assert in a `cubase-harness` unit test that it reads 45 presets
   and that every entry has a name and at least one lane — a contract built from
   a file that silently changed shape is worse than none.
2. Write `preset-recall.spec.ts`: attach over CDP, and for each index in turn
   select the preset in the shipping WebUI, wait for the editor to settle, and
   read back the lane count and note numbers. Compare against the contract.
   Collect every mismatch and fail once with all of them, rather than at the
   first — a run that takes a Cubase launch to produce should report everything
   it found.
3. The host surviving is itself an assertion: if the page or the CDP connection
   dies mid-iteration, fail naming the index that was being selected. That is
   the crash `kWebPresetLaneNames` once caused, and the index is the only useful
   thing to know about it.
4. Implement `POLY_E2E_MUTATE=s02-malformed-preset`: the contract reports a
   deliberately wrong lane count for one index, so the comparison fails naming
   it. The mutation is in the expectation, not in `web_ui_view.cpp`.
5. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S02.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S02`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
