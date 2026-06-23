---
id: T01
parent: S01
milestone: M011
key_files:
  - .pre-commit-config.yaml
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T14:17:55.877Z
blocker_discovered: false
---

# T01: Verified mirrors-clang-format v22.1.5 already pinned; removed clang-format from ci: skip list

**Verified mirrors-clang-format v22.1.5 already pinned; removed clang-format from ci: skip list**

## What Happened

The pre-commit config already had mirrors-clang-format v22.1.5 in place (replacing pocc/pre-commit-hooks). Removed clang-format from the ci: skip list since the version is now pinned and consistent across environments. The check-pragma-once and check-realtime-safety hooks remain skipped in ci: because they use language:script (local hooks).

## Verification

pre-commit run clang-format --all-files exits 0

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `pre-commit run --all-files` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `.pre-commit-config.yaml`
