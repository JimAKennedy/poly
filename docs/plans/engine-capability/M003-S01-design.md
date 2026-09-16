---
class: gated
---

# M003/S01 — Subdivision profiles: design

**Slice:** `M003/S01` in `docs/plans/engine-capability/ledger.md` (row `EC08`)
**Classification:** architectural — a new capability in the timing path, reached
by state and presets, changing the state version and the preset schema.

## The problem

`theory-brazilian` Rule 6 ends "until subdivision profiles ship, use light swing
(0.15–0.25) plus small per-lane offsets as an admitted approximation". The guide
is right to call it an approximation: swing displaces only alternate notes, and
samba's feel is a distribution across all four sixteenths of a beat.

Two mechanisms already exist and neither expresses it:

- `cellSizes[kMaxSteps]` is **`int`**. `{2,2,3}` is expressible; `{1.1, 0.9,
  0.95, 1.05}` is not, at any base subdivision that keeps the cycle the right
  length.
- `microTimingMs[kMaxSteps]` is **absolute milliseconds**, clamped to ±20 ms. At
  90 BPM a sixteenth is about 167 ms, so a fixed offset expresses a ratio only at
  the tempo it was measured at.

## The mechanism

Add to `LaneConfig`:

```cpp
// M003 S01 (EC08). Non-isochronous subdivision: per-step duration as a
// multiple of the base step. profileCount == 0 reproduces the even grid
// exactly, so every existing patch is byte-identical.
std::array<float, kMaxSteps> subdivisionProfile{};
int profileCount = 0;
```

`computeAdditiveCells` already turns a per-cell duration list into cumulative
PPQ positions. The profile feeds the same function: when `profileCount > 0`,
step *i*'s duration is `subdivisionProfile[i] * basePpq` and the cumulative sum
places it, exactly as `cellSizes[i] * basePpq` does today. One code path, and
today's integer aksak cells become the special case.

**Normalisation.** The profile is scaled so the cycle occupies the length it
would have occupied evenly: the sum is divided by `profileCount`, and each entry
by that factor. A profile is therefore a statement about *distribution*, never
about length — `{1.1, 0.9, 0.95, 1.05}` and `{2.2, 1.8, 1.9, 2.1}` are the same
feel. Without this, a profile summing to 4.2 would make its lane 5% longer than
every other lane and the ensemble would drift apart, which is not a feel.

**Precedence over `cellSizes`.** `profileCount > 0` wins. They are different
concerns — `cellSizes` is structure (how many units a cell spans), the profile
is feel (how the units are distributed) — but both reaching `computeAdditiveCells`
needs a stated winner, and silently combining two non-isochronies is the kind of
thing nobody can reason about later. A lane setting both is a configuration
error the tests pin.

## What changes

| File | Change |
|---|---|
| `engine/include/poly/types.h` | the two fields; `computeAdditiveCells` builds from the profile when present, with normalisation |
| `engine/src/engine.cpp` | `prepareLaneContext` sets `stepsInCycle`/`cyclePpqLen` from `profileCount`; `maxTimingShift` accounts for the longest profiled step |
| `engine/src/presets.cpp` | a named catalogue of shipped profiles; the samba profile that closes `EC08` |
| `engine/include/poly/state_io_*.h` | `kCurrentStateVersion` 19 → 20, guarded field pattern |
| `site/scripts/generate-presets-json.mjs` | `presets.json` schemaVersion 5 → 6 |

**Nothing in `plugin/source/`.** No new parameter IDs: the profile is state-only,
per the decision recorded in `M003-decisions.md`, and WebUI editing is
[#305](https://github.com/JimAKennedy/poly/issues/305).

## Real-time safety and determinism

The array is fixed-size and lives in `LaneConfig`, so `renderRange()` gains no
allocation, lock, or blocking call — it reads a member array and multiplies. The
normalisation is a division per step computed in `prepareLaneContext`, which
already runs per lane per block. Placement stays derived from absolute PPQ, so
the determinism golden applies unchanged: the same `(patch, seed, transport)`
produces identical output.

**Back-compatibility is the load-bearing default.** `profileCount == 0` takes
the existing branch untouched, so every one of the 45 factory presets is
byte-identical after this slice. A golden test asserts that rather than assuming
it.

## A consequence for M003/S02, named rather than designed here

S02 is now `Depends: M003/S01`, and this design does **not** by itself satisfy
its second definition-of-done clause. An additive lane sets `stepsInCycle` to
its cell count, so the `{2,2,3}` davul has three steps and nothing inside a cell
to divide — and the profile, as designed above, places steps rather than
grouping them.

What S02 will need is the grouping: a lane running one step per *unit* (the
zurna in that same preset already does — `cycle = {7, 8}`, no `cellCount`) that
knows its steps fall into 2+2+3 cells, so swing displaces within each cell and
the three-unit cell's internal division differs from the two-unit cells'. That
is a small addition on top of this mechanism, and it is S02's to design against
a landed S01 rather than to guess at now.

## Testing

- a profiled lane's onsets differ from **both** the isochronous grid and the
  swung grid — `EC08`'s stated verification, and the reason the swing comparison
  is there is that the guide's workaround is swing
- the profile is tempo-relative: the same profile at 90 and 140 BPM produces the
  same *ratios* between adjacent inter-onset intervals
- normalisation: two profiles differing only by a scale factor produce identical
  output, and a profile summing to more than its count does not lengthen the cycle
- `profileCount == 0` leaves output byte-identical across the factory presets
- precedence: a lane setting both `cellSizes` and a profile follows the profile
- the engine builds and passes with `-DPOLY_ENGINE_ONLY=ON`
- `renderRange()` passes `scripts/check-realtime-safety.sh`
