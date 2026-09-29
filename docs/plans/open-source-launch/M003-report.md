# M003 — A musician's DAW finds it and it plays

**Review-gate report.** Generated from the ledger, `git log` and
`M003-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** The one host the first release claims — Cubase — is one Poly has
been loaded in, routed and heard on both shipping platforms, and the README
says so and nothing more; the editor is exercised on both platforms; Logic is
supported or declined on purpose.

**Branch:** `milestone/M003-hosts` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## The milestone was re-scoped before it ran

The owner's direction on 2026-09-29, before any question was answered: the
first release supports Cubase only. The planning commit amended the ledger:
M003 measures Cubase alone and claims nothing else, and the other DAW
candidates (OS43) and the Logic wrapper question (OS44) are a new **M010 —
Other DAWs, after the release**, which waits on a published Release.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | Cubase is measured, and nothing else is claimed | OS14, OS15 | done |
| M003/S02 | The editor is exercised on both platforms | OS16 | done |
| M003/S03 | Logic is supported or declined on purpose | OS17 | done |

Every row is `done`.

## Definition of done

- [x] Cubase has been loaded, routed and played on both shipping OSes, with the
      host version and the exact routing steps recorded — Windows from a named
      nightly run, macOS from the owner's session
- [x] The README and guide chapter 17 carry the same host table: Cubase as
      supported, every other host as untested, and no "should work" claim
- [x] The default channel layout is decided against what Cubase did
- [x] pluginval runs its GUI tests on the macOS CI leg, or the evidence records
      why a hosted runner cannot and what covers the editor instead
- [x] The same is true of the Windows leg
- [x] The release workflow matches whatever CI settles on
- [x] The owner's decision is recorded in this milestone's decisions file and
      cited by M007/S01
- [x] If supported: an `aumi` unit passes `auval`, drives an instrument track in
      Logic (evidence), carries the real version, and the release builds and
      ships it — n/a, declined
- [x] If declined: the README and guide say Logic is not supported, and
      `build-au-macos` carries a comment saying it exists for validation only

## What changed

**One host table, Cubase in it** (OS14). The README's DAW compatibility
section and a new Hosts section at the top of chapter 17 carry the same
five-column table: Cubase Pro 15 on macOS from the owner's sessions, Cubase 14
on Windows from green nightly run 36511596046 on 2026-09-29, every other host
untested. `site/tests/host-table.test.mjs` holds the two copies identical and
forbids the old "should work with any VST3-compatible host"; it was red on
both files first.

**The default works, and the guide now says so first** (OS15). `midiChannel`
stays `-1`: a drum instrument on a new Cubase instrument track listens on all
channels and receives every lane on its GM note. Chapter 17's routing opens
with that case, names the MIDI Send alternative, and keeps multi-instrument
routing and channel filtering under an advanced heading; the sentence telling
readers to set every lane to Channel 1 is gone.

**pluginval opens the editor on every leg** (OS16). `--skip-gui-tests` is
removed from both `ci.yml` legs and both `release.yml` legs; the
release-workflow contract asserts it is absent and was red first. macOS is
proved by a local run at strictness 8 with the Editor, Open editor whilst
processing and Editor Automation tests completed. Windows is proved by this
branch's pull request, by the owner's decision.

**Logic is declined, with the reason in writing** (OS17). The SDK's AUv2
wrapper cannot produce the MIDI-FX unit Logic routes MIDI from. The README
and the guide's Load Poly list say Logic Pro is not supported and why, the
guide's Logic routing block is gone, and the AU job and target comments say
the Audio Unit exists for validation only and ships in no release. Two site
tests hold it, red first.

## Validation

Re-run on `dd9d4b5`, the head this report describes.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `dd9d4b5` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `dd9d4b5` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass — 285/285 | `dd9d4b5` |
| `site-unit` | `npm --prefix site test` | pass — 338/338 (331 before the milestone, 7 added) | `dd9d4b5` |
| `guards` | `bash scripts/check-guards.sh` | pass — 17 guard invocations | `dd9d4b5` |
| `unit` | `cmake --build build … && ctest …` | pass — **701/701** | `dd9d4b5` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `dd9d4b5` |

Every assertion this milestone added was seen red before it was trusted: the
host-table tests on both files, the routing-order test on the old chapter,
the release-workflow contract on the old flags, and the two Logic tests on
the old guide.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `55e4352` | M003/S01 | | docs(plans): re-scope M003 to Cubase, defer the other DAWs to M010, and plan all three slices |
| `b0762d7` | M003/S01 | OS14 | docs: one host table, Cubase in it, and no claim about hosts nobody has tried |
| `f324259` | M003/S01 | OS15 | docs(guide): chapter 17 starts with one drum instrument, and per-lane channels are the advanced case |
| `9d03010` | M003/S02 | OS16 | ci: pluginval runs its GUI tests on every leg, and the contract forbids skipping them |
| `dd9d4b5` | M003/S03 | OS17 | docs: Logic is declined for the first release, on purpose and in writing |

## What a reviewer should look at twice

### The Windows pluginval proof arrives with the PR, not before it

By the owner's decision, both legs were flipped now and CI runs only on the
milestone PR. If `pluginval-windows` is red on that run, ship's report says
so and the DoD's fallback applies: the Windows leg keeps the flag with that
run recorded as the reason. The evidence and the decisions file both say
this.

### The macOS Cubase row rests on the owner's report

Cubase Pro 15 on macOS is recorded from the owner's sessions and the
routing the owner described, not from an automated run; the Windows row is
the nightly. The table's Status column says `supported` for both, which is
the claim the owner is making.

### The guide still gives loading and routing steps for hosts nobody measured

`guide-using-poly.mdx` lists Ableton Live, Studio One and FL Studio under
Load Poly and under Multi-Channel Routing by DAW. They are not in the host
table and the README says other hosts are untested, but a reader of the
guide meets instructions for them. Those hosts are M010's; whether the
instructions should be softened before then is a question for M004/S04 (OS41
rewrites the guide's install section) or a row of its own.

### The AU is still built on every PR

`build-au-macos` keeps running `auval` on an Audio Unit no release will ship,
now with a comment saying so. It costs a macOS runner per PR. Keeping it was
the row's own decline arm; dropping the job would be a separate decision.

### One judgment call

The Cubase channel-filtering steps folded under the advanced heading said
the targets listen on Channels 0, 1 and 2 while the chapter's own table
assigns 1, 2 and 3; the steps now match the table. Recorded in the decisions
file.

### The ledger gained a milestone

M010 (OS43, OS44) was added in the planning commit. Its slices depend on
M006/S01 and M003/S01, and it is last in the sequence. The reconciliation
section records the amendment and that two items were added and none closed.

## Decisions

Copied from `M003-decisions.md` so the report stands alone.

# M003 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-29 — the milestone is re-scoped before planning

The owner's direction, before any question was answered: **the first release
supports Cubase only.** The other DAWs become a future milestone. The ledger
was amended in the planning commit: M003's vision, demo and S01 now measure
Cubase alone and claim nothing else; the candidate hosts (Reaper, Bitwig,
Ableton Live 12, FL Studio, Studio One) are M010/S01 (OS43) and the Logic
wrapper question is M010/S02 (OS44), both waiting on a published Release.

## 2026-09-29 — planning M003/S01, M003/S02 and M003/S03

Measured before asking, on `main` at `36336fe`. `README.md:153` says Poly
"should work with any VST3-compatible host" and names no version. Chapter 17
already has a "Single-Instrument Setup (Default)" section, but it tells the
reader to set every lane to Channel 1, and its Cubase routing section is the
multi-channel one. `guide-using-poly.mdx` lists Logic Pro under "Load Poly"
and under the recording steps, and describes the AU as a shipping format.
pluginval 1.0.4 with its GUI tests enabled passes locally on macOS at
strictness 8 against the built bundle: Editor, Open editor whilst
processing and Editor Automation all completed. `ci.yml`, `release.yml` and
`scripts/check-release-workflow.mjs` carry `--skip-gui-tests` on all four
legs and the contract test asserts exactly two. The self-hosted nightly runs
Cubase **14** on Windows (`POLY_CUBASE_VERSION`), and its latest green run is
`https://github.com/JimAKennedy/poly/actions/runs/36511596046`, dated
2026-09-29, on the M002 merge commit. The VST3 SDK's AUv2 wrapper
(`public.sdk/source/vst/auwrapper`) subclasses `MusicDeviceBase`, its
resource file allows only `kAudioUnitType_Effect` or `_MusicDevice`, and it
implements `kAudioUnitProperty_MIDIOutputCallback`; the AUv3 wrapper likewise
knows only the MusicDevice type. No release workflow builds the AU.

