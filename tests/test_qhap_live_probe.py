"""No-device tests for the bounded live QHAP telemetry probe."""
import argparse
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from qhap_live_probe import output_file, snapshot  # noqa: E402
from qhap_lab import parse_log  # noqa: E402


class LiveProbeTests(unittest.TestCase):
    def test_ram_only_path(self):
        self.assertEqual(output_file("/dev/shm/qhap_live_probe.jsonl"), Path("/dev/shm/qhap_live_probe.jsonl"))
        with self.assertRaises(argparse.ArgumentTypeError):
            output_file("/tmp/live_log.jsonl")

    def test_units_and_serial_source(self):
        serial = parse_log("=== LIVE NONCE SCAN ===\nRate: 268.64 kH/s\n")
        record = snapshot({"hashRate": 1291.2, "temp": 68.5, "vrTemp": 78,
                           "sharesAccepted": 4300, "sharesRejected": 0}, serial, 39.15)
        self.assertEqual(record["bitaxe"]["hashrate_ths"], 1.291)
        self.assertEqual(record["esp32"]["median_inner_scan_khs"], 268.64)
        self.assertEqual(record["esp32"]["source"], "live_serial_observed_since_probe_start")
        self.assertIsNone(record["esp32"]["weighted_wall_khs"])
        self.assertEqual(record["bitaxe"]["shares_rejected_total"], 0)
        self.assertEqual(record["raspberry_pi"]["cpu_temp_c"], 39.15)

    def test_no_fake_values(self):
        record = snapshot({}, parse_log(""), None)
        self.assertIsNone(record["bitaxe"]["hashrate_ths"])
        self.assertIsNone(record["esp32"]["median_inner_scan_khs"])


if __name__ == "__main__":
    unittest.main()
