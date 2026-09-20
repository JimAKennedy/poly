---
class: gated
---

# Verifiable references — vision

**Input document for `/jk:assess`.** Not a plan: the outcome, the current state
measured rather than assumed, and the decisions a plan will have to take.

## The principle

A reference is good when **the text can be obtained and read, and the citation
describes the work it points at**. Not when its venue sounds academic, and not
when its URL returns 200.

The second half was added after this document's first draft, because obtainability
alone would not have caught the defect that prompted it. See *The accuracy
problem* below.

That reframes what the guide has been optimising. The existing scheme sorts
sources into Tier A/B/C by the kind of thing they are, and enforces that a
named-theory claim cites Tier A. It is a reasonable rule and it was
implemented properly, but it answers a different question from the one a reader
has. A reader — or a maintainer checking whether a claim is true — needs to know
*how do I read this*, and the guide currently has no systematic answer.

Under the obtainability principle a YouTube video fails, and not because it is
informal: a video may be excellent, but there is no text to review, no page to
cite, and no way to check that it says what the guide claims it says. A link to
a *review* of a book fails for the same reason — the review is not the work.

## What is actually there

**The appendix holds two bibliographies, and they have had opposite treatment.**

| | Numbered `[1]`–`[46]` | Further Reading |
|---|---|---|
| Entries | **43** | **64** |
| Tiers | 16 A · 11 B · **16 C** | 60 A · 4 B |
| Carry a URL | 43 of 43 | **0 of 64** |
| Cited by some page | all | all |

The numbered list is the original, per-chapter bibliography: every entry is
clickable, and a third of it is Tier C — five YouTube videos, three Wikipedia
articles, a Scribd upload, and several blog and course-marketing pages.

Further Reading is what theory-audit **M005 Literature Enrichment** added: Arom,
Locke, Chernoff, Anku, Polak, Agawu, Charry, Kubik, Toussaint 2013, London
2012, Pressing 2002 — real scholarship, almost all of it Tier A. **None of it
carries a URL or any other indication of where to get it.**

So the enrichment milestone did what it said: it added the sources the audit
identified. It never promised to clean the numbered list, and it did not. That
is why the appendix still reads as "full of YouTube" — **the unreformed list is
the one at the top of the page**, and the good work is below it in a section a
reader may never reach.

## Three defects the current checks cannot see

**Three links are dead.** Measured across all 43 URLs:

| Ref | Tier | Status | Source |
|---|---|---|---|
| **`ref-9`** | **A** | **404** | Oluranti, *Polyrhythmic Structures in West African Drumming* |
| `ref-26` | B | 404 | `fiveable.me` Balkan study guide |
| `ref-22` | B | no response | `nios.ac.in` Hindustani theory PDF |

`ref-9` is the one that matters: **Tier A is the tier the guide requires for
named-theory claims**, and its URL goes nowhere. Every gate stayed green,
because no check fetches anything. `research_provenance` verifies that a
citation resolves to a bibliography *anchor*; `citation-tier` verifies the
declared tier. Neither asks whether the source exists.

**One reference cites the wrong artefact.** `ref-6` is Jones (1959), *Studies in
African Music* — and the URL is a **Cambridge review of the book**, not the
book. Tier A, cited on African rhythm claims, and what a reader reaches is
somebody else's two-page opinion of a work they still cannot read.

**Five more look dead and are not.** `academic.oup.com` (×2), `academia.edu`,
`alpaca.pubpub.org`, `noisemachines.studio` return 403 to a script and load
fine in a browser; `ethanhein.com` returns 406. Any liveness checking this
programme adds has to tolerate that, or it will cry wolf until it is ignored.

## The accuracy problem

**Of the three numbered references anyone has examined against their actual
source, all three were wrong about what they cite.**

| Ref | Found by | Defect |
|---|---|---|
| `ref-2` | theory-audit M002 (F17) | Real author, real journal, real URL — **fabricated title**. The article at that URL is Goldberg's Bulgarian-meter paper. |
| `ref-6` | this audit | Cites Jones (1959) *Studies in African Music*; the URL is a Cambridge **review of the book**, not the book. |
| `ref-9` | this audit | **Fabricated title and wrong subject.** Printed as "Polyrhythmic Structures in West African Drumming"; the dissertation is Oluranti (2012), *Polyrhythm as an Integral Feature of African Pianism* — piano works by Euba, Ligeti and Uzoigwe. |

Three of three is not a rate anyone should extrapolate from — it is three. But it
is three out of three, and the sample was not adversarially chosen: `ref-9` was
examined because its link was dead, and the title turned out to be invented as
well.