- **Q:** Which Cubase do you run Poly in on macOS? — **A:** Cubase Pro 15.
- **Decision:** the host table's macOS row is Cubase Pro 15, supported, from
  the owner's sessions; the Windows row is Cubase 14, supported, from the
  named nightly run — **Why:** both platforms are exercised, each by the
  means that actually exercises it.
- **Q:** OS15: keep the per-lane channel default? — **A:** yes. "By default,
  the plugin works fine when plugged into a drum machine. A consuming drum
  machine listening on all MIDI channels for the Poly MIDI output receives
  sensible notes for the various drum instruments. We should make sure the
  guide starts with that for its initial routing guide. The selection of
  specific MIDI channels for single lanes or groups of lanes also works fine,
  but it is more advanced, so we should cover those advanced options as well
  in the guide."
- **Decision:** `midiChannel = -1` stays; chapter 17's routing opens with the
  one-instrument case exactly as the owner described it and moves the
  per-lane channel routing under an advanced heading; no state migration —
  **Why:** the default already does the right thing with a drum instrument
  that listens on all channels, and the guide was telling readers to change
  it.
- **Q:** OS16: how is the Windows pluginval leg handled, given only the
  milestone PR can run it? — **A:** flip both legs now.
- **Decision:** `--skip-gui-tests` is removed on all four legs in `ci.yml`
  and `release.yml`, and the contract test asserts it is absent; the PR's
  Windows run is the proof — **Why:** macOS is proved locally, and if Windows
  is red the ship report says so and the DoD's fallback applies with the CI
  evidence as the reason.
