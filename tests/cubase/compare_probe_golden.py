"""Compare poly_midi_probe JSONL against an in-process test-host golden (M042 S08).

The L4 Cubase run and the L3 in-process host produce the same logical note
stream for a given preset. This harness closes the L3<->L4 cross-check: it reads
the probe JSONL Cubase produced (``POLY_PROBE_OUTPUT``) and the committed golden
the in-process host produced, and diffs them field-by-field.

Field parity (established in S06, ``tests/host/probe_tests.cpp``): both sides
carry ``ppq``, ``pitch``, ``velocity``, ``channel`` per note-on. The probe JSONL
also emits note-offs; the golden is note-ons only, so we filter the probe to
note-ons before diffing.

Serialization formats differ:
  * probe JSONL  -- one JSON object per line, keys type/ppq/pitch/velocity/channel
  * golden .txt  -- ``# comment`` header lines, then whitespace columns
                    ``ppq  pitch  velocity  channel`` (note-ons only)

Tolerance policy (see the constants below):
  * pitch          -- exact integer equality. A wrong pitch is a real
    mapping defect, never jitter.
  * velocity       -- exact to within EPS_VELOCITY. The golden holds the
    engine's continuous float velocity; the probe carries what Cubase delivered.
    Measured on run 31441420365 across all 94 aligned note-ons, the probe is a
    near-constant +0.0015 higher than the golden (max |diff| 0.00187, mean
    0.00169) -- a uniform host velocity bias, NOT 7-bit quantization (which was
    tried and rejected: quantizing the golden to round(v*127)/127 made the max
    diff WORSE, 0.0057, because Cubase keeps sub-integer float precision). So we
    do NOT quantize; we widen EPS_VELOCITY to absorb the ~0.0015 host bias while
    still catching a real +/-1-MIDI-step error (~0.008). Re-tighten with
    --eps-velocity if a host proves velocity-exact.
  * ppq            -- exact to within EPS_PPQ. Both sides derive ppq from the
    same musical position; a real DAW transport is sample-accurate, so the
    default epsilon is tight (a small fraction of a 1/960 tick). Widen only with
    evidence of benign host jitter, and document why.
  * channel        -- NOT compared on the L4 DAW path (``--ignore-channel``,
    default on). A Cubase instrument track structurally coerces incoming MIDI to
    a single channel (all events arrive as channel 0), so the engine's per-lane
    channel {0,1,2,3} cannot survive routing through the DAW -- confirmed on run
    31441420365 (probe: all channel 0; golden: {0,1,2,3}) and again after setting
    the probe track's input channel to "Any". Channel fidelity at the engine
    boundary stays covered by the in-process ``ProbeChain`` test
    (``tests/host/probe_tests.cpp``), which drives engine->probe with no DAW in
    the middle. Pass ``--no-ignore-channel`` to re-enable the exact channel check
    for a host that does preserve channel.

Scenario-boundary trim: the driver plays ``TAIL_SECONDS`` (0.5s) past the last
bar so the final beat is not clipped, so the probe captures a few note-ons past
the scenario end. The golden is a 4-bar render whose LAST onset is the bar-5
downbeat at ppq 16.0 (4 note-ons), so the boundary is INCLUSIVE: keep probe
note-ons with ppq <= --max-ppq and drop only those strictly after it. Confirmed
on run 31441420365: probe had exactly 4 note-ons at ppq 16.0 matching the golden,
plus 3 true overspill onsets at ppq 16.25/16.5/16.71 that the trim drops.

On mismatch the harness prints the first N diverging events (index, field, both
values) rather than a bare pass/fail, so a runner-side failure is diagnosable
from the captured log without re-running.

Exit codes: 0 match, 1 mismatch, 2 usage/parse error.
"""

import argparse
import json
import sys
from dataclasses import dataclass