**Obtainability would have cleared `ref-9`.** The dissertation is freely
available, archived, and readable. It simply is not the work the guide said it
was — a different title, and a subject one discipline away from the claim it sat
under. A programme that only asks *can I get this?* files that entry as
compliant.

So the audit has to ask a second question of every entry: **does the citation
describe the thing it points at** — title, author, year, and enough of the
subject to tell that it supports the chapter citing it.

That is a materially larger job than classifying availability, and this document
originally under-sized it. Checking a title against a landing page is a minute;
confirming a source is *about* what a claim needs may mean opening it. The plan
should size VR1 on that basis rather than on the fetching.

## Vision

Every source the guide cites can be read. For each one the project knows how:
downloaded and archived, purchasable, or held in a library — and where it cannot
be read at all, it is not cited. The claims in the guide point at sources a
reader can actually check, and a maintainer revisiting a claim in a year can put
their hands on the text it rests on.

## What success looks like

- Every reference carries an **obtainability status** and a **verified
  description** — title, author and year confirmed against the source itself,
  not against the guide's own entry.
- No citation points at a commentary on the work it names, as `ref-6` does.
- Everything freely downloadable **is** downloaded, into one archive with a
  stable naming convention — by script where that works, and from a **worklist
  of browser links** where it does not. A source that resists automation is not
  an unobtainable source; it is a manual retrieval, and one afternoon of them is
  a reasonable price for a bibliography that stays checkable.
- Nothing is cited that has no reviewable text. The YouTube entries are
  replaced by a source that says the same thing in prose, or the claim they
  support is rewritten to need no citation.
- `ref-6` cites the book or is replaced; no entry points at a review of itself.
- A dead link is **found by a check rather than by a reader**.
- The two bibliographies are reconciled — one list, one standard.

## The archive

**Outside the repository.** The obvious convenience is a gitignored directory in
the project root, and it is the wrong choice: these are copyrighted works, a
gitignore is one `git add -f` away from redistributing them, and the repository
is public. A Dropbox research folder outside the working tree —
`~/Library/CloudStorage/Dropbox/Research/drum generator/References` on the
maintainer's machine — is structurally safer: it cannot be committed by
accident.

**The archive path itself is configuration, not a constant.** It differs per
machine and it is a personal path, which this repository's own
`check-personal-paths` guard rejects in tracked files — it caught the absolute
form in an earlier draft of this document. The manifest should therefore record
filenames relative to an archive root supplied by the environment, so the
checked-in record stays portable and says nothing about anyone's home
directory.

**The repository holds a manifest, not the files.** A checked-in record of, per
reference: obtainability status, the archive filename if we hold it, and an
ISBN/DOI where one exists. That is what lets a check verify the archive and the
bibliography agree, and what lets the site say "we hold this" without shipping
it.

**The naming convention already exists** and should be kept — the four files
collected so far are `01 - Toussaint.pdf`, `02 - Goldberg.pdf`,
`04 - Rhythm in Sub-Saharan Africa - Wikipedia.pdf`,
`09 - 11866_DISSERTATION_-_FULL_Oluranti.pdf`. Number first, sorting in
citation order. Worth normalising the tail so the number is the only thing a
tool has to parse.

**Further Reading needs numbers too.** Its 64 entries have no citation number,
so the convention has no prefix to use. That is a small decision with a large
effect on whether the archive can be mechanically checked.

## Proposed milestones

### VR1 — Every reference is obtainable and accurately described

The audit, and the foundation for everything else. Two verdicts per entry, not
one.

**Obtainability** — free download, purchasable, library-only, or unobtainable,
recorded with the evidence and, where it is purchasable, **the price**.
"Purchasable" without a number is not a decision anyone can make.

**Accuracy** — title, author and year confirmed **against the source**, plus
enough of its subject to tell it can support the chapter citing it. Confirmed
against the source, emphatically not against the guide's own entry: the entry is
the thing under test, and `ref-9` would have passed any check that trusted it.

The obtainability method is validated rather than assumed. Two spot checks, one
per hard class: Anku (2000) is free at *Music Theory Online*; Locke (1998), a
White Cliffs Media book long out of print, is purchasable used from about $20
under ISBN 978-0941677905. Both determinable in a minute.

**The accuracy half is the expensive one, and it does not parallelise onto a
script.** A title and year can usually be confirmed from a landing page. Subject
fitness sometimes cannot — establishing that Oluranti (2012) is about pianism
rather than drumming took opening the PDF and reading its title page. Expect a
long tail where the only way to answer is to look at the document.

