"""Independent-parser validator for Poly's MIDI export (M032 UAT automation).

The M032 "MIDI Export Excellence" milestone deferred three manual UAT steps, all
variants of "export a .mid, open it in Cubase, and confirm N named tracks / no
dropped notes". The exported file's BYTES are already asserted at the byte level
by the C++/JS/TS gtest+node suites; what the manual step added was independent
confirmation that a *different* parser reads the Format-1 structure the way we
intend. This validator supplies that: it parses a .mid with ``mido`` (an engine
we do not own) and asserts the M032 export contract.

It is the shared assertion for two callers:
  * Normal CI: run against a .mid produced offline by ``poly_smf_emit`` (the same
    renderPatternToSMF primitive the Export chip uses). No Cubase required.
  * The Cubase nightly L4-web e2e: run against the .mid the SHIPPING in-plugin
    export path wrote to ``POLY_EXPORT_SINK`` while running inside Cubase — so it
    exercises the real in-DAW export code, not just the offline harness.

Contract asserted (maps to the M032 success criteria):
  1. The file is a Format-1 (type 1) SMF.
  2. Track 0 is a conductor track: it carries a set_tempo meta and NO note-ons.
  3. Every non-conductor track is named (track_name meta) and carries note-ons.
  4. The named tracks match the expected lane-name set (order-independent).
  5. Total note-on count is at least the expected floor (no dropped notes).
  6. The tempo meta corresponds to the expected BPM (criterion 4: host tempo).

Exit codes:
    0  the file satisfies the contract
    1  a contract assertion failed (details printed)
    2  usage / file / parse error
"""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass, field

try:
    import mido
except ImportError:  # pragma: no cover - dependency guard
    sys.stderr.write(
        "error: mido is not installed. Install with: "
        "pip install -r tests/cubase/driver/requirements.txt\n"
    )
    sys.exit(2)


# The default factory preset (index 0) lane names, mirroring
# kWebPresetLaneNames[0] in plugin/source/webui/web_ui_view.cpp and the GM
# mapping in engine/src/lane_name.cpp. poly_smf_emit renders preset 0, so an
# all-lanes export names its tracks from this set. Only ACTIVE lanes (lanes that
# emit at least one note) get a track, so the emitted set is a subset of these.
DEFAULT_PRESET0_LANE_NAMES = {
    "Kick",
    "Snare",
    "Hi-Hat",
    "Open Hat",
    "Tom Hi",
    "Tom Lo",
    "Ride",
    "Crash",
}


@dataclass
class TrackSummary:
    """One parsed MTrk, reduced to the fields the contract cares about."""

    index: int
    name: str | None
    note_on_count: int
    has_tempo: bool
    tempo_bpm: float | None


@dataclass
class ValidationResult:
    ok: bool
    errors: list[str] = field(default_factory=list)
    tracks: list[TrackSummary] = field(default_factory=list)
    midi_format: int | None = None


def summarize_track(index: int, track: "mido.MidiTrack") -> TrackSummary:
    """Reduce a mido track to name / note-on count / tempo presence."""
    name: str | None = None
    note_on_count = 0
    has_tempo = False
    tempo_bpm: float | None = None
    for msg in track:
        if msg.is_meta and msg.type == "track_name":
            # First track_name wins (SMF convention).
            if name is None:
                name = msg.name
        elif msg.is_meta and msg.type == "set_tempo":
            has_tempo = True
            tempo_bpm = round(mido.tempo2bpm(msg.tempo), 3)
        elif msg.type == "note_on" and msg.velocity > 0:
            # A note_on with velocity 0 is a running-status note-off; ignore it.
            note_on_count += 1
    return TrackSummary(
        index=index,
        name=name,
        note_on_count=note_on_count,
        has_tempo=has_tempo,
        tempo_bpm=tempo_bpm,
    )


