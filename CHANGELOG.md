# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Changed

- **WebUI is now the shipping DAW editor.** `POLY_WEB_UI` CMake default flipped from `OFF` to `ON` so `cmake -S . -B build` (no override) now produces the choc-webview UI. The legacy VSTGUI editor from `plugin/source/ui/*` remains buildable via `-DPOLY_WEB_UI=OFF` until M053 completes the feature-parity audit and decommissions it. (M052 S02)

### Added

- **Manual per-step pattern editing is reachable from the WebUI.** Each lane's pattern pane now has a **Timeline mode** toggle at the top. Flipping it into timeline mode replaces the Euclidean stepper controls with a step grid; clicking any step paints its on/off state directly, using the same C++ bridge that the accent row and micro-timing bars already use. Flipping back returns to Euclidean mode. Works identically in the DAW plugin and on the site preview since both surfaces share `webui/*`. (M052 S02)

## [0.1.0] - 2026-06-27

Initial open-source release of Poly, a polymetric drum pattern generator outputting MIDI via VST3.

### Added

#### Engine
- Euclidean rhythm generator with 4–8 independent lanes, each with configurable steps, pulses, and rotation
- Additive/aksak cell support for variable-width Euclidean rhythms (e.g. 2+2+3)
- Dynamic shaping: accent masks, emphasis probability, and ghost note floor per lane
- Envelope superposition system with 8 targets (velocity, density, pulse count, rotation, accent, ghost, swing, humanize), supporting Curve and StepList shapes
- Constraint layer: anchors, backbeat protection, and density guardrails
- A/B scene system with morph interpolation for live parameter blending
- Macro controls: complexity, density, syncopation, swing, tension, humanize — with transition smoothing
- Per-lane swing, humanize, and note duration controls
- Phrase gating with per-lane length/gap/offset controls in beats
- Per-lane pattern mutation with deterministic per-cycle variations
- Per-lane phase drift with deterministic PPQ-based rotation
- Kotekan pair mode and timing offset knob
- Per-step micro-timing maps for groove templates
- Timeline mode for fixed-pattern lanes immune to macro changes
- Deterministic output: same (patch, seed, transport) inputs produce identical MIDI every time

#### Plugin
- VST3 instrument with 16-channel MIDI output via IEventList
- Full Cubase automation: VST3 unit hierarchy, parameter formatting, clean naming
- RT-safe MIDI capture buffer and Standard MIDI File (SMF) writer for export
- State serialization with version stamping (kStateVersion) for preset compatibility
- Per-lane timing offset for groove pocket feel

#### UI
- VSTGUI editor with lane grid, velocity display, and design system alignment
- Phase and envelope visualization: envelope curves, phase alignment, lane indicators
- Lane edit view exposing core Euclidean parameters (steps, pulses, rotation)
- Cross-rhythm visualization showing only active lanes
- Cross-view lane selection highlighting with drift-aware phase display
- Phrase schematic visualization with beat labels
- Editable lane names
- Unified LANE section combining lane edit and phrase views

#### Presets
- Factory presets: Afrobeat, Bossa Nova, Drill, Funk, House, Jazz, Latin, Reggaeton, Techno, UK Garage, and more
- Genre presets showcasing phrase/mutation/drift/kotekan features

#### Infrastructure
- 3-platform CI (macOS, Linux, Windows) with pre-commit hooks and sanitizers (ASan, TSan, UBSan)
- NFR review workflow (nightly + per-PR) replacing CodeQL
- Pre-push quality gate: clang-format, RT safety check, build + test
- Headless UI interaction tests and visual regression framework
- 229 automated tests including golden output determinism tests
- Guide site built with Astro/Starlight at poly.jk.digital
