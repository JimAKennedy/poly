---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T05: Run NFR review locally and verify zero red findings

Run the NFR review tool locally against the codebase to verify all red findings are resolved and all amber findings are either fixed or skipped. This catches issues before they hit the PR gate.

## Inputs

- `all source files`
- `nfr-review.yaml`

## Expected Output

- `clean NFR review output`

## Verification

local NFR review run shows zero red and zero amber findings
