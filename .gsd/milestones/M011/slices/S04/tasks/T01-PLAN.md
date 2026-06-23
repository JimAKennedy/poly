---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T01: Add pre-push Git hook via pre-commit framework

Add a pre-push stage to .pre-commit-config.yaml that runs: (1) clang-format on modified C++ files, (2) check-realtime-safety.sh, (3) cmake build + ctest. This enforces the pre-push checklist automatically. Since GitHub branch protection requires Pro for private repos, this is our local enforcement mechanism. Document the limitation and suggest upgrading to Pro or making the repo public when branch protection becomes needed.

## Inputs

- `.pre-commit-config.yaml`
- `scripts/check-realtime-safety.sh`

## Expected Output

- `.pre-commit-config.yaml with pre-push hooks`
- `scripts/pre-push-check.sh if needed`

## Verification

git push to a test branch triggers the pre-push hook and blocks on failure
