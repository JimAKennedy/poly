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
- clang-format is pinned via `pre-commit/mirrors-clang-format` in `.pre-commit-config.yaml` — do not change version without verifying CI compatibility
- On macOS the local binary is at `/opt/homebrew/opt/llvm/bin/clang-format` (not in PATH by default)
- CI runs `pre-commit run --all-files` which includes clang-format — unformatted files fail the build

### MSVC Portability
- Always `#include <algorithm>`, `<utility>`, and other standard headers explicitly
- GCC/Clang provide `std::sort`, `std::min`, `std::max` transitively from `<cmath>` or `<vector>` — MSVC does not
- The Windows CI build (`windows-2022`, MSVC) will reject missing includes that compile on macOS/Linux

### Ownership Transfer Annotations
- All `new` expressions in `plugin/source/` that transfer ownership to VST3/VSTGUI must have `// ownership-transfer`
- The NFR review scanner flags unannotated raw `new` — this comment suppresses the finding

### Pre-Push Quality Gate
The pre-push hook (`scripts/pre-push-check.sh`) enforces quality checks automatically:
1. **Blocks direct pushes to main** — use a feature branch and PR instead
2. **clang-format** on all C++ files
3. **RT safety** — `scripts/check-realtime-safety.sh`
4. **Build + tests** — `cmake --build build && ctest --test-dir build`

Install via: `pre-commit install -t pre-push`
Bypass for emergencies: `git push --no-verify`

Note: GitHub branch protection requires Pro for private repos. The pre-push hook is the local enforcement mechanism until then.

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
