---
class: gated
---

# Open-source launch — delivery ledger

**Source:** docs/plans/open-source-launch/vision.md
**Slug:** open-source-launch

The rest of "first release". [`first-release`](../first-release/ledger.md)
decided what ships and is `done`. This programme owns what a stranger meets:
the pipeline's honesty about itself, the CI a contributor inherits, the hosts a
musician loads Poly in, the repository and site a visitor lands on, what Poly
says it is — and how a release is built, signed and installed, absorbed from
the installers vision on 2026-09-23 (M006–M009).

## Reconciliation, before any structure

Read the vision: 30 actionable items across eight sections, plus 7 owner
decisions.

| | |
|---|---|
| Already satisfied | 1 — OS18: issue creation is open to all users, checked in the repository's settings on 2026-09-23; the review's observation was not reproduced |
| Outstanding | 29 |
| Deferred and recorded | 6 — see Out of scope |

**Folded in on 2026-09-23:** the installers vision's 12 actionable items became
M006–M009 (OS31–OS42), so the two questions that ledger once "inherited" — the
first version number (OS02) and whether the AU ships (OS17) — are now plain
in-ledger dependencies, and the four items it "owned" are rows here. Its five
out-of-scope items joined this ledger's. Its signing premises were corrected
first: EV no longer buys SmartScreen reputation, OV keys need a token or HSM a
hosted runner cannot hold, and a signed `.pkg` needs a Developer ID Installer
certificate the pipeline does not know about. The owner decided the routes
before any row was written: **Azure Artifact Signing for Windows, the Apple
Developer Program for macOS.**

Every count below was measured on `main` at `412020d`, not carried over from
the vision.

### What the measurement changed

The review behind the vision proposed a `docs/` reorganisation, moving delivery
records under an internal directory. **Declined** and recorded as row OS25's
shape: `docs/plans/` is named by `jk-standards.yaml` (both `exempt_dirs` and
`ledger.roots`), by `.github/docs-drift-map.yml`, and by test fixtures, so a
move is a many-file edit that buys only a tidier listing. An index that says
which documents a contributor should read gets the benefit at none of the cost.

## Milestone M001 — The release describes itself

**Vision:** The version a DAW reports, the version the tag names and the notes
the Release publishes are the same thing, and the release tests what it ships.

**Branch:** milestone/M001-release-honesty
**Status:** in-progress
**Demo:** A dry-run of `release.yml` on a scratch tag builds, runs `ctest` on the
universal binary, and publishes zips whose plugin reports the tag's version,
beside a `SHA256SUMS` file and a provenance attestation.

**Why this lands before M006.** M006 cuts the first real pre-release and
records what a stranger experiences. If it runs against today's pipeline it
will record a plugin reporting `1.0.0` from a `0.1.0` build, with June's release
notes, and every finding will be one this milestone could have prevented.

### Slice M001/S01 — One version, from one place

**Plan:** M001-S01-plan.md
**Validation:** format, unit, guards, doc-discipline
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] `project(poly VERSION …)` is the only hand-edited version in the tree that
      reaches a shipped artifact
- [x] The plugin's factory class info reports that version, proved by changing
      it and reading the new value back from a built bundle
- [x] A check fails if a hand-typed version string returns to `plugids.h`, seen
      red before being trusted
- [x] The first version number is decided and recorded in this milestone's
      decisions file, and the CHANGELOG section `gen-release-notes.mjs` will
      extract for it describes the current tree

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS01 | Five version strings disagree: CMake `0.1.0`, `plugids.h:15` `kPolyVersionString` **`1.0.0`** (what a DAW displays), `webui/package.json` `0.1.0`, `site/package.json` `0.0.1`, and the AU plist's `0xFFFFFFFF` development sentinel | `defect` | `CMakeLists.txt`, `plugin/CMakeLists.txt`, `plugin/source/plugids.h` | `kPolyVersionString` is generated from `PROJECT_VERSION` via `configure_file`; a built bundle's class info reports the CMake version; the two npm manifests are marked private-and-unversioned or read from the same source. The AU sentinel is OS17's, because it only matters if the AU ships | `done` |
| OS02 | `CHANGELOG.md:102` holds `## [0.1.0] - 2026-06-27`, headed "Initial open-source release", but nothing was ever tagged from it. `release.yml` extracts the section matching the tag, so tagging `v0.1.0` today publishes that 397-word section and none of the 47 `[Unreleased]` entries | `defect` | `CHANGELOG.md`, `docs/plans/open-source-launch/M001-decisions.md` | The owner's decision on the first version number is recorded; `node scripts/gen-release-notes.mjs <version>` prints a section describing the current tree. M006/S01 names its tag from this decision rather than taking its own | `done` |

### Slice M001/S02 — The release proves what it ships

**Plan:** M001-S02-plan.md
**Validation:** format, guards
**Evidence:** evidence/M001-S02.md
**Status:** done

**Definition of Done**

- [x] The release build runs the unit and golden tests on the exact
      configuration it packages, before packaging
- [x] Every Release carries a `SHA256SUMS` asset and a build-provenance
      attestation for each zip
- [x] Every pluginval download in every workflow is verified against a pinned
      digest before it runs
