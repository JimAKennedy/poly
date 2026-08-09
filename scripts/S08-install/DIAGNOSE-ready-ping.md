---
class: gated
---

# Diagnosing the "no ready ping" nightly failure

The Cubase nightly (`.github/workflows/cubase-nightly.yml`) gets all the way
through build, tests, plugin install, Cubase launch, and "Wait for Cubase ready",
then fails at **"Play scenario (mido driver)"** with:

```
[driver:start] bars=4 tempo=120.0 port='poly-test'
[driver:port-open] in='poly-test 0' out='poly-test 1'
[driver:error] no ready ping (CC119=127) within 30.0s
```

The whole armed-transport flow hinges on ONE observable: **does `CC 119 value 127`
reach the `poly-test` output port after Cubase loads the MIDI Remote surface?**
The MIDI Remote script (`tests/cubase/midi-remote/JkDigital_PolyTest.js`) emits it
from `driver.mOnActivate`; the driver (`tests/cubase/driver/play_scenario.py`)
blocks on it in `wait_for_ready`.

Three things can independently break it. Run the tests below **on the runner, in
order, and stop at the first that fails** — each one splits a different half of
the flow apart. This is a scratch runbook; delete the file once the flow is green.

> **Port contention note (read first).** loopMIDI's `poly-test` output can be held
> open by only one app at a time. If you run the raw monitor in Test 1 **and** the
> driver at the same time, they fight over the port. Run one listener at a time.

---

## Step 0 — Find the exact port names mido sees (do this first)

Do NOT guess the port name. loopMIDI's OS-level name (`poly-test`, what Cubase
shows) is **not** what mido opens by — python-rtmidi appends a client/index number,
so the real name is something like `'poly-test 1'` (with a space and a digit), and
the exact suffix differs per box (loopMIDI instance count, rtmidi version). On the
CI runner it enumerated as `poly-test 0` (input) / `poly-test 1` (output); your box
may differ. Ask mido directly:

```powershell
cd C:\poly
python -c "import mido; print('INPUTS:'); [print(repr(n)) for n in mido.get_input_names()]; print('OUTPUTS:'); [print(repr(n)) for n in mido.get_output_names()]"
```

`repr()` is deliberate — it shows quotes, trailing spaces, and capitalization, so
you copy the *exact* string. Read the result:

- **An entry containing `poly-test` appears in both lists** → good. Copy those
  exact strings for any manual `open_input(...)`/`open_output(...)` below. The
  driver itself matches by substring (`find_port`), so `play_scenario.py` needs no
  name edit — it will bind whatever contains `poly-test`.
- **`poly-test` appears in NEITHER list** → loopMIDI isn't running or the port was
  never created. Open the loopMIDI tray app and confirm a `poly-test` port exists.
  Nothing downstream can work until it shows up here.

Whenever a manual command below says `open_input('poly-test 1')`, substitute the
exact name Step 0 printed on *your* box.

---

## Test 1 — Is the ping emitted at all?

Isolates the plugin/Cubase/MIDI-Remote side from the driver side. Open a raw MIDI
monitor on the loopback input the script pings, **before** launching Cubase.
`play_scenario.py` reads input `poly-test 1`, so that is what to watch.

```powershell
# Window A — raw monitor, leave running. Ctrl+C to stop.
cd C:\poly
python -c "import mido; p=mido.open_input('poly-test 1'); print('listening on poly-test 1'); [print(m) for m in p]"
```

```powershell
# Window B — launch Cubase 14 with the fixture, by hand.
cd C:\poly
scripts\cubase\launch-cubase.ps1 -CubaseVersion 14 -FixtureCpr tests\cubase\fixtures\poly-4bar.cpr
```

- **`control_change channel=0 control=119 value=127` prints in Window A** → the
  emit side works; the ping is on the wire. The bug is in the driver's wait —
  skip to **Test 3**.
- **Nothing prints** → `driver.mOnActivate` never fired, or the detection unit
  never bound the port pair. That is the real failure — go to **Test 2**.

---

## Test 2 — Did the MIDI Remote surface actually connect? (most likely culprit)

`driver.mOnActivate` fires only when Cubase **detects and binds** the port pair
via the `expectInputNameContains('poly-test')` / `expectOutputNameContains(...)`
detection unit. If the surface never connects, the hook never runs and no ping is
sent — the exact symptom we see.

**2a. Check the binding in the UI (via VNC).** Open **Studio → MIDI Remote
Manager**. Look for `JkDigital / PolyTest`:

