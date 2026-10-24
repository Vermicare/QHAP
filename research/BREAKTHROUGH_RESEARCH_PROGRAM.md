# QHAP — Breakthrough Research Program (Moonshot Mandate)

**Status:** Research strategy, not a claim of discovered acceleration. Adopted 2026-10-25 on the user's explicit instruction.
**Companion policy:** [Chess-Mindset Strategic Engineering Playbook](../docs/QHAP_STRATEGIC_PLAYBOOK.md).
**Scope:** The broad scientific research portfolio for increasing Bitcoin block discovery capability. Applies across future conversations and agents.

## User's standing directive

> "We are going for the big win — not for small improvements — we must think in every scientific fields to improve our mining."

Interpret literally as **prioritize the search for order-of-magnitude breakthroughs by exploring cross-disciplinary scientific hypotheses**, not spending the entire project squeezing modest percentages from an ESP32 or tweaking a fan. Be bold in *research*, rigorous in *validation*, and conservative when touching productive hardware.

Do not misconstrue this as permission to violate Bitcoin consensus rules, make unsupported claims, run unsafe experiments, deploy untested firmware, or spend money/cloud credits without a business case. Small corrections that preserve winning-block submission, uptime, thermal limits, or enable a breakthrough experiment are essential supporting work; they are not the end goal.

## Definition of the big win

Primary outcome: **increase expected successfully submitted, Bitcoin-consensus-valid blocks per elapsed time**, constrained by hardware cost, power, uptime, safety and availability.

Secondary outcomes: orders-of-magnitude better **useful verified hashes/s per currency**, **useful verified hashes/joule**, **scalable independent search coverage** without duplicates, **reduced cost to scale**, or a **scientifically demonstrated algorithmic/hardware advantage** with a plausible real Bitcoin deployment path.

A 10x, 100x, 1000x+ goal is a **research target**, not a forecast. At equal difficulty, independent valid hash attempts increase block-found probability proportionally for small probabilities; no device can make a valid trial intrinsically more likely without a change to the required target function. Do not confuse sample hashrate, simulated hashrate, lower *pool* share difficulty, block luck, expected rewards and actual BTC received.

Measure:
- Effective block-valid work rate: independently verified unique block-header hash attempts per wall second (or calibrated ASIC effective rate with error/stale bounds).
- Probability estimate tied to current network target and actual effective rate; show assumptions and uncertainty.
- Actual block/submit-path correctness and robustness.
- ASIC power, temperature, uptime, reliability and procurement cost, plus gross energy economics.
- Total opportunity cost, risk to existing production miners, and ability to reproduce results.

## Scientific fields / opportunity map

Every field is eligible for *screening*, but only mechanisms producing **the exact existing Bitcoin SHA256d proof of work** qualify for near-term production. Do not mistake a new proof-of-work proposal for a shortcut valid on Bitcoin today.

