# M004/S02 — The remaining unreviewable entries go

**Slice:** M004/S02 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR12 (YouTube), VR13 (entries that are not scholarship, any tier)
**Classification:** bounded, and editorial. Every change is to the two
bibliographies, the pages that cite them, and the manifest, each locked by a
claim test in the shape S01 and `REF1-TOUSSAINT` use. Decisions are in
`M004-decisions.md`; this plan consumes them and asks nothing.

## Task status

- [x] 1. No video is cited
- [x] 2. The non-scholarly entries are retired, and Agawu replaces ref-8
- [x] 3. Every mismatch is corrected to what its document is
- [ ] 4. The manifest holds no mismatch and nothing to replace

## Definition of Done

Copied verbatim from the slice:

- [ ] No YouTube entry remains in the bibliography
- [ ] Every remaining entry that is not scholarship — whatever its tier — is
      either reviewable text from a source fit to cite, or the primary
      artefact rather than a commentary on one, with that reason recorded
- [ ] Every entry the manifest marks as a description mismatch is corrected
      or replaced — M002's and M003's alike
- [ ] Every entry the manifest marks `to-replace` is replaced, or dropped with
      its claim rewritten

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Archive renames are proved locally by
`POLY_REFERENCES_ARCHIVE=<archive> node scripts/fetch-references.mjs --verify`,
with the output in the evidence.

## Retiring an entry

Used by tasks 1–3. To retire anchor `A`: copy its manifest record verbatim
into the evidence; delete its line from whichever bibliography defines it;
delete its link from every page that cites it, keeping the sentence
grammatical; delete its record from `references.json`. If a citation carried a
claim (only `ref-36` and `fr-holzapfel-2015` do), the claim is repointed
before the link goes, never left uncited. The archived file stays in the
owner's archive. `theory-bundle-references.test.mjs` and the manifest
completeness test prove nothing dangles, in both directions.

## Task 1 — No video is cited

**Files:** `site/tests/citation-tier.test.mjs`,
`site/src/content/docs/appendix-references.mdx`,
`site/src/content/theory/theory-references.mdx`, `theory-afro-cuban.mdx`,
`theory-afrobeat.mdx`, `theory-indian-classical.mdx`,
`site/src/data/references.json`; the evidence file.

1. Extend S01's tree-wide test into a list of forbidden hosts with a reason
   each, and add `youtube.com` and `youtu.be` (VR12). Add `present` to
   `POLICY-WIKIPEDIA`'s sibling `POLICY-VIDEO`:
   `['nor a video']`. Run: both red.
2. Extend the **What this bibliography cites.** paragraph: nor a video — it
   may be excellent, but it has no text a reader can check against the claim.
3. Retire `ref-10`, `ref-11`, `ref-14`, `ref-15`, `ref-25` from the See-also
   lines in `theory-afro-cuban.mdx`, `theory-afrobeat.mdx` and
   `theory-indian-classical.mdx`, the bundle, and the manifest.
4. Update the bundle introduction's measured entry count.
5. Run the claim file, `site-unit`, `format`, `doc-conformance`: all pass.
6. Evidence: the five records, the three See-also lines before and after,
   and the gates. Commit with `Rows: VR12`, VR12 `done`.

## Task 2 — The non-scholarly entries are retired, and Agawu replaces ref-8

**Files:** `site/tests/citation-tier.test.mjs`,
`site/tests/references-manifest.test.mjs`,
`site/src/content/docs/appendix-references.mdx`,
`site/src/content/theory/theory-references.mdx`,
`theory-sub-saharan-africa.mdx`, `theory-afro-cuban.mdx`,
`theory-afrobeat.mdx`, `theory-indian-classical.mdx`, `theory-balkan.mdx`,
`theory-minimalism.mdx`, `theory-electronic-breakbeat.mdx`,
`theory-funk-soul.mdx`, `site/src/data/references.json`; the archive; the
evidence file.

