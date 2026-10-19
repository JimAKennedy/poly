---
class: gated
---

# M006 — decisions

## 2026-10-18 — planning M006/S01–S07

- **Q:** Approve the seven slice designs: S01 mirrors every edit and lane
  action into `playbackCtx` and applies a preset to both contexts, exposing
  the playback context to tests; S02 appends WASM entry points for fill,
  seed lock, active lane count and manual fill, and rebuilds the committed
  artifacts; S03 adds capability flags that disable Note Map, Capture,
  Channel and Duration in the WASM host, and shows a notice on a failed WASM
  load; S04 lets step bars shrink so a 24-step ladder stays in its card, and
  sizes the Try it frame; S05 makes the step length `8 / subdivision /
  tempoMultiplier` eighths through one helper; S06 plays every preview card
  at 1440×900; S07 rewords the banner (S02 classed as an interface change)?
  — **A:** approve all
- **Q:** CA45: should lane rename work in the browser host, or be disabled
  there? — **A:** make it work: the WASM host keeps the name for the current
  scene in its JS lane-name state; the engine is not touched
- **Q:** CA48: grow the Try it frame to the 760 px the editor is laid out
  for, or lay the editor out for a 560 px frame? — **A:** grow the frame:
  `min(760px, 92vh)` with the modal widened to match; on a short screen the
  editor scrolls inside the frame
- **Q:** CA41: in the plugin Fill Now forces one render pass (one audio
  block); the browser renders a quarter note per pass. Should Fill Now cover
  the next render chunk, or the rest of the current bar? — **A:** the next
  render chunk, the same one-pass contract as the plugin
- **Decision:** S01 reaches the playing engine by applying each edit to
  `engineCtx` and, when it exists, `playbackCtx`, never by re-copying scenes
  — **Why:** `poly_copy_scenes` resets the chain to bar 0 (ledger CA38), and
  mute, macros, morph, chain and scene select already use this pattern.
- **Decision:** a test measures what plays by rendering a scratch context
  seeded from `playbackCtx` (`poly_copy_scenes` + macros) — **Why:** the
  scratch render reads the playing context's scenes without advancing its
  own render position or chain state.
- **Decision:** S02 adds lane fields at the end of `LaneFieldInt`
  (`FillEveryNBars`, `SeedLocked`) and two functions (`poly_set_active_lane_count`,
  `poly_trigger_manual_fill`); seed lock captures the global seed on the
  false→true edge as `processor.cpp` `kCoreSeedLock` does — **Why:** the
  JS side addresses fields by index, so appending is the only safe change,
  and the plugin is the reference behaviour.
- **Decision:** S02's rebuilt `webui/poly_engine.{js,wasm}` are committed;
  every other slice restores them after `e2e` — **Why:** S02 is the only
  slice that changes engine source (CLAUDE.md, "The e2e Gate Rewrites
  Committed Artifacts").
- **Decision:** S03's disabled controls carry their reason as a `title` and
  `aria-disabled`, and the WASM-load notice is a visible banner inside the
  editor — **Why:** the reason must be readable by hover and by assistive
  technology, and the notice must be on screen, not in the console.
- **Decision:** S04/CA47 drops the step bars' `min-height` from 6 px to
  1 px and clips the ladder (`overflow: hidden`) — **Why:** the stack is
  `flex: 1; min-height: 0`, so bars that may shrink fit any card height,
  and the clip guarantees no spill over the dial.
- **Decision:** S05's helper is `stepEighths(subdivision, tempoMultiplier)`
  in `groove-math.js`, used by the WASM host, the preview's `laneHitAt` and
  the step glyph — **Why:** one formula in one place is what keeps the
  playhead, the preview and the glyph from disagreeing again.
- **Decision:** S06's card list is read from the built site's HTML
  (`data-poly-preset` attributes), never written into the spec — **Why:**
  ledger CA50 requires that a new card is covered without editing the test.
- **Decision:** S02's `wasm-freshness` token is recorded as run but not
  passable before deploy: `check-wasm-freshness.sh` compares the committed
  artifacts with the deployed site, which carries the old engine until this
  milestone merges and deploys — **Why:** M002/S01 met the same limit; the
  slice's own proof that the artifacts carry the change is `e2e` (which
  rebuilds them) and the webui-e2e specs that call the new entry points.

## 2026-10-18 — executing M006/S01

- **Judgment call:** the page helpers live in
  `webui/tests/wasm-playback-helpers.mjs`, imported by S01's spec and the
  later M006 specs, rather than copied into each — **Why:** S02's plan says
  to copy them; one shared copy cannot drift between specs.
- **Judgment call:** every S01 case first asserts the edit changed the edited
  lane — **Why:** the first red run had three cases pass because their values
  left the lane as it was; a case that cannot fail proves nothing.
- **Finding:** `astro preview` (Astro 7.3.8) runs as a background server
  that outlives the shell that started it, and refuses to start a second
  one. A server on port 4322, the port `site-verify-local.sh` uses, was found
  still running from an earlier session. While one runs, the script's own
  preview is refused and its readiness check passes against the old server.
  Checked on 2026-10-18: a running server serves `dist/` as it is at request
  time (a line appended to a built file was served at once), so the `e2e`
  runs it served tested the fresh build, and their passes stand. The
  script's cleanup kills its own wrapper process, not the background server
  — **Why recorded:** a reviewer should know the gate can run against a
  server it did not start; fixing the script is outside this slice.

## 2026-10-18 — executing M006/S04

- **Judgment call:** a ladder of more than 16 steps also gets a 1 px gap
  (`.ladder.dense`) — **Why:** with the plan's `min-height: 1px` alone, a
  24-step ladder at 1200×520 still overlapped its dial by 8 px, because 23
  gaps of 3 px take 69 px; `overflow: hidden` would have hidden the top steps
  rather than fit them.
- **Judgment call:** the frame is `min(760px, calc(96vh - 3rem))` and the
  modal's `max-height` rises from 90vh to 96vh, rather than the decided
  `min(760px, 92vh)` — **Why:** the modal clips at its own max-height, so a
  92vh frame plus the modal's title bar would have been cut off; at 1440×900
  this gives the full 760 px, and below that the editor scrolls in the frame.
- **Judgment call:** `scripts/site-verify-local.sh` gains a 14th gate, "M006
  Try it", running `try-it-frame.spec.ts` (S06 adds its spec there) — **Why:**
  the script, which is both the `e2e` token and CI's site-e2e job, runs only
  the specs it names; without the gate the new site specs would run nowhere.
