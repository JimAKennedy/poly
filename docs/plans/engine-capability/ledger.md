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

**Row series.** `EC01`–`EC11`, one per item in the source document, and that
range is closed. Two later series have other origins.
`DAW01`–`DAW07` are M004's, requested directly rather than drawn from
`deferrals.md`. `PIPE01`–`PIPE02` are M005's, and are defects this programme
found in its own tooling while executing M001 — the role the theory-audit
ledger's `B` series played for defects that programme found rather than
inherited. `GAP01`–`GAP02` are M006's, found the same way while shipping M005,
and one of them has a tracker issue behind it. None of these series is covered
by the coverage claim above, which says every item in `deferrals.md` has a row,
not that every row traces back to it. Each is marked on its own milestone rather
than left for a reader to notice that eleven rows trace to no source document. The `D` prefix the source document
uses for its own section numbering is deliberately not reused here:
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
**Status:** done
**Demo:** Four rules that read "not checkable, absent column" now read
`checkable` and are checked; loading `Cuban Son Montuno` plays a son clave rather
than `E(5,16)`.

### Slice M001/S01 — Columns and their predicates

**Plan:** M001-S01-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] Each of the four pages' patch table carries the column its rule needs, with
      values consistent with the page's own rules and the roles the table
      already names
- [x] Rule 7's Humanize bound is stated in the same unit the new column uses
- [x] Each of the four rules reads `checkable` in `RULE_TRIAGE`, with a predicate
      that has been shown to fail when the table is mutated
- [x] The `absentColumn` reverse audit names four fewer rules, and still fails if
      a verdict claims a column the table actually has

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC01 | `theory-balkan` Rule 7 ("Humanize ≤ 0.15") is unverifiable because the patch table has no `Humanize` column, though every lane carries `humanizeMs` and `PresetTable` already renders that column name | `column` | `theory-balkan.mdx`, `site/tests/theory-patch-conformance.test.mjs` | The rule's triage entry flips to `checkable` with a predicate reading the new column; the predicate is mutation-proved by raising a lane's Humanize above the bound and watching the named case fail | `done` |
| EC02 | `theory-electronic-breakbeat` Rule 4 ("swing is a bus, not a per-note gesture") is unverifiable because the patch table has no `Swing` column, though every lane carries `swingAmount` | `column` | `theory-electronic-breakbeat.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by giving one lane a per-lane swing value the rule forbids | `done` |
| EC03 | `theory-gamelan` Rule 9 (density scales inversely with register) is unverifiable because the patch table has no `Note` column, though every lane carries `noteNumber` | `column` | `theory-gamelan.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by inverting two lanes' note numbers so the density relation reverses | `done` |
| EC04 | `theory-sub-saharan-africa` Rule 7 (register and rate separate the voices) is unverifiable because the patch table has no `Note` column | `column` | `theory-sub-saharan-africa.mdx`, `site/tests/theory-patch-conformance.test.mjs` | As EC01, mutation-proved by collapsing two voices onto one note number | `done` |

### Slice M001/S02 — Exact timelines

**Plan:** M001-S02-plan.md
**Validation:** format, unit, engine-isolation, site-unit, doc-conformance
**Evidence:** evidence/M001-S02.md
**Status:** done

**Definition of Done**

- [x] Son clave, rumba clave and Clapping Music ship as presets whose lanes carry
      hand-authored `fixedPattern` timelines, in the manner the four existing
      hand-authored timelines already use
- [x] `Cuban Son Montuno`'s clave lane is no longer the Euclidean pattern
      `lockReferentLane` bakes
- [x] A test asserts each shipped timeline equals its published pattern and
      differs from the Euclidean pattern of the same hit count and cycle length
- [x] The guide's instruction to hand-build a true clave in timeline mode no
      longer describes the only way to obtain one

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| EC05 | Issue #156, which the theory-audit ledger names as deferred: exact non-Euclidean timelines as first-class presets. Partly satisfied — the mechanism ships and four builders already hand-author exact patterns (`makeAfrobeat12_8`, `makeBossaNova`, `makeEweAgbekor`, `makeAfrobeatLagos`), covering the issue's bell variants and teleco-teco. Missing: clave and Clapping Music. The gap is visible in the guide, which tells the reader the exact son and rumba claves are non-Euclidean and to use the timeline-mode workflow to get the true pattern — a workaround the shipped `Cuban Son Montuno` preset needs, its clave lane being one of the 39 lanes `lockReferentLane` bakes from `euclidean()` | `tracker` | `engine/src/presets.cpp`, `tests/preset_tests.cpp`, `theory-afro-cuban.mdx` | A gtest asserts the clave lanes' `fixedPattern` matches the published son and rumba patterns and is not the Euclidean pattern of the same hit count and cycle; the site suite asserts the guide no longer presents the workaround as the only route | `done` |

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

