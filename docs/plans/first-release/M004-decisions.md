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
