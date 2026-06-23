# S01: Fix clang-format CI gate

**Goal:** Resolve the clang-format version mismatch between local and CI so headless_ui_host.cpp passes code-quality
**Demo:** pre-commit run --all-files passes locally and in CI with identical formatting

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [ ] **T01: Fix clang-format disagreement in headless_ui_host.cpp** `est:5min`
  CI clang-format formats `id (*)(Class, SEL)` as `id(*)(Class, SEL)` (no space before `(`). Our local LLVM 22.1.7 preserves the space. Fix by running CI-compatible formatting on the file. Lines 144 and 148 need `id (*)` changed to `id(*)` to match CI clang-format output.
  - Files: `tests/ui/interaction/headless_ui_host.cpp`
  - Verify: /opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror tests/ui/interaction/headless_ui_host.cpp

- [ ] **T02: Pin clang-format version in pre-commit config** `est:15min`
  The pocc/pre-commit-hooks clang-format hook uses whatever clang-format is available in the environment. CI (ubuntu-latest) gets a different version than local macOS (LLVM 22.1.7), causing formatting disagreements. Either:
  1. Pin the hook to use a specific clang-format version via `additional_dependencies` or a version arg
  2. Switch to a different clang-format hook that allows version pinning (e.g. pre-commit/mirrors-clang-format)
  3. Or accept CI's version as authoritative and ensure local matches
  - Files: `.pre-commit-config.yaml`
  - Verify: pre-commit run clang-format --all-files exits 0 locally

## Files Likely Touched

- tests/ui/interaction/headless_ui_host.cpp
- .pre-commit-config.yaml
