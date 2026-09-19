---
class: gated
---

# M003 — Review report

Status: current (2026-09-19)

Generated from `docs/plans/guide-parity/ledger.md`, git, and
`M003-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** A lane emits a sequence of pitches with their own durations, so Poly's polymetric machinery applies to melodic material and not only to percussion.

**Branch:** `milestone/M003-pitch`, cut from `main` at `76e9fcc`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | A lane emits a sequence of pitches | GP07 | done |
| M003/S02 | The sequence reaches a factory preset | GP08 | done |

## Definition of done

**M003/S01**

- [x] A lane can carry an optional sequence of pitches with per-note durations, supplying successive hits' pitch instead of the single `midiNote`
- [x] The field's name does not collide with the existing `phrase*` fields, and the chosen name is recorded with its reason
- [x] Every existing lane feature — drift, kotekan complement, tempo multiplier, additive cells — applies unchanged with a sequence set, asserted for at least two of them
- [x] With no sequence set, all 45 factory presets render byte-identically, proved by a golden test
- [x] A pre-bump state loads as the single-pitch behaviour it played

**M003/S02**

- [x] A lane's note sequence is expressible in a preset and reaches `site/src/generated/presets.json` under a raised schema version
- [x] At least one factory preset uses a sequence, and its lanes' pitches are asserted against the generated data
- [x] The preset count and any per-lane field-count guards are updated rather than bypassed
- [x] The guide names the capability and says the traditions chapters do not
      yet use it, locked by a `scope-framing` claim *(added at planning — see
      `M003-decisions.md`)*

## Validation

Run on the current head.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `unit` | `ctest --test-dir build` | pass, 691 tests |
| `engine-isolation` | `ctest --test-dir build-engine` | pass, 568 tests |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` | pass |
| `site-unit` | `npm --prefix site test` | pass, 295 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |
| — | `jk-standards ledger` | 3 ledgers conform |

## Traceability

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `5797fa9` | docs(plans): front-load M003's decisions and plan both slices | M003/S01, M003/S02 | — |
| `43dbe4b` | feat(engine): a lane can emit a sequence of pitches | M003/S01 | — |
| `2024d34` | test(engine): drift and kotekan still apply to a sequenced lane | M003/S01 | — |
| `803fc4d` | feat(engine): serialize the note sequence at state version 22 | M003/S01 | GP07 |
| `3a35d75` | feat(presets): Reich Phasing plays a melodic cell, not a repeated pitch | M003/S02 | — |
| `e516971` | docs(site): the guide names note sequences and what they do not do | M003/S02 | GP08 |

6 commits, **0 untraced**.

## What a reviewer should look at twice

1. **A false green from the probe harness itself.** The per-note-duration probe
   reported *passing* when run through a shell helper that edited, `touch`ed,
   rebuilt and ran `ctest` in quick succession — back-to-back touches inside one
   second defeat make's timestamp comparison, so `ctest` ran the previous
   binary. Applying the same probe by hand showed the real result, `0.125 vs
   0.125`. This is the **fourth** stale-build incident of the programme and the
   first inside the tooling built to avoid it; every later probe here waited a
   full second between edits.

2. **The byte-identity golden fired, by design, and was reconsidered rather
   than weakened.** M003/S01 wrote it as "no preset ships a sequence" — true
   then, and built to fail the moment one did. M003/S02 made it fail. The claim
   worth protecting is that an *unsequenced* lane is unmoved; a sequenced one is
   supposed to sound different. The sweep now renders every preset twice and
   also asserts at least one preset *does* carry a sequence, so the coexistence
   claim is not made against a tree where none exist.

3. **Two mistakes from engine-capability M003/S01, avoided.** That slice raised
   `schemaVersion` in the emitter's doc comment but not the string it writes,
   and the generator's gate caught it; and it missed
   `presets-json-schema.test.mjs` entirely, because that file runs under
   `site-unit` and the slice owed only engine tokens. Both were done together
   here, first time.

4. **A serialization task that existed from the start.** M001/S01 shipped two
   fields unserialised and had to be reopened after closing — its plan had no
   serialization task while its sibling's assumed one. This plan carried one as
   task 3 from the day it was written. That is the correction applied rather
   than described.

5. **Reich Phasing is the preset, and the reason is musical.** Its two lanes
   were identical at pitch 76 with one drifting — the technique's rhythm without
   its substance, since Piano Phase and Violin Phase phase a *pitched* cell. A
   sequence on a kick lane would have demonstrated the field while
   misrepresenting the preset. The cell is ours, in the spirit of the piece
   rather than a transcription, as the samba and jembe profiles are.

6. **The two design decisions that keep this orthogonal.** Durations set gate
   length only, so a sequence never becomes a second mechanism for placing
   onsets beside `subdivisionProfile` and `cellSizes` — the collision
   engine-capability M003 spent a slice resolving. And pitch indexes by absolute
   step, not hit ordinal, so a locate reproduces it and a dropped hit does not
   re-voice everything after it. Both are asserted by cases, not just stated.

7. **This milestone does not close a guide gap** — it opens territory the guide
   does not describe, and S02 was **deliberately widened** at planning to say so
   in `18-editors-and-views`. The section states where a tradition would use a
   sequence is a question the chapters have not been written to answer, rather
   than implying coverage that does not exist.

## Decisions

Verbatim from `M003-decisions.md`.

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
