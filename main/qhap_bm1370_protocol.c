#include "qhap_bm1370_protocol.h"

#include <string.h>

static uint16_t qhap_read_be16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) |
           (uint16_t)p[1];
}

static uint32_t qhap_read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

uint8_t qhap_bm1370_crc5(const uint8_t *data, size_t len)
{
    if (data == NULL && len != 0)
        return 0;

    uint8_t crc = 0x1FU;

    for (size_t i = 0; i < len; i++) {
        for (unsigned bit = 0; bit < 8; bit++) {
            uint8_t input =
                (uint8_t)(((data[i] >> (7U - bit)) & 1U) ^
                          ((crc >> 4) & 1U));

            crc = (uint8_t)((crc << 1) & 0x1FU);

            if (input != 0)
                crc ^= 0x05U;
        }
    }

    return (uint8_t)(crc & 0x1FU);
}

uint16_t qhap_bm1370_crc16_false(const uint8_t *data, size_t len)
{
    if (data == NULL && len != 0)
        return 0;

    uint16_t crc = 0xFFFFU;

    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;

        for (unsigned bit = 0; bit < 8; bit++) {
            crc = (crc & 0x8000U)
                ? (uint16_t)((crc << 1) ^ 0x1021U)
                : (uint16_t)(crc << 1);
        }
    }

    return crc;
}

size_t qhap_bm1370_build_frame(uint8_t header,
                               const uint8_t *payload,
                               size_t payload_len,
                               uint8_t *out,
                               size_t out_capacity)
{
    if (out == NULL ||
        (payload == NULL && payload_len != 0) ||
        payload_len > 122U) {
        return 0;
    }

    const bool is_job = (header & QHAP_BM1370_TYPE_JOB) != 0;
    const size_t checksum_len = is_job ? 2U : 1U;
    const size_t total_len = 4U + payload_len + checksum_len;

    if (total_len > out_capacity ||
        total_len > QHAP_BM1370_MAX_FRAME_SIZE) {
        return 0;
    }

    out[0] = 0x55U;
    out[1] = 0xAAU;
    out[2] = header;
    out[3] = (uint8_t)(payload_len + (is_job ? 4U : 3U));

    if (payload_len != 0)
        memcpy(out + 4, payload, payload_len);

    if (is_job) {
        uint16_t crc =
            qhap_bm1370_crc16_false(out + 2, payload_len + 2U);

        out[4U + payload_len] = (uint8_t)(crc >> 8);
        out[5U + payload_len] = (uint8_t)crc;
    } else {
        out[4U + payload_len] =
            qhap_bm1370_crc5(out + 2, payload_len + 2U);
    }

    return total_len;
}

void qhap_bm1370_build_version_mask_payload(uint32_t version_mask,
                                            uint8_t out_payload[6])
{
    if (out_payload == NULL)
        return;

    uint32_t rolling = version_mask >> 13;

    out_payload[0] = 0x00U;
    out_payload[1] = 0xA4U;
    out_payload[2] = 0x90U;
    out_payload[3] = 0x00U;
    out_payload[4] = (uint8_t)(rolling >> 8);
    out_payload[5] = (uint8_t)rolling;
}

bool qhap_bm1370_decode_result(const uint8_t raw[QHAP_BM1370_RESULT_SIZE],
                               qhap_bm1370_result_t *out)
{
    if (raw == NULL || out == NULL)
        return false;

    if (raw[0] != 0xAAU || raw[1] != 0x55U)
        return false;

    memset(out, 0, sizeof(*out));

    out->nonce = qhap_read_be32(raw + 2);
    out->midstate_num = raw[6];
    out->raw_id = raw[7];
    out->job_id = (uint8_t)((raw[7] & 0xF0U) >> 1);
    out->small_core_id = (uint8_t)(raw[7] & 0x0FU);
    out->raw_version = qhap_read_be16(raw + 8);
    out->version_bits = (uint32_t)out->raw_version << 13;
    out->crc5 = (uint8_t)(raw[10] & 0x1FU);
    out->is_job_response = (raw[10] & 0x80U) != 0;

    return true;
}

bool qhap_bm1370_protocol_self_test(void)
{
    /*
     * Known BM13xx command example:
     * 55 AA 51 09 00 18 F0 00 C1 00 04
     * The checksum byte must be 0x04.
     */
    static const uint8_t payload[6] = {
        0x00U, 0x18U, 0xF0U, 0x00U, 0xC1U, 0x00U
    };

    static const uint8_t expected[11] = {
        0x55U, 0xAAU, 0x51U, 0x09U,
        0x00U, 0x18U, 0xF0U, 0x00U, 0xC1U, 0x00U,
        0x04U
    };

    uint8_t frame[16] = {0};

    size_t len =
        qhap_bm1370_build_frame(
            QHAP_BM1370_TYPE_CMD |
            QHAP_BM1370_GROUP_ALL |
            QHAP_BM1370_CMD_WRITE,
            payload,
            sizeof(payload),
            frame,
            sizeof(frame));

    return len == sizeof(expected) &&
           memcmp(frame, expected, sizeof(expected)) == 0;
}
