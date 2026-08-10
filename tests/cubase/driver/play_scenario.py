"""Headless Cubase transport driver for the Poly nightly (M042 S08).

Opens the ``poly-test`` loopMIDI virtual port pair, polls the MIDI Remote
surface until it replies with the ready ping, starts Cubase transport, lets a
fixed number of bars play, stops transport, and exits. The poly_midi_probe
plugin captures Poly's output and flushes it to ``POLY_PROBE_OUTPUT`` on Cubase
deactivate; this driver only controls the transport.

Readiness is a driver-initiated handshake: this driver sends CC_POLL repeatedly
and the script replies with CC_READY. This removes the launch-vs-listen ordering
race that a one-shot activation ping had (the ping could fire before the driver
opened its input port, so it was never seen).

Protocol constants are the contract shared with
``tests/cubase/midi-remote/JkDigital_PolyTest.js`` -- keep the two in sync.

Phase logging goes to stdout as structured lines (``[driver:<phase>] ...``) so a
runner-side failure is diagnosable from the captured job log without shelling in.

Exit codes:
    0  scenario played and stopped cleanly
    1  runtime failure (see the logged phase)
    2  timed out waiting for the ready ping
    3  the ``poly-test`` port was not found
"""

import argparse
import sys
import time

# --- Protocol constants (must match JkDigital_PolyTest.js) ---
CHANNEL = 0  # MIDI channel 1 == 0-based channel index 0
CC_START = 20
CC_STOP = 21
CC_LOCATE = 22
CC_POLL = 118  # undefined CC in GM -- driver's "are you live?" poll (OUT)
CC_READY = 119  # undefined CC in GM -- ready-ping sentinel (IN, script's reply)
READY_VALUE = 127
# The poll ALTERNATES 127 / 0 rather than repeating a constant value. A Cubase
# MIDI Remote button surface value deduplicates same-value CCs — a second CC 127
# to a button already holding 127 is not a "change", so mOnProcessValueChange
# would not fire and only the first poll after connect would get a reply
# (re-introducing the one-shot race). Alternating forces every poll cycle to be a
# genuine value change: the 127 is a rising edge the script replies to, the 0 is
# the reset that makes the next 127 a fresh change. The script guards on
# value > 0 so only the 127 half triggers a reply — exactly one reply per cycle.
POLL_VALUE_HIGH = 127
POLL_VALUE_LOW = 0
PORT_NAME = "poly-test"  # loopMIDI virtual port pair name (substring match)

# --- Scenario defaults ---
DEFAULT_BARS = 4
DEFAULT_TEMPO_BPM = 120.0
DEFAULT_BEATS_PER_BAR = 4
DEFAULT_READY_TIMEOUT_S = 30.0
# How often the driver sends a poll CC while waiting for the reply. Because the
# poll alternates 127 / 0, only every OTHER send is a reply-triggering rising
# edge, so the effective rising-edge cadence is 2x this (~0.5s at 0.25s).
POLL_INTERVAL_S = 0.25
# Small guard so transport-stop is sent after the last bar's notes are emitted
# rather than clipping the final beat.
TAIL_SECONDS = 0.5


def log(phase, message):
    """Emit a structured phase line to stdout."""
    print(f"[driver:{phase}] {message}", flush=True)


def scenario_seconds(bars, tempo_bpm, beats_per_bar):
    """Wall-clock duration of ``bars`` bars at ``tempo_bpm``."""
    beats = bars * beats_per_bar
    return beats * (60.0 / tempo_bpm)


def find_port(candidates, name):
    """Return the first port whose name contains ``name`` (case-insensitive)."""
    lowered = name.lower()
    for candidate in candidates:
        if lowered in candidate.lower():
            return candidate
    return None


