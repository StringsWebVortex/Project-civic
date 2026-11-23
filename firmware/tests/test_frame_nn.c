/* Host tests for frame_nn.c: learning, anomaly ordering, bounds, real replay. */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame_nn.h"

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

static uint32_t rng = 0x12345u;
static uint8_t rnd8(void) { rng = rng * 1103515245u + 12345u; return (uint8_t)(rng >> 16); }

static const uint8_t A[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
static const uint8_t B[8] = {0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80};

int main(void)
{
    fnn_t *global_network = fnn_global();
    CHECK(global_network != NULL, "global network is lazily initialized");
    CHECK(fnn_global() == global_network, "global network remains stable after initialization");

    /* 1. Learning: alternating A/B periodic payload on one identifier. */
    static fnn_t n;
    fnn_init(&n);
    CHECK(fnn_on_frame(&n, 0x158, false, 8, A) < 0.0f, "first frame has nothing to predict from");
    double early = 0.0, late = 0.0;
    const int N = 4000;
    for (int i = 0; i < N; i++) {
        const uint8_t *p = (i & 1) ? A : B;   /* after A comes B, after B comes A */
        float s = fnn_on_frame(&n, 0x158, false, 8, p);
        CHECK(s >= 0.0f && s <= 1.0f && isfinite(s), "score in [0,1]");
        if (i >= 1 && i <= 100) early += s;
        if (i >= N - 200) late += s;
    }
    early /= 100.0; late /= 200.0;
    printf("learning: early mean err %.4f, late mean err %.4f\n", early, late);
    CHECK(late < 0.5 * early, "error drops by at least half after training");
    CHECK(late < 0.05, "periodic payload is predicted well after training");

    /* 2. Anomaly ordering: expected next frame scores lower than a random one. */
    /* Last trained frame was A (i = N-1 is odd), so the next expected payload is B. */
    float expected = fnn_score(&n, 0x158, false, 8, B);
    float rnd_sum = 0.0f; int rnd_n = 200; int worse = 0;
    for (int i = 0; i < rnd_n; i++) {
        uint8_t r[8]; for (int k = 0; k < 8; k++) r[k] = rnd8();
        float s = fnn_score(&n, 0x158, false, 8, r);
        rnd_sum += s; if (s > expected) worse++;
    }
    printf("anomaly: expected-next score %.4f, random mean %.4f, random above expected %d/200\n",
           expected, rnd_sum / rnd_n, worse);
    CHECK(worse >= 190, "random payloads score above the learned expected payload");
    CHECK(fnn_score(&n, 0x158, false, 8, B) == expected, "pure scoring has no side effect");

    /* 3. Bounds and invalid input. */
    fnn_t m; fnn_init(&m);
    CHECK(fnn_on_frame(&m, 0x1, false, 0, A) < 0.0f, "dlc 0 rejected");
    CHECK(fnn_on_frame(&m, 0x1, false, 9, A) < 0.0f, "dlc 9 rejected");
    CHECK(fnn_score(&m, 0x1, false, 8, A) < 0.0f, "unknown id scores -1");
    for (uint32_t id = 0; id < FNN_SLOTS; id++) fnn_on_frame(&m, 0x100u + id, false, 8, A);
    CHECK(fnn_on_frame(&m, 0x999, false, 8, A) < 0.0f, "table full: extra id ignored");
    CHECK(fnn_on_frame(&m, 0x100, true, 8, A) < 0.0f, "extended flag is a separate id");

    /* 4. Long random run stays finite and bounded. */
    fnn_t r; fnn_init(&r);
    int bad = 0;
    for (int i = 0; i < 100000; i++) {
        uint8_t d[8]; for (int k = 0; k < 8; k++) d[k] = rnd8();
        float s = fnn_on_frame(&r, 0x200u + (uint32_t)(i % 4), false, 8, d);
        if (s >= 0.0f && (!isfinite(s) || s > 1.0f)) bad++;
    }
    for (unsigned i = 0; i < FNN_HID; i++) for (unsigned j = 0; j < FNN_IN; j++)
        if (!isfinite(r.w1[i][j])) bad++;
    for (unsigned i = 0; i < FNN_OUT; i++) for (unsigned j = 0; j < FNN_HID; j++)
        if (!isfinite(r.w2[i][j])) bad++;
    CHECK(bad == 0 && isfinite(r.ema_err), "weights and scores stay finite over 100k random frames");

    /* 5. Real traffic replay from the SavvyCAN example log (candump format). */
    const char *path = "../SavvyCAN-continuous/examples/candump.log";
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("replay: %s not found, skipped\n", path);
    } else {
        static fnn_t rp; fnn_init(&rp);
        char line[256]; long lines = 0, scored = 0; double sum = 0.0; int rep_bad = 0;
        while (fgets(line, sizeof line, f)) {
            char *hash = strchr(line, '#');
            if (!hash) continue;
            char *sp = hash; while (sp > line && sp[-1] != ' ') sp--;
            char idbuf[16] = {0};
            size_t idlen = (size_t)(hash - sp); if (idlen == 0 || idlen > 8) continue;
            memcpy(idbuf, sp, idlen);
            uint32_t id = (uint32_t)strtoul(idbuf, NULL, 16);
            uint8_t d[8] = {0}; unsigned dlc = 0;
            char *p = hash + 1;
            while (*p && *p != '\n' && *p != '\r' && dlc < 8) {
                char t[3] = { p[0], p[1], 0 };
                if (!isxdigit((unsigned char)t[0]) || !isxdigit((unsigned char)t[1])) break;
                d[dlc++] = (uint8_t)strtoul(t, NULL, 16); p += 2;
            }
            if (dlc == 0) continue;
            lines++;
            float s = fnn_on_frame(&rp, id, false, (uint8_t)dlc, d);
            if (s >= 0.0f) { scored++; sum += s; if (!isfinite(s) || s > 1.0f) rep_bad++; }
        }
        fclose(f);
        printf("replay: %ld frames, %ld scored, mean err %.4f\n", lines, scored, scored ? sum / scored : 0.0);
        CHECK(lines > 0 && scored > 0, "replay produced scored frames");
        CHECK(rep_bad == 0, "replay scores finite and in [0,1]");
    }

    if (failures) { printf("test_frame_nn: %d failure(s)\n", failures); return 1; }
    printf("test_frame_nn: all checks passed\n");
    return 0;
}
