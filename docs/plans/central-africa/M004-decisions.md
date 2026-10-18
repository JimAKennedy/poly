---
class: gated
---

# M004 — decisions

Append-only. Every question asked at planning, its answer, and every choice made
on the user's behalf during execution.

## 2026-10-18 — planning M004/S01

- **Q:** The Chapter 2 patch table is the "Ewe Polymetric Ensemble" preset
  plus a 12/4 dance-beat lane that theory-audit M004 inserted as lane 2, which
  no preset carries, and it names "Sub-Saharan: Agbekor", which it matches on
  lane 1 only. Make it the Ewe preset exactly (dance beat becomes an
  add-a-lane instruction, `ssa-dance-beat` rewritten to require it), keep five
  lanes under a `Custom:` label, or add the lane to the preset in the engine?
  — **A:** the Ewe preset exactly
- **Q:** Approve the design: bounded; CA29 scopes the description and opening
  to West Africa; CA30 rewrites Polymetric Stacking from Locke 1982 (bell sets
  the period, kagan 4× and kidi 2× per bell pattern, cross-rhythms 3:2 and 3:4,
  sometimes 3:8) and presents 12/7/5 as Poly's extension, also rewording the
  Manding "Ewe stacks many independent cycles" sentence and the closing
  paragraph; CA31 rewrites both downbeat passages; CA33 names rotations 0, 2
  and 9 and cites Locke p. 225; CA34 links `/02b-central-africa/`? — **A:**
  approve
- **Decision:** classify M004/S01 as bounded — **Why:** prose edits to one
  existing chapter plus site tests in an existing harness; no engine,
  interface or preset change.
- **Decision:** Locke 1982 was read from the owner's archived copy, as page
  images, before planning; the quotes the plan permits are listed in the plan
  with pages — **Why:** CA30's verification requires Locke checked first, and
  `pdftotext` clips line ends on this scan, so only the images are a reliable
  reading.
- **Decision:** the Manding sentence "Where Ewe drumming stacks many
  independent cycles" and the closing paragraph are in CA30's scope — **Why:**
  both repeat the uncited cycle-length claim the DoD forbids, and the vision
  names the closing paragraph as building on it.
- **Decision:** the Agbekor preset's bell is at rotation 9 (onsets
  0,2,4,5,7,9,11), derived with `rotateRight(bjorklund(12, 7), 9)` — **Why:**
  CA33 needs a third rotation the page can name, and the preset is the
  record of what that bell plays.
- **Decision:** the chapter keeps the spelling "Ewe" and makes no claim that
  Locke's Atsiagbeko and the preset name "Agbekor" are one piece — **Why:** the
  references record already notes Eʋe/Ewe; the equivalence is not on the page
  of any source the chapter cites.
- **Decision:** the CA31 lock forbids the word "downbeat" anywhere in the
  chapter — **Why:** a narrower lock on the two phrases would let a third
  downbeat assertion in; the rewritten text has no need of the word.

## 2026-10-18 — executing M004/S01

- **Judgment call:** `literature-enrichment.test.mjs` F45 bound the Kubik
  citation to the phrase "it is the polymetric drumming of West and Central
  Africa", which CA29 removes; its `near` now names the rewritten sentence,
  and its `why` no longer says the opening claims the Central Sudanic belt —
  **Why:** the lock's purpose is that the citation stays attached to the
  opening sentence; the anchor follows the sentence, the binding is unchanged.
- **Judgment call:** `chapter-2-reconciled.test.mjs` is added to `REQUIRED`
  with its own comment rather than "beside the central-africa entries" as the
  plan says — **Why:** M003 put its two tests in the runner only, so there
  were no central-africa entries in `REQUIRED` to sit beside.
