---
class: gated
---

# M004/S04 — Editor lifecycle

**Slice:** `M004/S04` in `docs/plans/engine-capability/ledger.md`

Group 1 of the batching recorded in `M004-decisions.md`: its nightly
evidence comes from the dispatch that closes its group, not from a run of its
own.

## Task status

- [ ] 1. The spec, its contract, and the workflow step
- [ ] 2. Cite the nightly run and close the slice

## Definition of Done

Copied verbatim from the slice.

- [ ] The spec opens and closes the plugin editor repeatedly within one session
      and asserts the plugin still responds and still emits notes afterwards
- [ ] The spec has been shown to fail when the WebView does not re-attach
- [ ] A nightly run is named in the evidence with this spec green

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `cubase-harness` | `npm --prefix tests/cubase/e2e run typecheck && npm --prefix tests/cubase/e2e run test:unit && python3 -m unittest discover -s tests/cubase -p 'test_*.py'` |

`cubase-harness` does **not** run Cubase. It type-checks the specs and runs the
helper-lib and validator unit tests. The only gate that proves a nightly spec
works is the nightly, which is why the third definition-of-done item exists.

## Task 1 — Cycle the editor and assert the plugin still works

Creates `tests/cubase/e2e/editor-lifecycle.spec.ts`; modifies
`.github/workflows/cubase-nightly.yml`.

1. Write the spec: attach over CDP, then close and reopen the plugin editor a
   fixed number of times within one session. After the last cycle, assert two
   things — the bridge responds to a round-trip request, and the transport still
   emits notes, driven through the existing `play_scenario.py` path.
2. Re-attaching is the interesting part: WebView2 tears down the CDP target when
   the editor closes, so the spec must reconnect rather than assume its handle
   survives. Record that in a comment; it is the same class of runner-specific
   fact `toggle-step.spec.ts` documents about `127.0.0.1`.
3. Implement `POLY_E2E_MUTATE=s04-skip-reattach`: the spec keeps its stale page
   handle instead of reconnecting, so the post-cycle assertions fail. That is
   precisely the failure the definition of done names.
4. Add the workflow step. Run `format` and `cubase-harness`. Commit.

## Task 2 — Cite the nightly run and close the slice

1. Wait for the group's dispatch to complete. Read the run, not the exit code:
   confirm this slice's spec step is green, by name, in that run's log.
2. Append `evidence/M004-S04.md` with the token results, the run URL, and the
   red-path run URL showing this slice's `POLY_E2E_MUTATE` value failing.
3. Tick the task and definition-of-done boxes, set the row and the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M004/S04`.

**If the spec is red in the dispatch,** that is the deferred decision recorded in
`M004-decisions.md`: stop, report what failed with the log's headline, and let
the user choose between a fix-and-re-dispatch and leaving the slice open.
