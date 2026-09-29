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
