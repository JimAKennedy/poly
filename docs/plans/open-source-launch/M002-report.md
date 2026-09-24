# M002 — CI a stranger can trust

**Review-gate report.** Generated from the ledger, `git log` and
`M002-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** A contributor's first PR runs in workflows that hold least
privilege, execute no unpinned third-party code, fuzz the inputs strangers
control, and fail on a warning.

**Branch:** `milestone/M002-ci-trust` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M002/S01 | Workflows hold least privilege | OS06, OS07, OS08 | done |
| M002/S02 | The untrusted inputs are fuzzed | OS09, OS10 | done |
| M002/S03 | A warning means something | OS11, OS12, OS13 | done |

Every row is `done`.

## Definition of done

- [x] Every workflow declares a top-level `permissions:` block, and jobs elevate
      locally only where they write
- [x] A superseded push to a PR cancels the run it replaces
- [x] No workflow checks out or executes third-party code at a moving ref
- [x] A guard fails on a workflow with no top-level `permissions:` and on an
      unpinned third-party checkout, each seen red — the three jk-standards
      workflow checks all pass on today's tree, so none of them is that guard
- [x] The fuzz option is declared, documented, and built by a workflow
- [x] Both untrusted inputs `SECURITY.md` names — saved state and a dropped MIDI
      file — have a fuzz target with a seed corpus
- [x] The nightly runs each for a bounded time and files an issue on a crash,
      the same way the sanitizer nightly does
- [x] Each target is shown to find a bug planted on a scratch branch before it
      is trusted
- [x] `poly_engine` builds warning-free on GCC, Clang and MSVC with
      `POLY_WARNINGS_FATAL=ON`, and CI builds it that way
- [x] Every test-side warning that marked a real defect is fixed, not silenced
- [x] MIDI import's handling of pitch is a decision with a test naming it

## What changed

**Every workflow says what its token may do** (OS06). `ci.yml` and
`sanitizers.yml` gain top-level `permissions: contents: read`; `secrets-scan`
and the sanitizer `notify` job keep their job-local elevations. The
repository default was already read-only, so no token changed at runtime;
the grant is now in the tree. `scripts/check-workflow-hygiene.mjs` holds it
there and was red on both files before the blocks existed. It runs in
`check-guards.sh` and as a `code-quality` step in CI.

**A superseded PR push is cancelled** (OS07). `ci.yml` declares a
concurrency group keyed on workflow and ref with `cancel-in-progress` for
pull requests only; a push to `main` is never cancelled. The guard asserts
the exact shape.

**Nothing third-party floats** (OS08). `pr-af-review.yml`'s checkout of
`Agent-Field/pr-af` is pinned to upstream `main` as of 2026-09-21 by SHA,
with the bump rule in a comment; the guard requires a SHA on every
third-party `repository:` checkout in every workflow.

**The fuzz option exists and the nightly runs it** (OS09). `BUILD_FUZZ_TESTS`
is declared beside the sanitizer options; `sanitizers.yml` gains a `fuzz` job
that builds the targets with Clang and ASan, seeds the corpora, runs each
target for 300 seconds, uploads reproducers on failure, and reaches the
existing `notify` job so a crash lands on the `sanitizer-failure` issue. The
whole fuzz build is coverage-instrumented; before that the engine was a black
box to libFuzzer.

**The MIDI reader is fuzzed** (OS10). `fuzz_midi_reader` runs the dropped
bytes through `parseSMF`, the fitter and `importMidiToLane`;
`fuzz_seed_corpus` writes current-format state blobs and engine-written SMFs
at build time. Each target found a planted bug within a second on a scratch
branch that was never pushed.

**An engine warning fails the build** (OS11). 53 warning sites across two
compilers are gone, every fix an explicit cast that preserves the arithmetic
the compiler already performed; the golden tests are unchanged.
`POLY_ENGINE_WARNINGS_FATAL`, `ON` by default, makes `poly_engine` alone
`-Werror`/`/WX` on every build and every CI compiler; `engine-isolation`
passes it explicitly. Proved clean on GCC 15.2, Apple Clang 21 and
Emscripten; a planted sign conversion failed GCC and Clang. The tests, tools
and plugin keep their 63 sites under `POLY_WARNINGS_FATAL=OFF`, counted and
classified in the decisions file.

**The golden phrase test asserts what it computes** (OS12).
`EXPECT_FALSE(bothSilentSomewhere)`, with the bar table that justifies it,
seen red with both lanes on the same phrase.

**A dropped file imports the lane's own note** (OS13). `MidiParseResult`
carries `onsetNotes`; `importMidiToLane` fits only the onsets on the lane's
`midiNote` when the file has any, and every onset otherwise. Three tests
name the decision; the `d1` warning is gone.

## Validation

Re-run on the finished milestone. The head is `00e2419`; its only change over
`7cc363d` is the fuzz token's flag, the `CONTRIBUTING.md` snippet and two plan
documents, so the build tokens that ran on `7cc363d` ran on the same C++ and
CMake this head carries.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `00e2419` |
| `guards` | `bash scripts/check-guards.sh` | pass — 17 guard invocations | `7cc363d` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass — 5 drift mappings satisfied | `00e2419` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `00e2419` |
| `unit` | `cmake --build build … && ctest …` | pass — **701/701** | `7cc363d` |
| `engine-isolation` | `cmake -S . -B build-engine -DPOLY_ENGINE_ONLY=ON … && ctest …` | pass — 572/572 | `7cc363d` |
| `fuzz` | the `fuzz` token in `.jk/validations.yml`, Homebrew LLVM Clang | pass — 10 seeds; 820,529 and 490,958 runs, no crash | `00e2419` |
| `sanitizers` | the five-variant command in `.jk/validations.yml` | pass — 572/572 ×3, 61 + 59, 61 | `7cc363d` |

Every gate above was also run and recorded per task in the three evidence
files, and every guard and assertion this milestone added was seen red
before it was trusted: the hygiene guard on `ci.yml`, `sanitizers.yml` and
`pr-af-review.yml`; both fuzzers on planted bugs; the golden assertion on
identical phrases; the three import tests on the missing field; and the
fatal engine build on a planted conversion under two compilers.

## Traceability

Every commit on the branch carries a `Slice:` line. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `070a1f1` | M002/S01 | | docs(plans): front-load M002's decisions and plan all three slices |
| `b3307e7` | M002/S01 | OS06 | ci: every workflow declares its token's grant, and a guard holds it there |
| `9a16077` | M002/S01 | OS07 | ci: a newer push to a pull request cancels the run it supersedes |
| `6b4c9bc` | M002/S01 | OS08 | ci: the PR-AF checkout names a commit, so nothing third-party floats |
| `92ff906` | M002/S02 | | tests(fuzz): declare BUILD_FUZZ_TESTS, add a MIDI-reader target and a corpus seeder |
| `b892d8e` | M002/S02 | | ci(nightly): fuzz saved state and dropped MIDI files, and file an issue on a crash |
| `c4204c6` | M002/S02 | OS09, OS10 | tests(fuzz): each target finds a planted bug, and the option is documented |
| `f4b58c7` | M002/S03 | OS12 | tests(golden): assert that offset phrases never leave a bar silent on both lanes |
| `b700666` | M002/S03 | OS13 | engine(midi): a dropped file imports the lane's own note, and merges only without it |
| `7cc363d` | M002/S03 | OS11 | engine: warnings are errors, and there are none |
| `00e2419` | M002/S03 | | tests(fuzz): the local fuzz build finds crashes, so it does not gate warnings |

The `Plan:`, `Slice:` and `Rows:` lines sit in the paragraph above the
attribution line, as M001's commits did, so `git log --grep` resolves them
and git's own trailer parser does not. `/jk:ship` builds its body with the
former.

## What a reviewer should look at twice

### The engine is warnings-as-errors for every contributor, by default

`POLY_ENGINE_WARNINGS_FATAL` defaults `ON`. A contributor whose compiler
reports a diagnostic none of GCC 15, Apple Clang 21, Emscripten or MSVC
reports today will see a red engine build; the opt-out is the one option.
This was the owner's choice between "everywhere" and "CI only".

### A dropped MIDI file behaves differently

Before OS13 a multi-instrument file dropped on a lane merged every
instrument's onsets. Now it imports only the lane's own note when the file
carries it. No guide chapter or site claim described the old behaviour, so
nothing in prose changed; the header, the import contract and the wasm
export comment state the rule.

### One escape hatch was added

`engine/src/smf_writer.cpp` carries a GCC-only, file-wide
`-Wfree-nonheap-object` suppression with the reason and the version. GCC
15.2 reports it from inside libstdc++'s vector reallocation on a push_back
loop and a range insert alike; Clang and MSVC report nothing. It is the only
suppression the burndown needed.

### Three plan steps were corrected in flight, each recorded

The fuzz build had to be coverage-instrumented for the engine to be visible
to libFuzzer; the planted state-I/O bug had to be an unvalidated index
rather than the plan's one-past read, which ASan cannot see inside a struct;
and the hygiene guard gained a CI step because `check-guards.sh` is local
only. All three are judgment calls in the decisions file.

### The local fuzz build is the one place the engine is not fatal

Re-running every token on the finished head found the first collision
between S02 and S03: Homebrew LLVM 22's libc++ emits a `#warning` about the
10.15 deployment target, and the engine's new default promoted it. The
`fuzz` token and the `CONTRIBUTING.md` snippet pass
`-DPOLY_ENGINE_WARNINGS_FATAL=OFF` with the reason beside it. The nightly on
Ubuntu keeps the default; `engine-isolation` is the warnings gate. A
judgment call in the decisions file, landed as the branch's last commit.

