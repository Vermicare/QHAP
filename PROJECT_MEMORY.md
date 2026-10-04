# QHAP Project Memory

> **Purpose:** Persistent engineering context for QHAP so future work can resume without reconstructing the project from chat history.
>
> **Maintenance rule:** Update this file whenever a validated milestone, measured performance result, release/tag, architecture decision, development-environment change, known validation boundary, or active resume point changes.
>
> **Security rule:** Never store Wi-Fi passwords, wallet seed phrases, private keys, API keys, or other secrets here. The configured Bitcoin worker/address may exist in local configuration but is intentionally omitted from this document.

Last consolidated: **2026-10-04**

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
