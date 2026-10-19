---
class: gated
---

# M006 — Try it plays what the editor shows

> Every control a reader can touch in the guide's Try it either changes what they hear, at once, or is visibly disabled with the reason; the editor fits a laptop screen; the playhead and the sound agree; and a test fails if any of that regresses.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M006-try-it-works` · 17 commits including this report, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S01 | Edits reach the playing engine | CA38, CA39, CA40 | done |
| M006/S02 | The engine features the browser host drops | CA41, CA42, CA43 | done |
| M006/S03 | Controls the browser cannot honour are disabled | CA44, CA45, CA46 | done |
| M006/S04 | The editor fits a laptop | CA47, CA48 | done |
| M006/S05 | The playhead and the sound agree | CA49 | done |
| M006/S06 | Every card plays | CA50 | done |
| M006/S07 | The banner does not claim the guide is finished | CA51 | done |

## What changed

- **Edits reach the engine that is playing (S01, CA38–CA40).** The browser
  host edits one engine context and plays another, built from it on Play.
  Every lane edit, the seed and every lane action now writes both, through one
  `forBoth` helper, so VEL and the other controls that waited for Stop and
  Play act at once; a preset chosen while playing is the one heard. Nothing
  re-copies scenes, which would reset the chain. The playing context is
  exposed to tests, and the control audit now checks it rather than the
  edited one: it went red naming VEL on the pre-fix host.
- **Fill, Seed Lock and the lane count (S02, CA41–CA43).** The WASM API gains
  two lane fields appended to `LaneFieldInt` (`FillEveryNBars`, `SeedLocked`,
  which captures the global seed on the lock edge as the plugin does) and two
  functions (`poly_set_active_lane_count`, `poly_trigger_manual_fill`, which
  fills exactly the next render pass). Five engine tests cover them, and the
  host wires them. **The committed `webui/poly_engine.{js,wasm}` are rebuilt**
  (`.wasm` 200,387 → 200,797 bytes); this is the only slice that changes
  engine source.
- **Controls the browser cannot honour (S03, CA44–CA46).** Hosts declare four
  capabilities: note mapping, capture, MIDI channels, note length. The WASM
  host declares none, so Note Map, Capture bars, Arm, Channel and Duration are
  disabled in the browser with a tooltip giving the reason, and send nothing.
  Lane rename works in the browser (your decision). A failed WASM load now
  shows a status bar rather than silently running the mock host.
- **The editor fits a laptop (S04, CA47–CA48).** Step bars may shrink to
  1 px and a ladder of more than 16 steps uses a 1 px gap, so a 24-step lane
  stays below its dial at 1200×520. The Try it frame takes the 760 px the
  editor is laid out for, less the modal chrome.
- **The playhead and the sound agree (S05, CA49).** One helper,
  `stepEighths`, gives `8 / subdivision / tempoMultiplier` eighths; the
  rounded step had put 35 of 56 presets' playheads out of step with their
  sound. The preview's `laneHitAt` matches fractional steps, the mock
  scheduler walks sixths of an eighth, and the Steps row names the step by
  subdivision.
- **Every card plays (S06, CA50).** A site spec reads every preview card from
  the built site (39 today), plays each in Try it at 1440×900, and fails on
  silence, overflow or an error.
- **The banner (S07, CA51).** It no longer says the guide is complete and
  verified; it carries your three sentences word for word.
- **Tests.** WebUI Playwright 360 → 405 passed; engine `unit` 736 tests;
  site-unit 442 → 443; the e2e pipeline gains a 14th gate, "M006 Try it".

## Definition of done

**M006/S01 — Edits reach the playing engine**

- [x] Every control that changes the engine changes what is playing, without
      Stop and Play
- [x] Choosing a preset while playing plays the new preset
- [x] A test edits controls during playback and fails if the sound does not
      follow

**M006/S02 — The engine features the browser host drops**

- [x] Fill Every N and Fill Now fill in the browser as they do in the plugin
- [x] Seed Lock holds the seed in the browser
- [x] − Lane and + Lane change the number of lanes that play