### The fuzz job has not yet run as a nightly

It is proved by local runs of the same commands, including planted-bug
finds. The first scheduled run is at 03:00 UTC after merge; a
`workflow_dispatch` of `sanitizers.yml` would prove it sooner.

### Two guards M001 added still run only locally

`check-version-source.mjs` and `check-release-workflow.mjs` run in
`check-guards.sh` and nowhere in `ci.yml`, the gap the new guard avoided by
taking a `code-quality` step. Outside this milestone's rows; worth a
one-line follow-up.

### The `fuzz` token was added to S02's ledger line

The planning commit appended `fuzz` to the slice's validation tokens and
declared it pre-push-exempt in `.jk/validations.yml`, because a slice that
adds fuzzers should owe running them. Recorded as taken on the owner's
behalf.

## Decisions

Copied from `M002-decisions.md` so the report stands alone.

# M002 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-24 — planning M002/S01, M002/S02 and M002/S03

Measured before asking, on `main` at `804771d`. `ci.yml` and `sanitizers.yml`
declare no top-level `permissions:`; the repository's default workflow token
permission is already `read`, so a top-level `contents: read` changes nothing
at runtime and makes the grant visible in the tree. `secrets-scan` and the
sanitizer `notify` job carry the only local elevations. `pr-af-review.yml` has
been triggered nine times and every run was skipped — the `pr-af` label has
never been applied; upstream `Agent-Field/pr-af` has no tags and its `main`
was at `421fbd23bf1c5a2c3916d7046c97b5273958e1f4` on 2026-09-21. Engine-only
builds of `poly_engine` alone show **65 unique warning sites** on both GCC 15
and Apple Clang 21: `-Wsign-conversion` in `types.h` (70 diagnostics),
`engine.cpp` (29), `scene.h` (20), `euclidean.cpp` (15), `macro.cpp` (6);
`-Wdouble-promotion` in `rng.h` and `engine.cpp`; int-to-float conversions at
`engine.cpp:567`, `:570` and `envelope.cpp:43`; the unused `d1` in
`midi_reader.cpp`. GCC 15 alone also reports one `-Wfree-nonheap-object`
inside libstdc++'s `vector::push_back`, reached from `smf_writer.cpp`. The
repository holds no `.mid` fixture; the reader tests build SMF bytes in memory
with `writeSMF`. Homebrew's LLVM `clang++` links `-fsanitize=fuzzer`, so a
planted bug can be hunted locally. In `GoldenPhrase.OffsetPhraseBehavior` the
gaps fall in bars 2 and 5 for lane 0 (12-beat phrase) and bar 3 for lane 1
(16-beat phrase), so `bothSilentSomewhere` is never true for that patch.

