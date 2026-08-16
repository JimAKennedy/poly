"""Unit tests for validate_smf_export (M032 export UAT automation, Level C).

Runs with the stdlib + mido: ``python -m unittest test_validate_smf_export``
from ``tests/cubase/`` (mido is pinned in driver/requirements.txt). Synthesizes
good and malformed SMFs with mido and asserts the validator's contract has teeth
— a validator that never fails is worthless, so every contract clause has a
negative test that must flip the verdict to FAIL.
"""

import os
import tempfile
import unittest

import mido

import validate_smf_export as v

PRESET0 = v.DEFAULT_PRESET0_LANE_NAMES


def _write(mid: mido.MidiFile) -> str:
    fd, path = tempfile.mkstemp(suffix=".mid")
    os.close(fd)
    mid.save(path)
    return path


def make_good(
    lane_names=("Kick", "Snare"),
    notes_per_lane=4,
    bpm=120,
) -> str:
    """A well-formed Format-1: conductor (tempo, no notes) + named lane tracks."""
    mid = mido.MidiFile(type=1)
    conductor = mido.MidiTrack()
    conductor.append(mido.MetaMessage("set_tempo", tempo=mido.bpm2tempo(bpm)))
    conductor.append(mido.MetaMessage("end_of_track"))
    mid.tracks.append(conductor)
    for name in lane_names:
        t = mido.MidiTrack()
        t.append(mido.MetaMessage("track_name", name=name))
        for _ in range(notes_per_lane):
            t.append(mido.Message("note_on", note=36, velocity=100, time=0))
            t.append(mido.Message("note_off", note=36, velocity=0, time=120))
        t.append(mido.MetaMessage("end_of_track"))
        mid.tracks.append(t)
    return _write(mid)


def validate(path, **kw):
    defaults = dict(
        expected_lane_names=PRESET0,
        min_note_ons=1,
        expected_bpm=None,
        bpm_tolerance=0.5,
        expected_named_tracks=None,
    )
    defaults.update(kw)
    return v.validate_smf(path, **defaults)


class HappyPathTests(unittest.TestCase):
    def test_well_formed_export_passes(self):
        path = make_good()
        try:
            r = validate(path, min_note_ons=8, expected_bpm=120)
            self.assertTrue(r.ok, r.errors)
            self.assertEqual(r.midi_format, 1)
        finally:
            os.remove(path)

    def test_single_lane_exact_count_passes(self):
        path = make_good(lane_names=("Kick",), notes_per_lane=4)
        try:
            r = validate(path, expected_named_tracks=1, min_note_ons=4)
            self.assertTrue(r.ok, r.errors)
        finally:
            os.remove(path)


class ContractTeethTests(unittest.TestCase):
    """Every clause must be able to FAIL a bad file."""

    def test_format0_fails(self):
        mid = mido.MidiFile(type=0)
        t = mido.MidiTrack()
        t.append(mido.MetaMessage("set_tempo", tempo=mido.bpm2tempo(120)))
        t.append(mido.MetaMessage("track_name", name="Kick"))
        t.append(mido.Message("note_on", note=36, velocity=100, time=0))
        mid.tracks.append(t)
        path = _write(mid)
        try:
            r = validate(path)
            self.assertFalse(r.ok)
            self.assertTrue(any("Format-1" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_conductor_with_notes_fails(self):
        mid = mido.MidiFile(type=1)
        conductor = mido.MidiTrack()
        conductor.append(mido.MetaMessage("set_tempo", tempo=mido.bpm2tempo(120)))
        conductor.append(mido.Message("note_on", note=36, velocity=100, time=0))
        mid.tracks.append(conductor)
        lane = mido.MidiTrack()
        lane.append(mido.MetaMessage("track_name", name="Kick"))
        lane.append(mido.Message("note_on", note=36, velocity=100, time=0))
        mid.tracks.append(lane)
        path = _write(mid)
        try:
            r = validate(path)
            self.assertFalse(r.ok)
            self.assertTrue(any("conductor" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_unnamed_lane_track_fails(self):
        mid = mido.MidiFile(type=1)
        conductor = mido.MidiTrack()
        conductor.append(mido.MetaMessage("set_tempo", tempo=mido.bpm2tempo(120)))
        mid.tracks.append(conductor)
        lane = mido.MidiTrack()  # no track_name
        lane.append(mido.Message("note_on", note=36, velocity=100, time=0))
        mid.tracks.append(lane)
        path = _write(mid)
        try:
            r = validate(path)
            self.assertFalse(r.ok)
            self.assertTrue(any("no track_name" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_unexpected_lane_name_fails(self):
        path = make_good(lane_names=("Bagpipes",))
        try:
            r = validate(path)
            self.assertFalse(r.ok)
            self.assertTrue(any("not in the expected" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_dropped_notes_below_floor_fails(self):
        path = make_good(lane_names=("Kick",), notes_per_lane=1)
        try:
            r = validate(path, min_note_ons=100)
            self.assertFalse(r.ok)
            self.assertTrue(any("below the expected floor" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_wrong_tempo_fails(self):
        path = make_good(bpm=140)
        try:
            r = validate(path, expected_bpm=120)
            self.assertFalse(r.ok)
            self.assertTrue(any("BPM" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_wrong_named_track_count_fails(self):
        path = make_good(lane_names=("Kick", "Snare"))
        try:
            r = validate(path, expected_named_tracks=1)
            self.assertFalse(r.ok)
            self.assertTrue(any("exactly 1 named" in e for e in r.errors))
        finally:
            os.remove(path)

    def test_unparseable_file_fails_cleanly(self):
        fd, path = tempfile.mkstemp(suffix=".mid")
        os.write(fd, b"not a midi file")
        os.close(fd)
        try:
            r = validate(path)
            self.assertFalse(r.ok)
            self.assertTrue(any("could not parse" in e for e in r.errors))
        finally:
            os.remove(path)


if __name__ == "__main__":
    unittest.main()
