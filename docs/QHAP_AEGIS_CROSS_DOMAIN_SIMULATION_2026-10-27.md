# QHAP AEGIS — October 27, 2026 cross-domain simulation checkpoint

**Evidence class:** OFFLINE SIMULATION, NOT HARDWARE OPTIMIZATION
**Branch:** `feature/bm1370-integration`
**Equipment touched:** None. Bitaxe Gamma 601, production ESP32, experimental receiver, Pico, and Raspberry Pi were not accessed or reconfigured.

## Hypotheses and tested innovation

Proposed **QHAP-AEGIS**, a cross-layer mining-opportunity controller:
1. Protect independent valid work with unique job-range handling and stale-work tracking.
2. Recover from mining-pool interruptions using independently verified alternative paths (not actually deployed).
3. Model thermal outage hazard jointly with temperature, chip-error risk, and downclock throughput penalty.
4. Select features only after paired scenario comparison and later read-only miner telemetry calibration.
5. Maintain a separate **cryptanalytic falsification track**, testing purported first-SHA256-hash predictive gates on untouched data rather than equating training-set correlations with breakthroughs.

This is a QHAP-specific proposed architecture combining known engineering mechanisms, **not a discovery of altered Bitcoin probability, an ASIC breakthrough, or a unique cryptographic shortcut**.

## Actual experiment

Host executed deterministic Python simulation (seed `20261027`):
- 120,000 fully synthetic operations trials (30,000 each in healthy, pool-disruption, hot-room, and combined-stress regimes).
- 320,000 **real SHA256(SHA256(header))** evaluations on 80-byte Bitcoin-shaped headers; toy difficulty: final digest begins with zero byte.
- 160,000 examples for training candidate first-hash prefix gate, 80,000 validation, and 80,000 *untouched final test*.
- Static Bitcoin difficulty reference `140577680051919.59`, historic Gamma rate 1.337 TH/s, subsidy 3.125 BTC, **assumed** solo-pool fee 2%. Actual mining data was not freshly collected.

All ambient, pool recovery, latency, power loss, component reliability, thermal response and throttle effectiveness *distributions were invented for sensitivity testing*, not measured or calibrated. Reported 5th–95th percentiles are across selected synthetic inputs, not valid confidence bounds for real mining.

## Simulation outputs (all hypothetical, relative to synthetic baseline)

| Regime | Pool recovery only, median | Full AEGIS, median | Full AEGIS 5th–95th percentile | Fraction of synthetic trials helped |
|---|---:|---:|---:|---:|
| Healthy | +0.368% | +0.431% | +0.206% to +0.779% | 100.0% |
| Pool disruption | +2.924% | +3.161% | +1.621% to +5.570% | 100.0% |
| Hot room | +0.366% | **−0.199%** | −1.648% to +0.627% | **42.0%** |
| Combined stress | +2.917% | +2.178% | +0.122% to +4.794% | 96.3% |

**Critical negative result:** In 58.0% of the made-up hot-room cases, naive thermal throttling reduced valid useful hashes even though it reduced modeled thermal incidents. **Do not change real clocks, voltages, or fan settings from this simulation.**

### Exact-hash cheap gate falsification

Training selected a first-SHA digest prefix bit filter from 192 candidates. It kept about half the candidate headers and their final 8-zero-bit toy winning hashes:
- Train: 353/646 hits retained; theoretical useful toy hits per SHA compression = 65.6% of unfiltered.
- Validation: 148/326; usefulness = 54.5%.
- Untouched test: **155/309; usefulness = 60.2%** of unfiltered; about 40% WORSE.

**Rejected** as an early filter: drops true winners, fails independent uplift, and is not a lossless Bitcoin-mining optimization. Results relate to an idealized SHA compression count, not an actual BM1370 benchmarking pass.

## Bitcoin issuance and odds

Fixed-difficulty formula:
`P(block in time t) = 1 - exp(-H_effective*t/(difficulty*2^32))`.
At historical unobstructed 1.337 TH/s, probability of one block in a year approximately **0.006983%**; model expected **0.000000585929 BTC/day** at assumed subsidy/fee, *not daily solo payouts*.

One BTC/day in **long-run expected subsidy** requires around **2.282 EH/s** at frozen difficulty, around **1,706,691 historical Gamma units**. Actual future difficulty can change and solo payouts are discontinuous. The model found **no shortcut**.

## Follow-up experiment criteria

1. Collect real **read-only** metrics over 24–48 hours from Bitaxe/solo-pool and Pi *only after Pi microSD filesystem health is addressed*. Do not expose miner credentials.
2. Determine whether pool downtime/stales/duplicate work actually occur or are already prevented by stock AxeOS. Avoid assuming an unmeasured failure to manufacture a gain.
3. Replace synthetic thermal and outage curves with fitted coefficients from observed device data and held-out test periods.
4. Keep production Bitaxe, mining ESP32 and both experimental boards separated. Run isolated mock-pool and mock-receiver tests before any intervention.
5. Treat any proposed SHA shortcut as a formal exact-consensus and out-of-sample benchmarking challenge. No early gate may discard potential winning hashes without a losslessness proof.

### Original source references

- [QHAP canonical handoff](./QHAP_NEW_CHAT_HANDOFF_2026-10-27.md)
- [QHAP coupled physics model](./QHAP_COUPLED_PHYSICS_MINING_FORMULAS_2026-10-27.md)
- [Gamma official hardware repository](https://github.com/bitaxeorg/bitaxeGamma)
- [Bitcoin difficulty reference](https://www.coinwarz.com/bitcoin-difficulty)

The full local Python reproducer, machine-readable JSON and 120,000-row CSV were generated as downloadable conversation artifacts for this experiment. Those files are **not yet committed here**; this checkpoint preserves the method and numeric outputs, and should not be represented as independent GitHub Actions validation.
