#!/usr/bin/env python3
"""QHAP hypothetical coupled mining equations: zero mining/device access.
All voltage, thermal, timing, error, reliability coefficients are synthetic.
DO NOT treat search results as safe or validated settings for a Bitaxe.
"""
from math import exp

H0, P0, TC0, VR0 = 1.337e12, 24.5, 68.9, 79.0
DIFFICULTY, NET_BTC = 140577680051919.59, 3.125 * .98
CASES = {
 'optimistic': (.010, 15., 2.1, .00020, .10, .18, .9),
 'nominal': (.014, 35., 2.25, .00055, .35, .30, 1.25),
 'conservative': (.021, 85., 2.5, .0015, .90, .55, 1.8),
}
KB = 8.617333262145e-5
R_TH = (68.9-30)/20

def model(clock, voltage, par):
    leak_slope, timing_k, timing_exp, error_temp, error_timing, activation, vrm_w = par
    t = TC0
    for _ in range(100):
        pdyn = 16 * clock * voltage * voltage
        pleak = 4 * voltage * exp(min(20, leak_slope * (t-TC0)))
        tnew = 30 + R_TH * (pdyn+pleak)
        if abs(t-tnew)<1e-10: break
        t = .55*t + .45*tnew
    else: raise RuntimeError('Temperature equation did not converge')
    t = tnew
    watt = pdyn + pleak + 4.5
    tvr = VR0 + vrm_w * (watt - P0)
    m = max(0, clock / (voltage ** timing_exp)-1.01)
    timing = exp(-timing_k * m*m)
    err = min(.999, .002 + error_temp*max(0,t-TC0)**2 + error_timing*m*m)
    stress_chip = exp(activation/KB*(1/(TC0+273.15)-1/(t+273.15)))
    stress_vrm = exp(activation/KB*(1/(VR0+273.15)-1/(tvr+273.15)))
    uptime = 1/(1 + (1/.995-1)*(stress_chip+stress_vrm)/2)
    useful = H0 * clock * timing * (1-err) * .997 * uptime
    return dict(useful=useful, watts=watt, chip_c=t, vrm_c=tvr, j_per_th=watt/(useful/1e12))

bases = {name: model(1,1,params) for name,params in CASES.items()}
for b in bases.values():
    assert abs(b['watts']-P0)<1e-8 and abs(b['chip_c']-TC0)<1e-8
candidates=[]
for fc in range(85,111):
    for vv in range(95,108):
        f, v = fc/100, vv/100
        m = {name:model(f,v,p) for name,p in CASES.items()}
        feasible = all(s['chip_c']<=TC0+3 and s['vrm_c']<=VR0+3
                       and s['watts']<=P0+3 for s in m.values())
        if feasible:
            gains = {k:m[k]['useful']/bases[k]['useful'] for k in CASES}
            energy = {k:bases[k]['j_per_th']/m[k]['j_per_th'] for k in CASES}
            candidates.append((f,v,gains,energy))
assert len(candidates)==255
performance = max(candidates,key=lambda x:min(x[2].values()))
efficiency = max(candidates,key=lambda x:min(x[3].values()))
assert (performance[0],performance[1])==(1.04,1.01)
assert (efficiency[0],efficiency[1])==(.9,.95)
need = DIFFICULTY*2**32/(86400*NET_BTC)
print('QHAP COUPLED MODEL VERIFIED: 338 combos; 255 hypothetical feasible')
print('Hypothetical performance optimum',performance)
print('Hypothetical efficiency optimum',efficiency)
print('1 BTC/day effective hashrate target (EH/s):',need/1e18)
print('Historical Bitaxe multiples required:',need/H0)
print('Nothing sent to mining devices. Model cannot prescribe hardware settings.')
