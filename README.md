# Poly

Polymetric drum pattern generator -- VST3 instrument outputting MIDI.

[![CI](https://github.com/JimAKennedy/poly/actions/workflows/ci.yml/badge.svg)](https://github.com/JimAKennedy/poly/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/JimAKennedy/poly/graph/badge.svg)](https://codecov.io/gh/JimAKennedy/poly)

**[Read the guide at poly.jk.digital](https://poly.jk.digital)** -- polyrhythmic traditions, Euclidean rhythm theory, and preset walkthroughs.

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

This is the only supported Linux build (see [Supported platforms](#supported-platforms)).

## Supported platforms

Poly ships as a plugin on **macOS and Windows** only. Linux is an
**engine/WASM-only** target: the `poly_engine` library compiles and its tests
run on Linux (and the engine cross-compiles to WebAssembly for the web guide),
but no shipping Linux VST3 is built. Rebuilding on Linux gives you a fast
engine-only compile with no VST3 SDK or UI dependencies — use
`-DPOLY_ENGINE_ONLY=ON` as shown above.

This is decision **D029** (M054): Poly ships no Linux VST3 binary, so the Linux
CI leg is an engine/WASM portability compile rather than a full plugin build.
See `CHANGELOG.md` for the full scope statement.

## Installing a release

Download the `.zip` for your platform from a
[GitHub Release](https://github.com/JimAKennedy/poly/releases), unzip it, and
copy `poly_plugin.vst3` into your VST3 folder:

- **macOS** — `~/Library/Audio/Plug-Ins/VST3/` (per-user) or
  `/Library/Audio/Plug-Ins/VST3/` (all users)
- **Windows** — `C:\Program Files\Common Files\VST3\`
- **Linux** — no plugin zip is published. Poly is **engine/WASM-only** on Linux
  (decision **D029**), so there is no Linux VST3 to download — don't go looking
  for one in the GitHub Release. Build the engine locally with
  `-DPOLY_ENGINE_ONLY=ON` (see [Supported platforms](#supported-platforms)).

On macOS and Windows the zip extracts to a top-level `poly_plugin.vst3/` bundle;
copy that whole bundle directory (not its loose contents) into the VST3 folder
above.

### macOS Gatekeeper (unsigned builds)

Release builds are signed and notarized by Apple **only when the maintainer has
provisioned the Developer ID signing secrets** (see
[Signing and notarization](#signing-and-notarization)). Until then the shipped
bundle is **unsigned**, so macOS attaches a quarantine flag to the downloaded
`.zip` and Gatekeeper blocks the plugin — Cubase silently drops it from the
scan, or you get a "cannot be opened because the developer cannot be verified"
dialog.

Clear the quarantine flag on the extracted bundle before copying it in:

```bash
xattr -dr com.apple.quarantine poly_plugin.vst3
```

`-d` deletes the attribute, `-r` recurses into the bundle. Once a signed +
notarized + stapled release ships, this step is unnecessary — Gatekeeper
accepts the stapled bundle with no prompt.

## Signing and notarization

The macOS release leg (`.github/workflows/release.yml`) auto-signs, notarizes,
and staples the `.vst3` **the moment the six repository secrets below are
provisioned** — no code change required. Each signing step is gated on its
secrets being non-empty (`env.MACOS_* != ''`); while the secrets are absent the
steps **skip** and the leg ships an unsigned zip (see the Gatekeeper note
above). This is decision **D031** (M030 S03), which revises D004's
unsigned-forever deferral.

Provision these under **Settings → Secrets and variables → Actions** in the
GitHub repo:

| Secret | What it is |
|--------|-----------|
| `MACOS_CERTIFICATE_P12_BASE64` | Base64 of the exported *Developer ID Application* certificate + private key (`.p12`). Export from Keychain Access, then `base64 -i cert.p12 \| pbcopy`. |
| `MACOS_CERTIFICATE_PASSWORD` | The password set when exporting the `.p12`. |
| `MACOS_SIGNING_IDENTITY` | The codesign identity string, e.g. `Developer ID Application: Your Name (TEAMID)`. |
| `MACOS_NOTARY_APPLE_ID` | Apple ID email used for notarization. |
| `MACOS_NOTARY_PASSWORD` | An **app-specific password** for that Apple ID (appleid.apple.com → Sign-In and Security → App-Specific Passwords), *not* the account password. |
| `MACOS_NOTARY_TEAM_ID` | Your Apple Developer Team ID (the `TEAMID` in the identity string above). |

To provision:

1. Enroll in the [Apple Developer Program](https://developer.apple.com/programs/)
   and create a *Developer ID Application* certificate.
2. Export it from Keychain Access as a `.p12` (certificate **and** private key),
   set an export password, and base64-encode the file.
3. Generate an app-specific password for the notarization Apple ID.
4. Add all six secrets to the repo, then push a `v*.*.*` tag. The
   **Codesign / Notarize / Staple VST3 (macOS)** steps run after pluginval and
   before packaging; `notarytool submit --wait` fails the leg if Apple rejects
   the submission, so a bad build never ships mislabeled.

## Architecture

The core engine (`poly_engine`) is isolated from the plugin layer (`poly_plugin`).
The engine compiles and passes all tests without the VST3 SDK. The plugin feeds
transport and parameter state to the engine and drains its `NoteEvent` output to
the DAW's MIDI event list.

For current architecture see `ARCHITECTURE.md`. Active work is tracked in the
public [GitHub milestones](https://github.com/JimAKennedy/poly/milestones) and
`CHANGELOG.md`. `IMPLEMENTATION_PLAN.md` is archived Phase 0 planning kept for
historical context only.

## DAW compatibility

Primary target: **Cubase**. Should work with any VST3-compatible host.

## Guide

**[poly.jk.digital](https://poly.jk.digital)** -- the full guide covering polyrhythmic
traditions, Euclidean rhythm theory, and how to use Poly's preset system. Source is in `site/`.

## Contributing

Contributions are welcome. Start here:

- **[ROADMAP.md](ROADMAP.md)** — the public, issue-backed roadmap grouping open
  work by theme and priority, so you can see where the project is headed.
- **[CONTRIBUTING.md](CONTRIBUTING.md)** — prerequisites, build setup, code
  style, real-time safety rules, and the fork-branch-PR workflow.
- **[Good first issues](https://github.com/JimAKennedy/poly/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)**
  — scoped, self-contained tasks that are reviewable without deep engine
  context. The best place to make a first contribution.

## License

[GPLv3](LICENSE). Copyright 2024-2026 Jim Kennedy.
