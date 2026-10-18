---
class: gated
---

# M005 — decisions

## 2026-10-18 — planning M005/S01

- **Q:** Chapter 1's Cycle Independence says co-prime polymetre emerges
  "exactly as it does in the interlocking percussion ensembles of West Africa
  and Southeast Asia". M004 found Locke 1982 supports only simple ratios for
  Ewe drumming, and R7 says the same for Central Africa, so the CA37 aside
  would sit beside a claim it contradicts. Reword the clause in CA37, add the
  aside only and leave the clause, or halt and add a ledger row? — **A:**
  reword it in CA37
- **Q:** Approve the bounded design: CA35 adds a 24-step section to the
  Euclidean fixture with the five E(13,24) mould rotations, an E(7,12)
  rotation 0 row for the bòndò dì.kétɔ̀, and corrects the fixture note's
  gankogui rotation; CA36 adds a composite-subdivision section to the Timing
  Model; CA37 adds the Chapter 1 aside? — **A:** approve
- **Decision:** classify M005/S01 as bounded — **Why:** prose and fixture
  edits to three existing pages through the existing generator, plus site
  tests in the existing harness; no engine, interface or preset change.
- **Decision:** the fixture note's gankogui rotation becomes 2 — **Why:**
  `rotateRight(bjorklund(12, 7), 1)` has gaps 2+1+2+2+1+2+2 read from its
  first onset, not the 2+2+1+2+2+1+2 the note claims; rotation 2 gives that
  sequence, and rotation 2 is what the table row already prints.
- **Decision:** the five mould rows carry chapter `2b` in the Chapter column
  — **Why:** the column is free text to the generator and to
  `appendix-euclidean-claims.test.mjs`, and 2b is the chapter's own number.
- **Decision:** the Chapter 1 rewording drops "Southeast Asia" as well as
  "West Africa" rather than substituting another tradition — **Why:** no
  source cited on the page supports co-prime cycles in any named tradition,
  and the gamelan chapter does not claim it either; the sentence says the
  layering is Poly's own.
