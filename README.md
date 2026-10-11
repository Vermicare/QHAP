# QHAP

**QHAP** is an experimental Bitcoin lottery-mining project built for the **ESP32-S3** using ESP-IDF.

The goal is to push a general-purpose microcontroller as far as practical while performing **real Bitcoin SHA256d work from live Stratum jobs**, rather than displaying simulated hashrate.

## Current Release

**v0.2.0 — validated extranonce2 cycling and safe 32-bit nonce exhaustion handling**

QHAP is now a live-capable solo-mining firmware that can receive real Stratum work, build valid Bitcoin candidate headers, scan the complete 32-bit nonce space without wrapping, generate a new search space through `extranonce2`, and continue mining the same job.

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
- Safe final partial nonce chunk through `UINT32_MAX`
- Automatic `extranonce2` cycling after full 32-bit nonce-space exhaustion
- Same-job coinbase, Merkle-root, and header regeneration after `extranonce2` changes
- Real pool-share target comparison
- Automatic switching to newly received mining jobs
- `mining.submit` generation for qualifying shares
- Pool share acceptance/rejection parsing
- Restart after detected Stratum disconnection

## Observed Production Performance

After the v0.2.0 production firmware was flashed, live mining was observed at approximately:

**268–272 kH/s**

Typical live scans were around **270 kH/s** while processing real CKPool jobs.

Example production output:

```text
=== LIVE NONCE SCAN ===
Iterations: 100000
Rate: 271.30 kH/s
Nonce range: 0 - 99999
Final nonce: 99999
Pool-share candidates: 0
```

This rate represents actual ESP32-S3 hardware SHA256d work against live Bitcoin candidate headers.

## v0.2.0 Extranonce2 Validation

The complete nonce-exhaustion path was intentionally accelerated once for validation.

QHAP successfully scanned:

```text
Nonce range: 4294817295 - 4294917294
Iterations: 100000
```

followed by the final partial range:

```text
Nonce range: 4294917295 - 4294967295
Iterations: 50001
Final nonce: 4294967295
```

It then detected exhaustion of the current 32-bit nonce space:

```text
Nonce space exhausted for current extranonce2
Advancing extranonce2 and rebuilding job
```

The same Stratum job was rebuilt with:

```text
Extranonce2: 0000000000000001
```

The regenerated coinbase changed, which changed the Merkle root and block header. The rebuilt header was independently verified again:

```text
Hardware vs PSA: MATCH
```

Mining then resumed from nonce zero on the regenerated header:

```text
Rebuilt same job with extranonce2: 0000000000000001

=== LIVE NONCE SCAN ===
Iterations: 100000
Nonce range: 0 - 99999
```

This validates the intended continuous search-space expansion path instead of repeating nonce work or waiting unnecessarily for a new Stratum job.

## How QHAP Mining Works

The **ESP32-S3 itself is the miner**.

The development PC is only required to build, flash, configure, and inspect serial logs. Once flashed, the ESP32-S3 performs the mining work autonomously while powered and connected to Wi-Fi.

The live pipeline is:

```text
Bitcoin network
      ↑
   CKPool / Stratum
      ↓
ESP32-S3 QHAP
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
Nonce 0 ... 4,294,967,295
      ↓
Increment extranonce2
      ↓
Rebuild coinbase / Merkle root / header
      ↓
Repeat
```

Each nonce is a new SHA256d attempt. When the 32-bit nonce field is exhausted, changing `extranonce2` changes the coinbase transaction, which changes the Merkle root and therefore produces an entirely new block-header search space.

## Pool Share vs Bitcoin Block

These are not the same thing.

A **pool share** is a hash that satisfies the pool's easier share target. It proves that the miner performed real work.

A **Bitcoin block** is a hash that satisfies the much harder Bitcoin network target.

A network-valid Bitcoin block necessarily also satisfies an easier pool target, so QHAP's current share-submit path would submit such a result. However, the current firmware does **not yet emit a dedicated persistent "BITCOIN BLOCK FOUND" event** separate from an ordinary qualifying pool share.

Therefore:

```text
Pool-share candidate / accepted share
        ≠
Bitcoin block found
```

## Solo CKPool payout semantics

QHAP is currently configured for **solo mining through solo.ckpool.org**, not a proportional shared-reward pool.

That means an easier CKPool share is useful as proof/telemetry, but it does **not** accumulate a fractional Bitcoin balance. Under solo.ckpool.org's current published terms, shares are cosmetic for feedback and the service charges a **2% fee** if a miner actually solves a Bitcoin block.