- [x] `scripts/check-release-workflow.mjs` asserts all three, and each new
      assertion is seen red by removing what it guards

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS03 | `release.yml` runs build → validator → pluginval → sign → package and never `ctest`. The macOS **universal** binary is built nowhere else — `ci.yml` builds arm64 only — so the one configuration that ships is the one whose tests never run | `pipeline` | `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs` | A `ctest` step runs after Build and before packaging on both legs; the contract check fails when it is removed or moved after packaging | `done` |
| OS04 | A Release publishes zips and nothing a downloader can verify them against — no checksums, no provenance | `pipeline` | `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs` | The publish job attaches `SHA256SUMS`; each zip carries an `actions/attest-build-provenance` attestation that `gh attestation verify` accepts; the contract check asserts both | `done` |
| OS05 | pluginval is fetched with `curl` and executed unverified in four places: `ci.yml:434`, `:480`, `release.yml:98`, `:118`. Every action in the tree is SHA-pinned; the binary those jobs execute is not | `tooling` | `.github/workflows/ci.yml`, `.github/workflows/release.yml` | Each download is followed by a `sha256sum -c` (or `shasum -a 256 -c`) against a digest pinned beside the version; a tampered digest fails the step | `done` |

## Milestone M002 — CI a stranger can trust

**Vision:** A contributor's first PR runs in workflows that hold least privilege,
execute no unpinned third-party code, fuzz the inputs strangers control, and fail
on a warning.

**Branch:** milestone/M002-ci-trust
**Status:** planned
**Demo:** Every workflow declares top-level permissions, a superseded PR push
cancels its predecessor, the nightly fuzzes both untrusted inputs, and a new
engine warning fails CI.

### Slice M002/S01 — Workflows hold least privilege

**Validation:** format, guards
**Evidence:** evidence/M002-S01.md
**Status:** open

**Definition of Done**

- [ ] Every workflow declares a top-level `permissions:` block, and jobs elevate
      locally only where they write
- [ ] A superseded push to a PR cancels the run it replaces
- [ ] No workflow checks out or executes third-party code at a moving ref
- [ ] A guard fails on a workflow with no top-level `permissions:` and on an
      unpinned third-party checkout, each seen red — the three jk-standards
      workflow checks all pass on today's tree, so none of them is that guard

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS06 | `ci.yml` has no top-level `permissions:` block; only `secrets-scan` declares one. Every other job's `GITHUB_TOKEN` falls back to the repository default, which the tree cannot see. `jk-standards workflow-permissions` passes today because it checks only reusable-workflow calls against the caller's grant | `tooling` | `.github/workflows/ci.yml`, the jk-standards check or `scripts/` | Top-level `contents: read`; `secrets-scan` keeps its local `pull-requests: write`; the guard — preferably the jk-standards check extended upstream, since every portfolio repo has the same gap — fails on a workflow without the block | `open` |
| OS07 | `ci.yml` declares no `concurrency:` group, so every push to an open PR runs the full macOS and Windows matrix to completion even after a newer push supersedes it. `jk-standards workflow-concurrency` passes because it checks that declared groups are ref-scoped, not that a group exists | `tooling` | `.github/workflows/ci.yml` | A `concurrency` group keyed on workflow and ref with `cancel-in-progress` for pull requests only; pushes to `main` are never cancelled | `open` |
| OS08 | `pr-af-review.yml` checks out `Agent-Field/pr-af` with no `ref:` and runs `docker compose up` with `OPENROUTER_API_KEY` in its environment — whatever that repository's default branch holds today runs with the key. `jk-standards action-pinning` passes because it inspects `uses:` references, and a checkout's `repository:` input is invisible to it | `defect` | `.github/workflows/pr-af-review.yml` | The checkout names a 40-character SHA with the upstream tag in a comment, matching the pinning convention used for every action; the guard fails on a third-party `repository:` checkout without one | `open` |

### Slice M002/S02 — The untrusted inputs are fuzzed

**Validation:** format, engine-isolation, sanitizers
**Evidence:** evidence/M002-S02.md
**Status:** open

**Definition of Done**

- [ ] The fuzz option is declared, documented, and built by a workflow
- [ ] Both untrusted inputs `SECURITY.md` names — saved state and a dropped MIDI
      file — have a fuzz target with a seed corpus
- [ ] The nightly runs each for a bounded time and files an issue on a crash,
      the same way the sanitizer nightly does
- [ ] Each target is shown to find a bug planted on a scratch branch before it
      is trusted

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS09 | `tests/CMakeLists.txt:62` builds `fuzz_state_io` under `if(BUILD_FUZZ_TESTS)` — an option declared nowhere and referenced by no workflow or script, so the fuzzer runs on no machine | `coverage` | `CMakeLists.txt`, `tests/CMakeLists.txt`, `.github/workflows/sanitizers.yml` | `option(BUILD_FUZZ_TESTS …)` is declared beside the sanitizer options; the nightly builds it with Clang and runs it for a fixed duration; a planted out-of-bounds read in `state_io` is found | `open` |
| OS10 | The MIDI reader parses any file a user drops on a lane, and `SECURITY.md` names MIDI parsing as in scope, but it has no fuzz target | `coverage` | `tests/fuzz/`, `tests/CMakeLists.txt`, `.github/workflows/sanitizers.yml` | A `fuzz_midi_reader` target seeded from the repository's own SMF fixtures runs nightly beside OS09's; a planted bounds error in `midi_reader.cpp` is found | `open` |

