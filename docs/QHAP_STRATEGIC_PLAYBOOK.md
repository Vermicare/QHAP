# QHAP Strategic Engineering Playbook — Chess-Mindset Rule

**Status:** User-requested project-wide operating doctrine, adopted 2026-10-25.
**Applies to:** Assistant/Codex, manual hardware work, firmware, scripts, Raspberry Pi services, Bitaxe BM1370 integration, Pico H, experimental ESP32 boards, documentation, and future automation.

## The user's standing instruction

> "Be very careful and strategically with chess mindset in this project — rethink whatever we are doing."

Interpret this as **plan several moves ahead, check assumptions before committing, preserve winning positions, anticipate failure and counter-moves, and measure progress toward QHAP's actual goal**. It is not a request to play chess or to add chess-themed features.

QHAP must be developed as a disciplined engineering program, not an improvised sequence of gadget tests. A successful individual command is not necessarily a useful strategic move.

## Second standing instruction: chase the breakthrough

> "We are going for the big win — not for small improvements — we must think in every scientific fields to improve our mining."

**This is a project-wide mandate alongside the chess mindset, not a replacement for safety or scientific rigor.** Search systematically across mathematics, cryptography, physical computing, semiconductor and computer architecture, photonics, quantum information, thermodynamics, materials, power electronics, AI and distributed systems. Seek plausible 10x/100x+ leaps in **real Bitcoin-valid** hashing capability, efficiency or economically scalable capacity; recognize these magnitudes as goals, not promises or demonstrated outcomes.

A minor tweak is worth doing only when it protects a possible winner, maintains a reliable baseline, resolves a safety gate or unlocks a higher-upside experiment. Do not let incremental tuning, peripheral hookup, dashboards or test-count inflation displace moonshot research.

The authoritative research portfolio, screening criteria and evolving hypothesis list are:
- [Breakthrough Research Program](../research/BREAKTHROUGH_RESEARCH_PROGRAM.md)
- [Breakthrough Candidate Register](../research/BREAKTHROUGH_CANDIDATES.md)

**Guardrail:** Every proposed discovery must still output the exact Bitcoin consensus SHA256d proof. A new PoW function, easy-target shares, simulation, a toy quantum circuit or “AI-predicted winning nonce” does not count. Use independent proofs and measurements, and protect all running production miners. A bold idea is not a validated result.

## Mission / what counts as progress

Increase the probability that **real, consensus-valid Bitcoin hashing work** finds a winning block and that QHAP **does not lose a winner** due to job mapping, error handling, submission failure, stale work, crashes, resets, power failure or unsafe deployment.

Compare proposed work against one or more concrete outcomes:

- Correctness and block-safe share/candidate handling.
- More *useful independently verified* hashes per second, or larger reproducible effective search coverage with no duplicate work.
- Better measured hashes/joule, cooling headroom, uptime, job-switch latency, network resilience or recovery.
- Validated prerequisites for independent BM1370 control or multi-ASIC operation.

A new board connected, a test count rising, a new dashboard, or higher displayed raw hashrate **does not, by itself**, establish a higher probability of winning or an engineering performance advance.

## Chess mindset: required decision process

Before a meaningful engineering change, write a short decision record answering:

1. **Position:** What is physically connected, running, verified and at risk *right now*? Identify exact board/port, branch/commit, firmware, power and network roles rather than assuming them.
2. **Objective:** Which measurable bottleneck, risk or dependency does this move address? What alternative moves exist?
3. **Look-ahead:** What must work for the next **2–3 moves**? Which future integration does this enable or obstruct?
4. **Failure analysis:** How might this break production hashing, corrupt files, introduce duplicate nonces, damage hardware, leak credentials or conceal a failed test?
5. **Safety boundary:** Can the work run isolated, read-only, host-only, simulated or on a challenger board before touching the production miner?
6. **Validation gate:** What output, independent reference, pass/fail metric and comparison baseline would distinguish success from wishful thinking?
7. **Undo/escape:** What is the tested rollback, backup, recovery access and known-good configuration? If no safe undo exists, reduce scope or postpone the move.
8. **Decision:** Proceed, defer, redesign, or stop; explain *why*. Do not claim completion until observed evidence is checked.

Do not spend time chasing a neat but low-value experiment when a higher-value correctness or safety prerequisite is unresolved.

## Position protection / non-negotiable invariants

- Keep the ESP32-S3 **reference live miner** autonomous and running; do not reflash, inject synthetic packets, reset, change Wi-Fi credentials or power cycle it merely for a lab test.
- Keep the **Bitaxe Gamma 601 / BM1370 on stock AxeOS** mining independently. Currently the Pi retrieves read-only HTTP telemetry. Stock AxeOS telemetry **is not QHAP-operated physical ASIC mining**. No speculative UART/pin connections or firmware replacement on this production board.
- Physical Pico H BM1370-format response experiments use a **dedicated non-production ESP32**, separate USB power, common ground and verified **3.3 V logic**, after pinout/polarity checks. Never connect power rails together or apply 5 V to GPIO. Synthetic results never go to a public pool.
- Preserve Bitcoin SHA256d, header construction, endianness, targets, work identity and pool semantics. No fake accepted shares, false profitability, or unvalidated quantum speedups.
- Keep correctness ahead of tuning. Reject duplicate/stale job mappings and verify network-valid block candidates independently before they can be submitted.
- Enforce electrical/thermal envelopes outside any optimizer. No automatic firmware flashing, frequency/voltage increases, power changes, or production deployment as a by-product of a monitoring or research script.
- Protect secrets and private machine configuration. Never commit Wi-Fi passwords, pool credentials, keys, full private logs, backup archives or tokens.

