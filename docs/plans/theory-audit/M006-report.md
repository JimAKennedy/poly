# M006 — Bibliography Hygiene: review report

Every reference in the guide is cited by something, cited at a tier that carries
the claim, and listed once.

Ledger: `docs/plans/theory-audit/ledger.md`
Branch: `milestone/M006-bibliography-hygiene`
Generated at the `/jk:auto` review gate. `/jk:ship` is a separate, deliberate
act taken after reading this.

## The headline numbers

| | Before | After |
|---|---|---|
| `citation-tier-ok` suppressions | 10 | **0** |
| Appendix entries cited by nothing | 24 | **1**, which declares its contents unread |
| Entries listing the same work twice | 2 pairs | **0** |

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S01 | Upgrade the suppressed claim citations | B01–B07, B17 | done |
| M006/S02 | Resolve the orphaned references | B08 | done |
| M006/S03 | De-duplicate the appendix | B09 | done |
| M006/S04 | Unattested terms | B11 | done (`accepted`) |

## What the rows said, and what was true

Three of the four rows turned out to describe something other than what the tree
contained. In each case measuring is what showed it.

- **B08 said twenty-one entries were "cited by no page", and called an uncited
  entry "either dead weight or a source nobody checked".** Seventeen were
  neither. They sat inside range listings — `See also refs [21]–[25]` — which
  hyperlink only their endpoints, so the interior entries were pointed at in
  prose and unreachable by anchor. Ten such ranges existed, one per theory page,
  covering refs 4–43 almost continuously. **Applied literally, the planned
  policy would have deleted or re-homed seventeen references the theory pages
  already name in their own Sources lines.**
- **B09 named one duplicate; the sweep found two.** `ref-5` and `fr-arom-1991`
  are both Arom's *African Polyphony and Polyrhythm* (1991), which the row never
  mentioned.
- **B11 asked whether *kotekan polos* has an attestation.** The term does not
  denote what the audit says it denotes: *polos* is one of the two interlocking
  parts, and the *pokok* is the melody kotekan embellishes. M003/S05 refused the
  label because it collided with Rule 3's usage, and that refusal was right on
  evidence that did not exist when it was made.

**B17 was added during planning** for two suppressions citing `ref-42` whose own
markers said outright that no row owned them: *"M006/S01 carries B01–B07 for
seven sources and this is the eighth."*

## Definition of done

All nine boxes across the four slices are checked.

## Validation

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 234 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 213 tests |
| `ledger` | `jk-standards ledger` | conforms with every slice `done` |

The site suite grew from 228 to 234.

## Traceability

Eight commits, measured against `origin/main`. **Every one carries a `Slice:`
trailer; there is no untraced work.**

- `5a6db04` docs(plans): research M006's three open questions before planning it — M006/S01
- `16ce204` plan(M006): plan all four slices from what the tree and the sources say — M006/S01
- `7904482` docs(site): move six claims onto the sources their markers named — M006/S01, rows B01, B02, B03, B04, B05, B17
- `d3759ee` docs(site): cite Linn himself instead of a blog quoting him — M006/S01, rows B06
- `1e5e57f` docs(site): ground the Amen break in scholarship, and clear the last suppression — M006/S01, rows B07
- `4ed6dbe` docs(site): expand the reference ranges, and retire what is genuinely dead — M006/S02, rows B08
- `a90920f` docs(site): find the second duplicate, and the claim that retiring it broke — M006/S03, rows B09
- `e640a27` docs(plans): close B11 — the term does not mean what the audit says — M006/S04, rows B11

## What a reviewer should look at twice

- **Two mistakes of mine, both caught by checks this programme built.** Marking
  M006/S02 `done` before writing its evidence file — `jk-standards ledger`
  refused it, "a completion claim needs the record of what was run". And
  retiring `ref-5` after a `grep` showed one file citing it, which I read as one
  citation: there were two, and the second was a live superscript claim.
  `citation-tier.test.mjs`, built by M002, failed with
  `theory-sub-saharan-africa.mdx:13 cites ref-5 (tier undeclared)`.
- **A check reported the right answer for the wrong reason first.** The
  duplicate detector's first version used a span-spanning regex that read across
  neighbouring entries, and flagged `ref-5`/`fr-arom-1991` as a collision before
  the per-line version flagged it as a real one. A check that is right by
  accident is not evidence.
- **The upgrade pattern could have been an evasion.** The guide's grammar makes
  a `<sup>[N]</sup>` a claim and a plain link a listing, so deleting a
  superscript takes a claim *out* of the tier check's scope rather than sourcing
  it, and looks identical to a fix in a diff. M002 anticipated this in its own
  `S02-F18`; S01's case asserts both halves.
- **Two new sources are cited at tiers chosen deliberately.** The Linn interview
  is **tier B** — a professional publication interviewing the primary actor is
  not scholarship, and calling it A would weaken what the tier vocabulary means
  everywhere else. Harrison's Amen-break chapter is tier A, and the sentence's
  "widely described as" hedge was kept: he grounds the history, not a count.
