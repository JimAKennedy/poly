# S04: Specification and Design

**Goal:** Complete Phase 0 documentation: PRD, engine specification, Cubase workflow guide, automation mapping spec, and UI wireframes.
**Demo:** Complete docs/ directory with PRD, engine-spec, Cubase workflow guide, wireframe images, and automation mapping specification

## Must-Haves

- All five docs exist in docs/, are internally consistent with the implemented engine, and reference concrete types/APIs from the codebase.

## Proof Level

- This slice proves: Manual review — docs reference real types and match implemented behavior.

## Integration Closure

Docs form the authoritative Phase 0 reference for Phase 1 implementation work.

## Verification

- None — documentation only.

## Tasks

- [x] **T01: Product Requirements Document** `est:20min`
  Write docs/PRD.md synthesizing the roadmap vision into a formal requirements document covering scope, user stories, lane architecture, velocity/dynamics model, envelope system, and MVP success criteria.
  - Files: `docs/PRD.md`
  - Verify: File exists, references match roadmap.md and IMPLEMENTATION_PLAN.md

- [x] **T02: Engine Technical Specification** `est:25min`
  Write docs/engine-spec.md covering the poly_engine contract: timing model (PPQ-derived, no accumulation), cycle math, Euclidean rhythm generation, velocity/emphasis pipeline, envelope superposition design, and the renderRange() API. Reference actual types from engine/include/poly/.
  - Files: `docs/engine-spec.md`
  - Verify: All referenced types exist in engine/include/poly/; spec matches implemented renderRange() behavior

- [x] **T03: Cubase Workflow Guide** `est:15min`
  Write docs/cubase-workflow.md documenting the canonical Cubase routing and recording workflow: instrument track setup, MIDI output routing, recording generated patterns, freeze/export, and monitoring configuration.
  - Files: `docs/cubase-workflow.md`
  - Verify: Document covers load, route, record, export workflow steps

- [x] **T04: Automation Mapping Specification** `est:15min`
  Write docs/automation-mapping.md specifying which parameters are exposed to Cubase automation (lane velocity, probability, macro controls) vs which remain internal compound controls. Define ParamID ranges and normalized/plain value conventions.
  - Files: `docs/automation-mapping.md`
  - Verify: ParamID ranges don't conflict; exposed vs internal distinction is clear

- [x] **T05: UI Wireframe Descriptions** `est:20min`
  Write docs/wireframes.md with text-based wireframe layouts for the three primary views: lane overview (4-8 lane grid with macro row), focused lane editor (per-lane detail), and envelope editor. Include ASCII art layouts showing widget placement and information hierarchy.
  - Files: `docs/wireframes.md`
  - Verify: All three views described; layout references match lane/envelope/macro concepts from PRD

## Files Likely Touched

- docs/PRD.md
- docs/engine-spec.md
- docs/cubase-workflow.md
- docs/automation-mapping.md
- docs/wireframes.md
