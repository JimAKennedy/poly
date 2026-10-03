# M003 — The archive exists

**Review-gate report.** Generated from the ledger, `git log` and
`M003-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** Every source that can be obtained is in the archive, and the
repository records what it holds without holding it.

**Branch:** `milestone/M003-archive` · **Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | Scripted retrieval fills what it can | VR09 | done |
| M003/S02 | The browser worklist finishes the archive | VR10 | done |

## Definition of done

**S01**

- [x] Every source classified as a plain fetch is in the archive under the
      `NN - Name.pdf` convention, with the manifest updated to match
- [x] The retrieval reads its destination from the environment and fails with a
      clear message when unset, rather than writing somewhere arbitrary
- [x] Sources that resist scripted fetching are recorded as such, not retried
      silently

**S02**

- [x] A worklist names every browser-only source: URL, what to save, and the
      exact filename to save it as
- [x] The worklist has been worked and the archive contains its results
- [x] Sources that are free but only as unsearchable scans, or free only to an
      institution, are recorded as what they are rather than as archived

## The headline

**The archive holds 45 of the bibliography's works, and every free source is
accounted for.** Before M003 it held nine, under whatever names their hosts
gave them, and nothing recorded what had been tried.

| Retrieval state | Records | Meaning |
|---|---|---|
| `archived` | 45 | in the archive under the `NN - Name.pdf` / `FR - Surname Year.pdf` convention, title page read |
| `no-text` | 5 | a YouTube video, confirmed to exist, with its real title and uploader |
| `to-replace` | 4 | judged unsuitable by the owner (course material, a learning log); M004 replaces |
| `institution-only` | 1 | `fr-novotney-1998`, a dissertation behind ProQuest or a library |
| `not-attempted` | 51 | purchasable, borrowable or library-only: not free, so not archived |

A test holds every `open-access` and `browser-only` record to one of the
first four states, so "not tried" and "refused" cannot survive into a done
archive. `scripts/fetch-references.mjs --verify` checks the files themselves
against the manifest; CI has no archive, so that check is local and its output
is in the evidence.

**Seven more citations misdescribe their source**, found by reading the
documents the owner saved, and all recorded for M004:

| Entry | The entry says | The source is |
|---|---|---|
| `ref-17` | "Afro House Production Guide" | "Rhythm, Structure and Emotion Inside Afro House Productions", Lian Productions, 2025 |
| `ref-23` | M. Reindl, "Indian Rhythmic Systems in Comparative Perspective", *J. Int. Folk Art and World Music Society* | Tomáš Reindl, "Indian Rhythmic Systems as Sources of Inspiration for Western Composers", *Analytical Approaches to World Musics* 11(2), 2023 |
| `ref-41` | "Rhythmic Ambiguity in Aphex Twin", *Circuit* | Papavassiliou, "Stylistic Features of Intelligent Dance Music: Ambiguity and Rupture Phenomena in Aphex Twin's Rhythms", *Cahiers SQRM* 16(1–2), 2015 |
| `ref-46` | SNS-NOTE-CNTRL-100 | SNS-NOTE-CNTRL-99 (CNTRL-100 is Bjorklund's companion note on evenness) |
| `fr-scherzinger-2010` | *Proceedings of the ICTM*, 2010 | *Clash!*, ed. Hiekel (Schott, 2018), 144–63; the same work as `ref-35` |
| `fr-holzapfel-2015` | a title no work carries | by the owner's reading, Holzapfel & Bozkurt (2012), the work behind `ref-29` |
| `fr-linn-attack-2020` | 2020, no URL | 2 July 2013, Greg Scarth & Roger Linn, three web pages |

## Validation

| Slice | Token | Result |
|---|---|---|
| S01 | `site-unit` | exit 0, 371 tests, 0 fail (task 4) |
| S01 | `format` | exit 0 |
| S01 | `guards` | exit 0, 20 guard invocations |
| S01 | archive (`--verify`, local) | 32 archived records, 0 problems (task 4) |
| S02 | `site-unit` | exit 0, 372 tests, 0 fail (task 3) |
| S02 | `format` | exit 0 |
| S02 | archive (`--verify`, local) | 45 archived records, 0 problems (task 3) |

## Traceability

Every commit on the branch carries `Plan:` and `Slice:` lines.

- `e5192df` M003 planning — the archive: decisions, and plans for S01 and S02 — Slice M003/S01
- `b4f97bf` M003/S01 task 1 — the manifest records retrieval state, and the naming convention is code — Slice M003/S01
- `2077730` M003/S01 task 2 — the retrieval script, refusing to run without a destination — Slice M003/S01
- `afaac80` M003/S01 task 3 — the owner's existing files are matched, renamed and adopted — Slice M003/S01
- `4e32c2a` M003/S01 task 4 — the scripted pass runs, and every refusal is queued — Slice M003/S01, Rows VR09
- `d69e45e` M003/S02 task 1 — every pending row names its filename, and the dead guard is retired — Slice M003/S02
- `451af51` M003/S02 — the owner's first-round worklist answers, recorded verbatim — Slice M003/S02
- `7ad1ed0` M003/S02 — the owner's second-round worklist answers, recorded verbatim — Slice M003/S02
- `eaed47e` M003/S02 task 2 — the owner has worked the worklist — Slice M003/S02
- `a53a7fb` M003/S02 task 3 — the results are recorded, and nothing open is left unaccounted for — Slice M003/S02, Rows VR10

**Commits after the gate:**

- `f0530c7` Ledger: amend M004 from what M003 measured, before M003 ships — Plan
  only; an amendment to M004, at the owner's request, landing with M003.

**Untraced commits:** none.

## What a reviewer should look at twice

- **The archive is not in the diff.** The files behind the 45 archived
  records live in the owner's Dropbox folder: ten records adopted from files
  the owner already held (twelve files renamed in S01 task 3, plus
  `01 - Toussaint.pdf` renamed aside), 22 fetched by the script in S01 task 4,
  and 13 from S02, saved by the owner, printed (Linn) or copied (Holzapfel).
  The evidence files list every rename and every title-page reading.
  `--verify` is the only check that sees the files.
- **Two decisions overrode the plan as written, both the owner's.** The five
  linked-work mismatches stayed archived instead of being deleted (S01
  task 4), and `to-replace` was added as a seventh retrieval status (S02).
  Both are in the decisions file with their reasons.
- **A defect in the script was found by its first real run.** The
  bot-challenge detector refused seven ordinary articles on the word
  "captcha"; it was narrowed, locked by a regression test built from the
  snippets that fired, and the refusals re-run with `--retry`.
- **Two corrections were made in flight and are recorded, not hidden.** The
  planning entry and S01 task 1's evidence said ten records were archived
  before M003; it was nine. The S02 task 3 commit message first said "ten"
  mismatches where it recorded seven, and was amended before anything else
  landed on top of it.
- **`fr-holzapfel-2015`'s identity is the owner's reading,** not something
  the documents establish: the cited title matches no work, and the 2012
  paper fits the one place it is cited. Recorded as a mismatch for M004.
- **The S02 test of the worklist passes on an empty queue only when the
  queue says "Nothing is pending."** — an empty parse and a finished queue
  otherwise look the same.
- **Commit trailers sit in the paragraph above `Co-Authored-By`,** so
  `git interpret-trailers` sees only the co-author line. `git log --grep`
  — the join the workflow uses — finds them. M002 has the same shape.
- **M004 was amended on this branch.** Its rows now land in the theory
  bundle, VR13 covers non-scholarly sources of any tier, and S02 owes every
  manifest mismatch and `to-replace` record. Review the M004 section of the
  ledger alongside this report.
- **Handed onward, not done here:** `to-replace` ×4 with the owner's leads
  (Agawu for `ref-8`; Reina, Nelson and Young for `ref-24`), the
  `ref-25` video suggestion as a policy question for VR12, and two
  duplicate pairs for M005 (`ref-35`/`fr-scherzinger-2010`,
  `ref-29`/`fr-holzapfel-2015`).

## Decisions

## 2026-09-30 — planning M003/S01 and M003/S02

Measured before asking, on `main` at `6b3bb2c`. The manifest holds 106
records: 34 open-access, 20 browser-only, 26 purchasable, 15 borrowable, 11
library-only. Ten name an `archiveFile`. The owner's Dropbox archive holds 16
top-level items, of which four follow `NN - Name.pdf`; the rest were saved
under whatever name the host gave them (`cohnreich1992.pdf`,
`Locke-PrinciplesOffbeatTiming-1982.pdf`, …), and two Academia.edu bulk
downloads sit in subdirectories. Of the open-access entries, about eight serve
a PDF to a script; the rest are HTML pages. `POLY_REFERENCES_ARCHIVE` is not
set in the session's shell. Headless Chrome and poppler (`pdftotext`) are
installed.

- **Q:** About 25 open-access sources are web pages, not PDFs. How should
  the script archive them? — **A:** "PDF. However, also check [the Dropbox
  archive] for existing pdfs I've downloaded from various official sources.
  All are trustworthy if you can match them to the references in the site."
- **Decision:** web pages are printed to PDF with headless Chrome under the
  same naming convention; and before anything is fetched, every file already
  in the archive, subdirectories included, is matched against the
  bibliography by its title page and adopted into the manifest where it
  matches — **Why:** the owner's answer, and the owner's own precedent:
  `04 - Rhythm in Sub-Saharan Africa - Wikipedia.pdf` is a printed web page.
- **Q:** What should Further Reading entries be called, and should
  hand-saved files be renamed? — **A:** `FR - Surname Year`; rename all.
- **Decision:** numbered entries are `NN - Name.pdf` (two-digit number, Name
  the entry's lead author or, for an authorless entry, its title);
  Further Reading entries are `FR - Surname Year.pdf`, derived from the anchor
  (`fr-polak-london-2014` → `FR - Polak London 2014.pdf`). Every archived
  file that does not follow the convention is renamed, and the manifest
  follows — **Why:** the owner's answer; deriving FR names from the anchor
  makes them deterministic, since anchors are never reassigned.
- **Q:** May the script write directly into the Dropbox archive? — **A:** yes.
- **Decision:** runs set `POLY_REFERENCES_ARCHIVE` to the Dropbox folder for
  the command alone; the script never overwrites an existing file — **Why:**
  the archive is the deliverable, and "never overwrite" makes a re-run safe.
- **Q:** (deferred from M002) Generalise or retire M001/S01's `ref-22`
  worklist guard? — **A:** retire it.
- **Decision:** S02 removes the `nios.ac.in`/`ref-22` case from
  `citation-tier.test.mjs`; the manifest-keyed worklist guard M002 added
  covers the class — **Why:** `ref-22` was replaced, so the guard cannot fire,
  and a check that cannot fire is not a check.

### Taken on the owner's behalf

- **Retrieval state is a new manifest field, and the schema becomes version
  2.** `archiveFile: null` could mean "not tried", "tried and refused" or
  "cannot be archived", and VR10 requires the manifest to tell retrieved from
  unretrievable. Each record gains `retrieval: { status, checked, detail }`
  with `status` one of `archived`, `script-refused`, `scan-only`,
  `institution-only`, `no-text`, `not-attempted`. `archived` holds exactly
  when `archiveFile` is set. Nothing outside `site/tests/` reads the manifest,
  so the version bump breaks no consumer.
- **The script refuses to run without `POLY_REFERENCES_ARCHIVE`.** The
  read-side fallback to `.references/` stays for the tests; a writer that fell
  back would be "writing somewhere arbitrary", which the DoD forbids.
- **A refused entry is not retried unless asked.** `script-refused` records
  are skipped on later runs unless `--retry` is passed, and each one is added
  to the browser worklist in the same change, so "refused" always means
  "queued for a person".
- **Nothing is archived on the script's word alone.** A fetched file is kept
  only if it is a real PDF (`%PDF` magic) or a rendered page that is not a
  bot-challenge stub; then every file adopted or fetched in this milestone has
  its first page read before its record says `archived` — the owner's
  standing rule that a file is checked against its own title page.
- **Where an entry appears in both bibliographies with different links, the
  shipping appendix's link wins.** `ref-1` is the case: the appendix links the
  2005 BRIDGES paper the entry names, the theory bundle links a different
  2007 arXiv paper.
- **Owner-adopted files that match nothing in the bibliography are left
  where they are, untouched and unrenamed.** The two Academia.edu bundles are
  bulk downloads of "more papers by" and "similar papers"; renaming only
  applies to files the manifest names.

### Deferred

- **The browser worklist itself is worked by the owner.** M003/S02 task 2
  is a planned pause: the run stops there, the owner works the worklist in a
  browser, and re-issuing `/jk:auto M003` resumes at task 3 to record what was
  found. Not a question, but a boundary only a person can cross.

## 2026-09-30 — judgment calls during M003/S01 task 2

- **The script carries no SPDX header.** The plan said "SPDX header like its
  neighbours"; one of the eighteen `scripts/*.mjs` carries one and
  `check-spdx-headers` covers C/C++ sources only, so matching the neighbours
  means no header. Obviously right: the plan's intent was consistency, and
  that is what consistency is here.
- **Chrome renders with a throwaway profile.** `--user-data-dir` points at a
  temporary directory removed after each page, so a run never touches the
  owner's Chrome profile or collides with a running Chrome. The plan named
  the flags it needed and not this one; without it, a headless run would
  share the default profile with whatever Chrome the owner has open.
- **An entry with a link labelled PDF is fetched from that link, not its
  first.** The plan said "the href of the entry's first markdown link". The
  dry run showed `ref-34` routed to its JSTOR page, which is paywalled,
  while its second link is the free PDF the open-access verdict was reached
  on. Obviously right: the plan's purpose was the free copy, and the label
  says which link that is. Locked by a test, as is the companion fix for a
  URL containing parentheses (`ref-43`'s Wikipedia link was cut short).

## 2026-09-30 — correction, and judgment calls during M003/S01 task 3

- **Correction.** The planning entry above says ten manifest records named an
  `archiveFile`. It was nine (`ref-1`, `ref-2`, `ref-4`, `ref-9`,
  `fr-locke-1982`, `fr-vitale-1990`, `fr-powers-1980`, `fr-brailoiu-1951`,
  `fr-cohn-1992`), counted from `main` during task 3. Task 1's evidence
  repeated the error and is corrected in the same file.
- **`ref-1`'s archived file was not the cited work, so it was un-adopted and
  renamed aside.** `01 - Toussaint.pdf` is "The Distance Geometry of Music"
  (arXiv 2007), which M002 already recorded as the mismatch behind the theory
  bundle's link; the shipping appendix now cites and links the 2005 BRIDGES
  paper. Task 3 adopts only confirmed matches, so `ref-1` goes back to
  `not-attempted` for the scripted pass to fetch the BRIDGES PDF, and the
  arXiv file is kept, renamed `01 - Toussaint 2007 arXiv.pdf` so it no longer
  holds the name the cited work needs. Obviously right: the owner asked for
  every file to be renamed to the convention, and the alternative was either
  calling the wrong paper archived or never fetching the right one.
- **`fr-powers-1980`'s canonical file is the "Rhythm and tāla" section.**
  M002 recorded that section as the one chapter 6 leans on, which is the
  plan's rule for choosing; the article head and the cultural-context section
  are `- part 2` and `- part 3`.

## 2026-10-01 — halt during M003/S01 task 4, and the owner's answer

The scripted pass fetched five files that are what the entry's link serves
but not the work the entry names — `ref-20`, `ref-28`, `ref-29`, `ref-31`,
`ref-42`, each already a `mismatch` in M002. Task 4 step 4 said to delete
such a file and mark the record `script-refused`, which would have queued a
citation fault on the browser worklist as if a host had refused a script.

- **Q:** Keep the five as archived, keep them under a new non-archived
  status, or delete them? — **A:** keep them as archived (option 1).
- **Decision:** the five stay `archived` under the convention's names; each
  record's `retrieval.detail` says the file is the linked work, not the cited
  one, and points at the M002 mismatch note — **Why:** the file is what the
  citation links to, the mismatch is already recorded, and M004 needs the
  linked document in hand to correct the entry. Task 4 step 4's delete rule
  stands for a fetched file that is neither the cited work nor the linked one.

## 2026-10-01 — judgment calls during M003/S01 task 4

- **The challenge detector was narrowed mid-task.** The first pass refused
  seven ordinary pages as bot challenges because the word "captcha" appears
  in their scripts (MediaWiki config, reCAPTCHA comment forms, WooCommerce).
  The detector now needs Anubis's script, Cloudflare's challenge token, a
  challenge title, or "captcha" on a page under 20,000 characters; a test
  built from the six snippets that fired locks it, and `--retry` re-ran the
  refusals. Obviously right: the plan's purpose was to refuse challenge pages,
  and these were articles.
- **The worklist guard's non-vacuity proof is a permanent test, not a scratch
  copy.** The plan said to prove it on a discarded copy; a test that sets one
  record to `script-refused` and asserts the guard reports it does the same
  thing and keeps doing it.

## 2026-10-01 — judgment calls during M003/S02 task 1

- **`ref-8` and `ref-39` got instructions that match their rows.** Both sat
  under "No URL recorded" with "The entry carries no URL" while their URL
  cells carry one. Their instructions now say what a browser session should
  do with the URL each has, and the group is renamed "No URL recorded, or
  none that answers a script". Obviously right: DoD 1 asks every row to say
  what to save, and these two said something false.
- **`ref-30`'s description verdict became `verified`.** Its worklist row
  asked to confirm title and author; task 3 read both on the file's own first
  pages, which is M002's meaning of `verified` (the text was read).

## 2026-10-02 — the worklist, as worked (first round)

The owner's per-row answers, verbatim. Task 2 stays open: three rows asked
for more information, and several answers raise questions for a second
round.

> ref-7 - this is course material, so we need to find another more academic reference
>
> ref-21 - also course material. Needs replacing
>
> ref-26 - Downloaded from https://www.academia.edu/128341480/Measuring_Aksak_Rhythm_and_Synchronization_in_Transylvanian_Village_Music_by_Using_Motion_Capture?sm=a&rhid=43078958440 as the provided URL doesn’t allow access to the actual article. Added to Dropbox references folder. Note this requires an academia account
>
> fr-silverman-2007 - entire Muzikologija downloaded freely from https://muzikologija-musicology.com/index.php/MM/issue/view/No.7 (the URL provided in the doc doesn’t work) and added to Dropbox. Silverman starts on page 69
>
> fr-goldberg-2015 - downloaded from https://www.academia.edu/21699680/Timing_Variations_in_Two_Balkan_Percussion_Performances?sm=a&rhid=43078926096 (the other URL doesn’t allow access to the article) and saved to Dropbox. Note this requires an academia account
>
> ref-23 - added to Dropbox. Exact title is: “Indian Rhythmic Systems as Sources of Inspiration for Western Composers”. Journal is “Analytical Approaches to World Musics, Vol. 11, No. 2. Published December 2023.”
>
> fr-collins-2001 - I need more info in the doc to find this
>
> fr-novotney-1998 - I need more info in the doc to find this
>
> ref-39 - saved to Dropbox. Open accessible
>
> ref-8 - course materials as expected and it’s the unit you expected. Accessible with a scribd subscription. While we’re there, is there any value in (for example) this doc 477028076-The-rhythmic-structure-of-west-african-music-kofi-agawu.pdf (downloaded from https://www.scribd.com/document/477028076/The-rhythmic-structure-of-west-african-music-kofi-agawu) - or let’s find an alternative
>
> ref-18 - I don’t have access through my personal account at OUP, can be purchased for 54.79 (paperback) at https://www.amazon.com/dp/0190226994?tag=oxacglobal-20&linkCode=osi&th=1&psc=1
>
> ref-19 - also not accessible with my personal account. Can be purchased for $59.00 at https://www.amazon.com/dp/0195177894?tag=oxacglobal-20&linkCode=osi&th=1&psc=1
>
> Ref-24. This is someone’s learning log, so I don’t think it’s a suitable reference. It contains this potentially useful info though “here are a few texts in English on the artform. “The Art of Konnakkol” by is a great practical introduction, which would probably go well with some of the many video tutorials available that you can find online [undefined]. Lisa Young has made her Masters [undefined] and PhD [undefined] theses on the topic available online. Rafael Reina has a book on applying Karnatic rhythmical techniques to Western music [undefined], and David Nelson has published a Solkattu Manual [undefined]. There is also software with illuminating documentation, such as the Carnatic Music Typesetter by Arun K. As a British person exploring Karnatic music without having ever travelled to India, I’m mindful that I am missing much of the context of this artform, and feel at the start of a journey in many respects.
> ”
>
> ref-46 - I can’t find a way of getting this paper. The link gives access to citation details, but when you try to download the paper itself, the website is unresponsive. I am not eligible to access it via ResearchGate as I am not part of a scientific organization that has access.
>
> ref-10 - YouTube video exists. Publisher is Rhythm Notes.  Title is: Clave Rhythm - Why It's the Key to Latin Music
>
> ref-11 - Exists. Publisher is World Drum Club. Title is: Clave Explained / Son, Rumba, 3-2, 2-3 and more
>
> ref-14 - Exists. Publisher is Monumental Movement, only has 600ish subscribers, so not very credible at this point. Not sure if it’s the best source for Fela Kuti musical commentary, it’s focused on music in the context of a social movement. Title is: How Fela Kuti Turned Music into a Weapon: Afrobeat Creator and Political Icon
>
> ref-15 - Exists, looks better than ref-14 (100s of thousands of subscribers). Publisher name is Sound Field. Title: The Genius of Fela Kuti and Afrobeat (feat. Femi & Made Kuti)
>
> ref-25 - Exists. Publisher: Henrik Andersen. Not a huge number of subscribers, also demonstrating Indian rhythm but I’d honestly rather have someone ethnically Indian to present this - we should research a bit. It’s a great video though. Title is: Konnakol Mastery: Subdivisions & Polyrhythms in 7 (2+2+3) – Feel the Groove!
> Can we consider replacing with this: “KONNAKKOL BASICS | EP 1 | Introduction To Konnakkol” published by “Konnakkol Somashekar Jois” He has 1.k subscribers, >100K views of this intro video. URL is: https://www.youtube.com/watch?v=ZuZF8BaOt58
>
> ref-17 - Title is: Rhythm, Structure and Emotion Inside Afro House Productions. I've printed to pdf and stored as "17 - Afro House Production Guide.pdf" in Dropbox
>
> fr-scherzinger-2010 - I need more info in the doc to search for this, but I can access his website and there is a lot of material on there.
>
> ref-35 - exists. File downloaded to Dropbox and named as requested
>
> ref-41 - article title: "Stylistic Features of Intelligent Dance Music: Ambiguity and Rupture Phenomena in Aphex Twin’s Rhythms", publication: "Les Cahiers de la Société québécoise de recherche en musique". Saved as requested.

## 2026-10-02 — the worklist, as worked (second round)

The owner's answers to the follow-up information, verbatim. The five
questions put in the same round are still open.

> Ref-39 - I've saved ref-39 properly now. Article title is: Shaping rhythm: timing and
>   sound in five groove-based genres.
>
> fr-collins-2001: Nick Collins, "Algorithmic Composition Methods for Breakbeat Science"
> Full text available from: https://composerprogrammer.com/research/acmethodsforbbsci.pdf
> File downloaded to Dropbox
>
> fr-novotney-1998: I cannot find a downloadable link
>
> fr-scherzinger-2010
> Originally published at: "Clash! Generationen – Kulturen – Identitäten in der Gegenwartsmusik", edited by Jörn Peter Hiekel (Mainz: Schott, 2018), 144–63.
>
> ref-46 Bjorklund: sourced from wayback as you proposed and downloaded to Dropbox.
>
> - ref-18 and ref-19:
> Title: ANALYTICAL STUDIES IN WORLD MUSIC: Analytical Studies in World Music; ISBN-10: 0195177894
> Title: The Oxford Handbook of Algorithmic Music; ISBN-10: 0197554369

## 2026-10-02 — the second-round questions, answered

- **Q:** ref-7, ref-21, ref-8 (course material) and ref-24 (a learning log):
  what should M003 record? — **A:** a new status, `to-replace`.
- **Decision:** `RETRIEVAL_STATUS` gains `to-replace`, meaning the owner
  judged the cited source unsuitable and M004 replaces it; nothing is
  archived for these four. S02 task 3's end-state test admits it — **Why:**
  replacing citations is M004/VR13's work, and an honest record of the
  verdict is what M003 owes.
- **Q:** Five entries misdescribe their source; correct now or record for
  M004? — **A:** record; M004 corrects.
- **Decision:** `ref-17`, `ref-23`, `ref-41`, `ref-46` and
  `fr-scherzinger-2010` become `mismatch` with what the source actually is in
  `note` — **Why:** the same treatment as the five linked-work mismatches.
- **Q:** fr-novotney-1998 has no free copy; record as? — **A:**
  institution-only.
- **Q:** Housekeeping? — **A:** all four: delete the duplicate Scherzinger
  file; keep the whole Muzikologija issue; record Agawu 1987 as M004's lead
  for replacing ref-8, from the journal on JSTOR; hand the ref-25 video
  suggestion to M004 as a question for VR12.

### Handed to M004

- **ref-8 →** Agawu, V. K., "The Rhythmic Structure of West African Music",
  from its journal on JSTOR. The owner's Scribd copy
  (`477028076-The-rhythmic-structure-of-west-african-music-kofi-agawu.pdf`)
  is in the archive, unrenamed; its title page shows the title, the author
  and a first page of 400. The journal, volume and year are to be confirmed
  on JSTOR, not taken from here.
- **ref-24 →** the owner's leads: Rafael Reina on Karnatic rhythmic
  techniques applied to Western music; David Nelson's *Solkattu Manual*;
  Lisa Young's Master's and PhD theses on konnakol.
- **ref-25, a question for VR12 →** the owner would rather cite an Indian
  presenter and suggests "KONNAKKOL BASICS | EP 1 | Introduction To
  Konnakkol" by Konnakkol Somashekar Jois
  (`https://www.youtube.com/watch?v=ZuZF8BaOt58`). VR12 as written removes
  every YouTube citation, so this is a policy question for M004, not a swap.
- **ref-14 →** the owner doubts its credibility (a channel of about 600
  subscribers, focused on social movement more than music); ref-15 is the
  stronger of the two Fela Kuti videos.

## 2026-10-02 — the last two entries, and judgment calls during M003/S02 task 3

The end-state test caught two open-access entries that no step had touched:
both carry no link or DOI, so the script never selected them, and neither was
ever queued. Not anticipated by the plan, so both were put to the owner.

- **Q:** `fr-holzapfel-2015` cites "Metrical Structure in Turkish Makam
  Music", which no published work is called. The owner pointed out that
  `ref-29`'s file, Holzapfel & Bozkurt (2012), "Metrical Strength and
  Contradiction in Turkish Makam Music", is probably the paper meant. Treat it
  as a duplicate? — **A:** yes.
- **Decision:** archived as `FR - Holzapfel 2015.pdf`, a copy of
  `29 - Holzapfel.pdf`; `mismatch`, with the owner's reading in `note` —
  **Why:** the 2012 paper supports the one place the entry is cited
  (theory-balkan's "the related Turkish usul system").
- **Q:** May the Attack Magazine interview behind `fr-linn-attack-2020` be
  printed and archived here? — **A:** yes.
- **Decision:** its three web pages printed with headless Chrome and joined
  with `pdfunite`; `mismatch`, because the page dates it 2 July 2013 and
  credits Greg Scarth & Roger Linn, where the entry says 2020 — **Why:** the
  owner's approval, and the date read on the page itself.

### Judgment calls

- **The duplicate Scherzinger file was renamed, not deleted.** The owner
  allowed deleting `Piano-Phase-in-Global-Perspective-.pdf`, byte-identical
  to `35 - Scherzinger.pdf`. `fr-scherzinger-2010` is the same work and
  needs a file under its own conventional name, so the duplicate became
  `FR - Scherzinger 2010.pdf`. Obviously right: the same end as deletion (no
  stray non-convention file) without making a second copy.
- **`ref-18` and `ref-19` became `purchasable`, with the owner's prices.**
  They were `browser-only`; the owner found them buyable and not open to a
  personal account, which is what `purchasable` means, and VR06 requires the
  price that came with it.
- **The worklist test passes on an empty queue only when the queue says so in
  words.** An empty Pending section after the last pass is the goal; an empty
  parse is also what a changed table shape looks like. "Nothing is pending."
  tells the two apart.