- **One entry remains uncited on purpose.** `fr-peycheva-dimov-2002`, added by
  M005, declares `contents unverified` and passes on the stated exception the
  check encodes.

## Decisions

Reproduced verbatim from `docs/plans/theory-audit/M006-decisions.md`.


Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

## 2026-09-11 — planning M006, front-loaded

Six of the eight suppressions S01 burns down name a replacement already in this
repo, so most of the milestone is re-pointing citations at sources that are
present and already at tier A. Two needed sources from outside it, and B11 is a
question about a term rather than a citation. All three were researched before
any plan was written, on the M005 standard.

- **Q:** Two suppressions cite `ref-42` and no B row owns them — the markers say
  so in band. How should the gap close? — **A:** Add B17 to S01 and amend its
  definition of done.
- **Decision:** B17 covers both `10-brazilian.mdx` and
  `appendix-euclidean-reference.mdx`, and S01's DoD changes from "the seven" to
  eight — **Why:** the in-band markers already state that an eighth row is what
  is missing, and both replacements (`fr-sandroni-2001`, `fr-fryer-2000`,
  `ref-1`) are already in the appendix, so the work is the same shape as
  B01–B05.
- **Q:** B08 requires every entry to be cited, "with retired entries deleted
  rather than exempted", but M005 deliberately added an uncited one. How should
  that be reconciled? — **A:** Allow a stated exception, keep it narrow.
- **Decision:** the check requires every entry either to be cited **or** to
  carry the `contents unverified` phrase; the other orphans must still be cited
  or deleted — **Why:** the phrase is already in-band, greppable and reasoned,
  so the exception is a written rule rather than a hole, and it does not reverse
  a decision taken one milestone ago for stated reasons.
- **Q:** B06 wants the Attack Magazine interview; B07 names no replacement. How
  should they be handled? — **A:** Verify online, as M005 did.

### Verification results

| Row | Verdict |
|---|---|
| **B06** | The interview exists: "Roger Linn On Swing, Groove & The Magic Of The MPC's Timing", *Attack Magazine*, July 2020. It carries Linn describing the LM-1 mechanism directly — quantise to the nearest step, then delay every other step — which is exactly what the chapter's claim needs and what Brett's blog was standing in for. **Tier B**: a professional publication interviewing the primary actor, not scholarship and not a hobbyist blog. |
| **B07** | A genuine Tier-A upgrade exists, contrary to the suppression's expectation that only reception could be witnessed: Harrison, N. "Reflections on the Amen Break: A Continued History, an Unsettled Ethics", in *The Routledge Companion to Remix Studies*, 2nd ed., eds. Navas, Gallagher & burrough, Routledge, 2025, ch. 49. Harrison made the 2004 work that first documented the break's history. |
| **B11** | **The audit is wrong, and M003/S05 was right to refuse the term.** Sources agree that *polos* is one of the two interlocking kotekan parts — the on-beat one, paired with *sangsih* — not "a third player playing only the structural pokok tones". The pokok is the main melody on calung and ugal that kotekan embellishes; it is not what polos plays. This is exactly what the guide's own Rule 3 already says. |

- **Decision:** B11 closes as `accepted`, not by adding a citation — **Why:** the
  term the audit asked for does not denote what the audit says it denotes, so
  there is nothing to attest. `S05-F33`'s forbidden arm, which stops the label
  reappearing in Rule 5, is vindicated rather than superseded.
- **Decision:** B06 cites the interview at tier B rather than leaving the
  suppression in place — **Why:** the row's own text names that interview as
  "the primary source this should cite", and a primary source in a professional
  publication is a real upgrade over commentary quoting it second-hand.
- **Q:** S02 must resolve 20 orphaned entries, ranging from tier-A scholarship
  to YouTube links. What policy? — **A:** Delete tier C, find homes for tier A/B.
- **Decision:** the seven tier-C orphans are deleted, since M002's own argument
  is that such sources were never fit to carry a claim and an uncited one is
  pure dead weight; each tier-A and tier-B orphan is cited where it genuinely
  supports an existing claim, and retired with a reason where it has no honest
  home — **Why:** it keeps scholarship the guide may want (`ref-2` is the
  Goldberg article M002 found behind the fabricated title, and it is about
  Bulgarian meter) without writing passages to justify references, which is what
  M005/S04 halted rather than do.
- **Measured, not assumed:** 109 appendix entries, 21 orphaned — 18 numbered and
  3 in Further Reading. B08 says "eighteen numbered references and two Further
  Reading entries", which was right when written; M005 made it three by adding
  `fr-peycheva-dimov-2002` on purpose, and the stated exception covers it.
- **Interaction:** `fr-toussaint-2005` is both an orphan (S02) and B09's
  duplicate (S03). De-duplicating removes it, so S03 reduces S02's list by one.
  Whichever runs second must re-measure rather than trust a count taken earlier.
