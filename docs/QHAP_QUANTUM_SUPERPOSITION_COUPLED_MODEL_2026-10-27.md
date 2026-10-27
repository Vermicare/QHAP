# QHAP: Coupled physics + quantum superposition simulation
**Date:** 27 October 2026. **Evidence:** Offline numerical experiment; no QPU, no board changes, no BTC mined and no real mining advantage measured.

## 1. Integration across scientific disciplines

The prior [coupled physics research](QHAP_COUPLED_PHYSICS_MINING_FORMULAS_2026-10-27.md) links semiconductor switching-power, temperature-dependent leakage, nonlinear thermal equilibrium, voltage/clock timing margin, computing error rate, device reliability, stale work, effective hashrate and solo-block probability. Across 338 hypothetical clock/voltage pairs and three assumed coefficient sets, 255 passed illustrative restrictions; theoretical robust optimization improved useful hashes/second only **2.88–3.78%** (nominal **3.51%**). These equations are **uncalibrated** from actual Bitaxe telemetry and do not establish physically safe settings.

New quantum branch:
- Equal quantum superposition: |s> = (1/sqrt(N)) sum_x |x>.
- Correct proof-of-work oracle predicate: f(x)=1 if SHA256(SHA256(header(x))) is below the network target.
- Grover phase marking and diffusion create interference. Ideal success probability after r iterations: **p_G(r)=sin²((2r+1)asin(sqrt(M/N)))**, where N is the set of eligible headers and M marked solutions.
- An explicitly simplified per-round noise model mixes p_G with classical M/N according to coherence retention (1-epsilon)^r. A hypothetical job-validity factor exp(-r*tau_oracle/L) reflects a mean block-change lifetime L; mining pool work can expire earlier.
- Hybrid expected reward combines actual effective classical hashes with physically realizable quantum solution arrivals. Since **the owned Bitaxe Gamma 601, Raspberry Pi 5, ESP32-S3 and Pico H contain no fault-tolerant quantum computer**, the quantum contribution on current hardware is **zero**, regardless of software amplitude simulations.

## 2. Executed toy SHA256d statevector experiment

A Python experiment calculated **16,384 actual double SHA-256 hashes** for distinct 80-byte Bitcoin-style headers. The intentionally easy 8-bit toy target yielded **61 marked states** (initial probability **0.372314%**). The algorithm then *classically precomputed* this toy predicate and simulated amplitude evolution for 12 Grover iterations. This validates the quantum search mathematics, NOT a real physically computed reversible SHA256d oracle and NOT Bitcoin network mining.

| Grover iterations | Ideal success probability |
| ---: | ---: |
| 0 | 0.3723% |
| 1 | 3.3176% |
| 2 | 9.0335% |
| 4 | 27.2756% |
| 8 | 74.1943% |
| 12 | **99.8029%** |

The statevector result matched the Grover closed form at every checked checkpoint to better than **2e-12**. As a toy sensitivity model, 12-round probabilities under 1%, 5% and 20% coherence loss per iteration were **88.51%, 54.10%, and 7.21%**, respectively. These are assumed conditions, not actual quantum processor measurements.

## 3. Full Bitcoin difficulty and resources

With the prior frozen difficulty **140,577,680,051,919.59**, per-candidate block probability is **1.65624 × 10^-24** (~1 in 6.04 × 10^23). Grover high-probability search in an **adequately large** candidate space would need around **610.3 billion sequential oracle rounds**. A nonce-only 32-bit search space is not large enough; additional consensus-valid header/coinbase variation must be represented reversibly and kept current.

- Even a fictional complete error-corrected oracle round of **one microsecond** implies **~7.06 days** for 610.3 billion rounds, far exceeding usual mining job relevance.
- In an unrealistically favorable one-search-per-fixed-600-second-window model, **1 BTC/day expected subsidy less an assumed 2% fee** requires ~**18.51 billion oracle rounds/window**. That demands **32.42 nanoseconds or less per entire reversible, fault-tolerant SHA256d oracle+amplification iteration**, *before* extra overhead, reliability losses, job refreshes and electrical power are considered.
- Retaining 90% coherence across that many iterations, in the crude independent-error sensitivity model, requires an effective failure rate per round below ~**5.69 × 10^-12**. Nothing in the available hardware achieves this.
- In that same unphysical model, a perfect oracle at 30ns/round would produce ~1.17 BTC/day **mathematical expectation** and 1us/round ~0.00105 BTC/day. These are **counterfactuals and must never be presented as achievable QHAP performance**.
- Difficulty adjusts; massive changes in network hashrate invalidate fixed-difficulty extrapolations. The block subsidy is discrete: long-run expected BTC/day is not guaranteed daily income.

## 4. Research decisions and preservation

1. Treat Grover amplification as a valid **theoretical** quantum phenomenon, rather than a novel invention or a way for classical ESP32/ASIC hardware to try all nonces simultaneously.
2. Do **not** change working Bitaxe controls based on the earlier uncalibrated physical optimizer; its historical fan reached 100% and VRM temperature ~79 C.
3. Continue safe, read-only AxeOS telemetry, validate the Pico H → isolated ESP32-S3 synthetic BM1370 UART experiment, and measure physical computing energy/hash.
4. Any future quantum-lane design must budget reversible SHA256d circuit depth, logical/physical qubits, fault tolerance, total wall time, work freshness, joules, and independent Bitcoin-target validation.

## Reproducible artifacts

- [Standalone GitHub toy statevector and full-difficulty calculator](../research/qhap_quantum_superposition_sim.py) — Python standard library, entirely offline.
- [Earlier linked physical optimization reproducer](../research/qhap_coupled_formulas_reproducer.py) — Python standard library, entirely offline.
- The associated ChatGPT session also generated downloadable full coupled Monte Carlo/NumPy script, JSON/CSV numerical results and a longer Markdown report.

## Scientific references

- IBM Quantum, [optimal Grover iteration and amplitude formula](https://quantum.cloud.ibm.com/learning/en/courses/fundamentals-of-quantum-algorithms/grover-algorithm/number-of-iterations).
- IBM Quantum, [practical limitations of oracle circuits](https://quantum.cloud.ibm.com/docs/en/tutorials/grovers-algorithm).
- Babbush and colleagues, [PRX Quantum 7, 031001, published 21 August 2026](https://journals.aps.org/prxquantum/abstract/10.1103/j3xf-bw18), explaining why error correction and quantum search parallelization make mining impractical in the foreseeable future.

**Bottom line:** The coherent quantum toy example succeeds mathematically; there is no present physical route from the user's owned hardware to a quantum-accelerated 1 BTC/day mining system.