**M006/S03 — Controls the browser cannot honour are disabled**

- [x] Every control that cannot work in the browser is disabled there and
      says why, and stays enabled in the plugin
- [x] Lane rename works in the browser or is disabled there
- [x] A failed WASM load is visible to the reader rather than silently
      replaced by the mock host

**M006/S04 — The editor fits a laptop**

- [x] A 24-step lane's step stack stays inside its card at a laptop-sized
      window
- [x] The Try it frame gives the editor the height it is laid out for, or
      the editor is laid out for the frame it gets

**M006/S05 — The playhead and the sound agree**

- [x] The playhead shows the step that is sounding, for every factory preset

**M006/S06 — Every card plays**

- [x] A test opens every preset card on the published site at a laptop
      viewport, plays it, and fails if it does not sound, overflows, or
      throws

**M006/S07 — The banner does not claim the guide is finished**

- [x] The site banner no longer says the guide is complete and verified, and
      tells the reader that its music theory is still being revised and to
      check the references

## Validation

From each slice's evidence file, on its last task:

| Slice | Token | Result |
|---|---|---|
| S01 | `format` · `webui-e2e` · `site-unit` · `e2e` | exit 0 · 389 passed · 442 tests, 0 failures · every gate PASS, control audit 22 passed |
| S02 | `format` · `unit` · `webui-e2e` · `e2e` | exit 0 · 736 tests, 100% · 393 passed · every gate PASS |
| S02 | `wasm-freshness` | **exit 1**: the deployed engine differs from the rebuilt one, as expected until this milestone deploys (decisions) |
| S03 | `format` · `webui-e2e` · `e2e` | exit 0 · 400 passed · every gate PASS |
| S04 | `format` · `webui-e2e` · `e2e` | exit 0 · 401 passed · every gate PASS, new M006 gate 1 passed |
| S05 | `format` · `webui-e2e` · `e2e` | exit 0 · 405 passed · every gate PASS |
| S06 | `format` · `site-unit` · `e2e` | exit 0 · 443 tests, 0 failures · every gate PASS, M006 gate 41 passed |
| S07 | `format` · `site-unit` · `doc-discipline` | exit 0 · 443 tests, 0 failures · exit 0 after a `Docs-Not-Affected:` trailer |

## Look at twice

- **`wasm-freshness` cannot pass before deploy.** It compares the committed
  engine with the live site's, which is the old one until this merges. Run it
  against poly.jk.digital after the Pages deploy.
- **The rebuilt engine artifacts are in this branch** (S02). Every other
  `e2e` run restored them from HEAD.
- **`site-verify-local.sh` changed**: a 14th gate, "M006 Try it", runs the two
  new site specs and joins the exit code. It is also CI's site-e2e job, so
  the new specs run in CI.
- **A stale preview server can serve the e2e gate.** `astro preview` (Astro
  7.3.8) outlives the shell that started it, and one on port 4322 was found
  from an earlier session. While one runs, the script's own preview is
  refused and its readiness check passes against the old server. Checked:
  a running server serves `dist/` as it is at request time, so the gate still
  tested the fresh build. The script's cleanup kills its wrapper, not the
  server. Not fixed here; worth a ledger row.
- **A `Docs-Not-Affected:` trailer** on the S07 commit answers `doc-drift` for
  `docs/testing-strategy.md`, flagged by S02's one-line addition of a test file
  to the existing `poly_tests` binary.
- **Judgment calls**, all in the decisions below: shared spec helpers; the
  non-vacuous preconditions added to S01's and S05's cases after three and two
  cases first passed without proving anything; the dense-ladder gap; the
  frame's `calc(96vh - 3rem)`; the mock scheduler's sixths; the S06 overflow
  mutation.
- **Fill Now in the browser fills one quarter note** (one render pass), where
  the plugin's fills one audio block. Your decision, the same one-pass
  contract.
- **Not touched:** `site/src/audio/engine-dump.ts` still rounds `stepLen` the
  old way. It builds the parameter snapshot that goes with a dump, not the
  playhead, so it was outside CA49.

## Traceability