---

## Milestone M004 — Nightly DAW regression coverage

**Source note.** Unlike M001–M003, this milestone is not drawn from
`docs/plans/engine-capability/deferrals.md`. It was requested directly, and no
open issue asks for it. It is recorded here so the request has a plan of record;
the coverage claim this ledger makes about the deferrals document is unaffected,
because that claim is that every item in that document has a row, not that every
row traces to it.

**Vision:** The nightly Cubase run exercises the behaviours only a host can
break — session recall, preset recall, transport motion, editor lifecycle,
multiple instances, offline rendering and host automation — so a regression that
appears only inside a DAW fails the night it lands rather than in someone's
project.
**Branch:** milestone/M004-daw-regression
**Status:** planned
**Demo:** A nightly run whose summary lists a spec per area above, each green,
against a Cubase session the runner launched unattended.

**What exists today.** Three specs: `toggle-step` (toggle one kick step, play
the transport), `assert-probe` (the probe JSONL reflects that toggle), and
`export-midi` (the export chip and per-lane export write SMF an independent
`mido` parser validates). The harness under them is reusable and is what makes
this milestone tractable: Playwright attaching to the plugin's WebView over CDP,
`tests/cubase/driver/play_scenario.py` driving MIDI, and
`tests/cubase/compare_probe_golden.py` comparing probe output to a golden.

**The gate each slice actually owes.** `cubase-harness` type-checks the specs
and runs the helper-lib and validator unit tests, but it does not run Cubase.
The only thing that proves a nightly spec works is the nightly, so every slice
below requires a **named nightly run** in its evidence — the workflow run URL,
showing that spec green. A slice that cannot name one is not done, however green
its local gate is.