- **Green / connected** → binding is fine; the problem is elsewhere (re-check
  Test 1's monitor was on `poly-test 1`, then go to Test 3).
- **Disconnected / absent** → the detection unit didn't bind. Causes, in order of
  likelihood on a headless box:
  1. **Script not installed / wrong path.** Cubase only loads a driver script at
     the strict path
     `…\MIDI Remote\Driver Scripts\Local\JkDigital\PolyTest\JkDigital_PolyTest.js`.
     Re-run `scripts\S08-install\3-install-midi-remote.ps1` and confirm the file
     landed there.
  2. **Port name mismatch.** loopMIDI must enumerate a port whose name *contains*
     `poly-test`. Confirm the port exists and its name:
     ```powershell
     python -c "import mido; print('IN ', mido.get_input_names()); print('OUT', mido.get_output_names())"
     ```
     Both lists must contain an entry with `poly-test` in the name. If the port is
     named something else, rename it in loopMIDI (or the substring matcher won't
     bind).
  3. **loopMIDI port contention.** If Window A's monitor from Test 1 is still
     holding `poly-test 1` open, Cubase may fail to open the same endpoint. Close
     the monitor and relaunch Cubase before reading the Manager.

**2b. Prove the hook fires (log line already deployed).** The activation hook in
`JkDigital_PolyTest.js` already carries a diagnostic `console.log` (committed as
temp instrumentation in `da61059`), so you don't need to edit it — just make sure
the runner has the current script installed (`3-install-midi-remote.ps1`), relaunch
Cubase, and watch Cubase's **Script Console** (Studio → MIDI Remote Manager →
Scripting Tools / Console):

```javascript
// tests/cubase/midi-remote/JkDigital_PolyTest.js — already in the branch
driver.mOnActivate = function (activeDevice) {
    console.log('[poly-remote] mOnActivate fired — sending ready ping')
    midiOutput.sendMidi(activeDevice, [0xB0 + CHANNEL, CC_READY, READY_VALUE])
}
```

- **`[poly-remote] mOnActivate fired` appears, but no CC on the wire** → the hook
  fires; the problem is the `sendMidi` call or the output port it targets (wrong
  `activeDevice`/port).
- **The log line never appears** → binding, not the ping. Return to 2a.

The `console.log` (and the driver's `--verbose`) are temporary instrumentation —
strip them once the flow is green (this whole file goes away then too).

---

## Test 3 — Does the driver see it? (only if Test 1 printed the CC)

If the CC *is* on the wire but the driver still times out, the bug is in
`play_scenario.py`'s `wait_for_ready`. It filters on
`msg.control == 119 and msg.value == 127` on the input whose name contains
`poly-test`. Run just the driver against an already-running, already-activated
Cubase from Test 1 (close Window A's monitor first — port contention), and use the
**`--verbose`** flag so it logs the port lists it saw and **every** incoming MIDI
message during the wait:

```powershell
cd C:\poly
python tests\cubase\driver\play_scenario.py --bars 4 --tempo 120 --ready-timeout 60 --verbose
```

`--verbose` prints `[driver:ports]` (the full input/output name lists mido
enumerated) and a `[driver:midi-in]` line for each message that arrives while
waiting — so you can see whether CC119 shows up at all and, if it does, on which
channel/value.

- **`[driver:ready-received]` prints** → the flow works end-to-end; the nightly's
  30s timeout may just be too tight for a cold Cubase load. Consider bumping
  `--ready-timeout` in the workflow step.
- **`[driver:midi-in]` shows CC119 but it still times out** → a value/channel
  mismatch in the filter; compare the logged message to the expected
  `control=119 value=127`.
- **`[driver:midi-in]` shows nothing (or no CC119) while Window A saw the CC** → the
  driver opened a different input port than the monitor. Compare its
  `[driver:port-open] in=...` line to where the CC appeared in Test 1.

---

## Timing subtlety: the ping can fire before the driver is listening

`mOnActivate` fires **once**, at surface-connect time — which on the runner
happens during/just after Cubase load. If Cubase is launched and fully settled
*before* `play_scenario.py` opens its input port, the single ping may already have
been sent and missed (mido only buffers messages after the port is open).

The nightly's ordering (launch → wait-for-ready → run driver) makes this
plausible. If Test 1 shows the CC firing exactly once at activation but Test 3
(driver started afterward) misses it, this is the bug — the fix is to make the
ping **repeat** (e.g. emit on a short timer until acknowledged, or re-emit
periodically) rather than fire once. Note this if you see a one-shot ping in
Test 1 that the driver never catches.

---

## Cross-references

- `tests/cubase/midi-remote/JkDigital_PolyTest.js` — the ready-ping emit + port contract.
- `tests/cubase/midi-remote/README.md` — MIDI Remote script + CC map.
- `tests/cubase/driver/play_scenario.py` — the mido driver and its `wait_for_ready`.
- `scripts/cubase/launch-cubase.ps1` / `wait-for-ready.ps1` — the by-hand launch commands.
- `.github/workflows/cubase-nightly.yml` — the nightly this diagnoses.
