---
class: gated
---

# M002/S01 — Position-weighted stochastic decisions: design

**Slice:** `M002/S01` in `docs/plans/guide-parity/ledger.md` (row `GP03`)
**Classification:** architectural — it establishes the mechanism `M002/S02`,
`M002/S03` and `M002/S04` consume. Their slices are bounded against it.

## The problem

Four decisions in `engine.cpp` are position-blind:

```cpp
mutRoll  = rand(..., 8);  if (mutRoll  < mutationRate) { ... }   // whether to mutate
typeRoll = rand(..., 9);  // drop | ghost | add, by fixed thresholds
fillRoll = rand(..., 4);  if (fillRoll >= fillProb) { ... }      // whether to fill
```

Every step is equally likely to be mutated, to become a ghost, or to receive a
fill. The guide teaches otherwise in four places, and issues #152, #158, #154
and #155 name them: clave alignment, funk ghost grammar, phrase-boundary fills,
and call-and-response phrasing.

## The mechanism

**A weight scales a probability. `1.0` is a no-op.**

```cpp
if (mutRoll < cfg.mutationRate * w.add) { ... }
```

That is the whole idea, and the byte-identity guarantee falls out of it: a lane
with no weighting configured has every weight at exactly `1.0`, so the
arithmetic is the arithmetic that ran before.

### Settings live on the lane; weights live in the render context

`LaneConfig` gains only **settings** — roughly 24 bytes per lane:

| Field | Slice | Meaning |
|---|---|---|
| `timelineSourceLane` | S01 | which lane is the timeline; `-1` = none |
| `timelineStrength` | S01 | signed; positive attracts, negative avoids |
| `ghostGrammar` | S02 | how strongly ghosts favour the approach to an accent |
| `fillPhraseShape` | S03 | how sharply fills concentrate at the boundary |
| `responseSourceLane` | S04 | which lane this one answers |

The **weights** are computed per render into `LaneContext`, which is not
serialised and not copied per block. Storing four 64-float arrays per lane
instead would add about 8 KB to `GrooveState` — a 45% growth on a struct copied
three times per block — to hold values derivable from a handful of scalars.

The reference lane's onsets are resolved once per lane per block, exactly as the
kotekan source pattern already is at `engine.cpp:99-110`. That is an existing,
proven shape rather than a new one.

### Weights compose multiplicatively

A step can be metrically weak *and* against the clave *and* near a phrase end.
Each source contributes a factor and they multiply; an unset source contributes
exactly `1.0`.

**This deliberately differs from M003's ruling** that a subdivision profile takes
precedence over `cellSizes` rather than combining with it — and the difference is
the point. Those were two competing definitions of one grid, and combining them
produced a placement nobody could reason about. These are probabilities, and
composing is what probabilities do: a step that is favoured on two counts should
be favoured more than one favoured on one.

### Four weights, not one scalar

```cpp
struct StepWeights {
    float add = 1.0f;    // mutation-add, and fills
    float drop = 1.0f;   // mutation-drop
    float ghost = 1.0f;  // mutation-to-ghost
    float fill = 1.0f;   // fill-add
};
```

One number cannot mean attraction for adds, protection for drops, grammar for
ghosts and shape for fills at once. Overloading a single field with several
meanings is what M003 rejected; four named fields cost nothing in the context
and make each roll site say which weight it is reading.

## What this slice builds, and what the others add

**S01** builds the whole mechanism and supplies the **timeline** source: a
reference lane's onsets, a signed strength, `add`/`fill` attracted toward them
and `drop` protecting them. It is the slice that proves the mechanism because it
is the one that needs every part of it.

**S02** adds a metric-position source feeding `ghost`. **S03** adds a
phrase-proximity source feeding `fill`. Neither changes the mechanism.

**S04 is not a weight at all.** It couples one lane's phrase *gate* to another
lane's, and it is in this milestone because it reuses S01's reference-lane
resolution and its mutual-reference guard — the same "one lane names another"
shape, reading gate state rather than onsets. That is why it carries
`Depends: M002/S01` and why the ledger put it here rather than in a milestone of
its own.

## Real-time safety and determinism

The weights are computed in `prepareLaneContext`, which already runs per lane
per block, from a fixed-size array and scalar arithmetic. No allocation, no
locks. The roll values themselves are untouched — only the thresholds they are
compared against move — so `(patch, seed, transport)` still determines the
output exactly, and a locate or loop reproduces it.

**A mutual reference is refused, not followed.** `timelineSourceLane` uses the
guard `kotekanSourceLane` already has: out of range, self-reference, or a source
that points back is treated as no reference at all.

## Testing

- a weight of `1.0` everywhere leaves all 45 factory presets byte-identical,
  proved by a golden rather than by inspection
- with a positive `timelineStrength`, mutation-adds land on timeline-aligned
  steps more often than on contradicting ones, measured as a distribution over
  many seeds rather than a single roll
- with a negative strength the distribution inverts, which is the arm that
  proves the sign is read rather than the magnitude
- drops are less likely on high-weight steps than low-weight ones
- a mutual reference produces no weighting at all
- `renderRange()` passes `scripts/check-realtime-safety.sh`
