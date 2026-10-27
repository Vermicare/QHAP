# QHAP exact SHA-256d reuse experiment — 27 October 2026

**State:** Completed reproducible software computation and correctness verification. No physical mining hardware was touched; no cryptographic breakthrough claimed.

## Hypotheses

Bitcoin 80-byte block header requires two SHA-256 compression blocks for first SHA-256, plus one compression block for the second SHA-256. With the first 76 header bytes constant and only the nonce changing, the first compression block can be reused for every nonce, reducing nominal compression-block cost from three to two per candidate: theoretical 1.5x improvement in work, excluding setup and runtime effects. This *midstate* method is well known and likely already used by modern mining ASICs.

Additional candidate: precompute and reuse the first three SHA-256 rounds of the second block, since nonce is the fourth word and does not affect preceding rounds; cache message-schedule words W16 and W17 too. That changes round counts from 128 to 125 variable rounds for the double hash after midstate setup, offering only a theoretical 128/125 = 1.024x upper bound from round reduction (ignoring schedule/register overhead). This is a circuit/design optimization hypothesis, not a new cryptographic attack.

## Actual results

A pure Python SHA-256 compression engine, a reference native hash implementation and an exact midstate-copy implementation were tested. The published Bitcoin genesis block SHA256d vector passed. All four paths matched exactly for 2,060 nonce test cases. Eight independently changed 80-byte header prefixes each matched at five edge nonce values, checking that cached states are refreshed when work changes.

| Local implementation | Hashes/sec | Measured ratio |
| --- | ---: | ---: |
| Native hashlib full double SHA | 1,003,244 | 1.000x native baseline |
| Native hashlib copied midstate | 1,281,169 | 1.277x native baseline |
| Pure Python full double SHA compression | 2,269 | 1.000x Python baseline |
| Pure Python reused midstate | 3,513 | 1.548x Python baseline |
| Pure Python reused midstate + three cached rounds | 3,710 | 1.635x Python baseline |

**Replication:** In seven paired early-round-cache versus midstate trials, median speed ratio was only **1.011x**, ranging **0.941x to 1.068x**. No reliable further improvement is established at this sample size. Local software CPU speed ratios must not be extrapolated to the Bitaxe.

A separate trial computed 50,000 complete double hashes at an *artificial 8-bit target*: **198** hits versus **195.31** expected. Zero candidates were skipped. None of these are actual mined Bitcoin blocks.

## Conclusions and limitations

This experiment demonstrates correct computational reuse but not reduced block-winning difficulty. For a fixed candidate, probability of meeting the valid Bitcoin target is unchanged. Reusing a known SHA midstate may help a naive software design, but does not add a guaranteed 1.5x multiplier to an ASIC that already uses this technique. Even a theoretical improvement on this scale is far below the ~1.7-million-fold effective hashing increase required for a long-run expected 1 BTC/day from the user's historical Bitaxe Gamma 601 speed at the assumed difficulty.

**No Bitaxe firmware modifications, no Raspberry Pi or Pico hardware changes, and no ESP32-S3 flashing were performed.**

## Reproducibility and next steps

An independently runnable smaller correctness/benchmark reproducer was committed at [research/exact_sha256d_midstate_smoke.py](../research/exact_sha256d_midstate_smoke.py). It needs only standard Python and performs no network or device operations.

The full pure-Python 64-round implementation, early-round caching experiment, measurements JSON, CSV and extended report were generated as downloadable artifacts in this ChatGPT session, **but the full script bytes have not yet been uploaded to GitHub**.

Next scientific lane: compiled C comparison, custom SHA datapath timing/energy modeling, and synthesis-cost estimates before spending on FPGA/ASIC hardware. Separate hardware lane: already validated App1 receiver plus future Pico H UART test, and read-only Bitaxe telemetry. Keep production mining equipment undisturbed.

## References
- Bitcoin 80-byte header and SHA256d structure: https://developer.bitcoin.org/reference/block_chain.html
- Independent prior-art and honest GPU benchmark: https://github.com/sora5801/cuda-double-sha256-miner/blob/main/docs/04-midstate-optimization.md
- Earlier unsuccessful [QHAP conditional entropy gate](QHAP_CONDITIONAL_ENTROPY_GATE_2026-10-27.md).
