/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's speech coder (host tool): speech in, LPC frames out, the way the Speak & Spell did it, a little finer.
 *
 *   lpc_enc IN.raw OUT.lpc          IN: mono 16-bit at 11025 Hz. OUT: LPC2 pairs of frames, 100 frames a second
 *   lpc_enc -d IN.lpc OUT.raw       decode (a plain impulse / noise excitation) to listen to the coding alone
 *   LPC_STATS=1 lpc_enc IN.raw OUT  also prints how far the decoded spectral envelope is from the exact one (dB)
 *
 * A pair is 15 bytes, 20 ms:
 *   the KEY frame (10 bytes, LSB first): 12 reflection coefficients (arcsine-quantised, 7 7 6 6 6 5 5 5 5 5 4 3 bits), the
 *     residual's level (6 bits, 1.5 dB steps), the pitch period (8 bits, log scale over four octaves, 0 = unvoiced) and how
 *     voiced it is (2 bits: 0 breathy .. 3 clean);
 *   the BETWEEN frame (5 bytes): what the average of the two neighbouring keys misses: the 12 coefficients (4 4 3 3 3 3 3 3 3 3 2 2
 *     bits, steps DSTEP) and the level (4 bits, 1.5 dB). Its pitch and voicing are the previous key's.
 * The FM-1 decodes the same format in firmware/src/lila/synth.c (story_*), where the excitation is the synth's own oscillator
 * (a smoothed glottal pulse, a little jitter, and breath noise in the less voiced frames). */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FS 11025
#define HOP 110                       /* 100 frames a second */
#define WIN 330
#define ORDER 12
#define PMIN 25                       /* the pitch period range: 441 Hz .. 27 Hz */
#define PMAX 400
#define KEY_BYTES 10
#define BTW_BYTES 5
#define PAIR_BYTES (KEY_BYTES + BTW_BYTES)
#define PREEMPH 0.9375
#define PBITS 8
#define PQ_MAX 255
static const int KBITS[ORDER] = {7, 7, 6, 6, 6, 5, 5, 5, 5, 5, 4, 3};
static const int DBITS[ORDER] = {4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2};
static const double DSTEP[ORDER] = {0.03, 0.03, 0.03, 0.03, 0.03, 0.025, 0.025, 0.02, 0.02, 0.02, 0.02, 0.02};

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

