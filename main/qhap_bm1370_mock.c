#include "qhap_bm1370_mock.h"
#include "qhap_bm1370_protocol.h"

#include <string.h>

bool qhap_bm1370_mock_self_test(void)
{
    /*
     * Synthetic BM1370 result packet.
     *
     * This does NOT pretend to be real ASIC work.
     * It exercises QHAP's result-decoding path before
     * physical BM1370 hardware arrives.
     */
    static const uint8_t raw[QHAP_BM1370_RESULT_SIZE] = {
        0xAA, 0x55,

        /* nonce = 0xA1B2C3D4 */
        0xA1, 0xB2, 0xC3, 0xD4,

        /* midstate number */
        0x02,

        /*
         * raw id:
         * high nibble -> encoded job id
         * low nibble  -> small-core id
         */
        0x5A,

        /* raw version = 0x1234 */
        0x12, 0x34,

        /* job-response flag + synthetic CRC bits */
        0x8C
    };

    qhap_bm1370_result_t result;

    if (!qhap_bm1370_decode_result(raw, &result))
        return false;

    if (result.nonce != 0xA1B2C3D4U)
        return false;

    if (result.midstate_num != 0x02U)
        return false;

    if (result.raw_id != 0x5AU)
        return false;

    if (result.job_id != 0x28U)
        return false;

    if (result.small_core_id != 0x0AU)
        return false;

    if (result.raw_version != 0x1234U)
        return false;

    if (result.version_bits !=
        ((uint32_t)0x1234U << 13))
        return false;

    if (!result.is_job_response)
        return false;

    uint8_t corrupt[QHAP_BM1370_RESULT_SIZE];
    memcpy(corrupt, raw, sizeof(corrupt));
    corrupt[5] ^= 0x01U;
    if (qhap_bm1370_decode_result(corrupt, &result))
        return false;

    uint8_t generated[QHAP_BM1370_RESULT_SIZE];
    qhap_bm1370_mock_make_result(
        0x01020304U, 0x3BU, 0x1234U, true, generated);
    if (!qhap_bm1370_decode_result(generated, &result) ||
        result.nonce != 0x01020304U ||
        result.raw_id != 0x3BU ||
        !result.is_job_response)
        return false;

    return true;
}
void qhap_bm1370_mock_make_result(
    uint32_t nonce,
    uint8_t raw_id,
    uint16_t raw_version,
    bool job_response,
    uint8_t out_raw[QHAP_BM1370_RESULT_SIZE])
{
    if (out_raw == NULL)
        return;

    memset(out_raw, 0, QHAP_BM1370_RESULT_SIZE);

    out_raw[0] = 0xAA;
    out_raw[1] = 0x55;

    out_raw[2] = (uint8_t)(nonce >> 24);
    out_raw[3] = (uint8_t)(nonce >> 16);
    out_raw[4] = (uint8_t)(nonce >> 8);
    out_raw[5] = (uint8_t)nonce;

    out_raw[6] = 0x00;
    out_raw[7] = raw_id;

    out_raw[8] = (uint8_t)(raw_version >> 8);
    out_raw[9] = (uint8_t)raw_version;

    const uint8_t flags =
        (uint8_t)(job_response ? 0x80U : 0x00U);

    /* Mock-only CRC search: all 32 possible CRC5 values. */
    for (uint8_t crc = 0; crc < 32U; crc++) {
        out_raw[10] = (uint8_t)(flags | crc);
        if (qhap_bm1370_crc5(
                out_raw + 2,
                QHAP_BM1370_RESULT_SIZE - 2U) == 0U)
            return;
    }

    memset(out_raw, 0, QHAP_BM1370_RESULT_SIZE);
}