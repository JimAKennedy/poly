# Poly Testing Strategy — Plugin, Bridge, and Cubase-in-the-Loop

Status: proposed (2026-07-03), written against `main@63c960b`. Companion to
`docs/webui-migration.md` (this document expands its W5 phase into a full
strategy and adds the host-integration layers).

## 0. The shape of the problem

One hard constraint drives the whole design: **Cubase has no scripting or
automation API.** Steinberg provides nothing equivalent to ReaScript or
Bitwig's controller API, so "launch Cubase, load a project, drive the
transport, assert on output" cannot be done through a supported interface.
It can be done — but through side channels (MIDI remote control, CLI
project-open, a capture plugin) on dedicated hardware, and it will always
be the most expensive suite to run and maintain.

Therefore: a pyramid. Coverage lives below Cubase; Cubase verifies only
what *only Cubase* can break.

```
            ┌──────────────────────────────┐   nightly, self-hosted runner,
            │ L4  Cubase e2e (5–10 flows)  │   ~minutes, narrow + golden
            ├──────────────────────────────┤
            │ L3  Host-integration          │   pluginval + VST3 validator +
            │     (no Cubase)               │   in-process test host, CI
            ├──────────────────────────────┤
            │ L2  Bridge contract           │   Playwright (JS side) +
            │     (web UI ↔ C++)            │   gtest (C++ side), shared
            ├──────────────────────────────┤   fixtures, CI, seconds
            │ L1  Engine + unit             │   exists today: 300+ tests,
            │                               │   golden determinism, fuzz
            └──────────────────────────────┘
```

What exists today: L1 is strong (engine tests, golden determinism, state-IO
fuzzing, sanitize tests); the webui Playwright suite (8 tests) exists but
is not in CI; the VSTGUI visual/interaction harness covers the legacy
editor; the VST3 validator is available but disabled
(`SMTG_RUN_VST_VALIDATOR OFF`); nothing exercises a real host.

## 1. L2 — Bridge contract testing (web UI ↔ C++, no Cubase, no webview)

**Principle: never test "through" a real webview in CI.** WKWebView needs a
window server, WebView2 needs Windows, and coupling Playwright to a C++
build makes both suites brittle. The bridge already has a natural seam —
the JSON protocol in `webui/bridge-schema.md` — so cut there and test each
side against **shared fixtures**.

### 1.1 The contract artifacts

```
webui/tests/fixtures/
├── schema/bridge.schema.json      # JSON Schema for every message type
├── js-to-native/                  # recorded UI-emitted messages
│   ├── edit-macro-density.jsonl   #   begin/perform/end gesture triplet
│   ├── action-toggle-step.jsonl
│   └── ...
└── native-to-js/                  # canned state/frame pushes
    ├── state-afrobeat.json        # full State snapshot (real preset)
    ├── state-8-lanes.json
    └── frame-playing.json
```

### 1.2 JS side (Playwright — extends the existing suite)

- Test page sets `window.__POLY_EMBEDDED__ = true` and installs a **stub
  `polyHostCall`** that records outbound messages; a helper replays
  `native-to-js/*` fixtures through `window.polyHostPush`.
- Assertions in both directions:
  - interaction → message: "dragging the density macro emits exactly the
    begin/perform/end triplet in `edit-macro-density.jsonl`" (golden
    comparison, tolerant of value floats).
  - state → render: "`state-8-lanes.json` renders 8 strips with correct
    hues/ladders".
- Every recorded and replayed message is validated against
  `bridge.schema.json` with **ajv** in a test-global hook.

### 1.3 C++ side (gtest — new `bridge_tests.cpp`)

- Introduce a ~5-method shim interface between `WebUIView` and choc
  (`IWebViewShim`: `evaluate(js)`, `setCallHandler(fn)`, `navigate`,
  `bind`, view handle). Production implements it over choc; tests inject a
  mock.
- Tests feed the mock the *same* `js-to-native/*` fixtures and assert the
  controller/processor state (param edits with correct gesture calls,
  actions landing in the selected scene via the M029 message paths).
- `pushState()` output is captured from the mock and validated against
  `bridge.schema.json` (use **valijson** or nlohmann/json-schema — pick one
  and vendor/FetchContent-pin it) plus semantic asserts (lane counts,
  preset name).
- Round-trip property test: State-serialize → parse → compare against the
  `GrooveState` field-by-field for the fields the schema carries.

**Definition of done:** a bridge change that breaks either side fails CI in
seconds, with the failing fixture named. No webview, no display, no SDK
host needed beyond what the plugin target already builds.

