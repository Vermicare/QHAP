#include "qhap_bm1370_candidate.h"

#include <string.h>

static uint32_t read_le32(const uint8_t *p)
{
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void write_le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

/* Bitcoin displays SHA256d with bytes reversed from the raw digest. */
static bool hash_meets_target(const uint8_t hash_raw[32],
                              const uint8_t target_be[32])
{
    for (size_t i = 0; i < 32; ++i) {
        const uint8_t h = hash_raw[31U - i];
        if (h < target_be[i]) return true;
        if (h > target_be[i]) return false;
    }
    return true;
}

void qhap_candidate_init(qhap_candidate_gate_t *gate)
{
    if (gate != NULL) {
        memset(gate, 0, sizeof(*gate));
        gate->next_generation = 1U;
    }
}

static qhap_candidate_job_t *find_job(qhap_candidate_gate_t *gate,
                                      uint8_t job_id)
{
    for (size_t i = 0; i < QHAP_CANDIDATE_MAX_JOBS; ++i) {
        if (gate->jobs[i].used && gate->jobs[i].job_id == job_id)
            return &gate->jobs[i];
    }
    return NULL;
}

bool qhap_candidate_register_job(qhap_candidate_gate_t *gate,
                                 uint8_t job_id,
                                 const uint8_t header[80],
                                 const uint8_t pool_target_be[32],
                                 const uint8_t network_target_be[32],
                                 uint32_t version_mask)
{
    if (gate == NULL || header == NULL || pool_target_be == NULL ||
        network_target_be == NULL || gate->next_generation == 0U ||
        gate->next_generation == UINT64_MAX ||
        (version_mask & ~QHAP_BM1370_ROLLABLE_BITS) != 0U ||
        find_job(gate, job_id) != NULL)
        return false;

    for (size_t i = 0; i < QHAP_CANDIDATE_MAX_JOBS; ++i) {
        qhap_candidate_job_t *job = &gate->jobs[i];
        if (job->used) continue;
        memset(job, 0, sizeof(*job));
        job->used = true;
        job->job_id = job_id;
        job->generation = gate->next_generation++;
        job->version_mask = version_mask;
        memcpy(job->header, header, 80);
        memcpy(job->pool_target_be, pool_target_be, 32);
        memcpy(job->network_target_be, network_target_be, 32);
        return true;
    }
    return false;
}

bool qhap_candidate_mark_stale(qhap_candidate_gate_t *gate, uint8_t job_id)
{
    if (gate == NULL) return false;
    qhap_candidate_job_t *job = find_job(gate, job_id);
    if (job == NULL) return false;
    job->stale = true;
    return true;
}

bool qhap_candidate_retire_job(qhap_candidate_gate_t *gate, uint8_t job_id)
{
    if (gate == NULL) return false;
    qhap_candidate_job_t *job = find_job(gate, job_id);
    if (job == NULL) return false;
    job->used = false;
    return true;
}

static bool seen_before(const qhap_candidate_gate_t *gate,
                        uint64_t generation,
                        uint32_t nonce,
                        uint32_t version)
{
    for (size_t i = 0; i < QHAP_CANDIDATE_RECENT_SHARES; ++i) {
        const qhap_candidate_seen_t *seen = &gate->recent[i];
        if (seen->used && seen->generation == generation &&
            seen->nonce == nonce && seen->version == version)
            return true;
    }
    return false;
}

static void remember_share(qhap_candidate_gate_t *gate,
                           uint64_t generation,
                           uint32_t nonce,
                           uint32_t version)
{
    qhap_candidate_seen_t *entry = &gate->recent[gate->next_recent];
    entry->used = true;
    entry->generation = generation;
    entry->nonce = nonce;
    entry->version = version;
    gate->next_recent = (gate->next_recent + 1U) % QHAP_CANDIDATE_RECENT_SHARES;
}

qhap_candidate_status_t qhap_candidate_evaluate(
    qhap_candidate_gate_t *gate,
    const uint8_t raw[QHAP_BM1370_RESULT_SIZE],
    qhap_candidate_hash_fn hash_fn,
    void *hash_context,
    qhap_candidate_evaluation_t *out)
{
    if (out == NULL) return QHAP_CANDIDATE_INVALID_INPUT;
    memset(out, 0, sizeof(*out));
    out->status = QHAP_CANDIDATE_INVALID_INPUT;
    if (gate == NULL || raw == NULL || hash_fn == NULL)
        return out->status;

    qhap_bm1370_result_t result;
    if (!qhap_bm1370_decode_result(raw, &result) ||
        !result.is_job_response) {
        out->status = QHAP_CANDIDATE_BAD_PACKET;
        return out->status;
    }
    out->job_id = result.job_id;
    out->nonce = result.nonce;

    qhap_candidate_job_t *job = find_job(gate, result.job_id);
    if (job == NULL) {
        out->status = QHAP_CANDIDATE_UNKNOWN_JOB;
        return out->status;
    }
    out->generation = job->generation;
    out->stale = job->stale;

    if ((result.version_bits & ~job->version_mask) != 0U) {
        out->status = QHAP_CANDIDATE_VERSION_REJECTED;
        return out->status;
    }

    memcpy(out->header, job->header, sizeof(out->header));
    const uint32_t base_version = read_le32(out->header);
    /* BM1370 upstream reconstruction uses bitwise OR, not XOR. */
    out->version = base_version | result.version_bits;
    write_le32(out->header, out->version);
    write_le32(out->header + 76, result.nonce);

    if (!hash_fn(out->header, out->hash_raw, hash_context)) {
        out->status = QHAP_CANDIDATE_HASH_ERROR;
        return out->status;
    }
    out->hash_available = true;
    out->pool_match = hash_meets_target(out->hash_raw, job->pool_target_be);
    out->network_match = hash_meets_target(out->hash_raw, job->network_target_be);

    /* Check network target independently: never discard a winner solely
     * because a configured pool target is inconsistent. */
    if (!out->pool_match && !out->network_match) {
        out->status = QHAP_CANDIDATE_NO_SHARE;
        return out->status;
    }

    if (seen_before(gate, job->generation, result.nonce, out->version)) {
        out->status = QHAP_CANDIDATE_DUPLICATE;
        return out->status;
    }
    remember_share(gate, job->generation, result.nonce, out->version);
    out->status = out->network_match ? QHAP_CANDIDATE_NETWORK_BLOCK
                                     : QHAP_CANDIDATE_SHARE;
    return out->status;
}
