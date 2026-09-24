# M002/S03 — A warning means something

**Slice:** M002/S03 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS11 (`POLY_WARNINGS_FATAL` is off and the engine has 65 warning
sites), OS12 (a computed flag is never asserted), OS13 (the note number is
read and discarded)
**Depends:** nothing.
**Decisions consumed:** `M002-decisions.md`, 2026-09-24 — import keeps the
lane's own note and falls back to merging; `POLY_ENGINE_WARNINGS_FATAL`
defaults `ON` for the engine alone; fixes preserve numeric semantics; the
golden flag is asserted `EXPECT_FALSE`; test-side warnings are counted, not
fixed.

## Task status

- [x] Task 1 — The golden phrase test asserts what it computes (OS12)
- [ ] Task 2 — A dropped file imports the lane's own note, else everything
      (OS13)
- [ ] Task 3 — The engine builds warning-free and fatally on every compiler,
      and CI builds it that way (OS11); the slice closes

## Definition of Done

- [ ] `poly_engine` builds warning-free on GCC, Clang and MSVC with
      `POLY_WARNINGS_FATAL=ON`, and CI builds it that way
- [ ] Every test-side warning that marked a real defect is fixed, not silenced
- [ ] MIDI import's handling of pitch is a decision with a test naming it

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |

## Task 1 — The golden phrase test asserts what it computes (OS12)

**Files:** modify `tests/golden_tests.cpp`.

1. In `GoldenPhrase.OffsetPhraseBehavior`, replace the comment above
   `renderSorted` with the bar table: lane 0's 12-beat phrase rests in bars
   2 and 5, lane 1's 16-beat phrase rests in bar 3, so no bar is silent on
   both lanes and every rest is covered by the other lane. Add, after the
   `oneSilentOtherPlaying` assertion:
   `EXPECT_FALSE(bothSilentSomewhere) << "Offset phrase lengths must never leave a whole bar silent on both lanes";`
2. See it red for the right reason: temporarily set `lane1.phraseLength =
   8.0f` (same as lane 0, so both gaps land in bar 2), build `poly_tests`,
   run `ctest --test-dir build -R GoldenPhrase.OffsetPhraseBehavior`. It
   fails on the new line. Record the failure line in the evidence. Restore
   `12.0f`.
3. Run the test again: green. Run `unit` and `format`. Append evidence. Tick
   the box. Commit with `Rows: OS12`; OS12 `done`.

## Task 2 — A dropped file imports the lane's own note, else everything (OS13)

**Files:** modify `engine/include/poly/midi_reader.h`,
`engine/src/midi_reader.cpp`, `tests/midi_reader_tests.cpp`,
`engine/include/poly/wasm_api.h`.
**Produces:** `MidiParseResult::onsetNotes`; the filter in
`importMidiToLane`.

1. Write three tests in `tests/midi_reader_tests.cpp`, using `writeSMF` over
   hand-built `NoteEvent` vectors as the existing tests do. A helper
   `twoInstrumentEvents()` returns one cycle of E(3,8) at pitch 36 merged with
   one cycle of E(5,8) at pitch 38, subdivision 16, the last note of each
   sustained to the cycle end as `euclideanEvents` does.
   - `MidiReader.RecordsNoteNumberPerOnset`: parse the two-instrument file;
     `onsetNotes.size()` equals `onsetsPpq.size()`; the multiset of notes is
     three 36s and five 38s; each `onsetNotes[i]` matches the pitch of the
     event whose PPQ equals `onsetsPpq[i]`.
   - `ImportMidi.KeepsOnlyTheLanesOwnNoteWhenPresent`: `LaneConfig lane{}`
     with `midiNote = 36`; `importMidiToLane` returns true; the fit is E(3,8)
     — `hitCount == 3`, `cycle.steps == 8`, `patternMismatch`-free, so
     `timeline == false`.
   - `ImportMidi.MergesEveryNoteWhenTheLanesNoteIsAbsent`: same file,
     `midiNote = 42`; returns true; `hitCount` equals the merged onset count
     (3 and 5 share no step in E(3,8)+E(5,8)? — compute the union in the test
     from `euclideanOnsets` rather than hard-coding it) and the lane reflects
     the merged pattern.
2. Build and run the three: the first fails to compile (`onsetNotes` does not
   exist). Record that.
