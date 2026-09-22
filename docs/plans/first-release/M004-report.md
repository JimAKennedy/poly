# M004 — The docs describe what ships

**Review-gate report.** Generated from the ledger, `git log` and
`M004-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** Someone reading the guide finds the product they downloaded.

**Branch:** `milestone/M004-docs-match` · **Ledger:** `docs/plans/first-release/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S01 | The guide matches the build | FR21, FR22, FR26 | done |

Every row is `done`. FR26 was added mid-slice at the owner's request.

## Definition of done

- [x] No screenshot shows a control the shipped UI does not have
- [x] Chapter 18 describes the views that exist
- [x] `CHANGELOG.md` records the removals under the right heading
- [x] A lane's name stays legible with the per-lane export handle present, and
      a guard fails if it does not

## What changed

**The interface screenshot is generated, not hand-captured** (FR21). The
picture the guide carried showed CLOTH, DESK and LEARN and none of the capture
controls or Export; the vision had called it "already Desk". Its BPM and seed
were the mock host's defaults, so `webui/tests/capture-guide-screenshots.mjs`
regenerates it from the same source and refuses to write if the page carries
a control the shipped UI lacks, lacks one the walkthrough names, or hides
Export. The walkthrough was read against the frame: the seed controls and the
lane add/remove bar are now named, and the lane count and seed are no longer
listed as hidden parameters when both have on-screen forms.

**A lane's name no longer clips beside its export handle** (FR26). The first
capture with Export enabled rendered BELL as a 7px sliver: the third head
button narrows the name column, "Anchor pulse" wraps, and the name — a
shrinkable flex item in a fixed 40px head — absorbed the overflow. The line
heights are now explicit, the name does not shrink, and the head is the 42px
those make deterministic. `lane-head.spec.mjs` checks every lane with and
without the handle.

**Chapter 18 is "Editors", and the changelog has a Removed section** (FR22).
The title, sidebar label and four cross-references change; the slug and
anchors do not. Two Removed entries — the Cloth view with its Learn overlay,
and the theory deep-dive section, deferred not deleted — point at the long
entries under Fixed and Changed.

## Validation

Re-run on `a75ba90`, the head this report describes.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `a75ba90` |
| `site-unit` | `npm --prefix site test` | pass — **331/331** | `a75ba90` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass — 285/285 | `a75ba90` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `a75ba90` |
| `webui-e2e` | `npm --prefix webui test` | pass — **360/360** | `a75ba90` |
| `ledger` | `jk-standards ledger` | pass — 5 conform | `a75ba90` |

**358 webui tests before the milestone, 360 after**: the two lane-head cases.
The site suite is unchanged at 331.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `a7fed2c` | M004/S01 | | docs(plans): front-load M004's decisions and plan its one slice |
| `f2f7a89` | M004/S01 | FR26 | fix(webui): a lane's name stays legible with the per-lane export handle |
| `6debb6d` | M004/S01 | FR21 | docs(guide): the interface screenshot is regenerated from the one-view UI |
| `a75ba90` | M004/S01 | FR22 | docs(guide): chapter 18 is "Editors", and the changelog says what was removed |

## What a reviewer should look at twice

### A row was added mid-milestone, and it touches the plugin UI

The ledger scoped M004 to docs and named engine, preset and parameter changes
out of scope. FR26 changes `webui/ui.css` — one rule's line heights and a
head height — because the screenshot could not honestly show the toolbar the
plugin ships without also showing a clipped lane name. The owner chose to fix
it rather than capture around it; the alternative, a capture without the
export seam and an issue, is recorded in the decisions. The change is three
declarations and a comment, and every lane still starts its ring at the same
Y.

### The guard was vacuous once

`lane-head.spec.mjs`'s first draft passed on the broken CSS: a shrunken flex
item hides its overflow inside itself, so the column looked tidy while the
name was a sliver. The property that distinguishes the states is the name
element's own `scrollHeight` against its `clientHeight`; it was seen red
naming "Bell: 7px of text hidden" before being trusted. This is the third
guard in the first-release programme to be proved non-vacuous by mutation and
the first whose draft failed that proof.

### Two plan steps were wrong and were corrected before they ran

The plan named a mock-host hook, `__polyMockSetTimeSig`, that only a comment
refers to; the capture mutates `getState()` and calls `_pushState()` as the
existing screenshot spec does. And it counted one cross-reference to the old
chapter title where the tree had four. Both are documentation fixes recorded
in the decisions and the evidence.

### The capture is a script, not a spec

`playwright test` never runs it, so CI never rewrites the tracked PNG. The
cost is that nothing re-checks the picture automatically; the single-view
guard forbids the removed controls in the markup, and the script refuses to
capture them, which is the property that matters.

## Decisions

Copied from `M004-decisions.md` so the report stands alone.

## 2026-09-22 — planning M004/S01