### Slice M002/S03 — A warning means something

**Validation:** format, engine-isolation, unit
**Evidence:** evidence/M002-S03.md
**Status:** open

**Definition of Done**

- [ ] `poly_engine` builds warning-free on GCC, Clang and MSVC with
      `POLY_WARNINGS_FATAL=ON`, and CI builds it that way
- [ ] Every test-side warning that marked a real defect is fixed, not silenced
- [ ] MIDI import's handling of pitch is a decision with a test naming it

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS11 | `POLY_WARNINGS_FATAL` defaults `OFF` as a phase-one measure. An engine-only GCC 13 build reports **69 unique warning sites** in engine sources — mostly `-Wsign-conversion` from `types.h` (255 diagnostics as included) and `scene.h` (85) — plus `-Wdouble-promotion` in `rng.h`. The CMake comment's "~242" counts engine and tests together | `tooling` | `engine/`, `CMakeLists.txt`, `.github/workflows/ci.yml` | The `engine-isolation` CI job configures with `-DPOLY_WARNINGS_FATAL=ON`; a deliberately introduced sign conversion fails it. Tests are a later phase, recorded in the decisions file | `open` |
| OS12 | `tests/golden_tests.cpp:548` computes `bothSilentSomewhere` across six bars and never asserts on it, so half the test's stated intent — "their gaps should not always overlap" — is unchecked. By inspection the flag is never set for this patch, so the missing assertion may be `EXPECT_FALSE`, or the test's premise may be wrong | `defect` | `tests/golden_tests.cpp` | The flag is asserted with the polarity the test's intent requires, seen red by perturbing a phrase gap, or removed with the comment corrected | `open` |
| OS13 | `engine/src/midi_reader.cpp:203` reads `d1` — the note number — and discards it (`-Wunused-but-set-variable`). Every note-on's onset is kept regardless of pitch, so a multi-instrument drum file dropped on one lane merges every instrument's onsets into it | `defect` | `engine/src/midi_reader.cpp`, `tests/midi_reader_tests.cpp` | The owner decides between filtering by note (the lane's own, or a chosen one) and merging on purpose; either way a test drops a two-instrument file and asserts the chosen outcome, and the warning is gone | `open` |

## Milestone M003 — A musician's DAW finds it and it plays

**Vision:** Every host the release claims is one somebody has loaded Poly in,
routed its MIDI, and heard; the editor is exercised on both shipping platforms;
Logic is supported or declined on purpose.

**Branch:** milestone/M003-hosts
**Status:** planned
**Demo:** The README's host table names each supported DAW with its routing
steps, each backed by an evidence entry, and pluginval runs its GUI tests on
both platforms.

**Why this is measured by hand.** Only Cubase can be driven in CI, through the
self-hosted nightly. The other hosts are a one-time manual run recorded in
evidence — the same honesty M006/S01 asks of the first download.

### Slice M003/S01 — Hosts are measured, not assumed

**Validation:** format, doc-discipline, doc-conformance, site-unit
**Evidence:** evidence/M003-S01.md
**Status:** open

**Definition of Done**

- [ ] Each candidate host has been loaded, routed and played on at least one
      shipping OS, with the host version and the exact routing steps recorded
- [ ] The README and guide chapter 17 carry the same host table, and it
      distinguishes "supported" from "reported to work"
- [ ] The default channel layout is decided against what those hosts did

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS14 | `README.md:151` says Poly "should work with any VST3-compatible host". Routing MIDI **out** of a plugin is where hosts differ most — Ableton Live needs a second track whose *MIDI From* selects the plugin — and only Cubase is exercised. Candidates: Reaper, Bitwig, Ableton Live 12, FL Studio, Studio One | `verify` | `README.md`, `site/src/content/docs/17-midi-routing-note-map.mdx`, `evidence/M003-S01.md` | Evidence records host, version, OS, routing steps and outcome for each; the README and chapter 17 name only hosts with an evidence entry as supported | `open` |
| OS15 | `types.h:268` defaults `midiChannel` to `-1` — auto, one channel per lane — so a single-channel drum sampler hears one lane until the user finds the Note Map | `verify` | `engine/include/poly/types.h`, `engine/src/presets.cpp` | Decided from OS14's runs and recorded; if the default changes, a state-migration test proves saved projects keep their channels | `open` |

### Slice M003/S02 — The editor is exercised on both platforms

**Validation:** format, unit, guards
**Evidence:** evidence/M003-S02.md
**Status:** open

**Definition of Done**

- [ ] pluginval runs its GUI tests on the macOS CI leg, or the evidence records
      why a hosted runner cannot and what covers the editor instead
- [ ] The same is true of the Windows leg
- [ ] The release workflow matches whatever CI settles on

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS16 | pluginval runs with `--skip-gui-tests` on all four legs (`ci.yml:441`, `:486`, `release.yml:103`, `:122`). The editor is a WebView — the likeliest source of a host crash on open, close and reopen — and only the Windows Cubase nightly ever opens it; nothing automated opens it on macOS | `coverage` | `.github/workflows/ci.yml`, `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs` | The flag is removed on each leg that can run GUI tests, proved by a green run; any leg that keeps it carries a comment naming the reason and the evidence entry | `open` |

### Slice M003/S03 — Logic is supported or declined on purpose

**Validation:** format, doc-discipline, guards
**Evidence:** evidence/M003-S03.md
**Status:** open

**Definition of Done**

- [ ] The owner's decision is recorded in this milestone's decisions file and
      cited by M007/S01
- [ ] If supported: an `aumi` unit passes `auval`, drives an instrument track in
      Logic (evidence), carries the real version, and the release builds and
      ships it
- [ ] If declined: the README and guide say Logic is not supported, and
      `build-au-macos` carries a comment saying it exists for validation only

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS17 | `plugin/resource/au-info.plist` declares the AU as type `aumu` with version `0xFFFFFFFF`. `build-au-macos` builds it on every PR and no release contains it. Logic has no VST3 and routes generated MIDI only from a MIDI FX (`aumi`) unit, so as built it could not drive another instrument there even if it shipped. **Feasibility comes before the decision:** the VST3 SDK's AUv2 wrapper builds instruments and effects, and whether it can produce an `aumi` at all is unverified — if it cannot, "supported" means writing a wrapper, and the honest decision may be forced. M007 (the macOS installer) waits on this row | `disclose` | `plugin/resource/au-info.plist`, `plugin/CMakeLists.txt`, `.github/workflows/ci.yml`, `README.md` | The feasibility check is recorded first; then either arm of the DoD, not both; M007/S01 cites the decision rather than re-deciding it | `open` |

## Milestone M004 — A stranger can find it, file against it, and follow it

**Vision:** A visitor can tell in one sentence what Poly is, try it in the
browser, download it, set it up in their DAW, and report a problem — all from
pages written for them.

**Branch:** milestone/M004-front-door
**Status:** planned
**Demo:** A non-collaborator opens an issue from the bug template; the README
reads positioning → screenshot → try it → download → DAW setup before any
build instruction; the site's hero offers the download.

**Why the README waits for M003/S01.** Its DAW-setup section is the host table
M003/S01 measures. Written first, it would restate the claim that milestone
exists to replace.

### Slice M004/S01 — The repository accepts a stranger

**Validation:** format, doc-discipline
**Evidence:** evidence/M004-S01.md
**Status:** open

**Definition of Done**

- [ ] A non-collaborator account can open an issue from each template
- [ ] The About panel carries a description, the site URL and topics
- [ ] `ROADMAP.md` links to issues by label and milestone rather than
      enumerating numbers, so closing an issue cannot make it stale

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS18 | The review reported that GitHub shows a visitor "Issue creation is restricted in this repository". **Not reproduced.** On 2026-09-23 the owner read the repository's issue-creation setting as "All users"; the API shows issues enabled, no interaction limits, and no human non-collaborator has ever filed one (all 17 non-owner issues are the Actions app's nightly reports), so nothing contradicts the setting. A logged-out browser greys the button on every repository and proves nothing. Closed as already satisfied; if a signed-in non-collaborator ever meets the message, reopen | `defect` | repository settings | The setting reads "All users", recorded here; no further proof available without a second account | `done` |
| OS19 | The About panel has no description, website or topics, so the repository appears under neither `euclidean-rhythm` nor `vst3` | `docs` | repository settings | The description is OS21's sentence; the website is `poly.jk.digital`; topics include `vst3`, `midi`, `euclidean-rhythm`, `polyrhythm`, `drum-machine`, `audio-plugin` | `open` |
| OS20 | `ROADMAP.md` lists #172, #142, #89 and #111 — all closed — while the open issues are #320, #305, #282, #266 and #100; its Priority 3 table is empty. Enumerated issue numbers drift by construction | `docs` | `ROADMAP.md` | The roadmap names themes and links label and milestone queries; no `#NNN` issue reference remains, so there is nothing for a closure to invalidate | `open` |

