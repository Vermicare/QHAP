# QHAP ESP32 AutoGuard v2 — failure-aware recovery proposal

**Date:** 2026-11-11. **State:** Host-side guard logic tested, **not deployed**. The full standalone Arduino prototype and 25-test C++ harness are stored as the conversation artifact `QHAP_AutoGuard_v2.zip`, **not as source files in this commit**.

## Background
User reports repeated mains interruptions, slow Wi-Fi return, and recurring Bitaxe Gamma 601 AxeOS `Power Fault Detected. Check your Power Supply.` alerts. A previous pool screenshot showed a prolonged time without submitted shares. Bitaxe local address last supplied: `192.168.1.33`. Prior ASIC temperatures and hashrate are historical only, not live observations.

## Safety-critical distinction
AxeOS regulator's `power_fault` is **not synonymous with Wi-Fi outage**. Actual faults can reflect input undervoltage, overcurrent or high temperature. Resetting repeatedly can stress an electrically unsafe adapter/connector/regulator. The guard must **never automatically restart a confirmed electrical/hardware/thermal fault**. No smart-plug relay is implemented.

## ESP32-S3 architecture
A dedicated spare ESP32-S3 uses read-only `GET /api/system/info` and watches:
- `power_fault`, `hardware_fault`, input `voltage`, `temp`, `vrTemp`
- `hashRate`, `hashRate_1m`, `totalHashes`, `uptimeSeconds`
- `miningPaused`, `overheat_mode`, `primaryPoolError`, fallback pool indicator
- Local Wi-Fi status and system time

### State-to-action contract
| Observation | Behavior |
|---|---|
| ESP32 Wi-Fi down / mains just returned | Automatically reconnect controller's Wi-Fi; allow 10 min Wi-Fi settling and 15 min controller boot settling; never request Bitaxe restart |
| Bitaxe API unreachable | Log unavailable; no HTTP restart possible |
| Mining startup / pause | Wait; respect paused state |
| Pool errors | Diagnose as external connection/pool issue; no ASIC reboot |
| Actual power/hardware fault, unsafe or missing readings | Persistent electrical lockout; **no restart** |
| Confirmed genuine software hashing stall | Only after prior observed healthy hashing, zero current/1-min hashrate and no total hash progress for 20 min, Bitaxe uptime >30 min and valid sensors; may request one guarded AxeOS restart **only when deliberately armed after hardware inspection** |
| Repeated problems | One attempt max in 24 h, two in 7 d, persistent timestamps recorded before HTTP call, NTP time mandatory; when uncertain abstain |

Recovery defaults to **unarmed**: `HARDWARE_INSPECTION_COMPLETE=false`; the user must first correct the physical power fault and verify a correctly-rated supply/connector/temperatures. Ordinary loss of connectivity must not count toward a hashing-stall timer.

## Validation
A pure C++ logic module compiled with `g++ -std=c++17 -Wall -Wextra -Werror -pedantic` and passed **25 local host assertions** covering fault classifications, deliberate pause, overheat, low voltage, missing telemetry, Wi-Fi recovery, valid hash progress, no unintended cold-start restart, false stall and rolling restart budgets. This does **not** demonstrate an Arduino toolchain compile, field test or successful Bitaxe restart. No production firmware or live device was changed.

## Limitations / next physical evidence
- A monitor also needs power to run; a blackout stops both ESP32 and Bitaxe unless backed by appropriately rated power supplies/UPS.
- Original ESP32 mining firmware and the experimental COM7 OTA receiver must remain untouched.
- Before enabling any remote software restart, inspect the Bitaxe 5 V/6 A supply, barrel connector, voltage under load, VR thermal behavior, and stock settings. Local HTTP dashboard is read-only, trusted LAN only. Don't expose to Internet.
- This controller cannot repair an electrical fault; alert/remediation beyond its class rules requires a human.

## Official source interfaces
- [ESP-Miner OpenAPI](https://github.com/bitaxeorg/ESP-Miner/blob/master/main/http_server/openapi.yaml)
- [System info JSON implementation](https://github.com/bitaxeorg/ESP-Miner/blob/master/main/http_server/system_api_json.c)
- [TPS546 regulator fault decoding](https://github.com/bitaxeorg/ESP-Miner/blob/master/main/power/TPS546.c)

**Status:** Experimental design checkpoint. No restart automation is active on the physical miner.
