#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define QHAP_BM1370_RESULT_SIZE 11U
#define QHAP_BM1370_MAX_FRAME_SIZE 128U

#define QHAP_BM1370_TYPE_JOB 0x20U
#define QHAP_BM1370_TYPE_CMD 0x40U
#define QHAP_BM1370_GROUP_SINGLE 0x00U
#define QHAP_BM1370_GROUP_ALL 0x10U

#define QHAP_BM1370_CMD_SET_ADDRESS 0x00U
#define QHAP_BM1370_CMD_WRITE 0x01U
#define QHAP_BM1370_CMD_READ 0x02U
#define QHAP_BM1370_CMD_INACTIVE 0x03U

typedef struct {
    uint32_t nonce;
    uint8_t midstate_num;
    uint8_t raw_id;
    uint8_t job_id;
    uint8_t small_core_id;
    uint16_t raw_version;
    uint32_t version_bits;
    uint8_t crc5;
    bool is_job_response;
} qhap_bm1370_result_t;

uint8_t qhap_bm1370_crc5(const uint8_t *data, size_t len);
uint16_t qhap_bm1370_crc16_false(const uint8_t *data, size_t len);

size_t qhap_bm1370_build_frame(uint8_t header,
                               const uint8_t *payload,
                               size_t payload_len,
                               uint8_t *out,
                               size_t out_capacity);

void qhap_bm1370_build_version_mask_payload(uint32_t version_mask,
                                            uint8_t out_payload[6]);

bool qhap_bm1370_decode_result(const uint8_t raw[QHAP_BM1370_RESULT_SIZE],
                               qhap_bm1370_result_t *out);

bool qhap_bm1370_protocol_self_test(void);