3. In `midi_reader.h`, add to `MidiParseResult` after `onsetsPpq`:
   `std::vector<uint8_t> onsetNotes;` — "the note number of each onset,
   index-aligned with `onsetsPpq`". Update the file's header comment: the
   reader still merges every track and every pitch into one timeline; the
   import step is where the lane's note is chosen. Update
   `importMidiToLane`'s comment with the OS13 rule: onsets on the lane's
   `midiNote` when the file has any, otherwise every onset.
4. In `midi_reader.cpp`: collect onsets as a local `struct Onset { uint64_t
   tick; uint8_t note; }` vector instead of `onsetTicks`; push `{absTick,
   d1}` on a real note-on; sort by tick with a stable sort; fill both result
   vectors. `d1` is now used, so `-Wunused-but-set-variable` is gone. In
   `importMidiToLane`, after the parse: if `lane.midiNote` is in 0..127 and
   any `onsetNotes[i] == lane.midiNote`, build a filtered `onsetsPpq` of just
   those; otherwise use all. Fit the chosen vector with the file's
   `loopLengthPpq` and `tempoBpm` as today.
5. In `wasm_api.h`, extend the `poly_import_midi` comment with the same rule
   in one sentence, so the web preview's contract says what a drop does.
6. Run the three tests: green. Run the whole `MidiReader.*`, `ImportMidi.*`
   and `RevertImport.*` suites: green. Run `unit`, `engine-isolation`,
   `format`. Run `bash scripts/check-snippet-regions.sh`: the
   `midi-parse-result` region still resolves.
7. Append evidence. Tick the box. Commit with `Rows: OS13`; OS13 `done`. The
   engine headers this touches are not in the drift map; no trailer.

## Task 3 — The engine builds warning-free and fatally on every compiler, and CI builds it that way (OS11); the slice closes

**Files:** modify `CMakeLists.txt`, `.github/workflows/ci.yml`,
`engine/include/poly/types.h`, `engine/include/poly/scene.h`,
`engine/include/poly/rng.h`, `engine/src/engine.cpp`,
`engine/src/euclidean.cpp`, `engine/src/macro.cpp`,
`engine/src/envelope.cpp`, `engine/src/smf_writer.cpp`,
`docs/plans/open-source-launch/M002-decisions.md`,
`docs/plans/open-source-launch/ledger.md`.
**Consumes:** task 2's removal of the `d1` warning.

1. In `CMakeLists.txt`, after `POLY_WARNINGS_FATAL`, add
   `option(POLY_ENGINE_WARNINGS_FATAL "Treat the jk-standards warning set as errors for poly_engine (the tests, tools and plugin follow POLY_WARNINGS_FATAL)" ON)`.
   Rewrite the phase-1 comment block above the two options to describe the
   state after this task: the engine is at full conformance and fatal by
   default; the remaining targets carry the roster non-fatally until their
   own burndown, counted in this milestone's decisions file. In the
   `jk_target_warnings` wrapper, demote only when
   `NOT POLY_WARNINGS_FATAL AND NOT (target STREQUAL "poly_engine" AND POLY_ENGINE_WARNINGS_FATAL)`.
2. Configure engine-only with GCC 15 into a scratch directory
   (`-DCMAKE_C_COMPILER=gcc-15 -DCMAKE_CXX_COMPILER=g++-15
   -DPOLY_ENGINE_ONLY=ON`) and build `poly_engine`: it fails, on the first of
   the 65 sites. This is the red run; record the first error.
3. Burn down, file by file, GCC first then Clang. Rules: an integer whose
   sign changes gets `static_cast<size_t>` or `static_cast<int>` at the point
   of use, with the same value range the code already assumed; a `float` that
   met a `double` gets `static_cast<double>` so the arithmetic stays in
   double exactly as the compiler already promoted it — never `2.0f` for
   `2.0`; an `int` or `int64_t` that met a `float` or `double` gets the
   explicit cast to the type the expression already produced. Where a loop
   index feeds a `std::array` in `types.h` or `scene.h`, prefer changing the
   index type to `size_t` over casting at every use, when no arithmetic on
   it needs a signed type. For the GCC 15 `-Wfree-nonheap-object` at
   `smf_writer.cpp`'s `writeVLQToVec`: it fires inside libstdc++'s
   `push_back` under inlining and is a known GCC false positive; if
   reserving the vector's final size first silences it, do that with a
   comment; if not, wrap the function in `#pragma GCC diagnostic push` /
   `ignored "-Wfree-nonheap-object"` / `pop` guarded by `#if defined(__GNUC__)
   && !defined(__clang__)`, with the reason and the GCC version in the
   comment. Rebuild after each file until GCC's engine-only build is clean.