| Discipline | Moonshot hypothesis to test | Reality / first decisive experiment |
|---|---|---|
| Cryptanalysis / mathematics | A provable attack or structure in double-SHA256 reduces *expected exact-target search work* | Start literature review and toy reduced-round models; require independent full-strength proof before claiming any advantage. **No practical full-strength shortcut is established.** |
| Computer architecture | Unconventional SHA256 pipelines or reusable subexpressions yield large effective throughput gains | Verify exact output vectors; calculate area/time/energy lower bounds and compare to modern ASICs, not just ESP32. |
| VLSI / microelectronics | A custom or modular SHA core architecture improves effective hashes per dollar/joule at scale | FPGA proof before custom chip; consider fabrication/NRE, clocks, voltage, yields, cooling and supply chain. |
| Packaging, interconnect, 3D integration | Shorter physical data paths and higher density unlock substantially better ASIC scale/energy | Benchmark credible silicon/thermal models; not an automatic advantage. |
| Reversible / adiabatic computation | Reduced switching dissipation at practical throughput | Evaluate complete SHA256d implementation including clocking and error costs; theoretical thermodynamic potential is not a deployed miner. |
| Photonics / optical computing | Photonic primitives accelerate SHA256-specific Boolean operations or lower energy | Identify exact Boolean operations and conversion overhead; **optical-proof-of-work alternatives requiring changed PoW are not valid Bitcoin mining today**. |
| Quantum information | A Grover-type Bitcoin PoW oracle has net advantage after fault-tolerance and parallelization costs | Reversible full SHA256d oracle + qubit/depth/error correction + physical-clock budget. Current analyses find no near-term ASIC-beating implementation. |
| Neuromorphic / analog / unconventional physics | Novel substrates execute exact digital SHA256d faster/cheaper | Digital accuracy, Boolean fidelity and readout overhead are first gates; approximate hashes are useless. |
| AI / automated design | Hardware/software co-design discovers unexpectedly better validated hashing architectures | Search architecture choices with independent SHA vectors and cost models; no ability to “predict good nonces” from training. |
| Compiler, scheduling, FPGA/GPU research | Better utilization or heterogeneity reveals a previously missed architectural bottleneck | Measure *useful* throughput against ASIC baseline and power—not small microcontroller benchmark-only gains. |
| Distributed systems / networking | Scaling ASIC fleets with non-overlapping work and no lost winners | Per-device job identity, extranonce allocation, submit critical path, stale-share accounting and reconnection validation. |
| Cryogenics, thermal science and materials | A new cooling or physical design opens a higher clock/voltage *safe energy-efficient* operating region | Test joules/hash **including cooling power**; fan changes alone are not breakthrough mining claims. |
| Electrical engineering / power electronics | Lower conversion losses and robust supply enable better sustained fleet hash/watt | Account for full wall power, regulator losses, ripple, cost and failures; no unsafe overvoltage. |
| Reliability and storage engineering | Block-safe persistence and recovery materially increase *realized* success rate | Fault injection against **synthetic locally generated candidates**, never production pool fake shares. |
| Operations research and economics | A non-obvious acquisition, deployment, scheduling, thermal/power or hardware-sharing strategy delivers order-of-magnitude more useful hashes per budget | Model capital, power, maintenance, shipping, availability, noise, failure and pool/block variance; prove feasibility before purchases. |
| Information theory / complexity / statistical mechanics | Derive useful bounds to rule in/out speculative architectures and prioritize investments | Independent math derivation, falsifiable prediction and measured test; not an assertion that randomness is learnable. |
| Materials / nanotechnology / superconducting logic | Exotic computing technologies offer a long-horizon major switch-cost improvement | Obtain credible devices, operating envelopes, cryogenic overhead and whole-system SHA throughput estimates. |
| Biology / chemical computing / other fields | Scout rare new computing substrates without assuming relevance | Admit only proposals with a **concrete exact-SHA256d mechanism**, reproducible computation and competitive systems economics; otherwise archive as speculative. |

Fields that fail a Bitcoin-compatible deployment test may still be valuable for academic research, but belong in a separate long-horizon track; never advertise their performance as Bitcoin hashes.

## Research portfolio: bold upside with controlled risk

Run two tracks in parallel, with a non-negotiable safety gate:

**Track A — Production-capable scale (highest confidence path to a big win):**
- Preserve live stock Bitaxe Gamma 601 and ESP32 miner, protect winning-result submission.
- Evaluate verified ASIC fleet scaling, power efficiency and architecture; prioritize effective hashes per total cost and consistent uptime.
- Use Raspberry Pi solely for supervisory telemetry/experiment coordination until block-safe control is independently proven.
- Develop physical BM1370 ASIC control only on a recovery-capable sacrificial/experimental setup, not by interrupting the sole production Bitaxe.

**Track B — Scientific moonshots (high upside, uncertain outcomes):**
- Reproduce and challenge published work in full-strength SHA256d cryptanalysis, architecture, FPGA designs, reversible computation, photonics and quantum resource estimates.
- Investigate novel **combinations**: hardware-generator architectures with AI design-space search; multi-device work allocation with correctness verification; silicon thermal/electrical co-design.
- Rapidly reject dead ends using cheap analytic proofs or open-source simulators before purchasing specialized equipment or engaging paid cloud services.
- Explicitly preserve negative results and theoretical limits so future sessions do not repeat them.

