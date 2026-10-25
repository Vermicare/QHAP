# QHAP BM1370 UART Receiver — Verified Session Checkpoint

**Session date:** 25 October 2026 (IST)  
**Working branch:** `feature/bm1370-integration`  
**Status at shutdown:** The isolated ESP32-S3 receiver is successfully installed in OTA App1 and has marked its image `VALID`. CRC5 self-test, UART initialization, OTA persistence across manual restarts, and periodic idle heartbeats were observed. **The Pico H has not transmitted any UART test frame yet.** Power can safely be removed.  

This checkpoint records the practical state at the end of the session, distinguishes measured results from planned work, and preserves the safe resume procedure.

## 1. Hardware boundaries

| Device | Present state | Required handling |
|---|---|---|
| **Experimental ESP32-S3-DevKitC-1-N8R8** (rev v0.2, 8 MB flash / 8 MB PSRAM) | USB-to-UART COM7 on Windows; isolated BM1370 receiver application in OTA App1 | This is the board used for flashing and serial monitoring. |
| **Windows laptop** | ESP-IDF v6.1 project/build tools and verified private local backups | Keep source, build artifacts, and backups intact. |
| **Raspberry Pi 5 (8 GB)** | Separate powered computer | Shut down its operating system cleanly before removing power. |
| **Raspberry Pi Pico H** | Connected by USB to Raspberry Pi 5 | No Pico TX/ground jumper connection to experimental ESP32 yet. |
| **Bitaxe Gamma 601** | Separate stock AxeOS Bitcoin ASIC miner | **Not involved in these tests; do not change or power it down unless independently desired.** |
| **Existing QHAP real-miner implementation** | Separate from isolated receiver | Do not confuse lab UART receiver with Bitcoin mining performance, live Stratum work, or shares. |

Planned **signal-only** connection for the next session (verify actual physical board markings/orientation before connecting):
- Pico H **GP0 / UART0 TX (physical pin 1)** → experimental ESP32-S3 **GPIO17 / UART1 RX (J1 pin 10 per prior board mapping)**.
- Pico H **GND (physical pin 3)** → experimental ESP32-S3 **GND (J1 pin 22 per prior board mapping)**.
- Keep both USB-powered independently; **do not tie their 3V3, 5V, VBUS or VSYS power rails**. Disconnect both USB supplies before adding jumpers. Common ground is necessary for UART.

## 2. Local Windows build and implementation

**Project:** `C:\esp_projects\qhap_bm1370_receiver_lab`  
**Source:** `C:\esp_projects\qhap_bm1370_receiver_lab\main\qhap_bm1370_receiver_lab.c`  
**Component:** `C:\esp_projects\qhap_bm1370_receiver_lab\main\CMakeLists.txt`  
**Build image:** `C:\esp_projects\qhap_bm1370_receiver_lab\build\qhap_bm1370_receiver_lab.bin`  
**Backup folder:** `C:\esp_projects\qhap_lab_backups`  
**ESP-IDF:** v6.1; `idf.py -p COM7 monitor --no-reset` used with IDF monitor v1.10.0; esptool v5.4.0.

Receiver characteristics:
- Standalone **receive-only** UART1, ESP32-S3 **GPIO17 RX**, **115200 8N1**, 11-byte candidate frames with preamble `AA 55`.
- CRC5 algorithm starts `0x1F`, polynomial `0x05`, checks residue `0` over payload `frame[2:11]`.
- Built-in known-good mocked frame: `AA 55 A1 B2 C3 D4 02 5A 12 34 8C`; expected nonce `0xA1B2C3D4`, raw ID `0x5A`, version `0x1234`.
- Self-test also corrupts a byte and verifies bad CRC is rejected.
- Logs either `VALID nonce=...` or `BAD_CRC ...`, and a `READY valid=... rejected=...` heartbeat every 10 seconds.
- No Wi-Fi, pool, actual ASIC control, or mining implemented in this receiver lab.

The source was patched **locally** to add `#include "esp_ota_ops.h"` and to inspect the running OTA app state after CRC5 self-test and successful UART initialization. Only if the running image state is `ESP_OTA_IMG_PENDING_VERIFY` does it call `esp_ota_mark_app_valid_cancel_rollback()`, check `ESP_OK`, and log `QHAP OTA APP1 CONFIRMED VALID`. `main/CMakeLists.txt` now has `PRIV_REQUIRES esp_driver_uart app_update`. Saved `sdkconfig` contains `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y` (verified at line 684 at time of session). Both edited source files have prepatch `.before_ota_fix.bak` copies. **Do not assume this local source patch is pushed as source code to GitHub; this document only records it.**

## 3. Flash partition and recovery situation

The **original bootloader and partition table are retained**. Original flash partition layout:

