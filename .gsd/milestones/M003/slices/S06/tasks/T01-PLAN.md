---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T01: Envelope curve custom view

Implement EnvelopeCurveView (VSTGUI CView subclass) that draws the current envelope shape as a filled curve. Takes envelope shape, period, depth, and current phase as inputs. Renders shape waveform with a vertical phase-position marker. Register in controller createCustomView.

## Inputs

- `plugin/source/ui/lane_grid_view.h`
- `engine/include/poly/envelope.h`

## Expected Output

- `plugin/source/ui/envelope_curve_view.h`
- `plugin/source/ui/envelope_curve_view.cpp`

## Verification

cmake --build build && ctest --test-dir build
