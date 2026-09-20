---
class: gated
---

# M005/S02 — Each finding is fixed or recorded benign

**Slice:** guide-parity M005/S02 (row GP12; GP13 rides `accepted`)
**Ledger:** `docs/plans/guide-parity/ledger.md`
**Depends:** M005/S01
**Decisions consumed:** `M005-decisions.md` (2026-09-19)

## Task status

- [x] 1. Re-encode assertion 3 so it survives the round trip
- [x] 2. Show it still detects a genuine tear
- [ ] 3. Evidence and slice close-out

**Re-planned at execution.** This plan was written to audit a lockless triple
buffer for a residual race. M005/S01's hunt for a reproduction found one, and it
is not a race: a single-threaded `writeSceneState`/`readSceneState` round trip
reproduces the nightly's exact message, because `readSceneState` clamps
`noteMap` to `[0,127]` while the test encodes up to 32767. The remaining work is
a test fix, and `plugin/source/` is not touched. See `M005-decisions.md`.

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

## Task 1 — Re-encode assertion 3 so it survives the round trip

**Closes:** the Group B half of row GP12.

**Files:** modify `tests/host/host_tests.cpp`.

**Steps:**

1. Reproduce first, so the change has something to turn green. Assert the
   current encoding fails the invariant after a round trip for a writeId whose
   product exceeds 127 — `100000` gives product `19808`. Watch it fail with the
   nightly's own message.
2. Change `encodeNoteMapField` to mask with `0x7F` instead of `0x7FFF`, and the
   two comparison masks in assertion 3 to match. XOR with `i < 128` stays inside
   the same 7-bit block, so every value survives the `[0,127]` clamp untouched
   and `map[i] == map[0] ^ i` holds exactly.
3. Update the comment above assertion 3 to say **why** the range is 7 bits —
   that the serialization layer sanitizes, and a wider encoding is clamped into
   a false tear. Without that sentence the next person widens it again.
4. Run `HandshakeStress_NoTearNoLoss` and `HandshakeStress_TSanClean`.

**Check:** `unit` passes; the round-trip assertion from step 1 now holds.

**Do not change `sanitizeSceneState`.** The clamp is a deliberate defence
against a host supplying corrupt state, and `tests/state_migration_tests.cpp`
asserts that behaviour. Loosening the product to satisfy a test inverts the
relationship between them.

---

## Task 2 — Show it still detects a genuine tear

**Produces:** the evidence that the fix is not vacuous.

A check that can no longer fail is worse than one that false-positives, and this
change makes an assertion stop firing. The burden is to show it still fires on
the thing it exists for.

**Files:** modify `tests/host/host_tests.cpp`.

**Steps:**

1. Add a test that builds a `noteMap` whose first 64 entries come from one
   writeId and whose last 64 come from another — a synthetic torn read, with no
   threading — and asserts the invariant **rejects** it.
2. Run it and watch it pass; then invert it temporarily to confirm it would fail
   on a clean map, so the test itself is not vacuous.
3. Run the full `sanitizers` token — all five variants — and record each.
4. Dispatch `sanitizers.yml` on this branch and record the run id. A local pass
   on Darwin arm64 is evidence; the filed failures were ubuntu/gcc.

**Check:** `format`, `unit`, `rt-safety`, `sanitizers` all pass, and the
synthetic-tear test is shown to be load-bearing.

---

## Task 3 — Evidence and slice close-out

**Files:** create `docs/plans/guide-parity/evidence/M005-S02.md`; modify the
ledger.

**Steps:**

1. Append one entry per task: token, exit code, headline counts, date. No commit
   SHAs.
2. Record the before/after numbers: the invariant failed for 99,608 of 100,000
   writeIds and now fails for 0, while still detecting 400 of 400 synthetic
   tears.
3. Record that #142 and #274 are one defect with two observers, and that this
   closes both — Group A by `076f545` in August, Group B by this slice.
4. Set row GP12 to `done` and slice M005/S02 to `done`, ticking all three
   definition-of-done boxes. **GP13 stays `accepted`** — the #89 threshold
   change is deliberately not sliced.
5. Run `jk-standards ledger`, then the full validation set.

**Check:** `jk-standards ledger` passes with the slice `done`.
