"""Tests for the read-only pre-ASIC mining log analyzer (no device required)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from qhap_lab import parse_log, compare  # noqa: E402


def window(hashes=100000, seconds=0.4, reported=None):
    rate = hashes / seconds / 1000 if reported is None else reported
    return ("=== QHAP WALL-TIME MEASUREMENT ===\n"
            "Batch size: 100000\n"
            f"Completed hashes: {hashes}\n"
            f"Elapsed: {seconds:.3f} seconds\n"
            f"Overall rate: {rate:.2f} kH/s\n")


def result(name, windows=10, seconds=0.4, matching=True):
    raw = ("Hardware vs PSA: MATCH\n" if matching else "") + window(seconds=seconds) * windows
    parsed = parse_log(raw)
    parsed["source"] = name
    return parsed


class PreAsicLabTests(unittest.TestCase):
    def test_weighted_wallclock_not_sum_of_scan_rates(self):
        parsed = parse_log(
            "Hardware vs PSA: MATCH\n"
            "=== LIVE NONCE SCAN ===\nRate: 269.00 kH/s\n" +
            window(seconds=0.4) + window(seconds=0.5))
        self.assertEqual(parsed["wall_windows"], 2)
        self.assertEqual(parsed["wall_hashes"], 200000)
        self.assertAlmostEqual(parsed["weighted_wall_khs"], 200000 / 0.9 / 1000, places=4)
        self.assertEqual(parsed["median_inner_scan_khs"], 269.0)
        self.assertEqual(parsed["validation"], "observed")

    def test_detect_bad_reported_number(self):
        parsed = parse_log(window(seconds=0.4, reported=900.0))
        self.assertEqual(parsed["wall_windows"], 0)
        self.assertEqual(parsed["malformed_windows"], 1)

    def test_incomplete_window(self):
        parsed = parse_log("=== QHAP WALL-TIME MEASUREMENT ===\nCompleted hashes: 100000\n")
        self.assertEqual(parsed["malformed_windows"], 1)

    def test_passes_only_screening(self):
        a = result("board-b", seconds=0.4)
        b = result("board-a", seconds=0.38)
        diff = compare(a, b)
        self.assertEqual(diff["decision"], "promising-screen-only")
        self.assertGreater(diff["gain_percent"], 3.0)

    def test_reject_few_windows(self):
        a = result("board-b", windows=3)
        b = result("board-a", windows=3, seconds=0.38)
        self.assertEqual(compare(a, b)["decision"], "not-eligible-for-screen")

    def test_reject_missing_reference_match(self):
        a = result("board-b")
        b = result("board-a", matching=False)
        self.assertEqual(compare(a, b)["decision"], "not-eligible-for-screen")

    def test_reject_mismatch(self):
        a = result("board-b")
        b = parse_log("Hardware vs PSA: MATCH\nHardware vs PSA: MISMATCH\n" +
                      window(seconds=0.38) * 10)
        b["source"] = "board-a"
        self.assertEqual(compare(a, b)["decision"], "not-eligible-for-screen")

    def test_under_three_percent_no_promotion(self):
        a = result("board-b", seconds=0.4)
        b = result("board-a", seconds=0.399)
        self.assertEqual(compare(a, b)["decision"], "insufficient-gain")


if __name__ == "__main__":
    unittest.main()