typedef struct { double k[ORDER + 1]; int gq, pq, vq; } qframe_t;
static double pq_period(int pq) { return pq ? PMIN * pow(2, (pq - 1) * 4.0 / (PQ_MAX - 1)) : 0; }
static void make_q(qframe_t *q, const double *k, double gain, double period, int vq)
{
    int j, pq = 0, gq;
    double db = 20 * log10(gain + 1);
    for (j = 1; j <= ORDER; j++)
        q->k[j] = k[j];
    gq = (int)lround(db / 1.5);
    q->gq = gq < 0 ? 0 : gq > 63 ? 63 : gq;
    if (period > 0) {
        pq = (int)lround(log2((period > PMIN ? period : PMIN) / PMIN) / 4.0 * (PQ_MAX - 1)) + 1;
        pq = pq < 1 ? 1 : pq > PQ_MAX ? PQ_MAX : pq;
    }
    q->pq = pq;
    q->vq = pq ? vq : 0;
}
static void key_pack(qframe_t *q, uint8_t *b)
{
    int pos = 0, j;
    memset(b, 0, KEY_BYTES);
    for (j = 1; j <= ORDER; j++) {
        int v = kq(q->k[j], KBITS[j - 1]);
        put_bits(b, &pos, (unsigned)v, KBITS[j - 1]);
        q->k[j] = kd(v, KBITS[j - 1]);                   /* from now on: what the decoder will have */
    }
    put_bits(b, &pos, (unsigned)q->gq, 6);
    put_bits(b, &pos, (unsigned)q->pq, PBITS);
    put_bits(b, &pos, (unsigned)q->vq, 2);
}
static void key_unpack(const uint8_t *b, qframe_t *q)
{
    int pos = 0, j;
    for (j = 1; j <= ORDER; j++)
        q->k[j] = kd((int)get_bits(b, &pos, KBITS[j - 1]), KBITS[j - 1]);
    q->gq = (int)get_bits(b, &pos, 6);
    q->pq = (int)get_bits(b, &pos, PBITS);
    q->vq = (int)get_bits(b, &pos, 2);
}
/* the frame between two keys: the prediction is their average; its pitch and voicing the previous key's (or the next one's if that is unvoiced) */
static void btw_predict(const qframe_t *a, const qframe_t *n, qframe_t *p)
{
    int j;
    for (j = 1; j <= ORDER; j++)
        p->k[j] = 0.5 * (a->k[j] + n->k[j]);
    p->gq = (a->gq + n->gq + 1) / 2;
    p->pq = a->pq ? a->pq : n->pq;
    p->vq = a->pq ? a->vq : n->vq;
}
static void btw_pack(const qframe_t *ex, const qframe_t *a, const qframe_t *n, uint8_t *b)
{
    qframe_t pr;
    int pos = 0, j, d;
    memset(b, 0, BTW_BYTES);
    btw_predict(a, n, &pr);
    for (j = 1; j <= ORDER; j++) {
        int bits = DBITS[j - 1], lo = -(1 << (bits - 1)), hi = (1 << (bits - 1)) - 1, q = (int)lround((ex->k[j] - pr.k[j]) / DSTEP[j - 1]);
        q = q < lo ? lo : q > hi ? hi : q;
        put_bits(b, &pos, (unsigned)(q - lo), bits);
    }
    d = ex->gq - pr.gq;
    d = d < -8 ? -8 : d > 7 ? 7 : d;
    put_bits(b, &pos, (unsigned)(d + 8), 4);
}
static void btw_unpack(const uint8_t *b, const qframe_t *a, const qframe_t *n, qframe_t *o)
{
    qframe_t pr;
    int pos = 0, j, g;
    btw_predict(a, n, &pr);
    *o = pr;
    for (j = 1; j <= ORDER; j++) {
        int bits = DBITS[j - 1], lo = -(1 << (bits - 1)), q = (int)get_bits(b, &pos, bits) + lo;
        double k = pr.k[j] + q * DSTEP[j - 1];
        o->k[j] = k > 0.995 ? 0.995 : k < -0.995 ? -0.995 : k;
    }
    g = pr.gq + (int)get_bits(b, &pos, 4) - 8;
    o->gq = g < 0 ? 0 : g > 63 ? 63 : g;
}
/* the log spectrum (dB) of 1 / A(z) at w, from reflection coefficients */
static double envdb(const double *kp, double w)
{
    double a[ORDER + 1] = {1}, t[ORDER + 1], re = 1, im = 0;
    int i, j;
    for (i = 1; i <= ORDER; i++) {
        memcpy(t, a, sizeof a);
        for (j = 1; j < i; j++)
            a[j] = t[j] + kp[i] * t[i - j];
        a[i] = kp[i];
    }
    for (j = 1; j <= ORDER; j++) {
        re += a[j] * cos(w * j);
        im -= a[j] * sin(w * j);
    }
    return -10 * log10(re * re + im * im + 1e-12);
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
    int *pitch, *voic;
    double *pfrac;
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
    voic = calloc((size_t)nf + 2, sizeof(int));
    pfrac = calloc((size_t)nf + 2, sizeof(double));
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
            pfrac[f] = bl;
            if (pitch[f] && bl > PMIN && bl < PMAX) {              /* a fractional lag: a parabola through the peak */
                double y0 = ncc[bl - 1], y1 = ncc[bl], y2 = ncc[bl + 1], den = y0 - 2 * y1 + y2;
                if (den < -1e-9)
                    pfrac[f] = bl + 0.5 * (y0 - y2) / den;
            }
            voic[f] = best > 0.88 ? 3 : best > 0.76 ? 2 : best > 0.62 ? 1 : 0;
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
            if (m != b) pfrac[f] = m;
        }
        if (pitch[f] && !pfrac[f]) pfrac[f] = pitch[f];
        if (!pitch[f]) voic[f] = 0;
    }
    {   /* the pairs: a key frame at every even analysis frame, and what the neighbouring keys miss for the odd one */
        long np = (nf + 1) / 2, pidx;
        qframe_t *dec = calloc((size_t)np * 2 + 2, sizeof(qframe_t));
        uint8_t *buf = calloc((size_t)(np > 0 ? np : 1), PAIR_BYTES);
        for (pidx = 0; pidx < np; pidx++) {
            long fa = pidx * 2, fb = fa + 1 < nf ? fa + 1 : nf - 1;
            qframe_t qa, qn;
            uint8_t *o = buf + pidx * PAIR_BYTES;
            make_q(&qa, kk[fa], gain[fa], pitch[fa] ? pfrac[fa] : 0, voic[fa]);
            key_pack(&qa, o);
            dec[pidx * 2] = qa;
            if (pidx + 1 < np)                                   /* the next key, quantised, to predict from: encode it first */
                make_q(&qn, kk[fa + 2 < nf ? fa + 2 : nf - 1], gain[fa + 2 < nf ? fa + 2 : nf - 1], pitch[fa + 2 < nf ? fa + 2 : nf - 1] ? pfrac[fa + 2 < nf ? fa + 2 : nf - 1] : 0, voic[fa + 2 < nf ? fa + 2 : nf - 1]);
            else
                qn = qa;
            if (pidx + 1 < np) {
                uint8_t scratch[KEY_BYTES];
                key_pack(&qn, scratch);                          /* the decoder predicts from the quantised next key */
            }
            {
                qframe_t qb;
                make_q(&qb, kk[fb], gain[fb], 0, 0);
                btw_pack(&qb, &qa, &qn, o + KEY_BYTES);
                btw_unpack(o + KEY_BYTES, &qa, &qn, &dec[pidx * 2 + 1]);
            }
        }
        if (getenv("LPC_STATS")) {                              /* the spectral distance between the exact and the decoded envelope, energy weighted */
            double acc = 0, wsum = 0;
            for (f = 0; f < nf; f++) {
                double ref[32], got[32], d = 0, mr = 0, mg = 0;
                int fq, nfreq = 32;
                for (fq = 0; fq < nfreq; fq++) {
                    double w = M_PI * (fq + 0.5) / nfreq;
                    ref[fq] = envdb(kk[f], w);
                    got[fq] = envdb(dec[f].k, w);
                    mr += ref[fq];
                    mg += got[fq];
                }
                mr /= nfreq;
                mg /= nfreq;                                      /* the level is coded apart: compare the shapes */
                for (fq = 0; fq < nfreq; fq++) {
                    double e2 = (ref[fq] - mr) - (got[fq] - mg);
                    d += e2 * e2;
                }
                acc += sqrt(d / nfreq) * en[f];
                wsum += en[f];
            }
            fprintf(stderr, "envelope distance %.2f dB (energy weighted)\n", wsum > 0 ? acc / wsum : 0.0);
        }
        fwrite(buf, PAIR_BYTES, (size_t)np, fo);
        nf = np * 2;
        free(dec);
        free(buf);
    }
    fclose(fo);
    fprintf(stderr, "%s: %ld frames (%.1f s), %ld bytes\n", out, nf, nf / 100.0, nf / 2 * PAIR_BYTES);
    return 0;
}

