# QHAP Project Memory

> **Purpose:** Persistent engineering context for QHAP so future work can resume without reconstructing the project from chat history.
>
> **Maintenance rule:** Update this file whenever a validated milestone, measured performance result, release/tag, architecture decision, development-environment change, known validation boundary, or active resume point changes.
>
> **Security rule:** Never store Wi-Fi passwords, wallet seed phrases, private keys, API keys, or other secrets here. The configured Bitcoin worker/address may exist in local configuration but is intentionally omitted from this document.

Last consolidated: **2026-10-05**

---

## 1. Project identity and objective

**QHAP** is an experimental real Bitcoin solo-mining and mining-systems research project built initially around an **ESP32-S3**.

The project must always distinguish between:

- real Bitcoin SHA256d work and simulated/demo hashrate;
- a pool share and a Bitcoin-network-valid block;
- experimentally measured performance and estimates;
- engineering research value and expected financial return.

The current firmware performs real SHA256d work from live Stratum jobs.

### North-star objective

The current moonshot objective discussed for the project is:

> **Increase QHAP's total expected Bitcoin acquisition capability toward 1 BTC/day.**

This is a north-star systems target, **not a promise or guarantee**. The native ESP32 miner alone cannot approach that target. The roadmap therefore expands QHAP from a microcontroller miner toward a block-safe controller, ASIC architecture, multi-ASIC/fleet controller, supervisory layer, and later experimental quantum/hybrid research.

A broader future interpretation is allowed: maximize **BTC acquired per unit of money, energy, hardware, and time**, while retaining real solo-mining capability.

---

## 2. Current release and repository

Repository:

- **GitHub:** `Vermicare/QHAP`
- **Default branch:** `main`
- **Current documented release:** `v0.2.0`

Important commits:

| Commit | Meaning |
|---|---|
| `92b6202` | Production live ESP32-S3 Bitcoin miner |
| `1f77869` | Initial QHAP README |
| `41871ac` | README formatting baseline; tagged `v0.1.0` |
| `3e03ba9` | Extranonce2 cycling + safe nonce exhaustion |
| `82d54dd` | Document extranonce2 cycling |
| `284e76e` | Updated production firmware fingerprint; tagged `v0.2.0` |
| `f5fa273` | Expanded v0.2.0 technical/mining documentation |
| `3edc6d7` | Added probability-first engineering roadmap |
| `3107fbc` | Added persistent project memory |
| `d00693b` | Added Bitcoin network block-candidate detection; hardware-validated on ESP32 |
| `58bb197` | Merged block-safe candidate detection to `main` via PR #1 |

Relevant branches known during development:

- `main`
- `feature/extranonce2-cycling`
- local active development branch: `feature/block-safe-reliability`

At the start of the block-safe work, `feature/block-safe-reliability` was fast-forwarded locally to include `3edc6d7`.

---

## 3. Development environment

Primary development setup:

- **OS:** Windows
- **Shell:** PowerShell
- **Project path:** `C:\esp_projects\qhap`
- **ESP-IDF:** v6.1
- **ESP-IDF installation:** `C:\esp\v6.1\esp-idf`
- **ESP32 serial port used during development:** `COM7`
- **Language:** C
- **Runtime/platform:** FreeRTOS + lwIP
- **JSON:** cJSON managed component
- **Crypto/reference implementation:** PSA SHA-256
- **Optimized hashing:** ESP32-S3 SHA hardware via low-level SHA APIs/register access

Important configuration facts:

- Wi-Fi, Stratum host/port, worker/address, and Stratum password are configured through project Kconfig/`sdkconfig`.
- `sdkconfig` and `sdkconfig.old` are intentionally ignored by Git.
- Do not commit secrets.
- The production binary is intentionally not tracked by Git.

### PowerShell reliability note

Use **absolute paths** for scripted .NET file edits.

A later command using:

```powershell
[System.IO.File]::ReadAllText(".\main\main.c")
```

unexpectedly resolved against `C:\Users\Admin\main\main.c` even though the PowerShell prompt displayed `C:\esp_projects\qhap`.

For future automated edits, prefer:

```text
C:\esp_projects\qhap\main\main.c
```

and do not print a success message after an earlier failed validation/write.

---

## 4. Hardware

Validated mining hardware:

- **ESP32-S3-DevKitC-1-N8R8**
- ESP32-S3 revision observed: **v0.2**
- dual-core
- 240 MHz
- 8 MB flash
- 8 MB PSRAM
- 2.4 GHz Wi-Fi

Other project hardware available/planned:

- **Raspberry Pi 5 8 GB** — intended later as a supervisory/coordinator node, not the primary SHA256d engine.
- BH1750 and SCD40 were used in separate sensor experiments and are not required for QHAP mining.

Standalone power:

- A **5 V / 2 A Realme USB adapter** was checked and considered suitable for the ESP32-S3.
- Once flashed, the ESP32 can mine without the laptop as long as it remains powered and connected to Wi-Fi.
- Prefer open air or a ventilated enclosure for continuous operation; avoid a sealed box.

---

## 5. Current operational architecture

The **ESP32-S3 itself is the miner** in v0.2.0.

The laptop is used only for development, flashing, configuration, and serial monitoring.

Current live flow:

```text
CKPool / Stratum
      |
      v
ESP32-S3 QHAP
      |
      +-- mining.subscribe
      +-- mining.authorize
      +-- mining.notify
      +-- coinbase construction
      +-- coinbase SHA256d
      +-- Merkle-root construction
      +-- 80-byte Bitcoin header
      +-- SHA256d hardware mining
      +-- sequential nonce search
      +-- pool-target screening
      +-- mining.submit for qualifying work
```

After the complete 32-bit nonce field is exhausted, QHAP increments `extranonce2`, rebuilds the coinbase/Merkle root/header, and starts a new nonce search space from zero.

---

## 6. Pool and Stratum behavior

Validated live endpoint:

- **CKPool Stratum:** `stratum.ckpool.org:3333`

Validated protocol behavior:

- TCP connection
- `mining.subscribe`
- parse `extranonce1`
- parse `extranonce2_size`
- `mining.authorize`
- live `mining.notify`
- `mining.set_difficulty`
- live job switching
- pool target calculation
- `mining.submit` request generation
- share response parsing for request id 3

Example validated pool difficulty:

```text
10000
```

Corresponding pool target observed:

```text
0000000000068db22d0e5604189374bc6a7ef9db22d0e5604189374bc6a7ef9d
```

At about 270 kH/s, a difficulty-10000 share has a rough mean interval of about **5 years**, so seeing `Pool-share candidates: 0` for normal runs is expected.

### Important validation boundary

No real qualifying pool share has yet been encountered during development.

Therefore:

- the real `mining.submit` path exists;
- the pool-response parser exists;
- **actual accepted-share behavior has not yet been empirically observed**.

Never send fake/invalid shares merely to force an acceptance test.

---

## 6A. Solo CKPool payout semantics — confirmed 2026-10-07

QHAP's current endpoint is **solo.ckpool.org**, which is a solo-mining service rather than a proportional shared-reward pool.

Current published behavior:

- solo.ckpool.org advertises a **2% fee**;
- submitted lower-difficulty shares are primarily feedback/accounting telemetry and do **not** accumulate a fractional BTC payout;
- payout occurs only if the miner's work solves a **Bitcoin-network-valid block**;
- Bitcoin's current subsidy is **3.125 BTC per block**, plus transaction fees;
- therefore a current-era winning solo block is greater than 1 BTC even after the advertised 2% fee;
- ordinary hashes, near misses, and pool-share-only results produce **0 BTC payout by themselves** in this solo configuration;
- coinbase outputs require **100 blocks of maturity** before they are spendable.

Important distinction:

```text
pool share != partial Bitcoin payout on solo.ckpool.org
network-valid block = solo payout event
```

This means QHAP's current economics are discontinuous: it does not progressively earn 0.001/0.01/0.1 BTC for increasingly good hashes. A conventional shared pool would have different payout semantics.

Future halvings can eventually reduce the subsidy below 1 BTC, so documentation should state the current subsidy rather than treating ">1 BTC per solved block" as permanent.

---

## 7. Bitcoin target validation

One validated live job used:

```text
nBits: 17021ef0
```

Decoded Bitcoin network target:

```text
000000000000000000021ef00000000000000000000000000000000000000000
```

QHAP has already validated target comparison logic against live headers and known test cases.

The Bitcoin-network target is dramatically harder than the CKPool share target.

An ordinary accepted pool share is **not** proof of finding a Bitcoin block.

---

## 8. SHA256d correctness and optimization history

QHAP intentionally evolved from a slow independent reference implementation into a low-level hardware implementation while repeatedly checking correctness.

Approximate development progression:

| Stage | Approx. measured rate |
|---|---:|
| PSA SHA256d reference | 23.09 kH/s |
| ESP32-S3 hardware SHA baseline | 111.82 kH/s |
| Midstate reuse | 148.60 kH/s |
| Word-level optimization | 165.86 kH/s |
| Compiler / `-O2` stage | ~175 kH/s |
| Low-level `sha_ll` path | 186.39 kH/s |
| Word-level target screening | 209.55 kH/s |
| Direct-register path | 213.07 kH/s |
| Pipeline improvements | 223.95 kH/s |
| Register reuse/raw benchmark | 273.85 kH/s |
| Realistic Bitcoin target screening | 271.22 kH/s |

Some isolated development scans reached higher transient numbers, but the current production documentation should use the validated live range rather than cherry-picking peaks.

### Current production live performance

Validated v0.2.0 production live mining:

**approximately 268–272 kH/s**, typically around **270 kH/s**.

Example observed sequence:

```text
0 - 99999       271.30 kH/s
100000 - 199999 268.37 kH/s
200000 - 299999 269.90 kH/s
...
900000 - 999999 271.62 kH/s
```

These are real SHA256d attempts against live CKPool work.

---

## 9. Cryptographic validation

### Hardware vs PSA

Live block headers were repeatedly checked with both:

- optimized ESP32-S3 hardware SHA path; and
- independent PSA SHA-256 reference path.

Expected validation message:

```text
Hardware vs PSA: MATCH
```

### Bitcoin genesis validation

The Bitcoin genesis test was used as a known reference.

Validated nonce:

```text
2083236893
```

Expected raw double-SHA256 digest:

```text
6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000
```

The network-target comparison accepted the genesis nonce.

### Byte-ordering caution

Bitcoin/Stratum byte ordering was a significant part of development.

Do **not** casually change:

- prevhash transformation;
- header endianness;
- Merkle-root internal/display ordering;
- nonce encoding;
- target comparison ordering;
- high-order screening word.

The optimized screening path was corrected to use the appropriate high-order digest word.

The current `mining.submit` nonce formatting uses numeric `%08x`; preserve it unless a new protocol-level test proves a change is necessary.

---

## 10. Extranonce2 and full nonce-space validation

This is the major v0.2.0 milestone.

QHAP now stores the current `extranonce2` as bytes and regenerates its hex representation for submission.

The scanner safely calculates the final partial chunk rather than allowing `uint32_t` wraparound.

Controlled validation intentionally started near the end of the 32-bit nonce range.

Observed:

```text
Iterations: 100000
Nonce range: 4294817295 - 4294917294
```

Then:

```text
Iterations: 50001
Nonce range: 4294917295 - 4294967295
Final nonce: 4294967295
```

Then:

```text
Nonce space exhausted for current extranonce2
Advancing extranonce2 and rebuilding job
```

QHAP rebuilt the same live Stratum job with:

```text
0000000000000001
```

The rebuilt header again produced:

```text
Hardware vs PSA: MATCH
```

and mining resumed:

```text
Nonce range: 0 - 99999
```

This empirically validates:

- no low-nonce repetition after `UINT32_MAX`;
- correct extranonce2 increment;
- changed coinbase;
- changed Merkle root;
- changed header;
- successful same-job regeneration;
- continued mining from nonce zero on the new search space.

The temporary accelerated exhaustion test hook was removed before the production firmware was finalized.

---

## 11. Production firmware fingerprint

Current validated production binary SHA-256:

```text
2FD674648F9A4E98B4EF188BB524ED072C95AF226DCCEDF57CBC45DC6C87357E
```

The local ignored production binary copy was updated during the v0.2.0 release process.

The tracked fingerprint file and README were updated to the same value.

---

## 12. Important current code concepts

Key state in the production design includes:

- `g_extranonce1`
- `g_extranonce2_size`
- `g_pool_difficulty`
- `g_pool_target_be`
- `g_pool_target_valid`
- `g_pending_header`
- `g_pending_header_valid`
- `g_stratum_sock`
- `g_current_job_id`
- `g_current_ntime`
- `g_current_extranonce2`
- `g_extranonce2_value`
- `g_current_notify_line`
- `g_rebuilding_extranonce2`

Important functions:

- `qhap_sha256d`
- `qhap_build_coinbase_hash`
- `qhap_build_merkle_root`
- `qhap_build_header`
- `qhap_sha256d_hw_80`
- `qhap_live_nonce_scan`
- `qhap_nbits_to_target`
- `qhap_hash_meets_target`
- `qhap_pool_difficulty_to_target`
- `qhap_submit_share`
- `process_stratum_line`
- `qhap_receive_available`

The optimized miner uses:

- SHA-256 midstate reuse;
- low-level `sha_ll` operations;
- direct SHA register reads/writes;
- a 100,000-nonce normal scan chunk;
- top-word target pre-screening before full target comparison.

---

## 13. Known technical boundaries and risks

### Accepted share still unverified

A target-qualified live share has not naturally occurred yet.

### Reconnect path

A detected closed Stratum connection triggers restart logic, but deliberate network-fault testing has not yet been completed.

### Socket receive handling

The current nonblocking receive path treats a negative `recv()` broadly as "nothing available" and does not yet robustly distinguish all recoverable vs fatal socket errors.

### Share send handling

The current small `send()` path expects the whole request in one call. A robust production implementation should handle partial sends and acknowledgement state.

### Stale-job handling

Must be strengthened before larger hashrate is attached.

### Winner persistence

v0.2.0 does not yet persist a network-valid candidate in NVS.

### Dedicated block event

The post-v0.2.0 block-safe milestone now retains the live Bitcoin network target globally, checks pool-qualified candidates against that network target inside the high-speed nonce loop, emits a distinct **BITCOIN NETWORK BLOCK FOUND** event, and prioritizes submission of a network-valid candidate.

This code was build-valid and flashed to the real ESP32. Live CKPool mining remained stable at about 268–270 kH/s and `Hardware vs PSA: MATCH` remained intact.

The actual block-found branch has **not** naturally fired because no Bitcoin-network-valid hash has been found. NVS winner persistence, robust submit retry/ACK handling, stale-job hardening, and deliberate reconnect fault testing remain unfinished.

---

## 14. Probability context

QHAP is a real lottery miner.

Using one validated live target snapshot and approximately 270 kH/s, the README estimated:

- about `1.49 × 10^-11` probability per year;
- about `0.00000000149%` per year;
- roughly **1 chance in 66.9 billion per year**;
- equivalent mean waiting time around **66.9 billion years** if network target and hashrate never changed.

This is a snapshot estimate, not a countdown.

A winning hash could mathematically occur at any attempt, but the expected financial value of the native ESP32 miner is tiny.

---

## 15. Probability-first roadmap

The agreed priority order is:

### Priority 1 — Block-safe reliability

Before multiplying hashrate:

- explicit Bitcoin network-target check in the hot loop;
- separate pool-share and network-block events;
- NVS persistence of a network-valid result;
- preserve job ID, nonce, extranonce2, nTime, header, hash, submit state, and pool response;
- robust `mining.submit` retry/ACK handling;
- better socket failure detection;
- stale-job protection;
- intentional watchdog/reconnect testing;
- boot-time recovery/reporting of any persisted winner.

### Priority 2 — Dedicated Bitcoin ASIC integration

Move SHA256d execution to a dedicated ASIC while retaining QHAP as the controller/validator/submission engine.

Research target discussed: **BM13xx/BM1370-class** hardware or another open documented Bitcoin ASIC platform.

A BM1370-class architecture was discussed because an ESP32-S3 can serve as the controller while the ASIC performs the massive SHA workload.