## Risk tiers and promotion gates

| Move class | Examples | Required gate |
|---|---|---|
| **Observe** | Read-only API, serial capture, host tests | Correct device/source; no writes to miners; bounded read size/duration; sanitized output |
| **Sandbox** | Pico simulator, mock Stratum, new challenger firmware | Isolated/non-production board; correctness vectors; repeatable host tests; clear stop conditions |
| **Control experiment** | Electrical UART, job dispatch, power/thermal tuning | Exact schematic/pinout and voltage verification, safety limits, independent SHA and target verification, recovery plan |
| **Production change** | Reflash reference ESP32, replace AxeOS, new mining hot path | Known-good image/fingerprint, restorable backup, explicit deployment plan/approval, acceptance and block-safe checks, thermal/uptime observation, rollback and documented release evidence |
| **Unattended operations** | Scheduled Pi logging, automatic trials, autonomous evolution | Verified storage health, backup/restore exercise, reliable boot/power/network, bounded writes and logs, watchdog/stop gates, alerting and safety limits |

A passing host test is **host-valid**, not hardware-valid. A read-only Bitaxe API response is **telemetry-valid**, not QHAP ASIC-control-valid. Use explicit evidence labels instead of collapsing these states.

## Current position and next moves (2026-10-25)

**Known position:**
- Raspberry Pi 5 8GB with SC1148 active cooler and case; USB serial from the stable ESP32 and LAN HTTP access to Bitaxe 601 have been demonstrated.
- Recent Pi software tests: **22/22 passed**; a three-sample combined live probe captured Bitaxe and ESP32 data with output in RAM.
- The Bitaxe stock miner reported roughly 1.3 TH/s during prior samples; ESP32's live *inner-loop* scans were roughly 268 kH/s. These are **different metrics**, not additive claims of validated useful wall-clock throughput.
- Raspberry Pi microSD previously had extensive EXT4 inconsistencies. Current successful boots and clean startup log screening **do not replace offline filesystem validation**; unattended long-running writes/experiments remain blocked.
- Pico H/RP2040 ROM BOOTSEL drive was identified as USB `2e8a:0003`, `RPI-RP2`, UF2 Bootloader v3.0; `INFO_UF2.TXT` reported `Model: Raspberry Pi RP2` and `Board-ID: RPI-RP2`. It was mounted read-only at `/run/qhap-pico`. **This verifies bootloader identity, not firmware installation or functional MicroPython UART output.**

**Strategic sequence:**
1. Record and preserve the validated current position and recoverability. Clear Pi microSD offline integrity/backup gates before unattended work.
2. Complete the missing **ESP32 wall-clock and independent target-check** baseline, while keeping the stock miners productive.
3. Independently prepare the Pico H, then establish controlled synthetic BM1370 result experiments **only** with a non-production ESP32.
4. Validate result checksums, exact job ID/version mapping, stale/duplicate protection, reconstructed SHA256d and network/pool target classification with deterministic vectors.
5. Only then investigate real ASIC communication behind strict electrical/firmware isolation; compare useful effective hashes, errors, uptime, power and thermal metrics to stock AxeOS.
6. Introduce champion/challenger optimization only after the above safety and measurement gates. Expand to multi-ASIC if measured benefit outweighs integration and reliability costs.

## Workflow for every future chat/session

- Resume from the latest confirmed repository state, test evidence and physical inventory; **never infer a current connection or firmware from a historic plan**.
- Prefer one exact manual command at a time; inspect its output before the next command. If work can be completed safely through GitHub, commit it there and verify the exact diff/commit rather than delegating rote editing.
- Before *any* risky hardware, storage, network, firmware or mining-control change, explain purpose, impact, stop/rollback and proof of success.
- Separate **planned**, **implemented**, **host-tested**, **hardware-observed**, **pool-confirmed**, **production-validated**, and **unknown**.
- Record milestone evidence, surprising failures, assumptions invalidated, impact on the roadmap, and the **next safe step** in PROJECT_MEMORY.md; update ROADMAP.md if priorities changed.
- Do not repeat completed work or silently declare stalled/untested work successful. Do not spend several steps polishing documentation when a critical risk remains unaddressed.
- The assistant should do as much safe analysis, test preparation and documentation as possible without unnecessary user interaction, but must not bypass hardware safety or deployment consent.

**Standing maxim:** Protect the working position, think multiple moves ahead, never risk a production winner on an untested hypothesis, and promote only what independent evidence justifies.
