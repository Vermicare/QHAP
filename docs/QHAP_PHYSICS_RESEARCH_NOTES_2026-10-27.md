# QHAP Physics-First Research Notes — 27 October 2026

**Purpose:** Evaluate multidisciplinary ways to increase measurable, consensus-valid Bitcoin hashing productivity. These are scientific hypotheses and research plans, not demonstrated discoveries.

## Mathematics
Under the normal cryptographic uniformity assumption, each independent Bitcoin block-header SHA256d attempt succeeds with probability approximately 1 / (network difficulty × 2^32). Over time t, P(at least one result) ≈ 1 - exp(-effective_hashes_per_second × t / (difficulty × 2^32)). Higher real hashing work, uptime, and fewer redundant/stale candidates improve odds; changing how random numbers are selected does not confer a bias.

## Physical formulas
- Switching electricity in digital circuits: dynamic_power ≈ activity × capacitance × voltage^2 × frequency. This motivates careful energy/performance characterization, not automatic overclocking.
- Thermal steady state: chip_temperature ≈ ambient_temperature + power_dissipated × thermal_resistance. Thermal reliability, leakage, and cooling overhead matter.
- Useful mining efficiency: joules_per_terahash = electrical_power_watts / terahashes_per_second. Historical Bitaxe snapshot: 24.5 W / 1.337 TH/s ≈ 18.3 J/TH.
- Thermodynamic logical-bit erasure: energy minimum per erased bit k_B × temperature × ln(2). Not a bound for one entire SHA256d hash. Reversible computing requires different hardware.
- Physical quantum search requires expensive reliable quantum circuits: theoretical query speedup is not practical ASIC replacement.

## Research streams and next tests
1. **Read-only power/thermal analysis:** Raspberry Pi collects timestamped AxeOS temperature, fan, wattage, frequency, hashrate, errors, uptime. Establish baseline before proposing any clock or voltage changes.
2. **Reliability / signal physics:** Separately validate the Pico H synthetic BM1370 frame stream with the experimental ESP32-S3 UART receiver and logic analyzer. Avoid modifying the independently mining Bitaxe.
3. **AI with hard constraints:** Model effective hashrate and joules per valid hash; test predictions against held-out real telemetry. No unsafe automatic hardware controls.
4. **Computing architecture:** Research SHA256d datapath designs, logic efficiency, pipelining, energy accounting; verify every computed hash against a trusted reference.
5. **Frontier physics:** Reversible/adiabatic, photonic, spintronic and quantum computing are theoretical lanes requiring actual hardware demonstrations and energy/time comparisons.

**Safety/evidence:** Keep original stock miner and original app images isolated; do not conflate a valid mocked UART frame with a mined Bitcoin block; do not call a simulation a real measured result. User specifically wants large breakthroughs; advance any promising candidate only with independent reproducible benchmarks.

**Current next checkpoint:** The experimental ESP32 OTA receiver is confirmed VALID across deliberate resets. Pico-to-ESP32 UART physical connection and transmitted frame validation remain next, separate from read-only Bitaxe telemetry.

## External references
- Bitcoin developer guide: https://developer.bitcoin.org/devguide/block_chain.html
- CMOS dynamic power: https://docs.amd.com/api/khub/documents/7KECH4F2hgvQrFavb20mVg/content
- Thermodynamic erasure bound: https://physics.aps.org/articles/v11/49
- 2026 quantum resource assessment: https://journals.aps.org/prxquantum/abstract/10.1103/j3xf-bw18
- AxeOS source: https://github.com/bitaxeorg/ESP-Miner
