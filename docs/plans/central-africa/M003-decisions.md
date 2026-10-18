---
class: gated
---

# M003 — decisions

Append-only. Every question asked at planning, its answer, and every choice made
on the user's behalf during execution.

## 2026-10-16 — planning M003/S01 and M003/S02

- **Q:** The chapter goes directly after Chapter 2, and every chapter title
  carries its number. Number it "2b." with no renumbering, renumber chapters
  3–18, or leave it unnumbered? — **A:** "2b." — no renumber
- **Q:** Ex. 90 (ngbɔ̀lù drums alone, p. 281) has no tempo marking. Use ex.
  104's ♩. = 144, or leave the card at the site's default? — **A:** use ex.
  104's ♩. = 144
- **Q:** Should a copy of the owner's research note be committed to the repo?
  — **A:** no; the chapter cites Arom only, and every quote and grid is
  re-checked against the scan
- **Q:** Approve the design: both slices bounded; one page,
  `02b-central-africa.mdx`, in the vision's CA3 order; every quote checked
  against the scan image; an oddity-cell test in S01; eleven formula sections
  with cards, grids tagged by preset name, and a grid-against-`presets.json`
  test in S02; host BPM = pulsation tempo × values per pulsation × step length
  in beats? — **A:** yes
- **Decision:** host quarter-note BPM = Arom's pulsation tempo × values per
  pulsation × the step length in beats — **Why:** a pulsation is that many
  Poly steps, so the host must run that many beats per pulsation; for the Aka
  formulae at 1/8 the factor is 1.5, for gàzà (8 values at 1/16) 2, for
  kpóníngbó (4 at 1/16) 1, for àgà-tɔ̀rúmɔ̃ (5 at 1/16 with Tempo Mult 1.25) 1
- **Decision:** àgà-tɔ̀rúmɔ̃ takes ex. 100's marking (two tied quavers = 110,
  p. 285) and zɔ̀bɔ̀kɔ̀ takes ex. 115's ♩. = 160 (p. 294) — **Why:** ex. 99 and
  the zɔ̀bɔ̀kɔ̀ grids carry no marking; exx. 100 and 115 are the same music in
  staff notation, read from the scan on 2026-10-16
- **Decision:** quotes are checked by reading rendered scan pages, rotated
  where printed upside down — **Why:** no OCR tool is installed; the note's
  own OCR check is the owner's, and the DoD asks that each quote be found on
  its page in the scan

## 2026-10-16 — judgment calls in M003/S01 and M003/S02 task 1

- **Decision:** the planning entry's "two tied quavers = 110" for ex. 100 is
  corrected to a dotted quaver tied to a quaver = 110, and bòndò's ex. 102 is
  on p. 287, not p. 286 — **Why:** a close reading of the scan at 1200 dpi
  found a faint dot on the first note, matching ex. 123 (p. 301), and found
  ex. 102 on p. 287; the àgà marking is still five values to one pulsation,
  and neither page change moves a BPM
- **Decision:** zɔ̀bɔ̀kɔ̀ takes ex. 115's ♩. = 160, not p. 297's prose "♩ = 160"
  — **Why:** the zɔ̀bɔ̀kɔ̀ subdivision is ternary (p. 297 itself), so the
  pulsation is a dotted crotchet; the undotted prose is a misprint, and the
  page says which it follows
- **Decision:** the formulae sit between the norms and the build procedure —
  **Why:** the vision's CA3 order puts the formulae (item 5) and the mould
  (item 6) before the procedure (item 7)

## 2026-10-16 — judgment calls in M003/S02 task 2

- **Decision:** the page says mò.mbénzélé's dì.kétɔ̀ sits four pulsations after
  mò.nzòlì's, where the research note says one — **Why:** p. 298's offsets
  (2 and 6 pulsations) and the presets' rotations (22 and 10) both give four
- **Decision:** the lane tables are read from presets.json, so the bòndò
  rattle and the kpóníngbó pellet bells appear as drawn lanes — **Why:** the
  page describes the preset the card plays; M002 built those two lanes drawn
