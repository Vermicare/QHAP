# QHAP BTC-per-second feasibility experiment — 27 October 2026

## Objective and result
User requested a physics/mathematics formula and a simulation aiming to mine 1 BTC every second using existing QHAP hardware. No shortcut formula was established. We executed reproducible simulation code in an isolated Python environment, not on the user's physical miner or Raspberry Pi.

The existing measured physical mining baseline is Bitaxe Gamma 601 at 1.337 TH/s and roughly 24.5 W (historical, not live). The experimental ESP32-S3, Pico H, and Raspberry Pi 5 are not additional useful ASIC-class mining capacity; the ESP32 historical SHA256d rate was ~0.255 MH/s.

Difficulty reference for 2026-10-27: 140,577,680,051,919.59; reward subsidy 3.125 BTC/block; solo-pool scenario fee 2%. Fees from transactions were excluded.

## Formula
For cryptographically uniform candidate SHA256d outputs and fixed network difficulty D, expected hashes per network-valid block ≈ D × 2^32. At effective miner rate H, block-arrival rate λ = H/(D × 2^32); P(at least one block in t) = 1−exp(−λt). Expected solo-pool subsidy reward rate = λ × 3.125 BTC × 0.98.

Computed:
- λ ≈ 2.21439541789e−12 blocks/s.
- Expected reward ≈ 6.7815859673e−12 BTC/s, not an actual recurring payment.
- One-year block probability ≈ 0.0069878563% at constant difficulty; typical mean block wait ≈ 14,319 years.
- To reach 1 BTC/s of *mathematical expected subsidy reward at frozen difficulty* would require about 1.97151522733e23 hashes/s, or **197.15 ZH/s**, ~147.46 billion equivalent 1.337 TH/s Bitaxe miners, and ~3.613 terawatts at historical unit power (before infrastructure). This is not physically achievable using the owned devices or economically plausible.
- This counterfactual hashrate is about 182× the contemporaneous measured global network hashrate (~1.084 ZH/s); the network would retarget difficulty in response, invalidating frozen-difficulty extrapolation.
- At Bitcoin's target 600 seconds per block, the entire network mints only 3.125 / 600 ≈ 0.0052083 newly issued BTC per second, equivalent to 1 BTC per 192 seconds on average. Mining cannot sustainably produce 1 new BTC/s under these consensus rules. Transaction fees redistribute already-issued BTC.

## Executed simulations
1. Bitcoin genesis-block double SHA-256 known vector independently passed.
2. Hashed **1,000,000 distinct 80-byte candidate headers** using Python hashlib SHA256(SHA256(header)), holding genesis fields fixed while changing the nonce, against artificial easier proof-of-work thresholds (not real network mining):
   - 2^−8 threshold: **3892** successes, theory 3906.25
   - 2^−12 threshold: **216** successes, theory 244.14
   - 2^−16 threshold: **14** successes, theory 15.26
   - 2^−20 threshold: **0** successes, theory 0.954
   These demonstrate ordinary random fluctuation around standard probabilities; none represents a new Bitcoin block.
3. Drew **200,000 independent exponential waiting-time samples** at the 1.337 TH/s fixed-difficulty discovery rate (seed 20261027): simulated mean time ~14,319 years, median ~9,913 years, confirming theoretical behavior. Very short rare-event probabilities are estimated analytically, not from zero-hit Monte Carlo bins.

## Physics optimization research
Improving physically realized valid hash work can improve odds proportionally: CMOS switching energy, thermals, leakage/errors, pipeline throughput, midstate/redundancy avoidance, electrical efficiency, and uptime. No known physical formula can bias a consensus-valid SHA256d hash toward the target. Reversible/photonic/quantum architectures require novel hardware and rigorous correct SHA256d time/energy benchmarks. Suggested QHAP next work: safe, read-only AxeOS telemetry and controlled Pico→experimental ESP32 CRC5 frame tests, keeping Bitaxe stock and mining.

## Grounding links
- Difficulty, 2026-10-27: https://www.coinwarz.com/bitcoin-difficulty
- Network hashrate 2026-10-27: https://pro.startmining.io/en/network
- Bitcoin 3.125 BTC/block subsidy / issuance schedule: https://bitcoin.org/en/halving
- Prior research plan: [QHAP Physics Research Notes](QHAP_PHYSICS_RESEARCH_NOTES_2026-10-27.md)
- Receiver resumable checkpoint: [BM1370 Receiver Session Checkpoint](BM1370_RECEIVER_SESSION_CHECKPOINT_2026-10-25.md)

**Evidence level:** numerical simulation (actual scaled SHA256d + stochastic waiting-time model), not a real miner hardware run or breakthrough.


## Follow-up: 1 BTC per day (27 October 2026)

Unlike sustaining 1 new BTC per second, achieving a long-run average of 1 BTC/day is within Bitcoin's aggregate issuance (approximately 450 newly issued BTC/day at its ten-minute target and 3.125 BTC/block subsidy), although entirely infeasible with the existing personal lab hardware.

At difficulty **140,577,680,051,919.59**, current historical Gamma 601 speed **1.337 TH/s**, subsidy **3.125 BTC**, and 2% Solo CKPool fee (**3.0625 BTC per winning block**, excluding transaction fees), statistical expectation is **0.000000585929 BTC/day** or **58.59 satoshis/day**; this is *not* an actual daily solo payout. Approximate daily chance of finding a block is **1 in 5,226,743**. Fixed-difficulty calculation for 1 BTC/day of expected value: **2.28185 EH/s**, roughly **1,706,692** identical Gamma 601 miners or **41.814 MW** using the measured historical 24.5 W/unit (cooling and infrastructure excluded). That is around **0.21% of a reported 1,084 EH/s Bitcoin network hashrate**. Difficulty will retarget; network and hardware metrics change.

Solo reward distributions are bursty: at a mean of 1 BTC/day, expected network-valid blocks/day = 1/3.0625 ≈ **0.32653**. The probability of *at least one* block in a day is only **27.86%**, while **72.14%** of days pay zero. A reproducible Poisson Monte Carlo with seed 20261027 over 200,000 modeled days produced mean 1.00135 BTC/day and **72.1515% zero-payout days**, illustrating why an average is not a guaranteed daily reward. This is a probabilistic model, not live hardware mining or a special SHA256 shortcut.
