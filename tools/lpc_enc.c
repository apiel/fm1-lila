/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's speech coder (host tool): speech in, LPC frames out, the way the Speak & Spell did it.
 *
 *   lpc_enc IN.raw OUT.lpc          IN: mono 16-bit at 11025 Hz. OUT: 10-byte frames, 50 a second
 *   lpc_enc -d IN.lpc OUT.raw       decode (a plain impulse / noise excitation) to listen to the coding
 *
 * A frame: 12 reflection coefficients (arcsine-quantised, 7 7 6 6 5 5 5 5 4 4 4 4 bits), the residual's level (6 bits,
 * 1.5 dB steps) and the pitch period (7 bits, log scale, 0 = unvoiced): 75 bits in 10 bytes, LSB first. The FM-1 decodes
 * the same format in firmware/src/lila/synth.c (story_*), where the excitation is the synth's own oscillator. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FS 11025
#define HOP 220                       /* 50 frames a second */
#define WIN 440
#define ORDER 12
#define PMIN 25                       /* the pitch period range: 441 Hz .. 27 Hz */
#define PMAX 400
#define FRAME_BYTES 10
static const int KBITS[ORDER] = {7, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4};
#define PREEMPH 0.9375

static void put_bits(uint8_t *b, int *pos, unsigned v, int n)
{
    int i;
    for (i = 0; i < n; i++, (*pos)++)
        if ((v >> i) & 1u)
            b[*pos >> 3] |= (uint8_t)(1u << (*pos & 7));
}
static unsigned get_bits(const uint8_t *b, int *pos, int n)
{
    unsigned v = 0;
    int i;
    for (i = 0; i < n; i++, (*pos)++)
        v |= (unsigned)((b[*pos >> 3] >> (*pos & 7)) & 1u) << i;
    return v;
}
static int kq(double k, int bits)
{
    int m = (1 << (bits - 1)) - 1, q;
    if (k > 0.995) k = 0.995;
    if (k < -0.995) k = -0.995;
    q = (int)lround(asin(k) / (M_PI / 2) * m);
    return q + m;                                    /* offset: 0 .. 2m */
}
static double kd(int q, int bits)
{
    int m = (1 << (bits - 1)) - 1;
    return sin((double)(q - m) / m * (M_PI / 2));
}

/* Levinson-Durbin: autocorrelation r[0..p] -> reflection k[1..p] (A(z) = 1 + sum a_i z^-i), the residual energy */
static double levinson(const double *r, double *k, int p)
{
    double a[ORDER + 1] = {1}, tmp[ORDER + 1], e = r[0];
    int i, j;
    for (i = 1; i <= p; i++) {
        double acc = r[i];
        for (j = 1; j < i; j++)
            acc += a[j] * r[i - j];
        k[i] = e > 0 ? -acc / e : 0;
        if (k[i] > 0.995) k[i] = 0.995;
        if (k[i] < -0.995) k[i] = -0.995;
        memcpy(tmp, a, sizeof a);
        for (j = 1; j < i; j++)
            a[j] = tmp[j] + k[i] * tmp[i - j];
        a[i] = k[i];
        e *= 1 - k[i] * k[i];
    }
    return e;
}

static int16_t *read_all(const char *path, long *n)
{
    FILE *f = fopen(path, "rb");
    int16_t *d;
    if (!f) { perror(path); exit(1); }
    fseek(f, 0, SEEK_END);
    *n = ftell(f) / 2;
    fseek(f, 0, SEEK_SET);
    d = malloc((size_t)(*n + WIN + PMAX * 2) * 2);
    memset(d, 0, (size_t)(*n + WIN + PMAX * 2) * 2);
    if (fread(d, 2, (size_t)*n, f) != (size_t)*n) { perror("read"); exit(1); }
    fclose(f);
    return d;
}