- **Q:** Where does the guard for OS06 and OS08 live — a local `scripts/`
  contract, an upstream jk-standards extension, or both? — **A:** a local
  guard in `scripts/`.
- **Decision:** `scripts/check-workflow-hygiene.mjs`, a `node --test`
  contract wired into `check-guards.sh` and the `code-quality` CI job; an
  upstream jk-standards issue is filed to port it — **Why:** it lands in this
  pull request with no jk-standards release or pin bump, and a pin bump
  activates whatever else changed upstream.
- **Q:** Pin `pr-af-review.yml`'s third-party checkout, or delete the
  workflow that has never run? — **A:** pin it.
- **Decision:** `ref: 421fbd23bf1c5a2c3916d7046c97b5273958e1f4` with the
  upstream branch and date in a comment, as the row's verification says —
  **Why:** deleting it is a separate decision about the review tooling, one
  row-edit away if the owner wants it later.
- **Q:** When a multi-instrument MIDI file is dropped on a lane, which
  note-ons are imported — the lane's own note with a merge fallback, every
  pitch on purpose, the lane's note only, or the most frequent pitch? —
  **A:** the lane's note, else merge.
- **Decision:** `parseSMF` records the note number beside every onset;
  `importMidiToLane` keeps only the onsets on the lane's `midiNote` when the
  file has any, and otherwise fits every onset as today — **Why:** a full drum
  loop dropped on the kick lane imports the kick, and a single-instrument
  loop on any pitch still imports.
