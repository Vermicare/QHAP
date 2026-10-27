# QHAP Phase III — Advance 1000 research checkpoint (27 October 2026)

**Classification:** Offline executed statistical simulation and exact host CPU hash benchmark. **No live device accessed, reflashed, tuned, rewired, restarted, or altered. No SHA256d shortcut or real ASIC uplift demonstrated.**

## Scope and reproducibility

This builds on [Advance-1000 phase II](./QHAP_ADVANCE1000_2026-10-27.md) and the [QHAP engineering handoff](./QHAP_NEW_CHAT_HANDOFF_2026-10-27.md).

- Evaluated **1,024** distinct mining-share gap detection policies (4^5 parameter combinations) on **17,800** synthetic contexts: training 2,200, validation 2,600, untouched holdout 6,500, adversarial holdout 6,500. RNG seed **2026102703**.
- Analytically modeled arrival of *pool difficulty shares* as a Poisson process, under deliberately noisy hashrate/difficulty estimates and uncertain failover availability. This is **not** network block discovery or newly earned Bitcoin.
- Separately **compiled and ran exact SHA256d CPU implementations** with GCC -O2, GCC -O3 and Clang -O3: 3 approaches × 4 interleaved timed passes × 160,000 headers × 3 builds = **5,760,000 timed full double-SHA-256 hashes**.
- Benchmarked baseline OpenSSL SHA256d and two **known midstate caching** alternatives. All cached variants matched full OpenSSL reference on 2,000 different nonces **per build**.

## Observed CPU microbenchmark on host AMD EPYC

| Compiler | Standard H/s | Cached 64 H/s | Cached 76 H/s | Cached64 speedup |
|---|---:|---:|---:|---:|
| GCC -O2 | 1,625,018 | 3,033,783 | 3,010,480 | 1.867x |
| GCC -O3 | 1,557,042 | 2,901,615 | 2,905,253 | 1.864x |
| Clang -O3 | 1,634,143 | 3,020,871 | 3,026,866 | 1.849x |

**Important:** midstate caching is standard, not an invention. Dedicated BM1370 ASICs are designed for this problem and may already perform equivalent reuse. This is a reproducible **host-CPU** benchmark, **not a Bitaxe or ESP32 speed increase**.

## Pool-share anomaly detection (simulated; not deployed)

For assumed assigned pool difficulty `D_pool` and observed hashrate `H`:

`lambda = H / (D_pool * 2^32)`, expected share gap `1/lambda`, and expected gap alerts/day for threshold `t` about `86400*lambda*exp(-lambda*t)`.

Selected policy index **683** on train/validation; hypothetical parameters: false alert target 0.033/day, threshold factor 1.5, minimum 45 seconds, maximum 2400 seconds, estimated rate caution factor 1.25. **Not an approved live detector.**

| Scenario | Policy false alerts/day | Fixed 600-sec threshold false alerts/day | Policy median outage detection delay | Fraction synthetic outage time *potentially* recovered if valid failover works |
|---|---:|---:|---:|---:|
| Training | 0.0148 | 5.883 | 445 s | 35.9% |
| Validation | 0.0234 | 5.814 | 430 s | 28.1% |
| Independent holdout | **0.768** | 5.795 | 455 s | 13.1% |
| Adversarial | **6.032** | 5.662 | 463 s | 2.25% |

**RESULT: NOT DEPLOYABLE.** The selected detector violated its <1/30-days false-alarm objective by >23x on an unseen rate/difficulty shift and >180x on more severe adversarial data. Even highly optimized synthetic policies cannot be assumed safe from the selection dataset.

### Analytical bound / proof obligation

If share rate is completely unknown, the maximum expected long-gap alarms/day for a fixed silence threshold `t`, optimizing over any positive Poisson share rate `lambda`, equals `86400/(e*t)` (at `lambda=1/t`). A universal 600-second threshold has a **52.97/day** worst-case false alarm expectation, despite being safe for some assigned difficulties.

Consequently no fast, low-false-alarm detector can be guaranteed using *share silence alone* while refusing to establish a reliable current share rate; add independent connection-state evidence and abstain when estimates are stale.

### Realistic mining benefit bound

At any baseline useful utilization `u`, fixing losses alone cannot exceed `1/u` times the old effective work. To gain **1000x from loss recovery**, utilization would have to be <=**0.1%** and recovery would have to be virtually perfect. Merely restoring 1 hour of real downtime daily would yield at most **+4.35%** valid opportunities, assuming equal hashrate otherwise.

As before, user hardware's last validated hashrate **1.337 TH/s** and fixed illustration difficulty **140,577,680,051,919.59** do not imply live measurements. No credible 1,000× new ASIC improvement or protocol-level shortcut has been discovered.

## New proposal for *next* phase: QHAP-CONCORD, observation-only

1. Read AxeOS `poolDifficulty`, real accepted/rejected shares, local hashrate and uptime, pool connection/fallback state, error percentages, chip and regulator temperatures without changing settings.
2. Gate alerts on independently corroborated evidence; don't treat correlated telemetry as independent.
3. Refuse automated failover, downclocking, pool changes or mining resets until qualified by physically measured data and a reversible isolated test.
4. Research **bit-exact** circuit transformation on host/synthesis hardware with independent reference tests; neither nonce permutations nor lossy SHA filters count as acceleration.
5. Repair/check Raspberry Pi microSD offline before unattended logging; preserve original Bitaxe and production QHAP ESP32.

### Reproducer provenance

The Python simulator `qhap_advance1000_phase3.py`, C benchmark `qhap_phase3_exact_sha256d_bench.c`, three compiler results, full 1,024-grid CSV, machine-readable JSON, SHA-256 manifest and a detailed report were packaged as the ChatGPT conversation artifact `QHAP_ADVANCE1000_PHASE3_BUNDLE.zip`. The bundle's 12 SHA-256 manifest entries were verified and the ZIP passed integrity checks **in the host environment**; this GitHub checkpoint does *not* claim those sources were independently executed in GitHub Actions.

Sources for context: [Bitcoin developer mining guide](https://developer.bitcoin.org/devguide/mining.html); [AxeOS monitoring guide](https://bitaxe.de/en/bitaxe-monitoring).

**Status:** Keep researching. No live mining change approved on these results.