- `2b2a164` docs(plans): plan M006, Try it plays what the editor shows — Slice: M006/S01
- `87ad3b6` test(webui): edits in Try it do not reach the engine that is playing — Slice: M006/S01; Rows: CA40
- `1b87a12` fix(webui): edits in Try it reach the engine that is playing — Slice: M006/S01; Rows: CA38
- `f3564c5` fix(webui): a preset chosen while playing is the one heard — Slice: M006/S01; Rows: CA39
- `f987ada` test(site): the control audit checks what Try it is playing — Slice: M006/S01; Rows: CA40
- `1e41e21` feat(engine): the WASM API carries fill, seed lock, lane count and Fill Now — Slice: M006/S02; Rows: CA41, CA42, CA43
- `ab6c109` feat(webui): Fill, Seed Lock and the lane count work in Try it — Slice: M006/S02; Rows: CA41, CA42, CA43
- `9c326df` feat(webui): controls Try it cannot honour are disabled, and say why — Slice: M006/S03; Rows: CA44
- `0e96e28` feat(webui): lanes can be renamed in Try it — Slice: M006/S03; Rows: CA45
- `512950b` feat(webui): Try it says when the engine failed to load — Slice: M006/S03; Rows: CA46
- `4d68bf6` fix(webui): a 24-step lane's steps stay inside its card on a laptop — Slice: M006/S04; Rows: CA47
- `67b2144` fix(site): the Try it frame gives the editor the height it is laid out for — Slice: M006/S04; Rows: CA48
- `1048d12` fix(webui): Try it's step length is the engine's, for every preset — Slice: M006/S05; Rows: CA49
- `74277d2` fix(webui): the playhead, the preview and the step glyph follow the real step — Slice: M006/S05; Rows: CA49
- `4d0f79c` docs(site): the banner says the guide's theory is still being revised — Slice: M006/S07; Rows: CA51
- `d5519d6` test(site): every preview card plays in Try it — Slice: M006/S06; Rows: CA50

This report's own commit carries `Slice: M006/S06`.

Untraced commits: none.

## Decisions

Verbatim from `M006-decisions.md`, with headings demoted one level.

### 2026-10-18 — planning M006/S01–S07

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

### 2026-10-18 — executing M006/S01

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

### 2026-10-18 — executing M006/S04

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

### 2026-10-18 — executing M006/S05

- **Judgment call:** the plan's Task 2 said "callers in the WASM and mock
  hosts" of `laneHitAt`; it has one caller, the mock host's scheduler (the
  WASM host plays the engine's own events). That scheduler now walks integer
  sixths of an eighth, a grid both a 1/16 step and a 1/12 step land on, and
  `laneHitAt` matches the nearest step within 1e-6 — **Why:** the obviously
  right reading of the step; a whole-eighth walk skips every other 1/16 step.
- **Judgment call:** the preview case uses a 1/12 lane, not the plan's 1/16 —
  **Why:** 0.5 is exact in floating point, so a 1/16 lane passed under the old
  modulo test and proved nothing; 2/3 is not, and failed.

### 2026-10-18 — executing M006/S07

- **Judgment call:** S07's commit carries `Docs-Not-Affected:` for
  `docs/testing-strategy.md` — **Why:** `doc-drift` flagged S02's edit to
  `tests/CMakeLists.txt` (S02 did not owe `doc-discipline`, so S07's run is
  the first to see it). The edit adds `wasm_api_m006_tests.cpp` to the existing
  `poly_tests` binary, which the drift map's own reason says does not change
  the test taxonomy.

### 2026-10-18 — executing M006/S06

- **Judgment call:** the plan's overflow mutation (restoring `min-height:
  6px` on the step bars) does not fail any card at 1440×900, because S04's
  760 px frame gives a 24-step ladder room even with the old rule. The
  overflow check was proved instead with S04's whole fix reverted (the old
  bar rule, no dense gap, the 560 px frame): 24 cards fail on "a lane card
  overflows" — **Why:** the point of the step is to show the check can fail
  at the viewport it runs at; a voice() that never starts a note proved the
  sound check the same way (39 cards fail).
