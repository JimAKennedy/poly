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
