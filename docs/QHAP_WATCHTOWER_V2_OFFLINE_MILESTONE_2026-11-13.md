# QHAP Watchtower v2 — offline software checkpoint (2026-11-13)

**Scope:** Safe stand-alone ESP32-S3 monitoring concept for Bitaxe Gamma 601. This is a SOFTWARE-ONLY milestone, **not a successful hardware repair or mining deployment**.

## Current evidence from user
- AxeOS: `v2.12.2-dirty`; Gamma board `601`; `power_fault: Power Fault Detected.`; `hashRate: 0`.
- Reported `voltage: 5469 mV`, `coreVoltage: 1050 mV`, `coreVoltageActual: 21 mV`, `frequency: 525 MHz`, overclock disabled, `vrTemp: 30 C`, `temp: 27.75 C`, `current: -323 mA`. These are API readings, **not independently calibrated electrical measurements**.
- User supplied adapter-label photo: `YU0506`, 5 V DC, 6 A, center-positive. Label is **nominally compatible**; does not prove actual adapter regulation or plug dimensions.
- Official ESP-Miner v2.12.2 source confirms `GET /api/system/info` is supported, but `GET /api/system/logs` **is not**. Its regulator fault string may be generic. Source also confirms `power` includes an estimated board-power offset and `current` is the regulator output reading. Do not assert genuine -323 mA reverse current.
- **Physical live multimeter test declined by user for safety; DO NOT repeat the request or suggest another risky test.** Miner should remain off until electrical fault is resolved by safe supply swap or professional inspection.

## Software milestone verified off-device
- New deliverable: `QHAP_ESP32_Watchtower_V2.zip` distributed as a conversation artifact, not yet uploaded as source code to the GitHub repository.
- ZIP SHA-256: `a4c6163b5ebdbe7c6395821a99fff93cedf9455b2b465ca6136bb64de5d9da1d`.
- Standalone Arduino sketch defaults to **SIMULATION** with six representative states: hashing, repeated genuine API power-fault report, API unreachable, Wi-Fi offline, zero hashrate.
- Production-mode monitoring (when explicitly enabled) only **GETs** `/api/system/info` about once per minute and provides a local read-only dashboard. No `POST`, restart endpoint, GPIO manipulation, relay, flash or hardware actuation code.
- A separate Python local browser preview binds to `127.0.0.1:8765` for Windows testing and uses *only synthetic data*, with no miner contact.
- Portable C++ assessment logic passed **19 native assertions using GCC and Clang**. Python preview passed **3 unit tests** plus **local HTTP browser smoke test**. ZIP integrity checked successfully.
- This verifies **native C++ decision logic and Python preview only**. It is NOT an Arduino-ESP32 compile, physical flashing, real API trial, hardware safety certification or live hashrate improvement.

## Next guarded steps
1. User can extract ZIP on Windows and run `py .\tools\preview_simulation.py`; open `http://127.0.0.1:8765` on the same computer; stop with Ctrl+C.
2. Identify an **actually spare** ESP32-S3 board. Protect the QHAP reference miner and preserved receiver; DO NOT assume COM7 is free.
3. With a spare board confirmed, compile using Arduino IDE 2.x + Espressif ESP32 3.x + ArduinoJson 7.x; retain simulation-only mode for first flash and verify serial output/dashboard.
4. After adapter/connector/regulator root cause is independently resolved, optionally enable GET-only telemetry on same trusted LAN (no controls).
5. Do not promote automatic restart, overclock, core-voltage changes, experimental firmware or power-cycling hardware while electrical fault is unresolved.

## Official upstream basis
- `bitaxeorg/ESP-Miner`, tag `v2.12.2`, `main/http_server/http_server.c`
- `bitaxeorg/ESP-Miner`, tag `v2.12.2`, `main/power/TPS546.c`
- `bitaxeorg/ESP-Miner`, tag `v2.12.2`, `main/power/power.c`
- `docs/QHAP_STRATEGIC_PLAYBOOK.md`

**No physical operations, live measurements, firmware updates, Bitaxe settings changes, ESP32 flashes, or new hashing performance were performed.**

## COM9 backup checkpoint (user observed, 2026-11-14 IST)
- A physically new ESP32-S3 QFN56 revision 0.2 appeared on COM9, with 8 MB flash and 8 MB embedded PSRAM. Preserve COM7 and COM8.
- User ran `esptool v5.4.0 read-flash 0x0 0x800000` on COM9; read completed successfully in 66.6 seconds, reporting exactly **8,388,608 bytes**. `Get-Item` confirmed the output file size as **8,388,608 bytes**.
- Windows `Get-FileHash` SHA-256 was **FF59F30CF44FCFF26732266A0A4BC025A866BC37324F008939E26B463AEE531B**. This verifies a local file was saved with the stated digest; an independent second hardware read-and-compare was *not* performed.
- Backup is retained locally by user. **Never upload its raw flash contents** to the public repository: saved device configuration may contain secrets.
- No erase/write/flash operations occurred. **Next gate:** Verify the Arduino toolchain and compile the simulation-only Watchtower sketch; no board upload until compilation is clean and user opts to proceed. Use generic `ESP32S3 Dev Module` with 8 MB flash only after confirming toolchain and suitability.

## Arduino IDE hardware-target compile checkpoint (user observed 2026-11-14 IST)
- Arduino IDE 2.3.11, Espressif ESP32 platform 3.3.11, ArduinoJson 7.4.3.
- Board ESP32S3 Dev Module; 8 MB flash; 8M with SPIFFS (3 MB APP / 1.5 MB SPIFFS); OPI PSRAM; USB CDC on boot enabled. COM9 confirmed via read-only esptool as separate physical board.
- Final IDE **Verify PASSED**: sketch program **918,243 bytes (27% of 3,342,336 byte app partition)**; global memory **46,732 bytes (14% of 327,680 bytes)**.
- The ZIP sketch has `constexpr bool LIVE_MODE = false;`; it still calls `WiFi.begin()` with placeholders in `secrets.h`, so with no user-entered local Wi-Fi credentials the USB serial simulation runs but ESP32 browser dashboard is not accessible over WLAN. Do not collect passwords in chat.
- Before upload: ensure COM9 selected, Erase All Flash disabled, LIVE_MODE false; upload would replace COM9 application firmware only. Original full-flash readback exists locally; first live hardware test is **not yet demonstrated**.
