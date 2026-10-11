# QHAP Pre-ASIC Experiment Lab — 2026-10-11

**Stage:** design and software-only validation. The Bitaxe Gamma 602 / BM1370 has not arrived. No physical ASIC measurements are claimed.

## Hardware roles (already available)

| Node | Role now | Important isolation |
|---|---|---|
| ESP32-S3-DevKitC-1-N8R8 #1 (currently running miner; confirm board identity) | **Reference/control:** current known-good live Stratum miner (wall-clock ~254–255 kH/s, hardware scan ~266–269 kH/s) | Do not reflash or inject mock frames |
| ESP32-S3-DevKitC-1-N8R8 #2 | **Challenger:** independent ESP-IDF builds, controlled nonce-work and compiler/task experiments | Use a separate mining identity; do not publish fabricated work |
| Third ESP32 (model not yet identified) | **Protocol test receiver:** dedicated UART, job-switch, checksum and fault-handling experiments | Connect only to local synthetic sources, never CKPool |
| Raspberry Pi 5 (8 GB) | **Linux supervisor:** ingest logs, run controlled trials/statistical tests, local Stratum mock server and Evolution Engine | Does not sit in the block-submission critical path |
| Raspberry Pi Pico H (RP2040) | **Deterministic microcontroller stimulus:** synthesize BM1370-format result bytes, programmable UART delays/noise, later Programmable Input/Output (PIO) timing tests | Not a Linux computer, has no built-in Wi-Fi, and cannot emulate BM1370 hashrate |

**Identity boundary:** The user explicitly confirms **two** units of the pictured ESP32-S3-DevKitC-1-N8R8 model and **a third ESP32 of an as-yet unknown model**: total three ESP32 boards. The two Raspberry Pi products are **one Pi 5 (8 GB) Linux computer** and **one Pico H (RP2040) microcontroller**. The first purchased pair included one N8R8 and one Pi 5. Do not infer board identity from repeated Amazon screenshots; identify serial numbers/ports and the running firmware before operating on a board. The second N8R8 is a natural matched challenger; the third ESP32 is the provisional UART receiver only once its actual hardware is identified.

**Architecture:** USB serial (local logs) or later authenticated LAN telemetry from each ESP32 -> Pi collector -> SQLite/CSV/JSON run records -> correctness gates -> statistics -> champion/challenger report. The Pi should not be in the critical share submission path. The reference board remains autonomous.

## Experiments ordered by engineering value

| ID | Experiment | Control and challenger | Verification and metric | Do before ASIC? |
|---|---|---|---|---|
| E0 | Pi-based reproducible baseline | known-good live miner unchanged; Pi parses logs | SHA reference MATCH, measured completed hashes and wall time, nonzero errors | Yes, first |
| E1 | Two-board independent work | Board B live; second N8R8 ESP32-S3 on independent authenticated Stratum session, separate worker label | Unique pool-assigned extranonce1/job identity; combined useful hashes/sec; no repeated nonce/header pairs | Yes, after second N8R8 ESP32-S3 validated |
| E2 | Local deterministic pool adversarial tests | second N8R8 ESP32-S3 connects only to Pi mock Stratum server, known-good live miner untouched | Handle split TCP frames, delayed difficulty, clean_jobs, reconnects, stale candidates, partial sends | Yes |
| E3 | Mutation/evolution search | second N8R8 ESP32-S3 candidate parameter sets vs known-good live miner untouched reference | Correctness first; compare median/weighted wall-clock rate, error rate, restarts, stale work | Yes |
| E4 | BM1370 protocol/candidate simulator | Pico H -> third ESP32 (unverified model) over 3.3 V UART, with Pi-managed test vectors; isolated from live pool | Valid/corrupt CRC5, controlled delays, nonzero rolled version, exact header and target checks; reject stale ID / duplicates | Yes; physical ASIC validation pending |
| E4a | Controller transport resilience | Pico H sends burst, partial, repeated, delayed synthetic frames to third ESP32 (unverified model) | Receiver resynchronization, no buffer overflow, zero external fake shares | Yes |
| E4b | PIO hardware timing experiment | Pico H RP2040 PIO against third ESP32 (unverified model) UART input | Timestamped jitter/latency distribution at safe 3.3 V levels | Yes, after a simple UART baseline |
| E5 | Sustained 24-hour reliability | known-good miner remains live; second N8R8 ESP32-S3 alternative firmware | Uptime, reconnect speed, watchdog restarts, useful attempted hashes | Yes, only after short-run correctness gates |
| E6 | Energy efficiency | second N8R8 ESP32-S3 vs known-good miner under measured comparable power input | Hashes/J **only when external power measurement exists** | Defer power claims until measured |
| E7 | ASIC frequency/voltage tuning | Physical BM1370 with known safe limits | Real TH/s, temperature, errors and joules | No; requires ASIC |

### Do not repeat work
- Dual-core software SHA research already measured ~17.1 kH/s on Core 1, but did not check pool/network targets or submit winning results. Combined computational improvement was roughly 2.5–3%; retain PR #2 as research only.
- CRC5 result validation was merged as PR #3 and tested on the ESP32.
- Isolated BM1370 candidate gate has host CI success in draft PR #4; physical ASIC and live integration remain unverified.
- SHA256d must stay Bitcoin-consensus-correct; mutation is allowed for *execution and scheduling*, not the hash definition.

