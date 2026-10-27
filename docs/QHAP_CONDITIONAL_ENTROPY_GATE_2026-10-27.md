# QHAP Conditional Entropy Gate (CEG) — Invented and Tested Candidate Formula
**Date:** 27 October 2026  
**Evidence level:** Actual software SHA256d experiment using artificial easy difficulty. **NOT** validated Bitcoin network mining, ASIC acceleration, or a breakthrough.

## User goal
Discover a novel formula capable of raising the existing QHAP hardware's Bitcoin mining productivity to **1 BTC/day**, not merely optimize a handful of percentage points. This requires careful separation of an invented, falsifiable formula from an actual working cryptographic shortcut. The Bitaxe Gamma 601, Raspberry Pi 5, experimental ESP32-S3 and Pico H were **not modified or accessed** during this software experiment.

## Proposed candidate: Conditional Entropy Gate

Let x be a distinct 80-byte Bitcoin block header and m=SHA256(x), y=SHA256(m). Gate s(x) is the high nibble of the first byte of m. The test considers each of sixteen simple filters G_g(x)=[s(x)=g]. Each gate retains roughly 1/16 of candidate headers.

A conditional signal would mean favorable full SHA256d results y are **statistically enriched** given a cheap gate calculated from the first SHA256. The hypothetical enrichment metric is:

    E(g) = Pr[toy SHA256d target success | G_g(x)] / Pr[toy SHA256d target success]

At equal assumed per-SHA computational costs, projected wins per unit computing cost relative to full double hashing would be:

    Gain(g) = 2*p_g*E(g) / (1+p_g),   p_g=Pr[G_g(x)]=~1/16.

Under the random-oracle-like null, E≈1 and Gain≈0.118 (a massive loss). Even a perfect first-stage filter could only produce an ≈1.882x speed gain under this idealized model, not the ~1.7-million-fold gain required for 1 BTC/day on the current Bitaxe. Practical ASIC throughput also differs from this naive per-SHA cost model.

**Important:** SHA256(SHA256(x)) outputs are already believed to be effectively unpredictable from simple first-digest bit features. No evidence was available that this is a shortcut before testing; this was a speculative, deliberately falsifiable hypothesis, not a cryptographic claim.

## Executed experiment and safeguards
- Implemented real SHA256d with Python hashlib. **Bitcoin genesis block known SHA256d test vector passed.**
- Constructed distinct Bitcoin-format 80-byte headers, varying the 32-bit nonce.
- Tested **2,000,000** actual double hashes, 1,000,000 training and 1,000,000 disjoint hold-out validation.
- Used a **toy target success of one hash in 256** (last byte of the raw double SHA digest equals zero). These successes **do not qualify as real Bitcoin blocks**.
- Evaluated 16 high-nibble gate values on the training data; selected the best apparent enrichment value before looking at hold-out performance.
- Tested the chosen gate again on the independent held-out headers and measured Python CPU toy-target successes/second with and without gated outer SHA256.

| Metric | Training | Holdout |
| --- | ---: | ---: |
| Headers | 1,000,000 | 1,000,000 |
| Total toy-target hits (no gate) | 3,892 | 3,901 |
| Gate selected after training | 0xC | 0xC |
| Candidate headers passing gate | 62,866 | 62,753 |
| Toy-target hits passing gate | 271 | 229 |
| Conditional probability enrichment | **1.1076x** | **0.9355x** |
| Simplified equal-SHA computational efficiency | 0.1310x | 0.1105x |

**Actual host CPU benchmark:** held-out gated toy-target hits/second were **0.0951x the baseline** (approximately 10.5x fewer toy successes per elapsed second). Not measured on Bitaxe ASIC hardware. Apparent training correlation did **not** survive independent verification. **Reject the CEG nibble prefilter as an improvement candidate.**

## Goal magnitude
2026-10-27 Bitcoin network difficulty snapshot: 140,577,680,051,919.59. For historical Bitaxe 601 1.337 TH/s, subsidy 3.125 BTC/block and assumed 2% solo pool fee, the frozen-difficulty expected rewards would be ≈0.0000005859 BTC/day (~58.6 sats/day), NOT a regular solo-mining payout. Expected **1 BTC/day** would require **~1,706,691x** real effective block-winning work (equivalent roughly to 2.282 EH/s at fixed difficulty), which the available boards do not provide. Difficulty would adjust if global hashing capacity increased; at even 1 BTC/day expected solo mining, many individual days would pay nothing.

## Scientific decision
No known physics formula or simple reordering of nonce inputs makes each independent candidate more likely to satisfy Bitcoin's consensus double SHA256 proof of work. We can still invent novel circuit topologies or investigate cryptanalytic shortcuts, but must prove full consensus-correct hashes and time/energy advantage. The rational frontier experiments remain: physics/thermal/electrical power per hash, custom SHA datapath architectures with cost models, correctly bounded AI optimization, read-only Bitaxe telemetry, and Pico-to-experimental-ESP32 UART frame integrity tests. Production Bitaxe software settings must remain untouched without explicitly reviewed controlled tests.

## Local reproducibility
Experiment script: `qhap_formula_experiment.py`. Results CSV: `qhap_entropy_gate_results.csv`. Summary JSON: `qhap_entropy_gate_summary.json`. Report: `qhap_entropy_gate_report.md`. These artifacts were generated in the current ChatGPT sandbox, **not** flashed or run on any of the user's physical hardware, and are **not committed with this markdown note**.

## Reference context
- Bitcoin target validation: https://bitcoindevelopers.org/docs/developer-guides/block-chain/
- Network difficulty (2026-10-27): https://www.coinwarz.com/bitcoin-difficulty
- Existing physics research: [QHAP Physics-First Research Notes](QHAP_PHYSICS_RESEARCH_NOTES_2026-10-27.md)
