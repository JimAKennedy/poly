---
estimated_steps: 4
estimated_files: 2
skills_used: []
---

# T01: Add CodeQL SAST scanning to CI or skip with rationale

NFR review flags ci-security-scan-missing (red/high). Options:
1. Add a CodeQL workflow for C++ scanning (GitHub provides free CodeQL for public repos). Configure with proper exclusions for build/_deps/ and VST3 SDK code.
2. Skip the rule in nfr-review.yaml with documented rationale (e.g. sanitizers workflow already covers memory safety; gitleaks covers secrets).

Recommendation: Add a lightweight CodeQL step since it's free and catches real issues. Configure language as 'cpp', exclude build deps and SDK.

## Inputs

- `.github/workflows/ci.yml`
- `nfr-review.yaml`

## Expected Output

- `CI workflow with CodeQL step, or nfr-review.yaml with ci-security-scan-missing skip`

## Verification

yamllint .github/workflows/ci.yml (syntax check); or gh workflow run ci.yml --dry-run if possible
