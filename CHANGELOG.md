# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Fixed

- **The Cubase nightly runs unattended again: the L4-web CDP e2e no longer needs a human to launch Cubase.** Root cause of the M042 S09 dead end (runs #40–#47, where 0 of 18 `msedgewebview2.exe` children ever carried `--remote-debugging-port` while a hand-launched Cubase exposed it every time) was that the runner's `GitHubActionsRunner` logon task ran with `-RunLevel Highest`, so Cubase inherited high integrity — and [WebView2 discards browser flags delivered via the environment or the registry for elevated host apps by design](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags). Passing the flag "via code" instead is documented to survive elevation but is blocked by an open regression making the DevTools endpoint unreachable for elevated hosts on WebView2 Runtime ≥ 150 ([WebView2Feedback#5640](https://github.com/MicrosoftEdge/WebView2Feedback/issues/5640); the runner is on 151.x), so a non-elevated Cubase is the only supported configuration. The runner task is now registered `-RunLevel Limited`, the workflow installs the plugin to the per-user VST3 folder (`%LOCALAPPDATA%\Programs\Common\VST3`) that a non-admin can write, the `schedule:` trigger and automated launch are restored, and the `manual_cubase` dispatch input plus the `await-manual-cubase.ps1` operator gate are gone. `launch-cubase.ps1 -EnableCdp` and `launch-manual-cdp.ps1` now refuse to run elevated, the install step fails the job on a stale machine-wide bundle shadowing the fresh one, and `diagnose-editor-window.ps1` records process elevation in `editor-window-topology.txt`. (M042 S09)

### Changed

- **Factory presets were conformed for musical accuracy and their downstream artefacts regenerated to match.** The engine preset definitions in `engine/src/presets.cpp` were edited across S01–S04: the Kotekan Interlock polos was thinned to E(2,3) (hits 3→2) with its emphasis probability re-baked to 1.0, the Agbekor support drums were de-saturated, and neutralized default macros were removed. The engine-emitted `site/src/generated/presets.json` and the committed Try-It WASM engine (`webui/poly_engine.{wasm,js}`, which compiles `presets.cpp` in) were regenerated from the conformed engine in one reviewed pass so the site's Play card and Try-It agree on every edited preset (Balinese Kotekan, Tintal Groove, Rupak Tal, Minimal Techno, Classic Funk, Jungle Break, Liquid Drum and Bass, Afro-Electronic Fusion, Balkan Funk). The determinism golden fixtures use a synthetic patch independent of factory presets and were re-verified byte-identical, not regenerated. (M070)
- **The Euclidean onset generator is now Bjorklund's algorithm everywhere — engine, WASM, and site — replacing the engine's prior Bresenham distribution.** `engine/src/euclidean.cpp` now computes onsets with an RT-safe Bjorklund implementation that matches Toussaint's documented spellings (E(3,8), E(5,8), E(5,16), E(3,7), E(4,9), E(7,12)), and the site's diagram and Try-It playback now import one shared `site/src/audio/bjorklund.ts` so the browser cannot fork from the engine. The Try-It WASM engine (`webui/poly_engine.{wasm,js}`) and the engine-emitted `presets.json` were regenerated against the new generator, and the golden determinism fixtures were re-verified against it (they were already consistent). **Migration is lossless:** `kStateVersion` was bumped to 16 and a per-lane rotation is applied on load so any pre-switch (v15) saved state re-derives its original per-lane onsets under the new Bjorklund spelling — existing projects sound identical after the upgrade. (M068)
- **WebUI Timeline mode toggle now seeds the step pattern from the lane's current Euclidean configuration** (steps × hits × rotation) instead of starting empty. Enables the Euclidean-approximation-then-manual-refinement workflow: set steps/hits/rotation to get a close Euclidean starter (e.g. E(5,16) for son clave), flip Timeline mode to inherit those hits as an editable step grid, then shift one or two steps to reach the exact traditional pattern. Applies to the plugin (`web_ui_view.cpp`) and the site's mock host equivalently. (M047 S01)
- **WebUI is now the shipping DAW editor on macOS and Windows.** `POLY_WEB_UI` CMake default flipped from `OFF` to `ON` on the shipping platforms so `cmake -S . -B build` (no override) now produces the choc-webview UI. On Linux the default stays `OFF` — choc's WebKitGTK backend needs `libgtk-3-dev` + `libwebkit2gtk-4.1-dev` and CI toolchain wiring that Poly doesn't currently carry, and Poly ships no Linux VST3 binary anyway. M054 will decide whether to add the Linux WebUI toolchain or drop Ubuntu from CI entirely. The legacy VSTGUI editor from `plugin/source/ui/*` remains buildable on all platforms via `-DPOLY_WEB_UI=OFF` until M053 completes the feature-parity audit and decommissions it. (M052 S02)

### Added

- **MIDI export is now Format-1 with one named track per active lane, per-lane export, an accurate host-tempo meta, and a hardened VLQ writer.** `engine/src/smf_writer.cpp` gained `writeMultiTrackSMF()` (a conductor track plus one `FF 03`-named `MTrk` per lane) and `engine/src/lane_name.cpp` supplies the GM lane/note labels; `renderPatternToSMF()` takes a `laneFilter` so the WebUI Export chip and the new per-lane export/drag affordance can emit a single lane. The exported tempo meta reflects the host tempo the plugin last observed (`UISnapshot.tempoBpm`) instead of a hardcoded 120, and `writeVLQ` clamps values ≥ 2²⁸ rather than overflowing. (M032)
- **M032's "export a .mid, open in Cubase, confirm named tracks / no dropped notes" manual UAT is now automated in two layers.** Level C is an independent-parser validator (`tests/cubase/validate_smf_export.py`, using `mido`) that asserts the Format-1 / named-GM-track / no-loss / host-tempo contract against a real `.mid` rendered by the new `poly_smf_emit` tool through the same `renderPatternToSMF` primitive the Export chip uses — it runs in the Cubase-free `engine-isolation` CI job and is itself unit-tested. Level A drives the shipping in-plugin export inside Cubase over CDP (`tests/cubase/e2e/export-midi.spec.ts`): an env-gated `POLY_EXPORT_SINK` hook makes the Export chip and per-lane export write their exact SMF bytes to a file and skip the modal Save-As panel (which would hang the unattended runner), and the spec validates each captured file with the Level C parser. The export e2e runs in the unattended Cubase nightly alongside the toggle-step e2e. (M032)
- **Manual per-step pattern editing is reachable from the WebUI.** Each lane's pattern pane now has a **Timeline mode** toggle at the top. Flipping it into timeline mode replaces the Euclidean stepper controls with a step grid; clicking any step paints its on/off state directly, using the same C++ bridge that the accent row and micro-timing bars already use. Flipping back returns to Euclidean mode. Works identically in the DAW plugin and on the site preview since both surfaces share `webui/*`. (M052 S02)

### Docs

- **Chapter 1 (Foundations) now correctly dates the Bjorklund algorithm to 2003, and chapter 7 (Balkan) now identifies E(4,9) as *the* daichovo horo grouping (not "close to").** Chapter 1 previously called Bjorklund's algorithm "1960s nuclear physics" — off by ~40 years. Corrected per finding D4 of the 2026-07-16 product review: the algorithm was described in Bjorklund's 2003 technical note SNS-NOTE-CNTRL-100 (Los Alamos National Laboratory) for the Spallation Neutron Source timing system, and identified by Toussaint in 2004 as producing the same onset patterns as world rhythm. New reference ref-46 added to the appendix pointing at Bjorklund's Semantic Scholar entry. Chapter 7 previously said E(4,9) produces `2+2+3+2` "close to the standard daichovo grouping." Corrected per finding D5: at the rotation used elsewhere in the site, E(4,9) is `2+2+2+3` — which IS the standard daichovo grouping. The chapter previously contradicted its own line 16 definition of daichovo. New cross-link to the Euclidean reference appendix reinforces the match. (M047 S03)
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