**Where a description cannot be confirmed, say so** rather than passing the
entry. "Unverified" is a real verdict and a cheaper one than a wrong "verified";
it also tells VR3 exactly which entries need a human.

### VR2 — Everything downloadable is downloaded

Archive every source in the "free download" class, to the naming convention,
with the manifest updated to match.

**Three retrieval paths, and the plan should not pretend there is one.**

- **Scripted.** Plain PDFs over plain HTTP. Fetch them, name them, record them.
- **Manual.** Sites that serve a browser and refuse a script — the five 403s and
  the 406 measured above are already known to be in this class, and a login or
  a JavaScript-rendered download button puts a source here too. The deliverable
  is a **worklist**: URL, what to save, and the exact filename to save it as, so
  the pass is mechanical rather than a research session. Jim has confirmed a
  one-off manual pass is acceptable, which means this class is *retrievable*
  rather than *blocked*, and the plan should size it rather than avoid it.
- **Neither.** Free but only as unsearchable scanned images, or free only to
  members of an institution. These are recorded as what they are, because
  "we hold a PDF you cannot search" changes how quickly a maintainer can check
  a claim.

**The worklist is the milestone's real output**, more than the files it manages
to fetch on its own. A list of thirty links that takes an afternoon is a
finished archive; a script that gets twenty and silently skips ten is not.

### VR3 — Nothing unreviewable is cited

The editorial milestone, and the one that needs Jim's judgement rather than a
script.

Each of the 16 Tier-C entries is either replaced by a source that makes the
same point in reviewable text, kept because it is the primary artefact rather
than a commentary on one, or dropped along with the claim it supported. The
three dead links are fixed or replaced. `ref-6` cites Jones or goes.

**The Wikipedia question is a policy decision, not a cleanup.** A Wikipedia
article is reviewable text and is often a fair summary; it is also not a source
a scholarly guide should rest a claim on. The three entries are cited from
somewhere, so removing them is not free — and deciding "never" is as legitimate
as deciding "as orientation, never for a claim". The programme should decide
once rather than per entry.

### VR4 — The two bibliographies become one

Numbered and Further Reading merged into a single list under a single standard,
with Further Reading's 64 entries reachable and citable the way the numbered
ones are.

**This is the milestone that changes what a reader sees**, and it is deliberately
last: merging before VR3 would mix good sources into a list still carrying
Tier-C entries, and merging before VR1 would mean renumbering twice.

### VR5 — A dead reference is found by a check

A scheduled liveness check over every URL in the bibliography, tolerant of the
403/406 bot-blocking measured above, reporting rather than blocking.

**Advisory, not a gate.** A hard gate on third-party availability makes every
unrelated pull request hostage to somebody else's web server, and a flaky gate
gets ignored or disabled — which is worse than not having one. A scheduled job
that files an issue is the shape that survives.

## Sequencing

VR1 blocks everything: you cannot archive, replace or merge what you have not
classified. VR2 and VR3 both depend on it and are independent of each other —
one is mechanical fetching, the other is editorial judgement, and they can run
in parallel. VR4 depends on VR3, because merging a list you are still editing
means doing it twice. VR5 is independent of all of them and could land first;
it is placed last only because it protects an end state that does not exist yet.

**The bottleneck is judgement, not scripting**, and VR1 now carries more of it
than VR3. VR2 is mechanical, and where it is not — the browser worklist — it is
at least *unattended by the plan and attended by a person*, which is a known
cost rather than a risk. VR1's accuracy half is 107 judgements of the form
*is this citation about what it claims to be about*, with a tail that can only
be settled by opening the source. VR3 is 16 editorial decisions plus a policy
call on Wikipedia. None of that gets faster with better tooling.

## Out of scope, recorded so a later pass does not rediscover them

- **Re-reviewing whether each claim is true.** That was the theory-audit
  programme. The boundary is finer than it was: this programme checks that a
  citation names the work it links to and that the work is in the right subject
  area for the claim. It does not re-check that the work *supports* the claim,
  which is the theory audit's question. `ref-9` fails this programme's test —
  wrong title, wrong discipline — without anyone having to re-read the argument
  it was cited for.
- **Redistributing the archive.** The PDFs stay local; the repository holds a
  manifest. Hosting them would be a licensing question, not a tooling one.
- **Citations outside the guide.** `docs/` and code comments cite things too;
  this programme is the site's bibliography.
- **The tier scheme itself.** Obtainability is added alongside it, not instead
  of it, unless VR4 finds the two genuinely conflict.