static int encode(const char *in, const char *out)
{
    long n, nf, f, i, j;
    int16_t *s = read_all(in, &n);
    double *x = calloc((size_t)(n + WIN + PMAX * 2), sizeof(double)), *lp = calloc((size_t)(n + WIN + PMAX * 2), sizeof(double));
    double win[WIN], maxe = 1;
    int *pitch;
    double (*kk)[ORDER + 1], *gain, *en;
    FILE *fo = fopen(out, "wb");
    if (!fo) { perror(out); return 1; }
    for (i = 0; i < WIN; i++)
        win[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (WIN - 1));
    for (i = 0; i < n; i++)                           /* pre-emphasis */
        x[i] = s[i] - (i ? PREEMPH * s[i - 1] : 0);
    {   /* for the pitch: a low pass at ~900 Hz, twice */
        double a = exp(-2 * M_PI * 900.0 / FS), y1 = 0, y2 = 0;
        for (i = 0; i < n; i++) {
            y1 = (1 - a) * s[i] + a * y1;
            y2 = (1 - a) * y1 + a * y2;
            lp[i] = y2;
        }
    }
    nf = n / HOP;
    pitch = calloc((size_t)nf + 2, sizeof(int));
    kk = calloc((size_t)nf + 2, sizeof *kk);
    gain = calloc((size_t)nf + 2, sizeof(double));
    en = calloc((size_t)nf + 2, sizeof(double));
    for (f = 0; f < nf; f++) {
        long c = f * HOP;
        double r[ORDER + 1], w[WIN], e, best = 0, e0 = 0;
        int lag, bl = 0;
        for (i = 0; i < WIN; i++)
            w[i] = x[c + i] * win[i];
        for (j = 0; j <= ORDER; j++) {
            double acc = 0;
            for (i = j; i < WIN; i++)
                acc += w[i] * w[i - j];
            r[j] = acc;
        }
        r[0] *= 1.0001;                               /* a little white noise: a stabler filter */
        e = r[0] > 1e-3 ? levinson(r, kk[f], ORDER) : 0;
        if (r[0] <= 1e-3)
            for (j = 1; j <= ORDER; j++) kk[f][j] = 0;
        gain[f] = sqrt(e / (WIN * 0.397));            /* the residual's level (0.397: the Hamming window's power) */
        for (i = 0; i < WIN; i++)
            e0 += (double)s[c + i] * s[c + i];
        en[f] = e0 / WIN;
        if (en[f] > maxe)
            maxe = en[f];
        /* pitch: the normalised cross-correlation of the low-passed signal over two hops */
        {
            double ncc[PMAX + 1] = {0};
            for (lag = PMIN; lag <= PMAX; lag++) {
                double xy = 0, xx = 0, yy = 0;
                for (i = 0; i < WIN; i++) {
                    double a1 = lp[c + i], b1 = lp[c + i + lag];
                    xy += a1 * b1;
                    xx += a1 * a1;
                    yy += b1 * b1;
                }
                ncc[lag] = (xx > 0 && yy > 0) ? xy / sqrt(xx * yy) : 0;
                if (ncc[lag] > best) { best = ncc[lag]; bl = lag; }
            }
            /* octave errors: a shorter period nearly as good wins (the half, the third) */
            for (j = 2; j <= 3 && bl; j++) {
                int l2 = (int)lround((double)bl / j);
                if (l2 >= PMIN && ncc[l2] > best * 0.86) { bl = l2; best = ncc[l2]; }
            }
            pitch[f] = best > 0.5 ? bl : 0;
        }
    }
    for (f = 0; f < nf; f++)                          /* silence is unvoiced; a lone voiced or unvoiced frame follows its neighbours */
        if (en[f] < maxe * 1e-4)
            pitch[f] = 0;
    for (f = 1; f + 1 < nf; f++) {
        if (!pitch[f] && pitch[f - 1] && pitch[f + 1])
            pitch[f] = (pitch[f - 1] + pitch[f + 1]) / 2;
        else if (pitch[f] && !pitch[f - 1] && !pitch[f + 1])
            pitch[f] = 0;
        else if (pitch[f] && pitch[f - 1] && pitch[f + 1]) {   /* a median of three */
            int a = pitch[f - 1], b = pitch[f], cc = pitch[f + 1];
            int m = a > b ? (b > cc ? b : (a > cc ? cc : a)) : (a > cc ? a : (b > cc ? cc : b));
            pitch[f] = m;
        }
    }
    for (f = 0; f < nf; f++) {
        uint8_t b[FRAME_BYTES] = {0};
        int pos = 0, gq, pq = 0;
        double db = 20 * log10(gain[f] + 1);
        for (j = 1; j <= ORDER; j++)
            put_bits(b, &pos, (unsigned)kq(kk[f][j], KBITS[j - 1]), KBITS[j - 1]);
        gq = (int)lround(db / 1.5);
        gq = gq < 0 ? 0 : gq > 63 ? 63 : gq;
        put_bits(b, &pos, (unsigned)gq, 6);
        if (pitch[f]) {
            pq = (int)lround(log2((double)pitch[f] / PMIN) / 4.0 * 126) + 1;
            pq = pq < 1 ? 1 : pq > 127 ? 127 : pq;
        }
        put_bits(b, &pos, (unsigned)pq, 7);
        fwrite(b, 1, FRAME_BYTES, fo);
    }
    fclose(fo);
    fprintf(stderr, "%s: %ld frames (%.1f s), %ld bytes\n", out, nf, nf / 50.0, nf * FRAME_BYTES);
    return 0;
}

