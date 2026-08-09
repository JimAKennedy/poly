---
class: gated
---

# Poly Cubase Transport Driver

`play_scenario.py` drives Cubase's transport headlessly for the Cubase-in-the-loop
nightly. It opens the `poly-test` loopMIDI virtual port, waits for the MIDI
Remote script's ready ping, plays a fixed number of bars, and stops.

## What it does

1. Opens the `poly-test` input and output ports (substring match on the port
   name, so loopMIDI's `-1`/`-2` suffixes are tolerated).
2. Waits (bounded, fails loud) for the ready ping —
   `CC 119 value 127` — that `JkDigital_PolyTest.js` emits on activation.
3. Sends **locate-to-zero** then **transport START** (CC 22, then CC 20).
4. Sleeps for the scenario duration (`bars × beats/bar × 60/tempo`, plus a small
   tail so the last beat's notes are captured).
5. Sends **transport STOP** (CC 21) and exits.

The driver controls transport only. Poly's MIDI output is captured downstream by
`poly_midi_probe`, which writes JSONL to `POLY_PROBE_OUTPUT` when Cubase
deactivates. The probe-vs-golden comparison is a separate step
(`tests/cubase/compare_probe_golden.py`).

## Protocol

Shared with `tests/cubase/midi-remote/JkDigital_PolyTest.js` — change both together.

| Direction | MIDI | Meaning |
|---|---|---|
| driver → Cubase | CC 20, value 127, ch 1 | transport START |
| driver → Cubase | CC 21, value 127, ch 1 | transport STOP |
| driver → Cubase | CC 22, value 127, ch 1 | LOCATE to zero |
| Cubase → driver | CC 119, value 127, ch 1 | ready ping |

## Usage

```
pip install -r requirements.txt
python play_scenario.py --bars 4 --tempo 120 --beats-per-bar 4
```

`--bars`, `--tempo`, and `--beats-per-bar` are parameterized so S09 and future
scenarios reuse the driver. Defaults are 4 bars at 120 BPM in 4/4, matching the
committed golden `tests/golden/processor_default_4bars.txt`.

### Exit codes

| Code | Meaning |
|---|---|
| 0 | scenario played and stopped cleanly |
| 1 | runtime failure (mido missing, unexpected error — see logged phase) |
| 2 | timed out waiting for the ready ping |
| 3 | `poly-test` port not found |

## Phase logging

Each phase logs a structured stdout line for diagnosis from the captured job
log: `start`, `port-open`, `ready-received`, `scenario-start`, `scenario-end`,
`done`, or `error`.

## Verification

- **Dev machine:** `python -m py_compile play_scenario.py` and `ruff check`
  confirm the driver is syntactically clean; `python play_scenario.py --help`
  exercises arg parsing without needing mido or a MIDI port (mido is imported
  lazily inside `run()`).
- **Runner (owner, per R10):** with loopMIDI's `poly-test` port and the MIDI
  Remote script loaded, `python play_scenario.py` plays the fixture and the
  probe JSONL lands at `POLY_PROBE_OUTPUT`.

## Cross-references

- `tests/cubase/midi-remote/JkDigital_PolyTest.js` — the MIDI Remote side of the
  protocol.
- `tests/cubase/fixtures/README.md` — the fixture this driver plays.
- `tests/cubase/compare_probe_golden.py` — diffs the resulting probe JSONL.
