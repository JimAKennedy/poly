---
class: gated
---

# Poly Test — Cubase MIDI Remote Script

`JkDigital_PolyTest.js` is the Cubase MIDI Remote driver script that lets the
headless test driver control Cubase's transport over a virtual MIDI port, and
signals readiness back to the runner. It replaces S07's placeholder
window-present readiness check with a real "ready" ping emitted when the script
activates.

## Install path (runner, Cubase 14) — the filename is load-bearing

Cubase auto-detects a driver script only when **both** the folder and the
filename follow the strict convention `Local/<vendor>/<device>/<vendor>_<device>.js`,
all derived from the `makeDeviceDriver(<vendor>, <device>, ...)` call. A file
whose name does not match `<vendor>_<device>.js` is **silently ignored** — no
surface appears in the MIDI Remote tab and nothing connects.

```
<Documents>/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/JkDigital/PolyTest/JkDigital_PolyTest.js
```

The script declares `makeDeviceDriver('JkDigital', 'PolyTest', ...)` — single
tokens with no spaces so the path/filename derivation is unambiguous. Copy
`JkDigital_PolyTest.js` there (the `3-install-midi-remote.ps1` helper does this),
then in Cubase open the MIDI Remote tab — the script auto-detects and connects
when the `poly-test` virtual port pair is present.

> **Not an import.** Cubase's **Import Script** button only reads packaged
> `.midiremote` bundles, never raw `.js` driver scripts. A `.js` driver script
> is loaded by folder auto-detection, not by importing — so it correctly will
> not appear in the Import dialog.

## Prerequisite — the `poly-test` virtual port

The script binds a loopMIDI virtual port pair whose name **contains** `poly-test`
(`PORT_NAME` in the script). Create it in loopMIDI on the runner before loading
the script.

> **loopMIDI suffix:** loopMIDI appends a non-removable instance suffix, so a
> port created as `poly-test` is enumerated by Windows as `poly-test 1` (or
> `poly-test 2`, …). There is no way to strip the suffix. Because of this the
> detection unit uses `expectInputNameContains('poly-test')` /
> `expectOutputNameContains('poly-test')` (substring, not exact) so it binds
> regardless of the suffix. The mido driver (`play_scenario.py` `find_port`)
> already matches by substring, so both halves of the contract agree the port
> name *contains* `poly-test`.

## Protocol

The driver (`tests/cubase/driver/play_scenario.py`) and this script share one
set of constants. **If you change a number here, change it there too** — the two
files are the two halves of one contract.

| Direction | MIDI | Meaning |
|---|---|---|
| driver → Cubase | CC 20, value ≥ 64, ch 1 | transport START |
| driver → Cubase | CC 21, value ≥ 64, ch 1 | transport STOP |
| driver → Cubase | CC 22, value ≥ 64, ch 1 | LOCATE to zero (To Left Locator) |
| driver → Cubase | CC 23, value ≥ 64, ch 1 | File > Save (M004 S01) |
| driver → Cubase | CC 24, absolute, ch 1 | Poly's editor: ≥ 64 open, < 64 close (M004 S04) |
| driver → Cubase | CC 25, absolute, ch 1 | Poly's plugin automation Write switch (M004 S07) |
| driver → Cubase | CC 26, absolute, ch 1 | Poly's plugin automation Read switch (M004 S07) |
| driver → Cubase | CC 27, absolute, ch 1 | Poly parameter index 8, lane 0 "Active", set to value/127 (M004 S07) |
| driver → Cubase | CC 28, value ≥ 64, ch 1 | File > Export Audio Mixdown (opens the dialog; M004 S06) |
| Cubase → driver | CC 117, ch 1 | CC 27's read-back: the parameter's value × 126, or 127 if it could not be resolved |
| driver → Cubase | CC 118, ch 1 | ready poll |
| Cubase → driver | CC 119, value 127, ch 1 | ready ping (the reply to a poll) |

The M004 rows (`tests/cubase/driver/remote.py` sends them) act on **the first
instrument channel in the MixConsole** -- track 1, Poly, in every fixture --
rather than on the selected track, because the fixtures save with a probe
track selected. Two facts found building them on the runner (Cubase 14,
2026-10-07):

- **Direct-access tags are Cubase's numbering, not Poly's ParamIDs.** Lane 0's
  "Active" (ParamID 8) is tag 4209. The script resolves the tag from the
  parameter *index*, which does follow Poly's registration order, and checks
  the title before setting anything. The parameters live on the instrument
  slot's one child object; the slot object itself carries only Freeze,
  Activate Output and Extract Sound.
- **The CC 27 handler needs `page.mOnActivate`** for the active mapping that
  direct access takes. The warning below is about sending the ready ping from
  that callback; capturing the mapping there has worked in every unattended
  session `scripts/cubase/start-session.ps1` launched on the runner, which is
  the configuration the nightly uses.

Channel 1 is the API's channel index `0`. CC 119 is undefined in General MIDI,
so it is a safe sentinel that will not collide with musical CC traffic.

The ready ping is sent from the **device driver's** `driver.mOnActivate`
callback, which fires when the surface *connects* (the `poly-test` port pair is
detected and bound). It emits `sendMidi(activeDevice, [0xB0, 119, 127])`, and the
driver's bounded `wait_for_ready` blocks until it sees that ping.

> **Do not bind the ping to `page.mOnActivate`.** The mapping page's activation
> fires only when Cubase makes that page the *active* page — which needs the MIDI
> Remote surface focused/selected in the UI. On an unattended runner the page is
> never activated, so a page-bound ping never sends and the driver times out. The
> driver-level hook fires on connection, which is what a headless run gets.

## Verification

- **Dev machine:** `node --check tests/cubase/midi-remote/JkDigital_PolyTest.js`
  confirms the script parses. The `midiremote_api_v1` module only exists inside
  Cubase, so the script cannot be *executed* off-host — parse-clean plus a
  constants match against the driver is the authorable-here proof.
- **Runner (owner, per R10):** load the script in Cubase 14 and confirm it
  appears in the MIDI Remote tab and emits the ready ping when the `poly-test`
  port connects.

## Cross-references

- `tests/cubase/driver/play_scenario.py` — the driver whose constants must match.
- `scripts/cubase/wait-for-ready.ps1` — consumes the ready ping on the runner.
- `tests/cubase/fixtures/README.md` — the fixture this transport drives.