# Default tolerances. ppq is in quarter-note units; 1/960 tick ~= 0.00104, so
# 5e-4 is under half a tick -- tight enough to catch a one-tick shift.
EPS_PPQ = 5e-4
# Cubase applies a near-constant +0.0015 velocity bias (measured max 0.00187 on
# run 31441420365); 3e-3 absorbs that host bias with headroom while staying well
# under a single 1/127 MIDI step (~0.0079), so a real +/-1-step error still
# fails. See the module docstring for the measured distribution.
EPS_VELOCITY = 3e-3
# The 4-bar scenario boundary in quarter-note ppq (4 bars * 4 beats). The golden's
# last onset is the bar-5 downbeat AT ppq 16.0, so the boundary is inclusive:
# probe note-ons with ppq <= this are kept; only strictly-later overspill from
# the driver's TAIL_SECONDS overplay is dropped (see --max-ppq / trim_probe_tail).
DEFAULT_MAX_PPQ = 16.0
DEFAULT_MAX_DIVERGENCES = 10


@dataclass
class NoteOn:
    """One note-on event, normalized across both serialization formats."""

    ppq: float
    pitch: int
    velocity: float
    channel: int


class CompareError(Exception):
    """Raised on unparseable input (distinct from a content mismatch)."""


def trim_probe_tail(events, max_ppq):
    """Drop probe note-ons strictly after ``max_ppq`` (the scenario boundary).

    The driver plays past the last bar (TAIL_SECONDS) so the final beat is not
    clipped, which captures a few onsets after the scenario ends. Those are
    overspill, not a real count difference, so they are removed before diffing.
    The boundary is INCLUSIVE (ppq <= max_ppq is kept): the golden's last onset is
    the bar-5 downbeat AT ppq 16.0, and the probe legitimately captures it too --
    only ppq strictly greater than max_ppq (16.25, 16.5, ...) is overspill. A
    small epsilon guards the boundary compare against float noise. ``max_ppq`` <=
    0 disables the trim.
    """
    if max_ppq <= 0:
        return events
    return [e for e in events if e.ppq <= max_ppq + EPS_PPQ]


def parse_probe_jsonl(text, source):
    """Parse probe JSONL text into the note-on list (note-offs filtered out)."""
    events = []
    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if not line:
            continue
        try:
            obj = json.loads(line)
        except json.JSONDecodeError as exc:
            raise CompareError(f"{source}:{lineno}: invalid JSON: {exc}") from exc
        if obj.get("type") != "noteOn":
            continue
        try:
            events.append(
                NoteOn(
                    ppq=float(obj["ppq"]),
                    pitch=int(obj["pitch"]),
                    velocity=float(obj["velocity"]),
                    channel=int(obj["channel"]),
                )
            )
        except (KeyError, TypeError, ValueError) as exc:
            raise CompareError(
                f"{source}:{lineno}: missing/invalid field: {exc}"
            ) from exc
    return events


def parse_golden(text, source):
    """Parse the whitespace-column golden into the note-on list."""
    events = []
    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) != 4:
            raise CompareError(
                f"{source}:{lineno}: expected 4 columns, got {len(parts)}: {line!r}"
            )
        try:
            events.append(
                NoteOn(
                    ppq=float(parts[0]),
                    pitch=int(parts[1]),
                    velocity=float(parts[2]),
                    channel=int(parts[3]),
                )
            )
        except ValueError as exc:
            raise CompareError(f"{source}:{lineno}: non-numeric column: {exc}") from exc
    return events


def diff_events(probe, golden, eps_ppq, eps_velocity, ignore_channel):
    """Return a list of (index, field, probe_val, golden_val) divergences.

    A count mismatch is reported as a single ('count', ...) divergence first, and
    per-event comparison runs over the overlapping prefix so field divergences
    still surface alongside a count difference.

    Velocity is compared with EPS_VELOCITY widened to absorb Cubase's near-
    constant ~0.0015 host bias (see the module docstring; 7-bit quantization was
    tried and rejected). Channel is skipped when ``ignore_channel`` is set,
    because a Cubase instrument track coerces incoming MIDI to a single channel
    (channel fidelity is covered by the in-process ``ProbeChain`` test).
    """
    divergences = []
    if len(probe) != len(golden):
        divergences.append(
            (None, "count", len(probe), len(golden))
        )
    for i in range(min(len(probe), len(golden))):
        p, g = probe[i], golden[i]
        if p.pitch != g.pitch:
            divergences.append((i, "pitch", p.pitch, g.pitch))
        if not ignore_channel and p.channel != g.channel:
            divergences.append((i, "channel", p.channel, g.channel))
        if abs(p.velocity - g.velocity) > eps_velocity:
            divergences.append((i, "velocity", p.velocity, g.velocity))
        if abs(p.ppq - g.ppq) > eps_ppq:
            divergences.append((i, "ppq", p.ppq, g.ppq))
    return divergences


