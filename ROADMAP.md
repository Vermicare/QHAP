# QHAP Roadmap

QHAP is a real Bitcoin solo-mining research project built around an ESP32-S3. The roadmap is ordered by what most directly protects or increases the probability that valid Bitcoin work becomes a successfully submitted block.

## Guiding rule

QHAP must never claim simulated hashrate, fake share acceptance, or guaranteed Bitcoin earnings. Every performance or reliability claim should be tied to measured firmware behavior or an explicitly labeled estimate.

## Priority 1 — Block-safe reliability

Before increasing hashrate, make the winning-result path as difficult to lose as possible.

Planned work:

- explicit Bitcoin-network-target check in the mining hot path
- clearly separate ordinary pool-share candidates from network-valid block candidates
- permanent NVS record for any network-valid block candidate
- preserve job ID, nonce, extranonce2, nTime, header, hash, submit state, and pool response
- robust `mining.submit` retry and acknowledgement handling
- stronger socket error and disconnect detection
- stale-job protection
- watchdog/reconnect testing under intentional network interruption
- boot-time recovery/reporting of any persisted block-found record

This priority does not increase raw probability per hash. It reduces the risk of losing an extraordinarily rare valid result after it has been found.

## Priority 2 — Dedicated Bitcoin ASIC integration

The first major probability increase should come from moving SHA256d execution from the ESP32-S3 to a dedicated Bitcoin ASIC while keeping QHAP as the controller.

Target architecture:

```text
CKPool / Stratum
       |
   QHAP ESP32-S3
       |
  job construction
  target management
  nonce/extranonce allocation
       |
 dedicated SHA256d ASIC
       |
 returned candidate nonces
       |
   QHAP validation
       |
   mining.submit
```

Initial research target: BM13xx/BM1370-class hardware or another open, documented Bitcoin ASIC platform that can be controlled from the ESP32-S3.

All claimed performance must be measured on the actual hardware used.

## Priority 3 — Multi-ASIC QHAP

After one ASIC is stable, scale QHAP into a controller for multiple hashing engines.

Planned capabilities:

- deterministic non-overlapping search-space allocation
- extranonce2 partitioning between devices
- per-ASIC job tracking
- per-ASIC nonce/result validation
- temperature, voltage, frequency, power, and error telemetry
- automatic isolation/restart of unhealthy workers
- centralized share/block submission
- fleet-level effective hashrate accounting

The objective is linear or near-linear hashrate scaling without duplicate work.

## Priority 4 — ESP32 and controller optimization

Continue improving the controller and the native ESP32 miner where measurements justify it.

Research areas:

- SHA peripheral DMA experiments
- interleaving and pipeline scheduling
- job precomputation
- lower-copy Stratum parsing
- faster job-switch latency
- task/core allocation
- midstate reuse improvements
- reduced idle time during network/job transitions
- benchmark-driven compiler and memory-layout changes

The ESP32 native miner remains valuable as a reference implementation, validation engine, fallback miner, and research platform even after ASIC integration.

## Priority 5 — Raspberry Pi coordination layer

If QHAP grows beyond one controller, use a Raspberry Pi 5 as a supervisory node rather than as the primary SHA256d engine.

Potential responsibilities:

- QHAP node discovery and fleet management
- CKPool connection health monitoring
- persistent telemetry and logs
- block-found alerts
- automatic restart/recovery orchestration
- thermal and power monitoring
- firmware/release management
- comparison of expected vs measured work
- dashboard and long-term statistics

The Pi should not become a single point of failure for actual hashing or submission if that can be avoided.

## Priority 5B — QHAP Evolution Engine

After the single-BM1370 path is correct and measurable, use the Raspberry Pi 5 as an experiment coordinator that can automatically generate, run, score and retain controller/ASIC strategies.

The optimizer must never alter Bitcoin's consensus SHA256d algorithm. It may explore valid execution and control strategies such as:

- work partitioning and chunk sizes;
- job-switch scheduling;
- controller task/core allocation;
- safe build/compiler variants;
- UART batching and transport behavior;
- per-chip frequency/voltage/thermal settings inside hard safety limits;
- fan/thermal policy;
- heterogeneous ASIC scheduling.

Candidate search methods may include evolutionary search, Bayesian optimization, multi-armed bandits, simulated annealing and champion/challenger testing.

Primary metrics should include **useful verified hashes/s, hashes/joule, stale/duplicate fraction, error rate, temperature stability, job-switch latency and uptime**, rather than raw displayed hashrate alone.

Every candidate must pass a deterministic correctness/safety gate before it can become production-eligible. The first implementation can use the existing Raspberry Pi 5, ESP32-S3 and incoming Gamma 602; extra ESP32 boards are optional later for parallel experiment lanes.

