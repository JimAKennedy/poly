---
class: gated
---

# M002 — Decisions

Append-only. One entry per decision, with the reason.

## 2026-09-18 — planning M002 (all four slices, front-loaded)

**Classification.** `M002/S01` is **architectural**: it establishes a mechanism
the other three consume, so it carries a written design
(`M002-S01-design.md`), approved in chat before any code. `S02`, `S03` and
`S04` are **bounded** against that mechanism — each adds one weight source or,
in S04's case, reuses the reference-lane resolution for a different purpose.

- **Q:** The issues all say the per-step weights should be "precomputed per
  lane". Stored as arrays on `LaneConfig` that is roughly 8 KB per
  `GrooveState`, a 45% growth. Where should they live? — **A:** Settings on the
  lane, weights in the render context.
- **Decision:** `LaneConfig` carries ~24 bytes of scalar settings per lane; the
  weights are computed into `LaneContext`, which is per-render and never
  serialised. — **Why:** Eight kilobytes on a struct copied three times per
  block, to hold values derivable from a handful of scalars, is a poor trade.
  The reference lane's onsets resolve once per lane per block exactly as the
  kotekan source pattern already does, so this reuses a proven shape rather
  than inventing one. The issues' wording is a proposal, not a constraint.

- **Q:** A step can be metrically weak *and* against the clave *and* near a
  phrase end. How should multiple weight sources combine? — **A:**
  Multiplicatively, with `1.0` neutral.
- **Decision:** Each source contributes a factor and they multiply; an unset
  source contributes exactly `1.0`. — **Why:** The byte-identity guarantee then
  holds by construction rather than by assertion. **This deliberately differs
  from M003's ruling** that a subdivision profile takes precedence over
  `cellSizes` rather than combining with it, and the difference is the
  justification: those were two competing definitions of one grid, and combining
  them produced a placement nobody could reason about. These are probabilities,
  and composing is what probabilities do.

- **Decision:** Four named weights — `add`, `drop`, `ghost`, `fill` — rather
  than one scalar reinterpreted per decision. — **Why:** One number cannot mean
  attraction for adds, protection for drops, grammar for ghosts and shape for
  fills at once. Overloading one field with several meanings is what M003
  rejected; four named fields cost nothing in a per-render context and make each
  roll site say which weight it reads.

- **Decision:** `M002/S04` stays in this milestone rather than becoming its own.
  — **Why:** It reuses S01's reference-lane resolution and mutual-reference
  guard, reading gate state where S01 reads onsets. The shared part is small,
  and it was put to the user at the design gate as the cheap moment to split it
  back out; it was approved as grouped.

- **Decision:** Every slice owes a golden proving the 45 factory presets render
  byte-identically with its weight source unset. — **Why:** Inherited from the
  ledger, which took it from M003/S01, and reinforced by M001 — where the rule
  is what made a timing change reviewable without re-auditing 45 presets.

- **A standing instruction for every slice here, from M001's findings.** The
  discriminating case must be named in the evidence, because most cases in a
  weighting suite survive their mutation probe: determinism, range and
  preset-identity hold for the unweighted behaviour too. And a `present`-only
  site claim is proved by *removing* the correction, not by adding the old
  phrasing beside it — M001/S02's first probe did the latter and proved nothing.

- **Nothing deferred.** All four slices are planned below; none has a question
  that only becomes answerable once another has landed.
