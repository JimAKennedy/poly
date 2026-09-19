---
class: gated
---

# M003 — Decisions

Append-only. One entry per decision, with the reason.

## 2026-09-19 — planning M003 (both slices, front-loaded)

**Classification.** Both slices are **bounded**. Pitch assignment already exists
— `ev.pitch = cfg.midiNote`, one line — and this varies what it reads. No new
subsystem, and the three design questions below were taken before any code.

- **Q:** #245 asks for per-note durations. Should a note's duration affect when
  the *next* note starts, or only how long this one sounds? — **A:** Gate length
  only.
- **Decision:** The step grid keeps deciding when notes start; the sequence
  decides which pitch sounds and for how long. — **Why:** Durations that advance
  the clock would be a **second mechanism for placing onsets**, beside
  `subdivisionProfile` and `cellSizes`. Engine-capability M003 spent a slice
  resolving exactly that collision and ruled for one authority rather than two
  combining silently. Keeping pitch orthogonal to placement also means a
  sequenced lane still locks to the ensemble.

- **Q:** Which note does a given hit take? — **A:** By absolute step position.
- **Decision:** `pitch = sequence[absStep mod length]`, not the hit ordinal. —
  **Why:** It still phases — a five-note sequence over a seven-step lane is the
  device #245 asks for — and it is derived from absolute PPQ, so a locate
  reproduces it. The hit-ordinal reading is closer to the issue's wording but
  makes the melody a function of the mutation rolls: one dropped hit re-voices
  everything after it, which is a surprise rather than a feature.

- **Q:** The milestone records as open whether the guide grows to cover pitched
  lanes. What should this run do? — **A:** Ship ahead, and state it in the guide.
- **Decision:** `M003/S02` gains a definition-of-done item: a short passage names
  the capability and says the traditions chapters do not yet use it. — **Why:**
  This programme's premise is that engine and guide never diverge silently, and
  shipping an undocumented field would reproduce exactly that in the other
  direction. Writing full guide coverage is a content milestone's work with
  musicology to source, which a two-slice engine change cannot carry. **The
  slice is widened deliberately and recorded here** rather than the item being
  added quietly.

- **Decision (naming, proposed rather than asked):** `noteSequence` and
  `noteSequenceLength`. — **Why:** #245 flags that `phrase` is taken by
  `phraseLength`/`phraseGap`/`phraseOffset`, which mean gating rather than
  content, and the slice's own definition of done requires the chosen name and
  its reason to be recorded. `noteSequence` collides with nothing and says what
  it holds.

- **Decision:** Eight notes maximum, as #245 proposes as a starting point. —
  **Why:** A separate, smaller bound than `kMaxSteps` keeps the per-lane cost to
  a few dozen bytes. The struct has grown about 1% across this programme and the
  size guard records every increment; spending 64 floats a lane on a field most
  lanes will not use would be the trade M002 explicitly declined.

- **Standing instructions carried from M001 and M002**, which this milestone's
  plans repeat: name the discriminating case, because most cases survive their
  probe; add a render-level case before probing, because unit cases on a helper
  cannot see the wiring; prove a `present`-only site claim by **removing** the
  correction; restore a probed file by explicit edit; and rebuild before reading
  any mutation result.

- **Nothing deferred.** Both slices are planned below.
