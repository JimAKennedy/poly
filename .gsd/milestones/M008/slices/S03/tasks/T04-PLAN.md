---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Tests for micro-timing maps

Add tests: (1) per-step timing offset shifts PPQ correctly, (2) micro-timing + swing compose, (3) micro-timing + humanize compose, (4) micro-timing + additive cells compose, (5) all-zero map produces no change (regression), (6) golden test for groove-mapped pattern.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `Micro-timing test cases pass`
- `Golden test for groove-mapped pattern`

## Verification

cmake --build build && ctest --test-dir build
