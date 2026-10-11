#!/usr/bin/env python3
"""QHAP pre-ASIC lab: read-only analysis of ESP32 serial monitor logs.

No mining, pool access, SSH, reflashing, or changes to connected boards.
Requires only the Python standard library (Raspberry Pi OS compatible).
"""
from __future__ import annotations

import argparse
import json
import re
import statistics
from pathlib import Path

WALL_START = "=== QHAP WALL-TIME MEASUREMENT ==="
SCAN_START = "=== LIVE NONCE SCAN ==="
FLOAT = r"([0-9]+(?:\.[0-9]+)?)"


def parse_log(text: str) -> dict:
    windows = []
    scan_rates = []
    sha_matches = sha_mismatches = 0
    malformed_windows = 0
    pending = None
    live_scan = False
    for source_line in text.splitlines():
        line = source_line.strip()
        if line == WALL_START:
            if pending is not None:
                malformed_windows += 1
            pending = {}
            live_scan = False
            continue
        if line == SCAN_START:
            live_scan = True
            continue
        if line == "Hardware vs PSA: MATCH":
            sha_matches += 1
        elif line == "Hardware vs PSA: MISMATCH":
            sha_mismatches += 1

        if pending is not None:
            for key, pattern, converter in (
                ("hashes", r"^Completed hashes:\s*(\d+)$", int),
                ("seconds", r"^Elapsed:\s*" + FLOAT + r"\s+seconds$", float),
                ("reported_khs", r"^Overall rate:\s*" + FLOAT + r"\s+kH/s$", float),
            ):
                m = re.match(pattern, line)
                if m:
                    pending[key] = converter(m.group(1))
            if len(pending) == 3:
                if pending["hashes"] <= 0 or pending["seconds"] <= 0:
                    malformed_windows += 1
                else:
                    rate = pending["hashes"] / pending["seconds"] / 1000.0
                    if abs(rate - pending["reported_khs"]) > 0.06:
                        malformed_windows += 1
                    else:
                        windows.append({
                            "hashes": pending["hashes"],
                            "seconds": pending["seconds"],
                            "khs": round(rate, 5),
                        })
                pending = None
            continue

        if live_scan:
            m = re.match(r"^Rate:\s*" + FLOAT + r"\s+kH/s$", line)
            if m:
                scan_rates.append(float(m.group(1)))
                live_scan = False
    if pending is not None:
        malformed_windows += 1

    total_hashes = sum(w["hashes"] for w in windows)
    total_seconds = sum(w["seconds"] for w in windows)
    rate = total_hashes / total_seconds / 1000 if total_seconds > 0 else None
    return {
        "wall_windows": len(windows),
        "wall_hashes": total_hashes,
        "wall_seconds": round(total_seconds, 3),
        "weighted_wall_khs": round(rate, 5) if rate is not None else None,
        "median_wall_khs": round(statistics.median(w["khs"] for w in windows), 5)
        if windows else None,
        "scan_windows": len(scan_rates),
        "median_inner_scan_khs": round(statistics.median(scan_rates), 5)
        if scan_rates else None,
        "sha_reference_matches": sha_matches,
        "sha_reference_mismatches": sha_mismatches,
        "malformed_windows": malformed_windows,
        "validation": "observed" if sha_matches and not sha_mismatches
        else "failed" if sha_mismatches else "not-observed",
        "windows": windows,
    }


def summarize(path: Path) -> dict:
    if not path.is_file():
        raise ValueError(f"Missing log: {path}")
    result = parse_log(path.read_text(encoding="utf-8", errors="replace"))
    result["source"] = str(path)
    return result


def compare(baseline: dict, challenger: dict, minimum_windows: int = 10) -> dict:
    reasons = []
    if baseline["wall_windows"] < minimum_windows:
        reasons.append("baseline insufficient wall-clock windows")
    if challenger["wall_windows"] < minimum_windows:
        reasons.append("challenger insufficient wall-clock windows")
    for name, run in (("baseline", baseline), ("challenger", challenger)):
        if run["sha_reference_mismatches"]:
            reasons.append(name + " SHA256d mismatch detected")
        if not run["sha_reference_matches"]:
            reasons.append(name + " reference MATCH not observed")
        if run["malformed_windows"]:
            reasons.append(name + " malformed wall-clock measurement detected")
    base_rate = baseline["weighted_wall_khs"]
    contender_rate = challenger["weighted_wall_khs"]
    gain = None
    if base_rate is not None and contender_rate is not None and base_rate > 0:
        gain = (contender_rate / base_rate - 1) * 100
    else:
        reasons.append("missing comparable wall-clock rate")
    if reasons:
        decision = "not-eligible-for-screen"
    elif gain is not None and gain >= 3.0:
        decision = "promising-screen-only"
    else:
        decision = "insufficient-gain"
    return {
        "baseline": baseline["source"],
        "challenger": challenger["source"],
        "baseline_khs": base_rate,
        "challenger_khs": contender_rate,
        "gain_percent": round(gain, 3) if gain is not None else None,
        "decision": decision,
        "reasons": reasons,
        "notice": "Screen only: no firmware is selected, flashed or promoted. "
                  "Repeat alternating trials and validate work targets, "
                  "stale handling, power and thermals before deployment.",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    one = commands.add_parser("summary", help="Summarize one saved ESP32 log")
    one.add_argument("file", type=Path)
    two = commands.add_parser("compare", help="Compare two saved wall-clock logs")
    two.add_argument("--baseline", required=True, type=Path)
    two.add_argument("--challenger", required=True, type=Path)
    args = parser.parse_args()
    try:
        if args.command == "summary":
            result = summarize(args.file)
        else:
            result = compare(summarize(args.baseline), summarize(args.challenger))
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
