#pragma once

#include <stdint.h>
#include <stddef.h>

void qhap_sw_sha256(const uint8_t *data, size_t len, uint8_t out[32]);
void qhap_sw_sha256d(const uint8_t *data, size_t len, uint8_t out[32]);
void qhap_sw_sha256_midstate_64(
    const uint8_t first64[64],
    uint32_t midstate[8]);

void qhap_sw_sha256d_80_from_midstate(
    const uint32_t midstate[8],
    const uint8_t tail16[16],
    uint8_t out[32]);