### Priority 3 — Multi-ASIC controller

Scale to multiple hashing engines with:

- non-overlapping search-space allocation;
- extranonce2 partitioning;
- per-ASIC job/result state;
- validation;
- thermal/power telemetry;
- centralized submission;
- effective hashrate tracking.

### Priority 4 — ESP32/controller optimization

Continue benchmark-driven work such as:

- SHA DMA experiments;
- interleaving/pipeline scheduling;
- job precomputation;
- lower-copy Stratum parsing;
- job-switch latency;
- task/core allocation;
- midstate improvements;
- memory/compiler tuning.

### Priority 5 — Raspberry Pi coordination layer

Use the Pi 5 for:

- node/fleet management;
- persistent telemetry/logs;
- pool health monitoring;
- alerts;
- automatic recovery orchestration;
- thermal/power monitoring;
- dashboards;
- release management.

Do not make the Pi a needless single point of failure.

### Priority 6 — Quantum research

Quantum is a research lane, not the first practical probability improvement.

Topics discussed:

- nonce superposition;
- Grover amplitude amplification;
- reduced-bit mining oracles;
- reversible SHA-family circuits;
- hybrid classical/quantum search-space partitioning;
- circuit depth/noise/runtime scaling;
- reversible-midstate concepts.

Current cloud quantum computers are not expected to outperform modern Bitcoin ASICs for real SHA256d mining.

---

## 16. Superposition / quantum decision

Superposition does **not** allow us to simply test all nonces and read all answers.

The intended research concept is:

```text
nonce superposition
      |
reversible mining oracle
      |
mark valid states
      |
Grover amplitude amplification
      |
measurement
```

For research, QHAP-Q could begin with reduced nonce spaces (for example 8–16 variable bits), compare classical vs quantum search, and measure actual circuit depth, noise, QPU time, and success probability.

This should remain separate from production mining until experimentally useful.

---

## 17. Out-of-the-box expansion ideas

Ideas discussed beyond simple hashrate scaling:

- useful-heat recovery so mining heat replaces energy that would otherwise be purchased for heating;
- distributed QHAP nodes with deterministic non-overlapping work allocation;
- opportunistic acquisition/rental of external SHA256 hashrate when economically rational;
- self-optimizing ASIC voltage/frequency/thermal control;
- evolutionary benchmarking of alternative controller strategies;
- broader "QHAP Hydra" concept combining mining, useful energy/heat, compute/service revenue, and BTC accumulation.

All such ideas must ultimately be judged by measurable improvement in:

```text
expected BTC acquired / money / energy / time
```

and must not be presented as consensus-breaking shortcuts.

---

## 18. 1 BTC/day north-star

The user's stated target is **1 BTC per day**.

The important interpretation is:

- native ESP32 optimization alone cannot reach it;
- the target requires a systems-scale approach;
- solo-mining payouts are stochastic, so "1 BTC/day" should usually be treated as expected acquisition rate rather than a guaranteed daily solo block outcome;
- the roadmap should move through progressively larger real hashrate/economic capability rather than fake difficulty or display metrics.

Conceptual ladder discussed:

```text
~270 kH/s
    ->
1 TH/s class
    ->
10–100 TH/s
    ->
PH/s class
    ->
multi-PH/s
    ->
EH/s-scale systems if economically feasible
```

No step counts unless it increases real valid hashing, protects valid results, or increases legitimate BTC acquisition capacity.

---

## 19. Current v0.3 development resume point

### Current repository state

The first block-safe milestone is now merged into GitHub `main`.

Relevant commits:

```text
d00693b  Add Bitcoin network block candidate detection
58bb197  Merge PR #1 into main
```

The user's local branch at the time of this consolidation is still:

```text
feature/block-safe-reliability
```

and it tracks `origin/feature/block-safe-reliability`.

### Validated block-safe changes now present

The code now contains global Bitcoin network-target state:

```c
static uint8_t g_network_target_be[32] = {0};
static bool g_network_target_valid = false;
```

Live `nBits` parsing stores the target into those globals.

Inside `qhap_live_nonce_scan`, QHAP now tracks:

```c
uint32_t block_candidates = 0;
uint32_t first_block_nonce = 0;
uint8_t first_block_hash[32] = {0};
```

After a hash passes the easier pool target, QHAP also compares it against the Bitcoin network target. If it passes, it records the first network-valid candidate.

A network-valid candidate takes submission priority and emits:

```text
### BITCOIN NETWORK BLOCK FOUND! ###
```

before calling `qhap_submit_share(first_block_nonce)`.

### Build validation

After resolving the ESP-IDF environment mismatch with `idf.py fullclean`, the modified firmware completed:

```text
Project build complete.
```

### Hardware validation

The modified firmware was flashed to the real ESP32-S3 and connected successfully to CKPool.

Observed live job properties included:

```text
Pool difficulty: 10000
nBits: 17021ef0
Network target: 000000000000000000021ef00000000000000000000000000000000000000000
Hardware vs PSA: MATCH
```

Live mining continued normally with sequential 100,000-nonce chunks at approximately:

```text
267.80 kH/s
268.10 kH/s
269.34 kH/s
269.06 kH/s
270.39 kH/s
267.79 kH/s
```

This proves the new network-target/block-candidate logic did not break normal live mining.

### Validation boundary

The dedicated block-found branch has **not** been empirically triggered by a real network-valid hash. That is expected because such a result is extraordinarily rare.

Therefore the current milestone is:

- source-valid;
- build-valid;
- live-hardware-valid for normal mining;
- cryptographically consistent with the existing PSA cross-check;
- **not yet naturally block-event-valid**.

### Strategic decision

Because the user's north-star goal is **1 BTC/day**, further ESP32 reliability polish has diminishing probability value compared with adding dedicated ASIC hashrate.

The agreed direction after this minimum block-safe milestone is to pivot toward **ASIC integration / ~1 TH/s class**, while leaving these reliability items for later hardening before serious scale:

- NVS winner persistence;
- robust partial-send/retry/ACK state;
- stale-job hardening;
- deliberate reconnect/fault testing.

### Workflow correction retained

Use absolute paths such as:

```text
C:\esp_projects\qhap\main\main.c
```

for PowerShell/.NET source edits. Relative .NET paths previously resolved unexpectedly under `C:\Users\Admin`.

---

## 20. Expected next engineering sequence

The immediate probability-first continuation is now:

1. Select the fastest practical dedicated ASIC path for QHAP's first ~1 TH/s-class milestone.
2. Prefer a platform that preserves QHAP's ability to control/validate work rather than turning the project into a black-box miner.
3. Benchmark actual hashrate, power, thermals, and job-switch behavior.
4. Integrate QHAP's block-safe target validation and submission semantics with the ASIC work path.
5. Scale only after one ASIC path is stable and measured.
6. Return to NVS persistence, retry/ACK hardening, stale-job protection, and deliberate fault testing before moving to materially larger fleets.

The current ESP32 firmware can continue mining independently while ASIC research/procurement proceeds.

---

## 20A. ASIC research decision — 2026-10-05

After comparing current open-source BM1370 options, the preferred **first ASIC integration target** is:

```text
Bitaxe Gamma 602
1 × BM1370
~1.1–1.2 TH/s stock class
~17–20 W typical
ESP32-S3 controller
open hardware
USB-C programmable
official BM1370 support in ESP-Miner
```

Reasoning:

- single-ASIC topology minimizes first-integration complexity;
- it is already architecturally close to QHAP: ESP32-S3 controller + Stratum + BM1370;
- official ESP-Miner exposes the BM1370 command/job/result protocol in source;
- Gamma 602 is the newer board revision with DFM/routing improvements over 601;
- the jump from ~270 kH/s to ~1.2 TH/s is roughly 4.4 million× in raw hashing rate;
- once QHAP successfully drives one BM1370, the same controller concepts can be scaled to multi-chip boards.

Do **not** treat Gamma 602 as the endgame. It is the bring-up/development ASIC node.

Preferred next scaling target after successful single-chip integration:

```text
NerdQAxe++
4 × BM1370
~4.8 TH/s stock class
~72–80 W typical
ESP32-S3
open-source hardware/firmware ecosystem
```

A dual-BM1370 Bitaxe GT 801 and larger multi-chip designs remain valid alternatives, but they add multi-chip, power, cooling, and firmware complexity before QHAP has proven direct BM1370 control.

Purchase constraint:

- confirm exact **Gamma 602** board revision;
- include a stable **5 V supply capable of >4 A** (25–30 W class is preferred by the official project);
- active cooling is mandatory;
- preserve/backup stock firmware before flashing experimental QHAP firmware;
- expect seller warranty terms may exclude unsupported firmware modifications or overclocking.

The immediate engineering objective is not merely to run stock AxeOS. It is to use the Gamma 602 as a reference/bring-up platform and progressively integrate QHAP's Stratum, block-safe validation, and submission logic with direct BM1370 ASIC control.

---

## 20B. Frontier research screen and immediate experiment — 2026-10-05

A fresh scan of current mathematics/physics/computing research was used to separate ideas that are scientifically interesting from ideas that can improve QHAP now.

### Findings

- Full 64-round SHA-256 remains unbroken for practical preimage search. Recent practical results concern reduced-round compression functions and do not transfer to Bitcoin's full double-SHA256 mining problem.
- Current quantum mining remains far beyond available cloud hardware; Grover-style mining requires large reversible SHA256d oracles and fault-tolerant logical qubits.
- Reversible/adiabatic computing is a credible long-term energy-efficiency research direction, but requires different hardware and cannot improve the current ESP32 or BM1370 immediately.
- The ESP32-S3 has dual Xtensa LX7 cores and 128-bit PIE SIMD/vector extensions. Those vector instructions are not generally emitted automatically by the compiler and therefore create a concrete optimization research opportunity.
- Current BM1370 firmware ecosystems use version rolling/ASICBoost, but 2026 issue reports show at least some forks parsed pool version masks without correctly applying them to the ASIC. QHAP should implement and verify version-mask handling itself when the BM1370 integration begins.

### Immediate experimental invention: QHAP Heterogeneous Dual-Engine Miner

Hypothesis:

> Run the existing direct-register SHA hardware miner and a completely independent pure-software SHA256d miner concurrently on the other LX7 core, with disjoint search spaces. If aggregate verified hashes/sec rises without stale/duplicate work, then vectorize the software lane using ESP32-S3 PIE SIMD to process multiple nonces in parallel.

Stages:

1. Keep the current hardware SHA mining loop as the baseline (~268–272 kH/s).
2. Add a pure-software SHA256d reference lane pinned to the second core.
3. Partition nonce/header search spaces so hardware and software lanes never duplicate work.
4. Measure aggregate verified hashes/sec and power.
5. If concurrency is positive, implement a 4-way PIE SIMD SHA256d software lane.
6. Cross-check both lanes against the independent reference verifier.
7. Reject the experiment if throughput, stability, or energy efficiency gets worse.

Decision thresholds:

- <3% aggregate gain: discard as production optimization.
- 3–10%: retain as optional research path.
- >10%: integrate into native ESP32 mining.
- Any correctness discrepancy: stop and debug; never trade correctness for displayed hashrate.

This is a measurable probability improvement because any genuine increase in verified hashes/sec produces a proportional increase in block-finding probability.

### Secondary ASIC-era experiment

When BM1370 hardware is available, QHAP should implement a verified BIP310/version-rolling scheduler with pool-mask enforcement and benchmark job-switch latency, valid work rate, stale-work rate, and hashes/joule against stock firmware.

---

## 20C. Heterogeneous dual-engine ESP32 experiment — validated 2026-10-05

Research branch:

```text
feature/dual-engine-mining
commit 1000325
draft PR #2
```

Purpose: test whether unused ESP32-S3 Core 1 CPU capacity can add real SHA256d throughput while the existing direct-register hardware SHA miner continues on Core 0.

Validated results:

- independent pure-software SHA256d implementation matched PSA and hardware SHA bit-for-bit;
- Core 1 scalar software lane measured about 11.39 kH/s;
- caching the first 64-byte SHA-256 midstate raised the Core 1 lane to about 17.06–17.09 kH/s, roughly a 50% improvement in that software lane;
- concurrent hardware lane measured roughly 259–261 kH/s versus the ~269–270 kH/s hardware-only production baseline;
- combined computational throughput therefore measured roughly 276–278 kH/s, approximately 2.5–3% above the hardware-only baseline;
- the watchdog initially fired because IDLE1 was starved; periodic blocking every 1024 software hashes fixed the watchdog issue;
- a 16-word rolling message-schedule compressor was tested, remained cryptographically correct, but slowed the software lane to about 14.64 kH/s, so it was reverted;
- final restored cached-midstate implementation again measured ~17.08 kH/s and `Midstate vs PSA: MATCH`.

Important limitation:

The Core 1 worker currently computes valid SHA256d hashes for benchmarking but does not yet perform pool/network target checks or submit a winning nonce. Therefore the extra ~17 kH/s is not yet production mining hashrate and PR #2 must remain unmerged until that path is made block-safe.

Decision:

- preserve the experiment as a successful research result;
- do not spend more time on ESP32 micro-optimization now;
- pivot probability-improvement effort to BM1370 ASIC integration, where the hashrate jump is millions-fold;
- retain the heterogeneous scheduling idea for later transfer to ASIC/controller architecture.

---

## 20D. BM1370 pre-hardware result-path validation — validated 2026-10-05

Active branch:

```text
feature/bm1370-integration
```

QHAP now contains a BM1370 protocol substrate plus a virtual/mock result source that runs on the actual ESP32-S3.

Validated on the ESP32-S3:

- `qhap_bm1370_protocol_self_test() == PASS`;
- `qhap_bm1370_mock_self_test() == PASS`;
- a synthetic BM1370 result packet is decoded into nonce/job/core/version fields;
- a real live CKPool job is constructed normally by QHAP;
- the mock BM1370 returns nonce 0;
- QHAP reconstructs the exact 80-byte header from the decoded result;
- independent SHA256d of that reconstructed header matches the already-validated live header hash;
- this produced `BM1370 mock live-header hash: MATCH` on multiple different live CKPool jobs;
- the native ESP32 miner remained active and measured about **270.23 kH/s** during the same firmware run.

Important interpretation:

- the ESP32 is still performing **real Bitcoin mining** against live CKPool work;
- the BM1370 mock is only a controller/decoder verification path and contributes **no additional mining hashrate**;
- this validates the controller-side pre-hardware chain, not physical BM1370 compatibility;
- real UART timing, chip initialization, exact job encoding, returned checksum semantics, real nonce responses and silicon behavior remain pending until the Gamma 602 arrives.

