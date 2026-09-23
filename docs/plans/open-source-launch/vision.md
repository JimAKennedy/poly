---
class: gated
---

# Open-source launch — vision

**Input document for `/jk:assess`.** Not a plan: a statement of the outcome, the
current state measured rather than assumed, and the decisions a plan will have
to take. The milestone decomposition below is a proposal to argue with.

Scope note: [`docs/plans/first-release/vision.md`](../first-release/vision.md)
decided **what ships** — one view, no deep dives, a bibliography a reader can
finish — and its ledger is `done`. This document covers the rest of "first
release": **what a stranger meets** when Poly is offered as an open-source
project — the release pipeline's honesty about itself, the CI a contributor
inherits, the hosts a musician will actually load it in, the repository and
site a visitor lands on, what Poly says it is — and, in the [Delivery](#delivery--how-the-release-is-built-signed-and-installed)
section near the end, **how a release is built, signed and installed**. That
section absorbs the former installers vision, corrected against what was
measured on 2026-09-23; the installers programme was retired unassessed.

It is written from an external review of `main` at `412020d` (2026-09-23).

## The observation

The engine is not what stands between Poly and a credible launch. It builds,
its 569 engine tests pass in 2.3 seconds on GCC 13, its state format is
versioned and migrated losslessly, and its guards are mutation-proved. That is
above the norm for open-source plugins and in places above commercial ones.

What is not ready is everything at the edge: a release pipeline that has never
run and disagrees with itself about the version number, a host-compatibility
claim nobody has measured, a repository that will not accept an issue from the
people it is inviting, and a public face written for the maintainer rather than
for a musician deciding whether to download it.

Almost none of it needs designing. It needs measuring, deciding and saying.

## What is actually there

### The release does not know what version it is

| Where | Says |
|---|---|
| `CMakeLists.txt` `project(poly VERSION …)` | `0.1.0` |
| `plugin/source/plugids.h:15` `kPolyVersionString` — what a DAW shows | **`1.0.0`** |
| `webui/package.json` | `0.1.0` |
| `site/package.json` | `0.0.1` |
| `plugin/resource/au-info.plist` AU `version` | `0xFFFFFFFF` (the always-rescan development sentinel) |
| `CHANGELOG.md:102` | `## [0.1.0] - 2026-06-27`, headed "Initial open-source release" |
| `git tag` | one tag, `v0.1.0-doccov-baseline`; no release has been published |

The plugin a DAW scans reports itself as 1.0.0 while the build that produced it
is 0.1.0. And because `release.yml` builds its body from the CHANGELOG section
matching the tag, **tagging `v0.1.0` today would publish the 397-word June
section and none of the 47 entries under `[Unreleased]`** — the release notes
would describe a product three months and two programmes old.

### The release workflow validates, but does not test, what it ships

`release.yml` runs build → VST3 validator → pluginval → sign → package →
publish. It never runs `ctest`, so the one configuration nobody else builds —
the macOS **universal** binary (`ci.yml` builds arm64 only) — ships with its
unit and golden tests unexecuted. It publishes no checksums and no build
provenance, and it downloads pluginval with `curl` without verifying it
(`release.yml:98`, `:118`; the same pattern in `ci.yml:434`, `:480`).

### CI is strong, with four soft spots

Strong: actions pinned by SHA, gitleaks, nightly ASan/UBSan/TSan including
plugin-inclusive legs, pluginval at strictness 8 on both shipping platforms, an
engine-isolation job, the aggregate `ci-complete` gate, and a real-DAW nightly
whose self-hosted runner is reachable only by schedule and dispatch.

Soft:

- **`ci.yml` has no top-level `permissions:` block** — only `secrets-scan`
  declares one — so every other job's token falls back to whatever the
  repository default is. There is also no `concurrency:` group, so superseded
  pushes to a PR keep running the full macOS/Windows matrix.
- **`pr-af-review.yml` checks out `Agent-Field/pr-af` at its default-branch
  HEAD** and runs `docker compose up` with `OPENROUTER_API_KEY` in the
  environment. Every other third-party action in the tree is SHA-pinned; this
  one executes whatever that repository's main branch contains today.
- **The fuzzer runs on no machine.** `tests/CMakeLists.txt:62` builds
  `fuzz_state_io` under `if(BUILD_FUZZ_TESTS)`, an option declared nowhere and
  referenced by no workflow or script. The MIDI reader — the other untrusted
  input `SECURITY.md` names, reached by dropping a `.mid` file on a lane — has
  no fuzz target at all.
- **Warnings are advisory.** `POLY_WARNINGS_FATAL` defaults `OFF`. An
  engine-only GCC 13 build reports 69 unique warning sites in engine sources
  (mostly `-Wsign-conversion` from `types.h` and `scene.h`), and two of the
  test-side warnings are real: `tests/golden_tests.cpp:548` computes
  `bothSilentSomewhere` and never asserts it, and `engine/src/midi_reader.cpp:203`
  reads `d1` — the note number — and discards it, so a multi-instrument file
  dropped on one lane merges every instrument's onsets.

### Host compatibility is asserted, not measured

`README.md` says Poly "should work with any VST3-compatible host". Routing MIDI
**out of** a plugin is exactly where hosts differ most: Ableton Live needs a
second track whose *MIDI From* selects the plugin; Logic has no VST3 at all and
routes generated MIDI only from a MIDI FX (`aumi`) unit. Only Cubase is
exercised.

- **The AU is an instrument.** `au-info.plist` declares type `aumu`. The
  `build-au-macos` job builds it on every PR; no release contains it. As an
  `aumu`, it could not drive another instrument in Logic even if it shipped.
  The Delivery section asks whether the AU ships; this is the fact that
  question needs.
- **Default channel layout.** `types.h:268` defaults `midiChannel` to `-1`,
  auto: one channel per lane. A single-channel drum sampler hears one lane
  until the user finds the Note Map. That may be right; nobody has checked it
  against the hosts a first-time user will try.
- **The editor is not validated anywhere but Windows Cubase.** pluginval runs
  with `--skip-gui-tests` on all four legs (`ci.yml:441`, `:486`,
  `release.yml:103`, `:122`). The editor is a WebView — the likeliest source of
  a host crash on open/close — and nothing automated opens it on macOS.

### A stranger cannot file an issue

GitHub shows **"Issue creation is restricted in this repository"** to a
visitor. The README invites contributors to good-first-issues;
`.github/ISSUE_TEMPLATE/` carries bug and feature templates; `SECURITY.md`
names an email. The one channel a user with a broken install would reach for is
closed.

The repository's About panel carries no description, website or topics, so it
does not appear under the `euclidean-rhythm` or `vst3` topics where people look.

### The public face is written for the maintainer

- `README.md` puts Building (line 26) before Installing (line 67), carries six
  internal decision and milestone IDs (`D029` ×2, `M054`, `D031`, `M030 S03`,
  `D004`), and gives the signing-secrets procedure its own top-level section.
  It has no screenshot and does not mention the in-browser engine the site
  already ships.
- `ROADMAP.md` lists #172, #142, #89 and #111 — all closed — while the open
  issues are #320, #305, #282, #266 and #100. Its Priority 3 table is empty.
- The site shows **"🚧 Under active construction"** on every page
  (`site/src/components/Banner.astro:4`), and its hero offers *Start Reading*
  and *GitHub* but no download.
- `CLAUDE.md:18` still lists "VSTGUI 4"; VSTGUI support has been off since
  M053/S05. `.bg-shell/manifest.json` is tracked (content `[]`, from the initial
  commit) although `.gitignore:322` ignores `.bg-shell/`.
- `docs/` holds 247 files and roughly 345k words. Most are delivery records,
  which is right for this repository's way of working — but nothing tells a
  contributor which few are the ones to read.

### The release notes would be engineering narrative

`[Unreleased]` is 47 entries and about 13,100 words: precise, honest, and
written for the next maintainer. `release.yml` publishes the matching section
verbatim as the Release body. The first thing a musician reads about Poly would
be a paragraph about `lockPresetReferent`.

### What Poly says it is

The repository calls Poly a "polymetric drum pattern generator". The launch
intent is "an open-source Euclidean sequencer". Those are different pitches,
and the second is the weaker one.

Free Euclidean sequencers are a crowded category — HY-RPE2's free tier, HY-ESG,
XiiixxiQ, the open-source GenerativeMIDI (AU/VST3 including an `aumi`, plus
iOS), and Ableton Live 12's built-in rhythm generator. Against a feature
checklist Poly loses: no CC lanes, no CLAP, no Linux plugin, no Logic.

What Poly has that none of them pairs is **grooves grounded in named
traditions** — hand-authored clave, non-isochronous timing, kotekan interlock,
tihai, phrase gating, phasing — **with a cited guide explaining each of the 45
presets**, deterministic seeded output, drag-to-DAW MIDI, and an engine that
runs in the browser. Euclidean distribution is where a lane starts, not what
the product is.

## Decisions a plan must take

1. **What is the first version number?** `0.1.0` is already spent on a June
   section that describes a different product. Options: rename that section and
   tag `0.1.0`; or tag `0.2.0` and keep June as history. L6's first tag is
   named from this answer.
2. **Does Poly support Logic?** Build and ship an `aumi` MIDI FX, or declare
   VST3-only with Logic unsupported and say so. Either answers L7's AU scope
   question — after checking the SDK's AU wrapper can produce an `aumi` at all.
3. **Which hosts are "supported"?** The set a first release claims, each tested
   and with routing instructions, versus "reported to work".
4. **Is auto-per-lane the right default channel?** Decided against the host
   matrix, not in the abstract.
5. **Does MIDI import keep or discard pitch?** Today it discards it. Filtering a
   dropped file by note number is the alternative.
6. **What is the one-sentence positioning?** Recommended: lead with polyrhythmic
   traditions and the browser demo, not with "Euclidean".
7. **Where do musician-facing release notes live?** A short section per version
   that the release body is built from, with the engineering narrative kept —
   either in `CHANGELOG.md` below it or in a separate file.

## Proposed milestone decomposition

To argue with, not to accept.

| Milestone | Outcome | Depends |
|---|---|---|
| **L1 — The release describes itself** | One version from one source; the release tests what it ships and publishes checksums | — |
| **L2 — CI a stranger can trust** | Least-privilege workflows, pinned third-party code, fuzzed inputs, warnings that fail | — |
| **L3 — A musician's DAW finds it and it plays** | A measured host matrix, the editor validated on both platforms, Logic decided | — |
| **L4 — A stranger can find it, file against it, and follow it** | Issues open, positioning decided, README and site written for the downloader | L3 (for the host matrix the README publishes) |
| **L5 — The release notes are for musicians** | A short release body; the launch listed where musicians look | L1, L4 |
| **L6 — A release is cut, and says whether it is signed** | The pipeline has run against a real tag; an unsigned build cannot ship quietly | L1 |
| **L7 — macOS installs without a terminal** | A signed, notarized `.pkg`; no `xattr` | L6, L3 (the Logic decision) |
| **L8 — Windows installs without a warning** | A signed installer via Azure Artifact Signing | L6 |
| **L9 — The instructions match the artifacts** | README and guide describe the installers, and the Gatekeeper workaround is deleted | L7, L8, L4 |

**L1 lands before L6 cuts the first pre-release**, so the first tag exercises
the hardened pipeline rather than the one this document measures. L7 and L8
each split into a half that finishes unsigned and a half that waits on the
certificate, so the packaging completes while identity validation is pending.

## Delivery — how the release is built, signed and installed

Absorbed from the installers vision on 2026-09-23. Its measurements stand;
four of its premises about signing were overturned by the vendors' own pages
that day, and three facts it missed were found while checking it.

### What is there

`release.yml` runs on a `v*.*.*` tag, builds macOS-universal and Windows-x64,
gates each behind the VST3 validator and pluginval at strictness 8, and
publishes a GitHub Release whose body is the matching CHANGELOG section.
`scripts/check-release-workflow.mjs` locks that shape in 27 assertions. What it
produces is a `.zip` containing a `.vst3` bundle, and `README.md` asks the user
to unzip it, copy the bundle into a folder they are expected to know about,
and on macOS run `xattr -dr com.apple.quarantine` first. That instruction is
the tell: a musician is asked to use the terminal to defeat a security
mechanism, because the alternative is a plugin their DAW silently refuses.

**No release has ever been cut.** The one tag is a doc-coverage baseline
pointing at a July commit. The pipeline has never produced an artifact a
stranger downloaded.

**The macOS signing steps skip silently.** Codesign, notarize and staple each
gate on `env.MACOS_… != ''`; the repository holds one secret and it is not a
signing one, so the leg ships an unsigned zip and the workflow's own output
does not say so. **Windows has no signing at all.** And the workflow sets no
pre-release flag, so a release-candidate tag would publish as a full release.

### Why an installer without signing is worse than a zip

An unsigned zip is inert: the user copies a folder and clears one Gatekeeper
prompt. An unsigned **installer** asks for administrator rights from a binary
the OS cannot attribute to anyone: macOS refuses an unsigned `.pkg` outright
and SmartScreen presents an unsigned `.exe` as a probable threat. Signing is
not a polish step after the installers; it is what makes them worth building.

### What changed since the installers vision was written

- **EV certificates no longer bypass SmartScreen.** Microsoft removed the
  instant-reputation behaviour in 2024; EV and OV now build reputation the
  same way, so the vision's OV-versus-EV question collapses.
- **OV keys must sit on a hardware token or HSM** (CA/Browser Forum, June
  2023), which a GitHub-hosted runner cannot hold. Signing in CI means a cloud
  signing service.
- **Azure Artifact Signing** (formerly Trusted Signing) is Microsoft's
  recommended route for non-Store distribution: about USD 9.99 a month, no
  token, integrates with GitHub Actions, identity validation in a few business
  days, individuals eligible in the USA and Canada. **Decided: this is the
  Windows route.** SignPath Foundation offers free signing to qualifying
  open-source projects and is recorded as the alternative, not taken.
- **The Apple Developer Program is USD 99 a year** and covers both macOS
  certificates. **Decided: enrol.** A signed `.pkg` needs a *Developer ID
  Installer* certificate as well as the *Developer ID Application* one the
  README documents — a second certificate and a seventh secret the pipeline
  knows nothing about today.
- **First Windows downloads will still warn** until publisher reputation
  accumulates, whichever route is taken. The release notes say so rather than
  promising a warning-free first release.
- **The AU job builds arm64 only** while the release VST3 is universal, so if
  the AU ships (L3's decision), the release pipeline gains a universal AU
  build, not a copy step.

### Delivery decisions a plan must take

8. **The installer formats.** macOS is a `.pkg` (`pkgbuild`/`productbuild`),
   the only format Gatekeeper and notarization treat as first-class. Windows is
   a genuine choice — WiX/MSI, Inno Setup, NSIS — to be made on which signs
   and uninstalls cleanly in CI, not on taste.
9. **Whether an unsigned build may ever ship**, and if so how that is a
   recorded choice rather than a silent skip.

## Out of scope

- Editor resize or zoom (the view is a fixed 1160×760 at
  `plugin/source/webui/web_ui_view.cpp:70`), undo/redo, MIDI input and MIDI
  learn, JSON presets (factory presets are 2,981 lines of C++ in
  `engine/src/presets.cpp`), CLAP, and a Linux plugin (D029). Each is a real
  post-release candidate; none is a reason to hold the first release.
- Coverage thresholds. Codecov is informational (no `codecov.yml`); a threshold
  set before a baseline exists is a ratchet with nothing to hold.
- RealtimeSanitizer on the process path — worth adopting after release, when a
  Clang toolchain with it is on the sanitizer runners.
- **Auto-update.** Download-and-run is the whole delivery scope; in-plugin
  update checks are a different programme with a privacy surface of their own.
- **Store or package-manager distribution** — App Store, Homebrew, winget.
  The Releases page is the channel.
- **Installing presets or content.** The 45 factory presets are in the binary;
  there is no separate payload.