### 1.4 CI wiring (immediate, no dependencies)

Add to `ci.yml`: a `webui-tests` job — `npm ci && npx playwright test` in
`webui/` (cache `~/.cache/ms-playwright`; pin `@playwright/test` exactly,
as browser/package version skew breaks launches). Runs on ubuntu-latest in
~1 minute. The C++ contract tests join the existing `poly_tests` target.

## 2. L3 — Host integration without Cubase

### 2.1 Conformance (turn on this week)

- **VST3 validator**: flip `SMTG_RUN_VST_VALIDATOR ON` for CI plugin
  builds; it runs as a post-build step and fails the build on conformance
  errors.
- **pluginval** (Tracktion, free, cross-platform, headless): CI job that
  downloads the release binary and runs
  `pluginval --strictness-level 8 --skip-gui-tests <poly.vst3>` on the
  built artifact, macOS and Windows. Covers state round-trips under
  stress, threading, editor lifecycle, automation sweeps — the classic
  "works in my host" gaps.

### 2.2 The in-process test host (`tools/testhost/`) — the workhorse

This is where "load the plugin, change config, control the transport,
capture MIDI, check expected results" is *best* implemented — fully
deterministic and CI-runnable. Build a small host on the SDK's hosting
classes (`public.sdk/samples/vst-hosting/*` is the reference
implementation):

- Load the built `.vst3` module via `VST3::Hosting::Module`, instantiate
  processor + controller, connect them with `ConnectionProxy` (this
  exercises the REAL message paths — ApplyPreset, cellSizes, timeline,
  micro-timing, envelopes — exactly as a host would).
- Drive `process()` with scripted `ProcessData`/`ProcessContext`
  **transport scenarios**: clean start; stop mid-note; loop of N bars
  crossing cycle boundaries; song-position jump forward/backward; tempo
  change mid-phrase; `projectTimeMusic` invalid; buffer sizes 32→4096;
  sample rates 44.1k–192k; offline (freeze/export) flag.
- Collect the output `IEventList` per block into an event log; assert
  against **golden MIDI files** (Poly's determinism makes these
  byte-stable for a fixed seed/preset/tempo).
- Scenario definition as data (YAML/JSON in `tools/testhost/scenarios/`),
  so new cases are fixtures, not code.

Priority scenarios (each is a known bug class from the 2026-07 review era):
stop-flush (no hanging notes), loop-restart determinism (identical events
every pass), jump → chain state correctness, preset apply → array state
audible (aksak cells, bembé timeline), scene B editing, morph rendering,
note-map applied to both live output and capture, export extraction while
stopped.

### 2.3 What L3 cannot see

Cubase's actual event/timing quirks, focus and keyboard behavior, its
editor lifecycle, WebView2-in-Cubase interactions, version upgrades.
That — and only that — is L4's job.

## 3. L4 — Cubase-in-the-loop

### 3.1 Building blocks

1. **Project fixtures (`tests/cubase/fixtures/`)**: committed `.cpr` files —
   Poly on an instrument track, routed to the probe (below), one fixture
   per supported Cubase major version (Cubase rewrites project format
   per version; keep them pristine and read-only in the runner).
   Launching `Cubase <path>.cpr` opens the project (file-association
   argument; there are no further documented CLI flags).
2. **Capture: `poly_midi_probe` — a purpose-built logger plugin.** A
   trivial second VST3 (event input only) that appends
   `{ppqPosition, pitch, velocity, channel}` JSONL to a path from an env
   var, timestamped from the host's own `ProcessContext`. Placed
   downstream of Poly in the fixture, it gives sample-accurate,
   in-host capture with no OS MIDI routing flakiness. (~1 day to build;
   reuses the SDK boilerplate Poly already has.)
3. **Control: MIDI, not screen-scraping.** Virtual MIDI port (loopMIDI on
   Windows, IAC on macOS) + a Cubase-side mapping:
   - **MIDI Remote API** (Cubase 12+, JavaScript): transport
     start/stop/locate, track selection, and plugin parameters via focus
     Quick Controls. Poly's seed/preset/macros/lane cores are ordinary
     parameters, so "make config changes" is covered.
   - **Generic Remote (legacy)** where needed: maps MIDI to arbitrary key
     commands (e.g. "Edit Instrument" to open the plugin editor).
   - Test driver: Python + `mido` (~50 lines) that "plays" Cubase.
