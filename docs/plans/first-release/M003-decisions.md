# M003 — decisions

Append-only. One entry per decision that shaped the milestone, with the reason,
so a reviewer can see what was chosen on the owner's behalf and what the owner
chose themselves.

## 2026-09-21 — planning M003/S01, S02 and S03

Two questions were put to the owner before any slice was planned. Both came out
of measuring the milestone's own rows against the tree.

- **Q:** FR18 forbids `library-only` entries, but the six that are include
  Brăiloiu 1951 (the origin of "aksak"), Cohn 1992 (the canonical
  phase-shifting analysis) and Locke 1982 (the offbeat-timing principles
  chapter 2 builds on). Each supports a real claim. What happens to them? —
  **A:** find an obtainable copy; keep the citation where one exists.
- **Decision:** each of the six is searched for an open-access or borrowable
  copy — DOI, author's site, institutional repository, archive.org — and kept,
  repointed, where one is found. Only where none exists is the claim reworded or
  the entry dropped — **Why:** dropping all six applies the rule at the cost of
  the thing the rule protects. Chapter 7 would introduce "aksak" with no source,
  chapter 8 would lose the canonical Reich analysis, and chapter 2's founding
  principles would become unattributed assertions. Testing the principle per
  entry is honest; assuming the answer is not.

- **Q:** FR20's guard requires every shipping entry to be Tier A, and
  `fr-linn-attack-2020` is Tier B — the Attack Magazine interview in which Roger
  Linn describes the MPC's timing in his own words. — **A:** keep it, add the
  URL, and let the guard allow B.
- **Decision:** the guard requires tier A **or** B and forbids C — **Why:** it
  is a primary source, the designer describing his own machine, cited in
  preference to commentary quoting him second-hand, and Tier B is the honest
  label for a magazine interview. Promoting it to A would call an interview
  scholarship, which is the inaccuracy this programme exists to remove. Tier C
  is what actually marks unusable material — YouTube, marketing blogs, study
  guides — and every Tier C entry was cited only from the deep dives, which are
  now unpublished.

### A row's number is now provisional

**FR19 says the appendix holds 34 entries.** That figure was computed as
40 cited minus 6 `library-only`, which assumes all six are dropped. Under the
decision above, any entry with an obtainable copy is kept, so the final count is
**34 plus however many of the six survive** — between 34 and 40.

The row's verification clause names a number; the property it is really testing
is *every entry is cited from a shipping page and obtainable without an
institution*. The count is reported at close rather than asserted in advance,
and the guard in S03 tests the property rather than the number — a guard pinned
to 34 would fail the moment an entry is legitimately added.

Measured before planning, on this branch:

| | |
|---|---|
| appendix entries | 107 |
| cited by a shipping page | 40 |
| uncited, removed by FR19 | 67 |
| of the kept set: `library-only` | 6 |
| of the kept set: mismatches | 2 — `ref-1`, `ref-34` |
| of the kept set: not Tier A | 1 — `fr-linn-attack-2020` (Tier B) |
