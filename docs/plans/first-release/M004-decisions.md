# M004 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

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
