#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "cJSON.h"
#include "psa/crypto.h"
#include "esp_timer.h"
#include "sha/sha_core.h"
#include "hal/sha_ll.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAX_RETRIES        5

static EventGroupHandle_t wifi_event_group;
static int retry_count = 0;

static char g_extranonce1[65] = {0};
static int g_extranonce2_size = 0;
static double g_pool_difficulty = 0.0;
static uint8_t g_pool_target_be[32] = {0};
static bool g_pool_target_valid = false;

static uint8_t g_pending_header[80];
static bool g_pending_header_valid = false;
static int g_stratum_sock = -1;

static char g_current_job_id[128] = {0};
static char g_current_ntime[16] = {0};
static char g_current_extranonce2[65] = {0};

static bool qhap_hash_meets_target(const uint8_t hash_raw[32],
                                   const uint8_t target_be[32]);

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool hex_to_bytes(const char *hex,
                         uint8_t *out,
                         size_t out_capacity,
                         size_t *out_len)
{
    size_t chars = strlen(hex);

    if ((chars & 1) != 0)
        return false;

    size_t bytes = chars / 2;

    if (bytes > out_capacity)
        return false;

    for (size_t i = 0; i < bytes; i++) {

        int hi = hex_nibble(hex[i * 2]);
        int lo = hex_nibble(hex[i * 2 + 1]);

        if (hi < 0 || lo < 0)
            return false;

        out[i] = (uint8_t)((hi << 4) | lo);
    }

    if (out_len != NULL)
        *out_len = bytes;

    return true;
}

static bool qhap_sha256d(const uint8_t *data,
                         size_t len,
                         uint8_t out[32])
{
    uint8_t first[32];
    size_t written = 0;

    psa_status_t status = psa_hash_compute(
        PSA_ALG_SHA_256,
        data,
        len,
        first,
        sizeof(first),
        &written);

    if (status != PSA_SUCCESS || written != 32)
        return false;

    status = psa_hash_compute(
        PSA_ALG_SHA_256,
        first,
        sizeof(first),
        out,
        32,
        &written);

    return status == PSA_SUCCESS && written == 32;
}

