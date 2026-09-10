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
- **Q:** With the nine review defects already fixed, S05's real cost is how many
  of the 92 numbered rules the checklist covers. Which scope? — **A:** Create a
  new milestone for the comprehensive option, then proceed with the narrow scope
  under this slice.
- **Decision:** M007 — Named-rule coverage created, three slices, rows B12–B14,
  depending on M004/S05. S05's definition of done narrowed to the mechanism plus
  the five patches its siblings need and the two theory pages with no assertion,
  and gains an item naming M007 as the owner of the rest — **Why:** the DoD's
  literal reading is a tenfold expansion over 83 never-checked rules, which is a
  milestone rather than a slice; naming the owner in the DoD keeps the boundary
  explicit instead of implied by silence.
- **Q:** When the checklist finds a patch breaking a rule that no F-row covers,
  what should the unattended run do? — **A:** Mark it, open a B row, keep moving.
- **Decision:** an untriaged finding gets a `patch-divergence-ok` marker with the
  reason "found by the checklist, not yet triaged" and a B row naming the page
  and rule; M007/S03's B14 fails on any marker still carrying that placeholder —
  **Why:** the suite stays green and the finding stays greppable and tracked,
  without a slice widening to chase it; the placeholder check stops the backlog
  becoming permanent by inattention.
- **Finding, recorded rather than acted on:** the 2026-07-30 conformance
  review's nine defects are all fixed — the harness is green on all nine. The
  concern that S05 would open red on M069's work was wrong. The real exposure is
  the 83 rules never checked at all, which is what M007 owns.
- **Finding:** `theory-electronic-breakbeat` (9 rules) and `theory-minimalism`
  (8 rules) carry no assertion whatsoever. Folded into S05's scope rather than
  left for M007, because they are the only two pages with zero coverage.

## 2026-09-10 — executing M004/S05

- **Decision, under the pre-agreed policy:** `theory-electronic-breakbeat`'s
  patch violates its own Rule 7 — the kick's E(5,16) at rotation 3 puts an onset
  on pulse 12, one of the snare's two slots. Marked `patch-divergence-ok` with
  the untriaged reason and opened as ledger row B15 under M007/S02, rather than
  fixed here — **Why:** this is the "checklist finds something no F-row covers"
  case, and the fix is a musical choice between moving the rotation and
  qualifying the rule.
- **Judgment call:** Task 2's cases were written with the checklist data and its
  iteration together, so no case was watched to fail for lack of the mechanism —
  the plan's step 2 asked for that red. The mutation round is what establishes
  non-vacuity instead, and every rule was watched to fail on its own terms —
  **Why:** the ordering would have proved the harness ran, and the mutations
  prove each predicate bites, which is the property the definition of done
  actually names. Recorded rather than glossed.
- **Correction:** the plan's step 2 asserts the five seeded rules "hold today".
  Four do. `ebb-kick-avoids-snare` does not, which is how B15 was found.
- **Finding, recorded rather than chased:** the Chapter 2 Ewe table binds
  `preset="Sub-Saharan: Agbekor"`, but the preset's lanes (cowbell 12/7, conga
  5/4, tom 3/2, snare 7/4) agree with the table on lane 1 only — the table reads
  bell 12/7, support 12/3, responding 5/3, lead 7/5. `polypatch-preset-
  resolution.test.mjs` says the attribute "renders only as an italic display
  sub-title; it resolves nothing at runtime", so the contract is that the name
  resolves, not that the lanes match, and S01 is a doc edit rather than an
  engine change — **Why recorded:** a reader who loads the named preset gets
  something different from the printed table, which is a real conformance
  question. It is outside M004's rows and outside the checklist's remit, so it
  is noted here for a human rather than turned into work nobody asked for.
- **Decision:** S01 adds the dance-beat lane as lane 2 rather than appending it,
  renumbering the lanes below — **Why:** the theory page's construction lays
  bell, then dance beat, then supports, then lead, and a table that reads in
  construction order is the thing the reader is meant to follow.
