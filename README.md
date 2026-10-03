# QHAP

**QHAP** is an experimental Bitcoin lottery-mining project built for the **ESP32-S3** using ESP-IDF.

The goal is to push a general-purpose microcontroller as far as practical while performing **real Bitcoin SHA256d work from live Stratum jobs**, rather than displaying simulated hashrate.

## Current Status

QHAP currently supports:

- Wi-Fi connectivity
- Stratum V1 TCP connection
- `mining.subscribe`
- `mining.authorize`
- Live `mining.notify` job parsing
- Live pool difficulty updates
- Coinbase construction
- Coinbase SHA256d
- Merkle-root construction
- Real 80-byte Bitcoin block-header construction
- ESP32-S3 SHA hardware acceleration
- SHA-256 midstate reuse
- Sequential nonce scanning
- Automatic extranonce2 cycling after full 32-bit nonce-space exhaustion
- Real pool-share target comparison
- Automatic switching to new mining jobs
- `mining.submit` generation for qualifying shares
- Pool share acceptance/rejection parsing
- Restart after detected Stratum disconnection

## Observed Performance

Production live-mining firmware has typically measured approximately:

**263–265 kH/s**

Earlier isolated SHA256d optimization benchmarks reached higher rates before the complete Stratum, target-screening, and controller logic was integrated.

The displayed production rate represents actual ESP32-S3 hashing work.

## Validation

The optimized hardware SHA256d implementation was repeatedly cross-checked against an independent PSA SHA-256 implementation during development.

Live Bitcoin headers produced matching results:

```text
Hardware vs PSA: MATCH
```

The Bitcoin genesis block was also used as a known-reference validation case.

## Hardware

Development hardware:

- ESP32-S3-DevKitC-1-N8R8
- Dual-core ESP32-S3
- 240 MHz
- 8 MB flash
- 8 MB PSRAM
- 2.4 GHz Wi-Fi

## Software

- ESP-IDF 6.1
- C
- FreeRTOS
- lwIP
- cJSON
- ESP32-S3 SHA hardware accelerator
- Stratum V1

## Configuration

Run:

```powershell
idf.py menuconfig
```

Then open:

```text
QHAP Miner Configuration
```

Configure:

- Wi-Fi SSID
- Wi-Fi password
- Stratum server
- Stratum port
- Bitcoin address / worker
- Stratum password

Sensitive `sdkconfig` files are intentionally excluded from Git.

## Build

```powershell
idf.py build
```

## Flash and Monitor

```powershell
idf.py -p COM7 flash monitor
```

Replace `COM7` if the ESP32-S3 appears on another serial port.

## Mining Pipeline

```text
Wi-Fi
  ↓
Stratum TCP
  ↓
mining.subscribe
  ↓
mining.authorize
  ↓
mining.notify
  ↓
Coinbase construction
  ↓
SHA256d coinbase
  ↓
Merkle root
  ↓
80-byte Bitcoin header
  ↓
ESP32-S3 SHA hardware
  ↓
Midstate reuse
  ↓
Sequential nonce scan
  ↓
Pool-target comparison
  ↓
mining.submit
```

## Production Firmware Fingerprint

SHA-256:

```text
2FD674648F9A4E98B4EF188BB524ED072C95AF226DCCEDF57CBC45DC6C87357E
```

The production binary itself is intentionally excluded from the repository.

## Current Validation Boundary

The firmware contains a real `mining.submit` path and share-response parser.

However, no qualifying pool share has yet been encountered during development, so an actual accepted share has not yet been observed.

The automatic disconnect/restart path is implemented but has not yet been intentionally fault-tested.

## Security

Never commit:

- Wi-Fi passwords
- wallet recovery phrases
- private keys
- `sdkconfig`
- `sdkconfig.old`

A Bitcoin wallet recovery phrase is never required by QHAP.

## Purpose

QHAP is an engineering and research project exploring:

- Bitcoin block construction
- SHA-256 hardware acceleration
- low-level ESP32-S3 optimization
- Stratum protocol implementation
- embedded networking
- nonce-space management
- cryptographic correctness validation

It is **not a profitability claim**. Bitcoin network difficulty makes finding a block with an ESP32-S3 extraordinarily unlikely.

---

**QHAP — real Bitcoin work on hardware that was never designed to be a Bitcoin miner.**