def wait_for_ready(inport, outport, timeout_s, verbose=False):
    """Poll the MIDI Remote surface until it replies with the ready ping.

    Readiness is driver-initiated: this loop sends CC_POLL out every
    ``POLL_INTERVAL_S`` and blocks until the script replies with
    CC_READY=READY_VALUE (or ``timeout_s`` elapses). Polling removes the
    launch-vs-listen ordering race that a one-shot activation ping had — whenever
    a poll lands after the surface is connected, the reply arrives within one
    poll interval regardless of who started first.

    Returns True on ready, False on timeout. Non-ready messages are drained and
    ignored so a stale buffer can't mask the real ping.

    When ``verbose`` is set, every incoming MIDI message is logged as it arrives
    (M042 S08 ready-ping debug). This distinguishes "nothing on the wire"
    (surface never connected / poll never received) from "CC119 arrives but is
    filtered out" (channel or value mismatch) — the two failures produce the
    identical timeout otherwise.
    """
    import mido  # deferred like run()'s import so --help works without mido

    deadline = time.monotonic() + timeout_s
    next_poll = 0.0  # send the first poll immediately
    # Alternate 127 / 0 so each poll is a genuine value change the button can't
    # dedup (see POLL_VALUE_HIGH/LOW). Start high so the very first poll is a
    # rising edge the script replies to.
    poll_value = POLL_VALUE_HIGH
    while time.monotonic() < deadline:
        now = time.monotonic()
        if now >= next_poll:
            outport.send(
                mido.Message(
                    "control_change", channel=CHANNEL, control=CC_POLL, value=poll_value
                )
            )
            if verbose:
                log("poll", f"sent CC{CC_POLL}={poll_value}")
            poll_value = POLL_VALUE_LOW if poll_value == POLL_VALUE_HIGH else POLL_VALUE_HIGH
            next_poll = now + POLL_INTERVAL_S
        for msg in inport.iter_pending():
            if verbose:
                log("rx", str(msg))
            if (
                msg.type == "control_change"
                and msg.control == CC_READY
                and msg.value == READY_VALUE
            ):
                return True
        time.sleep(0.02)
    return False


def run(args):
    import mido  # imported here so --help works without mido installed

    log("start", f"bars={args.bars} tempo={args.tempo} port={PORT_NAME!r}")

    in_names = mido.get_input_names()
    out_names = mido.get_output_names()
    if args.verbose:
        log("ports", f"inputs={in_names} outputs={out_names}")
    in_name = find_port(in_names, PORT_NAME)
    out_name = find_port(out_names, PORT_NAME)
    if in_name is None or out_name is None:
        log(
            "error",
            f"port {PORT_NAME!r} not found. inputs={in_names} outputs={out_names}",
        )
        return 3

    with mido.open_input(in_name) as inport, mido.open_output(out_name) as outport:
        log("port-open", f"in={in_name!r} out={out_name!r}")

        if not wait_for_ready(inport, outport, args.ready_timeout, verbose=args.verbose):
            log(
                "error",
                f"no ready ping (CC{CC_READY}={READY_VALUE}) within "
                f"{args.ready_timeout}s",
            )
            return 2
        log("ready-received", "MIDI Remote script is live")

        # Locate to the scenario start, then start transport.
        outport.send(
            mido.Message(
                "control_change", channel=CHANNEL, control=CC_LOCATE, value=127
            )
        )
        outport.send(
            mido.Message(
                "control_change", channel=CHANNEL, control=CC_START, value=127
            )
        )
        play_s = scenario_seconds(args.bars, args.tempo, args.beats_per_bar)
        log("scenario-start", f"playing {args.bars} bars (~{play_s:.2f}s)")

        time.sleep(play_s + TAIL_SECONDS)

        outport.send(
            mido.Message(
                "control_change", channel=CHANNEL, control=CC_STOP, value=127
            )
        )
        log("scenario-end", "transport stopped")

    log("done", "clean exit")
    return 0


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Drive Cubase transport for the Poly nightly.")
    parser.add_argument("--bars", type=int, default=DEFAULT_BARS, help="bars to play")
    parser.add_argument(
        "--tempo", type=float, default=DEFAULT_TEMPO_BPM, help="tempo in BPM"
    )
    parser.add_argument(
        "--beats-per-bar",
        type=int,
        default=DEFAULT_BEATS_PER_BAR,
        help="beats per bar (time-signature numerator)",
    )
    parser.add_argument(
        "--ready-timeout",
        type=float,
        default=DEFAULT_READY_TIMEOUT_S,
        help="seconds to wait for the ready ping before failing loud",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="log the input/output port lists and every incoming MIDI message "
        "during the ready-ping wait (M042 S08 diagnostic)",
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    try:
        return run(args)
    except ImportError as exc:
        log("error", f"mido/python-rtmidi not installed: {exc}")
        return 1
    except Exception as exc:  # noqa: BLE001 - fail loud with the phase context
        log("error", f"unexpected: {exc}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