1. Add to the forbidden-host list, each with its reason: `scribd.com`,
   `scalar.usc.edu`, `pianowithjonny.com`, `samplesoundmusic.com`,
   `lianproductions.com`, `artiumacademy.com`, `pubpub.org`,
   `chromatone.center`, `allclassical.org`, `brettworks.com`,
   `noisemachines.studio`, `ethanhein.com`. Add claim `POLICY-WEB` with
   `present: ['course material']`. Add claim `AGAWU-1987` on
   `theory-references.mdx`:
   `presentRegex: [/id="fr-agawu-1987"[^\n]*Journal of Musicology/, /id="fr-agawu-1987"[^\n]*10\.2307\/763699/]`.
   Add a test to `references-manifest.test.mjs`: no record's
   `retrieval.status` is `to-replace`. Run: all red.
2. Extend the policy paragraph: nor course material, a commercial or
   marketing page, or a blog; a practitioner's own text may be cited, at
   Tier B, with the reason recorded beside it.
3. Repoint `ref-36` first. In `theory-electronic-breakbeat.mdx` the Rule 4
   attribution ("programmed swing's lineage … follows ref [36]") cites
   `fr-linn-attack-2020` instead. In `theory-funk-soul.mdx` line 27's
   micro-offset sentence drops `ref-36` and keeps Danielsen 2006 and `ref-39`.
4. Retire `ref-7`, `ref-8`, `ref-12`, `ref-16`, `ref-17`, `ref-21`, `ref-24`,
   `ref-27`, `ref-33`, `ref-36`, `ref-37`, `ref-38`.
