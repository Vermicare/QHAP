# QHAP Breakthrough Candidate Register — v0

**Created:** 2026-10-25. Hypotheses only; no scientific mining breakthrough is claimed.
**Strategy:** [Breakthrough Research Program](BREAKTHROUGH_RESEARCH_PROGRAM.md). The user wants **big wins**, including ideas across all scientific disciplines, rather than a project limited to 2–5% micro-optimizations. Real Bitcoin consensus and chess-mindset safety gates apply to all candidates.

## Screening criteria

Favor research that can potentially produce an **order-of-magnitude gain in valid effective hashing, energy efficiency, scale or win-capture reliability**, subject to realistic hardware cost and time. These are hypotheses for exploration, not quantitative predictions. First seek *cheap falsification* through math, simulations, source review and an independent known-good SHA256d oracle.

Status vocabulary: **proposed / literature-scoped / simulated / independently verified / hardware-tested / Bitcoin-valid / rejected**. No row is currently beyond **proposed** unless explicitly supported by cited project evidence.

| ID | Field | High-upside question | First low-cost falsification step | Confidence and blocker | State |
|---|---|---|---|---|---|
| M1 | ASIC architecture + scaling | Can we achieve a 10x+ **system-level increase in real effective hashes per budget** through modular ASIC/fleet scaling with independent, non-duplicate work? | Compare measured stock Gamma costs/power with additional independent ASIC strategies; model total capex, power, pool difficulty, thermal and network capacity | **Plausible path to 10x total hash**, but largely hardware acquisition/scaling, not an algorithmic breakthrough; costs may dominate | proposed |
| M2 | Custom digital architecture / FPGA | Is there any full SHA256d datapath, pipeline partition or co-designed control scheme with disruptive `hashes/joule × hardware cost` advantage over ASIC practice? | Independent reference implementation and architecture/energy model; benchmark against **real modern ASICs**, not ESP32 software | High technical risk; fabrication cost and mature ASIC competitors | proposed |
| M3 | Advanced fabrication / new electronics | Could 3D packaging, subthreshold/reversible logic or new interconnect techniques cause a step change in exact Bitcoin hashing energy use? | Survey validated device characteristics; include wiring, memory, cryogenics and cooling overhead in whole-system estimates | Medium-to-very-low near-term feasibility; manufacturing required | proposed |
| M4 | Mathematics / cryptanalysis | Is an independently reproducible **full-strength double-SHA256** search complexity shortcut possible? | Formalize exact target, expected attempts and reduced-round vs full-strength gap; independent mathematical review | **Extremely speculative:** no known practical shortcut; high proof burden | proposed |
| M5 | Quantum computing | Can a reversible SHA256d oracle and error-corrected Grover search outperform real ASICs after wall-clock, parallelization, and power costs? | Compute logical/physical qubits, oracle depth, logical gates per query, correction overhead, wall time; compare to ASIC baseline and recent resource studies | **No credible near-term advantage established**; major fault-tolerant hardware requirements | proposed |
| M6 | Photonics / optical computing | Can physical optical logic calculate **existing exact** SHA256d (not proposed replacement PoW) efficiently after conversion overhead? | Build gate-level Boolean requirements and A/D overhead model; explicitly exclude non-consensus optical PoW proposals | Highly speculative; BIP 52 optical **alternative PoW** was closed, not an implementable Bitcoin shortcut | proposed |
| M7 | AI design-space search / operations research | Can adaptive architecture search uncover radically better **fully correct** RTL/control architectures or hardware cost per valid hash? | Open-source model-driven design search with strict SHA256d test vectors; measure Pareto frontier vs validated baseline | AI can search designs, **not predict winning nonces**; may discover limited gains only | proposed |
| M8 | Distributed systems + reliability | Could lossless job scheduling / persistent winner detection turn a fragile ASIC fleet into a large reliably productive system? | Implement a local simulator of pool disconnects, races, duplicate jobs, stale/result retries and winner persistence; use synthetic work, never fake pool shares | Confidence in operational utility, but does not intrinsically increase hashes per second | proposed |
| M9 | Thermal, electrical, materials | Could substantially improved energy/thermal system design unlock truly better full-wall hashes/joule and ASIC fleet density? | Baseline stock ASIC whole-system wall power and temperatures; model realistic cooler, VRM, fan/room power tradeoffs and hardware limits | Possible practical gains; avoid presenting cooling tweaks as revolutionary | proposed |
| M10 | Nontraditional computing / cross-science scout | Do analog, superconducting, molecular, neuromorphic or other substrates have a credible **exact SHA256d** implementation path? | Require gate-accurate design, full accuracy, conversion/clock cost, scaling and compatibility *before* prototype investment | Very speculative; fail fast if no plausible exact cryptographic computation path | proposed |

## First-ranked investigation sequence

1. **M1: system-level 10x+ scale feasibility:** model what *realistic* cost, energy and number/type of ASICs would be required to lift the current ~1.3 TH/s order-of-magnitude, without implying guaranteed block discovery or spending money before assessment.
2. **M2 + M7: design-space scouting:** determine where current ASIC architecture is already optimal and whether unusual FPGA/RTL structures or AI search could yield a meaningful difference. Require hardware-realistic comparison, not just a simulated clock.
3. **M8: block-safe reliability:** preserve every real chance and establish synthetic fault-injection tests; this is a prerequisite to larger mining systems.
4. **M4/M5/M6/M10: high-risk moonshots:** maintained as a low-cost, evidence-driven literature and theoretical program rather than the default hardware build path.

Each candidate must have owner, baseline, falsification test, expected outcome range, actual result and next decision filled in when work begins. Mark hypotheses as rejected if their cheapest falsification test fails. Archive failure reasons to prevent repeated dead ends.

## Important hard boundary

A **new** proof-of-work function, a “faster alternative hash,” easier pool difficulty, altered block target, a model that guesses valid nonces, or toy/reduced-round quantum result cannot be substituted for the current Bitcoin network's exact SHA256d consensus computation. No accepted pool share or ASIC screen reading is proof of a discovered winning Bitcoin block.

## Immediate hardware status

Raspberry Pi Pico H is in a verified RP2040 BOOTSEL mode; MicroPython not yet installed. ESP32 reference miner and Bitaxe Gamma 601 remain production-isolated; Pi storage offline integrity gate remains open. **Do not change production hardware merely to pursue this register.**
