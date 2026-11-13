# QHAP Watchtower v3.0 — Three-in-one reliability release

**Date:** 2026-11-14 IST
**Status:** Local development package CREATED and host-side tests PASSED; **NOT YET COMPILED IN ARDUINO IDE OR DEPLOYED TO ESP32**.
**Hardware target:** separate, verified ESP32-S3 COM9, 8MB flash, 8MB OPI PSRAM.
**Pre-existing hardware truth:** Watchtower v2 on COM9 runs from independent USB power and reports live read-only AxeOS data; during an earlier verified power-fault event, Gamma 601 showed VIN 5.46875V, Vcore 21mV, zero hashrate. User subsequently restarted miner and said hashing resumed; root cause remains unknown.

## Objectives — implemented in v3 source package
1. **Persistent history** independent from Bitaxe memory, on ESP32 LittleFS `spiffs` data partition only. Sample every 5 minutes and immediately on classification changes. Four bounded CSV files: current/previous samples (320KiB each) and current/previous events (64KiB each). Includes observed miner restarts inferred from uptime decreasing. Does not store Wi-Fi credentials/pool/wallet details. One-time initialization of *previously unused filesystem partition* allowed; thereafter mount failure does **not** automatically erase records.
2. **Opt-in phone notifications** through Telegram Bot API `sendMessage` over verified HTTPS using publicly trusted Go Daddy Root Certificate Authority - G2 (no `setInsecure()`). Default disabled until private token/chat ID supplied. Repeated-reading fault confirmation (two polls for regulator faults, three for API/Wi-Fi and zero hashrate), 30-minute cooldown persisted in ESP32 NVS, 10-minute retry interval on failed alerts, recovery alert only after two hashing observations and a previous sent fault. Notifications cannot be instantaneous during total power/network loss.
3. **Reliability diagnostics**: classify actual regulator fault separately from Wi-Fi and API loss; track AxeOS VIN, ASIC core V, temperatures, reported hashrate, Wi-Fi signal and share counters; record reboot observations. These **cannot independently measure AC mains voltage or prove the supply is safe**.

## Security & safety
- Public local HTTP on a PG network is not a trustworthy place for unprotected telemetry: the v3 dashboard and CSV APIs require a locally configured **16+ character password**, using Digest authentication. HTTP content itself remains unencrypted; recommend personal trusted hotspot, no port forwarding.
- The v3 firmware's only request to the Bitaxe is **GET /api/system/info**. No `POST /api/system/restart`, GPIO relay control, overclock, settings PATCH, or power actuation. Telegram uses HTTPS POST to *api.telegram.org*, not the miner.
- The source and bundled `secrets_example.h` contain placeholders, NOT Wi-Fi passwords or bot tokens; `secrets.h` is excluded from the ZIP and should never be committed.
- User chose to skip physical multimeter probing. Keep respecting this. Correct adapter label 5V 6A did **not** establish electrical stability.
- Preserve existing COM9 backup 8,388,608 bytes SHA256 `FF59F30CF44FCFF26732266A0A4BC025A866BC37324F008939E26B463AEE531B`.
- Do not touch COM7, COM8 or Bitaxe firmware.

## Local development validation
- Portable native **19 state-assessment assertions PASS**.
- Portable native **17 alert-decision assertions PASS**.
- Python **6 structural/safety tests PASS**.
- Total **42 checks PASS**, plus ZIP integrity test.
- Full Arduino ESP32 sketch compile and ESP32 runtime **not executed yet**; Telegram end-to-end TLS/bot notification delivery untested.
- Generated ZIP file (in conversation artifacts): `QHAP_Watchtower_V3.zip`; SHA256 `0fd7a8ba3a67447d5fa6b85f077415c3c59314b0ab4c4ebb39d2ab69628b0724`.
- ZIP includes source, local credentials template, tests and thorough README. The source files are in the ZIP; they have **not yet been separately committed into this GitHub repository**.

## Final optional notification self-test
- A local Serial Monitor command `TEST_ALERT` sends a **benign Telegram test message** if configured, at most once every 30 minutes. It does not touch the Bitaxe, change the saved alert state, or induce faults; reports a failure if Telegram TLS/credentials/time are unavailable.

## Next acceptance gates
1. User extracts v3 ZIP, creates new `secrets.h` from `secrets_example.h`, privately migrates Wi-Fi settings and sets a unique dashboard password. A v2 `secrets.h` cannot be copied wholesale because v3 adds fields.
2. In Arduino IDE 2.3.11, Espressif core 3.3.11 and ArduinoJson 7.4.3, board ESP32S3 Dev Module, Flash=8 MB, PSRAM=OPI, Partition=8M with SPIFFS, USB CDC On Boot=enabled, Erase All Flash=disabled. **Verify** compiler first, no upload on failed build.
3. Confirm port corresponds to the new COM9 ESP32-S3, not production COM7 or experimental COM8. Upload only after build is clean. Initial boot: NVS marks filesystem initialized. Check Serial Monitor `STATUS` for storage `OK`, connected Wi-Fi, synchronized clock; if mount fails on subsequent boot, DO NOT autoformat.
4. Check HTTP Digest login/dashboard, simulated? (v3 defaults to LIVE read-only), verify current Bitaxe status and live history entries; download CSV and check rollover after longer use.
5. Opt-in Telegram setup privately with `@BotFather` only if the user wants alerts. Verify a real alert message arrives from a *controlled test condition* without deliberately provoking electrical faults. If TLS fails, do NOT bypass verification. Avoid repeated restarts to test.
6. Miner fault root cause is still unresolved. Require separate power-supply or professional electrical diagnosis before discussing automatic restart.

**No deployment, phone delivery, restored mining uptime or improved hashrate claimed for v3 yet.**
