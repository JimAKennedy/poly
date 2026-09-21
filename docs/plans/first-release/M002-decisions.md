# M002 — decisions

Append-only. One entry per decision that shaped the milestone, with the reason,
so a reviewer can see what was chosen on the owner's behalf and what the owner
chose themselves.

## 2026-09-21 — planning M002/S01 and M002/S02

Two questions were put to the owner before either slice was planned. Both came
out of measuring the surface rather than from the ledger, which does not
mention either.

- **Q:** Five links point *into* a deep dive's `#what-breaks-the-idiom` anchor
  from pages that are staying — three in `14-synthesis`, two in
  `appendix-presets`. Unlike the twelve callouts, they carry meaning. What
  happens to them? — **A:** reword to keep the claim, drop the pointer.
- **Decision:** the sentences keep stating that each tradition treats different
  things as idiom-breaking; only the links go — **Why:** the substance is
  already in the shipping prose, so nothing is lost from the guide and nothing
  points at a missing route. Inlining a summary per tradition would copy content
  out of the deep dives, and the two copies would drift before republication.

- **Q:** What goes in the deep dives' own bibliography? — **A:** only the
  entries they cite.
- **Decision:** `theory-references.mdx` carries the 93 anchors the deep dives
  cite — 66 exclusively theirs plus 27 shared with shipping pages — **Why:**
  self-contained for republication, and it leaves M003 free to cut the shipping
  appendix to 34 without touching anything the deep dives depend on. A full
  107-entry copy would duplicate the shared 27 and the copies would drift.

### Findings that correct the ledger

Measured before planning. None changes the shape of the work; all three change
what a row claims.

- **FR11 says twelve test files.** Ten reference a deep-dive *page*. The other
  two of the twelve — `doc-conformance-wiring.test.mjs` and
  `chapter-euclidean-guardrail.test.mjs` — matched on `theory-` because they
  name theory *test filenames* or mention the theory-audit programme in prose.
  Neither is affected by moving pages.

- **FR12 says the doc-conformance runner names three theory test files by path
  and so stops finding them after a move.** It does name them — but the test
  files are not moving. Only the pages are. The row is a non-issue and closes
  `accepted` with that reason rather than being silently dropped.

- **FR07 says twelve chapter callouts**, which is right, but four more shipping
  documents link into the deep dives without carrying a callout:
  `14-synthesis` (3 links), `appendix-presets` (2), `about-this-guide` (1) and
  `appendix-references` (1). Sixteen `.mdx` files link in, not twelve. FR08 and
  FR09 cover the last two; the first two are the subject of the first decision
  above.

### Taken on the owner's behalf

- **Decision:** the twelve callouts are deleted outright rather than reworded —
  **Why:** each is a pure pointer, of the form "for the rules … see [page]".
  There is no claim inside one to preserve.

- **Decision:** the deep dives' 265 outbound links to shipping pages and 14
  links to each other are left untouched — **Why:** shipping pages are not
  moving, and all twelve deep dives move together, so every one of those links
  resolves again the day the bundle is republished.

## 2026-09-21 — scope addition during M002/S01

- **Q:** Can we record the information M002/S01 removed — for example, have the
  deep dives refer back to the presets and explain how they were designed to
  break the idioms? — **A:** yes, as a new row in M002/S02, one section per deep
  dive.
- **Decision:** row **FR25** added to M002/S02, with a fifth definition-of-done
  item and a new task 5 — **Why:** S01 deleted 14 pointers from shipping pages
  into the deep dives, five of them into `#what-breaks-the-idiom` sections. The
  rules still exist in the deferred bundle; the link between a preset and the
  rule it knowingly breaks does not exist anywhere. Recording it from the deep
  dives' side is better than restoring the old direction: the deep dives are the
  pages that state the rules, shipping presets are stable so the references
  cannot rot while the bundle is deferred, and the relationship ends up inside
  the bundle, which is where republication needs it. M002/S02 already owns the
  moved bundle, so doing it there avoids reopening twelve files that were just
  settled.
- **Also repairs:** S05-F2 and S05-F3 collapsed into identical assertions when
  the deep-dive links went, because the link was the only thing distinguishing
  the gamelan break from the Balkan one in the same file. Preset
  back-references give each break a distinguishing string again.

- **Finding:** the new row was first numbered FR16, which already belongs to
  `ref-1`'s row in M003. `scripts/check-ledger-row-ids.mjs` caught it —
  *"row FR16 appears on lines 169 and 200"* — and it is renumbered FR25. The
  guard's own message names the lesson: grep the whole ledger before choosing an
  ID, not just the milestone being edited.

## 2026-09-21 — judgment call during M002/S02 task 3

- **Decision:** the plan's mutation for the drift map was replaced with a
  different one, rather than halting — **Why:** step 3 said to prove the mapping
  bites by touching a source the drift map pairs with a theory doc, and there
  are none: all twelve theory entries are `doc:` + `reason:` completeness
  declarations with no `sources:` key, so nothing can make `doc-drift` fire for
  them. The step's intent was to show the entries still do something after the
  move, and that is provable another way — pointing one at a nonexistent file
  fails `doc-completeness` with exit 1. The intent is met; only the named
  mechanism was wrong.
