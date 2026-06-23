---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Add KNOWLEDGE.md rules for all discovered issues

Add rules to KNOWLEDGE.md: R6: All new expressions in plugin/source/ must have // ownership-transfer when ownership transfers to VST3/VSTGUI. R7: Always include <algorithm> and other standard headers explicitly; MSVC does not provide them transitively. R8: clang-format version is pinned in .pre-commit-config.yaml; do not change without verifying CI compatibility. R9: nfr-review.yaml skip rules must have rationale comments.

## Inputs

- `.gsd/KNOWLEDGE.md`

## Expected Output

- `KNOWLEDGE.md with R6-R9 rules`

## Verification

grep -c 'R[6-9]' .gsd/KNOWLEDGE.md shows 4 new rules