## Experiment protocol (champion/challenger)

1. Preserve known-good miner stable firmware and record commit, binary fingerprint (if available), board, power conditions, Wi-Fi, pool settings, measurement method. Never record Wi-Fi secrets or wallet private keys.
2. Start with E0. The Raspberry Pi imports existing raw serial logs read-only; parse `=== QHAP WALL-TIME MEASUREMENT ===` and use **completed hashes / elapsed wall seconds**, not a sum of inner-loop rates.
3. Label each run by board, firmware commit, configuration, date, trial number, experiment ID and outcome. Keep original log locally for audit.
4. For any change to live mining firmware, require genesis/reference vectors, independent SHA256d check, pool/network target check, nonce partitioning and block-safe submission invariants before considering deployment.
5. Compare equal-duration runs with at least ten 30-second windows each as a **screening** threshold. Repeat promising comparisons across three alternating trials (A/B/A or B/A/B), taking heat, Wi-Fi, workload and background activity into account. No statistical-significance claim from a few windows.
6. A <3% gain is not enough to promote production changes based on earlier QHAP criteria. 3–10% remains experimental; >10% may justify integration **only after** correctness, reliable submissions, stable long runs, and power/thermal review.
7. Fault-injection, mock ASIC results, low artificial targets and locally forged shares must stay strictly on a **local test service**. Never send fake shares to CKPool.
8. Current solo CKPool setup does not pay fractional BTC for non-winning shares. Even three ESP32 miners together do not approach an economically meaningful block probability compared with ASICs.

## Local lab launch order

- **First:** On the Raspberry Pi, install/boot Raspberry Pi OS and enable SSH through Raspberry Pi Imager if running headless; keep the Pi and ESP32 boards on a trusted local network. Do not expose the Pi telemetry service or SSH to the public internet.
- **Second:** Run `tools/qhap_lab.py` against saved log text files. The lab can be tested on the laptop before the Pi boots; it does not access mining pool credentials or change firmware.
- **Third:** Connect other ESP32 to the Pi over USB and identify its exact model, serial port and firmware. Do the same separately for third ESP32 (unverified model) (model pending); the original mining board stays untouched.
- **Fourth:** Program the Pico H's MicroPython firmware, check its serial monitor independently, then run the synthetic packet generator from `tools/pico_bm1370_emulator.py` on the Pico (after USB-only tests). This does not exercise physical BM1370.
- **Fifth:** With both boards disconnected from any public pool, optionally connect Pico H GP0 (physical pin 1) -> third ESP32 (unverified model) (model pending) chosen RX GPIO, and Pico H GND -> ESP32 GND. Power each from its own USB. **Do not interconnect 5 V/VBUS/VSYS or 3.3 V power rails; never apply a 5 V signal to GPIO.** Validate selected ESP32 GPIO availability against that board's pinout before wiring.
- **Sixth:** Implement a local Pi simulated Stratum server for the fault matrix, then a telemetry link; keep the original miner out of these tests.
- **Seventh:** Only after validation, run other ESP32 against real mining jobs using a distinct pool worker label and independently check non-overlap.

### Stop/promotion rules

Stop a challenger experiment if SHA checks mismatch, a watchdog repeatedly restarts, nonce/header work duplicates, submit data is inconsistent, or the miner mishandles fresh/stale jobs. Store findings even for failed experiments.

Pi controls **experiment scheduling, reporting and optional offline parameter recommendations**. It does not remotely rewrite Board B or autonomously raise electrical/thermal limits. All future ASIC changes require hard firmware/hardware safety envelopes.

## Baseline evidence

Earlier board B wall-clock windows: 7,700,000 hashes per 30.337 and 30.150 seconds (~253.81 and 255.39 kH/s); earlier hardware-only scan rates ~263.55–269.15 kH/s. Do not mix the two types of measurement or claim the Core 1 software lane is productive.

## Future BM1370 bring-up

With Bitaxe Gamma 602 physically present: backup stock firmware; confirm 5 V supply >4 A and active cooling; inspect UART pinout and logic levels; detect chip; send known safe jobs; receive nonce results; verify using QHAP candidate gate; compare stock firmware and QHAP effective TH/s, stale/error rates and power. Until then, the Pi can emulate the controller-side protocol only.

## Source references

- [QHAP Project Memory](../PROJECT_MEMORY.md) sections 20B–20H.
- [QHAP Roadmap](../ROADMAP.md) priorities 1–5B.
- [Dual-engine research PR #2](https://github.com/Vermicare/QHAP/pull/2).
- [BM1370 CRC5 PR #3](https://github.com/Vermicare/QHAP/pull/3).
- [BM1370 isolated candidate gate PR #4](https://github.com/Vermicare/QHAP/pull/4).

## Status disclaimer

This file records a planned experiment sequence, not experimentally validated improvements for either new ESP32, the Pi 5, Pico H or a physical BM1370. The original miner's historical performance is not a benchmark for untested new boards.

> Hardware identification gate: all proposed ESP32 challenger/compiler experiments are conditional on identifying the actual target microcontroller, memory, peripheral availability and safe electrical pinout. The Pico emulator may be tested on Linux without wiring any ESP32.
