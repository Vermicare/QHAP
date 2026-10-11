#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "qhap_bm1370_protocol.h"

bool qhap_bm1370_mock_self_test(void);
void qhap_bm1370_mock_make_result(
    uint32_t nonce,
    uint8_t raw_id,
    uint16_t raw_version,
    bool job_response,
    uint8_t out_raw[QHAP_BM1370_RESULT_SIZE]);