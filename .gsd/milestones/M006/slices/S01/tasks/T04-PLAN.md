---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Write headless host smoke test

Write a smoke test that creates HeadlessUIHost with PolyController factory, calls open(), asserts success, reads a parameter value, then closes. Verifies the headless host lifecycle works.

## Inputs

- `tests/ui/interaction/headless_ui_host.h`
- `plugin/source/controller.h`

## Expected Output

- `tests/ui/interaction/interaction_smoke_tests.cpp`

## Verification

ctest --test-dir build -R interaction_smoke --output-on-failure passes
