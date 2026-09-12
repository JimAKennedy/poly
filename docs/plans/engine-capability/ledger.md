---
class: gated
---

# Engine Capability Ledger

Status: current (2026-09-12)

**Source:** `docs/plans/engine-capability/deferrals.md` — the eleven places where
the Poly Guide is honest about a limitation in Poly itself, gathered by the
theory-audit programme as it corrected the guide against an external audit. Every
item in that document appears below as exactly one row, `EC01`–`EC11`, and that
range is closed. The source document's own rule is what makes the coverage claim
checkable: an item belongs there only if the repo already says Poly should do it.

**Reconciled before planning.** Each of the eleven was checked against the tree
rather than taken from the document's summary — the document asked for this,
having shipped two wrong verdicts in this category before. All three prose
promises are present; all five `absentColumn` verdicts re-derive correctly from
the patch tables, each page carrying exactly one `<PolyPatch>`; all four trackers
are open. Nothing in the document is contradicted by the code. Two findings
rescoped the work and are recorded in the rows that carry them: the engine
already holds every value the five missing columns would report, and `#156`'s
mechanism already ships with four hand-authored exact timelines behind it.

**Row series.** `EC01`–`EC11`, one per source item. The `D` prefix the source
document uses for its own section numbering is deliberately not reused here:
`engine/src/presets.cpp` already carries row IDs `D017` and `D020` from the
programme that landed in PR #171, and a second `D` numbering in the same tree
would be ambiguous to grep.

**Row vocabulary.** Beyond the required `ID`, `Item`, `Verification` and
`Status`, each row carries a `Kind` (`prose` a promise in the guide's own voice ·
`rule` a rule weakened or unverifiable · `column` a rule the patch table cannot
express · `tracker` an issue the theory-audit ledger named as deferred) and a
`Lands in` pointer. Both are informational and ignored by the `ledger` check;
`Lands in` is a forecast while a row is open and a record once it closes.

**Numbering.** Milestone IDs `M001`–`M003` collide with both Poly's legacy
commit-message milestones and the theory-audit ledger's. The `Plan:` trailer
disambiguates: `git log --grep="Plan: docs/plans/engine-capability"` selects this
programme's commits and nothing else.

---

## Milestone M001 — What the engine can already deliver

**Vision:** Every promise the guide makes that needs no new engine capability is
kept — the patch tables report the values the engine already holds, and the
claves the guide calls non-Euclidean ship as presets that play them.
**Branch:** milestone/M001-already-deliverable
**Status:** planned
**Demo:** Four rules that read "not checkable, absent column" now read
`checkable` and are checked; loading `Cuban Son Montuno` plays a son clave rather
than `E(5,16)`.

### Slice M001/S01 — Columns and their predicates

**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M001-S01.md
**Status:** open

**Definition of Done**

- [ ] Each of the four pages' patch table carries the column its rule needs, with
      values that agree with `site/src/generated/presets.json`
- [ ] Each of the four rules reads `checkable` in `RULE_TRIAGE`, with a predicate
      that has been shown to fail when the table is mutated
