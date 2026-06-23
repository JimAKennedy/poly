---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Skip cmake-build-config, structure-weak-boundary, and informational rules

Add justified skips to nfr-review.yaml for: (1) cmake-build-config (8 amber) — subdirectory CMakeLists inherit from root; (2) structure-weak-boundary (6 amber) — engine/plugin boundary is intentional and tested by engine-isolation CI job; (3) sample-readme-exists (1 amber) — not applicable to VST3 plugin project; (4) otel-test-observability (1 amber) — not applicable to C++ GTest suite. Each skip gets a comment explaining the rationale.

## Inputs

- `nfr-review.yaml`
- `current amber finding list`

## Expected Output

- `nfr-review.yaml with all amber rules skipped with rationale`

## Verification

yamllint nfr-review.yaml or manual inspection of skip list
