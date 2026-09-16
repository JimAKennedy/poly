---
class: gated
---

# M003/S01 — Subdivision profiles

**Slice:** `M003/S01` in `docs/plans/engine-capability/ledger.md`
**Design:** `M003-S01-design.md` — approved 2026-09-15

## Task status

- [x] 1. The profile fields and `computeAdditiveCells`
- [x] 2. Wire the profile through `prepareLaneContext`
- [x] 3. State version 19 → 20
- [x] 4. The samba profile ships, and reaches `presets.json`
- [x] 5. Evidence and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane can play a non-isochronous subdivision profile rather than an even grid
- [ ] A profile is expressible in a preset and reaches `site/src/generated/presets.json`
- [ ] `renderRange()` gains no allocation, lock or blocking call
- [ ] The engine builds and passes its tests with no VST3 SDK present

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |

## Task 1 — The profile fields and `computeAdditiveCells`

Creates `tests/subdivision_profile_tests.cpp`; modifies
`engine/include/poly/types.h` and `tests/CMakeLists.txt`.

1. Write the failing test first, in a new `tests/subdivision_profile_tests.cpp`
   registered in `tests/CMakeLists.txt` beside `kotekan_mode_tests.cpp`. Three
   cases against `computeAdditiveCells` directly:
   - a profile of `{1.1f, 0.9f, 0.95f, 1.05f}` with `profileCount = 4` produces
     four cumulative positions whose successive differences are in those
     proportions, and a `totalPpq` equal to four even steps
   - a profile scaled by any constant produces identical output — assert
     `{2.2f, 1.8f, 1.9f, 2.1f}` gives the same `cumPpq` as the first case, to
     within `1e-9`
   - `profileCount == 0` with `cellCount = 3`, `cellSizes = {2,2,3}` produces
     exactly today's values, so the existing path is untouched
2. Run it and watch all three fail to compile — the fields do not exist. That is
   the right failure; make it a *test* failure by adding the fields empty first
   if the compile error is unclear.
3. Add to `LaneConfig` in `engine/include/poly/types.h`, after `cellSizes`:
   `std::array<float, kMaxSteps> subdivisionProfile{}` and `int profileCount = 0`,
   with a comment naming M003/S01 and EC08 and stating that `profileCount == 0`
   reproduces the even grid exactly.
4. Extend `computeAdditiveCells` to build from the profile when
   `profileCount > 0`, taking precedence over `cellCount`. Normalise: sum the
   first `profileCount` entries, divide by `profileCount` to get a scale factor,
   and divide each entry by it before accumulating. Guard a zero or negative sum
   by returning the empty `AdditiveCellInfo` — a profile that sums to nothing is
   not a feel, and dividing by it would produce infinities in the timing path.
5. Run the three cases and watch them pass. Run `unit` and `engine-isolation`.
   Commit with the slice's trailers, `Rows:` empty.

## Task 2 — Wire the profile through `prepareLaneContext`

Modifies `engine/src/engine.cpp` and `tests/subdivision_profile_tests.cpp`.

1. Add the failing cases first, rendering through the public engine API rather
   than the helper:
   - a profiled lane's onsets differ from the isochronous grid **and** from the
     same lane with `swingAmount = 0.2f` and no profile — `EC08`'s stated
     verification, and the swing arm is there because swing is the workaround
     the guide admits to
   - the same profile at 90 and at 140 BPM produces the same ratios between
     successive inter-onset intervals, to within `1e-6` — the property that
     distinguishes this from `microTimingMs`
   - a profile summing to more than `profileCount` does not lengthen the lane's
     cycle: its last onset plus its last step duration equals the even cycle
     length
2. Watch them fail: the fields exist but nothing reads them in `prepareLaneContext`.
3. In `prepareLaneContext`, treat `profileCount > 0` exactly as `cellCount > 0`
   is treated: `computeAdditiveCells` already returns a populated
   `AdditiveCellInfo`, so `ctx.isAdditive` becomes true through the existing
   `additive.count > 0` test and `stepsInCycle` follows. Verify by reading that
   `computeAdditiveCells` sets `info.count = profileCount` on the profile path.
4. In the `maxTimingShift` computation, the loop over `cellSizes` must also
   consider profiled steps — take the longest *normalised* profile entry times
   `sPpq`, so the lookahead window still covers the longest step. A profile
   whose longest step exceeds the base step would otherwise have its last onset
   dropped at a block boundary.
5. Add a golden case asserting the factory presets are byte-identical with no
   profile set, so back-compatibility is measured rather than assumed. Run
   `unit`, `engine-isolation`, and `rt-safety`. Commit.

## Task 3 — State version 19 → 20

Modifies `engine/include/poly/state_io_envelope.h`,
`engine/include/poly/state_io_write_lane.h`,
`engine/include/poly/state_io_read_lane.h`, and the tests.

1. Write the failing round-trip test first: a lane with a profile written and
   read back returns the same `profileCount` and the same entries; and a v19
   payload read by the new code yields `profileCount == 0` rather than garbage.
2. Follow the `kKotekanModeStateVersion` pattern exactly — it is the most recent
   precedent and sits in the same file. Add
   `kSubdivisionProfileStateVersion = 20`, bump `kCurrentStateVersion` to 20, and
   guard the new field's read and write on `version >= kSubdivisionProfileStateVersion`.
3. Write only the first `profileCount` entries, not all 64, and write
   `profileCount` before them so the reader knows how many to consume.
4. Run the round-trip cases and the existing state tests. Run `unit` and
   `engine-isolation`. Commit.

## Task 4 — The samba profile ships, and reaches `presets.json`

Modifies `engine/src/presets.cpp`, `site/scripts/generate-presets-json.mjs`, and
the generated `site/src/generated/presets.json`.

1. Add a named profile catalogue to `engine/src/presets.cpp` — a small file-local
   table mapping a name to its entries, with the samba long-short-short-long
   profile as its first member and a comment citing the guide's Rule 6 sources
   (Gerischer 2006, Naveda et al. 2011) that `theory-brazilian` already names.
   Do not invent measured values: use the shape Rule 6 states — first sixteenth
   slightly long, middle two compressed, fourth slightly long — and say in the
   comment that the depth is Poly's, as the page's own attribution line says of
   all its patch values.
2. Apply it to the samba preset's lanes.
3. Extend `site/scripts/generate-presets-json.mjs` to emit `subdivisionProfile`
   and `profileCount`, and raise its `schemaVersion` from 5 to 6. Check whether
   the script has a guard on the count of per-lane fields, as
   `generate-params-json.mjs` did — if so, update it, and if it has an
   `existsSync` early return of the kind M005's PIPE01 and M002 both had to
   remove, remove it here too rather than leaving the third instance.
4. Regenerate `presets.json`, confirm `schemaVersion` reads 6 and the samba
   preset carries its profile.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 5 — Evidence and close-out

1. Append `evidence/M003-S01.md` with each token's result and headline counts,
   the byte-identical golden result, and the tempo-relativity measurement.
2. Tick the four task boxes above, tick the four definition-of-done boxes, set
   `EC08` to `done` and the slice to `done`.
3. Run `jk-standards ledger`, then the whole validation set once more on the
   final tree. Commit with `Slice: M003/S01` and `Rows: EC08`.