- [ ] The `absentColumn` reverse audit names four fewer rules, and still fails if
      a verdict claims a column the table actually has

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC01 | `theory-balkan` Rule 7 ("Humanize ≤ 0.15") is unverifiable because the patch table has no `Humanize` column, though every lane carries `humanizeMs` and `PresetTable` already renders that column name | `column` | `theory-balkan.mdx`, `site/tests/theory-patch-conformance.test.mjs` | The rule's triage entry flips to `checkable` with a predicate reading the new column; the predicate is mutation-proved by raising a lane's Humanize above the bound and watching the named case fail | `open` |
| EC02 | `theory-electronic-breakbeat` Rule 4 ("swing is a bus, not a per-note gesture") is unverifiable because the patch table has no `Swing` column, though every lane carries `swingAmount` | `column` | `theory-electronic-breakbeat.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by giving one lane a per-lane swing value the rule forbids | `open` |
| EC03 | `theory-gamelan` Rule 9 (density scales inversely with register) is unverifiable because the patch table has no `Note` column, though every lane carries `noteNumber` | `column` | `theory-gamelan.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by inverting two lanes' note numbers so the density relation reverses | `open` |
| EC04 | `theory-sub-saharan-africa` Rule 7 (register and rate separate the voices) is unverifiable because the patch table has no `Note` column | `column` | `theory-sub-saharan-africa.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by collapsing two voices onto one note number | `open` |

### Slice M001/S02 — Exact timelines

**Validation:** format, unit, engine-isolation, site-unit, doc-conformance
**Evidence:** evidence/M001-S02.md
**Status:** open

**Definition of Done**

- [ ] Son clave, rumba clave and Clapping Music ship as presets whose lanes carry
      hand-authored `fixedPattern` timelines, in the manner the four existing
      hand-authored timelines already use
- [ ] `Cuban Son Montuno`'s clave lane is no longer the Euclidean pattern
      `lockReferentLane` bakes
- [ ] A test asserts each shipped timeline equals its published pattern and
      differs from the Euclidean pattern of the same hit count and cycle length
- [ ] The guide's instruction to hand-build a true clave in timeline mode no
      longer describes the only way to obtain one

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC05 | Issue #156, which the theory-audit ledger names as deferred: exact non-Euclidean timelines as first-class presets. Partly satisfied — the mechanism ships and four builders already hand-author exact patterns (`makeAfrobeat12_8`, `makeBossaNova`, `makeEweAgbekor`, `makeAfrobeatLagos`), covering the issue's bell variants and teleco-teco. Missing: clave and Clapping Music. The gap is visible in the guide, which tells the reader the exact son and rumba claves are non-Euclidean and to use the timeline-mode workflow to get the true pattern — a workaround the shipped `Cuban Son Montuno` preset needs, its clave lane being one of the 39 lanes `lockReferentLane` bakes from `euclidean()` | `tracker` | `engine/src/presets.cpp`, `tests/preset_tests.cpp`, `theory-afro-cuban.mdx` | A gtest asserts the clave lanes' `fixedPattern` matches the published son and rumba patterns and is not the Euclidean pattern of the same hit count and cycle; the site suite asserts the guide no longer presents the workaround as the only route | `open` |

---

## Milestone M002 — Kotekan modes

**Vision:** The interlock style a gamelan patch uses is a setting in Poly rather
than a description in the guide, so the two rules currently weakened or
unverifiable because the engine can only derive the strict complement become
checkable as written.
**Branch:** milestone/M002-kotekan-modes
**Status:** planned
**Demo:** A gamelan patch names norot, telu or empat; `theory-gamelan` Rule 4 is
checked as written rather than as construction step 4 specifies, and Rule 1's
predicate can be made to fail.

### Slice M002/S01 — The mode is something a patch can express

**Validation:** format, unit, engine-isolation, rt-safety, webui-e2e
**Evidence:** evidence/M002-S01.md
**Status:** open

**Definition of Done**

- [ ] A gamelan patch can name its interlock style, rather than only a source lane
- [ ] Polos–sangsih overlap is controllable, so the composite is no longer
      complete by construction
- [ ] The mode a preset uses reaches `site/src/generated/presets.json`, so a patch
      table can report it
- [ ] `renderRange()` gains no allocation, lock or blocking call
- [ ] The engine builds and passes its tests with no VST3 SDK present

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC06 | `theory-gamelan` Rule 4 ends "add deliberate doublings via a third lane or accent masks until kotekan modes ship" — issue #153. Poly's kotekan is a source-lane parameter (`KotekanSrc`, −1..7) with no modes, so Rule 5's four named interlock styles are a choice the guide describes and the tool cannot express | `prose` | `engine/`, `plugin/source/`, `webui/`, `engine/src/presets.cpp` | Engine tests cover each mode's composite against the mode's definition, including a case that the overlap control changes the composite; the exporter round-trip proves the mode reaches `presets.json` | `open` |

### Slice M002/S02 — The guide catches up

**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M002-S02.md
**Status:** open
**Depends:** M002/S01

**Definition of Done**

- [ ] Rule 4 no longer carries the "until kotekan modes ship" parenthetical
- [ ] Rule 4 is checked as written — pair-overlap — rather than as construction
      step 4 specifies
- [ ] Rule 1 reads `checkable`, with a predicate that has been shown to fail
- [ ] This ledger records that the theory-audit ledger's F41 weakening no longer
      describes the shipped behaviour

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC07 | Theory-audit row F41 records M004/S03 checking Rule 4 as construction step 4 specifies, because Rule 4 as written demands pair-overlap that Kotekan L-mode cannot produce. M007 then found the strict predicate could not be made to fail at all, and marked Rule 1 not checkable for the same cause. Two rules on one page, weakened by one engine limitation | `rule` | `theory-gamelan.mdx`, `site/tests/theory-patch-conformance.test.mjs` | Both predicates are mutation-proved against a patch using a mode with deliberate overlap — the mutation that could not fail under L-mode now fails | `open` |

---

## Milestone M003 — Non-isochronous timing

**Vision:** Poly plays the uneven subdivisions the guide describes — samba and
jembe feel, and the long beats of additive meters — instead of approximating them
with swing, and the guide stops admitting the approximation.
**Branch:** milestone/M003-non-isochronous-timing
**Status:** planned
**Demo:** A samba patch plays its long-short-short-long feel from a profile
rather than from swing plus offsets; a Balkan patch's long beat carries its own
subdivision.

### Slice M003/S01 — Subdivision profiles

**Validation:** format, unit, engine-isolation, rt-safety
**Evidence:** evidence/M003-S01.md
**Status:** open

**Definition of Done**

- [ ] A lane can play a non-isochronous subdivision profile rather than an even grid
- [ ] A profile is expressible in a preset and reaches `site/src/generated/presets.json`
- [ ] `renderRange()` gains no allocation, lock or blocking call
- [ ] The engine builds and passes its tests with no VST3 SDK present

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC08 | `theory-brazilian` Rule 6 ends "until subdivision profiles ship, use light swing (0.15–0.25) plus small per-lane offsets as an admitted approximation" — issue #150. Swing displaces only alternate notes, which is why the guide calls the workaround an approximation in its own voice | `prose` | `engine/`, `engine/src/presets.cpp` | Engine tests assert a profiled lane's onset times differ from both the isochronous grid and the swung grid, and are stable under the determinism golden | `open` |

### Slice M003/S02 — Cell-aware aksak swing

**Validation:** format, unit, engine-isolation, rt-safety
**Evidence:** evidence/M003-S02.md
**Status:** open

**Definition of Done**

- [ ] Swing on an additive meter applies per cell rather than to alternate notes
      across the bar
- [ ] A Balkan preset's long beat carries the feel the guide describes
- [ ] `renderRange()` gains no allocation, lock or blocking call

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC09 | Issue #157, which the theory-audit ledger names as deferred: cell-aware swing and long-beat feel for additive (aksak) meters. F32 locked the Balkan long-beat honesty note; the engine change stayed on the tracker | `tracker` | `engine/`, `engine/src/presets.cpp` | Engine tests assert swing applied to a 2+2+3 cell structure displaces within cells rather than across the bar, and that the long cell's internal division differs from the short cells' | `open` |

### Slice M003/S03 — The measured jembe profile

**Validation:** format, unit, site-unit, doc-conformance
**Evidence:** evidence/M003-S03.md
**Status:** open
**Depends:** M003/S01

**Definition of Done**

- [ ] A measured jembe profile ships as data, with its source cited in the guide's
      bibliography at a tier the citation check accepts
- [ ] `theory-sub-saharan-africa` construction step 5 no longer says Poly ships no
      measured profile
- [ ] The profile's values are reachable from a preset

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC10 | `theory-sub-saharan-africa` construction step 5 says "what Poly does not ship is a measured jembe profile to put in them". Row B10 established the distinction that scopes this: the mechanism exists (`microTimingMs`, exposed through the WebUI's micro-timing bars, clamped to ±20 ms) and the data does not — content to author, not code to write | `prose` | `engine/src/presets.cpp`, `theory-sub-saharan-africa.mdx`, `appendix-references.mdx` | A site case asserts the construction step no longer disclaims the profile and that its cited source resolves to a bibliography entry at an accepted tier | `open` |

### Slice M003/S04 — The guide catches up

**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M003-S04.md
**Status:** open
**Depends:** M003/S01

**Definition of Done**

- [ ] `theory-brazilian`'s patch table carries a `Timing` column expressing the
      per-beat profile
- [ ] Rule 6 reads `checkable`, with a predicate that has been shown to fail when
      the profile is flattened
- [ ] The "until subdivision profiles ship" sentence is gone from the page

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC11 | `theory-brazilian` Rule 6 (the long-short-short-long feel) is the one `absentColumn` verdict that is not a docs-only fix: the table has no `Timing` column, and a per-beat profile is not a single timing offset, so the column cannot be filled until EC08 ships | `column` | `theory-brazilian.mdx`, `site/tests/theory-patch-conformance.test.mjs` | The rule's triage entry flips to `checkable`; the predicate is mutation-proved by flattening the profile to an even grid and watching the named case fail | `open` |

---

## Sequencing

The graph is deliberately flat. No milestone depends on another: M001 needs no
new engine capability at all, and M002 and M003 are independent engine features
that touch different parts of the timing path.

Three slice-level dependencies are real:

- **M002/S02 depends on M002/S01.** The guide's parenthetical is true until the
  modes ship, and Rule 4's strict predicate cannot be made to fail before then —
  M007 established exactly that. Editing the prose first would make the guide
  claim a capability the engine lacks.
- **M003/S03 depends on M003/S01.** A measured profile is data with nowhere to
  go until the profile mechanism exists. B10's distinction holds: the micro-timing
  mechanism can hold the values today, but #150's template mechanism is what makes
  authoring them useful.
- **M003/S04 depends on M003/S01.** The `Timing` column reports a profile, so
  there is nothing to put in it until profiles exist.

M003/S02 depends on nothing and may land before or after M003/S01.

## Related issues

- [#150](https://github.com/JimAKennedy/poly/issues/150) — non-isochronous
  subdivision profiles. Closed by EC08, with EC10 and EC11 depending on it.
- [#153](https://github.com/JimAKennedy/poly/issues/153) — kotekan modes. Closed
  by EC06; EC07 is the guide-side consequence.
- [#156](https://github.com/JimAKennedy/poly/issues/156) — exact non-Euclidean
  timelines as presets. Partly shipped already; EC05 closes the remainder.
- [#157](https://github.com/JimAKennedy/poly/issues/157) — cell-aware swing for
  additive meters. Closed by EC09.

## Out of scope

Recorded so a later pass does not rediscover them as omissions. The source
document excluded each, and its reasoning is adopted here.

- **CI and tooling debt** — [#282](https://github.com/JimAKennedy/poly/issues/282),
  [#272](https://github.com/JimAKennedy/poly/issues/272),
  [#274](https://github.com/JimAKennedy/poly/issues/274),
  [#267](https://github.com/JimAKennedy/poly/issues/267),
  [#266](https://github.com/JimAKennedy/poly/issues/266). Real, but they
  interlock with nothing here and most are single fixes; a ledger's traceability
  costs more than it returns on work that does not interlock.

  One qualification this assessment adds. #282 is not wholly disjoint from M002
  and M003: both rebuild and commit `webui/poly_engine.{js,wasm}`, and a
  non-reproducible build means those bytes churn without a source change, so the
  discipline recorded in `CLAUDE.md` cannot distinguish a real rebuild from
  noise. It is a commit-hygiene hazard for those milestones, not a failing gate —
  `wasm-freshness` compares the deployed artifacts against the checked-in ones by
  hash and never rebuilds — so it stays out of scope, named rather than assumed.

- **Feature issues with no promise behind them** —
  [#245](https://github.com/JimAKennedy/poly/issues/245),
  [#152](https://github.com/JimAKennedy/poly/issues/152),
  [#154](https://github.com/JimAKennedy/poly/issues/154),
  [#155](https://github.com/JimAKennedy/poly/issues/155),
  [#158](https://github.com/JimAKennedy/poly/issues/158) and others are good
  ideas the guide does not currently claim. Including them would make this a
  backlog, and a ledger assessed from a backlog cannot claim complete coverage
  of anything.
