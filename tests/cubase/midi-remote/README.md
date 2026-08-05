---
class: gated
---

# Poly Test — Cubase MIDI Remote Script

`poly-transport.js` is the Cubase MIDI Remote driver script that lets the
headless test driver control Cubase's transport over a virtual MIDI port, and
signals readiness back to the runner. It replaces S07's placeholder
window-present readiness check with a real "ready" ping emitted when the script
activates.

## Install path (runner, Cubase 14)

Cubase discovers MIDI Remote driver scripts under the user Documents tree:

```
<Documents>/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/Jk Digital/Poly Test/poly-transport.js
```

The `<vendor>/<device>/` segments must match the `makeDeviceDriver('Jk Digital',
'Poly Test', ...)` call in the script. Copy `poly-transport.js` there, then in
Cubase open the MIDI Remote tab — the script auto-loads when the `poly-test`
virtual port pair is present.

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
| Cubase → driver | CC 119, value 127, ch 1 | ready ping (sent on script activation) |

Channel 1 is the API's channel index `0`. CC 119 is undefined in General MIDI,
so it is a safe sentinel that will not collide with musical CC traffic.

The ready ping is sent from the script's `mOnActivate` callback: when Cubase
loads the script and the port connects, it emits
`sendMidi(context, [0xB0, 119, 127])`. `wait-for-ready.ps1` (or the driver's
own bounded wait) blocks until it sees that ping, then proceeds; a bounded
timeout still guards against a script that never loads.

## Verification

- **Dev machine:** `node --check tests/cubase/midi-remote/poly-transport.js`
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
