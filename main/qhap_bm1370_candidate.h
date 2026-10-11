#pragma once

/* BM1370 controller-side verification only. No UART, mining.submit or flash. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "qhap_bm1370_protocol.h"

#define QHAP_CANDIDATE_MAX_JOBS 16U
#define QHAP_CANDIDATE_RECENT_SHARES 32U
#define QHAP_BM1370_ROLLABLE_BITS 0x1FFFE000U

typedef bool (*qhap_candidate_hash_fn)(const uint8_t header[80],
                                        uint8_t hash_raw[32], void *context);

typedef enum {
    QHAP_CANDIDATE_INVALID_INPUT = 0,
    QHAP_CANDIDATE_BAD_PACKET,
    QHAP_CANDIDATE_UNKNOWN_JOB,
    QHAP_CANDIDATE_VERSION_REJECTED,
    QHAP_CANDIDATE_HASH_ERROR,
    QHAP_CANDIDATE_NO_SHARE,
    QHAP_CANDIDATE_SHARE,
    QHAP_CANDIDATE_NETWORK_BLOCK,
    QHAP_CANDIDATE_DUPLICATE
} qhap_candidate_status_t;

typedef struct {
    bool used;
    bool stale;
    uint8_t job_id;
    uint64_t generation;
    uint32_t version_mask;
    uint8_t header[80];
    uint8_t pool_target_be[32];
    uint8_t network_target_be[32];
} qhap_candidate_job_t;

typedef struct {
    bool used;
    uint64_t generation;
    uint32_t nonce;
    uint32_t version;
} qhap_candidate_seen_t;

typedef struct {
    qhap_candidate_job_t jobs[QHAP_CANDIDATE_MAX_JOBS];
    qhap_candidate_seen_t recent[QHAP_CANDIDATE_RECENT_SHARES];
    size_t next_recent;
    uint64_t next_generation;
} qhap_candidate_gate_t;

typedef struct {
    qhap_candidate_status_t status;
    bool stale;              /* Do not discard a network winner solely for staleness. */
    bool hash_available;
    bool pool_match;
    bool network_match;
    uint8_t job_id;
    uint64_t generation;
    uint32_t nonce;
    uint32_t version;
    uint8_t header[80];
    uint8_t hash_raw[32];    /* Raw SHA256d digest byte ordering. */
} qhap_candidate_evaluation_t;

void qhap_candidate_init(qhap_candidate_gate_t *gate);

/* Caller must separately preserve the Stratum job ID, extranonce2 and nTime.
 * Rejects reused active ASIC job IDs to prevent ambiguous result mapping.
 * Serial access to gate state is required when used across FreeRTOS tasks.
 */
bool qhap_candidate_register_job(qhap_candidate_gate_t *gate,
                                 uint8_t job_id,
                                 const uint8_t header[80],
                                 const uint8_t pool_target_be[32],
                                 const uint8_t network_target_be[32],
                                 uint32_t version_mask);
bool qhap_candidate_mark_stale(qhap_candidate_gate_t *gate, uint8_t job_id);
bool qhap_candidate_retire_job(qhap_candidate_gate_t *gate, uint8_t job_id);

/* No network submission: caller must independently handle persistence and
 * valid, non-stale work submission policy based on the returned evaluation.
 */
qhap_candidate_status_t qhap_candidate_evaluate(
    qhap_candidate_gate_t *gate,
    const uint8_t raw[QHAP_BM1370_RESULT_SIZE],
    qhap_candidate_hash_fn hash_fn,
    void *hash_context,
    qhap_candidate_evaluation_t *out);