**One cost, stated once.** `jk-standards.yaml` declares `cubase-nightly` a
repo-wide global lock, because Cubase, loopMIDI and the interactive desktop
session exist once on a single self-hosted Windows runner. These seven slices
lengthen one serialised run, and that runner already has an open failure issue
([#267](https://github.com/JimAKennedy/poly/issues/267)).

### Slice M004/S01 — Session recall

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S01.md
**Status:** open

**Definition of Done**

- [ ] A Cubase project saved with a non-default Poly patch reopens carrying that
      patch — edited steps, selected preset, and per-step micro-timing
- [ ] The spec has been shown to fail when the saved state is perturbed before
      reopening, so it is a round-trip check rather than a "did it load" check
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW01 | Nothing exercises Poly's state round-trip inside a host. `kStateVersion` is at 16 and `CLAUDE.md` calls serialising without a version "a preset compatibility time bomb", but no test saves a Cubase project and reopens it | `coverage` | `tests/cubase/e2e/`, `.github/workflows/cubase-nightly.yml` | A spec saves the project, reopens it, and asserts the patch matches what was saved; proved by perturbing the saved state and watching the spec fail. Evidence names the nightly run | `open` |

### Slice M004/S02 — Preset recall across all 45

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S02.md
**Status:** open

**Definition of Done**

- [ ] Every one of the 45 factory presets is selected in a running Cubase
      instance, and each loads without crashing the host
- [ ] For each preset the spec asserts the lane count and note numbers against
      `site/src/generated/presets.json`, so a preset that loads wrongly fails
      rather than merely not crashing
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW02 | 31 of the 45 factory presets have never been selected inside a DAW. `kWebPresetLaneNames` is initialised sparsely at 14 rows against a `kFactoryPresetCount`-sized extent, and a null entry there once crashed Cubase on preset change; the null guard at the `applyPreset` call site is the only thing between that table and the same crash | `coverage` | `tests/cubase/e2e/`, `plugin/source/webui/web_ui_view.cpp` | A spec iterates every preset index, asserting the host survives and the loaded lanes match `presets.json`; proved by pointing one index at a deliberately malformed entry and watching it fail. Evidence names the nightly run | `open` |

### Slice M004/S03 — Transport motion

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S03.md
**Status:** open

**Definition of Done**

- [ ] The spec locates the transport backwards and forwards mid-playback, loops
      a range, and changes tempo, and asserts the emitted notes at those
      positions match the same positions played linearly
- [ ] The spec has been shown to fail against a lane whose phase is accumulated
      rather than derived from absolute PPQ
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW03 | The timing convention is that envelope and cycle phase derive from absolute PPQ and are never accumulated. Golden tests enforce determinism for linear playback only; nothing proves the property under a host's locate, loop or tempo change, which is the one situation where an accumulator and a derivation diverge | `coverage` | `tests/cubase/e2e/`, `tests/cubase/driver/play_scenario.py` | A spec drives locate, loop and tempo change and compares captured output against the linear capture at the same PPQ positions; proved against a deliberately accumulating lane. Evidence names the nightly run | `open` |

### Slice M004/S04 — Editor lifecycle

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S04.md
**Status:** open

**Definition of Done**

- [ ] The spec opens and closes the plugin editor repeatedly within one session
      and asserts the plugin still responds and still emits notes afterwards
- [ ] The spec has been shown to fail when the WebView does not re-attach
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW04 | `CLAUDE.md` records that some hosts call `setActive()` from the audio thread, so no allocation is permitted there, and that `allocateMessage()`/`sendMessage()` in `process()` is not guaranteed lock-free. Both conventions are documented and neither is exercised by opening and closing Poly's editor in a host | `coverage` | `tests/cubase/e2e/`, `plugin/source/webui/` | A spec cycles the editor open and closed, then asserts continued MIDI output and a responsive bridge; proved by forcing a failed re-attach. Evidence names the nightly run | `open` |

### Slice M004/S05 — Multiple instances

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S05.md
**Status:** open

**Definition of Done**

- [ ] Two Poly instances in one project each hold their own patch and emit their
      own MIDI, with no state or probe output crossing between them
- [ ] The spec has been shown to fail if the two instances share state
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW05 | Nothing exercises two Poly instances in one project. Per-instance isolation of state, probe output and the WebUI bridge is assumed rather than demonstrated, and the probe writes to a path the second instance would also want | `coverage` | `tests/cubase/e2e/`, `plugin/source/` | A spec loads two instances with different patches and asserts each emits its own; proved by pointing both at one state blob and watching the spec fail. Evidence names the nightly run | `open` |

### Slice M004/S06 — Offline bounce equivalence

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S06.md
**Status:** open

**Definition of Done**

- [ ] A bounced or offline-rendered passage matches the realtime capture of the
      same passage, note for note and position for position
- [ ] The spec has been shown to fail when the two diverge
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW06 | Determinism is asserted for the engine's own render, not across Cubase's offline and realtime paths. Offline rendering drives `process()` with different block sizes and a different clock, which is exactly where a block-size dependency would show | `coverage` | `tests/cubase/e2e/`, `tests/cubase/validate_smf_export.py` | A spec renders a fixed passage both ways and compares the captures; proved by perturbing one capture. Evidence names the nightly run | `open` |

### Slice M004/S07 — Host parameter automation

**Validation:** format, cubase-harness
**Evidence:** evidence/M004-S07.md
**Status:** open

**Definition of Done**

- [ ] A host automation lane driving a Poly parameter changes the emitted MIDI
      at the automated positions
- [ ] The spec has been shown to fail when automation is ignored, and when it is
      applied at the wrong position
- [ ] A nightly run is named in the evidence with this spec green

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| DAW07 | VST3 parameter automation from the host is untested. Poly exposes its parameters for automation and the plugin layer feeds them to the engine each block, but nothing drives one from a host's automation lane and checks the output moved when and where it should | `coverage` | `tests/cubase/e2e/`, `plugin/source/` | A spec writes an automation lane, plays it, and asserts the output changes at the automated positions and not before; proved by flattening the lane. Evidence names the nightly run | `open` |

---

## Milestone M005 — Preset pipeline integrity

**Source note.** Like M004, this milestone is not drawn from `deferrals.md`. Its
two rows are defects M001 found in the pipeline it had to use: both were worked
around to finish that milestone, both are recorded in
`evidence/M001-S02.md` and `M001-report.md`, and neither has an issue on the
tracker. They are written here so the workaround does not become the permanent
state.

**Vision:** The path from `engine/src/presets.cpp` to
`site/src/generated/presets.json` tells the truth — it rebuilds when the engine
changes, and it carries enough of a lane that a consumer can tell an authored
pattern from a generated one.

**Branch:** milestone/M005-preset-pipeline
**Status:** done
**Demo:** Edit a preset, run `npm --prefix site run generate-presets` with no
explicit build step, and see the change in the JSON; then ask the JSON alone
whether `Cuban Son Montuno`'s clave is the son clave or `E(5,16)`, and get an
answer.

**Why it has leverage early.** M002 and M003 both edit `presets.cpp`, so both
meet PIPE01 the moment they regenerate, and M002 must bump the emitter's
`schemaVersion` for EC06 regardless — the same change PIPE02 makes. Neither is a
dependency, and none is declared: M002 can be finished by building the emitter
by hand, exactly as M001 was. This is a recommendation about ordering, not a
constraint on it.

### Slice M005/S01 — The generator rebuilds its emitter

**Plan:** M005-S01-plan.md
**Validation:** format, site-unit
**Evidence:** evidence/M005-S01.md
**Status:** done

**Definition of Done**

- [x] Editing `engine/src/presets.cpp` and running the generator produces JSON
      that reflects the edit, with no explicit build step
- [x] A stale `presets.json` fails the site suite mechanically, rather than
      depending on someone noticing the count is wrong
- [x] The generator still succeeds from a clean tree, where the build directory
      does not yet exist
- [x] The hardcoded preset count in `presets-json-schema.test.mjs` is gone,
      derived from `kFactoryPresetCount` instead

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| PIPE01 | `ensureEmitter()` in `site/scripts/generate-presets-json.mjs` returns as soon as the emitter binary exists and rebuilds it only when missing, so a change to `presets.cpp` silently emits stale JSON. The file's own header calls a stale `presets.json` "a silent correctness bug we already paid for", and M001/S02 paid it again: the generator wrote 43 presets after the engine had 44, and reported success | `tooling` | `site/scripts/generate-presets-json.mjs`, `site/tests/presets-json-schema.test.mjs` | The early return is removed so the build target always runs — cmake is incremental, so an unchanged tree costs a no-op. Proved by reproducing the M001 failure: edit a preset, run the generator with no explicit build, and watch the new value appear where it previously did not. The schema test derives its expected count from `kFactoryPresetCount` in `engine/include/poly/presets.h`, so a stale file fails the suite; proved by regenerating against a deliberately stale binary | `done` |

### Slice M005/S02 — `presets.json` carries the pattern

**Plan:** M005-S02-plan.md
**Validation:** format, unit, engine-isolation, site-unit, doc-conformance
**Evidence:** evidence/M005-S02.md
**Status:** done
**Depends:** M005/S01

**Definition of Done**

- [x] A lane running in timeline mode carries its step pattern in
      `site/src/generated/presets.json`
- [x] `schemaVersion` is bumped, and the generator rejects a JSON written at the
      previous version rather than reading it as if the field were absent
- [x] A site test answers, from `presets.json` alone, whether `Cuban Son
      Montuno`'s clave is the son clave or `E(5,16)` — the question M001 could
      not ask of that file
- [x] Every existing consumer of the file still passes

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| PIPE02 | `engine/tools/emit_presets.cpp` (schemaVersion 3) writes `timeline` and `fixedPatternLength` but never `fixedPattern`, so an exact clave and a Euclidean bake of the same hit count and cycle serialise identically. M001/S02 made four presets carry hand-authored patterns and could not express the difference in the file the site reads: the plugin plays the right thing and nothing rendered from `presets.json` can show it | `tooling` | `engine/tools/emit_presets.cpp`, `site/src/generated/presets.json`, `site/tests/` | The emitter serialises the pattern for timeline lanes and bumps `schemaVersion`; the generator's version guard is updated to match. Proved by a site case that derives the onsets from the JSON and asserts they differ from `bjorklund` for the same hit count and cycle — the case fails against the pre-change file, which cannot answer it | `done` |

---

## Milestone M006 — Gate parity

**Source note.** Like M004 and M005, this milestone is not drawn from
`deferrals.md`. Both rows were found while shipping M005: `site-lint` failed on
a check that had run green nowhere locally, and the investigation found a second
gap beside it. `GAP02` has a tracker issue, [#272](https://github.com/JimAKennedy/poly/issues/272),
whose numbers have since drifted; `GAP01` has none.

**Vision:** A developer can run every check CI will run, and every test in the
tree runs somewhere in CI — so a green local gate means something, and a test
file cannot be proven only on the machine that wrote it.

**Branch:** milestone/M006-gate-parity
**Status:** in-progress
**Demo:** Add a doc-drift violation and a new site test on a branch; the local
gate catches the first and CI runs the second, without anyone knowing a special
incantation.

**What this is not.** `scripts/pre-push-check.sh` is not at fault and does not
need to grow. It runs clang-format, RT safety, snippet regions, the build and
the tests, which is exactly what `CLAUDE.md` says it runs. The gap is that one
check CI enforces has no local form at all, and that one directory of tests has
no CI form at all.

### Slice M006/S01 — `doc-drift` is runnable locally

**Plan:** M006-S01-plan.md
**Validation:** format, doc-discipline
**Evidence:** evidence/M006-S01.md
**Status:** done

**Definition of Done**

- [x] A developer can run the `doc-drift` check against the default branch with
      a documented command, without knowing to set an environment variable by
      hand
- [x] The `doc-discipline` token no longer reports success while silently
      skipping a check CI enforces — either it runs `doc-drift`, or a separate
      declared token does and slices that owe it name it
- [x] A run genuinely unable to determine a base still explains why rather than
      failing, so a detached or shallow checkout is not made unusable

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GAP01 | `doc-drift` is enforced in CI and cannot be run locally by any documented command. `jk-standards all` reports `doc-drift: no --base or GITHUB_BASE_REF — skipped` and exits 0, so the `doc-discipline` token passes while the check never executes. M005 ran its whole validation set green and CI still failed on a doc-drift violation, found only by setting `GITHUB_BASE_REF` by hand afterwards; M001 satisfied the same rule incidentally, having changed `docs/preset-taxonomy.md` only because adding presets forced the count updates. A check that passes locally for the wrong reason is worse than one that is absent | `tooling` | `.jk/validations.yml`, `scripts/`, `CLAUDE.md` | Introduce a doc-drift violation on a branch and watch the local command fail; remove it and watch it pass. The skip path is proved separately by running where no base can be determined and reading the explanation | `done` |

### Slice M006/S02 — Every site test runs in CI

**Plan:** M006-S02-plan.md
**Validation:** format, site-unit, doc-conformance
**Evidence:** evidence/M006-S02.md
**Status:** in-progress

**Definition of Done**

- [ ] Every `site/tests/*.test.mjs` file runs in at least one CI job
- [ ] A test file added to that directory cannot silently go unrun — something
      fails if it is covered by nothing
- [ ] #272's counts are corrected to what the tree holds, or the issue is closed
      by this work

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| GAP02 | No CI job runs `npm --prefix site test`. The only `site/tests/**` files CI executes are those named in `scripts/check-doc-conformance.sh`, so 6 of 23 run nowhere: `bjorklund`, `dump-mode`, `preset-patterns`, `presets-json-schema`, `sample-loader`, `smf-writer`. Issue [#272](https://github.com/JimAKennedy/poly/issues/272) raised this as 7 of 21 and the numbers have drifted since. It is not hypothetical: `presets-json-schema.test.mjs` carries M005/S01's staleness guard, so the guard that catches a stale `presets.json` is itself unproven in CI | `tooling` | `.github/workflows/ci.yml`, `scripts/check-doc-conformance.sh` | A CI job runs the files, proved by pushing a branch with a deliberately failing case in one of the six and watching CI go red. The no-orphan guard is proved by adding a file covered by nothing and watching the check fail | `open` |

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

**M004 carries no technical dependency at all.** It is sequenced last because
that is where it was asked for, not because anything blocks it: DAW regression
coverage needs neither kotekan modes nor subdivision profiles, and its seven
slices are independent of each other. If the Cubase runner's reliability becomes
the pressing problem it can be pulled forward whole, or slice by slice, without
disturbing M002 or M003. Recorded explicitly so a later reader does not infer a
dependency from the numbering.

**M006 declares no dependency and blocks nothing.** It is worth pulling forward
anyway, for a reason the graph cannot express: M002, M003 and M004 will each run
their validation sets locally and believe them, and GAP01 means one of those
checks reports success without running. Every milestone after this one is
cheaper to trust once it lands.

**M005 declared no dependency either, and that was deliberate.** Both M002 and
M003 would benefit from it landing first, and the milestone says so in prose —
but a slice-level `Depends` would be a fiction: each can be completed by
building the emitter by hand, which is how M001 finished. The recommendation is
recorded where a reader will see it; the graph stays honest.

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
