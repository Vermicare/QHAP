#!/usr/bin/env python3
"""Read-only QHAP Bitcoin SHA256d exact midstate verification and CPU benchmark.
This self-contained smoke test changes no mining device, pool or firmware.
For full 64-round pure-Python compression + 3-round cache experiment, see
docs/QHAP_EXACT_SHA256D_REUSE_2026-10-27.md and downloadable research artifacts.
"""
import hashlib
import struct
import time
import statistics

GENESIS = bytes.fromhex(
    "01000000" + "00"*32 +
    "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a" +
    "29ab5f49" + "ffff001d" + "1dac2b7c"
)
EXPECTED_HASH = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"

def sha256d(header):
    return hashlib.sha256(hashlib.sha256(header).digest()).digest()

class Job:
    def __init__(self, header80):
        if len(header80) != 80:
            raise ValueError("Bitcoin block header must be 80 bytes")
        self.prefix76 = header80[:76]
        self.midstate = hashlib.sha256(self.prefix76)

    def hash_nonce(self, nonce):
        inner = self.midstate.copy()
        inner.update(struct.pack("<I", nonce))
        return hashlib.sha256(inner.digest()).digest()

def benchmark(fn, nonces, repetitions=5):
    times = []
    for _ in range(repetitions):
        start = time.perf_counter()
        checksum = 0
        for n in nonces:
            checksum ^= fn(n)[0]
        times.append(time.perf_counter() - start)
    return len(nonces) / statistics.median(times)

def main():
    assert sha256d(GENESIS)[::-1].hex() == EXPECTED_HASH
    job = Job(GENESIS)
    for nonce in (0, 1, 2, 255, 65535, 2083236893, 0xffffffff):
        assert sha256d(job.prefix76 + struct.pack("<I", nonce)) == job.hash_nonce(nonce)
    for offset in (0, 35, 63, 64, 70, 75):
        changed = bytearray(GENESIS)
        changed[offset] ^= 0x51
        newjob = Job(changed)
        for nonce in (0, 1, 0xffffffff):
            assert sha256d(newjob.prefix76 + struct.pack("<I", nonce)) == newjob.hash_nonce(nonce)
    print("GENESIS AND EXACT HEADER HASH TESTS: PASS")
    print("CACHE RECREATED AFTER CHANGED JOB PREFIX: PASS")
    samples = range(200000, 400000)
    base = benchmark(lambda n: sha256d(job.prefix76 + struct.pack("<I", n)), samples)
    shared = benchmark(job.hash_nonce, samples)
    print(f"CPU reference hashes/s: {base:.1f}")
    print(f"CPU cached-midstate hashes/s: {shared:.1f}")
    print(f"CPU ratio: {shared/base:.3f}x (hardware gains NOT inferred)")
    print("NO MINER WRITES, NO NEW BITCOIN BLOCK CLAIMS")

if __name__ == "__main__":
    main()