**Risk-control support track:** storage health, device recovery, pinouts, independent Bitcoin vectors, test harnesses, project memory and power safety. Essential enablers, not the main invention objective.

## How to select moonshot experiments — mandatory breakthrough filter

Every proposed experiment must enter a candidate register with:

1. **Mechanism:** Exactly *what* physical, mathematical or architectural effect might reduce cost or increase valid hash attempts?
2. **Validity:** Does it compute the exact consensus SHA256d PoW on a complete 80-byte header with correct byte ordering and target comparison? If no, label it non-Bitcoin-compatible.
3. **Novelty:** What is already documented, and what part is *actually new*? Distinguish reproducing public work from an invention.
4. **Upside estimate:** Plausible order-of-magnitude gain for hashes/s, hashes/joule or validated fleet scale, with an explicit baseline and assumptions.
5. **Disproof test:** Cheapest decisive analytic/host/simulation test that could falsify the claim before hardware spending.
6. **Future moves:** What two or three follow-on validations/integrations would this unlock; what are their costs and barriers?
7. **Safety:** Which boards remain untouched; voltage, GPIO, thermal, network, file/storage and secret constraints; stop/rollback.
8. **Evidence:** Reproducible files, code, hashes, measurements, experimental controls, negative results and limitations.
9. **Decision:** Reject, archive, iterate or promote *only after independent correctness and scientific evidence*.

Use a portfolio ranking by **plausible upside × scientific credibility × path to demonstration / cost × implementation/recovery risk**. Avoid numerical certainty when uncertainty is extreme. An exciting story is not a passed gate.

## Red lines and misconceptions

- Bitcoin nodes require the exact double-SHA256 proof on the current header. A faster new PoW algorithm **would require consensus change**; it is not a deployable acceleration on the existing Bitcoin network.
- Candidate header hashes are effectively unpredictable. Never claim AI can forecast winning nonces or infer a secret mining formula without independent, convincing full-strength evidence.
- Reduced-round SHA256 attacks, quantum toy circuits, theoretical speedups, approximate digital/analog hashes, simulated hashrate and shares at an easy pool target are **not** equivalent to winning a Bitcoin block.
- New hardware or extra boards do not by themselves yield a scientific breakthrough. Cost-effective parallel ASIC deployment can still produce a very large *system-level* increase in block probability.
- We never risk losing a valid block because an experiment compromised the working ASIC/ESP32, wallet, pool path or storage.

## Current position (2026-10-25)

Hardware: Raspberry Pi 5 + SC1148, working USB-linked ESP32-S3 SHA miner, stock Bitaxe Gamma 601 reporting ~1.3 TH/s, Raspberry Pi Pico H in confirmed RP2040 BOOTSEL, other ESP32 boards planned as dedicated challengers. 22 host-side software tests passed and bounded Pi live monitoring was physically observed. Direct physical QHAP ASIC work injection is **not yet implemented or validated**. MicroSD EXT4 history requires offline repair/integrity proof before unattended operation.

**Next research deliverable:** a ranked, evidence-backed `BREAKTHROUGH_CANDIDATES.md` hypothesis register with a first tranche of low-cost analytic/simulation tests for a small set of high-upside scientific mechanisms. **Next hardware gate** remains careful Pico H setup and a non-production ESP32 BM1370 protocol test; no uncontrolled production change.

## Source anchors (verify revisions and full text before using in quantitative claims)

- Bitcoin developer mining guide: https://developer.bitcoin.org/devguide/mining.html
- Bitcoin developer consensus/blockchain overview: https://bitcoindevelopers.org/docs/developer-guides/block-chain/
- PRX Quantum 2026 resource analysis, quantum PoW constraints: https://journals.aps.org/prxquantum/abstract/10.1103/j3xf-bw18
- BIP 52 (optical PoW proposal, **not current Bitcoin PoW**, marked closed in 2026): https://bitcoin.org/bip/52/
