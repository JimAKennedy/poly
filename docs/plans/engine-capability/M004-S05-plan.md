---
class: gated
---

# M004/S05 — Multiple instances

**Slice:** `M004/S05` in `docs/plans/engine-capability/ledger.md`

Group 2 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [ ] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] Two Poly instances in one project each hold their own patch and emit their
      own MIDI, with no state or probe output crossing between them
- [ ] The spec has been shown to fail if the two instances share state
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — Two instances, two patches, two streams

Creates `tests/cubase/e2e/multi-instance.spec.ts` and a two-instance fixture
under `tests/cubase/fixtures/`; modifies
`.github/workflows/cubase-nightly.yml`.

1. Add the fixture: a Cubase project carrying two Poly instances on separate
   tracks with distinct MIDI output routing. Follow the existing fixture's
   conventions and record in a comment that a `.cpr` is Cubase-version-specific,
   as the workflow already notes of the current one.
2. Write the spec: attach to each instance's WebView in turn — the CDP target
   list carries one per editor, so selecting the right one by title or URL is
   the first thing the spec must get right and the first thing to assert. Give
   each a different patch, play, and capture per instance.
3. Assert each instance emits its own patch's notes and not the other's. The
   failure this guards is shared state, so the assertion that matters is
   asymmetry: instance A's stream must contain something B's does not.
4. Implement `POLY_E2E_MUTATE=s05-shared-state`: the spec applies the same patch
   to both instances, so the asymmetry assertion fails. Pointing both at one
   state blob is what the row describes and this is its testable form.
5. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S05.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S05`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