Measured before asking: `site/public/screenshots/ui-overview.png` (1100×870)
shows a **CLOTH/DESK** chip pair and a **LEARN** button in the toolbar, and no
capture controls or Export — so it is a two-view UI in both directions, and the
vision's line that "its screenshot is already Desk" was wrong. The header prose
was corrected by M001/S02 and no longer matches its own picture. The picture
was captured from the mock-host web UI: its BPM (126.0) and seed (88) are the
mock host's defaults for Afrobeat 12/8, and the mock has a
`__polyMockSetTimeSig` hook for the 12/8 the header shows.

- **Q:** How should the replacement screenshot be produced — a Playwright
  capture checked in as a script, or a hand capture from Cubase? — **A:** the
  Playwright capture.
- **Decision:** `webui/tests/capture-guide-screenshots.mjs` drives the mock-host
  page at 1100px, sets 12/8, asserts the toolbar carries no mode chip, no
  Cloth node and no Learn button, and writes the PNG — **Why:** reproducible,
  and the same provenance as the original; a Cubase capture would show host
  chrome the guide does not describe and could not be regenerated.
- **Q:** Chapter 18 is titled "Editors and Advanced Views" and its body is the
  three deep-pane editors. Retitle, or prose only? — **A:** retitle to
  "18. Editors".
- **Decision:** the title, the sidebar label and chapter 3's link text change;
  the slug and every anchor stay — **Why:** "Advanced Views" reads as the
  removed Cloth view, and the body already says the editors are inline in the
  one view.
- **Q:** The changelog records Cloth's removal under Fixed and the deep dives'
  move under Changed. FR22 asks for the removals under the right heading —
  add a Removed section, or record that the current headings are right? —
  **A:** add a Removed section.
- **Decision:** a `### Removed` heading under Unreleased with two short
  entries, each pointing at its long entry, which stays where it is — **Why:**
  Keep a Changelog has the heading for exactly this, and a reader scanning for
  what is gone should not have to read a Fixed entry to find out.

### Taken on the owner's behalf

- **The screenshots README is corrected in passing.** It says plugin UI
  screenshots were replaced by `PolyPreviewCard` and lists two DAW captures,
  while `ui-overview.png` sits beside it uncatalogued and cited by the guide.
  It gains the file and the command that regenerates it. Replacing the static
  image with a live `PolyPreviewCard` was considered and not taken: the owner
  chose a capture, and the "Plugin Interface" section reads against a fixed
  picture.
- **The capture is a script, not a spec.** A `.spec.mjs` would run on every
  `playwright test` in CI and rewrite a tracked binary on each run; a script
  runs when asked. It reuses `test-helpers.mjs` and Playwright's own Chromium.
- **No claim test for the retitle.** FR22's verification is "both describe
  what ships; `doc-discipline` passes", and a guard forbidding the words
  "Advanced Views" would pin a phrase, not a claim. The single-view guard in
  `webui-single-view.test.mjs` already forbids the controls the old picture
  showed, and the capture asserts the same selectors before it writes.
- **The vision's "version" in R4 stays out.** The ledger scoped M004 to the
  docs matching the build and assigned tagging to the installers programme;
  this plan does not reopen that.

## 2026-09-22 — judgment call during M004/S01 task 1

- **The plan named a mock-host hook that does not exist.** A comment in
  `mock-host.js` refers to `window.__polyMockSetTimeSig(num, den)`; nothing
  defines it. The existing screenshot spec changes state by mutating
  `PolyMockHost.getState()` and calling `_pushState()`, so the capture does
  the same. The plan step is corrected as a documentation fix before the task
  ran — obviously right, since the alternative was a script that throws.

## 2026-09-22 — scope addition during M004/S01 task 1

- **Q:** The capture taken with the mock's `?export=1` seam — so the header
  shows Export as the plugin does — renders the BELL lane's name as a 7px
  sliver. Measured: the per-lane export handle narrows the name column from
  104px to 76px, "Anchor pulse" wraps to two lines, and the fixed 40px head
  shrinks the name (a flex item) rather than clipping the role. This is what a
  downloader gets. Capture without the handle and file an issue, add a row and
  fix the CSS here, or ship the picture as it is? — **A:** add a row and fix
  it here.
- **Decision:** FR26 is added to M004/S01 with its own definition-of-done
  line, the slice gains the `webui-e2e` token, and the fix is explicit line
  heights plus `flex: none` on the name, with the head at the worst case those
  make deterministic (42px) — **Why:** a fixed pixel height against the
  browser's default line heights is platform-dependent, which is how "sized
  for the worst case" was already one pixel short before the handle made it
  seven.
- **Judgment call:** the first draft of the guard checked that the name's box
  lay inside the head and that the name column did not overflow, and it
  **passed on the broken CSS** — a shrunken flex item hides its overflow
  inside itself, so the column looks tidy while the name is a sliver. The
  guard now also asserts the name element's own `scrollHeight` does not exceed
  its `clientHeight`, and was seen red on the old CSS naming "Bell: 7px of
  text hidden" before being trusted. Recorded because a green guard is not
  evidence a guard works, and this one proved it again.
