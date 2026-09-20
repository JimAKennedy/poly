---
class: gated
---

# M005 — The sanitizer findings are understood

> Every sanitizer finding the nightly has filed is reproduced and classified,
> and each is either fixed or recorded as benign with its reason.

Ledger: `docs/plans/guide-parity/ledger.md` · 10 commits, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | Every filed finding is reproduced or its resistance recorded | GP14 | done |
| M005/S02 | Each finding is fixed or recorded benign | GP12 | done |

`GP13` (#89) remains `accepted` — the one-line threshold change, deliberately
not sliced, to land as an ordinary pull request.

## The result: there was no product defect to fix

The milestone was written to hunt "an intermittent finding at roughly 5%, across
two sanitizers … in plugin code — which this repo's entire real-time-safety
discipline assumes is clean." Reading the nightly history rather than the last
twenty runs showed that framing wrong in both halves.

| Window | Nights | Result |
|---|---|---|
| 2026-07-22 → 07-25 | 4 | success |
| **2026-07-26 → 08-16** | **22** | failure, every night, `TSAN-PLUGIN` |
| 2026-08-17 → 09-15 | 30 | success |
| **2026-09-16** | 1 | failure, `ASAN-PLUGIN` |

Not intermittent, and not one finding. **Two unrelated defects:**

**Group A — 22 occurrences.** A genuine ThreadSanitizer data race on the
two-slot host→RT exchange. **Already fixed** by `076f545` on 2026-08-16, which
replaced it with a lockless triple buffer and documented why two slots are
insufficient. The streak stopped the same day. Nothing suppressed —
`.github/tsan.supp` is still empty.

**Group B — 1 occurrence.** Not a race at all. Reproduced **single-threaded**:

```
writeId=100000  product=19808  -> "torn-read": map[1]=127 but map[0] implies 126
```

The nightly reported `torn-read: final noteMap[1]=127 but map[0] implies 126`.
Identical. `readSceneState` ends in `sanitizeSceneState`, which clamps `noteMap`
to `[0,127]`; the test encoded up to 32767. Clamping destroyed the invariant the
test checks, and it reported that as tearing.

It explains every observation the race hypothesis could not: TSan silent (no
race), ASan silent (no memory error), one night in 34 (an inject must land
last), and untouched by the triple-buffer fix (the tear was never in the
exchange).

## Definition of done

**M005/S01**

- [x] The `sanitizers` token runs all five variants locally and its result is recorded
- [x] Each of the filed occurrences is classified by sanitizer, stack and date
- [x] Either a finding reproduces locally, with the exact invocation and iteration count that produced it recorded — or the attempts are recorded with what was tried and what the filed occurrences' logs show

**M005/S02**

- [x] Every finding classified in S01 is either fixed, or recorded as benign with the reason and a suppression entry naming it
- [x] A fixed finding is shown gone by the means S01 established — the reproducing invocation, or a named nightly run if it never reproduced locally
- [x] No suppression is added without a written reason

## Validation

| Token | Result |
|---|---|
| `format` | pass |
| `unit` | pass — 693 tests (691 before) |
| `rt-safety` | pass |
| `sanitizers` | pass — ASan 568, UBSan 568, TSan 568, ASan-plugin 57 + 59 bridge, TSan-plugin 57 |

**On the filed configuration:** dispatched run `35479077564` — ubuntu-24.04,
gcc — all five jobs green, including `AddressSanitizer (plugin)`, the job that
filed the 2026-09-16 occurrence.

### The fix, as numbers

| | Old encoding | New encoding |
|---|---|---|
| Invariant holds, writeIds 1–100000 | 392 (0.39%) | 100000 (100%) |
| Invariant fails | 99,608 | 0 |
| Synthetic two-writeId tears detected | — | 400 / 400 |
| Clean single-writeId maps accepted | — | 400 / 400 |

## Traceability

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `220586b` | docs(plans): M005 is one deterministic race, not an intermittent finding | M005/S01, M005/S02  | — |
| `b27fe66` | docs(plans): the sanitizers token runs, and all five variants are clean here | M005/S01  | — |
| `980d32b` | docs(plans): the 22-night TSan streak was fixed, by other work, in August | M005/S01  | — |
| `bdac2c5` | docs(plans): the residual tear did not reproduce in 120 local runs or on CI | M005/S01  | — |
| `ba267b4` | docs(plans): close M005/S01 — 22 of 23 occurrences were already fixed | M005/S01  | GP14  |
| `8319b2b` | docs(plans): the Group B tear is a test defect, reproduced without threads | M005/S02  | — |
| `0842035` | docs(plans): re-plan M005/S02 around the defect that actually exists | M005/S02  | — |
| `907125e` | fix(tests): the torn-read invariant survives the state round trip | M005/S02  | — |
| `f870d5d` | test(host): prove the narrowed torn-read check still catches a tear | M005/S02  | — |
| `a06cf59` | docs(plans): close M005/S02 — the fix is in the test, verified on ubuntu | M005/S02  | GP12  |

No commit on this branch lacks a `Slice:` trailer.

## What a reviewer should look at twice

**A green run was nearly accepted as a mutation proof.** Proving the widened
encoding would still be caught, I applied the mutation and rebuilt inside the
same second — `make`'s timestamp check did not fire, `ctest` ran the previous
binary, and the mutation "passed". Re-run with the change verified by `grep` and
a full second before building, it failed as it should. Same trap that produced a
false green in M003 of this programme. It is in the evidence rather than quietly
corrected.

**GP12 was rewritten because the milestone disproved it.** The row asserted "a
plausible real memory or threading defect" in plugin code. Leaving it would have
been more comfortable than amending the plan of record to say the defect was in
the test.

**The verification is weaker than it looks, and says so.** One clean nightly of
a configuration that failed about one night in 34 proves little by itself. What
carries the claim is that the defect stopped being probabilistic — reproduced
single-threaded, and the same single-threaded test now passes for every writeId.

**No product code changed.** `plugin/source/` is untouched and so is
`sanitizeSceneState` — its clamp defends against a host supplying corrupt state,
and `tests/state_migration_tests.cpp` asserts that behaviour. Changing the
product to satisfy a test would have inverted which is authoritative.

**Two issues can close, for different reasons.** #142 because all 23 occurrences
are attributed and neither cause remains; #274 because its merge-gate flake had
the same cause as Group B. Neither closes because a flake was silenced.

## Decisions

The contents of `M005-decisions.md`, verbatim, follow.

---


Append-only. Every question asked at planning, its answer, and every choice made
on the user's behalf during execution.

## 2026-09-19 — orientation corrected the milestone's subject

The ledger described #142 as "an intermittent finding at roughly 5%, across two
sanitizers". Reading the full nightly history rather than the last twenty runs
showed otherwise.

| Window | Nights | Result |
|---|---|---|
| 2026-07-22 → 07-25 | 4 | success |
| 2026-07-26 → 08-16 | **22** | failure, every night, `TSAN-PLUGIN` |
| 2026-08-17 → 09-15 | 30 | success |
| 2026-09-16 | 1 | failure, `ASAN-PLUGIN` |
| 2026-09-17 → 09-19 | 3 | success |

**One test, one defect, two manifestations.** Every occurrence is
`HostTests.HandshakeStress_NoTearNoLoss`.

- Under TSan, run `31924034914` (2026-08-16) reports
  `WARNING: ThreadSanitizer: data race`, `SUMMARY: … in memcpy`, with frames in
  `tests/host/host_tests.cpp` at 1069, 1104, 1132 and 1139.
- Without TSan, run `35051069155` (2026-09-16) fails the test's **own**
  assertion at `host_tests.cpp:1307` —
  `torn-read: final noteMap[1]=127 but map[0] implies 126`. AddressSanitizer
  reported no memory error; the test found it.

That is the signature of a non-atomic multi-word copy of state shared across
threads: TSan flags it on every run because it instruments every access, while
an uninstrumented build only occasionally lands in the window where the tear is
observable.

**Three facts that shape the slice.** `.github/tsan.supp` holds no suppressions,
so nothing was silenced. The 2026-09-16 torn read proves the race is still live
after 30 quiet nights. And what stopped TSan reporting on 2026-08-17 is unknown:
`HandshakeStress_TSanClean`, the reduced-iteration TSan twin, predates the
streak (`3ab2691`), so it is not the explanation.

### Questions asked before planning

- **Q:** The failures are ubuntu-24.04 / gcc; this machine is Darwin arm64 /
  Apple clang 21. Where should S01 try to reproduce? — **A:** Local TSan first,
  CI dispatch as fallback.
- **Decision:** run the `sanitizers` token here and, if the race does not appear,
  dispatch `sanitizers.yml` on a branch — **Why:** TSan reported this race on 22
  consecutive nights, so it is a strong detector rather than a lucky one, and a
  local reproduction turns fix verification from overnight into seconds. A local
  non-reproduction is itself recorded, not treated as absence.

- **Q:** What stopped TSan reporting on 2026-08-17? — **A:** Investigate; it is a
  finding either way.
- **Decision:** bisect the 13 commits between run `31924034914` and the first
  success — **Why:** a race whose window merely narrowed is indistinguishable
  from a fixed one for 30 nights. If a code change reduced detectability without
  fixing the defect, that must be known *before* any fix is called verified; if
  instead the job stopped exercising the path, the sanitizer coverage has a hole.

- **Q:** #274 is "HandshakeStress_NoTearNoLoss flakes in the merge gate" — the
  same test. The ledger said touching it is "a finding to record, not a row to
  add here". — **A:** Record, and close both if one fix serves.
- **Decision:** no new row; the evidence records that #142 and #274 are one
  defect seen by two observers — the nightly sanitizer job and the merge gate —
  and if GP12's fix removes the race, both issues close and the report says so
  — **Why:** it honours the decision taken at assessment without pretending the
  two are unrelated, which would leave a future reader re-doing this triage.

### Carried in from the M004 close

The row-ID collision found at this milestone's orientation — M004 and M005 both
numbering a row `GP11` — was fixed in its own pull request before M005 began,
renumbering M005/S01 to **GP14** and adding
`scripts/check-ledger-row-ids.mjs`. It is not part of this milestone: M005's
vision is about sanitizer findings, and folding a ledger guard into it would
make the milestone a bag of unrelated work.

### Deferred — asked at the S01→S02 boundary

- **Deferred Q:** If M005/S01 does **not** reproduce the race locally, should
  S02 proceed to a fix on inspection of the TSan stack frames alone, or keep
  trying for a reproduction first?
- **Boundary:** the start of M005/S02 task 1. Reaching it is a planned pause,
  not a failure.
- **Why it cannot be answered now:** the answer turns on what S01 actually
  found. If TSan fires here, the question never arises — the reproducing
  invocation is the baseline and the fix is verified against it. If it does not,
  the choice is between fixing a race nobody in this session has observed and
  spending more effort trying to observe it, and that trade depends on what
  resisted and why.
- **What is already decided either way:** a fix is never declared verified by a
  single green run. S02 task 3 requires the iteration count to be recorded and
  the limit of the claim stated — a local pass on Darwin arm64 / Apple clang is
  evidence, not proof, against failures filed on ubuntu / gcc.

## 2026-09-19 — the Group B "torn read" is a test defect, not a race

The deferred question was answered "hunt the reproduction harder first". The
hunt succeeded, and what it found is not what GP12 assumed.

**Reproduced with no threading at all.** A single-threaded `writeSceneState` →
`readSceneState` round trip, with one map and no second thread, produces the
nightly's exact message:

```
writeId=100000  product=19808  -> "torn-read": map[1]=127 but map[0] implies 126
```

The filed 2026-09-16 failure (run `35051069155`) reads
`torn-read: final noteMap[1]=127 but map[0] implies 126 (writeIdTimes31=127)`.
The same.

**Mechanism.** `readSceneState` (`engine/include/poly/state_io.h:209`) ends with
`sanitizeSceneState`, which clamps every `noteMap` entry to `[0,127]`
(`engine/src/sanitize.cpp:149`). The test encodes
`((writeId * 31) ^ i) & 0x7FFF` — values up to 32767. Clamping destroys the
relation `map[i] == map[0] ^ i` that assertion 3 checks. Measured: the invariant
fails for **99,608 of 100,000 writeIds (99.61%)**.

**Why it nearly always passes anyway.** The final persisted map is usually the
prepared `setState` payload built from `makeNoteMap(1u)` — product 31, every
entry ≤ 127, so nothing clamps and the relation holds. Only when an injected map
with a large writeId wins the final race does the clamp bite. That is the
rarity, and it has nothing to do with the exchange.

**This explains every observation at once**, which the race hypothesis did not:
ThreadSanitizer silent (there is no race), AddressSanitizer silent (there is no
memory error), roughly one night in 34 (an inject has to land last), and
unaffected by `076f545`'s triple buffer (the tear was never in the exchange).

**Consequence for M005/S02.** GP12 describes "a plausible real memory or
threading defect" in plugin code. For Group B that is now disproved. The defect
is in `tests/host/host_tests.cpp`'s assertion 3, which asserts an invariant the
serialization layer does not preserve. The fix belongs in the test, not in
`plugin/source/`, and S02's plan — written to audit a triple buffer — needs
repair before it is executed.
