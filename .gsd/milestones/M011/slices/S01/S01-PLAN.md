# S01: Fix clang-format CI gate

**Goal:** Fix all code defects causing CI failures: pin clang-format version to eliminate local/CI mismatch, reformat files to match, and fix MSVC cross-platform build failure
**Demo:** pre-commit run --all-files passes locally and in CI with identical formatting

## Must-Haves

- pre-commit run --all-files passes locally; cmake build succeeds on macOS; no clang-format version mismatch between local and CI

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Pin clang-format to mirrors-clang-format with explicit version** `est:15min`
  Replace pocc/pre-commit-hooks clang-format hook with pre-commit/mirrors-clang-format which allows pinning to a specific LLVM release. Pin to a version available on both CI Ubuntu runners and local macOS. Remove the ci:skip for clang-format since the version mismatch will be resolved. Also remove pocc/pre-commit-hooks entirely if clang-format was its only hook.
  - Files: `.pre-commit-config.yaml`
  - Verify: pre-commit run clang-format --all-files exits 0 locally

- [x] **T02: Reformat all C++ files with pinned clang-format version** `est:10min`
  After pinning the version, run clang-format on ALL .cpp/.h files to normalize formatting. The id(*) vs id (*) spacing in headless_ui_host.cpp:144,148 will be resolved by this pass. This is a one-time bulk reformat to establish the baseline with the pinned version.
  - Files: `tests/ui/interaction/headless_ui_host.cpp`
  - Verify: pre-commit run clang-format --all-files exits 0

- [x] **T03: Fix missing includes for MSVC cross-platform build** `est:15min`
  Add #include <algorithm> to tests/euclidean_tests.cpp:1 (needed for std::sort at line 175). Scan all test and source files for other std:: usages that rely on transitive includes which MSVC may not provide (std::sort, std::find, std::min/max without <algorithm>, std::move without <utility>, etc.).
  - Files: `tests/euclidean_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build --output-on-failure

- [x] **T04: Run full local verification (pre-commit + build + test)** `est:10min`
  Run the complete pre-commit suite and build+test locally to confirm all code-quality and compilation issues are resolved before pushing.
  - Verify: pre-commit run --all-files && cmake --build build && ctest --test-dir build

## Files Likely Touched

- .pre-commit-config.yaml
- tests/ui/interaction/headless_ui_host.cpp
- tests/euclidean_tests.cpp
