---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Generate and commit baseline reference PNGs

Run the visual tests in baseline-generation mode (no reference exists yet = save as reference), verify PNGs are valid, commit them to tests/ui/visual/references/

## Inputs

- `tests/ui/visual/visual_smoke_tests.cpp`

## Expected Output

- `tests/ui/visual/references/lane_grid_default.png`
- `tests/ui/visual/references/velocity_default.png`
- `tests/ui/visual/references/lane_grid_high_complexity.png`
- `tests/ui/visual/references/velocity_high_density.png`

## Verification

ls tests/ui/visual/references/*.png && file tests/ui/visual/references/*.png
