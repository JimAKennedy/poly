---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Pin clang-format to mirrors-clang-format with explicit version

Replace pocc/pre-commit-hooks clang-format hook with pre-commit/mirrors-clang-format which allows pinning to a specific LLVM release. Pin to a version available on both CI Ubuntu runners and local macOS. Remove the ci:skip for clang-format since the version mismatch will be resolved. Also remove pocc/pre-commit-hooks entirely if clang-format was its only hook.

## Inputs

- `.pre-commit-config.yaml`

## Expected Output

- `.pre-commit-config.yaml with mirrors-clang-format pinned version`

## Verification

pre-commit run clang-format --all-files exits 0 locally
