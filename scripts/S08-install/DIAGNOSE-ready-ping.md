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

## Root cause (confirmed on the runner 2026-08-09)

Manual testing on the runner proved the ready ping *works* — with a listener
opened **first**, then Cubase launched, the `CC 119 value 127` ping arrived on
schedule. The failure was a **one-shot ordering race**, not a broken emit:

- The old script emitted the ping once, from `driver.mOnActivate`, which fires at
  surface-connect time — **during Cubase load**.
- The nightly's ordering is launch → wait-for-ready → *then* start
  `play_scenario.py`. By the time the driver opened its input port, the single
  fire-and-forget ping was long gone (mido only buffers messages after the port
  is open), so the driver waited on a ping that had already fired.

## The fix (deployed): a driver-initiated poll/reply handshake

Readiness is now **driver-initiated** so ordering can't matter:

- **Driver** (`play_scenario.py`): `wait_for_ready` sends a `CC_POLL` (CC 118)
  each loop iteration while waiting, alternating value **127 / 0** so the Cubase
  button surface can't dedup a constant value into a one-shot. It still blocks on
  `CC_READY` (CC 119) value 127.
- **Script** (`JkDigital_PolyTest.js`): the `mOnActivate` ping is **removed**. A
  `pollButton` bound to `CC_POLL` replies with the ready ping in
  `mOnProcessValueChange`, guarded on `value > 0` (so only the 127-half of each
  poll cycle replies).

Whenever a poll lands after the surface is connected, the reply arrives within one
poll interval — robust to launch-then-listen, listen-then-launch, and slow cold
Cubase loads alike.

This is a scratch runbook for verifying that fix on the runner. Run the tests
below **in order, stop at the first that fails**. Delete this file once the
nightly is green.

> **Port contention note (read first).** loopMIDI's `poly-test` endpoints can be
> held open by only one app at a time. Don't run a raw monitor and the driver at
> once — they fight over the port. Run one listener at a time.

---

## Step 0 — Find the exact port names mido sees (do this first)

Do NOT guess the port name. loopMIDI's OS-level name (`poly-test`, what Cubase
shows) is **not** what mido opens by — python-rtmidi appends a client/index
number, so the real name is something like `'poly-test 1'` (with a space and a
digit), and the exact suffix differs per box. On this runner it enumerated as
`poly-test 0` (**input**) / `poly-test 1` (**output**). Ask mido directly:

```powershell
cd C:\poly
python -c "import mido; print('INPUTS:'); [print(repr(n)) for n in mido.get_input_names()]; print('OUTPUTS:'); [print(repr(n)) for n in mido.get_output_names()]"
```

`repr()` is deliberate — it shows quotes, trailing spaces, and capitalization, so
you copy the *exact* string. Read the result:

- **An entry containing `poly-test` appears in both lists** → good. Copy those
  exact strings for any manual `open_input(...)`/`open_output(...)` below. The
  driver itself matches by substring (`find_port`), so `play_scenario.py` needs no
  name edit — it binds whatever contains `poly-test`.
- **`poly-test` appears in NEITHER list** → loopMIDI isn't running or the port was
  never created. Open the loopMIDI tray app and confirm a `poly-test` port exists.
  Nothing downstream can work until it shows up here.

Whenever a manual command below names a port, substitute the exact name Step 0
printed on *your* box. On this runner the **input** (what the driver listens on)
is `poly-test 0` and the **output** (what the driver sends polls on) is
`poly-test 1`.

---

## Test 1 — Does the whole handshake work end-to-end? (start here)

This is the fastest way to confirm the fix. Launch Cubase first, let it fully
settle, and *then* run the driver by hand with `--verbose` — the exact ordering
that failed before the poll/reply fix.

```powershell
# Window A — launch Cubase 14 with the fixture, by hand. Wait until it settles.
cd C:\poly
scripts\cubase\launch-cubase.ps1 -CubaseVersion 14 -FixtureCpr tests\cubase\fixtures\poly-4bar.cpr
```

```powershell
# Window B — once Cubase is up, run the driver. --verbose logs polls + every rx.
cd C:\poly
python tests\cubase\driver\play_scenario.py --bars 4 --tempo 120 --ready-timeout 60 --verbose
```

