---
class: gated
---

# M007/S02 — Reachability is itself checked

**Slice:** M007/S02 — `docs/plans/engine-capability/ledger.md`
**Rows:** GAP04
**Depends:** M007/S01
**Classification:** bounded. One assertion and one in-band marker.

## Task status

- [ ] 1. The reachability check, and the `local-unrunnable` marker
- [ ] 2. Prove it bites, confirm it runs in CI, close the slice

## Definition of Done

- [ ] A check fails if any `scripts/check-*` guard is reachable from no declared
      token, the pre-push gate, `pre-commit`, or the doc-conformance runner
- [ ] The check has been shown to fail by adding a guard reachable from nothing
- [ ] A guard that genuinely cannot run locally — one needing Cubase, a
      self-hosted runner, or a deployed URL — is declarable as such in-band,
      with a reason, rather than needing the check disabled
- [ ] The check itself runs in CI

## Context both tasks need

- **S01 must have landed.** The check asserts every guard is reachable, and
  before S01 eleven are not. Running this first would fail on work S01 does.
- **The one real exemption** is `check-wasm-freshness.sh`: it compares a
  deployed URL's artifacts against the checked-in ones by hash and has nothing
  to check from a clean checkout. It is the reason the escape hatch is a
  definition-of-done item rather than an afterthought.
- **The marker is `# local-unrunnable: <reason>`** in the guard's own header.
  In-band, greppable, reasoned — `grep -rn local-unrunnable scripts/`
  enumerates every exemption, and that listing is the audit. A marker with no
  reason after the colon must fail the check, or the hatch becomes a way to
  silence it.
- **The reachability sources** are the four a developer can actually invoke:
  `.jk/validations.yml`, `scripts/pre-push-check.sh`, `.pre-commit-config.yaml`,
  and `scripts/check-doc-conformance.sh`. A guard named in any of them is
  reachable; `scripts/check-guards.sh` counts because S01 declares it as a
  token.
- **M006/S02's lesson binds here.** That slice shipped a guard executed only by
  the CI step it guarded, and needed a follow-up commit to de-circularise it.
  This check must live in a file `scripts/check-doc-conformance.sh` names, and
  task 2 verifies that rather than assuming it.

## Task 1 — The reachability check and the marker

**Files:** `site/tests/doc-conformance-wiring.test.mjs`,
`scripts/check-wasm-freshness.sh`

1. Add a case that reads `scripts/` for `check-*.sh` and `check-*.mjs`, and for
   each asserts it appears in at least one of the four reachability sources, or
   carries a `# local-unrunnable:` marker with a non-empty reason. Run
   `site-unit` and watch it fail on `check-wasm-freshness.sh`, which is
   reachable from the `wasm-freshness` token but — check this rather than
   assume — may already be reachable and so not fail at all. If it does not
   fail, the case has no exemption to exercise yet: add the marker anyway in
   step 2 and prove the marker path in task 2 instead.
2. Add the marker to `check-wasm-freshness.sh`'s header with a reason naming
   what it needs: a deployed URL, which a clean checkout does not have.
3. Run `site-unit` and watch the case pass.
4. Run `format` and `doc-conformance`. Append evidence to
   `docs/plans/engine-capability/evidence/M007-S02.md`, tick task 1, run
   `jk-standards ledger`, commit with the slice's trailers.

## Task 2 — Prove it bites, confirm it runs in CI, close the slice

**Files:** `docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M007-S02.md`

1. **Prove the unreachable arm fires.** Add a throwaway
   `scripts/check-m007-probe.sh` referenced by nothing, run `site-unit`, and
   confirm the case fails naming it. Delete the probe and confirm the suite is
   green and `git status` clean.
2. **Prove the marker arm is not a blanket silencer.** Give the probe a
   `# local-unrunnable:` marker with an *empty* reason, confirm the case still
   fails; then give it a real reason and confirm it passes. Delete the probe.
3. **Confirm the check runs in CI**, by checking that the file it lives in is
   named in `scripts/check-doc-conformance.sh` and that the doc-conformance
   count rose. M006/S02 shipped a guard run only by the thing it guarded and had
   to correct it; do not repeat that by assuming.
4. Run the full validation set. Append evidence, tick task 2, set row GAP04 to
   `done`, tick all four definition-of-done boxes, set slice M007/S02 to `done`,
   run `jk-standards ledger`, commit with the slice's trailers.
