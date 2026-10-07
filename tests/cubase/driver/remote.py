"""Drive the PolyTest MIDI Remote surface beyond the transport (M004).

``play_scenario.py`` starts and stops Cubase's transport. The M004 specs need
the host to do three more things that Cubase has no CLI for, and each is a
binding in ``tests/cubase/midi-remote/JkDigital_PolyTest.js``:

    editor open|close   CC 24 -- open or close Poly's plugin editor   (S04)
    save                CC 23 -- File > Save on the open project      (S01)
    automate            CC 25/26/27 -- write an automation lane for lane 0's
                        "Active" and read it back                     (S07)
    export              CC 28 -- open File > Export Audio Mixdown     (S06);
                        scripts/cubase/export-audio-mixdown.ps1 presses its
                        Export Audio button, which no binding can reach

Every command first runs the same ready handshake as the transport driver, so
a surface that has not connected fails loud here rather than silently dropping
the CC. Protocol constants are the contract shared with the script -- keep the
two in sync.

Exit codes follow play_scenario.py: 0 ok, 1 runtime failure, 2 no ready ping,
3 port not found.
"""

import argparse
import os
import sys
import time

from play_scenario import (
    CC_LOCATE,
    CC_START,
    CC_STOP,
    CHANNEL,
    DEFAULT_BEATS_PER_BAR,
    DEFAULT_READY_TIMEOUT_S,
    PORT_NAME,
    STOP_DRAIN_SECONDS,
    TAIL_SECONDS,
    find_port,
    log,
    scenario_seconds,
    wait_for_ready,
)

# --- Protocol constants (must match JkDigital_PolyTest.js) ---
CC_SAVE = 23
CC_EDITOR = 24
CC_AUTO_WRITE = 25
CC_AUTO_READ = 26
CC_PARAM = 27
CC_EXPORT = 28
CC_PARAM_ECHO = 117
PARAM_ECHO_FAILED = 127
ON = 127
OFF = 0

# Settling time after a host-side change before the next command. The editor
# needs longer: closing it tears down the WebView2 browser process, and opening
# it boots a new one.
SETTLE_SECONDS = 0.5
EDITOR_SETTLE_SECONDS = 2.0
# How long a save is given before the command returns. The caller confirms the
# save by the project file's mtime; this only keeps the CC from racing a quit.
SAVE_SETTLE_SECONDS = 3.0
# A parameter change sent this soon after transport start is written at the
# head of the lane. It must be sent while rolling -- Cubase writes automation
# only during playback -- and a lane's first point is extended backwards to
# the start, so this one point decides the lane's value before the change.
LANE_HEAD_SECONDS = 0.1


def automation_schedule(bars, tempo, beats_per_bar, off_at_bar, flat):
    """The write pass, as ``(seconds_after_start, cc, value)`` triples.

    Lane 0 is switched on at the head of the lane and off at the downbeat of
    ``off_at_bar`` (1-based). ``flat`` omits the switch-off, which leaves a
    lane that never changes -- the red path a spec selects with
    ``POLY_E2E_MUTATE=s07-flatten-lane``.
    """
    if not 1 < off_at_bar <= bars:
        raise ValueError(f"off_at_bar must be in (1, {bars}], got {off_at_bar}")
    schedule = [(LANE_HEAD_SECONDS, CC_PARAM, ON)]
    if not flat:
        off_s = scenario_seconds(off_at_bar - 1, tempo, beats_per_bar)
        schedule.append((off_s, CC_PARAM, OFF))
    return schedule


def send(outport, cc, value):
    import mido

    outport.send(mido.Message("control_change", channel=CHANNEL, control=cc, value=value))


def play_pass(outport, play_s, during=()):
    """Locate, start, run ``during`` on its schedule, play out, stop."""
    send(outport, CC_LOCATE, ON)
    send(outport, CC_START, ON)
    start = time.monotonic()
    for at_s, cc, value in during:
        delay = start + at_s - time.monotonic()
        if delay > 0:
            time.sleep(delay)
        send(outport, cc, value)
    remaining = start + play_s + TAIL_SECONDS - time.monotonic()
    if remaining > 0:
        time.sleep(remaining)
    send(outport, CC_STOP, ON)
    time.sleep(STOP_DRAIN_SECONDS)