| Partition | Offset | Size |
|---|---|---|
| NVS | `0x9000` | 20 KiB |
| OTA selection metadata | `0xE000` | 8 KiB |
| App0 (original application) | `0x10000` | 1280 KiB |
| App1 (experimental receiver) | `0x150000` | 1280 KiB |
| SPIFFS | `0x290000` | 1408 KiB |
| Coredump | `0x3F0000` | 64 KiB |

- The original OTA App0 selector sector at `0xE000` has sequence `1`, state `UNDEFINED`, valid CRC; preserved in all prior controlled verification reads.
- The App1 selector at `0xF000` was originally inactive, then staged as sequence `2`, state `NEW`. First unconfirmed test boot transitioned to `ABORTED`, while App0 remained bootable. This **proved original bootloader automatic OTA rollback works**.
- A recovery rehearsal performed a real ROM-mode sector rewrite of the original contents to `0xF000` and verified the full 8 KiB OTA partition was unchanged. Recovery ROM connection on COM7 has been proven, not only planned.
- After the corrected App1 image was reflashed, a second verified App1 activation set the selector to seq `2`, state `NEW` and preserved the App0 selector. The receiver subsequently confirmed itself as `VALID` on its first corrected boot.
- Later **intentional**, manually triggered EN/RESET presses booted the same receiver and logged `OTA state=2; confirmation not required`: this is `ESP_OTA_IMG_VALID`. No automatic rollback on the corrected application was observed.
- There has **not yet been a separate ROM-mode flash read of the OTA metadata following the successful VALID confirmation**, but the application itself queried and reported state 2 on subsequent boots. There is no need to repeat activation.
- Manual ROM entry: with USB attached and monitor closed, hold **BOOT**, tap/release **RESET/EN**, release **BOOT**. For read-only ROM inspection, esptool is used with `--chip esp32s3 --port COM7 --baud 115200 --before no-reset --after no-reset --no-stub`.
- **Do not use `idf.py flash` on this lab board** when intending to preserve the original bootloader, partition table, App0 and other sectors. Previous firmware updates targeted App1 only at `0x150000`, with dedicated `write-flash` commands.

### Private backups (paths and hashes only; never commit the binary bytes or secrets)

