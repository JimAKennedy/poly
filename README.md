# Poly

Polymetric drum pattern generator -- VST3 instrument outputting MIDI.

[![CI](https://github.com/JimAKennedy/poly/actions/workflows/ci.yml/badge.svg)](https://github.com/JimAKennedy/poly/actions/workflows/ci.yml)

Poly generates evolving polyrhythmic grooves from 4-8 independent rhythmic lanes.
Each lane runs its own cycle length and Euclidean pattern, creating interlocking
rhythms that shift and recombine over time. The plugin outputs MIDI note events to
your DAW -- no audio processing, just patterns.

## Features

- **Euclidean rhythms** -- each lane distributes pulses across its cycle using the Euclidean algorithm
- **Genre presets** -- West African, Afro-Cuban, Gamelan, Balkan, electronic, and more
- **Cross-rhythm visualization** -- real-time display of how lanes align and diverge
- **Envelope shaping** -- velocity curves driven by cycle phase and macro controls
- **Swing and humanize** -- per-lane timing offsets and velocity variation
- **MIDI capture and export** -- record generated patterns as Standard MIDI Files
- **Scene system** -- save and recall complete lane configurations
- **Macro controls** -- single-knob morphing across density, complexity, and energy

## Building

Requires CMake 3.14+ and a C++20 compiler (Clang, GCC, or MSVC).

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The VST3 SDK is fetched automatically via CMake FetchContent.

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Engine-only build (no VST3 SDK)

The engine is a standalone C++ library with zero VST3 dependencies:

```bash
cmake -S . -B build -G Ninja -DPOLY_ENGINE_ONLY=ON
cmake --build build
```

## Architecture

The core engine (`poly_engine`) is isolated from the plugin layer (`poly_plugin`).
The engine compiles and passes all tests without the VST3 SDK. The plugin feeds
transport and parameter state to the engine and drains its `NoteEvent` output to
the DAW's MIDI event list.

See `IMPLEMENTATION_PLAN.md` for the full architecture and domain model.

## DAW compatibility

Primary target: **Cubase**. Should work with any VST3-compatible host.

## Guide

The `site/` directory contains a Starlight-based guide covering polyrhythmic
traditions, Euclidean rhythm theory, and how to use Poly's preset system.

## License

[GPLv3](LICENSE). Copyright 2024-2026 Jim Kennedy.
