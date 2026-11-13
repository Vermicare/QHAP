# QHAP — Gamma 601 chess-night engineering plan

**Date:** 2026-11-13 (Asia/Kolkata)
**Status:** PLAN ONLY; not executed on any device.
**Scope:** Keep the user's Bitaxe Gamma 601/BM1370 on stock AxeOS while investigating a recurring on-screen "Power Fault Detected. Check your Power Supply."
**Core doctrine:** Protect working equipment. Develop in reversible moves. Get evidence before actuation. One physical or configuration change at a time. Do not equate loss of connectivity with regulator fault.

## Known facts vs unknowns
- Historical hardware-reported reference: ~1.337 TH/s; ~24.5 W; chip ~68.9 C; voltage regulator ~79 C; fan 100%, from October 24, not live.
- User has reported an intermittent power-fault warning and temporary recovery after restarting. The source has NOT been diagnosed.
- The miner's current IP, AxeOS version, input voltage, regulator status, power supply label, core-voltage setpoint, and *present* hashrate must be observed rather than assumed. Do NOT store private local IP or pool/wallet credentials in public GitHub.
- Existing QHAP production ESP32-S3 miner and experimental OTA-valid BM1370 receiver are not spare development targets; preserve them.
- Raspberry Pi microSD had unresolved EXT4 anomalies. Do not initiate write-intensive unattended logging there.

## Opening: baseline evidence and electrical preflight
1. Observe physical miner from outside: power connector secure, no smell, discoloration, sparks, unusual heat or fan stoppage. If any hazard, disconnect AC adapter at wall and stop. Do NOT power-cycle repeatedly.
2. Read only AxeOS `GET /api/system/info` with a local device already connected to the same network. Capture only: `power_fault`, `hardware_fault`, `voltage`, `power`, `temp`, `vrTemp`, `coreVoltage`, `coreVoltageActual`, `hashRate`, `fanrpm`, `fanspeed`, `uptimeSeconds`, `axeOSVersion`, `boardVersion`. `power_fault` may be absent when no fault. Never infer miner healthy solely from an HTTP 200.
3. Keep error evidence: use read-only `GET /api/system/logs` if needed; sanitize logs before external sharing.
4. Photograph the adapter label for *output*: Gamma 601 requires **5 V DC**, approx **6 A capacity**, correct center-positive **5.5×2.1 mm barrel** supply. Never use 12 V. Check cable/plug condition with power **removed**. No opening power adapters, probing exposed AC mains, soldering or simultaneous external supplies.
5. Thermal camera: scan exterior of barrel connector/heatsink/regulator vicinity at safe separation while the miner is operating normally. Thermal camera readings depend on emissivity and reflective metals: treat as anomaly detection not absolute die temperature. Do not touch energized contacts.
6. If using UNI-T multimeter, use DC-voltage mode and correct sockets; only test an unplugged-from-miner adapter barrel output if confident the probes cannot short the connector. Never probe live exposed PCB or exposed mains.

## Conditional branches — do NOT guess
- **Hazard present (smell, melting, connector hot/damaged, sparks, voltage wrong, severe overheating):** Power down at wall and stop; replace/inspect with qualified technician or correct-rated supply; no remote restart.
- **Clearly low/falling input voltage, or log input-undervoltage:** Verify correct 5V rated supply/cable/connector, avoid DC extensions. Test with manufacturer-compatible known-good supply if available and verified. A 6A *capacity* is not a forced 6A current draw. Do not use 12V.
- **Regulator overcurrent/overvoltage/temperature status:** Stop/review power path or cooler; no automatic retries, no voltage/frequency increases.
- **Out-of-range core voltage in log:** Compare current configured setting to manufacturer-appropriate normal values and firmware version. The regulator's configured nominal valid range may begin at 1000 mV, but *do not prescribe a single target* or make changes without exact model/version and controlled rollback. Check that protective thermal reductions are not resulting in an invalid setpoint.
- **Wi-Fi/pool connection issue with normal power telemetry and no regulator fault:** Investigate 2.4 GHz connectivity and pool status, do not treat as electrical fault. An HTTP outage is not permission to power-cycle.
- **No current fault:** Use a clean read-only baseline window and stop changing settings; monitor for recurrence.

