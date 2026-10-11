# QHAP BM1370 Integration

Status: **Phase A — protocol + virtual result path validated on ESP32-S3; physical BM1370 pending**

Target hardware: **Bitaxe Gamma 602 / 1 × BM1370**.

The purpose of this branch is not to turn QHAP into a stock AxeOS clone. QHAP keeps ownership of Stratum job construction, network/pool target validation, candidate protection, scheduling and submission. The BM1370 is introduced as a hashing accelerator behind a narrow driver boundary.

## Architecture

```text
CKPool / Stratum
       |
       v
QHAP Stratum + coinbase + merkle + target logic
       |
       v
QHAP ASIC job adapter
       |
       v
BM1370 protocol + UART transport
       |
       v
BM1370
       |
       v
nonce + version result
       |
       v
QHAP independent SHA256d verification
       |
       +--> pool target -> mining.submit
       |
       +--> network target -> BLOCK FOUND priority path
```

## Verified upstream protocol facts

Current Bitaxe ESP-Miner uses the BM1370 with BM13xx framing:

- host-to-ASIC preamble: `55 AA`;
- command header type: `0x40`;
- job header type: `0x20`;
- broadcast/group-all bit: `0x10`;
- command packets append CRC5;
- job packets append CRC16/CCITT-FALSE;
- result packets are 11 bytes and use response preamble `AA 55`;
- the result contains nonce, midstate number, encoded job/core ID and version-rolling bits;
- current firmware changes the ASIC UART to 1,000,000 baud after initialization;
- version rolling is configured in the BM1370 and returned version bits are shifted by 13 before reconstructing the rolled Bitcoin version.

The QHAP protocol module independently implements framing/checksums rather than importing ESP-Miner source wholesale.

## Phase A milestone

Compile and execute the protocol self-test before any BM1370 hardware is connected.

The self-test reconstructs a known BM13xx command frame:

```text
55 AA 51 09 00 18 F0 00 C1 00 04
```

This verifies the command header/length/CRC5 path.

## Pre-hardware validation milestone — 2026-10-05

The protocol and result-decoder layers have now been executed on the real ESP32-S3 target.

Observed:

```text
BM1370 protocol self-test: PASS
BM1370 mock self-test: PASS
Hardware vs PSA: MATCH
BM1370 mock live-header hash: MATCH
```

The live-header match was observed on multiple different CKPool jobs.

The virtual path is:

```text
real CKPool mining.notify
  -> QHAP coinbase / Merkle / 80-byte header
  -> synthetic BM1370 result
  -> BM1370 result decode
  -> nonce reconstruction
  -> exact header reconstruction
  -> independent SHA256d
  -> hash MATCH
```

During the same run, the existing native ESP32 SHA miner continued doing real Bitcoin work at about 270.23 kH/s. The mock BM1370 path is validation-only and adds no mining hashrate.

This milestone validates controller-side integration logic only. It does **not** validate physical BM1370 UART timing, initialization, real job encoding, real returned nonce behavior or silicon compatibility.

Before physical RX is trusted, QHAP must also implement and validate the exact checksum/CRC semantics for returned 11-byte result packets. The current decoder parses the final byte but does not yet use it as a hard integrity gate.

## Phase B — transport

After Phase A passes:

1. add ESP-IDF UART transport;
2. keep BM1370 power/reset disabled by default;
3. detect a real BM1370 by chip-ID response;
4. validate RX preamble/checksum and reject malformed responses;
5. measure TX failures, RX conversion rate and duplicate results.

Do not start at high clock/voltage.

## Phase C — live Bitcoin work

Translate the already validated QHAP live job into the BM1370 job format. Preserve QHAP's byte-ordering rules and maintain a job table so returned job IDs can be mapped to the exact original Stratum job.

Every ASIC-returned candidate must be reconstructed and independently SHA256d-verified by QHAP before submission.

## Phase D — block-safe submission

A returned candidate is handled in this order:

1. stale-job check;
2. duplicate-result check;
3. rebuild exact header including rolled version;
4. independent SHA256d;
5. pool target check;
6. Bitcoin network target check;
7. network-valid result receives highest submission/persistence priority.

## Phase E — optimization/research

Only after correctness:

- BIP310 version-mask enforcement;
- job-switch latency measurement;
- duplicate suppression;
- hashes/joule tuning;
- adaptive frequency/voltage;
- measured nonce-space scheduling;
- multi-BM1370 scaling.

## Important 2026 observations

Recent ESP-Miner reports include duplicate-result/serial-integrity behavior on at least one Gamma 601 and discussions around avoiding nonce-range wraparound duplicates. QHAP should therefore treat duplicate filtering, serial integrity metrics, stale-job protection and conservative nonce-space timing as first-class requirements rather than optional cleanup.

Gamma 602 remains the preferred first board because it is the later hardware revision and the official release notes describe DFM/routing improvements.

## Current gate

No hardware command is sent by this branch yet.

The first gate is:

```text
protocol module compiles
+
qhap_bm1370_protocol_self_test() == true
```

Only then do we add UART and hardware initialization.
