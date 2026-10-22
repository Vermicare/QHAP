#include "software_sha256.h"

#include <string.h>

static const uint32_t k256[64] = {
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,
    0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,
    0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,
    0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,
    0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,
    0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,
    0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,
    0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,
    0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static inline uint32_t rotr32(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32U - n));
}

static inline uint32_t load_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
           ((uint32_t)p[3]);
}

static inline void store_be32(uint8_t *p, uint32_t x)
{
    p[0] = (uint8_t)(x >> 24);
    p[1] = (uint8_t)(x >> 16);
    p[2] = (uint8_t)(x >> 8);
    p[3] = (uint8_t)x;
}

static void sha256_compress(uint32_t state[8], const uint8_t block[64])
{
    uint32_t w[64];

    for (int i = 0; i < 16; i++)
        w[i] = load_be32(block + (i * 4));

    for (int i = 16; i < 64; i++) {
        uint32_t s0 =
            rotr32(w[i - 15], 7) ^
            rotr32(w[i - 15], 18) ^
            (w[i - 15] >> 3);

        uint32_t s1 =
            rotr32(w[i - 2], 17) ^
            rotr32(w[i - 2], 19) ^
            (w[i - 2] >> 10);

        w[i] = w[i - 16] + s0 +
               w[i - 7] + s1;
    }

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];
    uint32_t f = state[5];
    uint32_t g = state[6];
    uint32_t h = state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t S1 =
            rotr32(e, 6) ^
            rotr32(e, 11) ^
            rotr32(e, 25);

        uint32_t ch =
            (e & f) ^ ((~e) & g);

        uint32_t temp1 =
            h + S1 + ch + k256[i] + w[i];

        uint32_t S0 =
            rotr32(a, 2) ^
            rotr32(a, 13) ^
            rotr32(a, 22);

        uint32_t maj =
            (a & b) ^ (a & c) ^ (b & c);

        uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

void qhap_sw_sha256(const uint8_t *data,
                    size_t len,
                    uint8_t out[32])
{
    uint32_t state[8] = {
        0x6a09e667U,0xbb67ae85U,
        0x3c6ef372U,0xa54ff53aU,
        0x510e527fU,0x9b05688cU,
        0x1f83d9abU,0x5be0cd19U
    };

    size_t offset = 0;

    while (len - offset >= 64) {
        sha256_compress(state, data + offset);
        offset += 64;
    }

    uint8_t final[128] = {0};
    size_t remaining = len - offset;

    memcpy(final, data + offset, remaining);
    final[remaining] = 0x80;

    size_t final_len =
        (remaining < 56) ? 64 : 128;

    uint64_t bit_len = (uint64_t)len * 8ULL;

    for (int i = 0; i < 8; i++) {
        final[final_len - 1 - i] =
            (uint8_t)(bit_len >> (i * 8));
    }

    sha256_compress(state, final);

    if (final_len == 128)
        sha256_compress(state, final + 64);

    for (int i = 0; i < 8; i++)
        store_be32(out + (i * 4), state[i]);
}

void qhap_sw_sha256d(const uint8_t *data,
                     size_t len,
                     uint8_t out[32])
{
    uint8_t first[32];

    qhap_sw_sha256(data, len, first);
    qhap_sw_sha256(first, sizeof(first), out);
}

void qhap_sw_sha256_midstate_64(
    const uint8_t first64[64],
    uint32_t midstate[8])
{
    uint32_t state[8] = {
        0x6a09e667U,0xbb67ae85U,
        0x3c6ef372U,0xa54ff53aU,
        0x510e527fU,0x9b05688cU,
        0x1f83d9abU,0x5be0cd19U
    };

    sha256_compress(state, first64);
    memcpy(midstate, state, sizeof(state));
}

void qhap_sw_sha256d_80_from_midstate(
    const uint32_t midstate[8],
    const uint8_t tail16[16],
    uint8_t out[32])
{
    uint32_t first_state[8];
    memcpy(first_state, midstate, sizeof(first_state));

    uint8_t block[64] = {0};

    memcpy(block, tail16, 16);
    block[16] = 0x80;
    block[62] = 0x02;
    block[63] = 0x80;

    sha256_compress(first_state, block);

    memset(block, 0, sizeof(block));

    for (int i = 0; i < 8; i++)
        store_be32(block + (i * 4), first_state[i]);

    block[32] = 0x80;
    block[62] = 0x01;
    block[63] = 0x00;

    uint32_t second_state[8] = {
        0x6a09e667U,0xbb67ae85U,
        0x3c6ef372U,0xa54ff53aU,
        0x510e527fU,0x9b05688cU,
        0x1f83d9abU,0x5be0cd19U
    };

    sha256_compress(second_state, block);

    for (int i = 0; i < 8; i++)
        store_be32(out + (i * 4), second_state[i]);
}
