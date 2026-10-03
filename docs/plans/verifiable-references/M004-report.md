# M004 — Nothing unreviewable is cited

**Review-gate report.** Generated from the ledger, `git log` and
`M004-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** No claim in the guide rests on a source with no text to review.

**Branch:** `milestone/M004-reviewable` · **Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S01 | The Wikipedia policy is decided and applied | VR11 | done |
| M004/S02 | The remaining unreviewable entries go | VR12, VR13 | done |

## Definition of done

**S01**

- [x] The policy is written down: whether Wikipedia may be cited at all, and if
      so for what — with its reason, once, rather than per entry
- [x] The three Wikipedia entries conform to it
- [x] Any claim that loses its citation is rewritten to need none, not left
      uncited

**S02**

- [x] No YouTube entry remains in the bibliography
- [x] Every remaining entry that is not scholarship — whatever its tier — is
      either reviewable text from a source fit to cite, or the primary
      artefact rather than a commentary on one, with that reason recorded
- [x] Every entry the manifest marks as a description mismatch is corrected
      or replaced — M002's and M003's alike
- [x] Every entry the manifest marks `to-replace` is replaced, or dropped with
      its claim rewritten

## The headline

**The guide now says what it cites, and every entry left meets it.** One
paragraph in the shipping appendix's introduction states the policy once: no
Wikipedia, no video, no course material, commercial or marketing page, or
blog; a practitioner's own text at Tier B with its reason recorded. A
tree-wide test forbids fifteen hosts on any page, shipping or deferred.

| | Before (`main`) | After |
|---|---|---|
| Manifest records | 106 | 86 |
| Entries retired | — | 21 (3 Wikipedia, 5 YouTube, 12 non-scholarly, 1 duplicate) |
| Entries added | — | 1 (`fr-agawu-1987`, replacing `ref-8`) |
| `mismatch` verdicts | 15 | 0 |
| `to-replace` records | 4 | 0 |
| Theory-bundle entries | 93 | 73 |
| Archived records | 45 | 34 |
| Site tests | 372 | 395 |

**No claim lost its source.** Nineteen of the retired entries were only
links in a deep dive's closing "See also" line, after its list of
scholarship. The two that carried claims were repointed first: `ref-36`'s swing-lineage
attribution now cites Linn's own interview, its micro-offset sentence keeps
Danielsen and `ref-39`, and the "Holzapfel 2015" citation now names the
2012 paper it meant.

**Thirteen citations now name the work they link.** Among them, in the
shipping guide: Bjorklund's tech note is SNS-NOTE-CNTRL-99 (chapters 1, 7),
Scherzinger's chapter is 2018 in *Clash!* (chapter 8), and Linn's interview
is 2013 with its URL (chapter 9). In the theory bundle, two phantom Holzapfel
titles became Srinivasamurthy, Holzapfel & Serra (2014) and Holzapfel &
Bozkurt (2012), and `ref-42` credits Jovino Santos Neto rather than the
lecturer whose course directory hosts it.

## Validation

| Slice | Token | Result |
|---|---|---|
| S01 | `site-unit` | exit 0, 374 tests (task 2) |
| S01 | `format` | exit 0 |
| S01 | `doc-conformance` | exit 0 |
| S02 | `site-unit` | exit 0, 395 tests (task 4) |
| S02 | `format` | exit 0 |
| S02 | `doc-conformance` | exit 0 |
| S02 | archive (`--verify`, local) | 34 archived records, 0 problems |

## Traceability

Every commit on the branch carries `Plan:` and `Slice:` lines.

- `0f395c0` M004 planning — nothing unreviewable is cited: decisions, and plans for S01 and S02 — Slice M004/S01
- `a656a55` M004/S01 task 1 — the policy is written once — Slice M004/S01
- `25df07e` M004/S01 task 2 — the three Wikipedia entries are retired — Slice M004/S01, Rows VR11
- `dc77bc9` M004/S02 task 1 — no video is cited — Slice M004/S02, Rows VR12
- `5496ca6` M004/S02 task 2 — the non-scholarly entries are retired, and Agawu replaces ref-8 — Slice M004/S02
- `d578917` M004/S02 task 3 — every mismatch is corrected to what its document is — Slice M004/S02
- `4ce0b97` M004/S02 task 4 — the manifest holds no mismatch and nothing to replace — Slice M004/S02, Rows VR13

**Untraced commits:** none.

## What a reviewer should look at twice

- **The shipping guide changed.** The appendix introduction gains the policy
  paragraph; chapter 8's link text reads "Scherzinger 2018", chapter 9's
  "Linn 2013"; the appendix's `ref-46`, Scherzinger and Linn entries are
  corrected. Everything else is in the deferred theory bundle.
- **Retired numbers leave gaps.** The theory bundle's numbered list now runs
  with holes (no [4], [7], [8], …) because anchors are never reassigned. M005
  is where the two lists become one and numbering is reconsidered.
- **Two kept Tier-B entries rest on the owner's judgement** (`ref-20`
  Yudane, `ref-42` Santos Neto), recorded as practitioners' own texts.
- **Judgment calls in flight**, all in the decisions file: `POLICY-VIDEO`
  checks the sentence as written; a naming test's floor of 100 anchors now
  compares with the manifest; and three archived files were renamed when
  their authors were corrected.
- **Archive changes are outside the diff:** three renames (`28 -`, `41 -`,
  `42 -`) and Agawu's file renamed into the convention. Retired entries'
  files stay in the archive, unreferenced.
- **M005 inherits** the `ref-35`/`fr-scherzinger-2010` duplicate and the
  numbering gaps.

## Decisions

## 2026-10-03 — planning M004/S01 and M004/S02

Measured before asking, on `main` at `a68684b`. Every subject of VR11, VR12
and VR13 is in `site/src/content/theory/theory-references.mdx` and nowhere in
the shipping appendix. **All but one are cited only from a deep dive's closing
"See also refs [...]" line**, after the page's Primary list of scholarship, so
removing one rewrites no claim. The exception is `ref-36` (Brettworks), which
carries the programmed-swing lineage (theory-electronic-breakbeat, Rule 4) and
the funk micro-offset claim (theory-funk-soul). The manifest holds 15
`mismatch` verdicts; three of them (`ref-46`, `fr-scherzinger-2010`,
`fr-linn-attack-2020`) are cited from shipping chapters 1, 7, 8 and 9, so this
milestone touches the shipping guide as well as the bundle. `ref-1`, `ref-34`
and `fr-powers-1980` were corrected in the shipping appendix by first-release
M003, but the theory bundle still carries the old `ref-1` and `ref-34` text and
the manifest still says `mismatch` for all three. Crossref confirms Agawu,
"The Rhythmic Structure of West African Music", *Journal of Musicology* 5(3),
1987, 400–418, DOI `10.2307/763699`; and `ref-28`'s document is
Srinivasamurthy, Holzapfel & Serra (2014), *Journal of New Music Research*
43(1), 94–114, DOI `10.1080/09298215.2013.879902`, on Crossref and the
postprint's own title page alike.

- **Q:** VR11: the Wikipedia policy? — **A:** never cited.
- **Decision:** the guide cites no Wikipedia article; `ref-4`, `ref-32` and
  `ref-43` are retired, and a claim test forbids `wikipedia.org` anywhere under
  `site/src/content/` — **Why:** the owner's call; the three support no claim.
- **Q:** VR12: keep "no YouTube URL remains" as written? — **A:** yes, remove
  all five.
- **Decision:** `ref-10`, `ref-11`, `ref-14`, `ref-15`, `ref-25` are retired
  and a claim test forbids `youtube.com` and `youtu.be` under
  `site/src/content/`. The owner's Somashekar Jois suggestion for `ref-25` is
  declined by this policy, not by its merit; the Indian-classical deep dive
  already names Nelson's *Solkattu Manual* as primary — **Why:** the rule
  holds for a good video too, which is the point of VR12.
- **Q:** VR13: the default treatment? — **A:** drop, replace only on a lead.
- **Decision:** retire every VR13 subject from its See-also line and the
  bundle unless a decision below says otherwise; the only replacement is
  Agawu 1987 for `ref-8` — **Why:** the scholarly sources are already in each
  page's Primary list; a See-also line of blogs adds nothing they lack.
- **Q:** When an entry leaves both bibliographies, how is its manifest record
  handled? — **A:** retire the anchor.
- **Decision:** the bibliography entry and its manifest record are deleted;
  the number is never reused, the gap in numbering stays, the archived file
  stays in the owner's archive, and each record's last state is quoted in the
  evidence — **Why:** the manifest's completeness test requires every record
  to name a live anchor, and git keeps the history.
- **Q:** `ref-36`? — **A:** repoint to Linn, then drop.
- **Decision:** the swing-lineage claim cites `fr-linn-attack-2020` (Linn's
  own words); the micro-offset claim keeps Danielsen 2006 and `ref-39`, which
  carry it; `ref-36` is retired.
- **Q:** How does Agawu 1987 enter? — **A:** a new Further Reading entry.
- **Decision:** `fr-agawu-1987`, Tier A, in the theory bundle's Sub-Saharan
  Further Reading, taking `ref-8`'s place in the See-also line; the owner's
  copy is renamed `FR - Agawu 1987.pdf` and its manifest record names JSTOR as
  the route.
- **Q:** `ref-20` (Yudane) and `ref-42` (Santos Neto)? — **A:** keep both,
  corrected.
- **Decision:** each entry is corrected to what the document is — Yudane,
  "Notation for Gamelan Bali"; Santos Neto, J., "Ginga: a Brazilian way to
  groove" — stays Tier B, and its manifest note records the reason: a
  practitioner's own text, reviewable and free.
- **Q:** The Holzapfel trio? — **A:** correct two, retire one.
- **Decision:** `ref-28` becomes Srinivasamurthy, Holzapfel & Serra (2014)
  and `ref-29` Holzapfel & Bozkurt (2012); theory-balkan's one "Holzapfel 2015"
  citation is repointed to `ref-29` as "Holzapfel & Bozkurt 2012", and
  `fr-holzapfel-2015` is retired.

### Taken on the owner's behalf

- **The policy is stated once, in the shipping appendix's introduction.** It
  is the bibliography a reader meets, and the theory bundle's introduction
  points to it rather than restating it. S01 writes the Wikipedia clause; S02
  extends the same paragraph for video and non-scholarly web pages.
- **Every mismatch is corrected to what its document is, citation by
  citation, and the link text follows.** `fr-scherzinger-2010` becomes
  Scherzinger (2018) in *Clash!* and its inline citations read "Scherzinger
  2018"; `fr-linn-attack-2020` becomes Scarth & Linn (2013) with its URL and
  its inline citations read "Linn 2013". Anchors keep their names, as
  `fr-powers-1980` did: anchors are never reassigned.
- **A corrected entry's manifest verdict becomes `verified`**, with the
  correction in `note`, because the document was read in M002 or M003; `ref-1`,
  `ref-34` and `fr-powers-1980`, already corrected in the appendix, follow.
- **Each correction and each policy is locked by a claim test in
  `citation-tier.test.mjs`**, following the `REF1-TOUSSAINT` and
  `REF34-SCHWARZ` shape: the wrong form forbidden, the right one present and
  scoped to the entry's own line.
- **The naming test's `ref-43` case moves to a synthetic line.** It was the
  only bibliography URL with parentheses; `parseLine` is exported so the
  parenthesis rule is tested on a fixed string rather than on an entry this
  milestone retires.
- **The theory bundle's "93 entries" becomes the measured count**, and the
  sentence keeps its derivation claim, which the bundle test enforces.

### Deferred

- None. Every decision the milestone needs was taken above; no task waits on
  the owner.

## 2026-10-03 — judgment calls during M004/S02 task 1

- **`POLICY-VIDEO` checks "Nor does it cite a video", not "nor a video".**
  The plan's literal string only fits an ungrammatical sentence; the claim
  pins the sentence as written. Obviously right: the test exists to hold the
  policy in place, not a particular fragment.
- **`readBibliography spans both bibliographies` compares with the manifest,
  not with 100.** M003's test asserted more than 100 anchors; retiring the
  videos took the total to 98 and turned it red. The test now requires one
  parsed entry per manifest record, the count the completeness test already
  ties to the anchors. Obviously right: the floor was a literal standing in
  for "both files were read", and this milestone exists to shrink the count.

## 2026-10-03 — judgment calls during M004/S02 task 3

- **Three archived files were renamed when their entries' authors were
  corrected.** The convention names a numbered file by its lead author, so
  `28 - Holzapfel.pdf`, `41 - Rhythmic Ambiguity in Aphex Twin.pdf` and
  `42 - Schloss.pdf` became `28 - Srinivasamurthy.pdf`,
  `41 - Papavassiliou.pdf` and `42 - Santos Neto.pdf` (`mv -n`), and the
  manifest followed. Obviously right: the convention test turned red exactly
  as designed, and the old names credited the wrong authors.
