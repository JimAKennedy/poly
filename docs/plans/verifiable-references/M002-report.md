# M002 — Every reference has a verdict

**Review-gate report.** Generated from the ledger, `git log` and
`M002-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** Each of the 107 references carries a recorded obtainability status
and a description verified against the source itself.

**Branch:** `milestone/M002-verdicts` · **Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M002/S01 | The manifest exists and cannot drift from the bibliography | VR04, VR05 | done |
| M002/S02 | All 107 carry both verdicts | VR06, VR07, VR08, VR17 | done |

## Definition of done

**S01**

- [x] `site/src/data/references.json` has a declared shape carrying, per entry:
      anchor id, obtainability, description verdict, archive filename, and
      ISBN/DOI where one exists
- [x] A test fails when a bibliography anchor has no manifest record, and when a
      manifest record names an anchor that does not exist
- [x] The test is shown to fail in both directions before being trusted

**S02**

- [x] Every one of the 107 entries has an obtainability verdict with its
      evidence, and a price where the verdict is "purchasable"
- [x] Every entry has a description verdict — verified, mismatch, or
      **unverified** — reached against the source, not against the guide's entry
- [x] Every mismatch is recorded with what the source actually is, so M004 can
      act on it without repeating the work

## The headline

**Eight of 107 citations name something other than what they point at.** Seven
of those are in the 43 numbered entries — **16% of the guide's numbered
bibliography**.

| Entry | Cites | Actually is |
|---|---|---|
| `ref-1` | Toussaint (2005), "The Euclidean Algorithm Generates Traditional Musical Rhythms", BRIDGES | "The Distance Geometry of Music", Demaine, Gomez-Martin, Meijer, Rappaport, Taslakian, Toussaint, Winograd & Wood, arXiv:0705.4085, **2007**. Toussaint is one of eight authors |
| `ref-20` | Yudane, "Introduction to Balinese Gamelan" | "NOTATION FOR GAMELAN BALI — put together by Yudane" |
| `ref-28` | Holzapfel, "Metrical Structure in Turkish Makam Music" | Srinivasamurthy, Holzapfel & Serra (2014), "In Search of Automatic Rhythm Analysis Methods for Turkish and Indian Art Music" |
| `ref-29` | Holzapfel, "Metrical Strength and Syncopation Distribution in Turkish Usul" | Holzapfel & Bozkurt (2012), "Metrical Strength and Contradiction in Turkish Makam Music" |
| `ref-31` | Aji, "Arab Rhythmic Cycles (Iqa'at)" | Aji, "Rhythmic-Temporal Disruptions and the Feeling of Ṭarab" |
| `ref-34` | Reich, "Music as a Gradual Process, Part II" | K. Robert Schwarz, "Steve Reich: Music as a Gradual Process Part II", *Perspectives of New Music* 20(1/2) |
| `ref-42` | Schloss, "The Brazilian Groove: Ginga and Rhythmic Feel" | "Ginga: a Brazilian way to groove" by **Jovino Santos Neto** |
| `fr-holzapfel-2015` | Holzapfel, "Metrical Structure in Turkish Makam Music" | No work of that title exists in Crossref by any author — the same phantom as `ref-28` |

**`ref-1` is the guide's foundational citation.** The entire generator rests on
the Euclidean-rhythm claim, and the reference for it names one paper and links
another. Both are real and both discuss Euclidean rhythms, so the link is not
useless — but it is not the work the entry describes.

**`ref-42` credits the wrong person entirely.** The URL is fine and the PDF is
real and on topic; the entry names the faculty member whose course directory
*hosts* the file rather than the author of the paper.

**Not one of these eight would be caught by a link checker.** Every URL
involved returns 200 and serves a real document.

## Final distribution across all 107

| Obtainability | | | Description | |
|---|---|---|---|---|
| open-access | 33 | | **verified** | **23** |
| purchasable | 22 | | **mismatch** | **8** |
| browser-only | 19 | | unverified | 76 |
| library-only | 18 | | | |
| borrowable | 15 | | | |

**59 entries now carry a DOI or ISBN, against 2 when the milestone began.** All
64 Further Reading entries had no URL, DOI or ISBN of any kind; 45 of them now
have one.

`verified` means the text itself was read. 23 entries clear that bar. The other
76 are `unverified` with the reason recorded — paywalled, account-walled,
bot-blocked, or a book with no free copy — which is VR07's point: silence that
reads as diligence is the defect, and a recorded "not checked, here is why" is
not silence.

## Validation

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `51eac90` |
| `site-unit` | `npm --prefix site test` | pass — 322/322 | `51eac90` |
| `guards` | `bash scripts/check-guards.sh` | pass | `a0b64a3` |
| `ledger` | `jk-standards ledger` | pass — 4 ledgers conform | `51eac90` |

Every guard was mutation-proved. S01: five shape mutations, three completeness
mutations across both arms, and a personal-path mutation proved red in **two
independent places** — the test and `check-personal-paths`. S02: three mutations
on the queue guard.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Rows | Subject |
|---|---|---|
| `74a1ede` | — | front-load M002's decisions and plan M002/S01 |
| `d7d9c72` | VR04 | declare the manifest shape and seed 107 records |
| `a2d69a8` | VR04 | the manifest and the bibliography cannot drift apart |
| `a0b64a3` | VR05 | resolve the archive root from the environment |
| `51121f9` | — | plan M002/S02 as seven passes and a close-out |
| `9104fc1` | VR06, VR07 | Chapters 1–3, and a defect in ref [1] |
| `e6d9ab3` | VR06, VR07, VR17 | Chapters 4–7, and four more mismatches |
| `bbef48d` | VR06, VR07 | numbered bibliography complete — 7 of 43 |
| `8e313b0` | VR06, VR08 | Further Reading's first identifiers |
| `dd3e031` | VR06, VR07, VR08 | the same phantom citation in both lists |
| `bb92a3a` | VR06, VR08 | Minimalism through Synthesis, VR08 at its purest |
| `5b20e76` | VR06, VR07, VR08 | all 107 carry both verdicts |
| `51eac90` | VR06, VR07, VR08, VR17 | queue every unreachable entry, close S02 |

## What a reviewer should look at twice

### The price field is the loosest thing in this milestone

VR06 requires a price whenever obtainability is `purchasable`. 22 entries are
purchasable and **one** carries a real figure — Kubik (1999) at $35.00 from
University Press of Mississippi. The other 21 record the ISBN, the publisher,
and the statement that no price was established.

Publisher product pages proved reachable for about one book in three by URL
alone, so a real figure everywhere would have cost two or three fetches per book
at roughly half success, and would have made the verdict depend on whether a
scraper happened to work. **This is a judgment call, not an agreed decision, and
the owner may want real figures throughout.**

### `verified` was held to mean "read", which makes the number look worse

23 of 107. A metadata-match standard would have put it near 80. The owner chose
"open every source that can be opened" over "metadata match, then stop", so
letting a Crossref hit count as verification would have quietly restored the
rejected option. The metadata is recorded in every case, so M004 never repeats a
lookup.

### One entry was missed, and the guard could not catch it

`fr-powers-1980` was extracted in task 5 and never recorded, leaving the count
at 106 of 107. It was found by asking the manifest which anchors were still
`unassessed`, not by the completeness test — which catches an anchor with **no
record**, but not a record left at its **default verdict**. That is a real limit
of M002/S01's guard and it is recorded rather than quietly fixed.

### The two-list structure now has a concrete cost

`ref-28` and `fr-holzapfel-2015` carry the same phantom title, one in each list.
Correcting either would leave the other standing. That is an argument for M005
merging them that did not exist as evidence before this pass.

Separately, `ref-35` and `fr-scherzinger-2010` are the same work in both lists —
not an error, but the same maintenance burden.

### 19 entries are queued for a browser session

Grouped by host, because hosts fail as a class. Five YouTube, four with no URL
recorded at all, two Oxford Academic, two `martinscherzinger.org`, and one each
for Érudit, IFTAWM, PubPub, Academia.edu, Scribd, Semantic Scholar and
`lianproductions.com`. The IFTAWM one settles VR17.

### A second Anubis-200, which sharpens M006

`erudit.org` returns **HTTP 200** with "Making sure you're not a bot!" for a
`.pdf` URL, exactly as `emusicology.org` does. Two independent instances. M006's
definition of done currently requires only that 403 and 406 be distinguished
from 404 — a 200 carrying no document passes that check cleanly, so VR16 needs
to assert something about the body or content type.

## Decisions

Verbatim from `M002-decisions.md`:

## 2026-09-20 — planning M002/S01 and M002/S02

Four questions were put to the owner before either slice was planned, with the
measurements that made them answerable: 43 numbered entries and 64 Further
Reading, 107 total; 78 Tier A, 13 B, 16 C; two entries carry a DOI, 44 carry any
URL, and **no** Further Reading entry carries one.

- **Q:** How deep should verification go before recording `unverified`, given
  107 entries of which 64 have no URL at all? — **A:** open every source that
  can be opened.
- **Decision:** the description verdict is reached by reading the document where
  the document can be reached, not by matching metadata — **Why:** the owner's
  call, and the strongest available reading of "against the source, not against
  the guide's entry". It would have caught `ref-9` directly rather than by luck:
  every metadata field there matched a real dissertation, and only the text
  showed the subject was African pianism rather than drumming. **Stated cost:**
  this is the long tail the milestone's own note warns about, and several
  scholarly hosts refuse automation outright, so a large share of entries will
  reach the browser worklist rather than being settled in this pass. Which
  entries went to the queue, and why, is recorded per entry.

- **Q:** How should the manifest point at archived PDFs without committing a
  personal path? — **A:** bare filename plus an environment-supplied root.
- **Decision:** the manifest stores `ref-9-oluranti-2012.pdf` and nothing more;
  `POLY_REFERENCES_ARCHIVE` supplies the root at read time, defaulting to a
  gitignored `.references/` in the project root — **Why:** `check-personal-paths`
  rejected the absolute form in the vision's own first draft, and this makes the
  guard green by construction rather than by review. The owner's Dropbox path
  lives in their shell, not in git.

- **Q:** How should the manifest be structured? — **A:** keyed by anchor, rich
  record.
- **Decision:** an object keyed by anchor id, each record carrying
  obtainability, evidence, check date, description verdict, note, identifier,
  price, and archive filename — **Why:** keying makes duplicate anchors
  structurally impossible rather than something a test has to exclude, and the
  definition of done requires every one of those fields to have somewhere to go:
  VR06 wants a price, VR07 wants what the source actually is.

- **Q:** How should S02's 107 entries land as commits? — **A:** one commit per
  section of the bibliography.
- **Decision:** roughly six to eight commits following the appendix's own
  grouping, each naming what it verified and what it could not — **Why:** a
  107-record diff is one no reviewer can read, and a halt mid-pass leaves a
  coherent partial state rather than losing the work.

### Deferred

- **Q:** The queue guard M001/S01 left dominated — generalise it into the
  worklist mechanism M003/S02 needs, or retire it? — **Deferred to:** M003's
  planning. It is not a blocker for M002: nothing in this milestone reads or
  writes that guard, and the decision wants M003's requirements in view.

## 2026-09-20 — judgment calls during M002/S02

- **Decision:** `verified` means the text itself was read. Where a work exists
  and its bibliographic record matches the entry but the text is paywalled or
  needs an account, the verdict is `unverified` with the metadata confirmation
  recorded in `evidence` — **Why:** the owner chose "open every source that can
  be opened" over "metadata match, then stop", and letting a catalogue match
  count as `verified` would quietly restore the option they rejected. The
  metadata still goes in the record, so M004 never repeats the lookup.

- **Decision:** where a book is in print but no price could be obtained, `price`
  records what is actually known — the ISBN, the publisher, and that the price
  was not established — rather than a figure — **Why:** VR06 requires a price so
  a reader can decide, and an ISBN plus publisher supports that decision, while
  a fabricated or guessed figure would not. Publisher pages proved reachable for
  roughly one book in three by URL alone, so a real figure per book would cost
  two or three fetches each at ~50% success and would make the verdict depend on
  whether a scraper happened to work. Where a price *was* obtained it is
  recorded with its source and date, as for Kubik (1999) at $35.00.
  **Flagged for the review gate:** this is the loosest reading of VR06 in the
  milestone, and the owner may prefer real figures throughout.
