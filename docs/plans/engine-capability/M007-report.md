---
class: gated
---

# M007 — Review report

Status: current (2026-09-15)

Generated from `docs/plans/engine-capability/ledger.md`, git, and
`M007-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** A developer can run every check CI will run, and every test in the
tree runs somewhere in CI — so a green local gate means something, and a test
file cannot be proven only on the machine that wrote it.

**Branch:** `milestone/M007-runnable-guards`, cut from `main` at `214795d`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M007/S01 | The orphaned guards get a home | GAP03 | done |
| M007/S02 | Reachability is itself checked | GAP04 | done |

## Definition of done

**M007/S01**

- [x] Every guard listed in `GAP03` is reachable from a command declared in `.jk/validations.yml`
- [x] Running that command on a tree that breaks one of them fails, shown for at least one guard of each kind
- [x] `CLAUDE.md` names the command

**M007/S02**

- [x] A check fails if any `scripts/check-*` guard is reachable from no declared token, the pre-push gate, `pre-commit`, or the doc-conformance runner
- [x] The check has been shown to fail by adding a guard reachable from nothing
- [x] A guard that genuinely cannot run locally is declarable in-band, with a reason
- [x] The check itself runs in CI

## Validation

Run on the current head.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `guards` | `bash scripts/check-guards.sh` | pass, 13 invocations |
| `site-unit` | `npm --prefix site test` | pass, 284 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass, 268 tests |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |

## Traceability

Every commit carries a `Slice:` trailer. **No untraced commits.**

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `81b88e2` | docs(plans): plan M007's two slices and record its decisions | M007/S01, M007/S02 | — |
| `20035fb` | feat(scripts): make the eleven CI-only guards runnable locally | M007/S01 | — |
| `4033d5a` | docs: name the guards command in CLAUDE.md, and close M007/S01 | M007/S01 | GAP03 |
| `135e1ba` | test(site): fail when a repo guard is reachable from no local command | M007/S02 | — |
| `b8855c9` | docs(plans): close M007/S02 with the reachability check proved | M007/S02 | GAP04 |

Five commits, no corrections to earlier ones — the first milestone this session
that needed none.

## What a reviewer should look at twice

1. **The reachability check found a guard on its first run, and it was worse
   than the eleven.** `scripts/check-release-workflow.mjs` locks the release
   workflow's shape in 27 tests — that no Linux plugin zip returns, that the
   validator flag stays on, the build → validate → pluginval → sign → package
   order, the zip layout — and **no workflow runs it**. `release.yml` names it
   in three comments and never invokes it. The eleven `GAP03` counted at least
   ran in CI; this one ran nowhere. It is runnable and passes, so it joined
   `check-guards.sh`.

2. **S01's survey missed it, and the reason generalises.** That list was derived
   by grepping `ci.yml`, and this guard belongs to `release.yml`. A survey is
   only as wide as the file it greps — which is the same shape of error as
   M006's "6 of 23" (a grep that matched a comment) and M005's PIPE01 (a fix
   scoped to one of two sibling scripts).

3. **The empty-reason arm is the one that matters.** A `# local-unrunnable:`
   marker with nothing after the colon still fails the check. Without that, the
   hatch would be a way to silence the guard rather than declare an exemption,
   and a bare marker costs nothing to type.

4. **The check runs in CI, verified rather than assumed.**
   `doc-conformance-wiring.test.mjs` is one of the 20 files in
   `check-doc-conformance.sh`'s `TESTS` array, and the new case appears in that
   runner's output. M006/S02 shipped a guard executed only by the step it
   guarded; checking rather than assuming is that correction's whole lesson.

5. **The wrapper deliberately omits `set -e`,** and that was proved rather than
   asserted: three simultaneous breaks produced three named failures. With `-e`
   a developer would fix one guard per invocation.

6. **What this milestone does not do.** The guards are runnable, not automatic —
   `scripts/pre-push-check.sh` is unchanged at five checks. Wiring them in would
   have caught M002's ship failure without anyone remembering; the decision and
   its cost are both recorded. The reachability check stops a guard becoming
   unreachable; it does not stop someone forgetting to run one.

## Decisions

Verbatim from `M007-decisions.md`.

## 2026-09-15 — planning M007

- **Decision:** Both slices classified **bounded**; no design document.
  — **Why:** S01 declares a token wrapping eleven existing commands; S02 adds
  one assertion and one in-band marker. Neither introduces a mechanism.

- **Re-measured before planning.** Eleven guards run in CI and from no
  documented local command — six `check-*.sh` and five `check-*.mjs` — which is
  exactly what `GAP03` recorded. M006 added `check-doc-discipline.sh`, which is
  reachable through the `doc-discipline` token, so the count did not drift.

- **Q:** The eleven split across two CI jobs — eight in `code-quality`, three in
  `site-lint`. How should they be declared locally? — **A:** One `guards` token.
- **Decision:** A single `guards` token running all eleven. — **Why:** A
  developer asking what CI will run on their change gets one answer, and a slice
  touching anything guarded names one thing. Mirroring the CI job split would
  map a red job to one command, but that split is a fact about CI's job layout
  rather than about the guards, and each guard names itself when it fails.

- **Q:** How should a genuinely-unrunnable guard be declared?
  — **A:** An in-band marker in the script.
- **Decision:** `# local-unrunnable: <reason>` in the guard's own header, which
  the reachability check honours. — **Why:** It matches the escape-hatch
  discipline this repo documents: in-band, greppable, reasoned, so
  `grep -rn local-unrunnable scripts/` enumerates every exemption and that
  listing *is* the audit. A list in the test file would put the reason away from
  the script it excuses, which is how a registry drifts from the code.
  `check-wasm-freshness.sh` is the real case: it compares a deployed URL's
  artifacts and has nothing to check from a clean checkout.

- **Q:** Should the pre-push hook run the new guards? — **A:** Runnable, not
  automatic.
- **Decision:** `scripts/pre-push-check.sh` is left alone; the token is declared
  and named in `CLAUDE.md`. — **Why:** M006 established the hook is not at fault
  and does exactly what it documents, and this milestone's vision is that a
  developer *can* run every check CI will run — not that every push pays for all
  of them. The trade is real and worth naming: wiring it into pre-push would
  have caught M002's ship failure automatically, and leaving it out means a
  slice has to name the token. S02's reachability check is what stops a guard
  going unreachable again; it does not stop someone forgetting to run one.

## 2026-09-15 — executing M007/S02 task 1 (judgment call)

- **Finding:** `scripts/check-release-workflow.mjs` — 27 tests locking the
  release workflow's shape — is run by **nothing**. `release.yml` names it only
  in comments. It was worse off than the eleven `GAP03` counted, which at least
  ran in CI. Found by the reachability check on its first run.
- **Decision:** Added to `check-guards.sh` rather than marked
  `local-unrunnable`. — **Why:** It is runnable and passes (27 tests), so
  marking it unrunnable would be false. Obviously right and too small to halt
  for, but it is a twelfth guard beyond `GAP03`'s eleven, so it is recorded
  rather than left in the diff. S01's count missed it because that survey was
  derived from `ci.yml` and this guard belongs to `release.yml`.
- **Decision:** `scripts/check-guards.sh` is one of the reachability sources.
  — **Why:** The first run reported all eleven S01 guards unreachable, because
  `.jk/validations.yml` names the wrapper and not the guards inside it. The
  wrapper is a declared token, so what it runs is reachable through it.
