# Poly

Polymetric drum pattern generator — VST3 instrument outputting MIDI.

## Architecture

Poly is a **MIDI-only VST3 instrument** that generates evolving polyrhythmic grooves from 4-8 independent rhythmic lanes. It outputs MIDI note events to Cubase (primary DAW target) via the VST3 output `IEventList`.

**Engine isolation** is the core architectural principle: `poly_engine` is a pure C++ static library with zero VST3/audio-thread dependencies. The plugin layer (`poly_plugin`) feeds it transport/parameter state and drains its `NoteEvent` output. The engine must compile and pass all tests without the VST3 SDK.

See `ARCHITECTURE.md` for the current architecture. Active roadmap is public
[GitHub milestones](https://github.com/JimAKennedy/poly/milestones) +
`CHANGELOG.md`; `IMPLEMENTATION_PLAN.md` is archived Phase 0 planning.

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

### jk-standards Check Configuration
- Patterns in `jk-standards.yaml` (`boundaries.rules[].forbid`, `count_drift.triggers`,
  `status_prose.forbidden_extra[].pattern`) are **Python `re`**, not POSIX or PCRE
- POSIX classes like `[[:space:]]` are not supported — Python parses them as a
  literal character set (`[`, `:`, `s`, `p`, `a`, `c`, `e`) and reports only a
  `FutureWarning`, so the rule still passes while silently matching nothing
- **A green check is not evidence a rule works.** Before trusting a new rule,
  prove it is non-vacuous: confirm it produces zero matches where it should pass
  *and* non-zero matches somewhere it should fire. The engine-isolation rule was
  verified as 0 matches under `engine/`, 15 under `plugin/source/`
- Match include directives rather than bare identifiers when forbidding a
  dependency: `engine/include/poly/params_def.h` legitimately names
  `Steinberg::Vst::ParamID` in a comment, and a bare-word rule would ban the
  comment along with the dependency

### The e2e Gate Rewrites Committed Artifacts

`scripts/site-verify-local.sh` — the `e2e` validation token — rebuilds the WASM
engine and copies `poly_engine.js` and `poly_engine.wasm` over the committed
copies in `webui/`. Running it always leaves those two files dirty.

- **Never `git add -A` after running `e2e`.** Stage explicit paths. A milestone
  that touches no engine source has no business committing a new `.wasm`, and
  the sweep is easy to miss in a large commit.
- The rebuild is **not byte-reproducible** on every machine: a run with no
  source change moved the `.wasm` by 137 bytes and added a trailing-whitespace
  line to the `.js` that `pre-commit` then strips — so `e2e` and `format`
  interfere, and `pre-commit run --all-files` fails until the artifacts are
  restored.
- Restore with `git checkout origin/main -- webui/poly_engine.js
  webui/poly_engine.wasm` before committing, unless a deliberate engine change
  is being shipped.

Tracked as [#282](https://github.com/JimAKennedy/poly/issues/282).

### Pre-Push Quality Gate
The pre-push hook (`scripts/pre-push-check.sh`) blocks direct pushes to `main`,
then runs ten gates:

| # | Gate | Token |
|---|---|---|
| 0 | Build config — reconfigures `build/` if it would skip host tests | — |
| 1 | clang-format on staged C++ (falls back to the full tree) | — |
| 2 | RT safety — `scripts/check-realtime-safety.sh` | `rt-safety` |
| 3 | CodeSnippet region markers | `snippet-regions` |
| 4 | Build + ctest | `unit` |
| 5 | pluginval (strictness 5 locally, 8 in CI) | — |
| 6 | Doc-conformance guardrail suite | `doc-conformance` |
| 7 | Site unit tests | `site-unit` |
| 8 | Doc discipline, including `doc-drift` | `doc-discipline` |
| 9 | Repo guards — SPDX, personal paths, READMEs, manifests, bridge schema | `guards` |

Install via: `pre-commit install -t pre-push`
Bypass for emergencies: `git push --no-verify`

**The rule the hook follows: the local gate is everything CI runs, minus what
genuinely cannot run locally.** The exclusions are `e2e`, `webui-e2e`,
`wasm-freshness`, `cubase-harness`, and `engine-isolation` — the first four need
a deployed URL, a browser stack, or a DAW, and the fifth configures a second
build tree for minutes. Each is declared with its reason beside its token in
`.jk/validations.yml`, and `site/tests/doc-conformance-wiring.test.mjs` fails if
a token is neither run by the hook nor declared exempt. The rule is enforced,
not merely written here — which matters, because the sentence this replaced
claimed the hook covered five items when the script had seven steps.

Gates 8 and 9 were CI-only until M007. Three consecutive ships went green
locally and red in CI on checks no local command could run: M005 on `doc-drift`,
M006 on `check-scripts-readme`, M002 on the params-json emitter. Running them
costs about six seconds against a hook that already builds the plugin.

Two notes on the commands behind them:

- `bash scripts/check-guards.sh` runs the guards from CI's `code-quality` and
  `site-lint` jobs, plus `check-release-workflow.mjs` — 27 tests locking the
  release workflow's shape that, until M007, were executed by nothing at all.
- `bash scripts/check-doc-discipline.sh` wraps the doc gate. Run it rather than
  `jk-standards all` directly: the bare command skips `doc-drift` whenever no
  base ref is available, printing a `skipped` line and exiting 0. In a wall of
  green that reads like a pass, and it is how M005 shipped a doc-drift violation
  through a validation set that was green on every other count. The wrapper
  supplies the base CI supplies, so the check actually runs.

Note: GitHub branch protection requires Pro for private repos. The pre-push hook is the local enforcement mechanism until then.

### Compiler Warnings
Uses `jk_warnings.cmake` from `cmake/` — `-Wall -Wextra` (GCC/Clang), `/W4` (MSVC) from day one.

## Related Projects

- **audio-meta** (`~/dev/audio-meta`) — cross-project coordination, design system, shared CMake modules
- **Design system** — `~/dev/audio-meta/design/` has brand tokens, color/type/spacing CSS variables, component reference implementations

## No drumcore dependency

Poly's polymetric engine uses variable-length cycles (`LaneConfig`) which don't map to drumcore's fixed `[10][32]` bar-grid model. The engine is independent. A thin adapter for MIDI export interop may come later.

## Delivery Workflow

Programme planning lives in **delivery ledgers** under `docs/plans/<slug>/`, in the
format defined by jk-standards' ledger standard and enforced by its `ledger`
check: milestones own slices, slices own rows, and each slice carries a
definition of done plus the validation tokens it owes. The file is the state,
git is the history — there is no database to keep in step.

Validation tokens are named in a ledger and resolved to real commands by
`.jk/validations.yml`. A slice normally owes `format` plus whichever
subject-matter tokens its change touches (`unit`, `rt-safety`, `doc-conformance`,
`e2e`, …), with `gate` reserved for shipping.

The `/jk:*` commands drive the loop — `assess` a vision or audit document into a
ledger, `plan` a slice, `next` one task at a time, `ship` the milestone, `close`
it. They are vendored via `jk-standards install-commands` and pinned in
`skills-lock.json`; `/jk:status` reads the current state without writing.

Commits that implement a ledger carry `Plan:`, `Slice:` and `Rows:` trailers, so
the join between the tree and the plan is a `git log --grep` away in either
direction.

## Persisting What You Learn

Insights outlive the task that produced them, so write them where they will be
read again rather than into a scratch note:

- **Architectural decisions** — `ARCHITECTURE.md`, or the ledger row that records
  the choice and what verified it
- **Gotchas and conventions** — the Key Conventions section above; a gotcha that
  cost an hour once will cost it again
- **Reusable patterns** — the relevant doc under `docs/`, or a skill in
  `jk-standards` when the pattern is portable beyond poly
