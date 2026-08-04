"""Unit tests for compare_probe_golden (M042 S08 T04).

Runs with the stdlib: ``python -m unittest test_compare_probe_golden`` from
``tests/cubase/`` (no pytest dependency). Exercises the parsers and the
match/mismatch contract against synthetic probe/golden pairs.
"""

import io
import unittest
from contextlib import redirect_stdout

import compare_probe_golden as c

# A matching pair: two note-ons. Probe includes a note-off that must be filtered.
PROBE_MATCH = (
    '{"type":"noteOn","ppq":0.000000,"pitch":36,"velocity":0.750000,"channel":0}\n'
    '{"type":"noteOff","ppq":0.500000,"pitch":36,"velocity":0.000000,"channel":0}\n'
    '{"type":"noteOn","ppq":1.000000,"pitch":38,"velocity":0.600000,"channel":0}\n'
)
GOLDEN_MATCH = (
    "# processor golden: bars=4 tempo=120 blockSize=512 sampleRate=44100\n"
    "# ppq_position  pitch  velocity  channel\n"
    "0.000000   36  0.750000  0\n"
    "1.000000   38  0.600000  0\n"
)


class ParseTests(unittest.TestCase):
    def test_probe_filters_note_offs(self):
        events = c.parse_probe_jsonl(PROBE_MATCH, "probe")
        self.assertEqual(len(events), 2)
        self.assertEqual(events[0].pitch, 36)
        self.assertEqual(events[1].pitch, 38)

    def test_golden_skips_comments(self):
        events = c.parse_golden(GOLDEN_MATCH, "golden")
        self.assertEqual(len(events), 2)
        self.assertEqual(events[0].ppq, 0.0)
        self.assertEqual(events[1].velocity, 0.6)

    def test_probe_bad_json_raises(self):
        with self.assertRaises(c.CompareError):
            c.parse_probe_jsonl("{not json}\n", "probe")

    def test_golden_wrong_column_count_raises(self):
        with self.assertRaises(c.CompareError):
            c.parse_golden("0.0 36 0.5\n", "golden")


class CompareTests(unittest.TestCase):
    def _compare(self, probe_text, golden_text, **kw):
        probe = c.parse_probe_jsonl(probe_text, "probe")
        golden = c.parse_golden(golden_text, "golden")
        buf = io.StringIO()
        with redirect_stdout(buf):
            rc = c.compare(
                probe,
                golden,
                kw.get("eps_ppq", c.EPS_PPQ),
                kw.get("eps_velocity", c.EPS_VELOCITY),
                kw.get("max_divergences", c.DEFAULT_MAX_DIVERGENCES),
            )
        return rc, buf.getvalue()

    def test_match_returns_zero(self):
        rc, out = self._compare(PROBE_MATCH, GOLDEN_MATCH)
        self.assertEqual(rc, 0)
        self.assertIn("[compare:ok]", out)

    def test_pitch_mismatch_returns_one_and_prints(self):
        golden = GOLDEN_MATCH.replace("   36  ", "   40  ")
        rc, out = self._compare(PROBE_MATCH, golden)
        self.assertEqual(rc, 1)
        self.assertIn("pitch", out)
        self.assertIn("probe=36", out)
        self.assertIn("golden=40", out)

    def test_count_mismatch_reported(self):
        golden = GOLDEN_MATCH + "2.000000   41  0.500000  0\n"
        rc, out = self._compare(PROBE_MATCH, golden)
        self.assertEqual(rc, 1)
        self.assertIn("event count", out)
        self.assertIn("probe=2", out)
        self.assertIn("golden=3", out)

    def test_ppq_within_tolerance_matches(self):
        # Shift a probe ppq by less than EPS_PPQ; still a match.
        probe = PROBE_MATCH.replace('"ppq":1.000000', '"ppq":1.000300')
        rc, _ = self._compare(probe, GOLDEN_MATCH, eps_ppq=5e-4)
        self.assertEqual(rc, 0)

    def test_ppq_beyond_tolerance_mismatches(self):
        probe = PROBE_MATCH.replace('"ppq":1.000000', '"ppq":1.010000')
        rc, out = self._compare(probe, GOLDEN_MATCH, eps_ppq=5e-4)
        self.assertEqual(rc, 1)
        self.assertIn("ppq", out)

    def test_velocity_beyond_tolerance_mismatches(self):
        probe = PROBE_MATCH.replace('"velocity":0.600000', '"velocity":0.650000')
        rc, out = self._compare(probe, GOLDEN_MATCH)
        self.assertEqual(rc, 1)
        self.assertIn("velocity", out)

    def test_max_divergences_truncates(self):
        # Build 12 all-wrong events; only DEFAULT_MAX_DIVERGENCES printed.
        probe_lines = "".join(
            f'{{"type":"noteOn","ppq":{i}.000000,"pitch":36,"velocity":0.5,"channel":0}}\n'
            for i in range(12)
        )
        golden_lines = (
            "# h\n# h\n"
            + "".join(f"{i}.000000   40  0.5  0\n" for i in range(12))
        )
        rc, out = self._compare(probe_lines, golden_lines, max_divergences=10)
        self.assertEqual(rc, 1)
        self.assertIn("and 2 more", out)


if __name__ == "__main__":
    unittest.main()