def format_divergence(d):
    idx, field, pv, gv = d
    if field == "count":
        return f"  event count: probe={pv} golden={gv}"
    return f"  event[{idx}] {field}: probe={pv} golden={gv}"


def compare(
    probe_events, golden_events, eps_ppq, eps_velocity, max_divergences, ignore_channel
):
    """Compare and print. Return 0 on match, 1 on mismatch."""
    divergences = diff_events(
        probe_events, golden_events, eps_ppq, eps_velocity, ignore_channel
    )
    if not divergences:
        print(
            f"[compare:ok] probe matches golden "
            f"({len(golden_events)} note-ons, eps_ppq={eps_ppq}, "
            f"eps_velocity={eps_velocity}, "
            f"channel={'ignored' if ignore_channel else 'checked'})"
        )
        return 0
    print(
        f"[compare:mismatch] {len(divergences)} divergence(s); "
        f"showing first {min(len(divergences), max_divergences)}:"
    )
    for d in divergences[:max_divergences]:
        print(format_divergence(d))
    if len(divergences) > max_divergences:
        print(f"  ... and {len(divergences) - max_divergences} more")
    return 1


def run(args):
    try:
        with open(args.probe, encoding="utf-8") as fh:
            probe_events = parse_probe_jsonl(fh.read(), args.probe)
        with open(args.golden, encoding="utf-8") as fh:
            golden_events = parse_golden(fh.read(), args.golden)
    except FileNotFoundError as exc:
        print(f"[compare:error] {exc}", file=sys.stderr)
        return 2
    except CompareError as exc:
        print(f"[compare:error] {exc}", file=sys.stderr)
        return 2

    raw_probe_count = len(probe_events)
    probe_events = trim_probe_tail(probe_events, args.max_ppq)
    trimmed = raw_probe_count - len(probe_events)

    print(
        f"[compare:start] probe={args.probe} ({len(probe_events)} note-ons"
        f"{f', {trimmed} tail dropped at ppq>{args.max_ppq}' if trimmed else ''}) "
        f"golden={args.golden} ({len(golden_events)} note-ons)"
    )
    return compare(
        probe_events,
        golden_events,
        args.eps_ppq,
        args.eps_velocity,
        args.max_divergences,
        args.ignore_channel,
    )


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Diff poly_midi_probe JSONL against an in-process golden."
    )
    parser.add_argument("--probe", required=True, help="path to probe JSONL")
    parser.add_argument("--golden", required=True, help="path to golden .txt")
    parser.add_argument(
        "--eps-ppq", type=float, default=EPS_PPQ, help="ppq match tolerance"
    )
    parser.add_argument(
        "--eps-velocity",
        type=float,
        default=EPS_VELOCITY,
        help="velocity match tolerance",
    )
    parser.add_argument(
        "--max-ppq",
        type=float,
        default=DEFAULT_MAX_PPQ,
        help=(
            "drop probe note-ons at or after this ppq (scenario-boundary tail "
            "trim); <=0 disables"
        ),
    )
    parser.add_argument(
        "--ignore-channel",
        dest="ignore_channel",
        action="store_true",
        default=True,
        help=(
            "skip the channel check (default on; a Cubase instrument track "
            "coerces incoming MIDI to one channel)"
        ),
    )
    parser.add_argument(
        "--no-ignore-channel",
        dest="ignore_channel",
        action="store_false",
        help="re-enable the exact channel check (for a channel-preserving host)",
    )
    parser.add_argument(
        "--max-divergences",
        type=int,
        default=DEFAULT_MAX_DIVERGENCES,
        help="how many diverging events to print",
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