### Slice M004/S02 — One sentence says what Poly is

**Validation:** format, doc-discipline, site-unit
**Evidence:** evidence/M004-S02.md
**Status:** open

**Definition of Done**

- [ ] The owner has chosen a one-sentence positioning statement and recorded it
      with the reasoning
- [ ] The README's opening, the site's meta description, the About description
      and `CLAUDE.md` carry the same statement

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS21 | The repository says "polymetric drum pattern generator"; the launch intent says "open-source Euclidean sequencer". Free Euclidean sequencers are a crowded category — HY-RPE2's free tier, HY-ESG, XiiixxiQ, the open-source GenerativeMIDI, Ableton Live 12's built-in generator — and Poly loses a checklist against them. Its difference is tradition-grounded grooves with a cited guide to 45 presets, deterministic output, and an engine that runs in the browser | `docs` | `README.md`, `site/astro.config.mjs`, `CLAUDE.md`, `docs/plans/open-source-launch/M004-decisions.md` | The chosen sentence appears verbatim in all four surfaces and the About panel; the decisions file records the alternatives considered | `open` |

### Slice M004/S03 — The README is written for the person downloading

**Depends:** M003/S01, M004/S02
**Validation:** format, doc-discipline, doc-conformance, guards
**Evidence:** evidence/M004-S03.md
**Status:** open

