# QHAP: coupled physics + mining probability model (2026-10-27)

**Status:** Executed offline mathematical simulation. There is **no discovered cryptographic shortcut, no proof of a real improvement, and no achieved 1 BTC/day**. The original working Bitaxe, ESP32-S3, Raspberry Pi 5 and Pico H were **not connected, changed or flashed** during this experiment.

## Hypothesis and baseline

Combine laws from semiconductor switching power, temperature-dependent leakage, thermal impedance, voltage-frequency timing, failure-rate reliability, stale-work accounting, and proof-of-work probability. Optimize **effective valid and independent SHA256d attempts per second**, not nominal device clocks.

Historical Gamma 601 snapshot (not live): 1.337 TH/s, 24.5 W, 68.9 C ASIC, 79 C regulator, fan 100%. Reference Bitcoin difficulty used: 140,577,680,051,919.59. Block subsidy 3.125 BTC; 2% solo pool fee assumed, transaction fees excluded.

**Critical limitations:** All ambient, per-circuit power splits, thermal impedance, voltage-frequency timing, error, uptime and rejected/stale rate coefficients below are **assumed**, not fitted from the user's Bitaxe. This is a model-discovery exercise, not actionable tuning advice.

## Interdependent equations

Let c=f/f0 (clock ratio), v=V/V0 (voltage ratio), T chip temperature, Tvr regulator temperature. All coefficient values below are hypothetical.

1. Raw rate: **Hraw = 1.337 TH/s × c**.
2. Dynamic circuit power: **Pdyn = 16 W × c × v²**, reflecting the physical relation Pdynamic ~ alpha × C × V² × f.
3. Temperature-dependent leakage: **Pleak = 4 W × v × exp[gamma × (T - 68.9 C)]**.
4. Heat-power feedback (solve implicitly): **T = 30 C + 1.945 C/W × (Pdyn + Pleak(T))**; board power is **Pwall = Pdyn + Pleak + 4.5 W**. The ambient and thermal resistance were assumed to match only the historical baseline.
5. Estimated voltage-dependent timing validity: **eta = exp[-k × max(0,c/v^b-1.01)^2]**. k,b are uncalibrated sensitivity terms.
6. Hypothetical ASIC computation error rate: **e = 0.002 + aT × max(0,T-68.9)^2 + am × max(0,c/v^b-1.01)^2**.
7. Lifetime/uptime link: relative Arrhenius thermal stress **A(T) = exp[Ea/kB × (1/Tbase,K - 1/TK)]**; **uptime = 1/[1 + (1/0.995 - 1) × (A(Tchip)+A(Tvr))/2]**. Not an empirically measured Bitaxe failure rate.
8. Actual useful work: **Heff = Hraw × eta × (1-e) × (1-stale) × uptime** (synthetic stale fraction = 0.003). The historical observed speed may already contain some of these effects. Relative modeled gains use the *same* synthetic baseline in numerator and denominator.
9. Block probability: **P(at least one block in t) = 1 - exp[-integral Heff(t) / (D(t) × 2^32) dt]** (usual difficulty conversion approximation).
10. Expected subsidy-only proceeds: **E[BTC/day] = 86400 × Heff × 3.125 × 0.98 / (D × 2^32)**. Actual solo mining earns block-sized payouts, not smooth daily fractional BTC.
11. Robust AI-style decision: maximize **min_across_uncertain_models[Heff(c,v)/Heff(1,1)]** subject to thermal/power restrictions. Efficiency is separately measured as **Pwall / (Heff/10^12)** in joules/terahash.

## Reproducible simulation results

A grid of **338 hypothetical combinations** of clock ratios 0.85 through 1.10 and voltage ratios 0.95 through 1.07, each in increments of 0.01, was evaluated under optimistic/nominal/conservative timing, leakage, errors and thermal reliability assumptions. **255 combinations** met example restrictions (chip no more than +3 C, regulator no more than +3 C, board power no more than +3 W relative to previous readings). **Those are not electrical or temperature safety limits endorsed by the manufacturer.**

The *hypothetical* robust candidate for maximum valid hashes/sec uses normalized c=1.04, v=1.01, yielding modeled gains:

| Coefficient case | Effective rate relative to its baseline |
| --- | ---: |
| Optimistic | **+3.78%** |
| Nominal | **+3.51%** |
| Conservative | **+2.88%** |

Nominal model: 25.64 W, chip 71.12 C, regulator 80.43 C; predicted 1.3701 TH/s useful, compared to 1.3237 TH/s at synthetic baseline. Efficiency worsens from 18.51 J/TH to 18.71 J/TH, despite increased chances. **These normalized ratios are not voltage/clock recommendations to apply to a real device.**

A different, *hypothetical* minimum-J/TH candidate uses c=0.90, v=0.95, yielding about **4.80–5.35% better joules/TH**, but around **10% worse blocks/day** from reduced useful work. Thus, minimizing electricity and maximizing the chance of a Bitcoin block are competing objectives.

**Interdependence check:** nominal +4% clock by itself gave +0.62% effective hashes; nominal +1% voltage alone gave -0.04%; jointly they yielded +3.51%. Their nonlinear interaction occurs entirely through assumed timing/thermal/error equations. This demonstrates nonlinear coupling, **not an empirical finding about actual BM1370 voltage characteristics**.

## Does the model bridge the one-BTC/day target?

**No.** With unchanged reference difficulty and subsidy-only 2%-fee reward model, expected 1 BTC/day requires about **2.282 EH/s**, or **1,706,691 times** the historical 1.337 TH/s baseline. Even the modeled 2.88% gain leaves a gap of ~1.66 million times. A larger network hashrate will also affect future Bitcoin difficulty.

A valid Bitcoin block requires a correct SHA256d hash below the current network target. These engineering formulas can only improve valid candidate *production* and uptime; they **cannot make an individual candidate's hash more likely to win** under the usual unpredictability assumption.

## Required evidence before any actual optimization

1. Gather 24–48 h **read-only** AxeOS history (hashrate, actual frequency, power, chip/regulator temperature, error percentage, uptime, shares, timestamps). Omit credentials and pool worker/address from the uploaded data.
2. Measure ambient temperature; obtain **board-specific** manufacturer electrical/thermal operating limits. Existing fan previously at 100%; do not automatically increase the ASIC's power.
3. Fit voltage, frequency and temperature response using validated measurements rather than synthetic coefficients. Estimate uncertainty using held-out observations.
4. Only with explicit user approval and safety gates consider reversible small tuning experiments, independently comparing **valid effective hashes/day** and **joules per TH** to stock settings.
5. Continue separate Pico H->experimental ESP32 UART tests and deeper logic/circuit architecture research without disturbing the working Bitaxe.

## Reproduce

Run `python research/qhap_coupled_formulas_reproducer.py` from this branch. It uses the Python standard library, makes no network or hardware calls, checks baseline reproduction, and reports performance/energy optima. The longer full numerical solver with structured JSON results and detailed report were also produced as downloadable artifacts in the original chat.

## References

- Bitcoin proof of work and difficulty: https://developer.bitcoin.org/devguide/block_chain.html
- AxeOS read-only system information schema: https://github.com/bitaxeorg/ESP-Miner/blob/master/main/http_server/openapi.yaml
- CMOS switching power and leakage model: https://digicoll.lib.berkeley.edu/record/135559/files/ERL-02-6.pdf
- Thermal reliability Arrhenius examples: https://toshiba.semicon-storage.com/content/dam/toshiba-ss-v3/master/en/semiconductor/knowledge/reliability/reliability-handbook-tdsc-en.pdf
