---
estimated_steps: 5
estimated_files: 1
skills_used: []
---

# T02: Pin clang-format version in pre-commit config

The pocc/pre-commit-hooks clang-format hook uses whatever clang-format is available in the environment. CI (ubuntu-latest) gets a different version than local macOS (LLVM 22.1.7), causing formatting disagreements. Either:
1. Pin the hook to use a specific clang-format version via `additional_dependencies` or a version arg
2. Switch to a different clang-format hook that allows version pinning (e.g. pre-commit/mirrors-clang-format)
3. Or accept CI's version as authoritative and ensure local matches

Investigate the best approach and implement.

## Inputs

- `.pre-commit-config.yaml`
- `CI environment clang-format version`

## Expected Output

- `.pre-commit-config.yaml with pinned or version-controlled clang-format`

## Verification

pre-commit run clang-format --all-files exits 0 locally
