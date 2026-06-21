---
id: T03
parent: S01
milestone: M006
key_files:
  - tests/ui/visual/visual_smoke_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T01:00:04.429Z
blocker_discovered: false
---

# T03: Visual smoke tests render LaneGridView and VelocityView to offscreen bitmaps and save PNGs

**Visual smoke tests render LaneGridView and VelocityView to offscreen bitmaps and save PNGs**

## What Happened

Wrote visual_smoke_tests.cpp with two tests: LaneGridViewRenders creates a LaneGridView with an initialized PolyController, renders via renderViewToBitmap(), asserts non-null bitmap, saves output PNG. VelocityViewRenders does the same for VelocityView. Both views require a valid controller for getParamNormalized() calls during draw(). Created controller directly (new PolyController) to avoid VST3 diamond inheritance ambiguous cast issue.

## Verification

Both visual smoke tests pass, output PNGs generated in build/tests/visual_output/

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build -R VisualSmokeTest --output-on-failure` | 0 | 2/2 passed | 100ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/ui/visual/visual_smoke_tests.cpp`
