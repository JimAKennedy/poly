---
class: gated
---

# S08 runner install scripts

Run-once helper scripts for the `JIMW1` Cubase runner that automate the
retype-heavy steps of the M042 S08 walkthrough, so the owner runs one command
per step in a VNC session instead of retyping. They prepare everything *around*
the fixture; only Cubase can author the `.cpr` itself.

Companion to `tests/cubase/fixtures/README.md` (the fixture-authoring recipe)
and `docs/windows-test-runner-setup.md` (the full runner runbook).

## Prerequisites

- The loopMIDI `poly-test` virtual port exists (loopMIDI enumerates it as
  `poly-test 1`; the substring matcher tolerates the suffix).
- Poly and `poly_midi_probe` are built under `build/` (Release). To build:
  ```powershell
  cmake -S . -B build -G "Visual Studio 17 2022" -DSMTG_RUN_VST_VALIDATOR=ON
  cmake --build build --config Release --parallel
  ```

## Scripts

Run from any directory (each resolves the repo root from its own location).

| Script | What it does |
|---|---|
| `0-install-all.ps1` | Runs steps 1-4 then preflight, in order. The one-command path. |
| `1-sync-main.ps1` | `git fetch`/`checkout main`/`pull`; asserts the port-match fix is present. |
| `2-install-plugins.ps1` | Copies `poly_plugin.vst3` + `poly_midi_probe.vst3` from `build/` into the VST3 dir Cubase loads from. |
| `3-install-midi-remote.ps1` | Copies `JkDigital_PolyTest.js` into Cubase's MIDI Remote driver-scripts tree. |
| `4-install-driver-deps.ps1` | `pip install` the mido driver deps; checks a `poly-test` port is visible. |
| `5-preflight.ps1` | Read-only verifier: branch, both VST3 bundles, MIDI Remote script, loopMIDI port, Cubase 14, golden. Green = ready to author the `.cpr`. |
| `6-keep-runner-awake.md` | Config changes that keep the runner's console session live/unlocked overnight so the `schedule:` nightly's MIDI Remote handshake works unattended. Complements the runbook's Part 7. Not a script — a checklist to apply and later tidy up. |

## Order of operations

1. Create the loopMIDI `poly-test` port (once).
2. Build Poly + `poly_midi_probe` (once, or after code changes).
3. `pwsh scripts/S08-install/0-install-all.ps1` — installs + preflights.
4. If preflight is green: author the fixture in Cubase per
   `tests/cubase/fixtures/README.md`, save as
   `tests/cubase/fixtures/poly-4bar.cpr`, commit, and land on `main`.
5. Set `POLY_FIXTURE_CPR` in the workflow and dispatch the nightly.

## What these scripts do NOT do

- **Author the `.cpr`** — only Cubase can write one; follow the recipe by hand.
- **Set `POLY_FIXTURE_CPR` / dispatch** — that is a workflow edit + an Actions
  dispatch (the exit-criterion run the owner records).
- **Build** — they install from an existing `build/`; build first if needed.

## Cross-references

- `tests/cubase/fixtures/README.md` — the fixture-authoring recipe.
- `tests/cubase/midi-remote/README.md` — the MIDI Remote script + port contract.
- `tests/cubase/driver/README.md` — the mido driver these deps support.
- `docs/windows-test-runner-setup.md` — the full runner provisioning runbook.
- `.github/workflows/cubase-nightly.yml` — the nightly these scripts prepare for.
