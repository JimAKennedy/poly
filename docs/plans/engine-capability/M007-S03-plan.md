---
class: gated
---

# M007/S03 — The gate runs what it can

**Slice:** `M007/S03` in `docs/plans/engine-capability/ledger.md`

## Task status

- [x] 1. The hook runs `guards`, `doc-discipline`, and `site-unit`
- [x] 2. Lock the coverage rule, prove it bites, and close the slice

## Definition of Done

Copied verbatim from the slice.

- [x] The pre-push gate runs `guards`, `doc-discipline`, and `site-unit`
- [x] A check fails if a token in `.jk/validations.yml` is neither run by the
      pre-push gate nor declared exempt in-band with a reason
- [x] That check has been shown to fail, both for an unaccounted-for token and
      for an exemption with an empty reason
- [x] `CLAUDE.md` describes what the hook actually runs

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

## Task 1 — The hook runs `guards`, `doc-discipline`, and `site-unit`

Modifies `scripts/pre-push-check.sh` and `CLAUDE.md`.

The script's steps are labelled `[0/6]` through `[6/6]` — seven steps under a
six-step numbering, which is its own small instance of a document disagreeing
with the thing it describes. Renumber to `[0/9]`–`[9/9]` as part of this task.

Steps:

1. In `scripts/pre-push-check.sh`, inside the `region:pre-push-gates` markers
   and after the existing doc-conformance step, add three steps in this order —
   cheapest first, matching `.jk/validations.yml`'s own ordering:
   - `site-unit` → `npm --prefix site test`
   - `doc-discipline` → `bash scripts/check-doc-discipline.sh`
   - `guards` → `bash scripts/check-guards.sh`
   Each follows the file's existing idiom exactly: `if ! <command>; then` with
   an `echo "FAIL: …"` naming what to run to reproduce, and `FAILED=1` rather
   than an early exit, so one push reports every failing gate rather than the
   first. `site-unit` shares the doc-conformance step's `site/node_modules`
   guard — put it inside that same `else` branch rather than repeating the
   check.
2. Renumber every step label from `[N/6]` to `[N/9]`.
3. Run `bash scripts/pre-push-check.sh` directly and read the output: all ten
   labels present, the three new steps run, exit 0. Note the wall-clock cost of
   the three additions and record it in the evidence.
4. In `CLAUDE.md`, replace the "Pre-Push Quality Gate" section's numbered list
   and the paragraph asserting the hook covers "the five items above and nothing
   else, by design". That claim is false today and this task makes it more so.
   The replacement states what the hook runs and the rule it now follows: the
   local gate is CI minus what genuinely cannot run locally. Keep the two
   paragraphs naming `check-guards.sh` and `check-doc-discipline.sh` — they are
   still the right commands to name, but they now describe what the hook runs
   too, not a manual substitute for it.
5. Run `format`, `doc-discipline`, and `guards`. Commit with the slice's
   trailers and `Rows:` left empty — this task does not close `GAP05` on its
   own, because the row's verification requires the coverage check.

## Task 2 — Lock the coverage rule, prove it bites, and close the slice

Modifies `.jk/validations.yml`, `site/tests/doc-conformance-wiring.test.mjs`,
`docs/plans/engine-capability/evidence/M007-S03.md`, the ledger, and this plan.

The rule: **every token in `.jk/validations.yml` is either run by the pre-push
gate or declared exempt in-band, with a reason.** The check must not try to
infer which token a shell line corresponds to — matching command strings is
brittle, and a brittle check is one that gets deleted. Both sides declare
themselves instead.

Steps:

1. **Write the failing test first.** In
   `site/tests/doc-conformance-wiring.test.mjs`, beside the `GAP04` reachability
   case, add a case that parses the token names from `.jk/validations.yml`, the
   `# pre-push-token:` markers from `scripts/pre-push-check.sh`, and the
   `# pre-push-exempt:` markers from the yaml, then asserts:
   - every token appears in exactly one of the two sets — an unaccounted-for
     token fails naming itself, and so does one that is both run and exempted
   - every exemption's reason is non-empty after the em dash
   - every `# pre-push-token:` marker names a token that exists
   Follow the file's existing structure: `CHECKLIST.push(...)` **above** the
   `let liveMarkers = 0;` line, as the surrounding cases do.
2. Run it and watch it fail. With no markers anywhere yet, it must fail naming
   all fourteen tokens — not error, and not fail for a parse reason. A failure
   listing the tokens is the proof it reads both files correctly.
3. Add the run-markers. In `scripts/pre-push-check.sh`, put a
   `# pre-push-token: <name>` comment on the line above each gate that runs a
   token — the three added in task 1, and the existing `unit`, `rt-safety`,
   `snippet-regions` and `doc-conformance`. `format` is the one existing step
   that must **not** be marked: step 1 runs `pre-commit run clang-format`, which
   is narrower than the token's `pre-commit run --all-files`, and marking it
   would make the check certify something untrue.
4. Add the exemptions. In `.jk/validations.yml`, add
   `# pre-push-exempt: <token> — <reason>` beside each token the hook does not
   run: `format` (the narrower arm above), `engine-isolation` (configures a
   second build tree), `e2e` (minutes, and it rewrites the committed WASM
   artifacts), `webui-e2e`, `wasm-freshness` (needs a deployed URL),
   `cubase-harness`, and `gate` (it *is* the hook). Re-run the test and watch it
   pass — the transition from fourteen named tokens to zero is the check
   demonstrating it reads both sides.
5. Prove both arms, each with the tree restored afterwards and
   `git diff --quiet` confirming it:
   - add a token `probe-token: true` to `.jk/validations.yml` with no marker of
     either kind → the case fails naming `probe-token`
   - give it a `# pre-push-exempt: probe-token —` with nothing after the dash →
     the case fails naming the empty reason
6. Verify the check runs in CI rather than assuming it: confirm
   `doc-conformance-wiring.test.mjs` is in `check-doc-conformance.sh`'s `TESTS`
   array and that the new case appears in that runner's output. This is the
   step `M006/S02` got wrong.
7. Run every token in the slice's validation set. Append the evidence file,
   tick both task boxes, tick the four definition-of-done boxes, set `GAP05` to
   `done` and the slice to `done`, run `jk-standards ledger`, and commit with
   `Slice: M007/S03` and `Rows: GAP05`.