5. Add `fr-agawu-1987` (Tier A) to the bundle's Sub-Saharan Further Reading:
   Agawu, V. K. (1987). "The Rhythmic Structure of West African Music."
   *Journal of Musicology*, 5(3), 400–418. [DOI](https://doi.org/10.2307/763699)
   — with a one-clause gloss of its subject read from its first page. In
   `theory-sub-saharan-africa.mdx`'s See-also line, Agawu 1987 takes `ref-8`'s
   place. Add its manifest record: `purchasable` with the JPASS route as the
   price note (the owner's personal subscription), `verified` from the title
   page, `archiveFile: "FR - Agawu 1987.pdf"`, `retrieval: archived`. Rename
   the owner's file
   `477028076-The-rhythmic-structure-of-west-african-music-kofi-agawu.pdf` to
   that name with `mv -n`.
6. Update the bundle's measured entry count.
7. Run the claim file, the manifest file, `site-unit`, `format`,
   `doc-conformance`, and `--verify`: all pass.
8. Evidence: every retired record, every See-also line before and after, the
   two repointed `ref-36` sentences before and after, the Agawu title page
   reading, and the gates. Commit with `Rows:` empty (VR13 closes in task 4).

## Task 3 — Every mismatch is corrected to what its document is

**Files:** `site/tests/citation-tier.test.mjs`,
`site/src/content/docs/appendix-references.mdx`,
`site/src/content/theory/theory-references.mdx`,
`site/src/content/docs/08-minimalism.mdx`, `09-electronic.mdx`,
`theory-minimalism.mdx`, `theory-electronic-breakbeat.mdx`,
`theory-balkan.mdx`, `site/src/data/references.json`; the evidence file.

1. Write one claim per correction, each forbidding the wrong form and
   requiring the right one scoped to the entry's own line, and run them red:

   | Claim | File | Forbidden | Present (on the entry's line) |
   |---|---|---|---|
   | `REF1-TOUSSAINT-THEORY` | theory | `0705\.4085` | `bridgesmathart\.org/2005/bridges2005-47` |
   | `REF34-SCHWARZ-THEORY` | theory | `Reich, S. "Music as a Gradual Process` | `Schwarz, K\. R\.` |
   | `REF20-YUDANE` | theory | `Introduction to Balinese Gamelan` | `Notation for Gamelan Bali` |
   | `REF23-REINDL` | theory | `Comparative Perspective` | `Sources of Inspiration for Western Composers`, `Analytical Approaches to World Musics` |
   | `REF28-SRINIVASAMURTHY` | theory | `Metrical Structure in Turkish Makam Music` | `Srinivasamurthy`, `10\.1080/09298215\.2013\.879902` |
   | `REF29-HOLZAPFEL-BOZKURT` | theory | `Syncopation Distribution` | `Metrical Strength and Contradiction in Turkish Makam Music`, `Bozkurt` |
   | `REF31-AJI` | theory | `Arab Rhythmic Cycles` | `Rhythmic-Temporal Disruptions` |
   | `REF41-PAPAVASSILIOU` | theory | `Rhythmic Ambiguity in Aphex Twin` | `Papavassiliou`, `Cahiers de la Société québécoise de recherche en musique` |
   | `REF42-SANTOS-NETO` | theory | `Schloss, A\.` | `Santos Neto` |
   | `REF46-CNTRL-99` | appendix | `CNTRL-100` | `SNS-NOTE-CNTRL-99` |
   | `SCHERZINGER-2018` | appendix and theory | `Proceedings of the ICTM` | `Clash!`, `2018` |
   | `LINN-2013` | appendix and theory | `Attack Magazine\* \(2020\)` | `2013`, `attackmagazine\.com` |

   And two tree-wide forbids across `site/src/content`: the link texts
   `Scherzinger 2010` and `Linn 2020`. A claim covering two files is two
   `CLAIMS` entries sharing an id prefix.
2. Correct each entry in its bibliography (both, where it appears in both),
   using the manifest's `note` and title-page readings as the only source of
   the corrected text. `ref-31`: Aji, I. "Rhythmic-Temporal Disruptions and
   the Feeling of Ṭarab." *Theory and Practice* 49–50 (2025) — the volume and
   year as printed on the archived page. `fr-linn-attack-2020`: Scarth, G. &
   Linn, R. (2013), with its URL. Inline link texts change: "Scherzinger 2010"
   → "Scherzinger 2018" (`08-minimalism.mdx`, `theory-minimalism.mdx`),
   "Linn 2020" → "Linn 2013" (`09-electronic.mdx`,
   `theory-electronic-breakbeat.mdx`).
3. Retire `fr-holzapfel-2015`: repoint `theory-balkan.mdx`'s "Holzapfel 2015
   … (the related Turkish usul system)" to `ref-29` as "Holzapfel & Bozkurt
   2012", then retire it.
4. Set each corrected record's `description` to `verified`, keeping `note` as
   the history and adding the correction's date; the same for `ref-1`,
   `ref-34` and `fr-powers-1980`, corrected in the appendix by first-release
   M003.
5. Run the claim file, `site-unit`, `format`, `doc-conformance`: all pass.
   The shipping pages changed (`08`, `09`, the appendix), so read the three
   rendered sentences in the diff before committing.
6. Evidence: each entry before and after, the inline link texts before and
   after, the Holzapfel repoint, and the gates. Commit with `Rows:` empty.

## Task 4 — The manifest holds no mismatch and nothing to replace

**Files:** `site/tests/references-manifest.test.mjs`, the ledger, the
evidence file, `site/src/data/references.json` only if the test finds
something.

1. Add a test: no record's `description` is `mismatch`. Run it; if red, the
   offender is a task 3 omission — fix it in this task only if it is one of
   the entries task 3 named, otherwise halt.
2. For each remaining non-scholarly entry kept by decision (`ref-20`,
   `ref-42`, `fr-linn-attack-2020`), confirm its manifest `note` records the
   reason it stays (practitioner's own text, or primary account). Add the
   reason where missing.
3. Run `site-unit`, `format`, `doc-conformance`, `--verify`; all pass.
4. Evidence; tick the DoD; set VR13 and the slice `done`; run
   `jk-standards ledger`. Commit with `Rows: VR13`.

## Self-review

- **DoD 1** (no YouTube) — task 1, tree-wide host test.
- **DoD 2** (remaining non-scholarship fit to cite, reason recorded) — task 2
  retires the rest; task 4 step 2 records the reasons for the three kept.
- **DoD 3** (every mismatch corrected) — task 3, then task 4's no-mismatch
  test.
- **DoD 4** (every `to-replace` replaced or dropped with its claim
  rewritten) — task 2: `ref-8` replaced by Agawu, `ref-7`, `ref-21`, `ref-24`
  dropped (See-also only, no claim), locked by the no-`to-replace` test.
- **VR12** — "No `youtube.com` URL remains; a claim test forbids their
  return": task 1. **VR13** — each replaced, kept with a reason, or dropped:
  tasks 2 and 4.
- **Names** — `POLICY-WIKIPEDIA`, `POLICY-VIDEO`, `POLICY-WEB`, the
  forbidden-host list, and the "What this bibliography cites." paragraph are
  shared with S01.
- **Placeholders** — none; every corrected text is sourced from a recorded
  title-page reading or Crossref.