static bool qhap_build_coinbase_hash(const char *coinb1_hex,
                                     const char *coinb2_hex,
                                     uint8_t out_hash[32])
{
    static uint8_t coinbase[1024];

    size_t pos = 0;
    size_t n = 0;

    if (!hex_to_bytes(coinb1_hex,
                      coinbase + pos,
                      sizeof(coinbase) - pos,
                      &n))
        return false;

    pos += n;

    if (!hex_to_bytes(g_extranonce1,
                      coinbase + pos,
                      sizeof(coinbase) - pos,
                      &n))
        return false;

    pos += n;

    if (g_extranonce2_size <= 0 ||
        g_extranonce2_size > 32 ||
        pos + g_extranonce2_size > sizeof(coinbase))
        return false;

    memset(coinbase + pos, 0, g_extranonce2_size);

    memset(g_current_extranonce2,
           '0',
           g_extranonce2_size * 2);

    g_current_extranonce2[
        g_extranonce2_size * 2] = '\0';

    pos += g_extranonce2_size;

    if (!hex_to_bytes(coinb2_hex,
                      coinbase + pos,
                      sizeof(coinbase) - pos,
                      &n))
        return false;

    pos += n;

    printf("Coinbase bytes: %u\n", (unsigned)pos);

    return qhap_sha256d(coinbase, pos, out_hash);
}
static bool qhap_build_merkle_root(const uint8_t coinbase_hash[32],
                                   cJSON *branches,
                                   uint8_t out_root[32])
{
    if (!cJSON_IsArray(branches))
        return false;

    memcpy(out_root, coinbase_hash, 32);

    uint8_t branch_hash[32];
    uint8_t pair[64];

    int count = cJSON_GetArraySize(branches);

    for (int i = 0; i < count; i++) {

        cJSON *branch = cJSON_GetArrayItem(branches, i);

        if (!cJSON_IsString(branch))
            return false;

        size_t branch_len = 0;

        if (!hex_to_bytes(branch->valuestring,
                          branch_hash,
                          sizeof(branch_hash),
                          &branch_len) ||
            branch_len != 32) {

            return false;
        }

        memcpy(pair, out_root, 32);
        memcpy(pair + 32, branch_hash, 32);

        if (!qhap_sha256d(pair,
                          sizeof(pair),
                          out_root)) {

            return false;
        }
    }

    return true;
}
static bool qhap_build_header(const char *version_hex,
                              const char *prevhash_hex,
                              const uint8_t merkle_root[32],
                              const char *ntime_hex,
                              const char *nbits_hex,
                              uint32_t nonce,
                              uint8_t header[80])
{
    uint8_t temp4[4];
    uint8_t prev[32];
    size_t len = 0;

    /* Version: Stratum hex -> little-endian header bytes */
    if (!hex_to_bytes(version_hex, temp4, sizeof(temp4), &len) ||
        len != 4)
        return false;

    header[0] = temp4[3];
    header[1] = temp4[2];
    header[2] = temp4[1];
    header[3] = temp4[0];

    /*
     * Stratum V1 prevhash is transformed by reversing
     * the bytes within each 32-bit word.
     */
    if (!hex_to_bytes(prevhash_hex, prev, sizeof(prev), &len) ||
        len != 32)
        return false;

    for (int word = 0; word < 8; word++) {
        header[4 + word * 4 + 0] = prev[word * 4 + 3];
        header[4 + word * 4 + 1] = prev[word * 4 + 2];
        header[4 + word * 4 + 2] = prev[word * 4 + 1];
        header[4 + word * 4 + 3] = prev[word * 4 + 0];
    }

    /* Raw SHA256d Merkle root is already in header/internal order. */
    memcpy(header + 36, merkle_root, 32);

    /* nTime -> little endian */
    if (!hex_to_bytes(ntime_hex, temp4, sizeof(temp4), &len) ||
        len != 4)
        return false;

    header[68] = temp4[3];
    header[69] = temp4[2];
    header[70] = temp4[1];
    header[71] = temp4[0];

    /* nBits -> little endian */
    if (!hex_to_bytes(nbits_hex, temp4, sizeof(temp4), &len) ||
        len != 4)
        return false;

    header[72] = temp4[3];
    header[73] = temp4[2];
    header[74] = temp4[1];
    header[75] = temp4[0];

    /* Nonce -> little endian */
    header[76] = (uint8_t)(nonce);
    header[77] = (uint8_t)(nonce >> 8);
    header[78] = (uint8_t)(nonce >> 16);
    header[79] = (uint8_t)(nonce >> 24);

    return true;
}
static inline void qhap_hw_wait_sha(void)
{
    while (sha_ll_busy()) {
    }
}