def validate_smf(
    path: str,
    expected_lane_names: set[str],
    min_note_ons: int,
    expected_bpm: float | None,
    bpm_tolerance: float,
    expected_named_tracks: int | None,
) -> ValidationResult:
    """Parse ``path`` and assert the M032 export contract. Pure and testable."""
    result = ValidationResult(ok=True)
    try:
        mid = mido.MidiFile(path)
    except Exception as exc:  # noqa: BLE001 - report any parse failure verbatim
        result.ok = False
        result.errors.append(f"could not parse {path} as an SMF: {exc}")
        return result

    result.midi_format = mid.type
    result.tracks = [summarize_track(i, t) for i, t in enumerate(mid.tracks)]

    # 1. Format-1.
    if mid.type != 1:
        result.ok = False
        result.errors.append(
            f"expected Format-1 (type 1) SMF, got type {mid.type}"
        )

    if not result.tracks:
        result.ok = False
        result.errors.append("file has no tracks")
        return result

    # 2. Conductor track (track 0): tempo present, no note-ons.
    conductor = result.tracks[0]
    if not conductor.has_tempo:
        result.ok = False
        result.errors.append("conductor track (0) has no set_tempo meta")
    if conductor.note_on_count != 0:
        result.ok = False
        result.errors.append(
            f"conductor track (0) carries {conductor.note_on_count} note-on(s); "
            "the conductor must hold no notes"
        )

    # 6. Tempo matches expected BPM (M032 criterion 4).
    if expected_bpm is not None:
        if conductor.tempo_bpm is None:
            result.ok = False
            result.errors.append("no tempo meta to compare against expected BPM")
        elif abs(conductor.tempo_bpm - expected_bpm) > bpm_tolerance:
            result.ok = False
            result.errors.append(
                f"tempo {conductor.tempo_bpm} BPM differs from expected "
                f"{expected_bpm} BPM by more than {bpm_tolerance}"
            )

    # 3/4. Every non-conductor track is named, carries notes, and its name is in
    # the expected lane-name set.
    lane_tracks = result.tracks[1:]
    if not lane_tracks:
        result.ok = False
        result.errors.append("no lane tracks (only a conductor track present)")

    seen_names: list[str] = []
    for t in lane_tracks:
        if not t.name:
            result.ok = False
            result.errors.append(f"track {t.index} has no track_name meta")
            continue
        seen_names.append(t.name)
        if t.note_on_count == 0:
            result.ok = False
            result.errors.append(
                f"named lane track {t.index} ('{t.name}') carries no note-ons"
            )
        if t.name not in expected_lane_names:
            result.ok = False
            result.errors.append(
                f"track {t.index} name '{t.name}' is not in the expected lane "
                f"names {sorted(expected_lane_names)}"
            )

    # 4 (exact count, optional). Single-lane exports expect exactly one.
    if expected_named_tracks is not None and len(lane_tracks) != expected_named_tracks:
        result.ok = False
        result.errors.append(
            f"expected exactly {expected_named_tracks} named lane track(s), "
            f"got {len(lane_tracks)}: {seen_names}"
        )

    # 5. No dropped notes: total note-ons meet the floor.
    total_note_ons = sum(t.note_on_count for t in lane_tracks)
    if total_note_ons < min_note_ons:
        result.ok = False
        result.errors.append(
            f"total note-on count {total_note_ons} is below the expected floor "
            f"{min_note_ons} (possible dropped notes)"
        )

    return result


def _print_report(path: str, result: ValidationResult) -> None:
    print(f"[smf-validate] file: {path}")
    print(f"[smf-validate] format: {result.midi_format}")
    for t in result.tracks:
        role = "conductor" if t.index == 0 else "lane"
        tempo = f", tempo={t.tempo_bpm}bpm" if t.has_tempo else ""
        print(
            f"[smf-validate] track {t.index} ({role}): name={t.name!r} "
            f"note_ons={t.note_on_count}{tempo}"
        )
    if result.ok:
        print("[smf-validate] PASS — export satisfies the M032 contract.")
    else:
        print("[smf-validate] FAIL:")
        for err in result.errors:
            print(f"[smf-validate]   - {err}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Independent-parser validator for Poly MIDI export (M032)."
    )
    parser.add_argument("midi", help="path to the .mid file to validate")
    parser.add_argument(
        "--min-note-ons",
        type=int,
        default=1,
        help="minimum total note-on count across lane tracks (no-loss floor)",
    )
    parser.add_argument(
        "--expected-bpm",
        type=float,
        default=None,
        help="expected tempo in BPM (M032 criterion 4); omit to skip the check",
    )
    parser.add_argument(
        "--bpm-tolerance",
        type=float,
        default=0.5,
        help="absolute BPM tolerance for the tempo check",
    )
    parser.add_argument(
        "--expected-named-tracks",
        type=int,
        default=None,
        help="exact number of named lane tracks expected (e.g. 1 for a "
        "single-lane export); omit to only require >= 1",
    )
    parser.add_argument(
        "--lane-name",
        action="append",
        dest="lane_names",
        default=None,
        help="an allowed lane-track name; repeatable. Defaults to the preset-0 "
        "GM lane names when omitted.",
    )
    args = parser.parse_args(argv)

    expected_names = (
        set(args.lane_names) if args.lane_names else DEFAULT_PRESET0_LANE_NAMES
    )
    result = validate_smf(
        path=args.midi,
        expected_lane_names=expected_names,
        min_note_ons=args.min_note_ons,
        expected_bpm=args.expected_bpm,
        bpm_tolerance=args.bpm_tolerance,
        expected_named_tracks=args.expected_named_tracks,
    )
    _print_report(args.midi, result)
    return 0 if result.ok else 1


if __name__ == "__main__":
    sys.exit(main())