This is a research direction, not yet a demonstrated QHAP performance advantage.

## Priority 6 — Quantum mining research

Quantum work is a research track, not the first practical way to improve current Bitcoin-winning probability.

Research topics:

- nonce superposition
- Grover-style amplitude amplification
- reduced-bit mining oracles
- reversible SHA-family circuits
- hybrid classical/quantum search-space partitioning
- quantum experiment vs classical baseline
- circuit-depth, qubit-count, noise, and runtime scaling
- reversible midstate concepts

Current cloud quantum computers are not expected to outperform dedicated classical Bitcoin ASICs on real SHA256d mining. Any future advantage must be experimentally demonstrated.

## Things not prioritized for probability improvement

Do not spend significant time first on:

- CPU mining
- GPU mining
- changing pools merely to change pools
- lowering pool share difficulty as if it changed block probability
- fabricated or invalid share submission
- altered SHA256d formulas that would no longer satisfy Bitcoin consensus
- claimed quantum speedups without a real reversible oracle and measured hardware results

## Moonshot target ladder

The long-term question is not "can QHAP promise one Bitcoin every second?" Bitcoin's consensus rules, block interval, finite issuance, and global competition make that an invalid engineering promise.

Instead, use progressively harder measurable targets.

### Target A — Never lose a winner

Probability multiplier: none.

Goal: if QHAP finds a network-valid result, persist it, submit it correctly, retry safely when appropriate, and leave enough evidence to prove what happened.

### Target B — 1 TH/s class

Move from hundreds of kH/s to dedicated-ASIC territory.

Goal: millions-fold increase over the native ESP32 miner.

### Target C — 10 TH/s class

Coordinate multiple ASICs or a larger hashing device while preserving QHAP's independent validation and submission path.

### Target D — 100 TH/s class

Begin operating at the scale of a modern full-size Bitcoin miner.

### Target E — 1 PH/s class

Coordinate a small fleet. Reliability, power, cooling, networking, duplicate-work prevention, and uptime become as important as firmware speed.

### Target F — Probability-based block interval goals

For a solo miner, expected block interval is approximately inversely proportional to its share of total network hashrate.

At a network hashrate near the order of 1 ZH/s, rough sustained solo-mining scales are:

- about 1 block per year: several PH/s
- about 1 block per month: tens of PH/s
- about 1 block per day: several EH/s
- about 1 block per hour: on the order of one-sixth of the entire network
- sustained one block per minute is beyond 100% of the existing network at the same difficulty and would trigger difficulty adaptation

These are order-of-magnitude planning figures. QHAP should calculate them dynamically from live difficulty/hashrate before making current estimates.

## Solo payout interpretation

For the current solo.ckpool.org configuration, **expected BTC/time** must not be confused with smooth periodic income.

The miner receives no fractional payout merely for producing ordinary lower-difficulty pool shares. Actual payout is tied to solving a Bitcoin-network-valid block. Therefore the realized payout path is highly discontinuous even when the statistical expectation is well defined.

As of October 2026:

- subsidy: **3.125 BTC/block** plus transaction fees;
- solo.ckpool.org published service fee: **2%**;
- a current-era solved solo block is therefore greater than 1 BTC after the advertised service fee;
- all non-winning work can still result in **0 BTC realized payout**.

When comparing QHAP designs, use both:

```text
expected BTC/time
and
probability distribution / expected waiting time
```

Do not present expected value as if it were a smooth payout stream.

## Bitcoin-per-time interpretation

A block currently carries a 3.125 BTC subsidy plus transaction fees until the next halving. Bitcoin targets one block about every ten minutes across the entire network.

Therefore sustained "1 BTC every second from mining" is not a valid target under the present Bitcoin issuance schedule. Even controlling 100% of network block production would yield roughly the protocol-wide subsidy rate, not 1 BTC/s.

A more meaningful QHAP metric is:

```text
expected BTC/time = expected blocks/time × expected reward/block
```

with all values updated from live network conditions and clearly labeled as probabilistic expectations, never guarantees.

## Current implementation baseline

As of QHAP v0.2.0:

- live Stratum V1 mining works
- real coinbase/Merkle/header construction works
- ESP32-S3 hardware SHA256d is validated against PSA
- safe 32-bit nonce exhaustion is validated
- extranonce2 cycling is validated
- same-job regeneration after nonce exhaustion is validated
- production live rate observed around 268–272 kH/s
- real `mining.submit` path exists
- no qualifying pool share has yet been empirically observed
- intentional reconnect/fault testing remains incomplete

The next engineering milestone is Priority 1: block-safe reliability.
