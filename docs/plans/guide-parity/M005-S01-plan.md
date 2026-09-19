---
class: gated
---

# M005/S01 — Every filed finding is reproduced or its resistance recorded

**Slice:** guide-parity M005/S01 (row GP14)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Decisions consumed:** `M005-decisions.md` (2026-09-19)

## Task status

- [x] 1. The `sanitizers` token runs here, and its result is recorded
- [x] 2. Reproduce the race under TSan, or record what resisted
- [x] 3. Find what stopped TSan reporting on 2026-08-17
- [x] 4. Evidence and slice close-out

## Definition of Done

Copied verbatim from the slice:

- [ ] The `sanitizers` token runs all five variants locally and its result is recorded
- [ ] Each of the filed occurrences is classified by sanitizer, stack and date
- [ ] Either a finding reproduces locally, with the exact invocation and iteration count that produced it recorded — or the attempts are recorded with what was tried and what the filed occurrences' logs show

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `sanitizers` | the five-variant chain in `.jk/validations.yml` |

---

## Task 1 — The `sanitizers` token runs here, and its result is recorded

**Produces:** the first local evidence that the token works at all, and a
baseline for task 2.

The token was added at assessment because nothing ran the sanitizers outside CI.
It has never been executed. Running it is the slice's first definition-of-done
item in its own right, independent of what it finds.

**Files:** none. This task's product is evidence.

**Steps:**

1. Run the `sanitizers` token exactly as `.jk/validations.yml` declares it —
   all five variants, in order, without editing the command.
2. Record for **each** variant: whether it configured, whether it built, whether
   `ctest` ran, and the pass/fail counts. A variant that fails to *configure* on
   Darwin arm64 is a result, not an error to work around — record it and move on.
3. Note the wall-clock cost of the whole chain. A local sanitizer command nobody
   can afford to run is a token in name only, and the number belongs in the
   evidence so a later reader can decide.
4. **Do not fix anything yet.** If a variant reports a genuine finding other than
   the handshake race, record it and carry it into task 2's classification.

**Check:** the evidence names all five variants and what each returned.

**Expect friction.** The CI failures are ubuntu-24.04 / gcc; this is Darwin
arm64 / Apple clang 21. TSan and ASan are both supported there, but the
`POLY_TSAN_ACTIVE` guard in `tests/host/host_tests.cpp` keys off
`__has_feature(thread_sanitizer)`, which is a Clang feature — confirm the
TSan-only twin `HandshakeStress_TSanClean` actually compiles in, rather than
assuming the `#ifdef` fired.

---

## Task 2 — Reproduce the race under TSan, or record what resisted

**Consumes:** task 1's baseline. **Produces:** either a reproducing invocation
with its iteration count, or a recorded account of what was tried.

Both outcomes satisfy the definition of done. The slice was written that way
deliberately, and a recorded non-reproduction is not a failed task.

**Files:** none permanently.

**Steps:**

1. Build the TSan plugin variant alone and run `ctest -R "^HostTests\."`.
   Confirm from the output that **both** `HandshakeStress_NoTearNoLoss` and
   `HandshakeStress_TSanClean` ran — the filed failure was the former, and the
   latter existing does not mean the former was skipped.
2. If it passes, repeat with `ctest --repeat until-fail:20`. Record how many
   iterations were attempted, not just that it "did not reproduce".
3. If it still passes, dispatch `.github/workflows/sanitizers.yml` on this
   branch, which runs the filed configuration exactly — ubuntu-24.04, gcc. Record
   the run id and what it returned.
4. When it **does** reproduce, capture the full report: the `WARNING:
   ThreadSanitizer: data race` block, the `SUMMARY:` line, and every stack frame
   with its `host_tests.cpp` line number. Compare against the filed frames —
   1069, 1104, 1132, 1139 from run `31924034914` — and record whether they match.
   Different frames mean a different race, not a confirmation.
5. Record the exact invocation that produced it, including the iteration count.
   GP12's fix is verified against this and nothing else.

**Check:** the evidence names either the reproducing invocation or the attempts,
with counts.

**Do not attempt a fix in this slice.** The fix is GP12, in M005/S02.

---

## Task 3 — Find what stopped TSan reporting on 2026-08-17

**Consumes:** nothing. **Produces:** the answer to the one question that could
invalidate S02's verification.

TSan failed 22 consecutive nights and then stopped, while the 2026-09-16 torn
read proves the race survived. A race whose window merely narrowed is
indistinguishable from a fixed one for 30 nights, so this must be known before
any fix is called verified.

**Files:** none permanently.

**Steps:**

1. List the commits between the last failing run (`31924034914`, 2026-08-16) and
   the first subsequent success. Thirteen landed in that window.
2. Rule out the cheap explanations first, by reading rather than building:
   - did `.github/workflows/sanitizers.yml` change its `-R` filter or its build
     target, so the TSan job stopped running the failing test?
   - did `tests/host/host_tests.cpp` change the test's iteration count, thread
     count, or the `POLY_TSAN_ACTIVE` guard?
   - did the handshake implementation change — the code behind the frames at
     `host_tests.cpp` 1069/1104/1132/1139?
3. If none of those explains it, bisect: build the TSan plugin variant at the
   last failing commit and at the first passing one, and run the test at each.
   **Only do this if task 2 reproduced locally** — bisecting with a detector that
   does not fire here would be measuring nothing.
4. Record the answer, or record that it resisted and what was eliminated. "Not
   determined" with a list of ruled-out causes is a result; silence is not.

**Check:** the evidence names the cause, or names what was ruled out.

---

## Task 4 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M005-S01.md`; modify the
ledger.

**Steps:**

1. Append one entry per task: token, exit code, headline counts, date. Name no
   commit SHAs — this file ships inside the commit it would describe.
2. Record the **classification** the definition of done asks for: every filed
   occurrence by sanitizer, stack and date. The orientation in
   `M005-decisions.md` established this; restate it here as the slice's own
   record rather than pointing at another file.
3. Record that #142 and #274 are one defect with two observers, per the decision.
4. Set row GP14 to `done` and slice M005/S01 to `done`, ticking all three
   definition-of-done boxes.
5. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done`.