As of October 2026, Bitcoin's block subsidy is **3.125 BTC**, plus transaction fees. Therefore, with the current solo setup:

```text
ordinary hash / near miss      -> 0 BTC
CKPool share only              -> 0 BTC payout by itself
Bitcoin-network-valid block    -> full solo block reward, less CKPool's advertised fee
```

So QHAP does **not** gradually earn 0.001 BTC, 0.01 BTC, 0.1 BTC, etc. from partial work in the current solo configuration. It is effectively an all-or-nothing lottery at the payout level.

At the current 3.125 BTC subsidy, a successful solo block would produce a reward greater than 1 BTC even after a 2% service fee, before considering positive transaction fees. This changes across future Bitcoin halvings and would also be different if QHAP were intentionally moved to a conventional pooled payout scheme.

A coinbase reward is not immediately spendable; Bitcoin coinbase outputs mature after 100 blocks.

Reference: solo.ckpool.org's current public terms and Bitcoin's current subsidy schedule should be rechecked whenever payout documentation is updated.

## What Are the Chances?

QHAP is a genuine lottery miner, not an economically competitive Bitcoin miner.

During one validated live job, QHAP received:

```text
nBits: 17021ef0
```

At approximately **270 kH/s**, using that observed network target as a snapshot, the probability of finding a Bitcoin-valid block is approximately:

- **1.49 × 10^-11 per year**
- about **0.00000000149% per year**
- roughly **1 chance in 66.9 billion per year**
- an average statistical waiting time of roughly **66.9 billion years** if hashrate and network target remained unchanged

These are probability estimates, not a countdown. Every individual hash is independent, so a winning hash could theoretically occur immediately, while the network difficulty also changes over time.

The engineering value of QHAP is therefore far greater than its expected financial return.

## How Would I Know If It Found Bitcoin?

A real Bitcoin block would produce evidence outside the ESP32 itself.

The practical confirmation path is:

1. QHAP finds a hash that satisfies the Bitcoin network target.
2. That result also satisfies the pool target and is submitted through `mining.submit`.
3. The pool validates and broadcasts the block.
4. The block becomes visible on the Bitcoin network.
5. The configured mining address / wallet can be checked against the pool's solo-mining payout record and the public blockchain.

An ordinary accepted pool share is **not** sufficient proof that a Bitcoin block was found.

For stronger autonomous evidence, a future QHAP release should add a dedicated network-target check inside the high-speed scan path plus nonvolatile storage for a permanent block-found record.

## Validation

The optimized hardware SHA256d implementation was repeatedly cross-checked against an independent PSA SHA-256 implementation during development.

Live Bitcoin headers produced matching results:

```text
Hardware vs PSA: MATCH
```

The Bitcoin genesis block was also used as a known-reference validation case.

Validated behavior now includes:

- Correct SHA256d against known data
- Correct live coinbase construction
- Correct Merkle-root construction
- Correct live 80-byte Bitcoin header construction
- Hardware SHA vs independent PSA SHA agreement
- Correct pool target derivation
- Real sequential nonce scanning
- Safe handling of the final nonce `4294967295`
- Successful `extranonce2` increment
- Successful same-job header regeneration
- Successful resume from nonce zero on the regenerated header

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

To inspect an already-running miner **without reflashing**:

```powershell
idf.py -p COM7 monitor
```

Exiting the serial monitor does not stop the ESP32. As long as the board remains powered and connected to Wi-Fi, the flashed firmware continues running independently.

## Production Firmware Fingerprint

Current validated production binary SHA-256:

```text
2FD674648F9A4E98B4EF188BB524ED072C95AF226DCCEDF57CBC45DC6C87357E
```

The production binary itself is intentionally excluded from the repository.

## Current Validation Boundary

The firmware contains a real `mining.submit` path and share-response parser.

However:

- No qualifying pool share has yet been encountered during development, so an actual accepted share has not yet been empirically observed.
- A dedicated high-speed Bitcoin-network-target event is not yet separated from the ordinary pool-share submit path.
- The automatic disconnect/restart path is implemented but has not yet been intentionally fault-tested.

These are the main remaining validation boundaries after v0.2.0.

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
- autonomous long-running mining behavior on constrained hardware

It is **not a profitability claim**.

---

**QHAP — real Bitcoin work on hardware that was never designed to be a Bitcoin miner.**