4. Configure engine-only with the default Apple Clang into a second scratch
   directory and build: clean. Then `bash scripts/ensure-emsdk.sh` and
   `bash scripts/build-wasm.sh`: Emscripten's Clang builds the engine clean
   under `-Werror` too. Restore `webui/poly_engine.js` and
   `webui/poly_engine.wasm` with `git checkout -- webui/poly_engine.js
   webui/poly_engine.wasm` (the build rewrites them; this task ships no
   engine behaviour change).
5. Prove the gate: in `euclidean.cpp` temporarily assign a `size_t` loop
   counter to an `int` without a cast — a real `-Wsign-conversion` site.
   Build engine-only with GCC and with Clang: both fail on that line. Remove
   it. Record both failure lines.
6. In `ci.yml`'s `engine-isolation` job, add `-DPOLY_ENGINE_WARNINGS_FATAL=ON`
   to the configure command with a comment naming OS11: the default is
   already `ON`, and the flag is written so the job cannot lose the property
   if the default ever moves. The `build` matrix legs (macOS Clang, Windows
   MSVC) and `wasm-build` inherit the default, so every compiler CI uses
   builds the engine fatally.
7. Count the later phase: with `POLY_WARNINGS_FATAL` still `OFF`, build the
   full engine-only tree (tests and tools) with GCC 15 and list the remaining
   warnings by flag and file. Inspect every diagnostic whose flag is not
   `-Wsign-conversion`, `-Wconversion`, `-Wdouble-promotion` or
   `-Wimplicit-int-float-conversion`; fix any that marks a defect the way OS12
   and OS13 were, and name it in the evidence. Append the counts to
   `M002-decisions.md` under a dated heading as the tests phase's starting
   number.
8. Run `unit` (the golden tests prove the output is unchanged),
   `engine-isolation`, `format`. Every engine source this task touches is
   paired with `docs/engine-spec.md` in the drift map; the commit carries
   `Docs-Not-Affected: docs/engine-spec.md — every change is an explicit cast spelling out a conversion the compiler already performed; no value, range, or behaviour the spec describes moved, and unit's golden tests are unchanged`.
   Run `bash scripts/check-doc-discipline.sh` once for that pairing.
9. Tick every DoD box here and in the ledger; set the slice `done`; OS11
   `done`. Append evidence with the DoD-to-task table, the per-compiler
   clean-build facts, and the planted-warning failures. Run
   `jk-standards ledger`. Commit with `Rows: OS11`.

## Self-review

| DoD | Task |
|---|---|
| `poly_engine` warning-free and fatal on GCC, Clang, MSVC; CI builds it that way | 3 — GCC 15 and Apple Clang and Emscripten proved locally; MSVC is proved by the Windows `build` leg on the PR, which inherits the `ON` default; `engine-isolation` passes it explicitly |
| Every test-side warning that marked a real defect is fixed, not silenced | 1 (OS12) and 2 (OS13); 3 step 7 inspects the rest and records the count |
| MIDI import's pitch handling is a decision with a test naming it | 2 — the decision is in `M002-decisions.md`; `ImportMidi.KeepsOnlyTheLanesOwnNoteWhenPresent` and `ImportMidi.MergesEveryNoteWhenTheLanesNoteIsAbsent` name it |

| Row | Task | Verification produced |
|---|---|---|
| OS11 | 3 | `engine-isolation` configures with the fatal option; a planted sign conversion fails GCC and Clang; tests recorded as the later phase with a count |
| OS12 | 1 | `EXPECT_FALSE(bothSilentSomewhere)`, seen red with both lanes on the same phrase |
| OS13 | 2 | two-instrument file dropped on a lane; the lane's note wins when present, everything merges otherwise; `d1` warning gone |

The DoD's first line names `POLY_WARNINGS_FATAL=ON`; the decision recorded on
2026-09-24 delivers it through `POLY_ENGINE_WARNINGS_FATAL`, which is the
same roster made fatal for the engine target alone, so a full-tree
`POLY_WARNINGS_FATAL=ON` build still fails on the tests — that is the later
phase the row itself defers. Names used throughout: `onsetNotes`,
`POLY_ENGINE_WARNINGS_FATAL`, `twoInstrumentEvents`. No placeholders.