/* the decoder, as the firmware does it but with a plain pulse train: for listening to the coding alone */
static int decode(const char *in, const char *out)
{
    FILE *fi = fopen(in, "rb"), *fo = fopen(out, "wb");
    uint8_t b[FRAME_BYTES];
    double y[ORDER + 1] = {0}, prev = 0, ph = 0;
    unsigned rng = 1;
    if (!fi || !fo) { perror("open"); return 1; }
    while (fread(b, 1, FRAME_BYTES, fi) == FRAME_BYTES) {
        double k[ORDER + 1], a[ORDER + 1] = {1}, tmp[ORDER + 1], g;
        int pos = 0, j, i, P, gq;
        for (j = 1; j <= ORDER; j++)
            k[j] = kd((int)get_bits(b, &pos, KBITS[j - 1]), KBITS[j - 1]);
        gq = (int)get_bits(b, &pos, 6);
        P = (int)get_bits(b, &pos, 7);
        g = pow(10, gq * 1.5 / 20) - 1;
        P = P ? (int)lround(PMIN * pow(2, (P - 1) * 4.0 / 126)) : 0;
        for (i = 1; i <= ORDER; i++) {
            memcpy(tmp, a, sizeof a);
            for (j = 1; j < i; j++)
                a[j] = tmp[j] + k[i] * tmp[i - j];
            a[i] = k[i];
        }
        for (i = 0; i < HOP; i++) {
            double e, v;
            int16_t o;
            if (P) {
                ph += 1;
                e = 0;
                if (ph >= P) { ph -= P; e = g * sqrt((double)P); }
            } else {
                rng = rng * 1103515245u + 12345u;
                e = g * (((rng >> 16) & 0x7FFF) / 16384.0 - 1) * 1.73;
            }
            for (j = ORDER; j >= 1; j--)
                e -= a[j] * y[j];
            for (j = ORDER; j > 1; j--)
                y[j] = y[j - 1];
            y[1] = e;
            v = e + PREEMPH * prev;
            prev = v;
            o = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
            fwrite(&o, 2, 1, fo);
        }
    }
    fclose(fi);
    fclose(fo);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 4 && !strcmp(argv[1], "-d"))
        return decode(argv[2], argv[3]);
    if (argc == 3)
        return encode(argv[1], argv[2]);
    fprintf(stderr, "usage: lpc_enc IN.raw OUT.lpc | lpc_enc -d IN.lpc OUT.raw\n");
    return 2;
}
