"""CPython-only golden vectors for the optional Pico H UART stimulus.

These are software tests only: no Pico H, ESP32, UART or BM1370 connected.
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import pico_bm1370_emulator as pico


class PicoResultTests(unittest.TestCase):
    def test_qhap_known_raw_vector(self):
        raw = pico.make_result(
            0xA1B2C3D4, 0x5A, 0x1234,
            job_response=True, midstate_num=2)
        self.assertEqual(raw.hex(), "aa55a1b2c3d4025a12348c")
        self.assertEqual(pico.crc5(raw[2:]), 0)
        self.assertEqual((raw[7] & 0xF0) >> 1, 0x28)
        self.assertEqual(raw[7] & 0x0F, 0x0A)

    def test_crc5_generated_residue(self):
        for nonce in (0, 1, 0x01020304, 0xDEADBEEF, 0xFFFFFFFF):
            for ver in (0, 1, 0x1234, 0xFFFF):
                raw = pico.make_result(nonce, 0x3B, ver)
                self.assertEqual(len(raw), 11)
                self.assertEqual(pico.crc5(raw[2:]), 0)
                self.assertEqual(raw[2:6], nonce.to_bytes(4, "big"))

    def test_corrupt_rejected_by_crc5(self):
        raw = pico.make_result(0x10203040)
        broken = pico.corrupt_payload(raw)
        self.assertNotEqual(pico.crc5(broken[2:]), 0)
        self.assertNotEqual(broken, raw)

    def test_non_job_flag_and_version(self):
        raw = pico.make_result(1, raw_id=0, raw_version=0xFFFF,
                               job_response=False)
        self.assertFalse(raw[10] & 0x80)
        self.assertEqual(raw[8:10], b"\xff\xff")
        self.assertEqual(pico.crc5(raw[2:]), 0)

    def test_bad_input_rejected(self):
        for nonce in (-1, 0x100000000):
            with self.assertRaises(ValueError):
                pico.make_result(nonce)
        with self.assertRaises(ValueError):
            pico.make_result(5, raw_id=256)
        with self.assertRaises(ValueError):
            pico.corrupt_payload(b"bad")

    def test_no_hardware_session_on_import(self):
        self.assertFalse(hasattr(pico, "machine"))
        self.assertTrue(callable(pico.start_uart_test))


if __name__ == "__main__":
    unittest.main()
