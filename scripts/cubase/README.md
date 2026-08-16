---
class: gated
---

# Cubase launch/quit machinery (M042 S07)

PowerShell scripts the `Cubase Nightly (L4)` workflow
(`.github/workflows/cubase-nightly.yml`) calls on the self-hosted Windows
runner to drive Cubase-in-the-loop. They are the reusable substrate S08 builds
on when it adds the fixture `.cpr`, the mido transport driver, and the MIDI
Remote `ready` ping.

Cubase has no scripting/automation API, so control is deliberately coarse:
open via file-association, poll for a window, close via `CloseMainWindow` with
a hard-kill fallback. See `docs/testing-strategy.md` §3.1 for why.

## Scripts (invoked in this order)

| Script | Phase | Responsibility |
|---|---|---|
| `kill-stale-cubase.ps1` | kill-stale | Terminate any lingering Cubase before a run so state is clean. Idempotent. |
| `clear-safe-mode-flag.ps1` | clear-safe-mode-flag | Delete Cubase's `ApplicationStarted.txt` sentinel BEFORE launch so Cubase sees a clean prior shutdown and never pops the Safe Mode dialog. Root-cause fix (our quit always hard-kills, so the sentinel is never cleared and Safe Mode would otherwise fire every run). No-op when absent; never fails the run. |
| `launch-cubase.ps1` | launch | Resolve `Cubase<ver>.exe`, export `POLY_PROBE_OUTPUT`, open the fixture (or empty project in S07). Does not wait. With `-EnableCdp` it also sets `WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS` so the editor's WebView2 opens a CDP port — and **refuses to launch if the shell is elevated**, because WebView2 discards that flag for elevated hosts (see below). |
| `dismiss-safe-mode.ps1` | dismiss-safe-mode | Backstop for the Safe Mode dialog if the flag-clear above ever misses (e.g. a future Cubase changes the sentinel). Presses OK, keeps current preferences. No-op on a clean launch; never fails the run. NB: confirmed on the runner that SendKeys does NOT reliably close this modal — the flag-clear is the primary fix. |
| `wait-for-ready.ps1` | wait-ready | Block until Cubase presents a settled main window (rejecting the Safe Mode modal), or fail loud on a bounded timeout. S08 extends this to wait on the MIDI Remote `ready` ping. |
| `quit-cubase.ps1` | quit | Graceful `CloseMainWindow`, then hard-kill fallback so the runner is left clean. |
| `archive-logs.ps1` | archive | Collect Cubase prefs/logs, crash dumps, and probe JSONL into the artifact dir. |
| `focus-editor-cdp.ps1` | focus-editor-cdp | S09 gate: poll the OS TCP listen table until the editor's CDP endpoint is up on `127.0.0.1:<port>`, fail loud on timeout. Despite the name it does **not** force foreground — CDP is focus-independent. |
| `diagnose-editor-window.ps1` | — | Diagnostic (non-fatal): dump session/window-station/lock state, **process elevation**, the Cubase window topology, every `msedgewebview2.exe` command line, and the CDP listener state to `editor-window-topology.txt`. Read this first when the CDP port is missing. |
| `launch-manual-cdp.ps1` | — | Hand-launch Cubase on the fixture with the CDP port open, for diagnosis outside a CI run. Not used by the workflow. Must be run from a non-elevated shell. |
| `_common.ps1` | — | Shared helpers: structured phase logging, durable status/error persistence, Cubase path/process resolution. Dot-sourced by each script; not run directly. |

## The runner must not be elevated

WebView2 ignores browser flags delivered "via the local device environment" —
the `WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS` env var *and* the Edge/WebView2
registry policy keys — whenever the host app runs elevated
([Microsoft docs](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags)).
An elevated Cubase therefore cannot expose the CDP endpoint the L4-web e2e
attaches to, which is the whole of the M042 S09 dead end (runs #40–#47: 0 of 18
`msedgewebview2.exe` children carried `--remote-debugging-port`). The
`GitHubActionsRunner` logon task is registered `-RunLevel Limited` for this
reason; `launch-cubase.ps1` fails loud rather than launching a Cubase that
cannot work. See `docs/windows-test-runner-setup.md` Part 12.

## Observability

Every phase writes a structured JSONL line to
`<ArtifactDir>/cubase-run-status.jsonl` (`ts`, `phase`, `state`, `detail`, plus
domain fields). A terminal failure also writes `cubase-last-error.json`. Both
land in the uploaded `cubase-nightly-artifacts`, so an unattended failure is
diagnosable without shelling into the runner — read the status JSONL to see
which phase failed and why.

`wait-for-ready.ps1`'s timeout is the load-bearing safety property: the
unattended nightly must never hang the single runner. On timeout it fails loud,
the `quit` phase still runs (workflow `if: always()`), and the next run is not
blocked.

## Parameters worth knowing

- **Cubase version** is parameterized (`-CubaseVersion`, default target
  **14** per M042 S07). `_common.ps1`'s `Get-CubaseExePath` resolves the
  standard `C:\Program Files\Steinberg\Cubase <ver>\` layout.
- **`POLY_PROBE_OUTPUT`** is exported by `launch-cubase.ps1` so
  `poly_midi_probe` (`tools/midi_probe/`, S06) flushes captured MIDI as JSONL
  from within `process()` during playback (the file is on disk before the
  runner hard-kills Cubase; the transport-stop edge and deactivate are fallback
  triggers). The probe also writes a `probe-status.txt` sidecar next to the
  JSONL recording what it saw (env var, process-call/event counts, whether the
  stop edge fired, last flush result); `archive-logs.ps1` collects both.
- **`POLY_FIXTURE_CPR`** is empty in S07 (clean launch/quit only). S08 sets it
  to the committed fixture project and the smoke becomes a transport + probe
  run.

## Local dry-run

The non-Cubase logic (arg parsing, path building, status logging, timeout math)
runs on any machine with PowerShell 7. `kill-stale`, `wait-for-ready` (against
a nonexistent process, hitting the timeout path), and `archive-logs` (with no
sources present) all exercise cleanly without Cubase installed — useful for
verifying the plumbing before the runner-gated run.
