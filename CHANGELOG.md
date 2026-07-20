# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Changed

- **WebUI Timeline mode toggle now seeds the step pattern from the lane's current Euclidean configuration** (steps × hits × rotation) instead of starting empty. Enables the Euclidean-approximation-then-manual-refinement workflow: set steps/hits/rotation to get a close Euclidean starter (e.g. E(5,16) for son clave), flip Timeline mode to inherit those hits as an editable step grid, then shift one or two steps to reach the exact traditional pattern. Applies to the plugin (`web_ui_view.cpp`) and the site's mock host equivalently. (M047 S01)
- **WebUI is now the shipping DAW editor on macOS and Windows.** `POLY_WEB_UI` CMake default flipped from `OFF` to `ON` on the shipping platforms so `cmake -S . -B build` (no override) now produces the choc-webview UI. On Linux the default stays `OFF` — choc's WebKitGTK backend needs `libgtk-3-dev` + `libwebkit2gtk-4.1-dev` and CI toolchain wiring that Poly doesn't currently carry, and Poly ships no Linux VST3 binary anyway. M054 will decide whether to add the Linux WebUI toolchain or drop Ubuntu from CI entirely. The legacy VSTGUI editor from `plugin/source/ui/*` remains buildable on all platforms via `-DPOLY_WEB_UI=OFF` until M053 completes the feature-parity audit and decommissions it. (M052 S02)

### Added

- **Manual per-step pattern editing is reachable from the WebUI.** Each lane's pattern pane now has a **Timeline mode** toggle at the top. Flipping it into timeline mode replaces the Euclidean stepper controls with a step grid; clicking any step paints its on/off state directly, using the same C++ bridge that the accent row and micro-timing bars already use. Flipping back returns to Euclidean mode. Works identically in the DAW plugin and on the site preview since both surfaces share `webui/*`. (M052 S02)

### Docs

- **Chapter 8 (Minimalism) now prints Reich's *Clapping Music* pattern correctly and drops the false E(8,12) attribution.** Previously the chapter printed the pattern with only 7 claps (`x x x . x x . x . x . .`, missing the eighth clap at position 10) and claimed it was E(8,12). Corrected per finding D3 of the 2026-07-16 product review: Reich's actual 8-clap pattern is `x x x . x x . x . x x .` with gap sequence 1-1-2-1-2-2-1-2. It uses the same gap-value palette (1s and 2s) as E(8,12) but the three consecutive claps at positions 0, 1, 2 break Bjorklund's strict alternation — a run that no E(k,n) with n=12 produces at any rotation. The prose now teaches this as a compositional counter-example rather than a Euclidean claim. (M047 S02)
- **Chapter 3 (Afro-Cuban) and the Euclidean-reference appendix now correctly identify son clave and rumba clave as non-Euclidean patterns.** Previously chapter 3 claimed 'son clave 3-2 is exactly E(5,16)' and the appendix's 16-Steps table showed an arithmetically impossible grouping `3+3+2+4+3+1` (six intervals for five onsets) attributed to `Bossa nova bass / son clave`. Corrected per findings D1 and D2 of the 2026-07-16 product review: the son clave (gaps 3-3-4-2-4) contains a 2-gap that Euclidean distribution cannot produce; E(5,16)'s true grouping is 3-3-4-3-3 and its cultural home is bossa nova bass alone. Chapter 3 now teaches the seed-and-adjust workflow using the new Timeline mode seed behavior — start from E(5,16), flip Timeline mode, shift one step to reach the exact son clave. Both chapters cite Toussaint via existing footnotes. (M047 S01)
- **Chapters 2 (Sub-Saharan Africa), 4 (Afrobeat), and 10 (Brazilian) now link to chapter 18's timeline-mode workflow** from the exact paragraphs where they describe fixed bell patterns and locked ensemble parts, so readers can go straight from the musical context to the step-drawing UI. Chapter 3 (Afro-Cuban clave) got its own hook in M047 S01. (M052 S04)

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
