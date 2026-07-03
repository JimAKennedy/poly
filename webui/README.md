# Poly Web UI (M028 experiment)

The desk/cloth plugin UI as a host-agnostic web bundle. This is the working
artifact of the web-UI spike (see `docs/internal/v3-to-plugin-plan.md` —
gitignored planning docs): the same files run

1. **standalone in a browser** against `mock-host.js` (a WebAudio preview
   engine with the Afrobeat 12/8 factory preset) — used for development,
   Playwright CI, and eventually docs-site embeds; and
2. **inside the plugin** against `plugin-host.js`, which bridges to the
   native processor/controller over the webview message channel
   (`bridge-schema.md`).

## Run it

```bash
# no build step — open directly:
open webui/index.html            # macOS
xdg-open webui/index.html        # Linux

# UI tests (no VST3 SDK required):
cd webui && npm install && npx playwright test
```

Space = play/stop · 1/2 = cloth/desk · L = learn layer · Esc = collapse
strip · ⤢ on a strip opens the deep editors (Pattern / Timing / Envelopes).

## Files

| file | role |
|---|---|
| `index.html` | markup shell; picks mock vs plugin host |
| `ui.css` | the design language's tokens + layout (DESIGN_LANGUAGE.md) |
| `ui.js` | renderer + interactions; talks only to `window.PolyHost` |
| `host-iface.js` | the host contract (documented interface) |
| `groove-math.js` | pure pattern math shared by hosts and renderer (mirrors engine semantics) |
| `mock-host.js` | standalone host: model + WebAudio voices |
| `plugin-host.js` | native bridge host (transport only; C++ owns truth) |
| `bridge-schema.md` | JS ↔ C++ wire format, versioned |
| `tests/` | Playwright smoke tests |

## Status

Phase A of the spike: UI split from host, mock host complete, plugin host
implemented against the schema but the native shell (Phase B,
`-DPOLY_WEB_UI=ON`) is scaffolding — see `plugin/source/webui/`.