def param_echoes(inport):
    """Drain the input and return every CC_PARAM_ECHO value seen."""
    return [
        m.value
        for m in inport.iter_pending()
        if m.type == "control_change" and m.control == CC_PARAM_ECHO
    ]


def run_automate(args, inport, outport):
    """Three passes: a baseline, a write pass, and a read pass.

    The baseline is captured before any automation exists, so the spec can
    compare the read pass against what Poly plays untouched. The write pass
    plays live with W on; the read pass plays the lane back with W off and R on,
    and is the one the assertion is about.
    """
    play_s = scenario_seconds(args.bars, args.tempo, args.beats_per_bar)
    schedule = automation_schedule(
        args.bars, args.tempo, args.beats_per_bar, args.off_at_bar, args.flat
    )

    log("automate", "pass 1/3: baseline, no automation")
    play_pass(outport, play_s)

    # The knob's starting value is not known, and a binding only fires on a
    # change, so step it off and back to a known OFF before arming W. Nothing
    # is written while stopped. The plugin's own value is restored to ON by the
    # first scheduled point, which lands while rolling.
    send(outport, CC_PARAM, ON)
    time.sleep(SETTLE_SECONDS)
    send(outport, CC_PARAM, OFF)
    time.sleep(SETTLE_SECONDS)
    echoes = param_echoes(inport)
    if PARAM_ECHO_FAILED in echoes or not echoes:
        log("error", f"lane-0 Active could not be driven (echoes={echoes})")
        return 1
    send(outport, CC_AUTO_READ, ON)
    send(outport, CC_AUTO_WRITE, ON)
    time.sleep(SETTLE_SECONDS)

    log("automate", f"pass 2/3: write, schedule={schedule}")
    play_pass(outport, play_s, during=schedule)
    send(outport, CC_AUTO_WRITE, OFF)
    time.sleep(SETTLE_SECONDS)
    log("automate", f"write pass echoes={param_echoes(inport)}")

    log("automate", "pass 3/3: read the lane back")
    play_pass(outport, play_s)
    return 0


def run(args):
    import mido

    in_name = find_port(mido.get_input_names(), PORT_NAME)
    out_name = find_port(mido.get_output_names(), PORT_NAME)
    if in_name is None or out_name is None:
        log("error", f"port {PORT_NAME!r} not found")
        return 3
    with mido.open_input(in_name) as inport, mido.open_output(out_name) as outport:
        if not wait_for_ready(inport, outport, args.ready_timeout):
            log("error", "no ready ping")
            return 2
        log("ready-received", f"command={args.command}")
        if args.command == "editor":
            send(outport, CC_EDITOR, ON if args.state == "open" else OFF)
            time.sleep(EDITOR_SETTLE_SECONDS)
            code = 0
        elif args.command == "export":
            send(outport, CC_EXPORT, ON)
            time.sleep(SETTLE_SECONDS)
            code = 0
        elif args.command == "save":
            send(outport, CC_SAVE, ON)
            time.sleep(SAVE_SETTLE_SECONDS)
            code = 0
        else:
            code = run_automate(args, inport, outport)
        log("done", f"command={args.command} code={code}")
        sys.stdout.flush()
        # Same reason as play_scenario.py: rtmidi can hang closing the port.
        os._exit(code)


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Drive the PolyTest MIDI Remote surface.")
    parser.add_argument("--ready-timeout", type=float, default=DEFAULT_READY_TIMEOUT_S)
    sub = parser.add_subparsers(dest="command", required=True)
    editor = sub.add_parser("editor", help="open or close Poly's plugin editor")
    editor.add_argument("state", choices=["open", "close"])
    sub.add_parser("save", help="File > Save on the open project")
    sub.add_parser("export", help="open File > Export Audio Mixdown")
    auto = sub.add_parser("automate", help="write and read back a lane-0 Active lane")
    auto.add_argument("--bars", type=int, default=4)
    auto.add_argument("--tempo", type=float, default=120.0)
    auto.add_argument("--beats-per-bar", type=int, default=DEFAULT_BEATS_PER_BAR)
    auto.add_argument("--off-at-bar", type=int, default=3)
    auto.add_argument("--flat", action="store_true", help="write a lane that never changes")
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