Historical protocol gap at the 2026-10-05 milestone (resolved in PR #3 on 2026-10-11):

- the 11-byte BM1370 result decoder originally parsed the status/checksum byte without validating CRC5;
- this was remedied by CRC5 residue validation and corrupt-packet rejection in the 2026-10-11 integration-branch update; validation against actual received BM1370 hardware frames is still pending.

The next meaningful controller milestone is an independent ASIC-candidate gate:

```text
ASIC result
  -> job lookup
  -> stale check
  -> duplicate check
  -> exact header reconstruction
  -> independent SHA256d
  -> pool target check
  -> Bitcoin network target check
  -> submit / block-safe priority path
```

---

## 20E. QHAP Evolution Engine concept — recorded 2026-10-05

New architecture direction: QHAP should eventually include an **autonomous experiment/evolution layer** that searches for measurable improvements instead of relying only on manual one-variant-at-a-time tuning.

The Bitcoin consensus algorithm remains SHA256d. The engine must never mutate the mining formula into a non-consensus algorithm. It may instead optimize how valid work is executed, scheduled and controlled.

Proposed supervisory architecture:

```text
Raspberry Pi 5
QHAP Evolution / experiment controller
        |
        +-- generate candidate strategies
        +-- schedule controlled experiments
        +-- collect telemetry/results
        +-- statistical comparison
        +-- retain champion / reject regressions
        |
        v
ESP32-S3 controller(s)
        |
        +-- native SHA reference/research lane
        +-- BM1370 / later ASIC workers
```

Candidate variables include:

- compiler/build options and safe code-generation variants;
- SHA scheduling/midstate/controller strategies;
- nonce/extranonce/version work partitioning;
- job chunk sizes and job-switch behavior;
- task/core allocation;
- memory layout and copy reduction;
- UART batching/transport strategy;
- per-ASIC frequency, voltage, thermal and fan-control settings inside hard safety limits;
- per-chip work scheduling and heterogeneous fleet policies.

Candidate optimization methods to investigate include evolutionary search, genetic algorithms, Bayesian optimization, multi-armed bandits, simulated annealing and champion/challenger testing.

Do not optimize only for displayed hashrate. Candidate score/selection should consider measurable production value such as:

- useful verified hashes/s;
- hashes/joule;
- stale-work fraction;
- duplicate-work fraction;
- hardware/serial error rate;
- temperature and thermal stability;
- job-switch latency;
- uptime/recovery behavior.

Every automatically generated strategy must remain behind a deterministic correctness wall before it can become production-eligible:

```text
known SHA256d vectors
  -> genesis/reference tests
  -> PSA/reference comparison
  -> live-header reconstruction comparison
  -> target-check verification
  -> short benchmark
  -> long stability benchmark
  -> production eligibility
```

For ASIC tuning, hard electrical/thermal envelopes must be externally enforced so an optimizer cannot arbitrarily raise voltage/frequency or trade hardware safety for a temporary benchmark win.

Current hardware is already sufficient for a first evolution-engine prototype:

- Raspberry Pi 5 8 GB as the experiment coordinator/data store;
- existing ESP32-S3 as controller/reference hardware;
- incoming Bitaxe Gamma 602 / BM1370 as the first ASIC-under-test.

Additional ESP32-S3 boards may later be useful for parallel experiment lanes, but no extra controller purchase is required now.

This remains a **planned research architecture**, not a validated performance advantage. QHAP should call it innovative only after controlled experiments demonstrate reproducible improvement over a fixed baseline.

---

## 20F. BM1370 CRC5 result validation and wall-clock mining benchmark — validated 2026-10-11

Integration milestone: [PR #3](https://github.com/Vermicare/QHAP/pull/3) merged into `feature/bm1370-integration` from `feature/bm1370-crc5-validation-20261011`.

Commits:

- `7173b2c` — Validate incoming BM1370 result CRC5 and improve mock packet tests.
- `f8d5d48` — Add 30-second ESP32-S3 wall-clock mining benchmark.
- `4aa8268` — Merge PR #3 into the BM1370 integration branch.

Observed evidence from the Windows ESP-IDF v6.1 board B workflow:

- `idf.py -B build_board_b build`: `Project build complete`.
- ESP32-S3 serial output: `BM1370 protocol self-test: PASS`, `BM1370 mock self-test: PASS`.
- Live mining: `STRATUM TCP CONNECTED`, `Hardware vs PSA: MATCH`, `BM1370 mock live-header hash: MATCH`, sequential nonce scanning resumed.
- Synthetic result test updated to a CRC5-valid sample ending in `0x8C` (the older `0x9B` sample had an invalid remainder).
- Decoder now rejects responses whose CRC5 remainder over the 9 bytes following the `AA 55` preamble is nonzero.
- Mock generator produces valid CRC5 samples; the self-test also confirms rejection of a deliberately corrupted nonce byte.
- Sample wall-clock windows each reported 7,700,000 completed nonce attempts over approximately 30.15–30.34 seconds, about 254–255 kH/s.
- Separate hardware inner-loop measurements during subsequent live scans were about 266–267 kH/s; these are not the same measurement window as the wall-clock benchmark and should not be added to software-lane throughput.

Limitations and future work:

- Physical BM1370 UART communication, chip detection, actual mining work dispatch/receipt, nonzero rolled-version reconstruction, job mapping and stale/duplicate checking, and accepted pool-share behavior are not yet demonstrated.
- The prior Core 1 software hashing experiment does not have a production target-and-submit path; do not count its `~17 kH/s` as productive solo-mining hashrate.
- The two checked-in source files were exercised on the ESP32-S3; no GitHub Actions runs were found for the PR, so no automated CI validation is claimed.
- Local `build_board_b/` and `*.bak` artifacts remained untracked on the Windows PC and were not included in PR #3.
- The `main` branch was not modified by PR #3.

Next: implement a block-safe ASIC candidate verification and job-mapping layer behind the existing protocol decoder, then connect to real BM1370 hardware only after communication and electrical checks.

---

## 20G. Two-ESP32 + Raspberry Pi pre-ASIC lab — prepared 2026-10-11

The user confirmed that the Bitaxe Gamma 602/BM1370 has **not** yet arrived, but there are **two ESP32 boards** and a **Raspberry Pi 5 (8 GB)** available.

GitHub [PR #5](https://github.com/Vermicare/QHAP/pull/5) merged into `feature/bm1370-integration` (merge commit `cdd1fe9`), adding:

- [Pre-ASIC experiment plan](research/PRE_ASIC_LAB_PLAN.md) assigning current Board B as untouched live-mining reference, Board A as challenger, and Pi 5 as research supervisor;
- `tools/qhap_lab.py` — a read-only, standard-library Python analyzer of ESP32 serial output that distinguishes real wall-clock throughput from inner-loop rates and explicitly avoids automatic deployment;
- `tests/test_qhap_lab.py` and GitHub Actions tests including incomplete/bad measurements, SHA mismatch refusal, and minimum-window screening requirements.

GitHub Actions **completed successfully** for this host-side software, but **nothing has run on the Raspberry Pi or second ESP32 yet**; Board A's exact model/configuration still needs to be verified when connected.

Immediate next hardware interaction: confirm Pi OS and secure local SSH availability; then collect Board B reference logs and identify Board A. Do not interrupt Board B mining to conduct fault injection; use a local mock Stratum on Pi only for Board A tests. The Pi optimizer must not mutate SHA256d consensus, fabricate pool shares, claim energy efficiency without power measurements, or flash production boards on its own.

---

## 20H. Confirmed three identical ESP32 boards and Pico H — user-confirmed 2026-10-11

**Final hardware clarification supersedes ALL earlier inventory guesses:** user explicitly confirms **three (3) identical Espressif ESP32-S3-DevKitC-1-N8R8 development boards**. The earlier product screenshots were of the same *model*, not evidence that only one or two such boards were owned. Separately, user owns **one Raspberry Pi 5 (8 GB)** and **one Raspberry Pi Pico H (RP2040)**. **Total five available boards**, while the **Bitaxe Gamma 602 / BM1370 ASIC** is still awaiting delivery.

Unit-specific serial numbers, exact physical board identity and flashed firmware have not been checked by USB yet. The quantity/model inventory is user-confirmed, but exact which board is the live miner must be established before device operations.

Safe experimental assignments:

1. **ESP32-S3 N8R8 #1 — stable reference miner:** the board currently running verified real Bitcoin jobs. Do not reflash or connect to mock UART in the initial research phase.
2. **ESP32-S3 N8R8 #2 — matched experimental miner/challenger:** same model as reference; evaluate verified SHA256d computations, schedule/nonce partitioning, compiler effects and job switching, with reproducible tests.
3. **ESP32-S3 N8R8 #3 — isolated ASIC-controller test bench:** receive deliberately generated BM1370-format frames over 3.3 V UART from the Pico H and test job mapping, checksum, stale/duplicate handling and recovery. Never send synthetic results to a public pool.
4. **Raspberry Pi 5, 8 GB — Linux experiment supervisor:** log ingestion, reproducible trials, controlled mock Stratum server and future Evolution Engine. Installed OS and SSH connectivity not yet confirmed.
5. **Raspberry Pi Pico H, RP2040 — test stimulus generator:** MicroPython UART/Programmable I/O; emulates BM1370 **result frames only**, not actual hashing or real ASIC electrical behavior. Pico H does not run Raspberry Pi OS.

Updated documentation: `research/PRE_ASIC_LAB_PLAN.md`. Optional `tools/pico_bm1370_emulator.py` remains strictly isolated, synthetic-only. For future physical Pico-to-ESP32 tests, separately USB power the two devices and use a shared GND, confirmed 3.3 V RX GPIO and no tied power rails; never wire the production miner.

**Raspberry Pi setup status (user-confirmed 2026-10-11):** Neither Raspberry Pi-branded board has been touched or configured. The Raspberry Pi 5 (8 GB) has not yet been set up with Raspberry Pi OS, and the Pico H (RP2040) has not been programmed. **Next practical step:** on the Windows laptop install the official Raspberry Pi Imager, then prepare a dedicated microSD for the Pi 5 with headless/network settings (follow the on-screen setup; do not overwrite a drive with useful data). Boot the Pi 5 and validate access before programming the Pico H. Preserve the working ESP32 miner; label all three N8R8 units by serial port / current firmware before deploying experimental builds.

---

## 20I. Raspberry Pi 5 microSD filesystem recovery — 2026-10-13

**Status: unresolved; hardware failure not proven.** Supersedes the Raspberry Pi 5 'not yet configured' setup status in 20H. A dedicated Raspberry Pi OS microSD was prepared; a separate laptop running live Ubuntu was used to investigate extensive EXT4 `rootfs` block bitmap/checksum and free-block accounting inconsistencies. Earlier `e2fsck -f -n` exited 4; automatic and earlier interactive checks did not establish a clean result. The latest `e2fsck -f` was canceled at an interactive `Fix<y>?` question because the proposed bitmap changes were extensive. The `dumpe2fs` superblock 'clean' flag does not negate the errors. Filtered kernel logs did not show explicit storage I/O errors or USB resets, but card/reader health and the original cause remain unknown.

A read-only `ro,noload` mount made `/home/qhap/qhap` accessible (~576 KiB; `/home/qhap` ~644 KiB). The user's complete home directory, including hidden files and Git metadata, was encrypted into `qhap-home-backup-2026-10-13.tar.gz.gpg` (~192 KiB). Local GPG decryption and compressed archive listing printed `BACKUP VERIFIED`; a Google Drive browser screenshot showed the uploaded file in My Drive. **The backup is only the home directory, not a complete Raspberry Pi OS image.** Remote download/hash verification is still outstanding. The microSD EXT4 and boot partitions were confirmed unmounted at the last check. No subsequent filesystem repair, OS reinstall, or successful Pi boot has been confirmed.

**Recovery requirements:** protect any unique files under `/etc`, `/var`, `/boot/firmware` or other locations before accepting large repairs; re-identify the target block device before every risky operation; avoid the laptop's BitLocker SSD and live Ubuntu installer; obtain informed consent before repair/reflash; never run `fsck -y` or destructive writes reflexively. A full-card image requires roughly 128 GB on separate storage if needed. Keep production ESP32 miners isolated.

**Detailed incident, decision gates, verification and prevention:** [Raspberry Pi 5 storage recovery runbook](research/RPI5_STORAGE_RECOVERY.md). This recovery incident is a project operations milestone; do not silently treat it as a validated repair.

---

## 21. Workflow rules for future QHAP sessions

These are important because previous sessions became inefficient when verification loops repeated.

- Work **one action at a time** when guiding manually.
- Give one exact command/edit, then wait for the user's output.
- Do not start repetitive Git status/stage/commit/push loops.
- Do not claim a modification succeeded merely because a final `Write-Host` ran.
- Verify the actual file/output after every important scripted edit.
- Prefer absolute Windows paths for PowerShell/.NET edits.
- Preserve UTF-8 without BOM when rewriting source/docs.
- Do not change validated Bitcoin byte ordering casually.
- Do not fabricate accepted shares, profitability, or quantum speedups.
- Do not use fake invalid shares to test CKPool acceptance.
- Keep secrets out of Git and out of this memory document.
- When a milestone is validated, update this document as part of the milestone.
- The user prefers the assistant/Codex to do as much engineering work autonomously as possible; when manual interaction is necessary, minimize it.

---

## 22. Documentation roles

Use the repository documents as follows:

- **README.md** — public project overview, current validated capabilities, build/use instructions.
- **ROADMAP.md** — probability-first future architecture and priorities.
- **PROJECT_MEMORY.md** — detailed persistent engineering context, resume point, validated historical milestones, and workflow constraints.

If these disagree:

1. source code + measured output wins for implementation facts;
2. the newest validated test wins for measured performance;
3. this file should then be updated to remove the discrepancy.

---

## 23. Definition of a successful future milestone

A QHAP claim should move from "planned" to "validated" only when appropriate evidence exists.

Examples:

- **Build-valid:** `idf.py build` succeeds.
- **Hardware-valid:** flashed ESP32 produces expected behavior.
- **Crypto-valid:** optimized result matches independent reference.
- **Pool-valid:** real CKPool response confirms behavior.
- **Block-safe-valid:** network-target candidate path is independently exercised with a controlled test without corrupting production semantics.
- **Release-valid:** production test hook removed, clean build flashed, normal live mining reverified, fingerprint updated, docs updated, tag created.

---

**QHAP principle:** preserve correctness first, protect the winner second, multiply real work third, and treat every claimed improvement as something to measure rather than assume.


---

## 24. Raspberry Pi 5 + Gamma 601 read-only telemetry — validated 2026-10-24

The previous Gamma 602 purchase plan was superseded by the **physically received Bitaxe Gamma 601 / BM1370**, running its own stock AxeOS firmware. Do not conflate stock ASIC mining with QHAP hardware-controlled ASIC hashing; no direct QHAP-to-BM1370 UART work dispatch has occurred.

Observed hardware and connectivity:

- Raspberry Pi 5 (8GB) fitted with the purchased Raspberry Pi Active Cooler SC1148 and acrylic case. Fan spin test passed. Observed Pi CPU temperatures ~36.7°C at low load, ~39.7–40.25°C during telemetry; `vcgencmd get_throttled` reported `0x0` at the checked time.
- Pi recovered and boots Raspberry Pi OS, mounted EXT4 root read/write, with QHAP repo at `/home/qhap/qhap`. Startup kernel logs showed no explicit current I/O error. **Prior extensive EXT4 structural inconsistencies have not been cleared by an offline fsck; do not treat boot success as filesystem validation or enable unattended/durable storage writes yet.**
- Production QHAP ESP32-S3 still mines independently. Raspberry Pi reads its USB serial output via `/dev/ttyACM0` at 115200 baud without writing commands or reflashing it.
- Pi connected to the PG Wi-Fi, able to access AxeOS `GET /api/system/info` over the LAN (HTTP 200) and Raspberry Pi Connect. Previous OnePlus-hotspot routing was isolated from the Bitaxe's LAN.
- Existing `tools/qhap_lab.py` parsed a 12-second RAM-only serial sample: 31 observed inner-loop scan windows, **median 267.93 kH/s**, no malformed windows or observed SHA mismatches. No dedicated wall-clock benchmark or SHA reference match appeared in this short sample; these remain **not observed**, not validation failures.
- New committed `tools/qhap_snapshot.py` and `tests/test_qhap_snapshot.py` were run on the real Pi as part of 19 passing host tests. Combined snapshot read live Bitaxe telemetry (~1.291 TH/s, ASIC 68.5°C, VR 78°C, 4300 accepted/0 rejected shares) and a **saved** ESP32 log (~267.93 kH/s median inner scan), with a Pi CPU reading ~39.15°C. The sources were explicitly labeled live vs historical.
- New committed `tools/qhap_live_probe.py` and `tests/test_qhap_live_probe.py` were run on the Pi as part of **22/22 passing host tests**. A bounded 3-sample **real hardware** test collected Bitaxe HTTP telemetry, ESP32 USB serial and Pi CPU thermal data in the same session, and wrote credential-free JSONL only to `/dev/shm/qhap_live_probe.jsonl`.
- Observed Bitaxe readings across three samples: ~1.294 / 1.299 / 1.337 TH/s; accepted shares 4440→4441; 0 rejected shares; ASIC ~68.75–68.875°C; VR ~79°C; fan 100%; power ~24.64 W. ESP32 serial had 0 completed scan windows immediately on probe start, then 13 and 25; median inner-scan rates ~268.08 and 268.28 kH/s. Pi CPU ~39.7–40.25°C. No SHA mismatch messages; `weighted_wall_khs` stayed null because no wall-clock benchmark blocks appeared.

Validation boundary: **simultaneous read-only telemetry is hardware-proven**; it is NOT ASIC control, an improved hashrate, an ESP32 wall-clock benchmark, a validated cross-device nonce allocator, a 24/7 service, or proof of accepted shares from the ESP32. The Bitaxe fan is at 100%; the ~79°C regulator and ~69°C ASIC readings merit thermal monitoring.

Next: keep both production miners unchanged. Confirm Pico H USB identity independently on the Pi, then prepare its isolated BM1370-result stimulus path on a non-production ESP32. Finish offline SD-card filesystem integrity checks, storage/backup health gates, and the ESP32 wall-clock baseline before scheduling unattended Evolution Engine trials.


---

## 25. Standing strategic directive — QHAP chess mindset (2026-10-25)

**User's explicit standing instruction:** "Be very careful and strategically with chess mindset in this project — rethink whatever we are doing is [appropriate]."

The authoritative operating policy is [QHAP Strategic Engineering Playbook](docs/QHAP_STRATEGIC_PLAYBOOK.md). Apply this in every future QHAP session and to all engineering agents and tools.

Mandatory interpretation: **think 2–3 moves ahead, compare alternatives, question whether a step advances the real objective, protect currently productive miners, detect hidden assumptions, require verifiable pass/fail gates, plan recovery before risk, and avoid premature hardware/firmware changes**. A command working or a new board becoming visible is not by itself strategic progress. Distinguish measurement/telemetry from new hashing capacity and real physical BM1370 control.

Current hardware checkpoint from Raspberry Pi terminal, 2026-10-25:
- `lsusb -d 2e8a:0003`: Raspberry Pi RP2 Boot, ID `2e8a:0003`.
- Read-only `RPI-RP2` mount at `/run/qhap-pico`; `INFO_UF2.TXT`: `UF2 Bootloader v3.0`, `Model: Raspberry Pi RP2`, `Board-ID: RPI-RP2`.
- This validates Pico H's RP2040-family BOOTSEL identity **only**. MicroPython has **not** been flashed and electrical Pico-to-ESP32 UART has **not** been demonstrated.
- Reference miner at `/dev/ttyACM0` remains separate. USB device numbering may change; re-identify before interacting with any port.

Immediate execution guardrail: no need to flash the Pico merely because BOOTSEL enumeration passed. First ensure official RP2040 Pico firmware provenance, installation/recovery steps, isolation of production miners, the exact experimental proof sought, and remaining Pi storage health risks. Continue only with one verified step at a time. The existing offline EXT4 integrity/backup gate remains unresolved for unattended operations.


---

## 26. Second standing directive — breakthrough-first cross-disciplinary mining R&D (2026-10-25)

**User's instruction:** "Also we are going for the big win — not for small improvements — we must think in every scientific fields to improve our mining."

This is a **permanent QHAP objective and method**, complementary to Section 25's chess-mindset rule. QHAP must search broadly for **orders-of-magnitude scientific/system breakthroughs**, including unexpected cross-disciplinary combinations, instead of making 2–5% ESP32/peripheral tweaks the primary end goal. Preserve disciplined experiments, correctness and recovery protections, and the independence of the working ESP32-S3 and stock Bitaxe Gamma 601. Small reliability/performance work is justified when necessary to protect a winner, prove a baseline or enable a major high-upside study.

**Authoritative documentation:**
- [Breakthrough Research Program](research/BREAKTHROUGH_RESEARCH_PROGRAM.md) — scientific map, mechanism/consensus/falsification gates, portfolio strategy and genuine improvement metrics.
- [Breakthrough Candidate Register](research/BREAKTHROUGH_CANDIDATES.md) — initial ten testable candidates, including ASIC scaling, novel SHA datapaths, chip design, cryptanalysis, photonics, reversible/quantum computing, AI design-space search and materials/electrical/thermal systems.
- [Strategic Engineering Playbook](docs/QHAP_STRATEGIC_PLAYBOOK.md) — now includes this standing moonshot directive and requirement to think several moves ahead.
- [Roadmap](ROADMAP.md) — includes a parallel cross-disciplinary 10x+ moonshot program, without removing safety/Bitcoin-validity priorities.

**Critical evidence boundary:** No new breakthrough in full-strength double-SHA256 mining has been demonstrated. A 10x/100x+ scientific advantage is an aspiration; purchasing more independent ASIC hashrate is a potentially practical path to order-of-magnitude *total* capability but not automatically a novel algorithm. A different PoW, toy quantum result or simulated hash cannot mine current Bitcoin unless it computes the exact consensus proof. Any claim of an advantage requires independent full-strength correctness and reproducible cost/power/effective throughput evidence. AI must not be claimed to predict winning nonces.

**Current next move:** keep real miners running and both moonshot literature/cheap-simulation research and the isolated Pico/ESP32 protocol lane moving deliberately; finish outstanding Raspberry Pi offline EXT4 health validation before unattended experiments. This instruction and its linked files are required context when resuming QHAP from any future chat.


---

## 27. Pico H MicroPython boot and USB enumeration — hardware observed

**2026-10-25 observation after preparing the official Pico/RP2040 UF2 firmware:**

- Pico H initially in RP2040 BOOTSEL: `2e8a:0003`, `RPI-RP2` (`/dev/sda1`) bootloader drive; read-only inspected at `/run/qhap-pico`.
- `/dev/shm/qhap_pico_micropython.uf2` recognized by the Pi `file` command as an RP2040-family UF2 containing 1330 blocks. Its SHA-256 was printed locally; no independent published-checksum confirmation established.
- UF2 copied to the positively identified bootloader mount; subsequent `lsusb` displayed `2e8a:0005 MicroPython Board in FS mode` and `/dev/ttyACM1`, with a stable MicroPython `/dev/serial/by-id/usb-MicroPython_Board_in_FS_mode_*` symlink. BOOTSEL mass-storage device no longer appeared.
- Reference production ESP32 remained `303a:1001 Espressif USB JTAG/serial debug unit` at `/dev/ttyACM0`. Do not rely on numerical tty names in future; select Pico via stable /dev/serial/by-id path.

**Validation boundary:** UF2 application and MicroPython USB enumeration are **hardware-observed**, but MicroPython REPL identity/command-response, Pico GPIO UART operation, Pico↔challenger ESP32 wiring, and full BM1370 packet emulation are **not yet validated**. Next safe step: one harmless Pico-only REPL calculation, no writes to reference miner or Bitaxe. Pi's offline EXT4 filesystem-integrity gate remains open for unattended operations.


---

## 28. Pico H MicroPython REPL — observed PASS

At the Raspberry Pi 5 remote shell, the newly flashed Pico H's unique MicroPython `/dev/serial/by-id/usb-MicroPython_Board_in_FS_mode_*` serial symlink resolved to `/dev/ttyACM1` (reference mining ESP32 remained on `/dev/ttyACM0`). A bounded, no-file-write Python host script opened the identified Pico serial port, entered raw host terminal mode, sent the harmless REPL expression `print(6 * 7, "QHAP_PICO_OK")`, and captured its response.

**Pico's observed response:**

```text
MicroPython v1.29.0 on 2026-08-24; Raspberry Pi Pico with RP2040
>>> print(6 * 7, "QHAP_PICO_OK")
42 QHAP_PICO_OK
>>> 
REPL PASS
```

**Validation:** Pico H MicroPython interpreter **physically executed an isolated command** and host-to-Pico USB serial communication is working. This does **not** validate Pico UART GP0 electrical output, emulator code execution on the Pico, synthetic CRC5 frames arriving at any ESP32, physical BM1370 control, or a Bitcoin hashrate improvement.

**Safe next milestone:** Inspect availability of `mpremote` on the Pi without installing tooling blindly. Existing `tools/pico_bm1370_emulator.py` is MicroPython-compatible, not self-starting, and has 6 existing CPython-level tests; it must be transferred to the Pico and independently validated with generated CRC5 vector **without connecting GPIO**, then tested over 3.3 V UART with a positively identified *non-production* ESP32 only. Original miners remain untouched. Raspberry Pi offline EXT4 integrity gate is still unresolved before unattended experiments.


---

## 29. Pico H on-device BM1370 synthetic CRC5 vector — PASS

**Hardware-observed** on the flashed Raspberry Pi Pico H / RP2040, MicroPython v1.29.0, accessed through its unique USB `/dev/serial/by-id/usb-MicroPython_Board_in_FS_mode_*` symlink using temporary RAM-only `mpremote 1.29.0` on the Raspberry Pi 5. The user ran the repository's existing `tools/pico_bm1370_emulator.py` extended with an ephemeral MicroPython on-device vector test through `mpremote ... run`; the test file was prepared under `/dev/shm` and was not saved to Pico flash. The emulator did not auto-start UART output.

Observed output:

```text
Pico mock loaded but UART is NOT started automatically.
On a Pico, import this module and call start_uart_test() explicitly.
QHAP_PICO_CRC5_PASS aa55a1b2c3d4025a12348c
```

The on-device MicroPython code constructed a deterministic 11-byte BM1370-format synthetic result, matched the repository's independent golden vector, verified CRC5 residue zero, and verified that flipping a nonce bit without recomputing CRC5 produced a nonzero residue. **Valid:** Pico execution and mock packet/checksum generation. **Not validated:** physical Pico GP0 UART output, frame reception/CRC gating on a second ESP32, actual BM1370 ASIC communication, QHAP job dispatch to Bitaxe, Bitcoin proof-of-work acceleration. Do not equate this with more mining hashrate.

**Next chess-mindset move:** positively identify and inventory a physically separate non-production ESP32-S3 board before any Pico→ESP32 UART wiring or test firmware. Preserve the known-good ESP32 reference miner on its own USB device and Bitaxe Gamma 601 running stock AxeOS. Before physical UART, validate 3.3 V logic, common GND, GPIO pinout, and the independent receiver's refusal to publish synthetic shares to CKPool. Pi's offline EXT4 structural check remains unresolved, so no unattended storage-writing campaigns.


---

## 30. Experimental ESP32-S3 identification on Windows (2026-10-25)

A physically separate new ESP32-S3 board for the *isolated protocol-lab receiver* was connected to the Windows laptop via its CP210x USB-to-UART connector, not to the reference QHAP miner. Windows enumerated `Silicon Labs CP210x USB to UART Bridge (COM7)` (`USB VID_10C4/PID_EA60`); its COM7 presence was observed only when connected. With ESP-IDF v6.1 activated, `python -m esptool version` reported **esptool 5.4.0**, and the read-only `python -m esptool --port COM7 --baud 115200 flash-id` completed successfully:

- ESP32-S3 QFN56, chip revision **v0.2**, dual core up to 240 MHz, 40 MHz crystal.
- Flash memory **8 MB**, quad data lines, 3.3 V eFuse flash voltage; embedded **8 MB PSRAM** reported by esptool.
- The new board's hardware MAC **ends in :58**; the separately attached ESP32 on the Pi previously appeared under USB identity ending `...:34`. This differentiates the observed devices, but full physical board-role mapping still requires confirmation. Avoid publishing complete hardware MACs unnecessarily.
- After the diagnostic the board reset via RTS. **No flash write/erase, firmware transfer or GPIO wiring was performed.** USB data on the Windows laptop was verified; the same new board had not enumerated on the Raspberry Pi black or blue USB ports during previous tests. Reconnecting to Pi may appear as `/dev/ttyUSB*` through CP210x, not necessarily `/dev/ttyACM*`; the Pi-side USB issue is unresolved.
- Windows `C:\\esp_projects\\qhap` previously showed local branch `feature/bm1370-crc5-validation-20261011` with untracked `build_board_b/` and two timestamped `.bak` files. **Preserve these files and the production branch.** Pi/GitHub protocol-monitoring work is on `feature/bm1370-integration`. Do not change production/miner code or delete untracked files as an incidental step.

**Next chess-mindset gate:** Preserve any factory/example firmware (optional preflash backup) and create a separate experimental build/worktree on Windows. Only flash this positively identified new board after reviewing its experimental image and verifying target, flash/PSRAM configuration, no real-pool synthetic submissions, and rollback. Keep original mining ESP32 and stock Bitaxe running. After isolated USB receiver validation, test Pico→receiver on verified 3.3 V UART, then independently revisit Pi USB enumeration. The Pi's previous EXT4 structural-integrity concerns still block unattended disk writes/experiment services.


---

## 31. Experimental ESP32 flash backup diagnostic — NVS variation identified

**2026-10-25, Windows esptool v5.4.0, separate ESP32-S3 QFN56 rev v0.2 at COM7 / CP210x.** An attempt to read the full 8 MB flash at 460800 baud failed at ~18.1% with `Corrupt data, expected 0x1000 bytes but received 0xfff bytes`. The output is incomplete and not a usable recovery image. Two subsequent reads of offset `0x0`, size `0x10000` (64 KiB) at 115200 baud completed and produced different SHA-256 hashes despite equal sizes:

- First `esp32s3_probe_64kb.bin`: `B90E0CC887C60B961FD49406F20EDEE1304A1562CE18EDFB630C7CC91B7D2C90`.
- Second `esp32s3_probe_repeat.bin`: `8F6BB8691A691A702931000A57D8B93296788973D7C9658CF7C53E209E3F2689`.
- Bytewise comparison: `0x0B000` sector 1 different byte; `0x0D000` sector 1018 different bytes; other 14 4KiB sectors in this sample identical (1019 total byte differences).
- The binary partition table extracted from the original backup slice `0x8000–0x8FFF` was successfully decoded by the installed ESP-IDF `gen_esp32part.py`: `nvs` data partition `0x9000` size 20 KiB (covers `0x9000–0xDFFF`), `otadata` `0xE000` size 8 KiB, `app0` `0x10000` size 1280 KiB, `app1` `0x150000` size 1280 KiB, `spiffs` `0x290000` size 1408 KiB, and `coredump` `0x3F0000` size 64 KiB.

**Interpretation:** Both changed sectors are entirely within NVS, consistent with firmware-updated persistent configuration across resets; this is not yet a proven cause and does not demonstrate flash failure. A passing smaller 115200-baud read does not prove an 8MB transfer is reliable. **Do not erase/flash experimental firmware or declare a valid complete backup yet.**

**Next low-risk diagnostic:** compare two 64 KiB reads from static application partition address `0x10000` using 115200 baud and SHA-256. If identical, attempt a full original-flash backup at low baud while keeping a separate hash, labeling NVS as time-varying rather than requiring two whole-image digests to be identical across resets. Protect full flash images as potentially containing Wi-Fi/pool credentials. Preserve the working ESP32 miner and stock Bitaxe completely.


---

## 32. Experimental ESP32 stable application flash-region reread — PASS

**2026-10-25, Windows laptop, experimental ESP32-S3 on CP210x COM7.** Following the first 64 KiB flash read mismatch restricted to NVS sectors 0xB000 and 0xD000 (documented in section 31), the user read **two separate 64 KiB samples of the application's static flash region beginning at 0x10000**, both via `esptool v5.4.0 --chip esp32s3 --port COM7 --baud 115200 read-flash 0x10000 0x10000`. Files `esp32s3_app_read_A.bin` and `esp32s3_app_read_B.bin` were checked for full 65,536-byte length and equal SHA-256 digests in PowerShell. Observed terminal result: **`QHAP STATIC FLASH READ PASS`**.

**Interpretation:** Byte-for-byte matching application-sector data supports serial read repeatability at 115200 baud for the sampled static section. Does **not** establish reliability of a full 8 MiB read, validate all flash regions, prove NVS caused the earlier differing samples, or constitute a tested restore image. Keep prior failed full-read output and both probes as diagnostic evidence.

**Next gate:** Read the full 8 MiB into a **new private backup file** on Windows at 115200 baud without overwriting previous evidence; only if esptool succeeds and the output is exactly 8,388,608 bytes, compute SHA-256. Then compare the saved image's static application slice against the verified 64 KiB sample; separately recognize live NVS may change across resets. No erasing/flashing/production connection changes until a trustworthy recovery path is available. Never commit or share the binary image; it can contain secrets.


---

## 33. Full 8 MiB experimental flash read reached 100% but failed digest — unverified

**2026-10-25 — Windows esptool 5.4.0, separate experimental ESP32-S3 v0.2, CP210x COM7.** Following matching 64 KiB static application flash samples at 115200 baud (section 32), a full flash dump was attempted, with intended private local destination `C:\\esp_projects\\qhap_lab_backups\\esp32s3_full_8mb_115200_20261025.bin` and command:

```powershell
python -m esptool --chip esp32s3 --port COM7 --baud 115200 read-flash 0 0x800000 $backup
```

Reported progress `100.0% 8.00MB/8.00MB [12:38]`, after which esptool **failed** with `Digest mismatch: expected A6B4FE3EB233DFC10796D8ACADA3D85F, got B2C251BB856B5B31AA8041342D58AAF5`. The subsequent PowerShell status check printed `QHAP FULL BACKUP NOT VERIFIED`. The stub's read_flash protocol sends an MD5 at the end and the host verifies it. **Do not consider this a valid backup**, notwithstanding progress reaching 100%; no flash write or erase occurred. Prior 460800-baud full attempt failed at 18.1% with a short packet; 115200-baud static-region reads matched. NVS varying between separate reset/read sessions is plausible but **not proven to explain this in-transfer MD5 mismatch while the app was suspended in downloader mode**.

**Immediate next non-destructive action:** Inspect existence, size, local MD5 and SHA-256 of the above candidate dump; compare bytes at offset `0x10000` and length `0x10000` with prior static application sample `esp32s3_app_read_A.bin`. Do not reattempt a 13-minute 8MB transfer, program the board, or assert a verified restore file until diagnostic results guide next step. Keep all binary backups offline/private since NVS may expose secrets; never commit binaries to GitHub. ESP32 production miner/Bitaxe firmware unchanged.


---

## 34. Experimental ESP32 256 KiB application-partition transfer — verified PASS

**2026-10-25, Windows PowerShell, ESP32-S3 v0.2, COM7, esptool v5.4.0, baud 115200.** The user ran `python -m esptool --chip esp32s3 --port COM7 --baud 115200 read-flash 0x10000 0x40000 C:\\esp_projects\\qhap_lab_backups\\esp32s3_app_256kb_test.bin`. After successful command and expected-size checks, PowerShell printed **`QHAP 256KB VERIFIED READ PASS`** and the captured 262,144-byte file's SHA-256:

`D491E78A06A8CFABB1613B2A1623917E68B0A52B706D7C5DCF5617063367832A`.

This verifies a 256 KiB **application** region read at this baud rate and adds confidence in *shorter* integrity-checked captures. Previous complete 8 MiB read at 115200 baud failed esptool's final MD5 despite transferring 100%, and no complete file was retained. A successful 256 KiB chunk does **not** validate the rest of flash or solve long-transfer integrity failures. All reads are non-destructive.

**Next gate:** Capture the 256 KiB segment at flash offset `0x0` to a separate private `esp32s3_chunk_00_000000.bin`, then verify successful esptool exit, exact 262,144-byte output, and agreement between the saved segment's static app slice `0x10000..0x3FFFF` (192 KiB) and the previously verified app test sample `esp32s3_app_256kb_test.bin` (sample offsets `0x0..0x2FFFF`). This tests the NVS-containing first chunk and overlap before automating the remaining 31 contiguous 256 KiB segments. Name every segment with hex flash offset, independently verify digests/length, abort on a failed segment, never assume all slices reflect one atomic flash snapshot across app restarts, and compare overlapping app content before assembling. Keep binary images private, out of GitHub, and production miners isolated.


---

## 35. Experimental ESP32 flash chunk zero independently verified — PASS

**2026-10-25, Windows laptop, experimental ESP32-S3 on COM7.** The user performed a 256 KiB flash read from offset `0x0` with esptool v5.4.0 and baud 115200, saved privately as `C:\\esp_projects\\qhap_lab_backups\\esp32s3_chunk_00_000000.bin`. The read succeeded with a 262,144-byte file. A bytewise overlap comparison matched the first **192 KiB** of this saved chunk (`0x10000..0x3ffff`) against the independently verified prior application 256 KiB read from offset `0x10000` (`esp32s3_app_256kb_test.bin`): **0 differing bytes**. User terminal printed **`QHAP CHUNK 00 PASS`** and SHA-256 `250F117F31A93F5E5354E32AC93B144B81819D3F31CA3306DF100A9262472808`.

**Next:** Protect chunk 00 from overwrite. Capture remaining 31 chunks (indices 01–31, size `0x40000` each, offsets `0x40000` through `0x7c0000`) using a resumable PowerShell process at baud 115200; every chunk must pass esptool's built-in digest verification, exact-byte-length check, and local SHA-256. Each successfully captured file's digest should be stored in a small local sidecar for skip/resume validation; if any read fails, halt with the suspect attempt separate rather than overwrite or falsely accept it. Do not commit binaries, expose credential-bearing full flash images, or flash new firmware until all segments are verified and reconstructed image checks pass. Flash may mutate persistent partitions between captures and therefore is not necessarily an atomic point-in-time snapshot. Existing reference QHAP miner and Bitaxe remain untouched.


---

## 36. Chunk 07 stub failure and ROM double-read probe

On 2026-10-25, the experimental ESP32-S3 full-flash recovery attempt using 256 KiB segments at 115200 baud successfully saved chunks 00–06; chunk 07 (start 0x1C0000) transferred all 256 KiB but esptool stub mode raised an MD5 digest mismatch, so the loop stopped and chunks 08–31 remain uncaptured. A later green success banner was manually printed at the PowerShell prompt, not by the completed script, and is invalid as a completion signal. The user then captured **two separate 64 KiB reads of address 0x1C0000** with esptool `--no-stub`; SHA-256 fingerprints matched and the script printed **QHAP ROM FLASH PROBE PASS**. This verifies reproducibility over 64 KiB with the ROM loader, not recovery of full chunk 07 nor of the entire 8 MiB flash image. Planned next step: read the whole 256 KiB chunk at 0x1C0000 twice with `--no-stub`, compare complete captures with each other and their first 64 KiB against the saved ROM probe, then promote to chunk 07 only if all checks pass. Keep prior verified chunks, avoid writes to the ESP32, and leave the reference miner and Bitaxe untouched. Full flash backup files must stay private.


---

## 37. Experimental ESP32 chunk 07 independently verified — PASS

**2026-10-25, separate ESP32-S3 experimental board, COM7, esptool v5.4.0.** User reported the actual result **`QHAP CHUNK 07 INDEPENDENT VERIFICATION PASS`** from a read-only Windows PowerShell file-check (not merely the earlier manually printed `Write-Host` message). The check required both distinct 256 KiB ROM `--no-stub` captures `chunk07_rom_full_A.bin` and `chunk07_rom_full_B.bin`, the promoted final `esp32s3_chunk_07_1C0000.bin`, exact sizes, SHA-256 equality with local `.sha256` sidecar, and a bytewise match of the first 64 KiB to prior ROM probe. Expected/confirmed SHA-256: `3B874D3BA46C638FC3094F8E92FB744CA974893873F8885F54E23760F9B6311B`. Earlier the second ROM capture had been missing; independent verification now passed. Thus chunks 00 through 07 cover the first **2 MiB of 8 MiB** flash (25%), with 24 chunks 08–31 remaining to capture.

**Next operation:** safely capture chunks 08–31 at offsets `0x200000` to `0x7c0000`, each `0x40000` (256 KiB) using two `--no-stub` reads at 115200 baud and SHA-256/size equality checks. Implement resumable skip of only intact chunk+hash-sidecar pairs, block overwrite, guard against orphan files, and issue a final success banner only after validating all 32 chunks. Reconstruct and independently verify final 8 MiB image in a separate stage; this milestone is **not** full backup completion. No production hardware firmware changes. Keep flash dumps private.


---

## 38. ROM chunk backup interrupted at index 13

**2026-10-25, experimental ESP32-S3 flash backup, Windows COM7.** After the independently verified chunk 07 ROM recovery, a read-only resumable PowerShell script was used to capture chunks 08–31 (256 KiB each, two independent esptool `--no-stub` ROM reads at 115200 baud, hashes required to match). The script reported **`QHAP BACKUP STOPPED: Chunk 13 read A failed`**. The reported failure came from the first ROM read for index 13, flash offset **0x340000** (within the 0x290000–0x3effff SPIFFS partition). As the script proceeds sequentially, chunks **00–12 appear to have been saved/verified** before the halt, but must be independently reaudited; 13–31 are not yet verified. Expected provisional progress: 13/32 chunks, 3.25 MiB of 8 MiB.

The summary **does not reveal the underlying esptool error** (USB disconnect, ROM packet failure, timeout, etc.). Next safe operation: inspect and quote exact esptool error output immediately preceding the stop message; inspect the local chunk 13 temporary `chunk_13_readA_*.bin` files and the presence/size of the saved chunks 08–12 and sidecars. Do not retry the whole loop, discard temp files, overwrite verified chunks, change production mining hardware, or declare the full dump valid. Espressif troubleshooting notes possible USB cable, driver, power and serial port contention; diagnosis must be based on the actual error. Keep flash binaries private.


---

## 39. Chunks 08–12 independently rechecked after chunk-13 interruption

**2026-10-25, Windows experimental ESP32-S3 (COM7), read-only flash backups.** Following the stopped ROM loader operation at chunk 13, the user ran a separate local file-length and SHA-256/sidecar verification for each of chunks 08, 09, 10, 11, and 12. All five printed **`CHUNK N PASS`**. `Get-ChildItem` for failed `chunk_13_read*.bin` attempts returned no files. Combined with prior verified chunks 00–07, **13/32 chunks (3.25 MiB) are captured and locally verified**, subject to final assembly and consistency validation. Chunk 13 begins at flash offset `0x340000`, within SPIFFS, and did not complete its first ROM-mode 256 KiB read. The actual underlying esptool error text has not been supplied; do not attribute it to a particular cause yet. Next diagnostic: attempt **only** a new, uniquely named ROM `--no-stub` read of offset 0x340000, length 0x40000 at 115200 baud, preserving all existing chunks, and capture esptool's complete error if it fails. No flash erase/write or production miner modifications.


---

## 40. Chunk 13 initial ROM capture succeeded; FF-only hash discovery

**2026-10-25 (local filename timestamp 20261024), experimental ESP32-S3 COM7 flash-backup work.** Following the stop at chunk index 13, chunks 08–12 were independently checked for correct 256 KiB length and their SHA-256 sidecars, all reporting PASS. The first retry of chunk 13 using `esptool --chip esp32s3 --port COM7 --baud 115200 --no-stub read-flash 0x340000 0x40000` succeeded: `Read 262144 bytes from 0x00340000 in 58.2 seconds (36.1 kbit/s)`, file `C:\\esp_projects\\qhap_lab_backups\\chunk13_rom_A_20261024_215954_361.bin`; printed `QHAP CHUNK 13 FIRST READ PASS`.

The SHA-256 was `3B874D3BA46C638FC3094F8E92FB744CA974893873F8885F54E23760F9B6311B`, **identical to the previously verified Chunk 07 hash**, and independently identified as the exact SHA-256 of **262,144 0xFF bytes** (erased/blank flash). This strongly suggests both the original chunk 07 and this first chunk 13 capture are erased blank flash, though it does not prove the source of esptool stub digest failures or establish the completeness of the 8 MiB backup.

**Next validation gate:** One *second*, separately captured ROM-mode read of chunk 13 (0x340000, 0x40000) with no-overwrite semantics; require normal process exit, exact 262144-byte size, matching digest to first copy and the known 0xFF digest; only then promote to `esp32s3_chunk_13_340000.bin` with `.sha256` sidecar. Keep existing chunks untouched and flash data private. Chunks 14–31 remain unverified; do not program ESP32 until complete reconstructed image has been audited.


---

## 41. Chunk 13 independently verified and safely committed to local backup

**2026-10-25, separate experimental ESP32-S3 on COM7.** User confirmed terminal result **`QHAP CHUNK 13 INDEPENDENT VERIFICATION PASS`** after two independent `--no-stub` ROM reads at 115200 baud of flash offset `0x340000` length `0x40000` (256 KiB); final `esp32s3_chunk_13_340000.bin` and `.sha256` sidecar saved privately on Windows. SHA-256 `3B874D3BA46C638FC3094F8E92FB744CA974893873F8885F54E23760F9B6311B` is the exact SHA-256 of 262144 bytes of `0xFF`, as also seen for chunk 07. This is consistent with erased/unallocated flash, not by itself proof that the stub is faulty. Previous independently verified chunks 00–12 plus chunk 13 mean **14 of 32 chunks (3.5 MiB of 8 MiB, 43.75%)** are captured; **18 chunks indices 14–31** remain to capture. Continue with two independent `--no-stub` reads per chunk, enforce exact lengths and equal SHA-256, preserve previous chunks and sidecars, and stop on any error. After all 32 pass, reconstruct into an 8,388,608-byte file, validate source chunk hashes and the final image, and keep the flash data private. Do not flash the experimental board or change working miners before recoverability verification.


---

## 42. All 32 experimental ESP32 flash chunks verified — assembly pending

**2026-10-25, Windows PC, separate experimental ESP32-S3 (COM7) read-only backup.** The resumed ROM `--no-stub` dual-read chunk capture from chunk 14 through chunk 31 completed. The user shared the final PowerShell output: **`CHUNK 31 VERIFIED PASS`**, **`QHAP ALL 32 CHUNKS VERIFIED PASS`**, and `NEXT: RECONSTRUCT COMPLETE 8MB IMAGE`. The final script audited 32 individually captured 256 KiB (`0x40000`) binary files, using exact lengths and SHA-256 verification against local sidecars (with pinned known-good hashes for chunks 00/07/13), for a total of **8,388,608 bytes (8 MiB)** across flash offsets `0x000000–0x7FFFFF`. Chunk 07 and chunk 13 previously passed repeatable ROM capture tests; their identical SHA-256 `3B874D3BA46C638FC3094F8E92FB744CA974893873F8885F54E23760F9B6311B` corresponds to all-`0xFF` data. This completion message reflects actual loop progress + final checks, unlike a prior manually printed success banner. The individual verified files are saved privately in `C:\\esp_projects\\qhap_lab_backups`, with predictable names `esp32s3_chunk_XX_OFFSET.bin`; **do not upload binary backups to GitHub** since they can expose credentials.

**Next safety gate:** Assemble the 32 files *in numerical flash offset order* into a newly named full image without overwriting existing files; pre-validate every source length and hash again, write 8,388,608 bytes exactly, compare every reconstructed 256 KiB slice to the source chunk, validate the known application overlap (0x10000) against `esp32s3_app_256kb_test.bin`, compute and separately save a SHA-256 manifest, and confirm the partition table at 0x8000 matches the earlier inspected ESP-IDF layout. Preserve all 32 source chunks and any prior incomplete attempts. The backup is reconstructed from non-atomic reads across several ESP32 resets, so it is **not yet a proven restorable device image**. Do not write firmware or change production miner/Bitaxe until image assembly + safety plan are reviewed.


---

## 43. Complete 8 MiB experimental ESP32 flash image reconstructed and verified

**2026-10-25, Windows PC, *separate experimental* ESP32-S3 (QFN56 rev 0.2, embedded 8 MB PSRAM and 8 MB flash, UART CP210x COM7).** After separately verified `0x40000` (256 KiB) ROM/stub flash read chunks 00–31 were captured and backed up under `C:\\esp_projects\\qhap_lab_backups`, the user reconstructed them in ascending address order into a private full image at `C:\\esp_projects\\qhap_lab_backups\\esp32s3_full_8mb_reconstructed.bin`. PowerShell assembly and independent comparison reported ALL FOUR results:

```text
ALL 32 RECONSTRUCTED SECTIONS PASS
APPLICATION REFERENCE PASS
PARTITION TABLE REFERENCE PASS
QHAP FULL 8MB IMAGE VERIFIED PASS
SHA256: 4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3
```

Final length verified as **8,388,608 bytes**. Every reassembled 256 KiB segment was verified against the saved source chunk SHA-256. The flash application's 256 KiB at offset `0x10000` matched the independently captured `esp32s3_app_256kb_test.bin`; partition-table bytes `0x8000..0x8fff` matched `partition_table_from_probe.bin`. A complete-image SHA-256 was saved locally in `esp32s3_full_8mb_reconstructed.bin.sha256`. Chunks 07 and 13, SHA-256 `3B874D3BA46C638FC3094F8E92FB744CA974893873F8885F54E23760F9B6311B`, are consistent with all-`0xFF` erased flash.

**Recovery scope/caveats:** This is an integrity-validated reconstructed image, **not a restore-tested recovery image**. Source reads occurred across multiple resets and moments; mutable NVS, OTA data, SPIFFS or coredump contents might differ across collection time and may not represent an atomic snapshot. It contains private flash and may include Wi-Fi secrets; retain offline, do not commit/upload binary image or local flash contents to GitHub. The **text-only digest and milestone** may be documented in GitHub. A second copy of the image on another physical drive (protected access) is preferable before experimental firmware changes. Preserve all chunk originals and sidecars until recovery is proven. The *production ESP32 mining firmware and Bitaxe stock AxeOS* have not been touched.

**Next chess-mindset gate:** establish a separate Windows ESP-IDF v6.1 `C:\\esp_projects\\qhap_bm1370_receiver_lab` workspace (no resets, cleans, rewrites of `C:\\esp_projects\\qhap`), create an isolated BM1370 synthetic UART-frame receiver application only for this experimental board, inspect its flash/partition settings, build and review before considering a flash. Subsequently connect Raspberry Pi Pico H 3.3V UART TX and GND to ESP32 verified receiving GPIO+GND; neither board sends fake pool shares nor programs a real Bitaxe ASIC. Implement reproducible tests before hardware integration. No claim of BTC breakthrough yet.


---

## 44. Dedicated Windows ESP-IDF receiver lab project created

**2026-10-25.** Following validation of the separate experimental ESP32-S3's private reconstructed 8 MiB flash image (SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`), the user created a **new, separate Windows ESP-IDF v6.1 project** with `idf.py create-project qhap_bm1370_receiver_lab` from `C:\\esp_projects`. PowerShell listed `main` and `CMakeLists.txt` inside `C:\\esp_projects\\qhap_bm1370_receiver_lab`, establishing project creation; the separate manually typed `Write-Host "QHAP ISOLATED LAB PROJECT CREATED"` banner is not an additional independent validation. Original mining workspace `C:\\esp_projects\\qhap` was not modified. No firmware build or flashing has been performed in the new workspace.

**Next baby step:** `Set-Location "C:\\esp_projects\\qhap_bm1370_receiver_lab"; idf.py set-target esp32s3` in an activated ESP-IDF Windows PowerShell. This sets the target only in the new lab directory; check the command exit code. Later design a conservative UART receiver for synthetic BM1370 frames, build and review it before a specifically identified COM7 flash. Preserve the private flash image and make a second protected copy on a separate device before flashing. Keep production ESP32 and Bitaxe unchanged.


---

## 45. Isolated Windows ESP-IDF receiver lab target configured

**2026-10-25.** After creating `C:\\esp_projects\\qhap_bm1370_receiver_lab` as a dedicated standalone project, the user executed `idf.py set-target esp32s3` in an ESP-IDF v6.1 PowerShell and reported **`QHAP ESP32-S3 TARGET CONFIGURED`**. The original Windows mining project `C:\\esp_projects\\qhap` has not been changed and there has been no flash/build yet. The isolated lab is intended to receive **synthetic 11-byte BM1370 response frames** from the Pico H emulator `tools/pico_bm1370_emulator.py` over 3.3 V UART (preamble AA 55, nonce big-endian, midstate_num, raw_id, raw_version big-endian, status/CRC5 byte; CRC5 residue zero over bytes 2–10). Repo native decoder `main/qhap_bm1370_protocol.c` uses same layout. Initial receiver must **never submit shares to pools or command the real ASIC**.

**Next safe stage:** Inspect only the isolated lab's existing `main/` files and CMake configuration before replacing any auto-generated sample files; design receiver with a separate UART1 peripheral and deferred final GPIO choice until physical board pinout/availability is checked. Compile and test host-known frames before any flash. Final pre-flash requirements: protected second copy of reconstructed 8MiB backup off the laptop, positive COM7 experimental-board identity, review firmware partition compatibility and restore strategy, no impact to original mining board / stock Bitaxe.


---

## 46. Isolated ESP32-S3 BM1370 receiver build passed

**2026-10-25, Windows ESP-IDF v6.1, experimental ESP32-S3 project `C:\\esp_projects\\qhap_bm1370_receiver_lab`.** The user replaced the auto-generated empty `main/qhap_bm1370_receiver_lab.c` starter with a synthetic BM1370 UART1 115200 8N1 **receive-only** parser and backed up the starter source plus component CMake before doing so. The receiver has provisional ESP32 GPIO17 configured as RX (physical board pinout **not yet validated**), parses 11-byte `AA 55` frames matching `tools/pico_bm1370_emulator.py`, checks CRC5 residue zero, decodes nonce/job ID/raw version, reports to ESP-IDF logs, and runs an internal known-good/corrupt frame self-test before enabling UART. `main/CMakeLists.txt` declares `PRIV_REQUIRES esp_driver_uart`. No Wi-Fi, Bitcoin pool client, real ASIC commands or mining code are part of the lab program. The correct known-good frame `AA 55 A1 B2 C3 D4 02 5A 12 34 8C` has CRC5 residue 0; corrupting byte 5 with XOR 1 produces residue 29 according to a separate calculation.

The user ran `idf.py build` and reported **`QHAP RECEIVER BUILD PASS`**, with generated application binary `build/qhap_bm1370_receiver_lab.bin` exactly **195,888 bytes**. The terminal did not show a SHA-256 value; do not invent one. **No flashing/reset/COM7 access has taken place.** The experimental board's reconstructed original 8MiB flash recovery reference remains privately stored on Windows (image hash `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`), not restore-tested and not an atomic snapshot. Original miner and Bitaxe stock firmware unaffected.

**Next gates before flashing:** Inspect built image target/header and `sdkconfig` flash size/partition table/console routing (ESP-IDF defaults may not match physical 8MB), review candidate flash layout versus existing device partition layout, confirm GPIO17 board-pin accessibility, back up the private full image to a separate secure drive, positively re-identify COM7 as the experimental board, and then perform a deliberately limited lab flash. Verify boot runtime CRC5 self-test and logs before the Pico 3.3V UART signal/GND are connected. Do not claim runtime success from compile-time success.


---

## 47. Verified flash-size correction in receiver lab

2026-10-25: User ran ESP-IDF menuconfig in the isolated Windows receiver project and verified `CONFIG_ESPTOOLPY_FLASHSIZE="8MB"` in sdkconfig. This corrects the earlier 2MB project default to match the separately identified experimental ESP32-S3's physical 8MB flash. Last reported partition selection was still single application, which differs from the original verified OTA layout (NVS at 0x9000, OTA data 0xE000, app0 at 0x10000, app1 at 0x150000, SPIFFS 0x290000, coredump 0x3F0000). Do not flash based on the default single-app table. Next: create and validate a custom CSV matching the backup's partition layout inside the *isolated* project, compare with the backed-up binary partition table, and only then change the lab configuration and rebuild. The previous 195888-byte binary was built before the flash-size correction, so it must be rebuilt. The original QHAP miner and stock Bitaxe remain unchanged.


---

## 48. Custom OTA partition scheme selected in receiver lab

**2026-10-25.** In the isolated Windows ESP-IDF v6.1 receiver lab at `C:\esp_projects\qhap_bm1370_receiver_lab`, the user used menuconfig to select the **custom partition table CSV** and filename `partitions_qhap_original.csv`, preserving the default partition offset `0x8000` and MD5 table checksum. The user verified via sdkconfig output:

```ini
CONFIG_ESPTOOLPY_FLASHSIZE="8MB"
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions_qhap_original.csv"
```

The custom CSV's decoded six logical entries had been compared separately to the original device's binary partition table, and the earlier PowerShell comparison reported `QHAP PARTITION LAYOUT MATCH PASS` (NVS, otadata, app0/ota_0, app1/ota_1, SPIFFS, coredump). **No rebuild using the now-correct config has yet been confirmed, and no firmware was flashed.** Next: `idf.py build` exclusively inside the isolated lab, decode its actually *generated* `build/partition_table/partition-table.bin` via ESP-IDF `gen_esp32part.py`, compare all six decoded entries against the original saved binary table, inspect app image and flash arguments. Be especially careful with OTA select state: the existing `otadata` can still select app1 even if app0 is flashed; do not assume writing at 0x10000 will boot. Preserve original production miners and private reconstructed 8MB image.


---

## 49. Corrected receiver build partition binary matches original six entries

On 2026-10-25, the user reported `QHAP GENERATED PARTITION TABLE PASS` from a script that first ran `idf.py build` for `C:\\esp_projects\\qhap_bm1370_receiver_lab` with target ESP32-S3, flash size 8MB, and `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions_qhap_original.csv"`. The script then decoded `build/partition_table/partition-table.bin` and the original partition-table backup, requiring exactly six identical partition rows (NVS, OTA data, app0, app1, SPIFFS, coredump). **The generated table matches the recovered original layout; this is build-time validation, not a flashing or runtime test.** No flash has taken place.

**Next critical preflash gate:** Inspect saved original full-flash backup's OTA metadata at `0xE000` and `0xF000`, plus first byte of OTA app slot images at `0x10000` and `0x150000`, to learn whether the original device had a programmed boot selection or an erased OTA data partition. OTA bootloader consultation of `otadata` can cause a plain app-slot-0 flash to *not* boot. Check also the local build flasher args, and avoid `idf.py flash` (which writes the bootloader and partition-table as well). The reconstructed backup is temporally non-atomic, so saved OTA data need not reflect the current live state. Require a second protected independent copy of the private 8MB backup and positive COM7 board identity before future writing. Source firmware is receiver-only; original miners unchanged.


---

## 50. Saved OTA metadata indicates original App 0 selection

2026-10-25: After generating and matching the ESP32-S3 lab custom partition table to the original six entries, the user inspected the privately reconstructed 8 MiB pre-flash backup. At OTA sector offset 0xE000, the first 32 bytes start with little-endian ota_seq=1, have ota_state=0xFFFFFFFF, and stored CRC32=0x4743989A; using Espressif's OTA CRC convention (CRC32 of four-byte seq with initial value 0xFFFFFFFF), seq=1 computes the same 0x4743989A. At 0xF000, the first 32 bytes are all FF, although the full 4096-byte sector is not all FF, so the inactive sector's later bytes should be inspected before drawing conclusions about every byte there. Application partition 0 at 0x10000 starts with ESP image magic 0xE9; partition 1 at 0x150000 starts with 0xFF. For two OTA slots, sequence 1 selects ota_0 by (seq-1) % 2 if bootloader accepts the OTA record and other security constraints. These are findings from a non-atomic *saved* backup, not a fresh measurement of running firmware. App 1 has NOT yet been fully scanned for non-FF content. No firmware flashed.

Next safe check: a read-only local Python script with binascii.crc32(seq_raw,0xFFFFFFFF) to validate the 32-byte OTA slot entries, locate the first non-FF byte in second OTA sector, and scan entire app1 range [0x150000,0x290000) for evidence of image/data. Continue to preserve original App0; no blind idf.py flash because that would overwrite the bootloader and partition table and would not honor the existing OTA slot selection as intended. Retain independent protected copy of full flash backup before any writes to experimental ESP32; working Bitcoin miner and Bitaxe remain untouched.


---

## 51. Offline OTA CRC validated and second app slot completely erased

**2026-10-25** — user inspected *private offline* reconstructed 8MiB experimental ESP32-S3 flash image (SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`). Offline Python script confirmed:

- OTA entry at `0xE000`: `ota_seq=1`, `ota_state=0xFFFFFFFF`, stored CRC matches computed CRC; **CRC valid**. For two OTA slots, sequence 1 selects **ota_0** under standard ESP-IDF logic.
- OTA entry at `0xF000`: `ota_seq=0`, `ota_state=0xFFFFFFFF`, **CRC invalid**; first non-0xFF byte of that sector is at `0xF000` itself, so this sector is **not fully erased** (it starts with four bytes 0x00) and is not a valid competing OTA boot selection.
- **Entire app1 partition `[0x150000,0x290000)`, 0x140000 = 1,310,720 bytes / 1280KiB, consists of 0xFF bytes.** App0 at `0x10000` starts with ESP image magic `0xE9` in previous scan.
- Script printed `QHAP OFFLINE OTA VALIDATION COMPLETE`.

**Interpretation:** The backup strongly suggests the board formerly booted ota_0 and the app1 slot is available to host an isolated ~196KB lab receiver, *without overwriting app0*. This is a promising plan, not permission to flash: OTA data sampled across resets may be stale relative to the device; verify live current state and app/bootloader compatibility. Avoid `idf.py flash` because it also writes bootloader/partition table. Before experimental writes: query secure-boot/flash-encryption read-only with `python -m esptool --chip esp32s3 --port COM7 --baud 115200 --no-stub get-security-info` (ensure physical COM7 board identity), preserve a second protected copy of original flash image on another physical medium, confirm GPIO17 physical access and USB logging, inspect generated app image and bin size, and carefully plan specific flash addresses and **separate deliberate OTA metadata switch**. No change to production ESP32 miner or Bitaxe stock AxeOS.


---

## 52. Experimental ESP32-S3 security inspection: unsigned, unencrypted flash supported

On 2026-10-25, the user ran read-only esptool v5.4.0 `get-security-info` with `--chip esp32s3 --port COM7 --baud 115200 --no-stub` against the positively identified **separate experimental ESP32-S3 revision v0.2**. It reported **Security Flags 0x00000000, Secure Boot Disabled, Flash Encryption Disabled, SPI_BOOT_CRYPT_CNT 0x0**. The board reset via RTS after inspection. This rules out those two security requirements for unsigned/unencrypted lab application installation, but **does not prove bootloader/image compatibility or the current live OTA boot slot**. Prior offline full-image check verified App1 (0x150000–0x28FFFF) entirely erased and backup OTA sequence 1 CRC-valid selecting App0, subject to stale/non-atomic image limitations. No flashing, erasing or mining firmware modifications have occurred.

**Next prewrite gates:** Validate final freshly built receiver image as ESP32-S3 via local `python -m esptool image-info`, confirm binary length fits App1 1,310,720-byte partition and flash configuration, obtain independently protected second copy of the reconstructed original 8 MiB flash backup (SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`) on separate physical storage, physically verify GPIO17 label, then review narrow app1-only write and OTA metadata switch/rollback plan. Never casually run `idf.py flash` (it could write bootloader and partition table). The original QHAP miner and Bitaxe should remain untouched.


---

## 53. Lab receiver application image inspection completed and slot-size gate passed

**2026-10-25, Windows isolated ESP-IDF v6.1 lab, experimental ESP32-S3 v0.2.** The user shared direct `python -m esptool image-info` output for the compiled receiver application `C:\\esp_projects\\qhap_bm1370_receiver_lab\\build\\qhap_bm1370_receiver_lab.bin`: **ESP32-S3 image**, **195,888 bytes**, six segments, image header **8MB flash, DIO, 80MHz**, footer checksum `0xe2` **valid**, image validation hash `7d3d33dd83bf5a17919f56ae0d848fbcde64d25f7aad985f48bd932bb7c840d9` **valid**, project `qhap_bm1370_receiver_lab`, ESP-IDF v6.1, secure version 0, chip ID 9, chip revision allowance includes physical rev v0.2. The separate Windows PowerShell size check printed **`App image size: 195888 bytes`** and **`App 1 capacity: 1310720 bytes`**. The local application binary's SHA-256 shown in the screenshot was `14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE` (distinguish the firmware **file** SHA-256 from ESP-IDF **internal image validation hash**). A manual `Write-Host "QHAP APPLICATION IMAGE INSPECTION PASS"` followed, but the independently visible `esptool image-info` and exact file length substantiate the milestone. The app image fits the `ota_1` slot with considerable margin.

**Not yet proven:** This image will actually boot under the existing, potentially older bootloader and its OTA selection policy; original flash-mode/frequency and bootloader compatibility need review. It has not been flashed or tested at runtime. The reconstructed backup is not restore-tested or atomic. **Next required protection:** confirm a second access-controlled copy of the *original* 8,388,608-byte backup on a separate physical device, verifying original SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3`. Avoid uploading credential-bearing flash images to GitHub or public cloud. Before any app1-only flash and OTA switch, positively identify COM7 again, verify GPIO17 board pinout, plan rollback and OTA metadata operation, and preserve app0/bootloader/partition table. Working miner ESP32 and Bitaxe unaffected.


---

## 54. Second identical recovery image copy verified on D volume

**2026-10-25** — Windows user reported `QHAP SECOND LOCAL BACKUP VERIFIED PASS` after verifying the existing 8,388,608-byte experimental ESP32-S3 reconstructed backup and copying it from `C:\\esp_projects\\qhap_lab_backups\\esp32s3_full_8mb_reconstructed.bin` to `D:\\QHAP_Recovery_Private\\esp32s3_original_8mb_backup.bin`. Both local copies have expected SHA-256 `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3` and correct file size. Windows `Get-Disk` reported **one physical Micron 476.9 GiB NVMe SSD**, with C: and D: volumes on that same disk. Thus two integrity-validated local copies are **not** physically independent backups; a protected third copy on a separate physical device is still required prior to any experimental flash. The file may contain Wi-Fi/password secrets, so do not share/push binaries to GitHub.

Read-only next-stage plan: inspect the original ESP32-S3 second-stage bootloader at flash offset 0x0 from the **saved offline image**, using `python -m esptool --chip esp32s3 image-info` on a 32 KiB extracted copy without communicating with COM7. Espressif documents offset 0x0 for ESP32-S3 and general bootloader forward compatibility with newer ESP-IDF apps, but image/runtime compatibility is not guaranteed. Check bootloader IDF version, chip, flash mode/speed and OTA capabilities before any app1-only flash or controlled OTA switch. Never run `idf.py flash` casually; it writes bootloader/partitions. Original mining ESP32 and Bitaxe remain untouched.


---

## 55. Original ESP32-S3 bootloader examined offline — v5.5.4, image valid

**2026-10-25.** From the private reconstructed, SHA-256 validated 8MiB flash image of the *separate experimental ESP32-S3* (COM7), the user extracted 32,768 bytes from flash offset `0x0` to `original_esp32s3_bootloader_inspection.bin` and ran `python -m esptool --chip esp32s3 image-info`. Reported **ESP32-S3 bootloader** (Chip ID 9, bootloader version 1), **ESP-IDF v5.5.4**, compile date **2026-06-02**, 3 segments, flash header **4MB, DIO, 80MHz**, checksum **0x6e valid**, image validation SHA-256 **31fcf121f30c5f9d7ccaa304ae5a2e7cfbd7b090ba8ae277ffb7d54b24ea496c valid**. This is an *offline saved-image* finding, not a live boot trace. Separately constructed receiver app is ESP-IDF **v6.1**, ESP32-S3, 8MB/DIO/80MHz image, 195,888 bytes, app binary SHA-256 `14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE`, image header and validation hash passed, and app1 is 1280KiB. Existing board physically has 8MB flash, and both app slots fall under 4MB addressing. Espressif ESP32-S3 bootloader docs state older bootloaders generally boot newer ESP-IDF app builds (OTA forward compatibility), and bootloader reconfigures flash from selected app's header; a 4MB bootloader header vs 8MB app header is not alone an incompatibility, **but real boot has not yet been tested**.

**Next read-only validation:** extract original app0 `[0x10000,0x150000)` from the private full image and inspect its image header/version/checksum to compare with bootloader/new app; keep the extracted app private. Before flashing any app1 slot or altering otadata, safeguard original 8MB backup on **another physical drive or computer**, verify physical GPIO17 exposure, and plan OTA selection/rollback. The C: and D: copies on the same NVMe are not physically independent. **No flash writes/erases** yet; original real miner and Bitaxe unaffected.


---

## 57. Amazon order confirms DevKitC-1 N8R8 and GPIO17 header pin

**2026-10-25:** Amazon.in order-confirmation emails in the linked personal Gmail identify the boards purchased for QHAP as **Espressif ESP32-S3-DevKitC-1-N8R8** with **ESP32-S3-WROOM-1**, product ASIN **B09MHP42LY** (https://www.amazon.in/dp/B09MHP42LY). One unit was delivered September 25, 2026; two additional units were delivered October 10, 2026. The exact physical COM7 board is not individually serial-traceable from receipts, but its measured ESP32-S3 rev v0.2, physical 8MB flash/8MB PSRAM, and CP210x USB-UART interface match the SKU. The Raspberry Pi **Pico H with soldered headers** (ASIN B0GMHQHDH7) was delivered October 7, and **ApTechDeals 40 female-to-female jumper wires** (ASIN B074J8M43C) October 6. This inventory is sufficient for the intended isolated UART test; the jumper wires may be used if the header types physically match.

Official Espressif ESP32-S3-DevKitC-1 v1.0/v1.1 board manuals agree **GPIO17 = header J1 pin 10**, between GPIO16 (J1-9) and GPIO18 (J1-11), with **GND on J1 pin 22** or J3 pin 1. The pinout's built-in U1TXD function on GPIO17 does NOT prevent using it as UART1 RX because ESP-IDF uart_set_pin supports rerouting via GPIO matrix; the lab firmware currently binds **UART_NUM_1 RX to GPIO17**, 115200 8N1. The ESP32-S3-DevKitC-1 hardware revisions mainly differ in RGB LED GPIO assignment (GPIO48 versus GPIO38), not the GPIO17 breakout, so board revision identification is not needed for this particular RX pin. The Raspberry Pi Pico H official UART0 layout is **GP0 physical pin 1 = TX**, **GND physical pin 3**. Safe eventual wiring with boards independently powered by USB: Pico H GP0/TX pin 1 → only the isolated experimental ESP32 J1 pin 10/GPIO17; Pico H GND pin 3 → ESP32 GND J1 pin22 or J3 pin1; 3.3V logic, no VBUS/VSYS/5V/3V3 inter-board power connections. Test wiring only *after* experimental firmware is installed, powered off while connecting, and physical board labels checked at time of assembly. Never attach the production mining ESP32 or Bitaxe.

Official source: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html ; Espressif v1.0 guide is linked from it; Raspberry Pi official Pico pinout documentation https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf ; ESP-IDF UART GPIO remap https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/uart.html .

Remaining prewrite gate: separately assess app1-only image write, secure existing app0 and OTA recovery, protected backups (C and D same physical disk risk accepted by user), verify flash addresses and the live board identity and OTA state. No firmware flash has yet occurred.


---

## 58. Live COM7 OTA metadata matches saved App0 selection

**2026-10-25:** The user performed an 8,192-byte read-only flash dump from experimental ESP32-S3 COM7 at address 0xE000 using esptool v5.4.0, --no-stub and 115200 baud. The ROM identified ESP32-S3 revision v0.2, MAC suffix 0a:58. Live OTA entry at **0xE000** has sequence **1**, state **0xffffffff**, and CRC **0x4743989a**; second at **0xF000** has sequence **0**, state **0xffffffff**, CRC **0xffffffff**. These fields match the previously validated private reconstructed backup and indicate App0 remains selected by normal two-slot ESP-IDF sequence mapping. Full live OTA file is stored locally in the private QHAP backup directory. No flash changes made. Next: verify the CRC using a script and compare all 8192 live OTA bytes to the reference image; then check candidate App1 state live as needed before considering application-only programming. User chose to keep recovery images private on laptop C and D volumes (same physical NVMe; correlated-loss risk explicitly noted).


---

## 59. Live App1 first 64 KiB erased

On 2026-10-25, the separate experimental ESP32-S3 (COM7, rev v0.2) was read using esptool v5.4.0 in ROM no-stub mode: address 0x150000, length 0x10000 (65,536 bytes). The saved readback was checked by Python for exact length and every byte equal to 0xFF. Output: `QHAP LIVE APP1 FIRST 64KB ERASED PASS`. Previous read-only live OTA sector comparison also matched the historical 8 KiB image; boot selection remains App0. This is a read-only milestone: it proves only App1's first 64 KiB is blank. The rest of the 1280 KiB App1 partition still needs live verification. No flashing, OTA changes, or changes to the independent Bitaxe were performed.


---

## 60. Live App1 next 256 KiB verified erased

**2026-10-25:** User completed a read-only live flash read of the second App1 region on the experimental ESP32-S3, COM7, address `0x160000`, length `0x40000` (256 KiB). Python checked exact 262,144 bytes and that every byte was `0xFF`; output: `QHAP LIVE APP1 NEXT 256KB ERASED PASS`, with no exception from follow-on PowerShell exit-code check. Together with the earlier 64KiB at `0x150000–0x15FFFF`, **320 KiB of the 1,280 KiB App1 slot** is confirmed erased in separate live reads. **960 KiB remains** to check live: `0x1A0000–0x28FFFF`. The historical offline reconstructed backup already showed all App1 erased, but the live scan has not yet been completed. No flash write, OTA switch, or Bitaxe change.


---

## 61. Third live App1 read confirms 576 KiB erased

**2026-10-25:** User read 262,144 bytes read-only from experimental ESP32-S3 COM7 at address `0x1A0000` (size `0x40000`), esptool v5.4.0 `--no-stub`; Python confirmed exact byte length and every byte equal to `0xFF`. The screenshot displays `QHAP LIVE APP1 THIRD REGION ERASED PASS`; PowerShell exit-code check produced no error. Together with previously confirmed 64 KiB at 0x150000 and 256 KiB at 0x160000, **576 KiB of 1280 KiB App1** is verified erased on live device, with **704 KiB remaining**, `0x1E0000–0x28FFFF`. Follow-up live reads can be three disjoint 256/256/192 KiB segments beginning `0x1E0000`, `0x220000`, and `0x260000`; verify bytes separately. Original App0 boot selection and Bitaxe are untouched. No flash writes.


### Live App1 fourth-region erased check (2026-10-25)

Read-only COM7 inspection of the experimental ESP32-S3: the 256 KiB region starting at 0x1E0000 was confirmed entirely 0xFF, matching the historical offline backup. Combined live inspected App1 space is 832 KiB of 1280 KiB (65%). The remaining 448 KiB starts at 0x220000. Original App0 and OTA selection remain unchanged. No firmware was written.


### Live App1 fifth region (2026-10-25)

Read-only esptool v5.4.0 COM7 experimental ESP32-S3 read of 0x220000 for 0x40000 bytes succeeded. Offline Python verified exact 262144-byte length and all data 0xFF: `QHAP LIVE APP1 FIFTH REGION ERASED PASS`. Verified live App1 total: 1088 KiB out of 1280 KiB (85%). Final still-unverified region: 0x260000–0x28FFFF (0x30000, 192 KiB). No firmware flashed, OTA switch or Bitaxe modifications.


### Live App1 full partition verified erased (2026-10-25)

Read-only live ROM-mode flash inspections on the **separate experimental ESP32-S3** (COM7) have now covered its entire OTA App1 partition, half-open address interval `[0x150000,0x290000)`, **0x140000 = 1,310,720 bytes (1280 KiB)**, in six contiguous, nonoverlapping reads: `0x150000 +0x10000`, `0x160000 +0x40000`, `0x1A0000 +0x40000`, `0x1E0000 +0x40000`, `0x220000 +0x40000`, `0x260000 +0x30000`. Python scripts validated each saved read's exact length and that every byte equals `0xFF`. The sixth and final read returned **196,608 bytes** and the independently checked output `QHAP LIVE APP1 FINAL REGION ERASED PASS`, with no nonzero exit or PowerShell exception. Thus the live App1 partition was fully empty across those successful sequential checks. Earlier live OTA sectors were exact byte-for-byte matches to the validated historical backup, selecting the original App0. Application candidate remains compiled only, ESP32-S3 ESP-IDF 6.1, 195,888-byte image. No firmware flash, OTA selector change or Bitaxe modification. The recovery flash images are retained privately on the user's laptop, with C: and D: being the same physical SSD. As an additional prewrite check, verify all six downloaded region files form exactly the expected continuous range and compare with the original image; then review a narrowly scoped App1-only programming method and explicit OTA rollback plan. Do not use the broad `idf.py flash` shortcut.


### Final App1 six-region audit (2026-10-25)

User independently ran the complete local six-file App1 audit. Python checked the 8 MiB original reconstructed backup against its SHA-256, verified that the six live readback regions are correctly adjacent with no gaps or overlap, checked each exact region length and each byte equal to 0xFF, and confirmed each region exactly matches the original backup at the corresponding flash offset. Actual output: all six regions VERIFIED; `ALL SIX LIVE FLASH REGIONS: PASS`; `COMPLETE APP1 COVERAGE: PASS`; `LIVE APP1 VS ORIGINAL BACKUP: MATCH`; `QHAP APP1 PRE-INSTALLATION INTEGRITY PASS`. Region totals: 1,310,720 bytes, App1 0x150000–0x28FFFF. The app candidate has previously passed standalone integrity and partition-size checks. Next planned operation: carefully isolated application-only flash into App1, leaving existing bootloader, partition table, App0 and OTA boot selector unchanged, followed by independent readback before any OTA activation. An App1 write by itself does not start the new firmware. Prepare separate planned/manual rollback procedure before OTA activation. No write yet.


### App1-only receiver flash written successfully (2026-10-25)

The user supplied the full Windows PowerShell/esptool 5.4.0 output for the FIRST actual flash write to the separate experimental ESP32-S3 board, COM7, after host-side verification of application firmware size and hash, two local 8MB backups, and ESP32-S3 MAC identity. esptool identified chip ESP32-S3 rev v0.2 and the expected COM7 device. It reported `Flash will be erased from 0x00150000 to 0x0017ffff`, `Wrote 195888 bytes at 0x00150000 in 21.2 seconds`, `Hash of data verified`, and the PowerShell wrapper printed `QHAP APP1-ONLY FLASH WRITE COMPLETED`. This is the isolated ESP-IDF v6.1 BM1370 synthetic UART receiver firmware, not a miner. The command targeted ONLY flash App1 beginning 0x150000. The OTA boot selector was NOT deliberately modified by this command, and actual boot selection post-write has NOT yet been independently checked. Original App0, bootloader and partition table were not intentional write targets. **Next mandatory steps: independent read-back of all 195888 programmed bytes and byte-for-byte SHA-256 match to the original build, then re-read live OTA 8KB and compare to saved pre-flash OTA. Plan an explicit, reversible OTA selection method before booting the lab app.** The stored lab firmware has not yet been shown executing or wired to Pico. Keep private backups private; do not commit binary flash backups. Bitaxe and original production miner are separate and untouched.


### App1 independent postflash byte-for-byte verification (2026-10-25)

User supplied a successful Python comparison of ROM-mode flash readback from the separate experimental ESP32-S3 COM7 App1 at 0x150000 against the exact original built receiver application. Original and readback each have 195,888 bytes, both matched the expected firmware SHA-256 `14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE`, and the saved files matched byte-for-byte. Verified real program output: `QHAP INDEPENDENT FIRMWARE READBACK PASS`; `QHAP APP1 FIRMWARE BYTE-FOR-BYTE MATCH`; `QHAP APP1 FIRMWARE SHA256 VERIFIED`. The follow-on PowerShell `$LASTEXITCODE` check did not throw. App1 firmware is now installed and independently verified, NOT yet selected for boot, NOT tested executing, and NOT wired to the Pico. Next: read the current 0xE000–0xFFFF OTA sector from COM7 into a NEW local inspection file and compare it byte-for-byte with pre-flash OTA file and historic image; then design safe, recoverable OTA selection procedure using knowledge of original ESP-IDF v5.5.4 bootloader and its rollback configuration. Do not casually replace bootloader, partition table or App0. Keep private flash images off GitHub.


### Postflash OTA data unchanged after installing App1 receiver

On 2026-10-25, the user read the experimental ESP32-S3's entire 8192-byte OTA selection partition again after the successful App1-only flash. A separate Python integrity check confirmed that the postflash and preflash 8192-byte OTA files match byte for byte and that the original App0 entry still has sequence 1, state 0xFFFFFFFF and a valid CRC. Output included `POST-FLASH OTA VS PRE-FLASH OTA: MATCH`, `APP0 BOOT SELECTION PRESERVED: PASS`, and `QHAP POST-FLASH OTA INTEGRITY PASS`. App1 receiver firmware was previously read back in full and matched its build SHA-256 byte for byte. App1 is installed but not boot-selected, executed, or wired to Pico. Prior to activation, prepare a manual way to restore App0 OTA selection: original first OTA sector 0xE000 contains valid seq1 and can be preserved while modifying only second sector 0xF000. Automatic rollback cannot be assumed from the bootloader image alone. Both raw recovery images remain private on local laptop C and D drives.


### Offline OTA App1 candidate generated; not flashed (2026-10-25)

User verified an OFFLINE-ONLY 4096-byte candidate OTA second-sector image. Its intended slot is ota_1, sequence=2, CRC32=0x55F63774 (computed using ESP-IDF sequence checksum logic), and state=0 (ESP_OTA_IMG_NEW). Existing first OTA sector has valid sequence=1 selecting App0 and was preserved; user output: `APP0 ORIGINAL SELECTION: PRESERVED`, `APP1 PROPOSED OTA SEQUENCE: 2`, `APP1 CANDIDATE CRC: 0x55F63774`, `QHAP OTA ACTIVATION IMAGE PREPARATION PASS`, `NO DEVICE FLASH WRITES PERFORMED`. The candidate exists only in local private storage. Not yet written to 0xF000; App1 receiver is stored and independently verified but not selected to boot. Critical: the original v5.5.4 bootloader's automatic rollback configuration is unknown, so automatic rollback must NOT be presumed. ESP-IDF v5.5.x docs state state 0 (NEW) becomes PENDING_VERIFY and reverts on a subsequent reboot only IF CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE is compiled. Prepare and hash-verify an exact 4KB preflash second OTA sector recovery file from the verified saved preflash 8192-byte OTA image; recovery can restore just flash sector 0xF000 via ROM serial while preserving the original first sector 0xE000. Validate the recovery file and the physical BOOT/EN ROM entry path before the first boot. Note esptool normally hard-resets the chip at the end of write-flash; use deliberate reset behavior and serial-monitor plan, never issue the activation write blindly.


### OTA recovery sector prepared and verified 2026-10-25

User generated, validated and locally saved the 4096-byte original OTA second-sector readback as an App0 manual-rollback image. Validation checked the original full flash backup SHA-256, the pre- and post-App1-flash OTA metadata byte-for-byte match, valid original App0 sequence=1/CRC, and candidate App1 OTA sequence=2/state=0/CRC 0x55F63774. Output: `ORIGINAL APP0 OTA ENTRY: VALID`, `APP1 CANDIDATE ENTRY: VALID`, `APP0 RECOVERY SECTOR: 4096 BYTES`, `QHAP OTA RECOVERY IMAGE PREPARATION PASS`, `NO DEVICE FLASH WRITES PERFORMED`. Local recovery file `C:\esp_projects\qhap_lab_backups\qhap_app0_original_f000_RECOVERY.bin` SHA-256 `58B59EB397FCC8B699D265A18AF84E063DC9BAB0BD03632AD2F91E70038AA788`. This file is private; do not upload its binary to GitHub. Activation candidate likewise offline only. First-boot automatic OTA rollback depends on bootloader build settings and cannot be assumed. Before OTA activation, confirm that manual BOOT + RESET enters chip ROM serial download mode via a non-writing esptool `read-mac` with `--before no-reset --after no-reset`, and document the manual ROM recovery path. Distinguish recovery *file verified* from a successfully executed rollback (not done). No OTA activation yet.


### Manual download-mode recovery handshake not yet proven (2026-10-25)

User attempted a non-writing serial ROM access test with esptool v5.4.0 via COM7 using `--before no-reset --after no-reset --no-stub read-mac`. The tool returned `Failed to connect to ESP32-S3: No serial data received`. Follow-on PowerShell checks raised errors for failed manual ROM access and unexpected identity. A subsequently displayed green PASS line was manually printed with `Write-Host` despite the tool failure; it is **not** evidence of recovery access. Do not activate the OTA App1 candidate until a genuine ROM handshake succeeds. Next troubleshooting step: on the experimental ESP32-S3 connected through the USB-to-UART port, hold BOOT, tap and release RESET/EN, release BOOT, and immediately repeat the bare `read-mac` command without PowerShell output redirection. Check full actual esptool output and exit status; if unsuccessful, distinguish manual boot-entry failure from COM-port/driver failure with an automatic-reset read-only test. App1 receiver has been flashed and verified but remains not selected. App0 OTA selection and Bitaxe unaffected.


### Manual ROM download-mode access retest passed (2026-10-25)

User repeated manual BOOT / RESET entry on the separate ESP32-S3 laboratory board and provided real esptool 5.4.0 output from `read-mac --before no-reset --after no-reset --no-stub` on COM7. This time the tool printed `Connected to ESP32-S3 on COM7`, identified revision v0.2, reported the expected MAC suffix `0a:58`, and terminated with `Staying in bootloader.` Unlike the prior failed attempt, the PowerShell exit-code check and MAC check did not throw. A later manually printed PASS label is thus supported by real tool evidence. Manual serial ROM connection is now **verified**, which is important for emergency recovery independent of the application. ROM flash write rollback has not been attempted or validated. App0 boot selection remains intact, and App1 receiver (already byte-for-byte verified) is not yet boot-selected. Recommended next safe step is a read-only ROM-mode read of live OTA second sector 0xF000 length 0x1000 and byte comparison against the SHA-256-verified original recovery-sector image, with no writes. Verify manual path before controlled OTA activation; after any ROM read, reset normally to boot App0. Original miner/Bitaxe remains untouched.


### Manual ROM-mode F000 sector readback matches recovery image (2026-10-25)

The user confirmed a successful no-write ESP32-S3 COM7 ROM-mode read of 4096 bytes at offset `0xF000` with `--before no-reset --after no-reset --no-stub`. The independently checked live sector is byte-identical to local `qhap_app0_original_f000_RECOVERY.bin`, whose expected SHA-256 is `58B59EB397FCC8B699D265A18AF84E063DC9BAB0BD03632AD2F91E70038AA788`. Actual outputs: `MANUAL ROM FLASH READ: PASS`, `LIVE OTA SECTOR VS RECOVERY IMAGE: MATCH`, `QHAP MANUAL ROM RECOVERY READ VALIDATION PASS`, with no PowerShell exit error. This establishes manual download-mode access and a recovery-sector read, but NOT that recovery flash writing has been proven. Original OTA sector `0xE000` remains the active App0 selector; App1 already holds the verified experimental receiver firmware and is not boot-selected. Next step: carefully rehearse a write of the IDENTICAL original 4KB data back to unused/invalid OTA selector sector `0xF000`, keeping first sector intact, and independently verify full live `0xE000–0xFFFF` bytes afterward. This is a real sector erase/write, so gate by device identity, expected backup hash and manual download mode; don't accidentally reset/start App1.


### Recovery-sector rewrite rehearsal passes; original OTA remains unchanged (2026-10-25)

After successful manual ROM access and read-back of OTA sector F000, the user conducted the planned true flash-write rehearsal on the separate experimental ESP32-S3 COM7. The verified original 4096-byte sector image was reprogrammed only at 0xF000, not at App0, App1, bootloader or the first OTA sector (0xE000). In-device esptool operation reported `Staying in bootloader.` and the scripted follow-on 8192-byte OTA readback, compared to the pre-flash OTA reference, completed with `QHAP RECOVERY REHEARSAL WRITE VERIFIED PASS` and `FULL OTA CONTENTS UNCHANGED: PASS`. This confirms the emergency write path is operational and the current OTA boot selection is unchanged, while it does **not** yet demonstrate automatic rollback or actual recovery from a booted failed App1. A separate App1 activation 4096-byte image exists offline: seq=2, state=0 (NEW), CRC32=0x55F63774; the original F000 sector is backed up locally with SHA-256 58B59EB397FCC8B699D265A18AF84E063DC9BAB0BD03632AD2F91E70038AA788. Manual activation should program only F000 while leaving E000 intact, with `--before no-reset --after no-reset --no-stub`, and read back complete 8KB OTA data before any intentional hardware reset; the manual recovery write can restore the original second sector if needed. The actual v5.5.4 bootloader's rollback config is unknown: ESP_OTA_IMG_NEW may change to PENDING_VERIFY on first boot if configured; do not presume automatic rollback. App1 experimental receiver has not yet been booted or connected to Pico; Bitaxe unaffected.


### App1 first hardware boot succeeded, then board returned to original firmware (2026-10-25)

The user provided five screenshots from Windows ESP-IDF monitor on the isolated experimental ESP32-S3 (COM7). The first controlled App1 boot reached ESP-IDF **v6.1** app startup with project `qhap_bm1370_receiver_lab`, proving forward compatibility with the retained original v5.5.4 second-stage bootloader. Logs included `QHAP isolated BM1370 receiver starting`, `BM1370 CRC5 SELF-TEST PASS`, `UART1 RX GPIO17, 115200 8N1: receive-only`, `No network, mining pool, or ASIC control enabled`, and multiple periodic `READY valid=0 rejected=0` messages up to ~70 seconds. No Pico transmitter was connected, so zero valid and rejected UART frames is expected. A flash log warning reports physical flash size 8192 KB larger than binary image header 4096 KB; this did not prevent startup. The screenshots then show a fresh ESP-ROM reset banner `rst:0x1 (POWERON),boot:0x8`, followed by the **original App0 firmware**, identifiable by repeated Wi-Fi scanning and RGB LED test messages. This indicates the lab firmware ran but subsequently the device rebooted into App0. The reason for restart is unknown from screenshot; App1 OTA state may have changed due to bootloader rollback (`NEW` to `PENDING_VERIFY` and then `ABORTED` if reset without app confirmation) per Espressif IDF 5.5 ESP32-S3 OTA documentation, but must **not** be asserted as proven without read-only live OTA sector-state inspection. Do not repeat Wi-Fi network names from screenshot in documentation. No Pico wiring yet; no Bitcoin mining breakthrough claim. Next step: close serial monitor, deliberately enter manual ROM download mode (hold BOOT, tap RESET/EN, release BOOT), and read live 8192-byte OTA partition from `0xE000` with esptool `--before no-reset --after no-reset --no-stub` into new local private file. Decode both sequence, image state (0=NEW,1=PENDING_VERIFY,2=VALID,3=INVALID,4=ABORTED,0xFFFFFFFF=UNDEFINED), CRC with local Python. Do not overwrite boot selection or flash app while diagnosing. Recovery image locally backed up and write/recovery rehearsed successfully earlier.


### Receiver source review: missing OTA acknowledgement

The isolated receiver boots, passes its CRC5 self-test, and configures UART1 on GPIO17. After its first boot, App1 OTA metadata was observed as ABORTED while original App0 remained available. The local C source has no OTA image-validation call and main/CMakeLists.txt lists only the esp_driver_uart dependency. Planned minimal fix: add app_update dependency and, after successful self-test and UART setup, inspect running OTA state and call esp_ota_mark_app_valid_cancel_rollback only when PENDING_VERIFY. Also inspect/enable the rollback configuration in the lab app build, independent of the original preserved bootloader. No new flash write before rebuilt image and recovery checks.


### Lab source patched; rollback Kconfig still disabled (2026-10-25)

User ran a guarded local patch of the isolated ESP32-S3 receiver C file and its component CMakeLists.txt. Confirmed results: original source backups created, guarded OTA image-confirmation code added after the existing CRC5 self-test and UART startup, and `app_update` added to component dependencies. No compilation or flashing yet. Local `sdkconfig` inspection showed `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE` not enabled. Next step: open `idf.py menuconfig` and enable Bootloader config > Application Rollback > Enable app rollback support, then validate `sdkconfig` and rebuild. Only app1 application image should be written later; original bootloader remains untouched.


### ESP-IDF v6.1 lab rollback setting saved

User confirmed in Windows PowerShell: `sdkconfig:684:CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y` for the isolated experimental BM1370 receiver lab. This config was saved after the source/CMake OTA-ack patch (backup copies of both original files exist). The original ESP32-S3 bootloader already proved rollback through an App1 NEW -> ABORTED transition; lab-side `app_update` is now enabled to allow calling `esp_ota_mark_app_valid_cancel_rollback` after a successful CRC5 self-test and UART configuration, if the running OTA state is PENDING_VERIFY. Next: preserve known-good flashed firmware binary (195888 bytes; SHA-256 14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE) to a private new backup filename, rebuild with `idf.py build`, inspect ESP32-S3 application image hash and partition fit. No flash or OTA write until rebuilt image verified. Use application-only write at App1 0x150000 later, never `idf.py flash` as that would update original bootloader/partition table.


### OTA-confirming receiver rebuilt and validated (2026-10-25)

Isolated ESP32-S3 BM1370 receiver application with guarded app-valid acknowledgement after CRC5 self-test and UART initialization was built with ESP-IDF v6.1 after adding `app_update` dependency and enabling rollback support in local `sdkconfig`. `esptool image-info` reported an ESP32-S3 app, six segments, 8 MB DIO 80 MHz header, checksum 0x87 VALID, image validation hash `f9de02687baf5f508bd5563a0663c54fcd0ccf85ee88cae530de6da8e5183793` VALID. File size 217856 bytes, SHA-256 `91393A2C6721E14DA06414474C2532F73F8691693C274984272B68F79F84250B`. The build script reported `QHAP OTA-CONFIRMING RECEIVER BUILD VERIFIED PASS`; prior 195888-byte firmware was verified and backed up locally before the rebuild. The rebuilt firmware fits OTA App1. **Nothing has been reflashed yet.** App1 on COM7 still contains older successfully booted but unconfirmed firmware; its OTA state was ABORTED after rollback, with original App0 selection intact. Next perform carefully gated APP1-only write of the newly built 217856-byte binary at 0x150000, staying in manual ROM download mode, then independent byte-for-byte readback and controlled OTA re-arming. Do not replace the original bootloader/partition table, App0, or mining devices, and do not expose private flash backups.


### Receiver App1 update verified by esptool

Experimental UART receiver application rebuilt with self-test-gated OTA confirmation was installed at App1 offset 0x150000. esptool reported 217856 bytes written, hash verified, and staying in bootloader. Original App0 and boot selector were not targeted. Independent readback and first boot validation remain pending.


### Second App1 activation staged and verified

User successfully reactivated the receiver's OTA App1 slot by writing the prepared sequence 2 NEW activation sector. Post-write readback of the 8KB OTA metadata partition confirmed original App0 selection unchanged and the App1 sector exactly matched the prepared image. Reported checks: `ORIGINAL APP0 OTA SECTOR PRESERVED: PASS`, `NEW APP1 OTA SECTOR BYTE MATCH: PASS`, `FULL OTA READBACK VERIFIED: PASS`. The receiver has not yet restarted; its confirmed running state is still pending. Next: open no-reset serial monitor on COM7 and press EN once (without BOOT), capture CRC5 self-test plus `QHAP OTA APP1 CONFIRMED VALID`, then independently check OTA metadata state VALID.


### OTA App1 confirmed VALID across restarts

The corrected isolated receiver booted as App1, printed `BM1370 CRC5 SELF-TEST PASS` and `QHAP OTA APP1 CONFIRMED VALID`. After subsequent resets it booted the same receiver and reported `OTA state=2; confirmation not required`, demonstrating persistent VALID state. Heartbeats continued with valid=0/rejected=0 (Pico GPIO not wired yet). Serial output also revealed two unexpected POWERON resets, after about 110 seconds and then about 10 seconds. Root cause of resets remains unknown and should be investigated before UART integration. A flash-size mismatch warning (physical 8MB vs binary header 4MB reported at startup) remains visible despite 8MB new app image-info; do not treat as proven reset cause. No new firmware writes recommended until power/reset stability is assessed.
