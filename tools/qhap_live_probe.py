#!/usr/bin/env python3
"""Bounded, read-only live QHAP probe for Pi, Bitaxe and ESP32.

One AxeOS GET per sample; read existing ESP32 USB serial without sending data.
Writes only compact, credential-free JSONL to /dev/shm; never a daemon.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import select
import threading
import time

from qhap_lab import parse_log
from qhap_snapshot import fetch_bitaxe, number, pi_cpu_temp, private_ipv4


class SerialSampler:
    """Collect at most 256 KiB of observed serial text in memory."""

    LIMIT = 256 * 1024

    def __init__(self, device: Path):
        resolved = str(device.resolve())
        if not (resolved.startswith("/dev/ttyACM") or resolved.startswith("/dev/ttyUSB")):
            raise ValueError("Serial input must resolve to /dev/ttyACM* or /dev/ttyUSB*")
        self.fd = os.open(str(device), os.O_RDONLY | os.O_NONBLOCK | os.O_NOCTTY)
        self._data = bytearray()
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._run, daemon=True)

    def start(self):
        self._thread.start()

    def _run(self):
        while not self._stop.is_set():
            try:
                readable, _, _ = select.select([self.fd], [], [], 0.2)
                if not readable:
                    continue
                chunk = os.read(self.fd, 4096)
                if chunk:
                    with self._lock:
                        self._data.extend(chunk)
                        if len(self._data) > self.LIMIT:
                            del self._data[:-self.LIMIT]
                else:
                    self._stop.wait(0.1)
            except (OSError, ValueError):
                return

    def analysis(self):
        with self._lock:
            text = bytes(self._data).decode("utf-8", errors="replace")
        return parse_log(text)

    def close(self):
        self._stop.set()
        self._thread.join(timeout=2)
        os.close(self.fd)


def output_file(value: str) -> Path:
    path = Path(value).resolve()
    if path.parent != Path("/dev/shm") or path.name in ("", ".", ".."):
        raise argparse.ArgumentTypeError("Output must be a file directly under /dev/shm")
    return path


def snapshot(info: dict, serial: dict, temp: float | None) -> dict:
    return {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "bitaxe": {
            "source": "live_read_only_http",
            "hashrate_ths": number(info, "hashRate", 1000),
            "asic_temp_c": number(info, "temp"),
            "vr_temp_c": number(info, "vrTemp"),
            "power_w": number(info, "power"),
            "fan_percent": number(info, "fanspeed"),
            "fan_rpm": number(info, "fanrpm"),
            "shares_accepted_total": info.get("sharesAccepted"),
            "shares_rejected_total": info.get("sharesRejected"),
        },
        "esp32": {
            "source": "live_serial_observed_since_probe_start",
            "scan_windows_observed": serial["scan_windows"],
            "median_inner_scan_khs": serial["median_inner_scan_khs"],
            "wall_windows_observed": serial["wall_windows"],
            "weighted_wall_khs": serial["weighted_wall_khs"],
            "sha_validation_in_capture": serial["validation"],
            "sha_mismatches_observed": serial["sha_reference_mismatches"],
        },
        "raspberry_pi": {"cpu_temp_c": temp},
        "metric_note": "ESP32 scan rate is not a wall-clock rate; do not sum with Bitaxe hashrate.",
    }


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--bitaxe-ip", required=True, type=private_ipv4)
    p.add_argument("--serial", type=Path, default=Path("/dev/ttyACM0"))
    p.add_argument("--samples", type=int, default=3)
    p.add_argument("--interval", type=float, default=5)
    p.add_argument("--output", type=output_file, default=Path("/dev/shm/qhap_live_probe.jsonl"))
    args = p.parse_args()
    if not 1 <= args.samples <= 6 or not 2 <= args.interval <= 15:
        p.error("Choose 1-6 samples and an interval of 2-15 seconds (short test only)")
    try:
        reader = SerialSampler(args.serial)
    except (OSError, ValueError) as exc:
        p.error(f"Cannot read USB serial: {exc}")

    fd = os.open(str(args.output), os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    try:
        reader.start()
        with os.fdopen(fd, "w", encoding="utf-8") as out:
            for idx in range(args.samples):
                start = time.monotonic()
                try:
                    info = fetch_bitaxe(args.bitaxe_ip, timeout=4.0)
                    record = snapshot(info, reader.analysis(), pi_cpu_temp())
                except (OSError, ValueError, TimeoutError) as exc:
                    serial = reader.analysis()
                    record = {
                        "timestamp_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
                        "error": f"Bitaxe telemetry unavailable: {exc}",
                        "esp32": {"source": "live_usb_serial",
                                  "scan_windows": serial["scan_windows"],
                                  "median_inner_scan_khs": serial["median_inner_scan_khs"]},
                    }
                out.write(json.dumps(record, separators=(",", ":")) + "\n")
                out.flush()
                print(json.dumps(record, indent=2))
                if idx + 1 < args.samples:
                    time.sleep(max(0.0, args.interval - (time.monotonic() - start)))
    finally:
        reader.close()
    print(f"RAM-only snapshots saved to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
