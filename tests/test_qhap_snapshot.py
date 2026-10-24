"""Host-only checks for read-only Bitaxe + saved ESP32 QHAP snapshot."""
import argparse
import io
import json
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from qhap_snapshot import assemble_snapshot, fetch_bitaxe, private_ipv4  # noqa: E402
from qhap_lab import parse_log  # noqa: E402


class SnapshotTests(unittest.TestCase):
    def test_lan_address_only(self):
        self.assertEqual(private_ipv4("192.168.1.35"), "192.168.1.35")
        for invalid in ("8.8.8.8", "127.0.0.1", "169.254.1.2", "abc"):
            with self.subTest(ip=invalid):
                with self.assertRaises(argparse.ArgumentTypeError):
                    private_ipv4(invalid)

    def test_one_read_only_get(self):
        sample = {"hashRate": 1290.2, "temp": 68.8}
        with patch("qhap_snapshot.urlopen", return_value=io.BytesIO(json.dumps(sample).encode())) as mock:
            self.assertEqual(fetch_bitaxe("192.168.1.35"), sample)
            mock.assert_called_once_with("http://192.168.1.35/api/system/info", timeout=8.0)

    def test_bad_api_shape_rejected(self):
        with patch("qhap_snapshot.urlopen", return_value=io.BytesIO(b"[]")):
            with self.assertRaises(ValueError):
                fetch_bitaxe("192.168.1.35")

    def test_preserve_metric_units_and_source_boundaries(self):
        saved = parse_log("=== LIVE NONCE SCAN ===\nRate: 267.93 kH/s\n")
        snapshot = assemble_snapshot(
            {"hashRate": 1290.208, "temp": 68.875, "vrTemp": 77,
             "fanrpm": 8612, "fanspeed": 100, "power": 24.493,
             "frequency": 625, "sharesAccepted": 3963, "sharesRejected": 0},
            saved,
            pi_cpu_c=36.7,
        )
        self.assertEqual(snapshot["bitaxe"]["hashrate_ths"], 1.29)
        self.assertEqual(snapshot["bitaxe"]["asic_temp_c"], 68.875)
        self.assertEqual(snapshot["bitaxe"]["shares_rejected"], 0)
        self.assertEqual(snapshot["esp32"]["median_inner_scan_khs"], 267.93)
        self.assertIsNone(snapshot["esp32"]["weighted_wall_khs"])
        self.assertEqual(snapshot["esp32"]["sha_validation_in_log"], "not-observed")
        self.assertEqual(snapshot["esp32"]["source"], "saved_serial_log_not_live")
        self.assertEqual(snapshot["raspberry_pi"]["cpu_temp_c"], 36.7)

    def test_missing_fields_are_not_invented(self):
        saved = parse_log("")
        snap = assemble_snapshot({}, saved)
        self.assertIsNone(snap["bitaxe"]["hashrate_ths"])
        self.assertIsNone(snap["bitaxe"]["power_w"])
        self.assertIsNone(snap["esp32"]["median_inner_scan_khs"])


if __name__ == "__main__":
    unittest.main()