/* the decoder, as the firmware does it but with a plain pulse train: for listening to the coding alone */
static int decode(const char *in, const char *out)
{
    FILE *fi = fopen(in, "rb"), *fo = fopen(out, "wb");
    uint8_t cur[PAIR_BYTES], nxt[PAIR_BYTES];
    double y[ORDER + 1] = {0}, prev = 0, ph = 0, pl = 0, nzp = 0, jit = 1, shm = 1;
    unsigned rng = 1;
    int have;
    if (!fi || !fo) { perror("open"); return 1; }
    have = fread(cur, 1, PAIR_BYTES, fi) == PAIR_BYTES;
    while (have) {
        qframe_t a, n, fr[2];
        int h, more = fread(nxt, 1, PAIR_BYTES, fi) == PAIR_BYTES;
        key_unpack(cur, &a);
        if (more)
            key_unpack(nxt, &n);
        else
            n = a;
        fr[0] = a;
        btw_unpack(cur + KEY_BYTES, &a, &n, &fr[1]);
        for (h = 0; h < 2; h++) {
            double kk_[ORDER + 1], aa[ORDER + 1] = {1}, tmp[ORDER + 1], g;
            int j, i, P;
            memcpy(kk_, fr[h].k, sizeof kk_);
            g = pow(10, fr[h].gq * 1.5 / 20) - 1;
            P = fr[h].pq ? (int)lround(pq_period(fr[h].pq)) : 0;
            for (i = 1; i <= ORDER; i++) {
                memcpy(tmp, aa, sizeof aa);
                for (j = 1; j < i; j++)
                    aa[j] = tmp[j] + kk_[i] * tmp[i - j];
                aa[i] = kk_[i];
            }
            for (i = 0; i < HOP; i++) {
                double e, v, nz;
                int16_t o;
                rng = rng * 1103515245u + 12345u;
                nz = (((rng >> 16) & 0x7FFF) / 16384.0 - 1) * 1.73;
                if (P) {                         /* as the firmware: a smoothed pulse, jitter, breath in the less voiced frames */
                    double vc = fr[h].vq / 3.0, imp = 0, hp;
                    ph += jit;
                    if (ph >= P) {
                        ph -= P;
                        imp = sqrt((double)P) * shm;
                        rng = rng * 1103515245u + 12345u;
                        jit = 1 + 0.012 * (((rng >> 16) & 0x7FFF) / 16384.0 - 1);
                        rng = rng * 1103515245u + 12345u;
                        shm = 1 + 0.05 * (((rng >> 16) & 0x7FFF) / 16384.0 - 1);
                    }
                    pl += 0.7 * (imp - pl);
                    hp = (nz - nzp * 0.8) * 0.78;
                    nzp = nz;
                    e = g * (pl * 1.1 * (0.45 + 0.55 * vc) + hp * (0.12 + 0.75 * (1 - vc)));
                } else
                    e = g * nz;
                for (j = ORDER; j >= 1; j--)
                    e -= aa[j] * y[j];
                for (j = ORDER; j > 1; j--)
                    y[j] = y[j - 1];
                y[1] = e;
                v = e + PREEMPH * prev;
                prev = v;
                o = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
                fwrite(&o, 2, 1, fo);
            }
        }
        memcpy(cur, nxt, PAIR_BYTES);
        have = more;
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
