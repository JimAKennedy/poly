# M004 — A stranger can find it, file against it, and follow it

**Review-gate report.** Generated from the ledger, `git log` and
`M004-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** A visitor can tell in one sentence what Poly is, try it in the
browser, download it, set it up in their DAW, and report a problem — all from
pages written for them.

**Branch:** `milestone/M004-front-door` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S01 | The repository accepts a stranger | OS18, OS19, OS20 | done |
| M004/S02 | One sentence says what Poly is | OS21 | done |
| M004/S03 | The README is written for the person downloading | OS22, OS23, OS24, OS25 | done |
| M004/S04 | The site sends readers to the download | OS26, OS27, OS41 | done |

Every row is `done`.

## Definition of done

- [x] A non-collaborator account can open an issue from each template
- [x] The About panel carries a description, the site URL and topics
- [x] `ROADMAP.md` links to issues by label and milestone rather than
      enumerating numbers, so closing an issue cannot make it stale
- [x] The owner has chosen a one-sentence positioning statement and recorded it
      with the reasoning
- [x] The README's opening, the site's meta description, the About description
      and `CLAUDE.md` carry the same statement
- [x] The README's order is: positioning, screenshot, try it in the browser,
      download, DAW setup — then contributing and building
- [x] Signing-secret provisioning moves to a maintainer document the README
      links to
- [x] No internal decision or milestone ID appears in `README.md` or
      `CONTRIBUTING.md`, and a guard fails if one returns
- [x] A contributor can tell from one index which documents under `docs/` are
      for them
- [x] The two stale artefacts are gone
- [x] No page carries the construction banner
- [x] The home page's hero offers the download and the in-browser engine beside
      the guide
- [x] The guide's install section names the bundle a release actually contains

## What changed

**The About panel is filled** (OS19). Description, `poly.jk.digital` and
six topics, set through the API by the owner's permission and read back
into the evidence.

**The roadmap cannot go stale** (OS20). Themes link open-issue queries by
label and point at the delivery ledgers; no issue number remains, and the
repository's lack of GitHub milestones is why the ledgers, not milestone
queries, are named. `check-front-door.mjs` holds it.

**One sentence, four surfaces** (OS21). The owner's sentence opens the README
and `CLAUDE.md`, is the site's meta description, and is the About
description; `positioning.test.mjs` holds the first three verbatim.

**The README reads for the downloader** (OS22). Sentence, screenshot,
features, try it in the browser, download, DAW compatibility, guide,
contributing, building, architecture, licence. `RELEASING.md` is new and
holds the signing section whole. Six identifiers left the README and two
left `CONTRIBUTING.md`; the guard forbids their return and holds the order.

**`CLAUDE.md` names the WebView editor** (OS23), **the stale manifest is
untracked** (OS24), and **`docs/README.md` says which documents are for a
contributor** (OS25), kept complete by `check-docs-index.mjs`.

**The site sends readers to the download** (OS26, OS27, OS41). The banner is
a pre-release notice linking the Releases page; the hero offers Download,
Try it in the browser (the Foundations chapter) and Read the guide; the
guide's install section names the release zips and the `poly_plugin.vst3`
bundle they extract to, and no longer describes an Audio Unit no release
contains. `front-door.test.mjs` and `host-table.test.mjs` hold all of it.

## Validation

Re-run on `686bccf`, the head this report describes. No C++ or CMake
changed on the branch, so no slice owes `unit`.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `686bccf` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `686bccf` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass — 285/285 | `686bccf` |
| `site-unit` | `npm --prefix site test` | pass — 345/345 (338 before the milestone, 7 added) | `686bccf` |
| `guards` | `bash scripts/check-guards.sh` | pass — 19 guard invocations (17 before, 2 added) | `686bccf` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `686bccf` |

Every assertion this milestone added was seen red before it was trusted: the
roadmap, ID and section-order rules on the old files; the docs index with no
index; the positioning test on all three surfaces; the banner, hero and
install tests on the old site.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `90e4cb1` | M004/S01 | | docs(plans): front-load M004's decisions and plan all four slices |
| `ec4cc64` | M004/S01 | OS19 | docs(plans): the About panel says what Poly is, where the guide lives, and what to search for |
| `511bf9e` | M004/S01 | OS20 | docs: the roadmap links label queries and the ledgers, and never an issue number |
| `68270e0` | M004/S02 | OS21 | docs: one sentence says what Poly is, on every surface a stranger reads |
| `dd097a3` | M004/S03 | OS24 | chore: untrack the .bg-shell manifest that predated its ignore rule |
| `c9e23a2` | M004/S03 | OS23 | docs: CLAUDE.md names the WebView editor, not VSTGUI |
| `30cef03` | M004/S03 | OS25 | docs: an index says which documents under docs/ are for a contributor |
| `c3838aa` | M004/S03 | OS22 | docs: the README is written for the person downloading, and signing has its own page |
| `52187c3` | M004/S04 | OS26, OS27 | site: the banner says pre-release, and the hero offers the download and the engine |
| `686bccf` | M004/S04 | OS41 | docs(guide): the install section describes the zip that ships |

## What a reviewer should look at twice

### The About panel changed outside the tree

OS19 was landed with two API calls, not a commit. The evidence records the
read-back; the settings page is where to revise it. The description is the
same sentence the tree now holds, and if the sentence is ever changed in
the tree the panel must be changed by hand.

### The download links point at an empty Releases page

The README's Download section, the hero's Download action and the banner
all link the Releases page, and no release exists until M006 cuts one. That
is the row's own instruction and the ordering the ledger chose; a visitor
between this merge and M006 finds the link and an empty list.

### The roadmap names no milestones because the repository has none

OS20 asked for label and milestone queries. There are no GitHub milestones,
so the roadmap links labels and the delivery ledgers. `CLAUDE.md` and the
README no longer say milestones are the roadmap either. If milestones are
created later, the roadmap should link them.

### The README's bodies were moved, not rewritten

The reorder was assembled from the old sections with their identifiers
removed, so the prose is what it was. Two seams were touched by hand: the
Supported platforms paragraph now sits inside the engine-only subsection,
and one colon became a full stop where the two joined. The `xattr`
instruction stays until M009 deletes it.

### The guide still gives Live, Studio One and FL Studio steps

Unchanged from M003's report: the Load Poly list and the routing-by-DAW
section describe hosts the table calls untested. OS41 rewrote the install
section only; those hosts are M010's.

### Two new guards run on every push

`check-front-door.mjs` and `check-docs-index.mjs` join `check-guards.sh`,
which the pre-push hook runs. They fail on an issue number in the roadmap,
an identifier in the README or CONTRIBUTING, a README section out of order,
or a document under `docs/` missing from the index. Each is a rule a
contributor will meet.

## Decisions

Copied from `M004-decisions.md` so the report stands alone.

# M004 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-29 — planning M004/S01, S02, S03 and S04

Measured before asking, on `main` at `0b4736a`. The About panel is empty:
no description, no homepage, no topics. `ROADMAP.md` lists four issue numbers,
all closed, and its Priority 3 table is empty; the repository has **no GitHub
milestones at all**, so a "milestone query" would link to an empty page. The
README puts Building before Installing, gives signing-secret provisioning its
own section, has no screenshot, never mentions the in-browser engine, and
carries six internal IDs; `CONTRIBUTING.md:16` carries two. `RELEASING.md`
does not exist. `.bg-shell/manifest.json` is tracked and ignored at once.
`docs/README.md` does not exist, and a new document under `docs/` must be
named in the drift map or `doc-completeness` fails. `CLAUDE.md:18` lists
"VSTGUI 4". The site's construction banner is mounted from
`astro.config.mjs`; its hero offers *Start Reading* and *GitHub*; no
playground page exists, and the in-browser engine plays inside chapters
through `PolyPreviewCard`, first in the Foundations chapter
(`/01-foundations/`). `guide-using-poly.mdx` says Poly ships in two formats
and names the bundles `Poly.vst3` and `Poly.component`; the release zips are
`poly-<tag>-<platform>.zip` and each extracts to `poly_plugin.vst3`. No site
test covers the hero, the banner or the config's description.

- **Q:** OS21: which one sentence says what Poly is? — **A:** "Poly is a
  free, open-source polymetric drum sequencer for your DAW: grooves grounded
  in real drumming traditions, a guide that cites where every preset comes
  from, deterministic output, and an engine that runs in your browser."
- **Decision:** that sentence, verbatim, in the README opening, the site's
  meta description, the About description and `CLAUDE.md`, held by a site
  test — **Why:** it names the four things a free Euclidean sequencer does
  not have, which is the difference the ledger says Poly loses a checklist
  without. Alternatives offered and not taken: "An open-source VST3
  instrument that turns the world's drumming traditions into evolving
  polymetric MIDI grooves." (shorter, tradition-led) and "Poly generates
  polyrhythmic drum grooves rooted in the world's drumming traditions: a VST3
  MIDI instrument with 45 documented presets, deterministic output, and a
  browser demo." (product-first).
- **Q:** OS19: may the run set the About panel through the GitHub API, with
  the sentence, `poly.jk.digital`, and the topics vst3, midi,
  euclidean-rhythm, polyrhythm, drum-machine, audio-plugin? — **A:** yes.
- **Decision:** `gh api` sets description, homepage and topics; the evidence
  records the read-back — **Why:** reversible from the settings page, and the
  read-back is the proof the row asks for.
- **Q:** OS27: where does the hero's *Try it* point? — **A:** the Foundations
  chapter.
- **Decision:** `/01-foundations/`, the first chapter with playable preview
  cards — **Why:** no new page, and the reader hears the engine after one
  paragraph of context.
- **Q:** OS26: remove the construction banner or replace it? — **A:** replace
  it with a pre-release notice.
- **Decision:** the banner says the first release is in preparation and
  links the Releases page; a site test forbids the construction text; M006
  retires the notice when it tags — **Why:** the owner's choice; the site is
  verified but the download does not exist yet, and a visitor should be told
  which.

### Taken on the owner's behalf

- **The roadmap links label queries and the ledgers, not milestone
  queries.** No GitHub milestone exists, and inventing one to link would be
  the enumeration problem in a new form. Each theme links an open-issues
  query by label; planned work points at `docs/plans/`. If milestones are
  created later, the roadmap can link them then.
- **A `check-front-door.mjs` guard, in `guards`,** asserts no `#NNN` issue
  reference in `ROADMAP.md` and no `D0NN`/`M0NN` identifier in `README.md`
  or `CONTRIBUTING.md`. One file for the two rows' guards, each assertion
  seen red before its fix.
