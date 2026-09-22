# Poly Guide Screenshots

Static images the guide cites. Interactive pages embed the live web UI through
`PolyPreviewCard`; these files cover what an embed cannot show.

## Files

- `ui-overview.png` — the plugin interface, cited by `guide-using-poly.mdx`.
  Generated from the mock-host web UI, not hand-captured: run
  `node webui/tests/capture-guide-screenshots.mjs` from the repo root after any
  change to the toolbar or lane layout. The script refuses to write a picture
  that carries a control the shipped UI does not have.
- `02-cubase-routing.png` — Cubase MIDI channel routing setup
- `15-cubase-automation.png` — Cubase Density macro automation lane
