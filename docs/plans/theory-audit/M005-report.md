# M005 — Literature Enrichment: review report

The bibliography carries the sources the audit identifies as the standard works.

Ledger: `docs/plans/theory-audit/ledger.md`
Branch: `milestone/M005-literature-enrichment`
Generated at the `/jk:auto` review gate. `/jk:ship` is a separate, deliberate
act taken after reading this.

## What made this milestone different

M001–M004 corrected and locked material already in the tree, where every claim
was checkable against a file. **M005 imports external facts**: bibliography
entries for works whose existence, titles, publishers and subject matter cannot
be established from this repo at all.

`research_provenance` checks that a citation's anchor resolves — not that the
work behind it exists, nor that it supports the claim attached to it. That gap
is [jk-standards#85](https://github.com/JimAKennedy/jk-standards/issues/85),
still open, and it is what let reference [2] carry a title appearing in no issue
of *Music Theory Online* into M002.

So every source was verified online **before** any entry was written. Six of the
ten rows did not survive that contact.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | Sub-Saharan sources | F44, F45, F46, F52 | done |
| M005/S02 | Afro-Cuban sources | F47 | done |
| M005/S03 | Indian-classical sources | F48 | done |
| M005/S04 | Balkan sources | F49 | done |
| M005/S05 | Minimalism and electronic sources | F50, F51 | done |
| M005/S06 | Brazilian maracatu | F53 | done |

## What verification changed

| Row | The audit said | Verification found |
|---|---|---|
| F47 | Acosta (2004) | **2003**, Smithsonian Books |
| F48 | "Kippen is in Further Reading only" | Already cited inline — **M002/S04 did it**. Powers' *New Grove* article confirmed; his coverage of Carnatic tala **not** |
| F49 | Peycheva & Dimov for wedding music | Their verifiable work is on **zurna** and Romani musicians. Silverman (2007) is on the subject |
| F51 | Born & Hesmondhalgh, authors | **Editors** |
| F52 | "Arom's later methodological writings" | **No such work.** The 2004 Cambridge edition is a reissue of the 1991 already in the appendix |
| F53 | "the treatment is thin" | Scoped to five named parts and a source that covers them |

Four verified exactly as stated: Charry, Kubik, Agawu (2003), and Born &
Hesmondhalgh's title and publisher.

**Without the research, F52 would have produced a fabricated entry**, and
nothing in this repo could have caught it.

## Definition of done

All twelve boxes across the six slices are checked.

## Validation

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 228 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 207 tests |
| `doc-discipline` | `jk-standards all` | exit 0 |
| `gate` | `bash scripts/pre-push-check.sh` | exit 0, 7 stages, 589 ctest tests |
| `ledger` | `jk-standards ledger` | conforms with every slice `done` |

The site suite grew from 214 to 228 and doc-conformance from 193 to 207.
`site/tests/literature-enrichment.test.mjs` is M005's host, wired into both the
runner and `doc-conformance-wiring.test.mjs`'s `REQUIRED` array — CI never runs
`npm --prefix site test` ([poly#272](https://github.com/JimAKennedy/poly/issues/272)),
so a host outside the runner never runs remotely.

## The source nobody here can read

**Peycheva & Dimov (2002) is listed and cited by nothing.** Chapter 7 has no
Romani-musicianship passage and `theory-balkan.mdx` Rule 7 already credits Rice
for ensemble tightness, so there was no claim their book was confirmed to
support — and writing one to host the citation is what every M005 plan forbids.

The entry states its own limits in the fixed phrase **`contents unverified`**:
existence, authors, year, title and series confirmed from catalogue records; the
contents not, the work being in Bulgarian with no translation obtained.
`grep -rn "contents unverified" site/` enumerates the set — one entry today.

A tier value could not carry that. Tier B is a judgement about a work's
standing, not about whether anyone here can read it.

**This is a deliberate orphan**, and M006/S02's row B08 exists to burn down
entries cited by no page. F49's `Verification` says so, so the burn-down does
not delete a source that was added advisedly.

## Traceability

Thirteen commits, measured against `origin/main`. **Every one carries a
`Slice:` trailer; there is no untraced work.**

- `824c886` docs(plans): verify M005's sources before planning, and amend six rows — M005/S01
- `1b266ac` plan(M005): plan all six slices, and say what the sources were checked against — M005/S01
- `66f97a4` docs(site): add Charry, Kubik and Agawu (2003) to the appendix — M005/S01
- `9762ecd` docs(site): cite the sub-Saharan sources where they do their work — M005/S01, rows F44, F45, F46, F52
- `bba6d8f` docs(site): put Acosta beside Lomax at the Spanish tinge — M005/S02, rows F47
- `7830358` docs(site): cite Powers for the frame Chapter 6 narrows from, and lock Kippen — M005/S03, rows F48
- `cf914e8` docs(plans): correct M005/S03's evidence — site-unit returned 222, not 221 — M005/S03
- `dbf9b50` docs(site): give the Bulgarian wedding-music claim a source — M005/S04
- `9213967` docs(site): list the source nobody here can read, and say so in the entry — M005/S04, rows F49
- `301c676` docs(site): cite Scherzinger in the chapter that makes the claim — M005/S05, rows F50
- `c8888c7` docs(site): frame Chapter 14's cross-pollination argument before making it — M005/S05, rows F51
- `0f16b37` docs(site): name the maracatu ensemble instead of only weighing it — M005/S06, rows F53
- `5942db5` docs(plans): close M005/S06 — the bibliography carries its sources — M005/S06

## What a reviewer should look at twice

- **Every citation rests on a description, not a reading.** All ten works were
  verified bibliographically and by publishers' and reviewers' accounts of their
  subject matter. **None was read.** Annotations therefore say what a work is
  cited *for*, and no task paraphrases an argument nobody here has followed.
  Peycheva & Dimov is the sharpest case, not the only one.
- **Two judgment calls placed citations where the rows did not ask.** F45's
  Kubik went to the chapter opening's West-and-Central-Africa scope claim,
  because Chapter 2 has no passage on the diaspora retentions the row
  describes. F48's Powers went to Chapter 6's scope statement rather than to a
  Carnatic tala claim, because his coverage of Carnatic tala was the one thing
  verification could not confirm. Both are in the decisions file.
- **Two planned halts fired**, both written into the plans for exactly the case
  they caught: S04's when Chapter 7 had no Romani-musicianship passage, and
  S06's scope bound, which stopped the maracatu addition at the parts and roles
  verification established rather than inventing per-part patterns.
- **Two execution errors were corrected in the open.** An evidence file recorded
  `site-unit → 221` where the run returned 222, because the number was written
  in the same command block as the run producing it; it has its own correction
  commit and a recorded lesson. And F51's cases were written into S05 task 1
  where the plan puts them in task 2, which would have made that task commit
  against a red suite.
- **The milestone's `Status` is deliberately still `planned`.** `/jk:close` sets
  it after merge. I set it to `done` during the close and reverted it — moving
  that into `/jk:ship` is the proposal in
  [jk-standards#100](https://github.com/JimAKennedy/jk-standards/issues/100),
  which is not adopted.

## Decisions

Reproduced verbatim from `docs/plans/theory-audit/M005-decisions.md`.


Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

## 2026-09-10 — planning M005, front-loaded

M005 is unlike M001–M004. Those corrected and locked things already in the tree,
where every claim was checkable against a file. M005 **imports external facts**:
bibliography entries for works whose existence, titles, publishers and subject
matter cannot be verified from this repo. `research_provenance` checks that a
citation's anchor resolves — not that the work exists or supports the claim
attached to it. That gap is jk-standards#85, still open, and it is what let
reference [2] carry a fabricated title into M002.

- **Q:** How should the eight new bibliography entries be sourced? — **A:**
  Verify each online before adding it.
- **Decision:** every work is confirmed by web search for author, exact title,
  year, publisher and enough subject matter to justify the claim it is cited
  for; anything unconfirmable becomes a row rather than an entry — **Why:** it
  is the only option meeting the standard M002 set and B11 enforces.

### Verification results

| Work | Verdict |
|---|---|
| Charry, E. (2000). *Mande Music: Traditional and Modern Music of the Maninka and Mandinka of Western Africa*. University of Chicago Press | as stated; jembe drumming is one of its four spheres |
| Kubik, G. (1999). *Africa and the Blues*. University Press of Mississippi | as stated |
| Agawu, K. (2003). *Representing African Music: Postcolonial Notes, Queries, Positions*. Routledge | as stated |
| Acosta, L. (2003). *Cubano Be, Cubano Bop: One Hundred Years of Jazz in Cuba*. Smithsonian Books | **year wrong in ledger** — 2003, not 2004 |
| Powers, H. (1980). "India, subcontinent of". *The New Grove Dictionary of Music and Musicians*, vol. 9 | article confirmed; the *Carnatic tala* specific claim was not |
| Born, G. & Hesmondhalgh, D., eds. (2000). *Western Music and Its Others: Difference, Representation, and Appropriation in Music*. University of California Press | as stated, but they are **editors** |
| Silverman, C. (2007). "Bulgarian Wedding Music between Folk and Chalga: Politics, Markets and Current Directions". *Muzikologija* | on-topic and openly accessible |
| Peycheva, L. & Dimov, V. (2002). *The Zurna Tradition in Southwest Bulgaria: Romani Musicians in Practice*. Bŭlgarsko muzikoznanie | verifiable, but on **zurna**, not wedding music |
| Crook, L. (2009). *Focus: Music of Northeast Brazil*, 2nd ed. Routledge | covers Candomblé, Afoxé and **Maracatu** |
| Arom's "later methodological writings" | **not found.** The 2004 Cambridge edition is a paperback reissue of the 1991 already in the appendix; the 1985 French original predates it |

- **Q:** F49 names Peycheva & Dimov for Bulgarian wedding music, but their
  verifiable work is on the zurna tradition. Silverman (2007) is directly on
  svatbarska muzika. How should S04 proceed? — **A:** Cite both.
- **Decision:** Silverman (2007) carries the wedding-music claim and Peycheva &
  Dimov (2002) is cited where Chapter 7 touches Romani musicianship — **Why:**
  the row's named scholars stay in the bibliography where their work actually
  supports something, rather than being attached to a subject they do not cover.
- **Q:** F52 asks for Arom's later methodological writings, which do not appear
  to exist as a distinct work. What should happen to the row? — **A:** Cite the
  1991 more fully instead.
- **Decision:** F52 becomes an inline citation to the existing `fr-arom-1991`
  from `theory-sub-saharan-africa.mdx`'s methodology material, and its `Item`
  records why no later work was added — **Why:** the row's intent is that the
  methodology section leans on Arom; the specific bibliographic claim behind it
  is unsupported and saying so is better than inventing an entry to satisfy it.
- **Q:** Two rows are underspecified — how should they be handled? — **A:** Pin
  both down before planning.
- **Decision:** F52 resolved as above; F53's scope is fixed to naming the
  maracatu ensemble's parts — caixa, alfaia, mineiro, agbê and gonguê — and
  citing Crook (2009), which covers maracatu directly — **Why:** "the section is
  thin" is not a scope; the parts and the source are.

### Findings recorded rather than acted on

- **F47's year is wrong.** The ledger says Acosta (2004); the book is 2003.
- **F48's premise is stale.** It says "Kippen (1988) is in Further Reading
  only", but `06-indian-classical.mdx` already cites it inline — M002/S04 did
  that. What remains of F48 is Powers alone.
- **F48's Powers claim is partly unconfirmed.** The *New Grove* "India" article
  exists and Powers wrote it, but that it covers the Carnatic tala system
  specifically was not established, so the citation is attached to Indian
  art-music theory generally rather than to a Carnatic tala claim.
- **F51's authors are editors.** Born and Hesmondhalgh edited the volume; the
  entry says so rather than presenting them as authors.
- **Q:** How should the tiers be assigned, given that Peycheva & Dimov (2002) is
  in Bulgarian and unreadable here? — **A:** Tier A throughout, Peycheva & Dimov
  at B, and state the verification limit as a fact rather than letting the tier
  imply it.
- **Decision:** every new entry is tier A except Peycheva & Dimov at B, and that
  entry's annotation carries the fixed phrase **`contents unverified`** naming
  what was confirmed — existence, authors, year, title, series — what was not,
  and that a translation is being sought — **Why:** a downgraded tier is a
  judgement about a work's quality, not about our ability to read it, and using
  it to stand in for a caveat nobody wrote would put the uncertainty somewhere a
  reader cannot see. The phrase is fixed so `grep -rn "contents unverified"
  site/` enumerates the set, the way `patch-divergence-ok` and
  `citation-tier-ok` do.
- **Standing qualification on all ten works:** each was verified
  bibliographically and by publishers' and reviewers' descriptions of its
  subject matter. **None was read.** Entry annotations therefore say what a work
  is cited *for*, and the plans forbid paraphrasing arguments nobody here has
  followed. Peycheva & Dimov is the sharpest case, not the only one.

## 2026-09-10 — executing M005

- **Judgment call, M005/S01 task 2:** F45 says Kubik covers "the African
  rhythmic retentions that bridge Ch 2 and Ch 4", but Chapter 2 has no such
  passage. Kubik is cited instead at the opening's claim that the subject is
  "the polymetric drumming of West and Central Africa", which his survey of the
  Western and Central Sudanic belt supports directly — **Why:** the alternative
  was to write a diaspora passage to host the citation, which every M005 plan
  forbids. The diaspora thread is mentioned as what Chapter 4 picks up rather
  than asserted as a claim of its own.
- **Finding:** F52's premise was half stale. `theory-sub-saharan-africa.mdx`
  already listed Arom under `## Sources` as "Primary: [Arom 1991] (referent
  theory)" — a listing, not a citation at a claim, which is the distinction M002
  built the tier check around. The remaining work was real: the dance-beat
  sentence now names him where the methodology is asserted.
- **Mistake, recorded:** M005/S03's evidence file was written with
  `site-unit → 221 tests` in the same command block as the run that produced the
  number, so it recorded a guess rather than a result; the run returned 222.
  Corrected in a follow-up commit — **Lesson:** write evidence counts after
  reading the output, never in the same block as the command producing it. An
  evidence file whose numbers were predicted rather than read is not evidence.
- **Planned halt fired, M005/S04 task 2.** Chapter 7 has no Romani-musicianship
  passage, and `theory-balkan.mdx` has none either — its Rule 7 covers
  wedding-band tightness and already attributes that to Rice. **Q:** how should
  it resolve? — **A:** add the entry and cite it from the theory page if a claim
  fits; if none does, add it and record that nothing needs it.
- **Decision:** the entry is added, tier B, carrying `contents unverified`, and
  cites nothing — **Why:** attaching it to Rule 7 would assert that a book
  nobody here has read supports a claim already attributed to someone else, and
  writing a passage to host it is what the plan forbids.
- **Interaction flagged:** this creates a deliberately orphaned entry, and
  M006/S02's B08 exists to burn down entries cited by no page. F49's
  `Verification` says so explicitly, so the burn-down does not delete a source
  that was added advisedly.
