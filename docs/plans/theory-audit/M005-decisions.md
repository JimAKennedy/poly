# M005 — decisions

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