- **Q:** How does `poly_engine` become warnings-as-errors — on every build,
  or only when CI asks? — **A:** on every build.
- **Decision:** a new `POLY_ENGINE_WARNINGS_FATAL` option, default `ON`,
  makes `poly_engine` alone `-Werror`/`/WX` on GCC, Clang, MSVC and
  Emscripten; `POLY_WARNINGS_FATAL` keeps governing tests, tools and the
  plugin and stays `OFF`; the `engine-isolation` job passes the new option
  explicitly as well — **Why:** a contributor sees a new engine warning fail
  before they push, and the option name says what is fatal.

### Taken on the owner's behalf

- **This run targets `open-source-launch` M002.** `engine-capability` M004
  is `in-progress` but its own ledger records that the remaining six slices
  wait on fixtures built by hand at the Cubase runner, and
  `verifiable-references` M003 waits on the owner working a browser
  worklist; neither can run unattended. M002 is next in the programme the
  last four pull requests shipped, and its dependencies are none.
- **The concurrency group is `${{ github.workflow }}-${{ github.ref }}`
  with `cancel-in-progress: ${{ github.event_name == 'pull_request' }}`**, so
  a superseded PR push cancels its predecessor and a push to `main` never
  cancels. The guard asserts exactly this shape.
- **Both `ci.yml` and `sanitizers.yml` gain top-level
  `permissions: contents: read`.** The row names `ci.yml`; the DoD says every
  workflow, and `sanitizers.yml` is the other one without a block.
  `secrets-scan` keeps `pull-requests: write` and `notify` keeps
  `issues: write`, job-local.
- **The guard's own tests carry inline red fixtures**, so "seen red" is
  built into every run rather than proved once: each rule is asserted to fire
  on a synthetic workflow that violates it and to stay silent on the tree.
- **The guard also runs as a step in `ci.yml`'s `code-quality` job.**
  `check-guards.sh` is the local mirror; nothing in CI runs it, and a guard
  CI never runs is not a gate.
- **Seed corpora are generated, not committed.** A small `fuzz_seed_corpus`
  executable, built beside the fuzz targets, writes serialized states and
  engine-written SMF files into a directory; the nightly and the local token
  run it before the fuzzers. Binary blobs in git would drift from the state
  format the moment `kCurrentStateVersion` moves.
- **The fuzz job lives in `sanitizers.yml`**, built with Clang and ASan,
  each target bounded to 300 seconds, crash artifacts uploaded, and the
  existing `notify` job extended so a crash files or comments on the
  `sanitizer-failure` issue exactly as a sanitizer failure does.
- **A `fuzz` validation token is added** to `.jk/validations.yml`, marked
  pre-push-exempt because it needs an LLVM Clang with libFuzzer, and appended
  to M002/S02's tokens: a slice that adds fuzzers owes running them.
- **Planted bugs are proved on a scratch branch that is never pushed.** Each
  fuzz target is run against a deliberately broken tree — one bounds check
  removed in `state_io_read_lane.h`, one in `midi_reader.cpp` — and the
  evidence records the crash headline. The branch is deleted afterwards.
- **`GoldenPhrase.OffsetPhraseBehavior` asserts `EXPECT_FALSE` on
  `bothSilentSomewhere`.** The test's premise holds for its patch (bar
  analysis above), so the missing assertion is the stronger one; it is seen
  red by giving both lanes the same phrase length and gap, which puts both
  gaps in bar 2.
