---
id: T04
parent: S01
milestone: M006
key_files:
  - tests/ui/interaction/interaction_smoke_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T01:00:08.817Z
blocker_discovered: false
---

# T04: Interaction smoke tests verify HeadlessUIHost lifecycle and macro knob discovery with PolyController

**Interaction smoke tests verify HeadlessUIHost lifecycle and macro knob discovery with PolyController**

## What Happened

Wrote interaction_smoke_tests.cpp with two tests: ControllerLifecycle opens HeadlessUIHost with PolyController factory, asserts open succeeds, reads MacroComplexity parameter value, verifies CFrame is non-null, then closes. MacroKnobExists verifies the Complexity knob can be found by its control tag. Both tests include GTEST_SKIP for non-macOS platforms. Initial segfault in HostApplication constructor was resolved by linking sdk_hosting instead of manually compiling hostclasses.cpp.

## Verification

Both interaction smoke tests pass on macOS

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build -R InteractionSmokeTest --output-on-failure` | 0 | 2/2 passed | 100ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/ui/interaction/interaction_smoke_tests.cpp`
