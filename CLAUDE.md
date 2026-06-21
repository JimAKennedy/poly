# Poly

Polymetric drum pattern generator — VST3 instrument outputting MIDI.

## Architecture

Poly is a **MIDI-only VST3 instrument** that generates evolving polyrhythmic grooves from 4-8 independent rhythmic lanes. It outputs MIDI note events to Cubase (primary DAW target) via the VST3 output `IEventList`.

**Engine isolation** is the core architectural principle: `poly_engine` is a pure C++ static library with zero VST3/audio-thread dependencies. The plugin layer (`poly_plugin`) feeds it transport/parameter state and drains its `NoteEvent` output. The engine must compile and pass all tests without the VST3 SDK.

See `IMPLEMENTATION_PLAN.md` for the full architecture, domain model, and phase plan.

## Tech Stack

- **C++20** (trial before wider jk.digital adoption — C++17 is the current portfolio standard)
- VST3 SDK 3.7+, VSTGUI 4
- CMake 3.14+, Google Test
- clang-tidy, clang-format

## Key Conventions

### Real-Time Safety
- No heap allocation, locks, exceptions, or I/O in `process()` or `renderRange()`
- Pre-allocate in `initialize()`, only clear/reset in `setActive()`
- `allocateMessage()`/`sendMessage()` in `process()` is NOT guaranteed lock-free by the SDK — offload to non-RT thread
- Some DAW hosts call `setActive()` from the audio thread — no allocation there either

### State Serialization
- Write `kStateVersion` as the first int32 in `getState()`, branch on version in `setState()`
- Never serialize without a version number — it's a preset compatibility time bomb

### Timing & Determinism
- Derive envelope/cycle phase from absolute PPQ position (`projectTimeMusic`), never accumulate
- Same `(patch, seed, transport)` inputs must produce identical output every time
- Golden tests enforce determinism in CI

### Code Formatting
- **Before every commit**, run `clang-format -i --style=file` on all new/modified `.cpp`/`.h` files
- On macOS the binary is at `/opt/homebrew/opt/llvm/bin/clang-format` (not in PATH by default)
- CI runs `pre-commit run --all-files` which includes clang-format — unformatted files fail the build

### Compiler Warnings
Uses `jk_warnings.cmake` from `cmake/` — `-Wall -Wextra` (GCC/Clang), `/W4` (MSVC) from day one.

## Related Projects

- **audio-meta** (`~/dev/audio-meta`) — cross-project coordination, design system, shared CMake modules
- **Design system** — `~/dev/audio-meta/design/` has brand tokens, color/type/spacing CSS variables, component reference implementations

## No drumcore dependency

Poly's polymetric engine uses variable-length cycles (`LaneConfig`) which don't map to drumcore's fixed `[10][32]` bar-grid model. The engine is independent. A thin adapter for MIDI export interop may come later.

## Memory Auto-Capture

After completing a task, evaluate whether insights should be persisted:
- **Decisions** — architectural choices, library selections
- **Gotchas** — non-obvious pitfalls
- **Conventions** — coding patterns, naming rules
- **Patterns** — reusable approaches

Use `capture_thought` to store in `.gsd/gsd.db`. Categories: `architecture`, `convention`, `gotcha`, `pattern`, `preference`, `environment`.