- **Warning fixes preserve numeric semantics.** Every `-Wsign-conversion`,
  `-Wconversion` and `-Wdouble-promotion` site becomes an explicit cast that
  spells out the conversion the compiler was already performing; no float
  arithmetic becomes double or vice versa, and `unit`'s golden tests prove
  the output is unchanged. The GCC 15 `-Wfree-nonheap-object` diagnostic is
  a known false positive on `vector::push_back` under inlining and is
  handled in-band at the site with its reason, not by a global flag.
- **Test-side warnings are counted and classified, not fixed.** OS11 records
  tests as a later phase; T3 lists the remaining test-side diagnostics by
  flag, inspects any that is not a conversion, and records the count here so
  the phase has a number.

## 2026-09-24 — judgment call during M002/S02 task 1

- **The whole fuzz build is compiled with `-fsanitize=fuzzer-no-link`.**
  `-fsanitize=fuzzer` on a target instruments only that target's own
  translation unit, so `poly_engine` was a black box to libFuzzer's coverage
  feedback: the first seeded run of `fuzz_midi_reader` reached 40 edges after
  780,780 executions, and `fuzz_state_io`'s 247 came from the header-inlined
  readers alone. With the option-gated global flag the same 60 seconds reach
  329 and 639 edges. Obviously right: a guided fuzzer that cannot see the code
  it is fuzzing is a random-input loop, and the plan's premise — that the
  targets find planted bugs — depends on the guidance.

## 2026-09-24 — judgment call during M002/S02 task 3

- **The planted state-I/O bug is an unvalidated count used as an index, not
  the plan's one-past array read.** The plan said to lengthen the
  `microTimingMs` loop by one; `microTimingMs` is followed by `laneSeed`
  inside `LaneConfig`, so that read stays inside the object and ASan cannot
  see it — the proof would have been a proof of nothing. The substitute
  writes `cellSizes[cellCount]` with `cellCount` straight from the file, the
  bug class the row names, and reaches arbitrarily far outside the object.
  Obviously right: the plan's aim was a detectable planted out-of-bounds
  access in the state reader, and the site was a detail.

## 2026-09-24 — the tests phase's starting number, measured during M002/S03 task 3

With `poly_engine` clean and fatal, the full engine-only tree under GCC 15
(`POLY_WARNINGS_FATAL` still `OFF`) reports **63 unique warning sites** in
test and tool sources, none in `engine/`: 60 `-Wsign-conversion`, 2
`-Wdangling-else`, 1 `-Wunused-function`. By file: `golden_tests.cpp` 32,
`euclidean_tests.cpp` 14, `envelope_tests.cpp` 7, `emit_presets.cpp` 4,
`dynamic_shaping_tests.cpp` 3, and one each in `subdivision_profile_tests.cpp`,
`step_weight_tests.cpp`, `emission_tests.cpp`. The three that are not
conversions were inspected: the two `-Wdangling-else` sites are gtest
`EXPECT_NEAR` macros inside brace-less `if`s in one kotekan velocity test —
each `if` is independent and the macro's own `else` binds inside it, so no
assertion is mis-scoped; the `-Wunused-function` is `backbeatPattern()` in
`step_weight_tests.cpp`, a helper nothing calls, dead rather than wrong.
None marks a defect; the two the row found (OS12, OS13) were fixed in tasks 1
and 2. The tests phase starts at 63.

Two notes for whoever runs that phase. GCC 15 on this Mac cannot compile
googletest against the macOS 26 SDK (`mach/message.h` parse errors), so the
count came from the test translation units, which compiled, not from a linked
`poly_tests`; CI's Linux GCC is unaffected. And Apple Clang reports every
`-Wsign-conversion` site GCC does, so the number is the same on both.

## 2026-09-24 — judgment call after M002/S03 closed

- **The local `fuzz` token turns the engine's fatal option off.** Re-running
  every token on the final head, the fuzz build failed under Homebrew LLVM
  22: its libc++ emits `#warning "The selected platform is no longer
  supported by libc++."` for the 10.15 deployment target, and
  `POLY_ENGINE_WARNINGS_FATAL` now promotes it. It is a system-header notice,
  not a Poly warning, and appears only with Homebrew's Clang on macOS. The
  token and the `CONTRIBUTING.md` snippet pass
  `-DPOLY_ENGINE_WARNINGS_FATAL=OFF` with the reason: that build exists to
  find crashes, `engine-isolation` is the warnings gate, and the nightly on
  Ubuntu keeps the default. Obviously right: the alternative was a
  `-Wno-#warnings` that would hide a real one.
