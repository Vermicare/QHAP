#!/usr/bin/env python3
"""Read-only QHAP snapshot: live Bitaxe AxeOS API + a SAVED ESP32 serial log.

No miner configuration changes, firmware flashing, or automatic deployment.
Run from the Raspberry Pi; this performs one HTTP GET on a local IP only.
Uses the Python standard library and the existing qhap_lab analyzer.
"""
from __future__ import annotations

import argparse
import ipaddress
import json
from datetime import datetime, timezone
from pathlib import Path
from urllib.request import urlopen

from qhap_lab import summarize


def private_ipv4(value: str) -> str:
    try:
        ip = ipaddress.ip_address(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Supply a literal local IPv4 address") from exc
    if ip.version != 4 or not ip.is_private or ip.is_loopback or ip.is_link_local:
        raise argparse.ArgumentTypeError("Bitaxe must have a private LAN IPv4 address")
    return str(ip)


def fetch_bitaxe(ip: str, timeout: float = 8.0) -> dict:
    # Only read the documented system-information endpoint. Never send POSTs.
    with urlopen(f"http://{ip}/api/system/info", timeout=timeout) as response:
        data = json.load(response)
    if not isinstance(data, dict):
        raise ValueError("Bitaxe API returned a non-object JSON value")
    return data


def number(data: dict, key: str, scale: float = 1.0) -> float | None:
    value = data.get(key)
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return None
    return round(float(value) / scale, 3)


def assemble_snapshot(bitaxe: dict, esp32: dict, pi_cpu_c: float | None = None) -> dict:
    return {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "bitaxe": {
            "source": "live_read_only_http",
            "hashrate_ths": number(bitaxe, "hashRate", 1000.0),
            "asic_temp_c": number(bitaxe, "temp"),
            "voltage_regulator_temp_c": number(bitaxe, "vrTemp"),
            "fan_percent": number(bitaxe, "fanspeed"),
            "fan_rpm": number(bitaxe, "fanrpm"),
            "power_w": number(bitaxe, "power"),
            "frequency_mhz": number(bitaxe, "frequency"),
            "shares_accepted": bitaxe.get("sharesAccepted"),
            "shares_rejected": bitaxe.get("sharesRejected"),
        },
        "esp32": {
            "source": "saved_serial_log_not_live",
            "scan_windows": esp32["scan_windows"],
            "median_inner_scan_khs": esp32["median_inner_scan_khs"],
            "wall_windows": esp32["wall_windows"],
            "weighted_wall_khs": esp32["weighted_wall_khs"],
            "sha_validation_in_log": esp32["validation"],
            "sha_reference_mismatches": esp32["sha_reference_mismatches"],
            "malformed_windows": esp32["malformed_windows"],
        },
        "raspberry_pi": {"cpu_temp_c": pi_cpu_c},
        "notes": [
            "Bitaxe hashRate is supplied by AxeOS in GH/s and converted to TH/s.",
            "ESP32 serial-log data is historical; do not treat it as simultaneous live mining telemetry.",
            "An inner-loop scan rate is not a weighted wall-clock throughput measurement.",
            "Not-observed SHA validation in a short log is not a failed SHA test.",
            "No mining performance values are summed or treated as shares/block rewards.",
        ],
    }


def pi_cpu_temp() -> float | None:
    try:
        return round(int(Path("/sys/class/thermal/thermal_zone0/temp").read_text().strip()) / 1000, 2)
    except (OSError, ValueError):
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bitaxe-ip", type=private_ipv4, required=True)
    parser.add_argument("--esp32-log", type=Path, required=True)
    args = parser.parse_args()
    try:
        saved = summarize(args.esp32_log)
        live = fetch_bitaxe(args.bitaxe_ip)
    except (OSError, ValueError, TimeoutError) as exc:
        parser.error(f"Snapshot unavailable: {exc}")
    print(json.dumps(assemble_snapshot(live, saved, pi_cpu_temp()), indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