4. **Keyboard automation as last resort** (AutoHotkey / pywinauto on
   Windows, AppleScript System Events on macOS) for anything unmappable.
   Cubase's custom-drawn UI defeats accessibility-tree automation; key
   commands are the reliable channel. Keep this to project-open dialogs
   and editor-open only.
5. **Forensics**: run under screen recording (ffmpeg/OBS), collect crash
   dumps, upload both as artifacts on failure.

### 3.2 Infrastructure

- **Self-hosted GitHub Actions runner** — a Windows box first (it also
  unlocks L4-web below), optionally a Mac mini second. Cubase activated
  via Steinberg Licensing on the machine; generic low-latency ASIO
  driver; Hub/start dialogs disabled; prefs directory seeded and backed
  up. Label `cubase`.
- **Security**: self-hosted runners must not execute arbitrary PR code.
  Gate the Cubase workflow to `schedule` (nightly) + `workflow_dispatch`
  on main only — never `pull_request`.
- A test run is: kill stale Cubase → launch with fixture → wait for the
  MIDI Remote script's "ready" ping → driver executes scenario →
  assert on probe JSONL → quit Cubase via key command → archive logs.

### 3.3 The golden flows (keep it to ~5–10)

1. Project opens; Poly instantiates; transport plays 8 bars; probe output
   matches the in-process test host's golden for the same preset (the
   cross-check that ties L4 to L3).
2. Loop 4 bars × 4 passes → identical events every pass.
3. Stop mid-note → note-offs arrive (no hang).
4. Preset change via Quick Control → probe shows the new pattern
   (exercises ApplyPreset + array state in a real host).
5. Parameter automation write/read round-trip on one macro.
6. Project save → reopen → state identical (probe output unchanged).
7. (Web UI era) editor opens/closes 10× without crash or focus loss.

## 4. L4-web — Playwright against the real UI inside Cubase

**Windows is the flagship and it genuinely works:** choc uses WebView2
there, and WebView2 honors
`WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=--remote-debugging-port=9222` set in
Cubase's environment. Playwright then attaches with
`chromium.connectOverCDP('http://localhost:9222')` — official Playwright
WebView2 support — and drives the *actual plugin editor running inside
Cubase* while the mido driver runs the transport and the probe captures
output. Golden flow: Playwright toggles a kick step → transport plays →
probe JSONL shows the changed pattern.

**macOS:** WKWebView exposes no CDP. Options, in order of pragmatism:
(a) Windows-only for true Playwright e2e; macOS relies on L2 + L3 + a
native editor-open smoke; (b) later, compile a small WebSocket test hook
into debug UI bundles so a custom driver can dispatch DOM events. Start
with (a).

## 5. Rollout order

| # | Item | Depends on | Effort |
|---|---|---|---|
| 1 | `webui-tests` CI job (existing Playwright suite) | — | 0.5 d |
| 2 | Bridge contract fixtures + JS stub host + schema (ajv) | — | 1–2 d |
| 3 | `IWebViewShim` + C++ contract tests + C++ schema validation | web-UI shell exists (migration W1) | 1–2 d |
| 4 | VST3 validator ON + pluginval CI jobs | — | 0.5 d |
| 5 | In-process test host + transport scenarios + golden MIDI | — | 3–5 d |
| 6 | `poly_midi_probe` plugin | — | 1 d |
| 7 | Cubase fixtures + MIDI Remote script + mido driver | 6, runner | 2–3 d |
| 8 | Self-hosted Windows runner + nightly workflow | hardware | 1–2 d |
| 9 | Playwright-over-CDP e2e golden flows | 7, 8, migration W1 | 2–3 d |

Items 1, 2, 4, 5, 6 have no dependency on the web-UI migration and can
start immediately; items 1+4 are half-day wins.

## 6. Retirement and ownership notes

- The VSTGUI visual/interaction harness (`tests/ui/`) is superseded by L2
  Playwright once the web UI becomes the shipping editor (migration W6);
  until then it keeps guarding the legacy editor.
- Golden MIDI files are the shared truth between L3 and L4 flow #1 —
  regenerate only via the test host with a recorded reason in the commit.
- Fixture `.cpr` files are per-Cubase-version artifacts; opening one in a
  newer Cubase and re-saving is itself a nightly test (project-upgrade
  flow) — do it deliberately, not accidentally.
- The probe plugin, scenario DSL, MIDI Remote script, and mido driver are
  all plugin-agnostic: candidates for `audio-meta` once a second plugin
  needs them.
