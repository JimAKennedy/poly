# M003 — A bibliography a reader can finish

**Review-gate report.** Generated from the ledger, `git log` and
`M003-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** Every reference in the shipping guide supports a claim, is Tier A,
and can be obtained without an institution.

**Branch:** `milestone/M003-bibliography` · **Ledger:** `docs/plans/first-release/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | The two mismatches are corrected | FR16, FR17 | done |
| M003/S02 | Nothing needs an institution | FR18, FR19, FR24 | done |
| M003/S03 | A guard keeps it true | FR20, FR23 | done |

FR23 closes **`accepted`** — the vision's 10–20 target, declined with the
measurement that disproved it; every other row is `done`.

## Definition of done

- [x] `ref-1` names and links the same work
- [x] `ref-34` attributes the article to its author
- [x] Each correction is locked by a claim test seen red before the edit
- [x] No entry in the shipping appendix is `library-only`
- [x] Every claim whose source left carries a source a reader can obtain, or no
      longer asserts something that needs one
- [x] The appendix contains only entries cited from a shipping page
- [x] The verifiable-references ledger records what this programme changed about
      its remaining milestones
- [x] A test fails if an appendix entry is cited from nowhere
- [x] A test fails if an entry is not Tier A, or is `library-only` — B allowed
      by the owner's decision; C fails
- [x] Both arms are shown to fail before being trusted

## What changed

**107 entries became 40**, and the property behind the number is what the
milestone asserts: every entry is cited from a shipping page, is Tier A or B,
and needs no institution. 66 entries left for the theory bundle's own
bibliography, where every one of them is still defined. One was dropped.

**Two mismatches corrected** (S01). `ref-1` now points at Toussaint's 2005
BRIDGES paper in the official archive, read directly, instead of the 2007
eight-author arXiv paper it linked; `ref-34` names Schwarz, not Reich. Each
change was seen red on a claim test first and mutation-proved after.

**Six `library-only` entries settled** (S02). Automation found no obtainable
copy of any; the owner's browser found five and the sixth, Harrison (2025),
now points at its book as purchasable. Reading each title page before
recording caught a mis-saved file and a wrong edition — see below.

**A guard with no exemption** (S03). `appendix-entry-rules.test.mjs` asserts
three properties and never a count. Every arm was named red by mutation, and
an emptied appendix throws in all three.

## Validation

Re-run on `4ce08ff`, the last content commit; the pre-push gate runs again on
the docs-sync head and the PR records that result.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `4ce08ff` |
| `site-unit` | `npm --prefix site test` | pass — **331/331** | `4ce08ff` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass | `4ce08ff` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `4ce08ff` |
| `ledger` | `jk-standards ledger` | pass — 5 conform | `4ce08ff` |

**327 tests before the milestone, 331 after.** Added: two claim tests for the
S01 corrections, and the three-arm appendix guard; retired: the theory-audit
case that pinned the dropped entry's phrase.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `80cf7ef` | M003/S01 | | docs(plans): front-load M003's decisions and plan all three slices |
| `6313f3c` | M003/S01 | FR16 | docs(references): point ref [1] at the Toussaint paper it names |
| `e077b23` | M003/S01 | FR17 | docs(references): attribute ref [34] to Schwarz, not Reich |
| `1e0d667` | M003/S02 | FR18 | docs(references): record the retrieval attempt for the six library-only entries |
| `4a61354` | M003/S02 | | docs(plans): queue the six library-only entries for a browser session |
| `e082222` | M003/S02 | FR19 | docs(references): reduce the appendix to what the shipping guide cites |
| `45abedf` | M003/S02 | FR24 | docs(plans): record what first-release overtook in verifiable-references |
| `65988bc` | M003/S02 | FR18 | docs(references): the six library-only entries are settled, FR18 closes |
| `3d948ab` | M003/S03 | FR20 | test(site): a guard keeps the shipping appendix cited, tiered and obtainable |
| `4ce08ff` | M003/S03 | | docs(references): drop Peycheva & Dimov; the appendix guard has no exemption |

`1e0d667` carries `Rows: FR18` from the retrieval attempt that did not close
the row; `65988bc` is the commit that did.

## What a reviewer should look at twice

### The milestone paused, and the pause was the right call

Task 2 of S02 was written to reword any claim whose source automation could
not find. Automation found none of the six, and rewording six chapters on that
basis would have recorded "none exists" where the evidence said "none reachable
from here". The plan was repaired in place, the six went to the browser
worklist, and every one of them turned out to be obtainable by a person. That
is the strongest evidence yet for the verifiable-references programme's VR10.

### Two things the download found that no listing could

The file first saved as Cohn (1992) had the same MD5 as the Vitale download
saved a minute earlier; it was re-fetched and the replacement's cover sheet
read before its record was written. And the Grove article a subscriber reaches
is the 2001 second-edition rewrite by Qureshi, Powers, Katz, Widdess and
sixteen others, not Powers's 1980 article the entry named — the same class of
defect as `ref-1` and `ref-9`, found because the title page was read. The
entry now cites 2001 and says what it supersedes; `fr-powers-1980` keeps its
name because anchors are never reassigned and a theory-audit guard pins it.

### An exemption was taken and reversed on the same day

The one uncited entry, Peycheva & Dimov (2002), was first carried through
S03's guard on the "contents unverified" declaration FR19 had already
accepted. Asked whether it was still needed, the owner dropped it: uncited,
unread, library-only, absent from the theory bundle, and not about aksak or
wedding music. The guard lost its only exemption; both decisions are on the
decisions page in order.

### A count that moved, and was never pinned

FR19 said 34; the appendix holds 40. The plan said the number would be
reported at close rather than asserted, and no guard pins it. The Demo line
and FR23 were updated to say 40 with the 34 recorded beside it.

## Decisions

Copied from `M003-decisions.md` so the report stands alone.

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

## 2026-09-22 — the orphan is dropped, and the guard loses its exemption

- **Q:** Do we still need Peycheva & Dimov (2002)? It is cited by nothing, Tier
  B, `library-only`, unread, in Bulgarian, absent from the theory bundle, and on
  the zurna rather than on aksak or wedding music — it exists because
  theory-audit F49 named its authors for a subject their work does not cover. —
  **A:** drop it.
- **Decision:** the entry leaves the shipping appendix and the manifest, the two
  theory-audit guards that pinned it are retired with it, and S03's guard has no
  exemption at all — **Why:** the exemption taken earlier today had exactly one
  subject, and a rule with one subject is a carve-out wearing a rule's clothes.
  Chapter 7's aksak material rests on Brăiloiu, Goldberg, Rice and Silverman,
  all Tier A and obtainable; nothing loses a source. This reverses the earlier
  decision on this page, and that entry stays as the record of what was
  considered first.
- **Consequence:** the appendix holds **40**, every one cited, and the guard's
  three properties hold for every entry without a carve-out for any.