**Definition of Done**

- [ ] The README's order is: positioning, screenshot, try it in the browser,
      download, DAW setup — then contributing and building
- [ ] Signing-secret provisioning moves to a maintainer document the README
      links to
- [ ] No internal decision or milestone ID appears in `README.md` or
      `CONTRIBUTING.md`, and a guard fails if one returns
- [ ] A contributor can tell from one index which documents under `docs/` are
      for them
- [ ] The two stale artefacts are gone

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS22 | `README.md` puts Building (line 26) before Installing (line 67), gives signing-secret provisioning its own top-level section (line 105), has no screenshot, never mentions the in-browser engine, and carries six internal IDs: `D029` ×2, `M054`, `D031`, `M030 S03`, `D004` (lines 63, 77, 112). `CONTRIBUTING.md:16` carries two more | `docs` | `README.md`, `CONTRIBUTING.md`, `RELEASING.md`, `scripts/` | The section order matches the DoD; a guard matching `\bD0\d\d\b` and `\bM0\d\d\b` over both files fails when one is reintroduced, seen red; `RELEASING.md` holds the secrets table unchanged | `open` |
| OS23 | `CLAUDE.md:18` lists "VSTGUI 4" in the tech stack; `CMakeLists.txt` has turned SDK VSTGUI support off since M053/S05 and the editor is a choc WebView | `docs` | `CLAUDE.md` | The line names the WebView editor; no VSTGUI reference remains that describes current state | `open` |
| OS24 | `.bg-shell/manifest.json` is tracked (content `[]`, added in `0d383fc`) although `.gitignore:322` ignores `.bg-shell/` — it predates the ignore rule | `tooling` | `.bg-shell/manifest.json` | `git rm --cached`; `git ls-files .bg-shell` is empty and the pre-commit excludes for it can be dropped | `open` |
| OS25 | `docs/` holds 247 files and roughly 345k words, most of them delivery records. That is right for how this repository works, but nothing tells a contributor which few documents to read. **A move is declined** — `docs/plans/` is named by `jk-standards.yaml`, the drift map and test fixtures | `docs` | `docs/README.md` | An index classifies `docs/` into user, contributor, and delivery-record documents, and the README's Contributing section links it; `doc-discipline` passes | `open` |

### Slice M004/S04 — The site sends readers to the download

**Depends:** M004/S02
**Validation:** format, site-unit, doc-conformance, guards
**Evidence:** evidence/M004-S04.md
**Status:** open

**Definition of Done**

- [ ] No page carries the construction banner
- [ ] The home page's hero offers the download and the in-browser engine beside
      the guide
- [ ] The guide's install section names the bundle a release actually contains

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS26 | `site/src/components/Banner.astro:4` shows "🚧 Under active construction — content being developed and verified" on every page of a guide whose bibliography, screenshots and chapters first-release M001–M004 just verified | `docs` | `site/src/components/Banner.astro`, `site/astro.config.mjs` | The banner is removed or replaced by a release notice; a site test asserts the construction text is absent | `open` |
| OS27 | The hero in `site/src/content/docs/index.mdx` offers *Start Reading* and *GitHub*. There is no download, and the in-browser engine — the one thing no competitor has — is not on the front page | `docs` | `site/src/content/docs/index.mdx` | The hero's actions are download (the Releases page), try it, and read the guide; `guards` passes its site-asset checks | `open` |
| OS41 | `guide-using-poly.mdx`'s install section names the VST3 bundle `Poly.vst3` when the artifact is `poly_plugin.vst3`, and tells the reader to copy `Poly.component` into their Components folder when no release contains an AU. First-release M004 matched the guide to the plugin UI, not to the release artifacts | `defect` | `site/src/content/docs/guide-using-poly.mdx` | The section describes the zip as it ships — the real bundle name, and the AU only if OS17 says it ships; a claim test forbids `Poly.vst3` returning. M009/S01 rewrites the same section for the installers | `open` |

## Milestone M005 — The release notes are for musicians

**Vision:** The first thing a musician reads about a release is short, says
what Poly does, which hosts it supports and what is known to be wrong — and the
launch is listed where musicians look.

**Branch:** milestone/M005-release-notes
**Status:** planned
**Demo:** The Release body for the first version is a few hundred words a
non-developer can read, and the engineering history is one link away.

### Slice M005/S01 — A release body a musician reads

**Depends:** M001/S01, M003/S01
**Validation:** format, doc-discipline, guards
**Evidence:** evidence/M005-S01.md
**Status:** open

**Definition of Done**

- [ ] The owner has decided where musician-facing notes live
- [ ] The Release body for the first version names what Poly does, the
      supported hosts, the known issues, and how to report one
- [ ] The engineering narrative is kept, reachable from the notes, and not the
      Release body
