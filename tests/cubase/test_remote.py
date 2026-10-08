"""Unit tests for the M004 remote driver's pure parts (no MIDI, no Cubase)."""

import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "driver"))

import remote as r  # noqa: E402 - path set up above


class AutomationScheduleTest(unittest.TestCase):
    def test_lane_switches_on_at_head_and_off_at_the_bar(self):
        schedule = r.automation_schedule(4, 120.0, 4, 3, flat=False)
        self.assertEqual(
            schedule,
            [(r.LANE_HEAD_SECONDS, r.CC_PARAM, r.ON), (4.0, r.CC_PARAM, r.OFF)],
        )

    def test_flat_lane_never_switches_off(self):
        # The s07-flatten-lane red path: the lane exists but never changes.
        schedule = r.automation_schedule(4, 120.0, 4, 3, flat=True)
        self.assertEqual(schedule, [(r.LANE_HEAD_SECONDS, r.CC_PARAM, r.ON)])

    def test_switch_off_tracks_tempo(self):
        schedule = r.automation_schedule(4, 60.0, 4, 2, flat=False)
        self.assertEqual(schedule[-1], (4.0, r.CC_PARAM, r.OFF))

    def test_head_point_precedes_the_change(self):
        # A lane's first point is extended back to the start, so the ON point
        # must come first or the whole lane reads OFF.
        for bar in (2, 3, 4):
            schedule = r.automation_schedule(4, 120.0, 4, bar, flat=False)
            self.assertLess(schedule[0][0], schedule[1][0])

    def test_change_must_fall_inside_the_passage(self):
        for bad in (0, 1, 5):
            with self.assertRaises(ValueError):
                r.automation_schedule(4, 120.0, 4, bad, flat=False)


if __name__ == "__main__":
    unittest.main()
