# M004 — decisions

Append-only. Every question `/jk:auto` asked before running, every answer, and
every choice taken on the owner's behalf.

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
