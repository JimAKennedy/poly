---
class: gated
---

# M005 — decisions

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
