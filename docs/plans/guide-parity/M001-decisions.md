---
class: gated
---

# M001 — Decisions

Append-only. One entry per decision, with the reason.

## 2026-09-18 — planning M001 (both slices, front-loaded)

**Classification.** Both slices are **bounded**: well-scoped changes to flows
that already exist in this repo. Swing and humanize are both applied today in
`applyTimingShifts`; neither slice introduces a subsystem. The design below was
presented in chat and approved before any code was written.

- **Decision:** Both features sit behind a **per-lane, state-only mode field**,
  following M002's `kotekanMode`. — **Why:** The per-lane expression parameter
  family is full at `kParamsPerLane == 16` and the core family is at 14, so
  neither has room; and a feel-mode is a style choice rather than something
  anyone automates. One `kStateVersion` bump covers both fields, both defaulting
  off.

- **Decision:** The `Swing` VST3 parameter keeps its normalised 0–1 range; what
  changes is what `1.0` *means*, and only with the mode on. — **Why:** `Swing` is
  expression slot 5, `Kind::Unit01`. Widening the mapping unconditionally would
  move every existing preset that sets a swing value, which the milestone's
  byte-identity rule forbids. Off: `swingAmount * stepDurPpq / 3`, untouched.
  On: a tempo-dependent divisor with the ceiling widened to about 0.43 step
  (≈3.5:1), which is the figure issue #149 itself gives.

- **Q:** The swing curve has no published coefficients to copy — Friberg &
  Sundström report a shape and two anchor figures. How should the shipped curve
  be framed? — **A:** Ship the shape, state it is ours.
- **Decision:** Encode a curve reproducing the reported behaviour — about 3.5:1
  reachable at ballad tempi, narrowing toward straight near 300 BPM — with
  coefficients that are Poly's, said so in the comment beside them. — **Why:**
  This is exactly the framing M003 used for the jembe profile: a rendering of a
  published *finding*, not a transcription of a published table. Refining the
  values against a source in hand is then a data change needing no code, and
  claiming they are the measured numbers would be the kind of citation this
  repo's first programme existed to remove.

- **Q:** `M001/S02`'s definition of done says the Humanize pages "no longer
  describe it as jitter where they now mean drift". Only two pages mention
  randomness at all, and neither becomes false — they become imprecise. What
  should that item require? — **A:** Rescope to the two pages and lock the
  correction.
- **Decision:** `appendix-design-decisions.mdx`'s "humanize jitter" and
  `18-editors-and-views.mdx`'s "a different random displacement on every cycle"
  are corrected to name the correlated behaviour, each locked by a
  `scope-framing` claim. The slice's DoD item is rewritten from "the pages" to
  those two. — **Why:** Fourteen pages mention Humanize; two describe its
  mechanism. Editing the other twelve to satisfy a checklist would be prose
  written for a gate rather than for a reader. The finding that neither page was
  actually *wrong* is recorded here so a later reader does not mistake the small
  edit for an oversight.

- **Decision:** Every engine slice in this milestone owes a golden proving the
  45 factory presets render byte-identically with its feature off. — **Why:**
  Inherited from the ledger, which took it from M003/S01. It is what makes a
  timing change reviewable without re-auditing 45 presets by hand.

- **Nothing deferred.** Both slices are planned in full below; neither has a
  question that only becomes answerable once the other has landed.
