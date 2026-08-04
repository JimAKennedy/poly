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

Tolerance policy (see ``FieldTolerance``):
  * pitch, channel -- exact integer equality. A wrong pitch or channel is a real
    routing/mapping defect, never jitter.
  * velocity       -- exact to within EPS_VELOCITY. Poly's velocities are
    deterministic; the epsilon only absorbs %.6f round-trip, not real drift.
  * ppq            -- exact to within EPS_PPQ. Both sides derive ppq from the
    same musical position; a real DAW transport is sample-accurate, so the
    default epsilon is tight (a small fraction of a 1/960 tick). Widen only with
    evidence of benign host jitter, and document why.

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
EPS_VELOCITY = 1e-4
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


def diff_events(probe, golden, eps_ppq, eps_velocity):
    """Return a list of (index, field, probe_val, golden_val) divergences.

    A count mismatch is reported as a single ('count', ...) divergence first, and
    per-event comparison runs over the overlapping prefix so field divergences
    still surface alongside a count difference.
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
        if p.channel != g.channel:
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


def compare(probe_events, golden_events, eps_ppq, eps_velocity, max_divergences):
    """Compare and print. Return 0 on match, 1 on mismatch."""
    divergences = diff_events(probe_events, golden_events, eps_ppq, eps_velocity)
    if not divergences:
        print(
            f"[compare:ok] probe matches golden "
            f"({len(golden_events)} note-ons, eps_ppq={eps_ppq}, "
            f"eps_velocity={eps_velocity})"
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

    print(
        f"[compare:start] probe={args.probe} ({len(probe_events)} note-ons) "
        f"golden={args.golden} ({len(golden_events)} note-ons)"
    )
    return compare(
        probe_events,
        golden_events,
        args.eps_ppq,
        args.eps_velocity,
        args.max_divergences,
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
