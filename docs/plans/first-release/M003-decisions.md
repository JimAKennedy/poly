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

## 2026-09-22 — planned pause in M003/S02

- **Q:** Automation found no obtainable copy for any of the six library-only
  entries. Rewording six claims on that basis assumes none exists, when what was
  established is that none is reachable from here. How should task 2 proceed? —
  **A:** queue them for the owner's browser first.
- **Decision:** the six go to the browser worklist and **FR18 stays open** until
  the owner reports back — **Why:** four are paywalled by publication model
  rather than bot-blocking, so a browser alone does not change them, but the
  owner's Academia.edu and Scribd accounts are exactly where reposted copies of
  canonical articles like Cohn 1992 and Locke 1982 tend to sit. `ref-9` and
  `ref-22` were both settled that way after automation had given up. Rewording
  chapter 7 to introduce "aksak" unsourced, on evidence with a known gap, is not
  a trade worth making to finish a milestone in one run.

- **Decision:** tasks 3 and 4 proceed anyway — **Why:** neither depends on the
  six. Task 3 removes the 67 entries **no shipping page cites**, and all six are
  cited; task 4 annotates another programme's ledger. Stopping the whole slice
  on FR18 would leave work undone that the pause does not block.

- **Consequence:** M003/S02 cannot close, and **M003/S03 is blocked** — its
  guard asserts no shipping entry is `library-only`, which cannot hold while the
  six are there under any outcome except removal. The milestone resumes when the
  owner reports.

- **Plan repair:** task 2's step said *"for each entry task 1 found nothing for:
  the claim must stop depending on it"*. It now queues instead. The rewording
  path is retained in the plan for the entries the browser does not settle,
  because it remains correct for those.

## 2026-09-22 — the browser session, and what it decided

- **Q:** The owner's JSTOR access is a paid JPASS personal subscription. Does a
  paid personal subscription count as "obtainable without an institution"? —
  **A:** yes. It is purchasable by anyone, which is the test; the manifest
  records it as `purchasable` with JSTOR's own price, exactly as a book carries
  its publisher's price.
- **Q:** No copy of Harrison (2025) exists outside its book. Drop the citation
  and reword chapter 13, or keep it? — **A:** keep it and point it at the
  Routledge book. A purchasable chapter of a purchasable book passes the
  programme's test; the description stays `unverified` because the text has not
  been read. The owner may make another pass for a self-archived copy later.
- **Q:** The Grove article a reader can reach is the 2001 second-edition
  rewrite with twenty credited authors, not Powers's 1980 article the entry
  named. Cite the 2001 article, or keep 1980 and reword chapter 6? —
  **A:** cite the 2001 article — **Why:** it is the version that exists to be
  obtained, chapter 6 needs only "Grove's survey", and the entry now says what
  it supersedes so nobody rediscovers the change as a defect.
- **Decision:** the anchor stays `fr-powers-1980` — **Why:** anchors are
  identifiers and are never reassigned (the ledger rule M005/S01 applies to the
  guide's own citations), and `literature-enrichment.test.mjs` pins the name in
  two places for theory-audit F48. The entry text carries the truth; the anchor
  carries continuity.
- **Decision:** `fr-peycheva-dimov-2002`, the one entry cited by nothing, keeps
  its place while recorded as `library-only` — **Why:** the FR19 decision above
  kept it on the strength of its own declaration that its contents are
  unverified and no claim rests on it. An entry that makes no claim cannot fail
  the obtainability test any more than the citation test; applying the
  exemption to one arm and not the other would be two rules where the
  declaration is one. S03's guard exempts the `contents unverified` phrase in
  both arms, and only that phrase, so the exemption stays enumerable with
  `grep -rn "contents unverified" site/`.
- **Consequence:** the appendix holds **41** — 40 cited plus the exempt one —
  not the 34 FR19 first named. The count is reported in the evidence, as the
  plan said it would be, and nothing pins it.
