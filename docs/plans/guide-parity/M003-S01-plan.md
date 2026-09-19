---
class: gated
---

# M003/S01 — A lane emits a sequence of pitches

**Slice:** `M003/S01` in `docs/plans/guide-parity/ledger.md`
**Classification:** bounded — pitch assignment already exists at
`engine.cpp:606`, one line; this varies what it reads. Design decisions taken
at planning, see `M003-decisions.md`.

## Task status

- [x] 1. The sequence field, and the pitch it supplies
- [ ] 2. Existing lane features still apply
- [ ] 3. The sequence survives a reload
- [ ] 4. Evidence and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane can carry an optional sequence of pitches with per-note durations, supplying successive hits' pitch instead of the single `midiNote`
- [ ] The field's name does not collide with the existing `phrase*` fields, and the chosen name is recorded with its reason
- [ ] Every existing lane feature — drift, kotekan complement, tempo multiplier, additive cells — applies unchanged with a sequence set, asserted for at least two of them
- [ ] With no sequence set, all 45 factory presets render byte-identically, proved by a golden test
- [ ] A pre-bump state loads as the single-pitch behaviour it played

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
## Standing instructions

From `M003-decisions.md`, carried from M001 and M002's findings:

- **Add the render-level case before probing.** M002/S02's probe killed nothing
  because every case tested the helper directly; the render case then found a
  real wiring defect immediately.
- **Name the discriminating case** in the evidence. Most cases survive their
  probe — determinism, range and preset-identity hold for the old behaviour too.
- **Prove a `present`-only site claim by removing the correction**, never by
  adding the old phrasing beside it.
- **Restore a probed file by explicit edit**, not `git checkout`.
- **Rebuild before reading any mutation result**, and check the build succeeded:
  a probe that fails to compile makes `ctest` run the previous binary and report
  everything passing.


## Task 1 — The sequence field, and the pitch it supplies

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`; creates
`tests/note_sequence_tests.cpp` and registers it in `tests/CMakeLists.txt`.

1. Write the failing tests first, against a `noteSequencePitch(cfg, absStep)`
   helper and the fields that do not exist yet:
   - with `noteSequenceLength == 0`, the helper reports "no sequence" and the
     lane keeps `midiNote` — the byte-identity guarantee as arithmetic
   - with a sequence set, `absStep` 0..N maps to `sequence[absStep mod length]`,
     asserted across more than one wrap so an off-by-one at the boundary shows
   - **a five-note sequence over a seven-step cycle phases**: the pitch at step
     `s` and at step `s + 7` differ, which is the device #245 asks for and the
     reason indexing is by position rather than by hit ordinal
   - a negative `absStep` wraps correctly rather than indexing out of bounds
2. Add to `LaneConfig`: `std::array<NoteSequenceEntry, kMaxNoteSequence>
   noteSequence{}` and `int noteSequenceLength = 0`, where
   `NoteSequenceEntry` carries `int16_t pitch` and `float durationBeats`, and
   `kMaxNoteSequence = 8` per #245's own proposal. Comment the naming: `phrase*`
   is taken by gating rather than content, which the slice's DoD requires be
   recorded.
3. Implement the helper in `types.h` beside the other inline helpers. Pure, no
   allocation — it runs per emitted note on the audio thread.
4. **Write the render-level case before probing**, per the standing instruction:
   a lane with a sequence emits the expected pitches in order through
   `renderRange`, and one without emits `midiNote` throughout.
5. At `engine.cpp:606`, read the sequence when present. The duration is the
   entry's `durationBeats` when positive, else the lane's existing
   `noteDuration` — **gate length only**, per the decisions file: the onset is
   still the step grid's.
6. **Add the byte-identity golden**: with no sequence set, all 45 factory
   presets render identically, and no preset ships a sequence. This is the
   slice's fourth definition-of-done item, and it is in this task rather than
   left to close-out because M001/S01 lost a DoD item that way and had to be
   reopened after it had already closed.
7. Probe both the helper and the wiring, rebuilding and checking the build
   before reading either result.
8. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit; the row stays
   open.

## Task 2 — Existing lane features still apply

Modifies `tests/note_sequence_tests.cpp` only.

The slice's third definition-of-done item asks for at least two existing
features asserted to still work with a sequence set. Choose the two that could
plausibly break, not the two that are easiest:

1. **Drift** rotates which step index a position maps to. Assert that a drifting
   lane with a sequence still takes its pitch from the position's step index, so
   drift and the sequence compose rather than one overriding the other.
2. **Kotekan complement** derives a lane's pattern from another's. Assert a
   sequenced kotekan lane emits the complement's steps with the sequence's
   pitches — the pattern and the pitch source are independent, and a test that
   only checked one would miss the other silently.
3. Run `format`, `unit`, `engine-isolation`. Commit.

## Task 3 — The sequence survives a reload

Modifies the three `state_io_*` headers and `tests/state_migration_tests.cpp`.

**M001/S01 shipped two fields unserialised and had to be reopened.** The gap was
that its plan had no serialization task while its sibling assumed one. This task
exists so that cannot repeat here.

1. Write the failing round-trip tests first: a lane with a sequence is written
   and read back unchanged, entry for entry; and a pre-bump payload loads with
   `noteSequenceLength == 0` rather than whatever the bytes happen to hold.
2. Follow `kFeelModeStateVersion`'s pattern in the same file. Add
   `kNoteSequenceStateVersion = 22`, bump `kCurrentStateVersion`, and guard both
   read and write.
3. Write `noteSequenceLength` first and only that many entries, so a lane
   without a sequence costs four bytes rather than the whole array — the shape
   M003's subdivision profile already uses.
4. Reject a length outside `[0, kMaxNoteSequence]` on read rather than trusting
   it: a forged count would walk the reader off the end.
5. Probe both guards; they should be complementary, each killing one case.
6. Run `format`, `unit`, `engine-isolation`. Commit.

## Task 4 — Evidence and close-out

1. Append `evidence/M003-S01.md` with token results, every mutation outcome
   including which cases survived and why, and the byte-identity golden.
2. Tick the task and definition-of-done boxes, set `GP07` and the slice `done`.
3. Run `jk-standards ledger`, then the whole validation set on the final tree.
   Commit with `Slice: M003/S01` and `Rows: GP07`.