static bool qhap_sha256d_hw_80(const uint8_t header[80],
                               uint8_t out[32])
{
    uint32_t block1[16];
    uint32_t block2[16] = {0};

    uint32_t midstate[8];
    uint32_t first_digest[8];
    uint32_t second_block[16] = {0};
    uint32_t final_digest[8];

    memcpy(block1, header, 64);
    memcpy(block2, header + 64, 16);

    block2[4]  = 0x00000080;
    block2[15] = 0x80020000;

    esp_sha_acquire_hardware();
    sha_ll_set_mode(SHA2_256);

    sha_ll_fill_text_block(block1, 16);
    sha_ll_start_block(SHA2_256);
    qhap_hw_wait_sha();

    sha_ll_read_digest(SHA2_256, midstate, 8);

    sha_ll_fill_text_block(block2, 16);
    sha_ll_write_digest(SHA2_256, midstate, 8);
    sha_ll_continue_block(SHA2_256);
    qhap_hw_wait_sha();

    sha_ll_read_digest(SHA2_256, first_digest, 8);

    memcpy(second_block, first_digest, 32);

    second_block[8]  = 0x00000080;
    second_block[15] = 0x00010000;

    sha_ll_fill_text_block(second_block, 16);
    sha_ll_start_block(SHA2_256);
    qhap_hw_wait_sha();

    sha_ll_read_digest(SHA2_256, final_digest, 8);

    esp_sha_release_hardware();

    memcpy(out, final_digest, 32);

    return true;
}
static bool qhap_submit_share(uint32_t nonce)
{
    if (g_stratum_sock < 0) {
        printf("Share submit FAILED: no Stratum socket\n");
        return false;
    }

    if (g_current_job_id[0] == '\0' ||
        g_current_ntime[0] == '\0' ||
        g_current_extranonce2[0] == '\0') {

        printf("Share submit FAILED: incomplete job identity\n");
        return false;
    }

    /*
     * Stratum expects the serialized 4-byte nonce.
     * Our numeric nonce is written little-endian into the
     * Bitcoin header, so encode those same four bytes.
     */
    char nonce_hex[9];

    snprintf(nonce_hex,
             sizeof(nonce_hex),
             "%08x",
             (unsigned)nonce);

    char request[512];

    int length = snprintf(
        request,
        sizeof(request),
        "{\"id\":3,\"method\":\"mining.submit\","
        "\"params\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]}\n",
        CONFIG_QHAP_STRATUM_USER,
        g_current_job_id,
        g_current_extranonce2,
        g_current_ntime,
        nonce_hex);

    if (length <= 0 ||
        length >= sizeof(request)) {

        printf("Share submit FAILED: request too large\n");
        return false;
    }

    printf("\n=== SHARE FOUND ===\n");
    printf("Job ID: %s\n", g_current_job_id);
    printf("Extranonce2: %s\n", g_current_extranonce2);
    printf("nTime: %s\n", g_current_ntime);
    printf("Nonce: %s\n", nonce_hex);

    int sent = send(g_stratum_sock,
                    request,
                    length,
                    0);

    if (sent != length) {
        printf("mining.submit SEND FAILED\n");
        return false;
    }

    printf("mining.submit SENT\n");
    printf("===================\n");

    return true;
}
static void qhap_live_nonce_scan(const uint8_t base_header[80], uint32_t start_nonce)
{
    const uint32_t iterations = 100000;
    const uint32_t end_nonce =
        start_nonce + iterations - 1;

    uint32_t block1[16];
    uint32_t block2[16] = {0};

    uint32_t midstate[8];
    uint32_t final_state[8];
    uint32_t candidate_state[8];

    uint32_t share_candidates = 0;
    uint32_t first_share_nonce = 0;
    uint8_t first_share_hash[32] = {0};

    uint32_t pool_target_top =
        ((uint32_t)g_pool_target_be[0] << 24) |
        ((uint32_t)g_pool_target_be[1] << 16) |
        ((uint32_t)g_pool_target_be[2] << 8)  |
        ((uint32_t)g_pool_target_be[3]);

    memcpy(block1, base_header, 64);
    memcpy(block2, base_header + 64, 16);

    /*
     * SHA-256 padding for an 80-byte Bitcoin header.
     */
    block2[4]  = 0x00000080;
    block2[15] = 0x80020000;

    /*
     * Preserve the live job's Merkle tail, nTime and nBits.
     * Only block2[3] (nonce) changes during scanning.
     */
    const uint32_t live_word0 = block2[0];
    const uint32_t live_word1 = block2[1];
    const uint32_t live_word2 = block2[2];

    esp_sha_acquire_hardware();
    sha_ll_set_mode(SHA2_256);

    /*
     * First 64 bytes never change for this job:
     * calculate the SHA-256 midstate once.
     */
    sha_ll_fill_text_block(block1, 16);
    sha_ll_start_block(SHA2_256);
    qhap_hw_wait_sha();

    sha_ll_read_digest(SHA2_256, midstate, 8);

    /*
     * Load the second header block once.
     */
    sha_ll_fill_text_block(block2, 16);

    uint32_t *sha_text =
        (uint32_t *)SHA_TEXT_BASE;

    uint32_t *sha_digest =
        (uint32_t *)SHA_H_BASE;

    int64_t start_us = esp_timer_get_time();

    for (uint32_t offset = 0;
         offset < iterations;
         offset++) {

        uint32_t nonce =
            start_nonce + offset;

        /*
         * Bitcoin nonce occupies bytes 76..79.
         */
        REG_WRITE(&sha_text[3], nonce);

        /*
         * Finish first SHA-256 from cached midstate.
         */
        sha_ll_write_digest(SHA2_256, midstate, 8);
        sha_ll_continue_block(SHA2_256);
        qhap_hw_wait_sha();

        /*
         * Feed first digest directly into second SHA-256.
         */
        REG_WRITE(&sha_text[0], REG_READ(&sha_digest[0]));
        REG_WRITE(&sha_text[1], REG_READ(&sha_digest[1]));
        REG_WRITE(&sha_text[2], REG_READ(&sha_digest[2]));
        REG_WRITE(&sha_text[3], REG_READ(&sha_digest[3]));
        REG_WRITE(&sha_text[4], REG_READ(&sha_digest[4]));
        REG_WRITE(&sha_text[5], REG_READ(&sha_digest[5]));
        REG_WRITE(&sha_text[6], REG_READ(&sha_digest[6]));
        REG_WRITE(&sha_text[7], REG_READ(&sha_digest[7]));

        REG_WRITE(&sha_text[8],  0x00000080);
        REG_WRITE(&sha_text[15], 0x00010000);

        sha_ll_start_block(SHA2_256);
        qhap_hw_wait_sha();

        /*
         * sha_digest[7] contains the most-significant
         * 32 bits of the displayed Bitcoin hash after
         * byte reversal.
         */
        uint32_t raw_high =
            REG_READ(&sha_digest[7]);

        uint32_t hash_high =
            __builtin_bswap32(raw_high);

        if (g_pool_target_valid &&
            hash_high <= pool_target_top) {

            sha_ll_read_digest(
                SHA2_256,
                candidate_state,
                8);

            uint8_t *candidate_bytes =
                (uint8_t *)candidate_state;

            if (qhap_hash_meets_target(
                    candidate_bytes,
                    g_pool_target_be)) {

                share_candidates++;

                if (share_candidates == 1) {

                    first_share_nonce = nonce;

                    memcpy(first_share_hash,
                           candidate_bytes,
                           32);
                }
            }
        }

        if (offset == iterations - 1) {
            sha_ll_read_digest(
                SHA2_256,
                final_state,
                8);
        }

        /*
         * Restore live second-block words for next nonce.
         */
        REG_WRITE(&sha_text[0], live_word0);
        REG_WRITE(&sha_text[1], live_word1);
        REG_WRITE(&sha_text[2], live_word2);

        REG_WRITE(&sha_text[4], 0x00000080);
        REG_WRITE(&sha_text[5], 0);
        REG_WRITE(&sha_text[6], 0);
        REG_WRITE(&sha_text[7], 0);
        REG_WRITE(&sha_text[8], 0);
        REG_WRITE(&sha_text[15], 0x80020000);
    }

    int64_t elapsed_us =
        esp_timer_get_time() - start_us;

    esp_sha_release_hardware();

    if (share_candidates > 0) {
        qhap_submit_share(first_share_nonce);
    }

    double seconds =
        (double)elapsed_us / 1000000.0;

    double rate =
        (double)iterations / seconds;

    printf("\n=== LIVE NONCE SCAN ===\n");
    printf("Iterations: %lu\n",
           (unsigned long)iterations);

    printf("Rate: %.2f kH/s\n",
           rate / 1000.0);

    printf("Nonce range: %lu - %lu\n",
           (unsigned long)start_nonce,
           (unsigned long)end_nonce);

    printf("Final nonce: %lu\n",
           (unsigned long)end_nonce);

    printf("Pool-share candidates: %lu\n",
           (unsigned long)share_candidates);

    if (share_candidates > 0) {

        printf("First share nonce: %lu\n",
               (unsigned long)first_share_nonce);

        printf("First share hash: ");

        for (int i = 31; i >= 0; i--)
            printf("%02x", first_share_hash[i]);

        printf("\n");
    }

    printf("Hardware final hash raw: ");

    uint8_t *final_bytes =
        (uint8_t *)final_state;

    for (int i = 0; i < 32; i++)
        printf("%02x", final_bytes[i]);

    printf("\n");
    printf("=======================\n");
}
static bool qhap_nbits_to_target(const char *nbits_hex,
                                 uint8_t target_be[32])
{
    uint8_t compact[4];
    size_t len = 0;

    if (!hex_to_bytes(nbits_hex,
                      compact,
                      sizeof(compact),
                      &len) ||
        len != 4)
        return false;

    uint8_t exponent = compact[0];

    uint32_t mantissa =
        ((uint32_t)compact[1] << 16) |
        ((uint32_t)compact[2] << 8)  |
        ((uint32_t)compact[3]);

    memset(target_be, 0, 32);

    if (mantissa == 0)
        return true;

    if (exponent <= 3) {

        uint32_t value =
            mantissa >> (8 * (3 - exponent));

        for (int i = 0;
             i < exponent;
             i++) {

            target_be[31 - i] =
                (uint8_t)(value >> (8 * i));
        }

    } else {

        if (exponent > 32)
            return false;

        int start = 32 - exponent;

        if (start < 0 || start + 2 >= 32)
            return false;

        target_be[start]     = compact[1];
        target_be[start + 1] = compact[2];
        target_be[start + 2] = compact[3];
    }

    return true;
}