- [ ] `check-release-workflow.mjs` asserts the body's source

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS28 | `[Unreleased]` is 47 entries and about 13,100 words of engineering narrative — accurate, and written for the next maintainer. `release.yml` publishes the matching section verbatim as the Release body, so the first thing a musician reads would be a paragraph about `lockPresetReferent` | `pipeline` | `CHANGELOG.md` or a notes file, `scripts/gen-release-notes.mjs`, `scripts/check-release-workflow.mjs` | `gen-release-notes.mjs <version>` prints the musician-facing section; a test fails if it exceeds a word ceiling the decision sets; the engineering entries survive unchanged | `open` |
| OS29 | Nothing in a Release tells a user what is known to be wrong or which hosts are supported — the two things that decide whether a broken setup is their mistake or Poly's | `docs` | the notes source from OS28 | The first Release body carries a Supported hosts list taken from OS14's evidence and a Known issues list linking open issues | `open` |

### Slice M005/S02 — The launch is listed where musicians look

**Depends:** M005/S01, M006/S01
**Validation:** format
**Evidence:** evidence/M005-S02.md
**Status:** open

**This slice has no gate beyond its evidence.** Its deliverables are listings
on sites this repository does not control, so no token can check them; the
evidence file's URLs and dates are the proof, and the reviewer reads them. Said
here so nobody mistakes `format` for a check of the work.

**Definition of Done**

- [ ] A product page exists on KVR Audio linking the Release and the site
- [ ] The release has been submitted to at least two outlets that cover free
      rhythm plugins
- [ ] Each listing uses OS21's sentence

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS30 | A GitHub release is invisible to the people it is for. Free Euclidean and rhythm plugins reach musicians through KVR Audio listings and outlets such as Rekkerd and Bedroom Producers Blog, which cover exactly this category | `docs` | external listings, `evidence/M005-S02.md` | Evidence records each listing's URL and the date submitted; waits on M006/S01's first published Release | `open` |

## Milestone M006 — A release is cut, and says whether it is signed

**Vision:** The pipeline has run to completion against a real tag, what a
stranger meets on each platform is recorded verbatim, and an unsigned build
cannot ship without that being a deliberate, recorded choice.

**Branch:** milestone/M006-first-cut
**Status:** planned
**Demo:** A pre-release tag publishes a Release marked pre-release, with both
zips, checksums and provenance; the Release body says whether the artifacts are
signed; and the evidence records every dialog a clean machine showed.

**Why this waits for M001.** The cheapest way to learn what the release path
produces is to cut one, but cutting one today would record a plugin reporting
`1.0.0` from a `0.1.0` build with June's notes — findings M001 exists to
prevent. Cut the first tag against the hardened pipeline.

### Slice M006/S01 — The first pre-release exists

**Depends:** M001/S01, M001/S02
**Validation:** format, guards, doc-discipline
**Evidence:** evidence/M006-S01.md
**Status:** open

**Definition of Done**

- [ ] A tag with a pre-release suffix publishes a Release marked pre-release,
      and the contract check asserts the mapping
- [ ] Both zips were downloaded and installed by hand on a clean machine per
      platform, and the evidence records the host, OS version and every warning
      dialog verbatim
- [ ] Anything the first cut broke is a row here or an issue, not a note

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS31 | No release has ever been cut: one tag, `v0.1.0-doccov-baseline`, pointing at a July commit, and an empty Release list. The pipeline has never produced an artifact a stranger downloaded, so any plan that assumes it works because it exists is assuming | `verify` | `.github/workflows/release.yml`, `evidence/M006-S01.md` | A real pre-release tag runs the workflow to completion; the evidence names the run, the assets and their sizes | `open` |
| OS32 | `release.yml` publishes every `v*.*.*` tag as a full release: `softprops/action-gh-release` is called without `prerelease:`, so a release-candidate tag lands on the Releases page as the latest stable version | `defect` | `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs` | A tag carrying a hyphen suffix (`v0.2.0-rc.1`) publishes with `prerelease: true`; the contract check asserts it and is seen red when the flag is removed | `open` |
| OS33 | What a stranger experiences on a clean machine — the quarantine prompt, SmartScreen, the folder hunt — has never been observed, only described from the README. The zip path stays the fallback for anyone who declines an installer, so its experience is recorded even after M007 and M008 land | `verify` | `evidence/M006-S01.md` | The evidence records the install by hand on one clean macOS and one clean Windows machine, dialog text verbatim, and whether the DAW found the plugin on the next scan | `open` |

### Slice M006/S02 — An unsigned build cannot ship quietly

**Depends:** M006/S01
**Validation:** format, guards
**Evidence:** evidence/M006-S02.md
**Status:** open

**Definition of Done**

- [ ] With no signing secrets, the release job fails, unless a repository
      variable named for the purpose explicitly allows an unsigned release
- [ ] The Release body states whether each artifact is signed and notarized
- [ ] The contract check asserts both, each seen red

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS34 | The codesign, notarize and staple steps gate on `env.MACOS_… != ''` and **skip** when the secrets are absent — which they are: the repository's only secret is an API key. The build succeeds and the workflow's output does not distinguish "signed" from "did not sign". Windows has no signing step at all, so nothing there can even skip | `defect` | `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs`, `RELEASING.md` | A step after signing fails the leg when the artifact is unsigned and `ALLOW_UNSIGNED_RELEASE` is not `true`; the generated body carries a "Signed: yes/no" line per platform; the contract check asserts both | `open` |

## Milestone M007 — macOS installs without a terminal

**Vision:** A macOS user downloads one `.pkg`, opens it, answers the
installer's questions, and their DAW finds Poly on the next scan — no folder
conventions, no quarantine flag.

