---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Implement cross-rhythm view (VSTGUI CView)

Create cross_rhythm_view.h/cpp as a CView subclass. Draws horizontal lanes with step markers at computed PPQ positions. Highlights convergence points. Reads lane configs via tag-based parameter access. Handles both equal and additive cell widths.

## Inputs

- `plugin/source/ui/phase_alignment_view.h`
- `plugin/source/ui/phase_alignment_view.cpp`

## Expected Output

- `cross_rhythm_view.h/cpp`

## Verification

cmake --build build && ctest --test-dir build
