/*
 * Host-side test of the isolated ASIC verification gate.
 * Build from the repo root (OpenSSL development headers required):
 * gcc -std=c11 -Wall -Wextra -Werror -I main \
 *   main/qhap_bm1370_protocol.c main/qhap_bm1370_mock.c \
 *   main/qhap_bm1370_candidate.c tests/test_bm1370_candidate.c \
 *   -lcrypto -o test_bm1370_candidate
 * ./test_bm1370_candidate
 *
 * No serial hardware or CKPool connection is involved.
 */
#include "qhap_bm1370_candidate.h"
#include "qhap_bm1370_mock.h"

#include <assert.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>

static bool sha256d(const uint8_t header[80], uint8_t raw[32], void *ctx)
{
    (void)ctx;
    uint8_t first[SHA256_DIGEST_LENGTH];
    if (SHA256(header, 80, first) == NULL) return false;
    return SHA256(first, sizeof(first), raw) != NULL;
}

static bool refuse_hash(const uint8_t header[80], uint8_t raw[32], void *ctx)
{
    (void)header; (void)raw; (void)ctx;
    return false;
}

static void read_hex(const char *hex, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        unsigned x = 0;
        assert(sscanf(hex + i * 2, "%2x", &x) == 1);
        out[i] = (uint8_t)x;
    }
}

int main(void)
{
    /* Published Bitcoin genesis block header and independently known hash. */
    static const char genesis_header[] =
        "01000000"
        "0000000000000000000000000000000000000000000000000000000000000000"
        "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a"
        "29ab5f49ffff001d1dac2b7c";
    static const char genesis_hash_raw[] =
        "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000";
    uint8_t header[80], expected_hash[32];
    uint8_t target[32] = {0};
    read_hex(genesis_header, header, sizeof(header));
    read_hex(genesis_hash_raw, expected_hash, sizeof(expected_hash));
    target[4] = 0xff; target[5] = 0xff;

    assert(qhap_bm1370_protocol_self_test());
    assert(qhap_bm1370_mock_self_test());

    qhap_candidate_gate_t gate;
    qhap_candidate_init(&gate);
    assert(qhap_candidate_register_job(&gate, 0x28, header, target, target, 0));
    assert(!qhap_candidate_register_job(&gate, 0x28, header, target, target, 0));

    qhap_candidate_evaluation_t eval;
    uint8_t packet[QHAP_BM1370_RESULT_SIZE];
    qhap_bm1370_mock_make_result(2083236893U, 0x5a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_NETWORK_BLOCK);
    assert(eval.network_match && eval.pool_match && eval.hash_available && !eval.stale);
    assert(memcmp(eval.hash_raw, expected_hash, 32) == 0);
    assert(memcmp(eval.header, header, 80) == 0);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_DUPLICATE);
    assert(eval.network_match); /* Duplicate winner remains identifiable. */

    uint8_t bad[QHAP_BM1370_RESULT_SIZE];
    memcpy(bad, packet, sizeof(bad));
    bad[2] ^= 1;
    assert(qhap_candidate_evaluate(&gate, bad, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_BAD_PACKET);

    qhap_bm1370_mock_make_result(2083236893U, 0x6a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_UNKNOWN_JOB);

    qhap_bm1370_mock_make_result(2083236893U, 0x5a, 1, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_VERSION_REJECTED);

    qhap_bm1370_mock_make_result(2083236893U, 0x5a, 0, false, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_BAD_PACKET);

    assert(qhap_candidate_mark_stale(&gate, 0x28));
    qhap_bm1370_mock_make_result(2083236893U, 0x5a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_DUPLICATE);
    assert(eval.stale && eval.network_match);

    assert(qhap_candidate_retire_job(&gate, 0x28));
    assert(qhap_candidate_register_job(&gate, 0x28, header, target, target, 0));
    assert(qhap_candidate_mark_stale(&gate, 0x28));
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_NETWORK_BLOCK);
    assert(eval.stale && eval.network_match); /* No suppression of stale winner. */

    assert(qhap_candidate_retire_job(&gate, 0x28));
    uint8_t easy_pool[32];
    memset(easy_pool, 0xff, sizeof(easy_pool));
    assert(qhap_candidate_register_job(&gate, 0x28, header, easy_pool, target, 0));
    qhap_bm1370_mock_make_result(0, 0x5a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_SHARE);
    assert(eval.pool_match && !eval.network_match);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_DUPLICATE);

    qhap_bm1370_mock_make_result(1, 0x5a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, refuse_hash, NULL, &eval) ==
           QHAP_CANDIDATE_HASH_ERROR);

    assert(qhap_candidate_retire_job(&gate, 0x28));
    assert(qhap_candidate_register_job(&gate, 0x28, header, target, target, 0));
    qhap_bm1370_mock_make_result(0, 0x5a, 0, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_NO_SHARE);

    assert(qhap_candidate_retire_job(&gate, 0x28));
    assert(qhap_candidate_register_job(&gate, 0x28, header, easy_pool, target,
                                       QHAP_BM1370_ROLLABLE_BITS));
    qhap_bm1370_mock_make_result(0, 0x5a, 1, true, packet);
    assert(qhap_candidate_evaluate(&gate, packet, sha256d, NULL, &eval) ==
           QHAP_CANDIDATE_SHARE);
    assert(eval.version == (1U | (1U << 13)));
    assert(eval.header[1] == 0x20);

    puts("BM1370 candidate gate host tests: PASS");
    return 0;
}