- **`RELEASING.md` is created, not "kept unchanged".** OS22 assumed it
  existed; it does not. The README's signing section moves there whole, and
  the README's Building section links it for maintainers.
- **The README's install section keeps the `xattr` instruction** for
  unsigned builds until M009 (OS40) deletes it; OS22 reorders, it does not
  re-promise signing.
- **`docs/README.md` gets a guard too**, `check-docs-index.mjs` in `guards`:
  every top-level document and subdirectory under `docs/` must appear in the
  index, the same rule `check-scripts-readme.sh` applies to `scripts/`,
  because an index that silently omits new documents is worse than none. The
  index is declared `cannot_drift` in the drift map with that reason.
- **The hero's three actions replace both existing ones.** *GitHub* leaves
  the hero; the header's GitHub icon remains. The tagline stays.
- **The guide's install section loses the Audio Unit entirely** — the "two
  formats" sentence, the `Poly.component` copy, the AU build-from-source and
  `auval` prose — since OS17 declined it and no release contains it. The VST3
  build-from-source stays. A test forbids `Poly.vst3` and `Poly.component`
  returning.
- **`CLAUDE.md`'s ownership-transfer convention says "VST3"**, not
  "VST3/VSTGUI", in the same edit as the tech-stack line; no VSTGUI target
  exists.
- **S01 runs before S02** as the ledger orders, so the About description is
  set in S01 with the sentence S02 lands in the tree; the sentence is decided
  above, and the read-back in S01's evidence is the same text S02's test
  holds.
