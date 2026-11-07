/* LILA: render every world's note to a WAV and check the output is sane (no NaN, no clipping, not silent). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include "../firmware/src/lila/lila.h"

int plat_midi_in(uint32_t *pkt) { (void)pkt; return 0; }
static FILE *wav;
static uint32_t nframes;
static float peak, rms_acc; static uint32_t rms_n; static int bad;

static void run(int ms)
{
    static int32_t blk[512];
    int i, k, n = ms * 44100 / 1000 / 256;
    for (i = 0; i < n; i++) {
        engine_render(blk, 256);
        for (k = 0; k < 512; k++) {
            float f = (float)blk[k] / 8388608.0f;
            int16_t s = (int16_t)(f * 32767.0f);
            if (f != f) bad++;
            if (fabsf(f) > peak) peak = fabsf(f);
            rms_acc += f * f; rms_n++;
            if (wav) fwrite(&s, 2, 1, wav);
        }
        nframes += 256;
    }
}
static void wav_head(FILE *f, uint32_t frames)
{
    uint32_t v, bytes = frames * 4;
    fseek(f, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, f); v = 36 + bytes; fwrite(&v, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    { uint16_t a[2] = {1, 2}; uint32_t r[2] = {44100, 44100 * 4}; uint16_t b[2] = {4, 16}; fwrite(a, 2, 2, f); fwrite(r, 4, 2, f); fwrite(b, 2, 2, f); }
    fwrite("data", 1, 4, f); fwrite(&bytes, 4, 1, f);
}
int main(int argc, char **argv)
{
    int p, fails = 0;
    const char *dir = argc > 1 ? argv[1] : "build/t";
    char path[256];
    engine_init();
    engine_set_master(4096);
    for (p = 0; p < NPRESET; p++) {
        preset_load(p);
        snprintf(path, sizeof path, "%s/preset%02d_%s.wav", dir, p, PRESETS[p].name);
        wav = fopen(path, "wb"); { uint8_t z[44] = {0}; fwrite(z, 1, 44, wav); }
        nframes = 0; peak = 0; rms_acc = 0; rms_n = 0; bad = 0;
        ev_post(EV_KEY_ON, 7, 0); run(500); ev_post(EV_KEY_OFF, 7, 0); run(300);
        ev_post(EV_KEY_ON, 11, 0); ev_post(EV_KEY_ON, 14, 0); run(500); ev_post(EV_KEY_OFF, 11, 0); ev_post(EV_KEY_OFF, 14, 0); run(1200);
        wav_head(wav, nframes); fclose(wav);
        printf("%-8s peak %.3f rms %.4f nan %d\n", PRESETS[p].name, peak, sqrtf(rms_acc / rms_n), bad);
        if (bad || peak < 0.02f || peak > 1.0f) { printf("  FAIL\n"); fails++; }
    }
    /* polyphony load: 10 keys, everything on: time it */
    {
        clock_t t0; double s; int i;
        preset_load(4);
        P[P_ECHO] = 80; P[P_SPACE] = 80; P[P_FLUFF] = 80; P[P_ROBOT] = 40; P[P_BEAT] = 3; P[P_BUDDY] = 100; P[P_METAL] = 60; P[P_BIG] = 80;
        for (i = 0; i < 10; i++) ev_post(EV_KEY_ON, i * 2, 0);
        t0 = clock();
        wav = 0; run(4000);
        s = (double)(clock() - t0) / CLOCKS_PER_SEC;
        printf("load: 4 s of audio with 10 voices and all effects took %.3f s (%.1f%% of real time on this machine)\n", s, s / 4.0 * 100.0);
    }
    return fails ? 1 : 0;
}