Reconstructed original entire 8,388,608-byte flash dump:
- `C:\esp_projects\qhap_lab_backups\esp32s3_full_8mb_reconstructed.bin`
- `D:\QHAP_Recovery_Private\esp32s3_original_8mb_backup.bin`
- Both verified SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`.
- Important limitation: reconstruction used multiple reads across resets, so not an atomic snapshot and not independently restore-tested. C: and D: were reported to reside on the **same physical NVMe SSD**, so they are not independent disaster-recovery copies.

Original OTA second-sector recovery image:
- `C:\esp_projects\qhap_lab_backups\qhap_app0_original_f000_RECOVERY.bin` (4096 bytes).
- SHA-256 `58B59EB397FCC8B699D265A18AF84E063DC9BAB0BD03632AD2F91E70038AA788`.

App1 activation template:
- `C:\esp_projects\qhap_lab_backups\qhap_app1_ota_candidate_OFFLINE_ONLY.bin` (4096 bytes).
- Sequence 2, `ESP_OTA_IMG_NEW` state 0, CRC `0x55F63774`. **Not to be programmed again while App1 is already VALID.**

## 4. Receiver firmware build and verification results

| Image | Size | SHA-256 | Meaning |
|---|---:|---|---|
| Original test receiver | 195,888 bytes | `14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE` | First successful App1 boot; lacked OTA image confirmation. |
| Corrected OTA-validating receiver | **217,856 bytes** | `91393A2C6721E14DA06414474C2532F73F8691693C274984272B68F79F84250B` | **Current verified App1 image**. |

Before rebuilding, the original application build binary was hash checked and backed up as `C:\esp_projects\qhap_lab_backups\receiver_before_ota_confirmation_fix.bin`.

The corrected image was built using ESP-IDF v6.1. `esptool image-info` reported:
- ESP32-S3 target, six segments, `8MB / DIO / 80MHz` image header.
- Valid checksum (`0x87`) and valid image validation hash `f9de02687baf5f508bd5563a0663c54fcd0ccf85ee88cae530de6da8e5183793`.
- Corrected image size **217,856 bytes**, well within the App1 1,310,720-byte partition.
- Application-only ROM flash at `0x150000` reported `Hash of data verified`.
- A second independent ROM-mode read of **exactly 217,856 bytes** from `0x150000`, saved as `C:\esp_projects\qhap_lab_backups\app1_ota_fixed_readback.bin`, compared **byte for byte** to the built image and matched SHA-256:
  - `QHAP OTA-FIXED APP1 READBACK PASS`
  - `BYTE-FOR-BYTE FIRMWARE MATCH: PASS`
  - `SHA256 VERIFICATION: PASS`

## 5. Boot evidence and resolved rollback issue

**First App1 boot (old receiver):**
- Booted successfully under the preserved original second-stage bootloader, reached ESP-IDF v6.1, printed `BM1370 CRC5 SELF-TEST PASS`, started UART1 RX and emitted `READY valid=0 rejected=0`.
- Did not acknowledge OTA validation. A subsequent manual reboot selected original App0.
- Live read-only OTA sector inspection: App0 seq 1 / UNDEFINED / CRC valid, App1 seq 2 / **ABORTED** / CRC valid. This proved automatic rollback had occurred.

**Second App1 boot (corrected receiver):**
- App1 activation metadata programmed and independently verified before a normal EN reset.
- The actual monitor log showed:
  - `QHAP isolated BM1370 receiver starting`
  - `BM1370 CRC5 SELF-TEST PASS`
  - `UART1 RX GPIO17, 115200 8N1; receive-only`
  - `No network, mining pool, or ASIC control enabled`
  - **`QHAP OTA APP1 CONFIRMED VALID`**
  - Repeated `READY valid=0 rejected=0`.
- After two **user-confirmed manual** RESET/EN presses, both reboots returned to the lab receiver and logged `OTA state=2; confirmation not required`. The final run showed continuous heartbeats through at least **150 seconds**. These resets were **not spontaneous faults**.
- The logs still include a warning that 8192 KiB physical flash exceeds a 4096 KiB image-header setting during boot, despite `image-info` on the corrected app reporting 8 MB. This warning did not prevent operation and is a **non-blocking investigation item**; its origin has not been proven.

## 6. Safe shutdown tonight

1. On the Windows laptop, close `idf.py monitor` with **Ctrl+]**; then unplug the ESP32-S3's USB power.
2. On the Raspberry Pi 5, execute `sudo shutdown -h now`; after orderly OS shutdown, disconnect power.
3. Pico H may remain physically connected to the Pi's USB port while unpowered; if the Pico still has power, disconnect the Pico USB cable.
4. The independent Bitaxe may remain running; it was not modified by the UART tests.
5. Do **not** erase flash, rewrite OTA metadata, or wire the two boards while powered. All firmware and configuration already written to flash survive loss of power.

## 7. Resume plan — next session

**No reflashing needed. No OTA reactivation needed.**

1. Power on the Raspberry Pi 5 and Windows laptop. Leave the Pico/ESP32 GPIO signal wiring unconnected initially.
2. Disconnect Pico H USB and experimental ESP32-S3 USB before adding any jumper wires, and visually confirm both boards' GPIO pin labels/orientation.
3. Add only **Pico GP0 TX → ESP32 GPIO17 RX** and **Pico GND → ESP32 GND**. No power-rail connections.
4. Restore USB power (Pico via Raspberry Pi, ESP32 via Windows laptop).
5. In the **ESP-IDF PowerShell**, verify the ESP32-S3 receiver with:
   ```powershell
   Set-Location "C:\esp_projects\qhap_bm1370_receiver_lab"
   idf.py -p COM7 monitor --no-reset
   ```
   If the device booted before the monitor attached and logs were missed, a manual EN/RESET on the receiver can recapture startup logs. Do not hold BOOT for ordinary boot. Expect `BM1370 CRC5 SELF-TEST PASS`, `OTA state=2; confirmation not required`, and `READY valid=0 rejected=0` with the Pico not transmitting yet.
6. Prepare **Pico H UART0 transmitter program** on the Pi (not yet flashed/written), using 115200 8N1 on GP0, then send known-good 11-byte frame `AA 55 A1 B2 C3 D4 02 5A 12 34 8C`. Expect ESP32 log showing valid nonce `0xA1B2C3D4` and an increasing valid counter. Ensure actual physical frame receipt; a Pico terminal `print("PASS")` alone does not count.
7. Send an intentionally CRC-corrupted frame, verify the ESP32 increments rejected count and resynchronizes. Repeat controlled tests and add logic-analyzer capture if useful.
8. Document observed GPIO frame results and tests. Only then proceed to extended BM1370 protocol simulation and optional logic-analyzer experiments.

**Important bounds:** Validated CRC5 UART frames from a Pico emulator are not evidence of mining new BTC, a BM1370 ASIC response, or live ASIC integrations. The separate Bitaxe Gamma 601 and original mining board remain outside the experiment.

---
**Source of truth:** verified PowerShell outputs and serial logs in the 25 October 2026 ChatGPT engineering session. Never commit private full-flash dumps, credentials, wallet secrets or identifiable network details.
