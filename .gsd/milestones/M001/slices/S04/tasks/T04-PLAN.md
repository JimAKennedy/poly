---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Automation Mapping Specification

Write docs/automation-mapping.md specifying which parameters are exposed to Cubase automation (lane velocity, probability, macro controls) vs which remain internal compound controls. Define ParamID ranges and normalized/plain value conventions.

## Inputs

- `plugin/source/controller.h`
- `plugin/source/plugids.h`
- `IMPLEMENTATION_PLAN.md`

## Expected Output

- `docs/automation-mapping.md`

## Verification

ParamID ranges don't conflict; exposed vs internal distinction is clear