## Middlegame: controlled reversible improvements
- Restore clean airflow, leave approved 5 V active fan connected, inspect dust with power removed, stabilize device stand/cable strain, keep flammable material clear.
- **No** heatsink removal/thermal paste work, firmware flashes, frequency increases, core-voltage changes, ASIC UART injections or GPIO modification tonight without separate proof-backed safety gate.
- Make only one documented correction per iteration. Observe post-change input voltage, regulator/chip temperatures, hashRate, faults, and network shares. Abort immediately on a repeated power fault.
- Do not claim a higher theoretical hashrate raises valid *block chances* if the device is losing uptime.

## Endgame: QHAP ESP32 watchtower, independent from miner
- Use a **verified genuinely spare** ESP32-S3, powered separately by its own USB supply. Do **not** repurpose the validated App1 receiver or reference mining ESP32. Connect to same 2.4 GHz Wi-Fi; no electrical wires between ESP32 and Bitaxe.
- Poll `GET /api/system/info` at approximately 60 s intervals; maintain local RAM ring buffer with time/uptime, fault strings, VIN, VR temperature, chip temperature, power, hashrate and fan. Avoid high-frequency flash writes and leaks of wallet/network credentials.
- Implement power fault state machine: UNKNOWN -> HEALTHY or FAULT_LATCHED -> MANUAL_INSPECTION. When API inaccessible, state is UNKNOWN, not "power fault". Alert with reason/time; do not mask electrical protection.
- **Restart endpoint exists** (`POST /api/system/restart`) but MUST remain disabled until the underlying electrical fault is resolved, controlled tests are passed, an independent manual arm is provided, a cooldown is enforced and attempts are limited. Never add an automatic power-cut relay to override protection.
- If approved later for non-electrical software faults: one attempted restart maximum in 24h; post-restart verification; repeat fault => permanent stop and human intervention.

## Victory/pass-fail criteria
PASS for *diagnostics* = known adapter identity and condition, saved status snapshot, classified regulator fault from actual logs, no dangerous interventions, candidate cause recorded.
PASS for *return to operating observation* = stable input power (per the board's validated normal range), normal cooling, non-zero stable hashrate, pool accepted work, no repeat electrical fault over observation. 24–48h telemetry is preferable before tuning.
FAIL/STOP = short, melting, damaged wiring, thermal anomaly, persistent low voltage, overcurrent, explicit VR fault or unverified supply compatibility; no auto restart.
Any apparent optimization must be benchmarked against a baseline as **effective valid hashes × uptime**, never marketed as a SHA shortcut.

## First noninvasive local Windows PowerShell command
```powershell
$miner = Invoke-RestMethod 'http://<YOUR-BITAXE-IP>/api/system/info' -TimeoutSec 8
$miner | Select-Object axeOSVersion,boardVersion,power_fault,hardware_fault,voltage,power,temp,vrTemp,coreVoltage,coreVoltageActual,hashRate,fanrpm,fanspeed,uptimeSeconds | Format-List
```
Replace placeholder locally; do not paste private SSID/password, wallet, or unredacted logs into a public issue.

## Evidence & sources
- Project playbook: `docs/QHAP_STRATEGIC_PLAYBOOK.md`
- Prior handoff: `docs/QHAP_NEW_CHAT_HANDOFF_2026-10-27.md`
- Official ESP-Miner API: https://github.com/bitaxeorg/ESP-Miner/blob/master/main/http_server/openapi.yaml
- Official Gamma hardware repository: https://github.com/bitaxeorg/bitaxeGamma
- Gamma model and fault guidance: https://bitaxe.de/en/bitaxe-hardware-fault-diagnosis
- Official voltage regulator defaults vary by device/family and firmware; do not overgeneralize.

**Explicitly not executed:** No miner access, measurement, power-cycle, firmware operation, ESP32 flash, or verified mining improvement performed in this conversation.
