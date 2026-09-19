---
class: gated
---

# M005/S02 — Each finding is fixed or recorded benign

**Slice:** guide-parity M005/S02 (row GP12; GP13 rides `accepted`)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Depends:** M005/S01
**Decisions consumed:** `M005-decisions.md` (2026-09-19)

## Task status

- [ ] 1. A test that fails on the race, by the means S01 established
- [ ] 2. Fix the torn read at its source
- [ ] 3. Show it gone, and say what "gone" is worth
- [ ] 4. Evidence and slice close-out

## Definition of Done

Copied verbatim from the slice:

- [ ] Every finding classified in S01 is either fixed, or recorded as benign with the reason and a suppression entry naming it
- [ ] A fixed finding is shown gone by the means S01 established — the reproducing invocation, or a named nightly run if it never reproduced locally
- [ ] No suppression is added without a written reason

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
| `sanitizers` | the five-variant chain in `.jk/validations.yml` |

---

## Task 1 — A test that fails on the race, by the means S01 established

**Consumes:** S01's reproducing invocation, or its recorded non-reproduction.
**Produces:** the thing the fix is measured against.

**This task's shape depends on what S01 found, and the plan says so rather than
guessing.** Two cases:

- **S01 reproduced locally.** The reproducing invocation *is* the failing test.
  Record it as the baseline, confirm it still fails on this branch before any
  change, and go to task 2.
- **S01 did not reproduce locally.** Then there is no local red to turn green,
  and the fix can only be verified by the nightly. Say so here, and **stop to
  ask** whether to proceed on inspection alone or to keep trying for a
  reproduction first. Fixing a race you cannot observe is how a plausible change
  gets called a fix.

**Files:** possibly `tests/host/host_tests.cpp`.

**Steps:**

1. Re-run S01's exact invocation on this branch and confirm the failure is still
   present. A baseline that has gone quiet on its own invalidates everything
   after it.
2. If the reproduction needs many iterations to be reliable, record how many and
   how long — task 3 has to run it enough times for "gone" to mean something.

**Check:** a named invocation that fails, with its failure output recorded. Or a
recorded halt.

---

## Task 2 — Fix the torn read at its source

**Consumes:** task 1's baseline.

The evidence points at a non-atomic multi-word copy of state shared between the
host thread and the audio thread: TSan's summary named `memcpy`, and the test's
own assertion caught a payload whose `noteMap[0]` disagreed with the rest of the
array it was copied with.

**Files:** `plugin/source/` — the handshake behind the frames S01 recorded.

**Steps:**

1. Read the handshake implementation completely before changing it, starting
   from the stack frames S01 recorded. Identify which structure is copied
   non-atomically and which two threads touch it.
2. **Follow the repo's existing discipline rather than inventing one.**
   `CLAUDE.md` and `ARCHITECTURE.md` describe the lock-free patterns this
   codebase already uses — seqlock-style versioned publication, atomic
   ping-pong, double buffering. A fix that introduces a new mechanism where an
   existing one fits is a fix a reviewer cannot check against anything.
3. **No allocation, no locks, no exceptions, no I/O on the audio-thread path.**
   `rt-safety` is in this slice's token set precisely because the fix is in that
   path, and a mutex would be the obvious wrong answer.
4. Make the smallest change that addresses the race. Adjacent improvements go in
   the report, not the diff.

**Check:** `unit` and `rt-safety` pass; task 1's invocation is run but not yet
trusted — that is task 3.

---

## Task 3 — Show it gone, and say what "gone" is worth

**Consumes:** task 2's change.

A race that took 22 nights to show under TSan and 1 night in 34 without it
cannot be declared fixed by one green run. The claim has to be proportionate to
the evidence.

**Steps:**

1. Run S01's reproducing invocation enough times that a pass means something.
   Record the count. If the baseline failed within N iterations, run
   substantially more than N and say how many.
2. Run the full `sanitizers` token — all five variants — and record each.
3. **State the limit of the claim in the evidence.** If the fix is verified
   locally on Darwin arm64 / Apple clang while the filed failures were
   ubuntu / gcc, then a local pass is evidence and not proof. Dispatch
   `sanitizers.yml` on this branch and record the run id; name that run as the
   verification, per the definition of done.
4. Connect it to S01 task 3's answer. If the 2026-08-17 quieting was a window
   narrowing rather than a fix, a green run here means less than it appears, and
   the evidence must say so.
5. **If any finding is not fixed**, record it as benign with its reason and add a
   suppression entry naming it. No suppression without a written reason — that
   is a definition-of-done item, and the repo's escape-hatch discipline requires
   the reason to be a claim someone could check.

**Check:** `format`, `unit`, `rt-safety`, `sanitizers` all pass, and the evidence
states what the verification is worth.

---

## Task 4 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M005-S02.md`; modify the
ledger.

**Steps:**

1. Append one entry per task: token, exit code, headline counts, date. No commit
   SHAs.
2. Record the fix in one paragraph a reviewer can check: which structure was
   racing, which two threads, and which existing pattern was applied.
3. Record that #142 and #274 are one defect, and whether this fix closes both.
   Name the nightly run id that verifies it.
4. Record any finding left unfixed, with its reason and its suppression entry.
5. Set row GP12 to `done` and slice M005/S02 to `done`, ticking all three
   definition-of-done boxes. **GP13 stays `accepted`** — it is the #89
   threshold change, deliberately not sliced, to be landed as an ordinary pull
   request.
6. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done`.