**Branch:** milestone/M007-macos-pkg
**Status:** planned
**Demo:** A signed, notarized, stapled `.pkg` on the Releases page opens with
no warning on a clean Mac and installs the VST3 (and the AU, if OS17 says it
ships) where the DAW looks, with a per-user option.

**The half that needs no purchase lands first.** S01 builds and tests the
package unsigned; S02 signs it once the Apple Developer Program enrolment and
the two certificates exist. The split is deliberate: the packaging can finish
while the paperwork is pending.

### Slice M007/S01 — A package is built and tested unsigned

**Depends:** M006/S02, M003/S03
**Validation:** format, guards
**Evidence:** evidence/M007-S01.md
**Status:** open

**Definition of Done**

- [ ] The release workflow builds a `.pkg` that installs the VST3 to
      `/Library/Audio/Plug-Ins/VST3/`, or per-user on request, and the AU to
      `Components/` only if OS17 decided it ships
- [ ] Installing on a clean Mac, then removing, leaves the plug-in folders as
      they were, and the evidence records both
- [ ] The contract check asserts the package step's position and inputs

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS35 | The macOS artifact is a zip the user must unpack and place by hand. A `.pkg` built with `pkgbuild`/`productbuild` — the one format Gatekeeper and notarization treat as first-class — places the bundle where the DAW looks and offers the per-user choice through the installer's own UI | `pipeline` | `.github/workflows/release.yml`, `scripts/packaging/`, `scripts/check-release-workflow.mjs` | The pkg installs and uninstalls cleanly on a clean machine (evidence); the release publishes it beside the zip; if the AU ships, the release builds it universal, since `build-au-macos` is arm64-only today | `open` |

### Slice M007/S02 — The package is signed, notarized and stapled

**Depends:** M007/S01
**Validation:** format, guards, doc-discipline
**Evidence:** evidence/M007-S02.md
**Status:** open

**Definition of Done**

- [ ] The bundle is signed with Developer ID Application and the package with
      Developer ID Installer, the package is notarized and the ticket stapled
- [ ] The seven secrets are documented in `RELEASING.md` and provisioned
- [ ] The package opens on a clean Mac with no warning and no `xattr`, recorded
      in the evidence with the macOS version

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS36 | The Apple Developer Program (USD 99 a year) is not enrolled, so no macOS certificate exists. A signed `.pkg` needs a **Developer ID Installer** certificate as well as the Developer ID Application certificate `README.md` documents — a second certificate and a seventh secret the workflow and the README do not know about | `pipeline` | `.github/workflows/release.yml`, `RELEASING.md`, repository secrets | `productsign` with the Installer identity runs after `pkgbuild`; `notarytool submit --wait` and `stapler` run on the pkg; the contract check asserts the order; the evidence records `spctl --assess` accepting the package on a clean machine | `open` |

## Milestone M008 — Windows installs without a warning

**Vision:** A Windows user downloads one installer, runs it, and SmartScreen
attributes it to a named publisher rather than warning against it.

