---
id: M002
title: "Core Groove Engine MVP"
status: complete
completed_at: 2026-06-20T23:51:07.851Z
key_decisions:
  - Absolute PPQ phase derivation — no accumulators, eliminates drift on loop/transport jumps
  - Engine isolation — poly_engine has zero VST3 SDK dependency, builds and tests independently
  - Macro 0.5 passthrough — all macros at default 0.5 produce unmodified lane parameters
  - Per-lane MIDI channels — each lane routes to its own MIDI channel (0-7) for DAW flexibility
  - kStateVersion first int32 — version-stamped serialization for preset compatibility
key_files:
  - engine/include/poly/engine.h
  - engine/include/poly/types.h
  - engine/include/poly/envelope.h
  - engine/include/poly/macro.h
  - engine/include/poly/bridge.h
  - engine/include/poly/state_io.h
  - engine/src/engine.cpp
  - engine/src/envelope.cpp
  - engine/src/macro.cpp
  - engine/src/bridge.cpp
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/ui/lane_grid_view.cpp
  - plugin/source/ui/velocity_view.cpp
  - tests/golden_tests.cpp
  - tests/macro_tests.cpp
  - tests/envelope_tests.cpp
  - tests/swing_humanize_tests.cpp
  - tests/dynamic_shaping_tests.cpp
  - tests/plugin_tests.cpp
lessons_learned:
  - C++20 features (std::span, designated initializers) worked well with VST3 SDK and CMake toolchains — no friction encountered
  - Golden determinism tests catch subtle regressions across the full pipeline (velocity + envelope + swing + macro) that unit tests miss
  - Engine isolation as a CI job prevents accidental VST3 SDK coupling early
  - Pre-commit RT safety checks (grep-based) catch obvious violations but aren't exhaustive — consider clang-tidy custom checks for deeper coverage
---

# M002: Core Groove Engine MVP

**Complete polymetric groove engine with velocity pipeline, envelopes, swing/humanize, macro controls, VST3 bridge, and VSTGUI editor — 101 tests, deterministic output, RT-safe**

## What Happened

M002 delivered a musically useful polymetric groove generator across 6 slices and 25 tasks. The engine generates evolving polyrhythmic patterns from 4-8 independent lanes with distinct Euclidean rhythms, dynamic velocity shaping (accents, emphasis, ghost notes), multi-period envelope modulation (sine, ramp, triangle, square), per-lane swing and humanize, and 6 coherent macro controls. The VST3 plugin bridge reads transport state, maps parameters, emits MIDI via IEventList with per-lane channels, and serializes state with version stamping. The VSTGUI editor provides real-time lane grid and velocity visualization. All timing derives from absolute PPQ position for deterministic loop/transport behavior. 101 tests across 11 suites verify correctness, and CI enforces RT safety, engine isolation, and golden determinism on every push.

## Success Criteria Results

All 8 success criteria pass: transport stability (PPQ-absolute timing), seed determinism (14 golden tests), UI lane correspondence (VSTGUI editor), velocity variation (dynamic shaping), state round-trip (versioned serialization), RT safety (no alloc/lock/throw in process), macro coherence (19 macro tests), envelope evolution (12 envelope tests).

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

Cubase DAW integration testing (manual UAT), preset library, additional envelope shapes, performance profiling under high lane counts, MIDI timing accuracy tests across sample rates
