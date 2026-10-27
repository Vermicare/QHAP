#!/usr/bin/env python3
"""QHAP: offline toy Bitcoin SHA256d Grover amplitude experiment.
Not physical quantum hardware, not a quantum SHA oracle implementation,
not real Bitcoin mining, and not a recommended ASIC configuration.
Standard Python only. No network calls or hardware writes.
"""
import hashlib
import math

N = 1 << 14
D = 140577680051919.59      # frozen 2026-10-27 research snapshot
REWARD = 3.125 * .98          # illustrative subsidy less assumed pool fee
BASE_HASHRATE = 1.337e12     # historical Gamma 601 snapshot
PREFIX = bytes(range(76))

def double_sha(header80):
    assert len(header80) == 80
    return hashlib.sha256(hashlib.sha256(header80).digest()).digest()

# Mark toy targets based on actual Bitcoin-style 80-byte header hashing.
mask = [int.from_bytes(double_sha(PREFIX + n.to_bytes(4, "little")), "big")
        < 2**248 for n in range(N)]
M = sum(mask)
assert 0 < M < N

# Simulate amplitudes, NOT quantum hardware. The lookup table is
# classically precomputed using every SHA256d evaluation; no quantum speedup
# is obtained in the emulator's execution time.
amplitude = [1 / math.sqrt(N)] * N
theta = math.asin(math.sqrt(M / N))
rounds = round(math.pi / (4 * theta) - .5)
readings = {}
for r in range(rounds + 1):
    prob = sum(a*a for a, chosen in zip(amplitude, mask) if chosen)
    formula = math.sin((2*r + 1)*theta)**2
    assert abs(prob - formula) < 2e-12
    readings[r] = prob
    if r != rounds:
        for idx, chosen in enumerate(mask):
            if chosen: amplitude[idx] = -amplitude[idx]
        mean = sum(amplitude) / N
        amplitude = [2*mean - a for a in amplitude]

print("TOY SHA256D ORACLE: TRUE MARKED", M, "OF", N)
print("TOY GROVER ITERATIONS:", rounds)
print("IDEAL TOY P:", round(readings[rounds], 10))
for eps in [0.0,.01,.05,.20]:
    coherence = (1-eps)**rounds
    p = coherence*readings[rounds] + (1-coherence)*M/N
    print("TOY P AFTER", round(eps*100), "PERCENT ERROR PER ROUND:", round(p,8))

# Full-target extrapolation is a counterfactual mathematical bound, *not*
# a physical quantum SHA256d oracle. Pool jobs can change before 600 seconds.
p_hash = 1/(D*2**32)
r_opt = round(math.pi/4*math.sqrt(1/p_hash))
p_day = 1 / (REWARD * 144) # successes per hypothetical 600s window
r_need = math.ceil((math.asin(math.sqrt(p_day))
                    /math.asin(math.sqrt(p_hash))-1)/2)
max_oracle_s = 600/r_need
print("FULL DIFFICULTY P/HASH:", p_hash)
print("FULL DIFFICULTY THEORETICAL GROVER ROUNDS:", r_opt)
print("FICTIONAL ONE BTC/DAY ORACLE SECONDS:", max_oracle_s)
print("FICTIONAL COHERENCE ERROR/ROUND FOR 90% RETENTION:",
      -math.log(.9)/r_need)
print("CLASSICAL BASELINE EXPECTED BTC/DAY:",
      86400*BASE_HASHRATE*p_hash*REWARD)
print("OWNED HARDWARE HAS NO QUANTUM CO-PROCESSOR: ZERO QUANTUM CONTRIBUTION")
print("ALL TESTS PASSED: SIMULATED QUANTUM AMPLITUDES ONLY")
