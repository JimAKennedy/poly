---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Engine API Surface Audit

Review the poly_engine public API: types, functions, constants. Assess completeness against PRD requirements, identify missing APIs for Phase 1 (envelope evaluation, macro resolution, accent application, humanization), and flag any API design concerns.

## Inputs

- `engine/include/poly/engine.h`
- `engine/include/poly/types.h`
- `engine/include/poly/euclidean.h`
- `engine/include/poly/rng.h`
- `docs/PRD.md`

## Expected Output

- `docs/review/api-audit.md`

## Verification

All public headers reviewed; Phase 1 gaps identified with specific API suggestions