- **Q:** OS17: with the first release Cubase-only, decline Logic? — **A:**
  decline for the first release.
- **Decision:** the README and guide say Logic is not supported and why;
  `build-au-macos` keeps building the `aumu` for validation only, with a
  comment; no release contains the AU; M007 packages the VST3 alone; M010/S02
  revisits with a wrapper spike — **Why:** the SDK wrapper cannot produce the
  MIDI-FX unit Logic routes MIDI from, so "supported" would mean a wrapper of
  Poly's own, which is not first-release work.

### Taken on the owner's behalf

- **The host table has the same five columns in both places** — Host,
  Version, Platform, Status, Routing — and a site test asserts the README's
  and chapter 17's tables are identical row for row, so they cannot drift
  apart. The Status vocabulary is `supported` and `untested`; `reported to
  work` is M010's to introduce when a host earns it.
- **The routing steps in the evidence are the owner's**, as stated above
  and as the guide's "MIDI Send (recommended)" option records them; the
  nightly's own routing (loopMIDI to the probe) is cited for Windows as what
  the run actually did.
- **The local pre-push gate keeps `--skip-gui-tests`.** OS16 names the four
  CI legs; the hook runs at strictness 5 on every push and opening editor
  windows there was not asked for. Recorded so nobody reads the asymmetry as
  an oversight.
- **S03 touches Logic's lines only, not the guide's AU install prose.** The
  guide's "two formats" and `Poly.component` copy instructions are OS41's
  (M004/S04), whose row already says the AU is described "only if OS17 says
  it ships". S03 records the decision OS41 will consume.
- **The milestone runs S01, S02, S03 in ledger order** with no pause: the
  Cubase evidence comes from the nightly and the owner's answers above, so no
  manual step remains inside the run.

## 2026-09-29 — judgment call during M003/S01 task 2

- **The advanced routing steps now number channels from 1.** The Cubase
  step being folded under the advanced heading said the Groove Agent track
  "listens on Channel 0, Battery on Channel 1, Kontakt on Channel 2" while the
  chapter's own paragraph two screens up says Poly shows channels 1-16 as the
  DAW does, and the patch table above the step assigns Channels 1, 2 and 3.
  The step now matches the table. Obviously right: the same passage was being
  rewritten, and leaving a 0-based sentence beside a 1-based table would have
  shipped a contradiction the restructure had just moved closer together.
