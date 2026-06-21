---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Macro knob interaction tests

Test that scrolling/dragging the 6 macro knobs (Complexity, Density, Syncopation, Swing, Tension, Humanize) changes their parameter values. Verify parameter changes appear in getParameterValue() and the edit log.

## Inputs

- `tests/ui/interaction/headless_ui_host.h`
- `plugin/source/plugids.h`

## Expected Output

- `6+ interaction test cases for macro knobs`

## Verification

All macro knob tests pass via ctest -R InteractionSmokeTest
