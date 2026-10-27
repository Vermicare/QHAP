# QHAP — Complete New-Chat Engineering Handoff

**As-of:** 27 October 2026, India time.  
**Project:** Quantum-Audited Heterogeneous SHA Pipeline (QHAP), Bitcoin solo-mining and experimental mining-systems research.  
**Repository:** https://github.com/Vermicare/QHAP  
**Active research branch:** `feature/bm1370-integration` (not necessarily the repository's default branch).  
**Authoritative long-term record:** [PROJECT_MEMORY.md](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/PROJECT_MEMORY.md).  
**Working rule:** Never infer current physical wiring, live hashrate, temperature or miner connectivity from historical logs. This handoff describes *last verified* conditions.

## 0. Start a new chat with this exact instruction

> Resume my QHAP Bitcoin-mining research from the 27 October 2026 engineering handoff, not from a blank slate. First read the GitHub branch `Vermicare/QHAP`, `feature/bm1370-integration`: `docs/QHAP_NEW_CHAT_HANDOFF_2026-10-27.md`, `PROJECT_MEMORY.md`, `docs/QHAP_STRATEGIC_PLAYBOOK.md`, and the BM1370 25 October receiver checkpoint. Distinguish verified hardware observations, host simulations, uncalibrated predictions, and incomplete tests. Preserve my running Bitaxe and production ESP32 miner. Research big cross-disciplinary improvements and the long-run 1 BTC/day goal, but never claim a nonexistent mining shortcut. Give me one safe baby step at a time, explain the purpose, risks and pass/fail criteria, and update GitHub at real milestones. **Do not ask me to repeat known project history.**

## 1. North-star goals and operating doctrine

- **Ambition:** Search for a way to reach an *expected* **1 BTC per day** or a major improvement in Bitcoin acquisition efficiency. This is a research target, **not** an achieved output, a forecast, or a promise. Bitcoin solo mining pays discontinuously when a whole network-valid block is found; accepted low-difficulty shares do not accumulate fractional rewards with Solo CKPool.
- **Broad scientific streams:** Bitcoin proof-of-work mathematics and cryptanalysis; exact SHA-256d computing; CMOS and power electronics; thermal/semiconductor reliability; ASIC architecture; reversible/adiabatic and photonic concepts; quantum computing; AI-assisted experimentation; system-level scaling and economics.
- **Chess mindset:** Protect working equipment, think several moves ahead, prefer cheap falsification before buying hardware, and have an independent recovery route before dangerous changes. Label results `planned`, `simulated`, `host-verified`, `hardware-observed`, `pool-confirmed`, or `not tested`.
- **Practical working style:** Detailed explanations, full forms of acronyms, very small sequential steps for physical actions, exact PowerShell commands, real output checks (not fabricated PASS labels), no unsolicited reflashing or auto-overclocking. Record reproducible milestones in GitHub. Avoid exposing credentials, private flash binaries or network identifiers.

## 2. Current machines and strict separation

| Component | Purpose | Last confirmed state | Next caution |
|---|---|---|---|
| **Bitaxe Gamma 601, BM1370 ASIC, stock AxeOS** | Only major dedicated real Bitcoin hashing capacity | Independent solo miner; last measured ~1.337 TH/s, 24.5–24.64 W, ASIC ~68.9°C, regulator ~79°C, fan 100%, 4,441 accepted / 0 rejected shares | **No changes made by later mathematical experiments. Live state as of 27 October unknown.** Do not change voltage/clock/firmware without safety bounds and explicit approval. |
| **Original production QHAP ESP32-S3** | Real SHA-256d work from CKPool Stratum jobs | Earlier ~0.255–0.268 MH/s; live job/CRC/hash validation research in GitHub; serial attached to Pi `/dev/ttyACM0` in Oct 24 evidence | Keep separate from experimental board. Pool accepted shares not observed in earlier data. |
| **Experimental ESP32-S3-DevKitC-1-N8R8** | Isolated UART1 BM1370-result *receiver*, not a real ASIC miner | Windows `COM7`, ESP-IDF v6.1 receiver in OTA App1, **VALID after intentional reboots**, 10-second `READY` messages; no Pico UART frame received yet | Do not reflash or re-arm OTA. Preserve original App0 and bootloader. Power state now unknown after planned shutdown. |
| **Raspberry Pi 5, 8GB + Active Cooler** | Lab host, monitoring, potential AI/control research | Raspberry Pi OS boots; previous live, read-only concurrent Bitaxe and ESP32 telemetry PASSED | Earlier microSD EXT4 structural inconsistencies were **not cleared by offline fsck**. Avoid unattended storage-intensive automation until offline integrity/backup checks. |
| **Raspberry Pi Pico H (RP2040)** | Future synthetic BM1370 UART transmitter | MicroPython v1.29.0 booted; Pi-host REPL command passed; on-device isolated CRC5 11-byte vector matched reference | USB connected to Pi in last discussion; **GPIO TX to ESP32 has not been connected/tested**. |
| **8-channel 24 MHz logic analyzer, multimeter, ESD board** | Later signal-integrity work | In inventory; no measured cross-board UART waveforms yet | Use 3.3 V UART logic and correct ground. |

The user also previously reported multiple identical ESP32 boards. **Re-identify each physical board and serial device before touching it**; never assume that the production ESP32 on the Pi and experimental COM7 ESP32 are the same.

## 3. Experimentally validated receiver — precise 25 October checkpoint

Full source: [BM1370 receiver checkpoint](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/BM1370_RECEIVER_SESSION_CHECKPOINT_2026-10-25.md).

- **Local Windows root:** `C:\esp_projects\qhap_bm1370_receiver_lab`; source `main\qhap_bm1370_receiver_lab.c`; build output `build\qhap_bm1370_receiver_lab.bin`; ESP-IDF 6.1.
- ESP32-S3 hardware: v0.2, QFN56, 8 MiB flash and 8 MiB PSRAM; USB-to-UART COM7 (if the Windows port assignment remains the same).
- Receiver: UART1 **RX GPIO17**, **115200 8N1**, receive-only; synthetic 11-byte frames with preamble `AA 55`; CRC5 polynomial `0x05`, initialization `0x1F`, expected CRC residue zero; known-good vector `AA 55 A1 B2 C3 D4 02 5A 12 34 8C`. Its nonce is `0xA1B2C3D4`, raw ID `0x5A`, version `0x1234`.
- Device has no network/pool/ASIC control in this receiver build; a successful synthetic frame is **not mining work**.
- **Original ESP32 bootloader, partition table, App0 preserved.** App1 at `0x150000`, 1280 KiB. OTA metadata `0xE000`–`0xFFFF`; original App0 OTA selector sequence 1 at `0xE000`. Original bootloader reportedly ESP-IDF v5.5.4.
- First receiver build (195,888 bytes, SHA-256 `14019AE2FDCA009AED24CE6F07D174A83D06CA10A33BFC1EC0613644DE10DDCE`) booted and passed CRC5 but failed to confirm the OTA image. After an intentional reset, OTA state became **ABORTED**, and original App0 booted—proving automatic rollback was active.
- C source patched to `#include "esp_ota_ops.h"`; add `app_update` component; saved `sdkconfig` contains `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y`. After self-test and UART initialization, application calls `esp_ota_mark_app_valid_cancel_rollback()` **only** in `ESP_OTA_IMG_PENDING_VERIFY`; all API results checked. Backups of both original source files exist locally under `.before_ota_fix.bak` suffixes.
- **Current corrected App1 firmware**: 217,856 bytes; SHA-256 **`91393A2C6721E14DA06414474C2532F73F8691693C274984272B68F79F84250B`**. Valid checksum, `esptool image-info` header 8MB/DIO/80MHz, and independent flash readback matched compiled file **byte for byte**.
- Re-staged OTA App1 selector at `0xF000` with sequence 2, state NEW and correct CRC. Verified full OTA metadata readback while original App0 selector remained untouched. Boot log then showed **`BM1370 CRC5 SELF-TEST PASS`**, **`QHAP OTA APP1 CONFIRMED VALID`**, repeated `READY valid=0 rejected=0`. Two later **manual RESET/EN presses** produced `OTA state=2; confirmation not required`. The user explicitly confirmed both resets were manual, **not spontaneous crashes**. Final observed continuous heartbeats lasted at least 150 seconds.
- Warning in boot logs: physical flash 8 MB, boot-time app/bootloader image header reports 4 MB; corrected `esptool image-info` reports 8 MB. Nonblocking in verified runs; root cause not established. **Do not change original bootloader to silence this warning without a separate recovery plan.**
- **Hardware current state after user discussed power-down is unknown.** Firmware and validated OTA selection persist after removing power. No more OTA writes or new flash commands needed to resume.

### Recovery and private files — do not upload binary contents

| Item | Local path / fingerprint |
|---|---|
| Reconstructed 8 MiB original entire flash image | `C:\esp_projects\qhap_lab_backups\esp32s3_full_8mb_reconstructed.bin` |
| Copy of same flash image | `D:\QHAP_Recovery_Private\esp32s3_original_8mb_backup.bin` |
| Both full-flash SHA-256 | `4DC4BC1F7E359BD15BAE169E15EC23F1E8953CD002532862C4D0813BD2D9E2C3` |
| Original second OTA sector recovery image | `C:\esp_projects\qhap_lab_backups\qhap_app0_original_f000_RECOVERY.bin` (4096 bytes) |
| Recovery sector SHA-256 | `58B59EB397FCC8B699D265A18AF84E063DC9BAB0BD03632AD2F91E70038AA788` |
| App1 second-activation candidate | `C:\esp_projects\qhap_lab_backups\qhap_app1_ota_candidate_OFFLINE_ONLY.bin` (seq 2, NEW) — **DO NOT APPLY to VALID App1** |
| Verified current App1 post-flash readback | `C:\esp_projects\qhap_lab_backups\app1_ota_fixed_readback.bin` |
| Previously working receiver image | `C:\esp_projects\qhap_lab_backups\receiver_before_ota_confirmation_fix.bin` |

Important recovery limits: original 8 MiB reconstruction came from multiple reads, not a single atomic capture, and was not fully restore-tested. Windows C: and D: backups were reported to reside on **one physical SSD**; they are not independent off-device disaster-recovery copies. Original second-sector OTA recovery **was** tested via a successful ROM-mode write-and-readback rehearsal. Never run `idf.py flash` on the experimental board when intending to preserve its original bootloader and App0; the earlier writes explicitly targeted only App1 and OTA second sector. Routine resume **requires no flash or OTA operations**.

## 4. Pi/Pico lab evidence and next physical UART experiment

On 25 October the Pico H had official-family RP2040 MicroPython running (observed v1.29.0), responded to a serial REPL `6 * 7 = 42`, and executed a temporary on-device synthetic result test emitting `QHAP_PICO_CRC5_PASS aa55a1b2c3d4025a12348c`. This validates frame creation/checksum **inside Pico firmware**; it does *not* prove the GP0 pin transmitted that frame, nor that the ESP32 received one. The Pico UART mock does **not autostart**.

**Next controlled physical wiring, only after double-checking actual board silkscreen and with USB power disconnected:** Pico H **GP0 / physical pin 1 (TX)** -> experimental ESP32 **GPIO17 RX (J1 pin 10 according to previously established mapping)**; Pico H **GND / physical pin 3** -> ESP32 **GND (J1 pin 22 according to previous mapping)**. **Do not connect VBUS/5V/3V3 or other power rails.** Pi powers Pico via USB; Windows laptop powers experimental ESP32 via USB. Production reference ESP32 and Bitaxe stay physically separate.

Then check experimental ESP32 normal boot (with no BOOT button) in IDF PowerShell: `Set-Location "C:\esp_projects\qhap_bm1370_receiver_lab"; idf.py -p COM7 monitor --no-reset`. If startup log was missed, intentional press of EN/RESET is okay on the experimental board. It should log OTA state 2 VALID and READY heartbeat. Only once wiring is audited, use an approved bounded Pico UART transmission at 115200 8N1 to produce a **real observed** `VALID nonce=0xA1B2C3D4` on the ESP32. Follow with an intentionally altered CRC frame, confirm rejected count increments and resynchronization, and if necessary capture UART with the logic analyzer. **No actual ASIC job has yet been dispatched from QHAP over the Pico GPIO path.**

Pi storage prerequisite: on 24 October read-only combined live telemetry was verified, with 22/22 host tests for `tools/qhap_live_probe.py` and tests. Sample readings Bitaxe ~1.294/1.299/1.337 TH/s, temperatures and fan as above, ESP32 serial loop ~268 kH/s. The Pi microSD had earlier extensive EXT4 inconsistencies, and an **offline integrity validation remains outstanding**. Avoid unattended durable writes until checked and backed up. USB `/dev/ttyACM0` was the reference ESP32, `/dev/ttyACM1` was the Pico in last observation, but numerical device names can change: use stable `/dev/serial/by-id` or verified USB IDs.

## 5. Bitcoin probability: historical baseline and fundamental limits

Use the **27 October 2026 reference difficulty** D = **140,577,680,051,919.59** as a **fixed-difficulty illustration**, not a live constant. Historical Bitaxe H = **1.337 × 10^12 hashes/s**, 3.125 BTC/block subsidy, assumed Solo CKPool fee 2%, ignoring transaction fees.

For uniformly distributed SHA256d proof of work:

- Per-independent-header win probability approx `1 / (D * 2^32)`.
- Effective rate `H_effective = valid, timely, unique hashing opportunities per second` (accounting for actual uptime/errors/stales without double-counting metrics).
- At fixed D, `P(at least one block by time t) = 1 - exp(-H_effective * t/(D*2^32))`.
- `Expected BTC/day = H_effective * 86400 * (3.125 * 0.98)/(D * 2^32)` (long-run value, *not* a solo-pool daily payout).

For the historic Bitaxe baseline under those assumptions: **~0.00699% probability of a block in one year**, mean wait **~14,310 years**, expected reward **~0.0000005859 BTC/day** (about **58.6 satoshis/day** as mathematical expectation, not money actually paid daily). Finding a valid solo block would yield around **3.0625 BTC subsidy after the assumed fee**, plus transaction-fee proceeds as applicable; accepted ordinary shares are not fractional BTC payouts.

**1 BTC/day long-run expected value** would require about **2.282 exahashes/second** = 2.282 × 10^18 hashes/s at frozen difficulty, **~1,706,691** identical historical Bitaxe units, and roughly **41.8 megawatts** of their measured electrical draw before cooling/infrastructure. Increased network hashrate would raise future difficulty, undermining any permanent fixed-difficulty extrapolation. Even a miner whose *expected value* is 1 BTC/day would see around **72% of individual solo-mining days pay zero**, because block rewards are discrete.

**1 newly mined BTC/second sustained** is not permitted by current protocol issuance targets: 3.125 BTC average per 600-second block = about 0.005208 new BTC/sec for the entire network, i.e. one new BTC every 192 seconds on average. Fees redistribute existing BTC. No formula can alter consensus rules while still mining actual Bitcoin.

## 6. Research experiments completed on 27 October — evidence and outcomes

All later simulations ran **offline on a host computer**; they did not access Bitaxe/ESP32/Pi/Pico, mine live blocks, or change any firmware. Cross-disciplinary physics models were built with uncalibrated hypothetical coefficients: **none is a validated hardware optimization or proof of a mining shortcut**.

| Experiment | Method and sample | Result | Verdict |
|---|---|---|---|
| **1 BTC/s feasibility** | Real SHA256d on **1,000,000** 80-byte candidate headers with toy targets and 200,000 simulated exponential waiting times | Toy successes followed random expectation; inferred hardware gap enormous | No shortcut. 1 new BTC/s sustained contradicts issuance target. |
| **1 BTC/day feasibility** | Frozen difficulty and 200,000 Poisson-modeled solo-mining days | Needed 2.282 EH/s expected value; ~72.15% modeled zero-payout days | Possible as network-scale expected earnings, not feasible on owned hardware. |
| **Conditional Entropy Gate (invented test formula)** | **2,000,000** full SHA256d hashes; 16 intermediate-hash nibble filters; 1M training and disjoint 1M validation, artificial 1/256 target | Apparent training enrichment 1.108× collapsed to **0.935× validation**; useful toy hits/sec **0.095×** baseline | **Rejected**; no predictive power proved. |
| **Exact SHA256d midstate reuse** | Genesis Bitcoin vector + **2,060** nonce tests and changed-header checks | Reusing invariant first compression block gives nominal **3 -> 2** compression blocks/candidate; host CPU pure Python ~1.55× versus naive | Correct but established mining technique, likely already built into ASIC. |
| **Additional 3-round cache** | Invariant first 3 rounds of second SHA compression plus schedule words; Python and compiled GCC/Clang CPU trials | Only **128 -> 125 variable rounds** in idealized model; seven paired trials median **1.012×**, noisy; compiled GCC -O3 essentially no gain | No reproducible significant extra improvement. |
| **Coupled physics + reliability model** | **338** clock/voltage grid settings, three synthetic thermal/timing/reliability coefficient sets; **255** met *illustrative* limits | Best modeled valid-hash gains **+2.88% to +3.78%**, versus a different minimum-J/TH solution improving efficiency by **4.8% to 5.4%** but reducing hash throughput | **Hypothetical only.** Does not justify applying any voltage/frequency to Bitaxe. |
| **Quantum/Grover toy** | Separate local numerical statevector model, **16,384** real SHA256d toy-header evaluations; 61 toy-marked headers then oracle array; ideal Grover amplitude simulation | High simulated ideal target measurement probability after iterations, but the oracle was **constructed classically** and real hardware quantum throughput remains zero | Illustrates quantum math, not a feasible miner or hardware improvement. |

### Linked formulas and physical constraints

- Dynamic CMOS switching approximation: `P_dyn ≈ activity * capacitance * V^2 * f`.
- Temperature-dependent leakage: `P_leak(T,V) ≈ P0 * (V/V0) * exp(gamma*(T-T0))`.
- Thermal feedback: `T_chip = T_ambient + R_thermal * [P_dyn + P_leak(T_chip)]`.
- Timing efficiency can be modeled as a function of `clock/voltage^b`, but **coefficients are unknown for actual BM1370**.
- Error rate, stale work, uptime and resulting useful mining opportunity interact: `H_eff = H_raw * eta_timing * (1-error) * (1-stale) * uptime`.
- Effective reward and probability equations are above. Physics can improve *rate, cost, or reliability*, not the intrinsic likelihood of success for an individual uniformly distributed SHA256d hash.
- Thermodynamic Landauer bound `k_B*T*ln(2)` applies to erasing an information bit; does **not** directly specify energy required for a full SHA256d hash. Reversible computing, quantum, photonic, materials and cryptanalytic ideas need new theoretical/hardware evidence and independent consensus-hash correctness.

The coupled model's example 'best' ratios **c=1.04 clock, v=1.01 voltage are not safe instructions**: thermal headroom was uncertain and the historical fan already at 100%. Work with real sensor data and manufacturer limits first.

## 7. GitHub source-of-truth and reproducer links

**Key project references on branch `feature/bm1370-integration`:**

1. [PROJECT_MEMORY.md — long detailed chronology](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/PROJECT_MEMORY.md)
2. [QHAP Strategic Engineering Playbook](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_STRATEGIC_PLAYBOOK.md)
3. [Breakthrough Research Program](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/research/BREAKTHROUGH_RESEARCH_PROGRAM.md)
4. [Breakthrough Candidate Register](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/research/BREAKTHROUGH_CANDIDATES.md)
5. [BM1370 UART receiver full recovery checkpoint](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/BM1370_RECEIVER_SESSION_CHECKPOINT_2026-10-25.md)
6. [Physics-first research notes](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_PHYSICS_RESEARCH_NOTES_2026-10-27.md)
7. [One BTC/sec and day feasibility model](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_ONE_BTC_PER_SECOND_FEASIBILITY_2026-10-27.md)
8. [Conditional Entropy Gate negative experiment](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_CONDITIONAL_ENTROPY_GATE_2026-10-27.md)
9. [Exact SHA256d reuse and compiled benchmark](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_EXACT_SHA256D_REUSE_2026-10-27.md)
10. [Coupled physical mining equations](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/docs/QHAP_COUPLED_PHYSICS_MINING_FORMULAS_2026-10-27.md)
11. [Reproducible midstate Python smoke test](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/research/exact_sha256d_midstate_smoke.py)
12. [Coupled model Python reproducer](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/research/qhap_coupled_formulas_reproducer.py)
13. [SHA256d host CPU bench CSV](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/research/exact_sha256d_cpu_bench_2026-10-27.csv)
14. [Pico BM1370 emulator source](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/tools/pico_bm1370_emulator.py)
15. [Read-only Bitaxe/Pi/ESP32 live probe](https://github.com/Vermicare/QHAP/blob/feature/bm1370-integration/tools/qhap_live_probe.py)

**Locally generated downloadable companion files in the 27 October conversation:** `qhap_btc_feasibility_sim.py`, `qhap_formula_experiment.py`, `qhap_exact_sha_experiment.py`, `qhap_sha256d_compiled_benchmark.c`, `qhap_coupled_formula_sim.py`, supporting reports/JSON/CSV, and `qhap_quantum_coupled_sim.py`. They were **generated by ChatGPT**, not executed on the Pi or installed on the user's Windows machine. Only the specific GitHub sources listed above are definitely versioned; future work should commit any additional reproducer after reviewing source and test provenance. The Bitaxe live data was from October 24, not remeasured by these simulations.

## 8. Top outstanding engineering and scientific work

### First physical laboratory task (baby steps, one at a time)

1. Confirm whether experimental ESP32 and Pi/Pico are currently powered off or on; don't presume wiring.
2. Audit Pico H and experimental ESP32 board silkscreen/pin orientation with power removed.
3. Make **two** safe signal connections only (Pico GP0 TX to ESP32 GPIO17 RX; common GND) while USB power is off. Keep power rails separate.
4. Reconnect USB power, confirm App1 receiver remains VALID, and send one bounded Pico H known-good test frame. Require actual ESP32 `VALID nonce=0xA1B2C3D4` logging. Then test rejected CRC and instrument with logic analyzer.
5. Document physical result and exact serial outputs to GitHub. This step is an **ASIC-protocol emulator test**, not Bitaxe control or new Bitcoin mining capacity.

### Parallel science/reliability tasks

- Complete safe, offline **Raspberry Pi microSD filesystem integrity and backup** work before writing experiment databases unattended. Do not merely infer filesystem health from ability to boot.
- Reconfirm current Bitaxe live AxeOS data through already validated **read-only** Pi telemetry tools. Ideally collect 24–48 hours of timestamped hashrate, voltage, fan, ASIC/regulator temperatures, power, errors, accepted/rejected shares and uptime. Avoid secrets and avoid high-rate polling that loads the device.
- Fit the coupled physics/reliability parameters to measured data with held-out validation and uncertainty; never use synthetic optimizer settings as actual overclock recommendations.
- Keep a separate **moonshot** lane for correct SHA256d architecture search, ASIC/FPGA synthesis cost, reversibility/photonic/quantum resource estimates, and cryptanalysis—using cheap simulation and independent test vectors before physical purchases.
- Preserve accepted-share, network-block-candidate, stale/duplicate, reset recovery and miner fleet safety requirements from the existing QHAP Strategic Playbook. Follow prior proven GitHub code/test workflow rather than overwriting production firmware.

## 9. Nonnegotiable limits and common pitfalls

- **No '1 BTC/day' formula was found or demonstrated** as of this handoff. Do not turn hypothetical gains into actual payouts.
- **Midstate reuse is known prior art** and the BM1370 may already use it. Python/GCC benchmarks don't predict gains for a dedicated ASIC.
- **Quantum superposition simulation is not quantum hardware**. Existing RP2040/ESP32/Raspberry Pi/Bitaxe have no large-scale fault-tolerant SHA256d Grover oracle.
- An apparent prediction trained on toy SHA256d data must survive untouched holdout data and full consensus-hash testing. CEG did not.
- **Valid pool shares do not mean earned BTC** in solo mining; a pool share isn't a Bitcoin-network-winning block.
- Raspberry Pi and Pico's prior positive tests do not imply Pico GPIO UART transmit success.
- Do not conflate the **production QHAP ESP32 miner**, the **experimental COM7 receiver**, and the **Bitaxe Gamma 601**.
- The user's ESP32 'POWERON' reset logs were from manual EN presses; do not reopen an unexplained reset investigation without new evidence.
- **Preserve original ESP32 flash recovery images privately**. Avoid uploading binary dumps that may contain stored credentials. Do not assume C and D copies protect against physical SSD loss.
- A numerical improvement in **joules/terahash** does not necessarily increase *block odds per day* if it reduces total useful hashrate. Evaluate both metrics.

## 10. New-chat continuation — immediate answer template

A high-quality next assistant response should begin: "I have the QHAP 27 October handoff. Your experimental ESP32-S3 App1 receiver is verified VALID; your Pico H has passed on-device CRC5 but has not transmitted over GPIO; your Bitaxe Gamma 601 stock miner is separate and remains unmodified. The coupled physical model and SHA256d tests were simulations, not a working 1 BTC/day shortcut. I will preserve your miners. Our first safe practical action is to identify the boards/power state and wire GP0 TX + GND with power off; our parallel research action is to collect read-only telemetry and independently benchmark new architecture ideas." Then proceed one validated step at a time without making the user repeat this history.

---

**End of handoff.** Update this document when new evidence changes the state, rather than appending contradictory unchecked claims. The full chronology remains in `PROJECT_MEMORY.md`; this document is a concise authoritative resume map.