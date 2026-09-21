# M002 — The deep dives stop shipping

**Review-gate report.** Generated from the ledger, `git log` and
`M002-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** No theory deep dive renders, nothing links to one, and every guard
that asserts their content still runs.

**Branch:** `milestone/M002-unpublish-theory` · **Ledger:** `docs/plans/first-release/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M002/S01 | The site stops pointing at them | FR06, FR07, FR08, FR09 | done |
| M002/S02 | The pages move, and the guards follow | FR10–FR15, FR25 | done |

FR12 closes **`accepted`**; every other row is `done`.

## Definition of done

- [x] No navigation entry offers a theory deep dive
- [x] No chapter carries a `:::note[Theory deep dive]` callout
- [x] No prose anywhere on the site tells a reader the deep dives exist
- [x] The site builds with no broken internal link
- [x] No `theory-*` page is a published route
- [x] Every guard that asserted a deep dive's content still runs, against the new path
- [x] The moved bundle carries the references only it cites
- [x] `jk-standards.yaml` records why a sibling directory falls outside its roots
- [x] Each deep dive records which shipping presets knowingly bend its rules

## What changed

Twelve pages moved to `site/src/content/theory/` with `git mv`, **imports
byte-identical** — `'../../components/PolyPatch.astro'` resolves to the same
file from the new directory, which is why the destination is a sibling under
`site/src/content/` and not `docs/theory/`.

The build emits **35 pages and zero theory routes**, verified by inspecting
`site/dist` rather than by the absence of an error.

The bundle carries `theory-references.mdx` — **93 entries**, derived from the
moved pages rather than curated — so M003 can cut the shipping appendix to 34
without dangling a single citation.

## Validation

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | **327 / 327** |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0 |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | exit 0 |
| `guards` | `bash scripts/check-guards.sh` | exit 0 |
| `ledger` | `jk-standards ledger` | 5 ledgers conform |

**324 tests before the milestone, 327 after.** The three added are the two
bibliography arms and the preset back-reference guard. The count never fell
below its starting point except in task 1, which was red by design.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Rows | Subject |
|---|---|---|
| `f23ea86` |  | docs(plans): front-load M002's decisions and plan both slices |
| `dd2c4ac` | FR06 | docs(site): the sidebar stops offering a theory section |
| `2c04225` | FR07 | docs(guide): remove the twelve theory deep dive callouts |
| `cef0fe8` | FR07, FR08, FR09 | docs(guide): remove the last references to the theory deep dives |
| `f1d21bc` |  | docs(plans): add FR25 — the deep dives record which presets bend their rules |
| `85379f5` | FR10 | refactor(site): move the theory deep dives out of the published docs |
| `af4c155` | FR11, FR12 | test(site): the guards follow the deep dives to their new root |
| `af426b4` | FR13 | chore(docs): repoint the drift map at the theory pages' new root |
| `9c98969` | FR14 | feat(theory): the deferred bundle carries its own bibliography |
| `0275864` | FR25 | docs(theory): record which presets knowingly bend each tradition's rules |
| `a3eb08a` | FR15 | chore(standards): record why the theory bundle sits outside both doc roots |
| `ffad575` | FR10 | docs(plans): close FR10, which task 1 left open |

## What a reviewer should look at twice

### A row was left open twice, and the cause is structural

`FR06` in S01 and `FR10` in S02 were both left `open` by the task whose trailer
named them. Neither was visible until the slice claimed `done`, because
`jk-standards ledger` only enforces row closure at that point — five tasks later
in S02's case.

Both were then committed against an unread result: the block that set the slice
`done` printed `1 violation(s) found` and the commit ran in the same block.
**That is the third time in this programme** that validation and commit shared a
block and the result went unread — M001 carries the other two.

The fix is not resolving to read more carefully. A task's plan checkbox and its
ledger row are one action described in two files, and they are already scripted
together; closing the row in the same edit that ticks the box is what removes
the gap.

### The suite was the census, three times over

The plan said **ten** test files needed repointing. **Six** did, and they were
not the six a grep would have named.

Five failed on `ENOENT`. Four of those five read shipping chapters **and** deep
dives in the same test, so a blanket repoint of `DOCS` would have broken them —
each got a `docRoot(file)` resolver instead. The sixth, `citation-tier`, never
appeared in any `ENOENT`: its claim helper reports a missing file as a *failed
claim*, so an `ENOENT` grep would have missed it entirely.

Two more guards failed for reasons unrelated to path resolution:
`M006/S02: every appendix entry is cited by a page` used `readdir(DOCS)`, so the
66 entries only the deep dives cite looked orphaned; and `S01-F24` requires
exactly twelve deep dives and found none — then found **thirteen** once
`theory-references.mdx` existed.

### A plan step was impossible, and a better mutation existed

Task 3 said to prove the drift map still bites by touching a source it pairs
with a theory doc. There are none: all twelve entries are `doc:` + `reason:`
completeness declarations with no `sources:` key, and the run said so —
`doc-drift: no mapped sources touched`.

The intent was provable another way. Pointing one entry at a nonexistent file
fails `doc-completeness` with exit 1, which establishes what the step actually
wanted: the entries are **not** inert after the move.

### FR25 was added mid-milestone at the owner's request

M002/S01 deleted fourteen pointers into the deep dives, five of them into
`#what-breaks-the-idiom` sections. The rules survived; the link between a preset
and the rule it knowingly breaks survived nowhere. FR25 records it from the deep
dives' side, where it cannot rot while the bundle is unpublished.

The pairs were **recovered, not recalled** — `git show` returns the five asides
verbatim, and the guard's `rule` strings name each rule. All five preset names
were checked against `presets.json`.

It also repairs collateral damage from S01: `S05-F2` and `S05-F3` had collapsed
into identical assertions when the links went, because the link was the only
thing distinguishing the gamelan break from the Balkan one in the same file.

### A guard from another programme was narrowed, with the owner's decision

`idiom-break-framing.test.mjs` (theory-audit M071 S05) required both the
`:::note[Bending the idiom]` aside **and** a deep-dive link. The run halted
rather than deciding alone. The owner chose to keep the aside and drop the link;
all four FRAME cases were **re-proved red** after the narrowing, because a guard
nobody re-proves after weakening it is worse than no guard.

## Decisions

Verbatim from `M002-decisions.md`:

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
