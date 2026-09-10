# M003 — decisions

Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

**Scope note.** This file was created during the `/jk:auto` run that executed
M003/S05's last task, so it records S05's decisions in full and does not
reconstruct S01–S04's. Those slices' reasoning is in their commit messages and
in `evidence/M003-S0{1,2,3,4}.md`, which are the contemporaneous record.

## 2026-09-09 — planning M003/S05

- **Q:** F33 asks Rule 5 to name *kotekan polos*, "a third player on structural
  pokok tones". Nothing in the repo attests the term, the audit cites no source
  for it, and Rule 3 already uses *polos* for one of the interlocking pair. How
  should it be handled? — **A:** Write the substance, drop the label.
- **Decision:** Rule 5 gains the practice without the term; the sourcing
  question becomes M006/S04 row B11 — **Why:** M002 spent a milestone on
  citation integrity, and shipping an unsourced term into the same guide would
  undo it.
- **Q:** F34's definition of done says the table should mark its "tupan and
  kaval notes" as GM stand-ins, and the audit refers to a "Note" column. The
  table has no Note column, and the preset shows tupan maps to real drums while
  kaval and gadulka are the stand-ins. Which should be written? — **A:** Add a
  GM column sourced from `presets.json`.
- **Decision:** A `Note` column for all four lanes plus a paragraph naming the
  GM sounds, with the audit's two errors recorded in F34's `Item` cell —
  **Why:** the preset makes every value verifiable, so the lock has a real
  oracle instead of hand-copied numbers.
- **Decision:** The audit's miscount of Rule 5 as listing "five styles" (it
  lists four) is recorded in F33's `Item` rather than raised as its own row —
  **Why:** no outstanding work follows from it.

## 2026-09-09 — executing M003/S05

- **Decision:** F35 cites Tenzer (2000), not Geertz's *Negara* (1980) — **Why:**
  the audit mentions Geertz as the framing's origin but recommends Tenzer, and
  Geertz has no appendix entry; adding one is M006's scope.
- **Decision:** `S05-F35`'s pattern binds the citation to the opening paragraph
  with a bounded span — **Why:** `05-gamelan.mdx` cites Vitale further down, so
  an unbounded match would pass on a page whose opening was still uncited.
- **Q:** Task 2's "cite `fr-tenzer-2000`" could not be followed honestly: no
  source in the repo attests that a third part doubles the pokok rather than
  interlocking. How should the plan be repaired? — **A:** Ground it internally,
  fold the attribution into B11.
- **Decision:** Rule 5's sentence carries no citation and cross-references Rule
  6 and Construction's Pokok lane; B11 grew to own the attribution as well as
  the term — **Why:** citing on the strength of an appendix annotation would
  have swapped an unsourced term for an unsourced attribution, the same defect
  B11 exists for.
- **Decision:** `S05-F33` matches `third part`, not `third (?:part|lane)` —
  **Why:** Construction step 4 already says "a sparse third lane" and the patch
  names a "Pokok melody" lane, so the `lane` alternative would have passed on an
  unchanged Rule 5.
- **Decision:** Rule 6 is cross-referenced in plain text, `(Rule 6)` — **Why:**
  the plan said to use "the anchor convention the page already uses", and the
  page defines no rule anchors; plain-text parentheticals are its convention.
- **Decision:** `S05-F34` reads `site/src/generated/presets.json`, not the
  `site/public/webui/presets.json` the plan named — **Why:** the two are
  byte-identical, and `src/generated` is the copy
  `preset-table-conformance.test.mjs` already imports.
- **Decision:** `S05-F34`'s prose window is bounded to the region between the
  table and the next heading — **Why:** the first draft ran to end of file,
  which would have let a mention anywhere later in the chapter satisfy it.
- **Decision:** the `Note` header sits between `Subdivision` and `Velocity` with
  bare note numbers — **Why:** that is what `PresetTable` emits for the column,
  so a later migration off the hand-written table is a drop-in.
- **Judgment call:** Task 2's step-1 sample `rule:` string still read "cited and
  linked to Rule 6" after the repair. The rule string was written to match step
  3, the definition of done and the F33 row, all of which say the sentence
  carries no citation — **Why:** the intent was unambiguous and a stale
  adjective in an illustrative snippet is not worth a halt. Recorded in
  `evidence/M003-S05.md` and still unswept in the plan.
- **Judgment call:** `site-unit` was run as `npm --prefix site test` rather than
  the plan's `node --test site/tests/`, which collects one file from the repo
  root and exits 1 — **Why:** the plan itself instructs that
  `.jk/validations.yml` is authoritative on disagreement. Corrected in the plan
  by the repair commit.