**Branch:** milestone/M008-windows-installer
**Status:** planned
**Demo:** A signed installer on the Releases page places the VST3 in
`C:\Program Files\Common Files\VST3\`, uninstalls from Apps & features, and
carries a publisher name in its SmartScreen dialog.

**Independent of M007:** different platform, different signing service,
different installer tooling, no shared code.

### Slice M008/S01 — An installer is built and tested unsigned

**Depends:** M006/S02
**Validation:** format, guards
**Evidence:** evidence/M008-S01.md
**Status:** open

**Definition of Done**

- [ ] The installer format is chosen on which one signs and uninstalls cleanly
      in CI, with the alternatives and the reason in this milestone's decisions
      file
- [ ] The release workflow builds it; installing on a clean machine places the
      bundle where the DAW looks, and uninstalling removes it
- [ ] The contract check asserts the package step's position and inputs

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS37 | The Windows artifact is a zip. The installer format — WiX/MSI, Inno Setup, NSIS — is a genuine choice to be made on CI signing and clean uninstall, not on taste; whichever is chosen must produce a single artifact `signtool` can sign in one step | `pipeline` | `.github/workflows/release.yml`, `scripts/packaging/`, `docs/plans/open-source-launch/M008-decisions.md` | The decision is recorded with what was tried; the installer installs and uninstalls cleanly on a clean machine (evidence); the release publishes it beside the zip | `open` |

### Slice M008/S02 — The installer is signed through Azure Artifact Signing

**Depends:** M008/S01
**Validation:** format, guards, doc-discipline
**Evidence:** evidence/M008-S02.md
**Status:** open

**Definition of Done**

- [ ] An Azure Artifact Signing account exists, identity validation has passed,
      and the signing action is SHA-pinned like every other
- [ ] The installer and the plugin binary inside it carry a valid Authenticode
      signature naming the publisher, verified with `signtool verify /pa` in
      the workflow
- [ ] The release notes say that first downloads may still show SmartScreen
      until reputation accumulates

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS38 | Windows has no signing at all: no `signtool` step, no certificate, no gate. An unsigned installer meets SmartScreen's "Windows protected your PC", whose default is *Don't run*. **Decided 2026-09-23: Azure Artifact Signing** — about USD 9.99 a month, no hardware token, GitHub Actions integration, individuals eligible in the USA and Canada | `pipeline` | `.github/workflows/release.yml`, `RELEASING.md`, repository secrets | The signing action runs after packaging and before publish; `signtool verify /pa` passes in the workflow; the contract check asserts the order and the pin | `open` |
| OS39 | Whichever route signs, SmartScreen reputation builds per publisher over releases, so early downloads warn anyway. A release that promises a warning-free first download would be wrong | `disclose` | the release-notes source (OS28), `README.md` | The notes and the README's install section carry the sentence; the evidence records what the first signed download actually showed | `open` |
| OS42 | An EV certificate, which the installers vision weighed for its instant SmartScreen reputation, is **declined**: Microsoft removed that behaviour in 2024 and EV now builds reputation like OV at three times the price with a hardware token a hosted runner cannot hold. OV from a CA is declined for the same token reason. SignPath Foundation's free open-source signing was considered and not taken, since the owner chose a route with a standing account; it stays the fallback if Azure's identity validation fails | `docs` | `docs/plans/open-source-launch/M008-decisions.md` | Recorded so a later pass does not reopen the EV question | `accepted` |

## Milestone M009 — The instructions match the artifacts

**Vision:** Someone who reads the install instructions finds the installers the
Releases page offers, and nobody is told to use the terminal.

**Branch:** milestone/M009-install-docs
**Status:** planned
**Demo:** The README's install section is a few sentences and a link, the
Gatekeeper workaround is gone, and the guide's install section describes the
same installers.

**Why it waits for M004/S03.** That slice reorders the README around the
downloader against the zip that ships today; this milestone replaces its
install section once the installers exist, so the README is right at every
point between.

### Slice M009/S01 — The README and guide describe the installers

**Depends:** M007/S02, M008/S02, M004/S03
**Validation:** format, doc-discipline, doc-conformance, site-unit
**Evidence:** evidence/M009-S01.md
**Status:** open

**Definition of Done**

- [ ] The README's install section names one artifact per platform and links
      the Releases page, in a few sentences
- [ ] The macOS Gatekeeper section is deleted, and a guard fails if `xattr`
      returns to the README or the guide
- [ ] The guide's install section and chapter 17 describe the same installers,
      and the zip as the fallback

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| OS40 | `README.md` lines 67–104 teach the zip, the folder conventions and `xattr -dr com.apple.quarantine`. Once an installer exists, documentation that still describes the zip is worse than none | `docs` | `README.md`, `site/src/content/docs/guide-using-poly.mdx`, `site/tests/` | The section matches the DoD; a claim test forbids `xattr` in both files, seen red before the edit; `doc-discipline` passes | `open` |

## Sequencing

**M001, M002 and M003 are independent** — the release workflow, the CI
workflows and the hosts share no file except `release.yml`, where M001/S02
and M003/S02 touch different steps. Land M001 first if only one runs, because
M006 is waiting on it.

**M006 follows M001** — the first cut should exercise the pipeline M001
hardens, and OS02's version decision is what the first tag is named from.
**M007 also waits on M003/S03**, because the package cannot know whether to
carry the AU until OS17 is decided. **M007 and M008 are independent of each
other** and each splits into an unsigned half that finishes now and a signed
half that waits on the Apple enrolment or Azure's identity validation.
**M009 is last**, after both installers and the README rewrite it replaces a
section of.

**M004/S03 depends on M003/S01 and M004/S02** — the README publishes the host
table and the positioning sentence, so it is written after both exist.
M004/S01 and M004/S02 can start at once.

**M005/S01 depends on M001/S01 and M003/S01** — it needs the version and the
host list. **M005/S02 also depends on M006/S01**, because there is nothing to
list until a Release exists.

## Related issues

| Issue | Relation |
|---|---|
| #282 | The committed WASM engine is not byte-reproducible. Not a launch blocker; relevant to OS04 only if provenance is later extended to the site's artifacts |
| #100 | `appendix-presets.mdx` documents 14 of 43 factory presets — now 45. OS20's roadmap still points contributors at it |

## Out of scope

- Editor resize or zoom — fixed at 1160×760 (`web_ui_view.cpp:70`, no
  `canResize`). The first post-launch candidate: cramped on a 13-inch laptop
- Undo/redo in the editor, MIDI input, MIDI learn
- JSON presets. Factory presets are 2,981 lines of C++ in
  `engine/src/presets.cpp`; a data format is the largest community lever and a
  design of its own
- CLAP and a Linux plugin (D029). Both widen the open-source audience; neither
  is needed to launch
- Coverage thresholds — Codecov is informational and there is no
  `codecov.yml`; a threshold before a baseline is a ratchet with nothing to hold
- RealtimeSanitizer on the process path — adopt when the sanitizer runners have
  a Clang with it; the grep-based `check-realtime-safety.sh` stands until then
- Auto-update — download-and-run is the whole delivery scope; in-plugin update
  checks are a separate programme with a privacy surface of their own
- Store or package-manager distribution — App Store, Homebrew, winget; the
  Releases page is the channel
- Installing presets or content — the 45 factory presets are in the binary
- The WASM/site build — deployed by `deploy-site.yml`, unrelated to plugin
  distribution