static bool qhap_hash_meets_target(const uint8_t hash_raw[32],
                                   const uint8_t target_be[32])
{
    /*
     * Bitcoin interprets the SHA256d digest as a little-endian
     * 256-bit integer. Therefore compare the reversed raw digest
     * against the big-endian target.
     */
    for (int i = 0; i < 32; i++) {

        uint8_t h = hash_raw[31 - i];
        uint8_t t = target_be[i];

        if (h < t)
            return true;

        if (h > t)
            return false;
    }

    return true;
}
static bool qhap_pool_difficulty_to_target(double difficulty,
                                           uint8_t target_be[32])
{
    /*
     * Stratum difficulty-1 target:
     * 00000000ffff0000000000000000000000000000000000000000000000000000
     */
    static const uint8_t diff1_target[32] = {
        0x00,0x00,0x00,0x00,
        0xff,0xff,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    if (difficulty < 1.0 ||
        difficulty > 4294967295.0)
        return false;

    uint32_t divisor = (uint32_t)difficulty;

    /*
     * Current CKPool difficulty is an integer.
     * Refuse approximate conversion if a future pool sends
     * a fractional value; we can extend that separately.
     */
    if ((double)divisor != difficulty ||
        divisor == 0)
        return false;

    uint64_t remainder = 0;

    for (int i = 0; i < 32; i++) {

        uint64_t value =
            (remainder << 8) |
            diff1_target[i];

        target_be[i] =
            (uint8_t)(value / divisor);

        remainder =
            value % divisor;
    }

    return true;
}
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {

        if (retry_count < MAX_RETRIES) {
            retry_count++;
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        printf("Wi-Fi connected\n");
        printf("IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));

        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_connect(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL));

    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.sta.ssid,
            CONFIG_QHAP_WIFI_SSID,
            sizeof(wifi_config.sta.ssid) - 1);

    strncpy((char *)wifi_config.sta.password,
            CONFIG_QHAP_WIFI_PASSWORD,
            sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    return (bits & WIFI_CONNECTED_BIT) != 0;
}

static void process_stratum_line(const char *line)
{
    cJSON *root = cJSON_Parse(line);

    if (root == NULL) {
        printf("JSON parse failed\n");
        return;
    }

    cJSON *method = cJSON_GetObjectItem(root, "method");
    cJSON *response_id = cJSON_GetObjectItem(root, "id");

    /*
     * mining.submit responses have no "method".
     * QHAP currently uses request id 3 for share submissions.
     */
    if (!cJSON_IsString(method) &&
        cJSON_IsNumber(response_id) &&
        response_id->valueint == 3) {

        cJSON *result =
            cJSON_GetObjectItem(root, "result");

        cJSON *error =
            cJSON_GetObjectItem(root, "error");

        printf("\n=== SHARE RESPONSE ===\n");

        if (cJSON_IsTrue(result)) {

            printf("CKPool share: ACCEPTED\n");

        } else {

            printf("CKPool share: REJECTED\n");

            if (error != NULL) {
                char *error_text =
                    cJSON_PrintUnformatted(error);

                if (error_text != NULL) {
                    printf("Pool error: %s\n",
                           error_text);

                    cJSON_free(error_text);
                }
            }
        }

        printf("======================\n");

    } else if (cJSON_IsString(method)) {

        if (strcmp(method->valuestring, "mining.set_difficulty") == 0) {

            cJSON *params = cJSON_GetObjectItem(root, "params");
            cJSON *difficulty = cJSON_GetArrayItem(params, 0);

            if (cJSON_IsNumber(difficulty)) {
                g_pool_difficulty =
                    difficulty->valuedouble;

                printf("Pool difficulty: %.0f\n",
                       g_pool_difficulty);

                g_pool_target_valid =
                    qhap_pool_difficulty_to_target(
                        g_pool_difficulty,
                        g_pool_target_be);

                if (g_pool_target_valid) {

                    printf("Pool share target: ");

                    for (int i = 0; i < 32; i++)
                        printf("%02x",
                               g_pool_target_be[i]);

                    printf("\n");

                    if (g_pending_header_valid) {
                        printf("Pool target ready - queued job can now be mined\n");
                    }

                } else {

                    printf("Pool share target calculation FAILED\n");
                }
            }

        } else if (strcmp(method->valuestring, "mining.notify") == 0) {

            cJSON *params = cJSON_GetObjectItem(root, "params");

            if (!cJSON_IsArray(params) ||
                cJSON_GetArraySize(params) < 9) {

                printf("Invalid mining.notify\n");
                cJSON_Delete(root);
                return;
            }

            cJSON *job_id   = cJSON_GetArrayItem(params, 0);
            cJSON *prevhash = cJSON_GetArrayItem(params, 1);
            cJSON *coinb1   = cJSON_GetArrayItem(params, 2);
            cJSON *coinb2   = cJSON_GetArrayItem(params, 3);
            cJSON *branches = cJSON_GetArrayItem(params, 4);
            cJSON *version  = cJSON_GetArrayItem(params, 5);
            cJSON *nbits    = cJSON_GetArrayItem(params, 6);
            cJSON *ntime    = cJSON_GetArrayItem(params, 7);
            cJSON *clean    = cJSON_GetArrayItem(params, 8);

            printf("\n=== LIVE BITCOIN JOB ===\n");

            if (cJSON_IsString(job_id)) {

                strncpy(g_current_job_id,
                        job_id->valuestring,
                        sizeof(g_current_job_id) - 1);

                printf("Job ID: %s\n",
                       g_current_job_id);
            }

            if (cJSON_IsString(prevhash))
                printf("Prevhash: %s\n", prevhash->valuestring);

            if (cJSON_IsString(version))
                printf("Version: %s\n", version->valuestring);

            if (cJSON_IsString(nbits))
                printf("nBits: %s\n", nbits->valuestring);

            if (cJSON_IsString(ntime)) {

                strncpy(g_current_ntime,
                        ntime->valuestring,
                        sizeof(g_current_ntime) - 1);

                printf("nTime: %s\n",
                       g_current_ntime);
            }

            if (cJSON_IsArray(branches))
                printf("Merkle branches: %d\n",
                       cJSON_GetArraySize(branches));

            if (cJSON_IsString(coinb1))
                printf("Coinbase1 hex chars: %u\n",
                       (unsigned)strlen(coinb1->valuestring));

            if (cJSON_IsString(coinb2))
                printf("Coinbase2 hex chars: %u\n",
                       (unsigned)strlen(coinb2->valuestring));

            if (cJSON_IsBool(clean))
                printf("Clean jobs: %s\n",
                       cJSON_IsTrue(clean) ? "true" : "false");

            uint8_t coinbase_hash[32];

            if (cJSON_IsString(coinb1) &&
                cJSON_IsString(coinb2) &&
                qhap_build_coinbase_hash(
                    coinb1->valuestring,
                    coinb2->valuestring,
                    coinbase_hash)) {

                printf("Coinbase SHA256d: ");

                for (int i = 0; i < 32; i++)
                    printf("%02x", coinbase_hash[i]);

                printf("\nCoinbase TXID: ");

                for (int i = 31; i >= 0; i--)
                    printf("%02x", coinbase_hash[i]);

                printf("\n");

                uint8_t merkle_root[32];

                if (qhap_build_merkle_root(
                        coinbase_hash,
                        branches,
                        merkle_root)) {

                    printf("Merkle SHA256d raw: ");

                    for (int i = 0; i < 32; i++)
                        printf("%02x", merkle_root[i]);

                    printf("\nMerkle root display: ");

                    for (int i = 31; i >= 0; i--)
                        printf("%02x", merkle_root[i]);

                    printf("\n");

                    static uint8_t header[80];
                    static uint8_t header_hash[32];

                    if (cJSON_IsString(version) &&
                        cJSON_IsString(prevhash) &&
                        cJSON_IsString(ntime) &&
                        cJSON_IsString(nbits) &&
                        qhap_build_header(
                            version->valuestring,
                            prevhash->valuestring,
                            merkle_root,
                            ntime->valuestring,
                            nbits->valuestring,
                            0,
                            header)) {

                        printf("Header bytes: ");

                        for (int i = 0; i < 80; i++)
                            printf("%02x", header[i]);

                        printf("\n");

                        if (qhap_sha256d(header,
                                         sizeof(header),
                                         header_hash)) {

                            printf("Header SHA256d raw: ");

                            for (int i = 0; i < 32; i++)
                                printf("%02x", header_hash[i]);

                            printf("\nHeader hash display: ");

                            for (int i = 31; i >= 0; i--)
                                printf("%02x", header_hash[i]);

                            printf("\n");

                            uint8_t network_target[32];

                            if (qhap_nbits_to_target(
                                    nbits->valuestring,
                                    network_target)) {

                                printf("Network target:     ");

                                for (int i = 0; i < 32; i++)
                                    printf("%02x",
                                           network_target[i]);

                                printf("\nNonce 0 block-valid: %s\n",
                                       qhap_hash_meets_target(
                                           header_hash,
                                           network_target)
                                           ? "YES"
                                           : "NO");
                            }

                            uint8_t hw_hash[32];

                            if (qhap_sha256d_hw_80(header, hw_hash)) {

                                printf("Hardware SHA256d raw: ");

                                for (int i = 0; i < 32; i++)
                                    printf("%02x", hw_hash[i]);

                                printf("\nHardware vs PSA: %s\n",
                                       memcmp(hw_hash,
                                              header_hash,
                                              32) == 0
                                           ? "MATCH"
                                           : "MISMATCH");

                                memcpy(g_pending_header,
                                       header,
                                       sizeof(g_pending_header));

                                g_pending_header_valid = true;

                                if (g_pool_target_valid) {
                                    printf("Live job queued for mining\n");
                                } else {
                                    printf("Mining deferred until pool difficulty arrives\n");
                                }
                            }

                        } else {

                            printf("Header SHA256d FAILED\n");
                        }

                    } else {

                        printf("Header construction FAILED\n");
                    }

                } else {

                    printf("Merkle root construction FAILED\n");
                }

            } else {

                printf("Coinbase construction FAILED\n");
            }

            printf("========================\n");
        }
    }

    cJSON_Delete(root);
}

static int qhap_receive_available(int sock)
{
    static char recvbuf[1024];
    static char linebuf[8192];
    static size_t used = 0;

    int messages = 0;

    /*
     * Drain everything already waiting on the socket,
     * but never block the mining loop.
     */
    for (int round = 0; round < 16; round++) {

        int len = recv(sock,
                       recvbuf,
                       sizeof(recvbuf),
                       MSG_DONTWAIT);

        if (len == 0) {
            printf("STRATUM CONNECTION CLOSED\n");
            return -1;
        }

        if (len < 0)
            return messages;

        for (int i = 0; i < len; i++) {

            char ch = recvbuf[i];

            if (ch == '\n') {

                linebuf[used] = '\0';

                if (used > 0) {
                    process_stratum_line(linebuf);
                    messages++;
                }

                used = 0;

            } else if (ch != '\r') {

                if (used < sizeof(linebuf) - 1) {
                    linebuf[used++] = ch;
                } else {
                    printf("Stratum line too large\n");
                    used = 0;
                }
            }
        }
    }

    return messages;
}

void app_main(void)
{
    printf("\n=== QHAP LIVE JOB PARSER ===\n");

    if (!wifi_connect()) {
        printf("Wi-Fi FAILED\n");
        return;
    }

    char port_string[8];

    snprintf(port_string,
             sizeof(port_string),
             "%d",
             CONFIG_QHAP_STRATUM_PORT);

    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(CONFIG_QHAP_STRATUM_HOST,
                    port_string,
                    &hints,
                    &res) != 0 || res == NULL) {

        printf("DNS FAILED\n");
        return;
    }

    int sock = socket(res->ai_family,
                      res->ai_socktype,
                      res->ai_protocol);

    if (sock < 0) {
        printf("Socket FAILED\n");
        freeaddrinfo(res);
        return;
    }

    if (connect(sock,
                res->ai_addr,
                res->ai_addrlen) != 0) {

        printf("TCP CONNECTION FAILED\n");
        close(sock);
        freeaddrinfo(res);
        return;
    }

    freeaddrinfo(res);

    g_stratum_sock = sock;

    printf("STRATUM TCP CONNECTED\n");

    struct timeval timeout = {
        .tv_sec = 10,
        .tv_usec = 0
    };

    setsockopt(sock,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &timeout,
               sizeof(timeout));

    const char *subscribe =
        "{\"id\":1,\"method\":\"mining.subscribe\","
        "\"params\":[\"QHAP/0.1\"]}\n";

    send(sock,
         subscribe,
         strlen(subscribe),
         0);

    char buffer[2048];

    int len = recv(sock,
                   buffer,
                   sizeof(buffer) - 1,
                   0);

    if (len <= 0) {
        printf("NO SUBSCRIBE RESPONSE\n");
        close(sock);
        return;
    }

    buffer[len] = '\0';

    char *line_end = strchr(buffer, '\n');
    if (line_end != NULL) {
        *line_end = '\0';
    }

    cJSON *sub_root = cJSON_Parse(buffer);

    if (sub_root == NULL) {
        printf("Subscribe JSON parse FAILED\n");
        close(sock);
        return;
    }

    cJSON *sub_result = cJSON_GetObjectItem(sub_root, "result");

    if (!cJSON_IsArray(sub_result) ||
        cJSON_GetArraySize(sub_result) < 3) {

        printf("Invalid subscribe result\n");
        cJSON_Delete(sub_root);
        close(sock);
        return;
    }

    cJSON *extranonce1 =
        cJSON_GetArrayItem(sub_result, 1);

    cJSON *extranonce2_size =
        cJSON_GetArrayItem(sub_result, 2);

    if (!cJSON_IsString(extranonce1) ||
        !cJSON_IsNumber(extranonce2_size)) {

        printf("Invalid extranonce data\n");
        cJSON_Delete(sub_root);
        close(sock);
        return;
    }

    strncpy(g_extranonce1,
            extranonce1->valuestring,
            sizeof(g_extranonce1) - 1);

    g_extranonce2_size = extranonce2_size->valueint;

    printf("Subscribe response parsed\n");
    printf("Extranonce1: %s\n", g_extranonce1);
    printf("Extranonce2 size: %d bytes\n",
           g_extranonce2_size);

    cJSON_Delete(sub_root);

    char authorize[256];

    snprintf(authorize,
             sizeof(authorize),
             "{\"id\":2,\"method\":\"mining.authorize\","
             "\"params\":[\"%s\",\"%s\"]}\n",
             CONFIG_QHAP_STRATUM_USER,
             CONFIG_QHAP_STRATUM_PASSWORD);

    send(sock,
         authorize,
         strlen(authorize),
         0);

    printf("Authorization sent\n");
    printf("Waiting for live mining job...\n");

    static uint8_t active_header[80];
    char active_job_id[128] = {0};

    uint32_t next_nonce = 0;
    printf("QHAP continuous mining controller starting...\n");

    while (true) {

        int rx = qhap_receive_available(sock);

        if (rx < 0) {

            printf("Stratum connection lost - restarting QHAP in 2 seconds...\n");

            g_stratum_sock = -1;
            close(sock);

            vTaskDelay(pdMS_TO_TICKS(2000));

            esp_restart();
        }

        if (g_pool_target_valid &&
            g_pending_header_valid &&
            g_current_job_id[0] != '\0') {

            /*
             * A different Stratum job immediately resets
             * the nonce search to zero on the new header.
             */
            if (strcmp(active_job_id,
                       g_current_job_id) != 0) {

                strncpy(active_job_id,
                        g_current_job_id,
                        sizeof(active_job_id) - 1);

                active_job_id[
                    sizeof(active_job_id) - 1] = '\0';

                memcpy(active_header,
                       g_pending_header,
                       sizeof(active_header));

                next_nonce = 0;

                printf("\nActivated mining job: %s\n",
                       active_job_id);
            }

            qhap_live_nonce_scan(
                active_header,
                next_nonce);

            /*
             * Advance without repeating nonce work.
             */
            if (next_nonce > UINT32_MAX - 100000U) {

                printf("Nonce space exhausted for current job\n");

                /*
                 * Do not repeat the nonce space.
                 * Wait for fresh Stratum work.
                 */
                g_pending_header_valid = false;
                active_job_id[0] = '\0';
                next_nonce = 0;

            } else {

                next_nonce += 100000U;
            }

            /*
             * Give FreeRTOS/Wi-Fi a scheduling opportunity,
             * then return to the socket before the next chunk.
             */
            vTaskDelay(1);

        } else {

            usleep(50000);
        }
    }

    g_stratum_sock = -1;
    close(sock);

    printf("QHAP controller stopped\n");
}

