- **`[driver:ready-received]` prints, then the scenario plays and stops** → the
  handshake works headless. This is the success case — the nightly should now pass.
  Move on to stripping the temp instrumentation (see the end of this file).
- **Still times out** → the poll isn't producing a reply. Go to Test 2 to split
  the two halves apart.

`--verbose` prints `[driver:ports]` (the mido name lists), a poll line each cycle,
and a `[driver:rx]` line for every incoming message — so you can see whether any
`CC119=127` reply comes back at all.

---

## Test 2 — Split the handshake: does the poll arrive, does the reply come back?

Two consoles tell the two halves apart:

1. **Cubase's Script Console** (Studio → MIDI Remote Manager → Scripting Tools /
   Console) — the script's `console.log` lines (deployed as temp instrumentation).
2. **The driver's `--verbose` stdout** from Test 1's Window B.

Read them together:

- **Script console shows `[poly-remote] poll received … replying ready ping`** but
  the driver's `--verbose` never shows `CC119=127` coming back → the reply is sent
  but not reaching the driver's input. Check that the driver opened the loopback
  side that Cubase's output feeds (compare its `[driver:port-open] in=…` line to
  the Step 0 enumeration).
- **Script console shows nothing** (no `poll received` line) → the surface isn't
  receiving the poll. Either the surface never connected (go to Test 3) or the
  poll is landing on a port Cubase isn't bound to (compare the driver's
  `[driver:port-open] out=…` to Step 0).
- **Script console shows `poll received` on only the *first* poll, then silence**
  → the button is deduping same-value CCs after all. The driver already alternates
  127/0 to prevent this; if you still see it, confirm the runner pulled the
  `ec2dc25` alternation commit.

---

## Test 3 — Did the MIDI Remote surface actually connect?

If Test 2 shows the poll never reaching the script, the surface may not be bound.
`mOnProcessValueChange` (and any binding) only works once Cubase **detects and
binds** the port pair via the `expectInputNameContains('poly-test')` /
`expectOutputNameContains(...)` detection unit.

Open **Studio → MIDI Remote Manager** (via VNC). Look for `JkDigital / PolyTest`:

- **Green / connected** → binding is fine; the problem is port routing, not
  connection (back to Test 2's port comparison).
- **Disconnected / absent** → the detection unit didn't bind. Causes, in order of
  likelihood on a headless box:
  1. **Script not installed / wrong path.** Cubase only loads a driver script at
     the strict path
     `…\MIDI Remote\Driver Scripts\Local\JkDigital\PolyTest\JkDigital_PolyTest.js`.
     Re-run `scripts\S08-install\3-install-midi-remote.ps1` and confirm the file
     landed there.
  2. **Port name mismatch.** loopMIDI must enumerate a port whose name *contains*
     `poly-test` (Step 0 confirms this). If the port is named something else,
     rename it in loopMIDI or the substring matcher won't bind.
  3. **loopMIDI port contention.** If another app is holding a `poly-test`
     endpoint open, Cubase may fail to bind it. Close other listeners and relaunch.

---

## When the flow is green: strip the temp instrumentation

The following are temporary M042 S08 diagnostics — remove them once the nightly
passes headless, then delete this file:

- The `console.log` lines in `JkDigital_PolyTest.js`'s poll-reply handler.
- The `--verbose` flag in `play_scenario.py` (optional to keep — it's inert unless
  passed, and the nightly doesn't pass it).

The poll/reply handshake itself (CC_POLL binding, alternating driver poll) is the
permanent fix and **stays**.

---

## Cross-references

- `tests/cubase/midi-remote/JkDigital_PolyTest.js` — the poll/reply handshake + port contract.
- `tests/cubase/midi-remote/README.md` — MIDI Remote script + CC map.
- `tests/cubase/driver/play_scenario.py` — the mido driver and its `wait_for_ready` poll loop.
- `scripts/cubase/launch-cubase.ps1` / `wait-for-ready.ps1` — the by-hand launch commands.
- `.github/workflows/cubase-nightly.yml` — the nightly this diagnoses.
