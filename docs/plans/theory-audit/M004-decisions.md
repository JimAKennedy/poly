# M004 — decisions

Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

## 2026-09-10 — planning M004, front-loaded

- **Q:** M004/S05 builds the rule-conformance suite and the divergence-marker
  contract that S01–S03 name as their verification, but it declares `Depends` on
  S01–S04. Which way should the milestone run? — **A:** S05 first, machinery
  then chapters.
- **Decision:** S05's `Depends` drops the sibling slices and keeps M003/S03;
  S01–S03 gain `Depends: M004/S05`; S04 stays independent because the tihai
  worked example needs neither the checklist nor the marker — **Why:** with S05
  last, the escape hatch would be reached for before it was designed, and each
  chapter slice would assert ad hoc against an oracle that did not exist yet.
- **Q:** F40 says Chapter 3's patch has "conga at 10% mutation and quinto at
  30%" against the one-free-voice rule. The preset has `mutationRate` 0.00 on
  all five lanes and there is no quinto lane. How should the row be handled? —
  **A:** Close as already-satisfied, with evidence.
- **Decision:** F40's disposition changes from `patch-align` to `verify`; its
  `Item` records that the premise does not exist and names M070's preset
  conformance as the likely cause; it closes on a lock asserting the
  one-free-voice rule still holds — **Why:** a row that closes because someone
  looked and saw nothing wrong is indistinguishable from one nobody checked; a
  lock makes the claim falsifiable and catches a future regression.
- **Q:** F37's definition of done permits either carrying Chapter 2's missing
  dance-beat lane or cross-referencing the theory page's fuller construction in
  band. Which? — **A:** Add the dance-beat lane.
- **Decision:** The Ewe patch gains a low drum at 12 steps, 4 hits — E(4,12),
  every third pulse — and the table gains a `Timeline` column — **Why:** the
  milestone's Vision is that a patch follows its page's rules or says why not,
  and there is no reason this one should not follow them. The column is what
  makes the second definition-of-done item true, since the bell's timeline mode
  is already `true` in the preset but invisible in the table.
- **Decision:** F37's `Item` records that the audit's Rule 1 half is already
  satisfied — the `Ewe Polymetric Ensemble` preset carries `timeline: true` on
  its bell lane — **Why:** the row would otherwise licence work that is already
  done, and the remaining gap is narrower than the audit describes.
