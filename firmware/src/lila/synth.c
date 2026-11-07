/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's audio engine, in the audio interrupt (engine_render): ten voices, the effects, the arpeggiator, the melody
 * player, the beat and the looper. Every one of them runs on the sample clock, so they stay together.
 *
 * A voice is two oscillators (a morphing wave and a detuned twin), a sub, an FM partner, a little noise, a
 * state-variable filter and an envelope. The sound is a handful of numbers (P[]), read live: turning a knob changes the
 * notes that are sounding. A single float "control rate" of 8 samples runs the envelopes, the pitch and the filter.
 *
 * The UI writes P[] and posts events; this file never calls the UI. Single precision only (the FM-1's FPU has no double). */
#include "lila.h"
#include "../dsp/fastmath.h"
#include "../app/plat.h"

#define FS 44100.0f
#define CTRL 8
#define MAX_GAIN 0.80f                         /* the MASTER pot at its top: a toy for small ears */

volatile uint8_t P[NP];
volatile uint32_t eng_load_pct;

/* ------------------------------------------------------------------ shared --- */
volatile uint8_t vis_level, vis_peak, vis_voices, vis_beat_hit, mel_playing, mel_idx, mel_pos, mel_len, mel_done;
volatile uint8_t rec_state, rec_phase, arp_notes, arp_cur;
volatile int8_t vis_lfo, mel_expect = -1;
volatile uint32_t vis_beat_n;

static vev_t vring[32];
static volatile uint8_t vr_w, vr_r;
static void vis_push(int type, int note, int vel, int src)
{
    uint8_t w = vr_w;
    vring[w & 31u].type = (uint8_t)type;
    vring[w & 31u].note = (uint8_t)note;
    vring[w & 31u].vel = (uint8_t)vel;
    vring[w & 31u].src = (uint8_t)src;
    vr_w = (uint8_t)(w + 1u);
}
int vis_pop(vev_t *e)
{
    if (vr_r == vr_w)
        return 0;
    if ((uint8_t)(vr_w - vr_r) > 32u)
        vr_r = (uint8_t)(vr_w - 32u);              /* the UI fell behind: skip the oldest */
    *e = vring[vr_r & 31u];
    vr_r = (uint8_t)(vr_r + 1u);
    return 1;
}

typedef struct { uint8_t type; int16_t a, b; } uev_t;
static uev_t uq[64];
static volatile uint8_t uq_w, uq_r;
void ev_post(int type, int a, int b)
{
    uint8_t w = uq_w;
    if ((uint8_t)(w - uq_r) >= 64u)
        return;                                    /* full: drop (never happens at human speed) */
    uq[w & 63u].type = (uint8_t)type;
    uq[w & 63u].a = (int16_t)a;
    uq[w & 63u].b = (int16_t)b;
    uq_w = (uint8_t)(w + 1u);
}

static int voice_cap = NVOICE;                      /* shrinks when the audio runs out of time, grows back when it has some */
void engine_load(uint32_t pct, uint32_t frames)
{
    static uint32_t calm;
    (void)frames;
    eng_load_pct = pct;
    if (pct > 88u) {
        if (voice_cap > 5)
            voice_cap--;
        calm = 0;
    } else if (pct < 62u) {
        if (++calm > 40u && voice_cap < NVOICE) {
            voice_cap++;
            calm = 0;
        }
    } else
        calm = 0;
}

/* -------------------------------------------------------------------- maths --- */
static uint32_t rng_s = 0x1234567u;
static inline uint32_t rnd32(void)
{
    rng_s ^= rng_s << 13;
    rng_s ^= rng_s >> 17;
    rng_s ^= rng_s << 5;
    return rng_s;
}
static inline float frnd(void) { return (float)(int32_t)rnd32() * (1.0f / 2147483648.0f); }   /* [-1, 1) */

#define SINE_N 512
static float sine_tab[SINE_N + 2];
static inline float sn(float ph)                     /* ph in [0, 1) */
{
    float x = ph * (float)SINE_N;
    int i = (int)x;
    float f = x - (float)i;
    return sine_tab[i] + (sine_tab[i + 1] - sine_tab[i]) * f;
}
static inline float sn_fast(float ph)                /* no interpolation: for the FM partner, the sub and the LFO */
{
    return sine_tab[(int)(ph * (float)SINE_N + 0.5f)];
}
static inline float tri_w(float ph)
{
    float t = ph + 0.25f;
    t -= (float)(int)t;
    return 1.0f - 4.0f * fm_fabsf(t - 0.5f);
}
static inline float blep(float t, float dt)
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}
static inline float saw_w(float ph, float dt) { return 2.0f * ph - 1.0f - blep(ph, dt); }
static inline float sqr_w(float ph, float dt)
{
    float p2 = ph + 0.5f, s = ph < 0.5f ? 1.0f : -1.0f;
    if (p2 >= 1.0f)
        p2 -= 1.0f;
    return s + blep(ph, dt) - blep(p2, dt);
}
/* the wave knob: sine, triangle, saw, square, morphing (w in 0 .. 3) */
static inline float shape(float ph, float dt, float w)
{
    if (w < 1.0f) {
        float s = sn(ph);
        return s + (tri_w(ph) - s) * w;
    }
    if (w < 2.0f) {
        float t = tri_w(ph);
        return t + (saw_w(ph, dt) - t) * (w - 1.0f);
    }
    {
        float s = saw_w(ph, dt);
        return s + (sqr_w(ph, dt) - s) * (w - 2.0f);
    }
}
static inline float sat(float x)                      /* a soft clip, -+1 at -+1.5, no division */
{
    if (x > 1.5f)
        return 1.0f;
    if (x < -1.5f)
        return -1.0f;
    return x - x * x * x * 0.14814815f;
}

/* ------------------------------------------------------------- derived sound --- */
typedef struct {
    float w;                                 /* wave 0..3 */
    float buddy_ratio, buddy_lvl, sub, fm_idx, fm_ratio, noise;
    float pe_amt, pe_c;                      /* pitch start offset, semitones; its decay per control step */
    float cut_oct, k, drive, boing, keytrk;
    int ftype;
    float atk_step, dec_c, sus, rel_c;
    float vol, oct_st;
} vp_t;

static float knob(const uint8_t *s, int i) { return (float)s[i] * (1.0f / 127.0f); }

/* the sound's numbers from the knobs; cut_shift: octaves (the mood, the size); rel_mul: the release's length factor */
static void derive(vp_t *vp, const uint8_t *s, const preset_t *pr, float cut_shift, float rel_mul)
{
    float a, p, r, tau;
    vp->w = knob(s, P_WAVE) * 3.0f;
    a = knob(s, P_BUDDY);
    vp->buddy_ratio = fm_exp2f((4.0f + a * 30.0f) / 1200.0f);
    vp->buddy_lvl = a * 12.0f > 1.0f ? 1.0f : a * 12.0f;
    vp->sub = knob(s, P_BIG) * 0.9f;
    vp->fm_idx = knob(s, P_METAL) * 1.2f;
    vp->fm_ratio = (float)pr->fm_ratio4 * 0.25f;
    vp->noise = (float)pr->noise * (1.0f / 127.0f) * 0.5f;
    vp->pe_amt = (float)pr->pe_amt;
    tau = (float)pr->pe_ms * 0.004f * 0.45f + 0.002f;
    vp->pe_c = fm_expf(-(float)CTRL / (FS * tau));
    vp->cut_oct = 5.3219f + knob(s, P_CUT) * 8.2f + cut_shift;
    vp->k = 1.7f * (1.0f - knob(s, P_RESO)) + 0.07f;
    vp->ftype = s[P_FTYPE] > 2 ? 2 : s[P_FTYPE];
    vp->drive = 1.0f + knob(s, P_GRIT) * 7.0f;
    vp->boing = knob(s, P_BOING) * 5.0f;
    vp->keytrk = 0.55f;
    a = 0.002f * fm_exp2f(knob(s, P_ATK) * 10.2f);
    vp->atk_step = (float)CTRL / (FS * a);
    p = knob(s, P_PLUCK);
    vp->sus = 1.0f - p;
    tau = 2.5f * fm_powf(0.03f, p) / 2.3f;
    vp->dec_c = fm_expf(-(float)CTRL / (FS * tau));
    r = 0.012f * fm_exp2f(knob(s, P_REL) * 9.0f) * rel_mul;
    vp->rel_c = fm_expf(-(float)CTRL * 6.9f / (FS * r));
    vp->vol = (float)pr->vol * 0.01f;
    vp->oct_st = (float)pr->oct * 12.0f;
}

static vp_t g_vp;                           /* the sound on the knobs, live */
static vp_t zoo_vp[NPRESET];                /* every world's own sound, for the ZOO band */
static vp_t spark_vp;                       /* the sparkles' bell */

/* ------------------------------------------------------------------ voices --- */
enum { SRC_KEY, SRC_ARP, SRC_MEL, SRC_LOOP, SRC_SPARK };
typedef struct {
    uint8_t on, gate, stage, src;
    int16_t id;
    float note, vel, env, pe, nz, gprev;
    float pa, pb, ps, pm;                    /* phases: wave, twin, sub, FM partner */
    float s1, s2;                            /* the filter */
    uint32_t age;
    const vp_t *vp;
} voice_t;
static voice_t voice[NVOICE];
static uint32_t age_n;

/* shared controls, per control step */
typedef struct { float lfo_pitch, lfo_filter, trem, bend, wig, cut_mood; } ctl_t;
static ctl_t ctl;

static void voice_on(int src, int id, float note, float vel, const vp_t *vp)
{
    int i, pick = -1, used = 0;
    voice_t *v;
    for (i = 0; i < NVOICE; i++)
        used += voice[i].on;
    for (i = 0; i < NVOICE && used < voice_cap; i++)
        if (!voice[i].on) {
            pick = i;
            break;
        }
    if (pick < 0) {                          /* steal: the quietest releasing voice, else the oldest */
        float best = 9.0f;
        for (i = 0; i < NVOICE; i++)
            if (!voice[i].gate && voice[i].env < best) {
                best = voice[i].env;
                pick = i;
            }
        if (pick < 0) {
            uint32_t oldest = 0xFFFFFFFFu;
            for (i = 0; i < NVOICE; i++)
                if (voice[i].on && voice[i].age < oldest) {
                    oldest = voice[i].age;
                    pick = i;
                }
        }
    }
    v = &voice[pick];
    v->on = 1;
    v->gate = 1;
    v->stage = 0;
    v->src = (uint8_t)src;
    v->id = (int16_t)id;
    v->note = note + vp->oct_st;
    v->vel = vel;
    v->env = vp->atk_step >= 1.0f ? 1.0f : 0.0f;
    v->stage = vp->atk_step >= 1.0f ? 1 : 0;
    v->pe = 1.0f;
    v->nz = 1.0f;
    v->gprev = 0.0f;
    v->pa = v->pb = v->ps = v->pm = 0.0f;
    v->s1 = v->s2 = 0.0f;
    v->age = ++age_n;
    v->vp = vp;
}

static void voices_release(int id)
{
    int i;
    for (i = 0; i < NVOICE; i++)
        if (voice[i].on && voice[i].gate && voice[i].id == id) {
            voice[i].gate = 0;
            voice[i].stage = 2;
        }
}
static void voices_release_all(void)
{
    int i;
    for (i = 0; i < NVOICE; i++)
        if (voice[i].on && voice[i].gate) {
            voice[i].gate = 0;
            voice[i].stage = 2;
        }
}

/* one control step (8 samples) of a voice, added to out */
static void voice_run(voice_t *v, float *out)
{
    const vp_t *vp = v->vp;
    float st, f, incA, incB, incS, incM, fc, g, a1, a2, a3, k, gain, dgain, fmi, e;
    float pa = v->pa, pb = v->pb, ps = v->ps, pm = v->pm, s1 = v->s1, s2 = v->s2, w = vp->w;
    int i, nz_on, ft = vp->ftype, drv;

    switch (v->stage) {
    case 0:
        v->env += vp->atk_step;
        if (v->env >= 1.0f) {
            v->env = 1.0f;
            v->stage = 1;
        }
        break;
    case 1:
        v->env = vp->sus + (v->env - vp->sus) * vp->dec_c;
        if (!v->gate)
            v->stage = 2;
        else if (vp->sus < 0.001f && v->env < 0.003f) {   /* a pluck that has died: the key is still down */
            v->on = 0;
            return;
        }
        break;
    default:
        v->env *= vp->rel_c;
        if (v->env < 0.0006f) {
            v->on = 0;
            return;
        }
        break;
    }
    e = v->env;
    v->pe *= vp->pe_c;
    v->nz *= 0.9985f * 0.9985f * 0.9985f * 0.9985f;
    st = v->note + vp->pe_amt * v->pe + ctl.bend + ctl.wig + ctl.lfo_pitch;
    f = 440.0f * fm_exp2f((st - 69.0f) * (1.0f / 12.0f));
    incA = f * (1.0f / FS);
    if (incA > 0.45f)
        incA = 0.45f;
    incB = incA * vp->buddy_ratio;
    incS = incA * 0.5f;
    incM = incA * vp->fm_ratio;
    if (incB > 0.45f)                                    /* above Nyquist a phase would wrap twice a step */
        incB = 0.45f;
    if (incM > 0.49f)
        incM = 0.49f;
    fmi = vp->fm_idx * (0.3f + 0.7f * e) * (1.0f - 0.35f * (w * (1.0f / 3.0f)));
    /* filter */
    fc = fm_exp2f(vp->cut_oct + vp->keytrk * (st - 60.0f) * (1.0f / 12.0f) + vp->boing * e * (0.5f + 0.5f * v->vel) +
                  ctl.lfo_filter + ctl.cut_mood);
    if (fc > 15000.0f)
        fc = 15000.0f;
    if (fc < 25.0f)
        fc = 25.0f;
    g = fm_tanf(FM_PI * fc * (1.0f / FS));
    k = vp->k;
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
    gain = e * v->vel * vp->vol * ctl.trem * 0.34f;
    dgain = (gain - v->gprev) * (1.0f / (float)CTRL);
    gain = v->gprev;
    v->gprev = e * v->vel * vp->vol * ctl.trem * 0.34f;
    nz_on = vp->noise > 0.0f;
    drv = vp->drive > 1.02f;
    {
        float noise_a = vp->noise * (0.3f + 0.7f * v->nz), bl = vp->buddy_lvl, sub = vp->sub, norm = 1.0f / (1.0f + bl * 0.6f);
        int use_b = bl > 0.001f, use_fm = fmi > 0.002f, use_sub = sub > 0.002f;
        for (i = 0; i < CTRL; i++) {
            float x, ph, v1, v2, y;
            pa += incA;
            if (pa >= 1.0f)
                pa -= 1.0f;
            ph = pa;
            if (use_fm) {
                pm += incM;
                if (pm >= 1.0f)
                    pm -= 1.0f;
                ph = pa + fmi * sn_fast(pm);
                ph -= (float)(int)ph;
                if (ph < 0.0f)
                    ph += 1.0f;
            }
            x = shape(ph, incA, w);
            if (use_b) {
                pb += incB;
                if (pb >= 1.0f)
                    pb -= 1.0f;
                x = (x + shape(pb, incB, w) * bl) * norm;
            }
            if (use_sub) {
                ps += incS;
                if (ps >= 1.0f)
                    ps -= 1.0f;
                x += sn_fast(ps) * sub;
            }
            if (nz_on)
                x += frnd() * noise_a;
            if (drv)
                x = sat(x * vp->drive) * (1.0f / (0.55f + 0.45f * vp->drive * 0.25f));
            v2 = x - s2;
            v1 = a1 * s1 + a2 * v2;
            y = s2 + a2 * s1 + a3 * v2;
            s1 = 2.0f * v1 - s1;
            s2 = 2.0f * y - s2;
            if (ft == 0)
                x = y;
            else if (ft == 1)
                x = v1 * k;
            else
                x = x - k * v1 - y;
            out[i] += x * gain;
            gain += dgain;
        }
    }
    v->pa = pa;
    v->pb = pb;
    v->ps = ps;
    v->pm = pm;
    v->s1 = s1;
    v->s2 = s2;
}

/* -------------------------------------------------------------- the scheduler --- */
enum { SC_ON, SC_OFF, SC_DRUM };
typedef struct {
    uint32_t t;
    uint8_t type, src, note, vel;
    int16_t id;
    const vp_t *vp;
    uint8_t used;
} sched_t;
#define NSCHED 40
static sched_t sched[NSCHED];
static uint32_t now_s;

static void sched_add(uint32_t t, int type, int src, int id, int note, int vel, const vp_t *vp)
{
    int i;
    for (i = 0; i < NSCHED; i++)
        if (!sched[i].used) {
            sched[i].used = 1;
            sched[i].t = t;
            sched[i].type = (uint8_t)type;
            sched[i].src = (uint8_t)src;
            sched[i].id = (int16_t)id;
            sched[i].note = (uint8_t)note;
            sched[i].vel = (uint8_t)vel;
            sched[i].vp = vp;
            return;
        }
}

/* ---------------------------------------------------------------- the notes --- */
static const int8_t FRIEND_A[4] = {0, 12, 7, 4};
static const int8_t FRIEND_B[4] = {0, 0, 0, 7};
static const uint8_t SNAP_PENTA[12] = {0, 2, 2, 4, 4, 7, 7, 7, 9, 9, 12, 12};
static const uint8_t SNAP_MAJOR[12] = {0, 2, 2, 4, 4, 5, 7, 7, 9, 9, 11, 11};

static int key_note(int key)                           /* a key's note: octave, the easy scale, the key */
{
    int n = KEY0_NOTE + key + 12 * ((int)P[P_OCT] - 3), pc, oc;
    if (P[P_EASY]) {
        pc = ((n % 12) + 12) % 12;
        oc = n - pc;
        n = oc + (P[P_EASY] == 1 ? SNAP_PENTA[pc] : SNAP_MAJOR[pc]);
    }
    return n + (int)P[P_KEY] - 6;
}

/* play a note on the keys, the arpeggiator, a melody or the loop: the friends come along */
static void play_note(int src, int id, int note, int vel)
{
    const vp_t *vp = &g_vp;
    int n = (int)(P[P_FRIENDS] & 3u);
    float v = (float)vel * (1.0f / 127.0f);
    if (src == SRC_MEL && P[P_MBAND])
        vp = &zoo_vp[rnd32() % NPRESET];
    voice_on(src, id, (float)note, v, vp);
    if (n) {
        voice_on(src, id, (float)(note + FRIEND_A[n]), v * 0.85f, vp);
        if (FRIEND_B[n])
            voice_on(src, id, (float)(note + FRIEND_B[n]), v * 0.85f, vp);
    }
    vis_push(src == SRC_MEL ? VE_MELODY_NOTE : src == SRC_ARP ? VE_ARP : src == SRC_LOOP ? VE_LOOP : VE_NOTE, note, vel, src);
    if (src != SRC_SPARK && P[P_STARS] && (int)(rnd32() & 127u) < (int)P[P_STARS]) {   /* a little fairy dust after the note */
        int step = (int)(FS * 0.07f);
        sched_add(now_s + (uint32_t)step, SC_ON, SRC_SPARK, 0x400 | (note & 127), note + 12, vel * 6 / 10, &spark_vp);
        sched_add(now_s + (uint32_t)(step * 2), SC_ON, SRC_SPARK, 0x400 | (note & 127), note + 19, vel / 2, &spark_vp);
        sched_add(now_s + (uint32_t)(step * 3), SC_ON, SRC_SPARK, 0x400 | (note & 127), note + 24, vel * 4 / 10, &spark_vp);
        sched_add(now_s + (uint32_t)(step * 3 + 3000), SC_OFF, SRC_SPARK, 0x400 | (note & 127), 0, 0, 0);
    }
}

static inline uint32_t beat16(void)                    /* samples in a sixteenth */
{
    return (uint32_t)(FS * 15.0f / (float)BPM_OF(P[P_TEMPO]));
}

/* ---------------------------------------------------------------- the arpeggiator --- */
#define ARP_MAX 12
static uint8_t arp_held[ARP_MAX];                       /* note numbers, sorted */
static int arp_cnt, arp_idx, arp_dir = 1, keys_down;
static uint32_t arp_next;
static uint8_t arp_last_mode;

static void arp_add(int n)
{
    int i, j;
    for (i = 0; i < arp_cnt; i++)
        if (arp_held[i] == n)
            return;
    if (arp_cnt >= ARP_MAX)
        return;
    for (i = 0; i < arp_cnt && arp_held[i] < n; i++)
        ;
    for (j = arp_cnt; j > i; j--)
        arp_held[j] = arp_held[j - 1];
    arp_held[i] = (uint8_t)n;
    arp_cnt++;
}
static void arp_del(int n)
{
    int i;
    for (i = 0; i < arp_cnt; i++)
        if (arp_held[i] == n) {
            for (; i + 1 < arp_cnt; i++)
                arp_held[i] = arp_held[i + 1];
            arp_cnt--;
            return;
        }
}
static const float ARP_DIV[4] = {1.0f, 0.5f, 0.25f, 0.125f};      /* beats */

static void arp_tick(void)
{
    int mode = P[P_ARP], total, rng = (int)P[P_ARPRNG] + 1, i, note;
    uint32_t step;
    if (!mode)
        return;
    if (!arp_cnt) {
        arp_notes = 0;
        arp_cur = 0;
        arp_next = now_s;
        return;
    }
    if ((int32_t)(now_s - arp_next) < 0)
        return;
    total = arp_cnt * rng;
    if (arp_idx >= total || arp_idx < 0)
        arp_idx = mode == 2 ? total - 1 : 0;
    if (mode == 4)
        i = (int)(rnd32() % (uint32_t)total);
    else
        i = arp_idx;
    note = arp_held[i % arp_cnt] + 12 * (i / arp_cnt);
    play_note(SRC_ARP, (SRC_ARP << 8) | (note & 127), note, 100);
    arp_cur = (uint8_t)note;
    step = (uint32_t)(FS * 60.0f / (float)BPM_OF(P[P_TEMPO]) * ARP_DIV[P[P_ARPSPD] & 3]);
    sched_add(now_s + step * 55u / 100u, SC_OFF, SRC_ARP, (SRC_ARP << 8) | (note & 127), 0, 0, 0);
    arp_next += step;
    if ((int32_t)(now_s - arp_next) > 0)
        arp_next = now_s + step;
    if (mode == 1)
        arp_idx = (arp_idx + 1) % total;
    else if (mode == 2)
        arp_idx = (arp_idx + total - 1) % total;
    else if (mode == 3) {
        if (total < 2)
            arp_idx = 0;
        else {
            arp_idx += arp_dir;
            if (arp_idx >= total - 1) {
                arp_idx = total - 1;
                arp_dir = -1;
            } else if (arp_idx <= 0) {
                arp_idx = 0;
                arp_dir = 1;
            }
        }
    }
}

/* ------------------------------------------------------------------- melodies --- */
#define MEL_MAX 96
static uint8_t mel_n[NMEL][MEL_MAX], mel_l[NMEL][MEL_MAX], mel_cnt[NMEL];
static const uint8_t LEN_UNITS[10] = {1, 2, 4, 6, 8, 3, 12, 0, 16, 0};   /* '0'.. '9' */

static void melodies_parse(void)
{
    int m;
    for (m = 0; m < NMEL; m++) {
        const char *s = MELODIES[m].notes;
        int n = 0;
        while (*s && n < MEL_MAX) {
            int pc = -1, rest = 0, oct, len;
            char c = *s;
            if (c == ' ') {
                s++;
                continue;
            }
            if (c == 'r')
                rest = 1;
            else if (c >= 'a' && c <= 'g') {
                static const int8_t PCS[7] = {9, 11, 0, 2, 4, 5, 7};
                pc = PCS[c - 'a'];
            } else {
                s++;
                continue;
            }
            s++;
            if (!rest) {
                if (*s == '#' && s[1] >= '0' && s[1] <= '9') {
                    pc++;
                    s++;
                } else if (*s == 'b' && s[1] >= '0' && s[1] <= '9') {
                    pc--;
                    s++;
                }
                oct = *s - '0';
                s++;
            } else
                oct = 0;
            len = LEN_UNITS[(*s - '0') % 10];
            if (*s)
                s++;
            mel_n[m][n] = (uint8_t)(rest ? 0 : 12 * (oct + 1) + pc);
            mel_l[m][n] = (uint8_t)(len ? len : 4);
            n++;
        }
        mel_cnt[m] = (uint8_t)n;
    }
}
const char *melody_name(int i) { return MELODIES[(unsigned)i % NMEL].name; }
int melody_count(void) { return NMEL; }
int melody_notes(int i, const uint8_t **notes, const uint8_t **lens)
{
    i = (int)((unsigned)i % NMEL);
    *notes = mel_n[i];
    *lens = mel_l[i];
    return mel_cnt[i];
}

static uint32_t mel_next;
static int mel_i;                                       /* the index of the next note */
static void mel_set_expect(void)
{
    int note, base, k;
    while (mel_i < (int)mel_cnt[mel_idx] && mel_n[mel_idx][mel_i] == 0)
        mel_i++;                                         /* rests have no key */
    mel_pos = (uint8_t)mel_i;
    if (mel_i >= (int)mel_cnt[mel_idx]) {
        mel_expect = -1;
        return;
    }
    note = mel_n[mel_idx][mel_i];
    base = KEY0_NOTE + 12 * ((int)P[P_OCT] - 3);
    k = note - base;
    while (k < 0)
        k += 12;
    while (k > 26)
        k -= 12;
    mel_expect = (int8_t)k;
}

static void mel_start(int idx)
{
    voices_release_all();
    mel_idx = (uint8_t)((unsigned)idx % NMEL);
    mel_len = mel_cnt[mel_idx];
    mel_i = 0;
    mel_pos = 0;
    mel_playing = 1;
    mel_next = now_s;
    mel_set_expect();
}
static void mel_stop(void)
{
    mel_playing = 0;
    mel_expect = -1;
    voices_release_all();
}
static void mel_end(void)                                /* the melody's last note has been played */
{
    mel_done++;
    if (P[P_MREPEAT] == 0)
        mel_stop();
    else {
        if (P[P_MREPEAT] == 2)
            mel_idx = (uint8_t)((mel_idx + 1u) % NMEL);
        mel_len = mel_cnt[mel_idx];
        mel_i = 0;
        mel_next = now_s + beat16() * 4u;                /* a short breath */
        mel_set_expect();
    }
}
static void mel_play_one(void)                           /* play the note at mel_i and move on */
{
    int note = mel_n[mel_idx][mel_i], len = mel_l[mel_idx][mel_i];
    uint32_t dur = beat16() * (uint32_t)len;
    int id = (SRC_MEL << 8) | (note & 127);
    play_note(SRC_MEL, id, note + (int)P[P_KEY] - 6, 105);
    sched_add(now_s + dur * 88u / 100u, SC_OFF, SRC_MEL, id, 0, 0, 0);
    mel_i++;
    mel_next += dur;
    if (mel_i >= (int)mel_cnt[mel_idx])
        mel_end();
    else
        mel_set_expect();
}
static void mel_tick(void)
{
    if (!mel_playing || P[P_MMODE] == 1)
        return;
    while (mel_playing && (int32_t)(now_s - mel_next) >= 0) {
        if (mel_i < (int)mel_cnt[mel_idx] && mel_n[mel_idx][mel_i] == 0) {   /* a rest */
            mel_next += beat16() * mel_l[mel_idx][mel_i];
            mel_i++;
            if (mel_i >= (int)mel_cnt[mel_idx])
                mel_end();
            else
                mel_set_expect();
            continue;
        }
        if (mel_i >= (int)mel_cnt[mel_idx]) {
            mel_end();
            continue;
        }
        mel_play_one();
    }
}
/* follow-me: a key with the right pitch plays the next note */
static void mel_follow_key(int key)
{
    int note, want;
    if (!mel_playing || P[P_MMODE] != 1 || mel_i >= (int)mel_cnt[mel_idx])
        return;
    note = KEY0_NOTE + key + 12 * ((int)P[P_OCT] - 3);
    want = mel_n[mel_idx][mel_i];
    if (((note - want) % 12) != 0)
        return;
    mel_next = now_s;
    mel_play_one();
}

/* ----------------------------------------------------------------- the beat --- */
typedef struct { float kph, kf, ka, sa, sta, stph, ha, hprev, ca, cprev; uint32_t cburst; } drums_t;
static drums_t dr;
static uint32_t beat_next;
static int beat_step, beat_on;

static void drum_hit(int mask, int vel)
{
    float tune = fm_exp2f(((float)P[P_TUNING] - 64.0f) * (1.0f / 64.0f)), v = (float)vel * (1.0f / 127.0f);
    if (mask & 1) {
        dr.kf = 150.0f * tune;
        dr.ka = v;
    }
    if (mask & 2) {
        dr.sa = v * 0.7f;
        dr.sta = v;
    }
    if (mask & 4)
        dr.ha = v * 0.8f;
    if (mask & 8) {
        dr.ca = v;
        dr.cburst = 2200u;
    }
    vis_beat_hit |= (uint8_t)mask;
}
static void beat_tick(void)
{
    int style = (int)P[P_BEAT];
    uint32_t s16;
    if (!style) {
        beat_on = 0;
        return;
    }
    s16 = beat16();
    if (!beat_on) {
        beat_on = 1;
        beat_step = 0;
        beat_next = now_s;
    }
    while ((int32_t)(now_s - beat_next) >= 0) {
        const beat_t *b = &BEATS[(style - 1) % NBEAT];
        int mask = 0, bit = 1 << (beat_step & 15);
        uint32_t late = (beat_step & 1) ? (uint32_t)((float)s16 * 0.5f * (float)P[P_BOUNCE] * (1.0f / 127.0f)) : 0u;
        if (b->kick & bit)
            mask |= 1;
        if (b->snare & bit)
            mask |= 2;
        if (b->hat & bit)
            mask |= 4;
        if (b->clap & bit)
            mask |= 8;
        if (mask) {
            if (late > 8u)
                sched_add(now_s + late, SC_DRUM, 0, 0, mask, 100 + (beat_step % 4 == 0 ? 20 : 0), 0);
            else
                drum_hit(mask, 100 + (beat_step % 4 == 0 ? 20 : 0));
        }
        vis_beat_n = vis_beat_n + 1u;
        beat_step = (beat_step + 1) & 15;
        beat_next += s16;
    }
}
static void drums_run(float *out, int n)
{
    int i;
    float vol = (float)P[P_BEATVOL] * (1.0f / 127.0f) * 0.55f, tune = fm_exp2f(((float)P[P_TUNING] - 64.0f) * (1.0f / 64.0f));
    if (!beat_on && dr.ka < 0.001f && dr.sa < 0.001f && dr.ha < 0.001f && dr.ca < 0.001f && dr.sta < 0.001f)
        return;
    for (i = 0; i < n; i++) {
        float y = 0.0f, nz = frnd();
        if (dr.ka > 0.0005f) {
            dr.kf += (46.0f * tune - dr.kf) * 0.0011f;
            dr.kph += dr.kf * (1.0f / FS);
            if (dr.kph >= 1.0f)
                dr.kph -= 1.0f;
            y += sn(dr.kph) * dr.ka * 1.1f;
            dr.ka *= 0.99984f;
        }
        if (dr.sa > 0.0005f || dr.sta > 0.0005f) {
            dr.stph += 190.0f * tune * (1.0f / FS);
            if (dr.stph >= 1.0f)
                dr.stph -= 1.0f;
            y += (nz * dr.sa * 0.7f + sn(dr.stph) * dr.sta * 0.45f);
            dr.sa *= 0.99965f;
            dr.sta *= 0.9990f;
        }
        if (dr.ha > 0.0005f) {
            float hp = nz - dr.hprev;
            dr.hprev = nz;
            y += hp * dr.ha * 0.35f;
            dr.ha *= 0.9991f;
        }
        if (dr.ca > 0.0005f) {
            float hp = nz - dr.cprev * 0.7f;
            dr.cprev = nz;
            y += hp * dr.ca * 0.45f;
            dr.ca *= 0.99965f;
        }
        if (dr.cburst) {                                 /* a clap is three quick bursts */
            dr.cburst--;
            if (dr.cburst == 1500u || dr.cburst == 750u)
                dr.ca = 0.9f;
        }
        out[i] += y * vol;
    }
}

/* ------------------------------------------------------------------ the looper --- */
#define REC_MAX 160
typedef struct { uint32_t t; uint8_t on, note; } rev_t;
static rev_t rec_ev[REC_MAX];
static int rec_n;
static uint32_t rec_len, rec_t0;
static float rec_pos, rec_full;
static int rec_play_i;
static uint8_t rec_down[NKEYS_L];

static uint32_t rec_loop_samples(void)
{
    static const uint8_t BARS[4] = {1, 2, 4, 8};
    return (uint32_t)(FS * 60.0f / (float)BPM_OF(P[P_TEMPO]) * 4.0f * (float)BARS[P[P_RLEN] & 3]);
}
static void rec_finish(int early)
{
    uint32_t beat = (uint32_t)(FS * 60.0f / (float)BPM_OF(P[P_TEMPO])), el = now_s - rec_t0;
    int k;
    if (early) {
        rec_len = ((el + beat - 1u) / beat) * beat;
        if (rec_len < beat)
            rec_len = beat;
    } else
        rec_len = rec_full > 0.0f ? (uint32_t)rec_full : el;
    for (k = 0; k < NKEYS_L; k++)                         /* keys still down end with the loop */
        if (rec_down[k] && rec_n < REC_MAX) {
            rec_ev[rec_n].t = rec_len - 1u;
            rec_ev[rec_n].on = 0;
            rec_ev[rec_n].note = (uint8_t)key_note(k);
            rec_n++;
            rec_down[k] = 0;
        }
    if (rec_n == 0) {
        rec_state = REC_IDLE;
        return;
    }
    rec_pos = 0.0f;
    rec_play_i = 0;
    rec_state = REC_LOOPING;
}
static void rec_event(int on, int key)
{
    if (rec_state == REC_ARMED && on) {
        rec_state = REC_RECORDING;
        rec_t0 = now_s;
        rec_full = (float)rec_loop_samples();
        rec_n = 0;
        for (int q = 0; q < NKEYS_L; q++)
            rec_down[q] = 0;
    }
    if (rec_state != REC_RECORDING || rec_n >= REC_MAX)
        return;
    rec_ev[rec_n].t = now_s - rec_t0;
    rec_ev[rec_n].on = (uint8_t)on;
    rec_ev[rec_n].note = (uint8_t)key_note(key);
    rec_down[key] = (uint8_t)on;
    rec_n++;
}
static void rec_tick(void)
{
    if (rec_state == REC_RECORDING) {
        uint32_t el = now_s - rec_t0;
        rec_phase = (uint8_t)(rec_full > 0.0f ? (el * 255.0f / rec_full > 255.0f ? 255.0f : el * 255.0f / rec_full) : 0.0f);
        if (rec_full > 0.0f && (float)el >= rec_full)
            rec_finish(0);
    } else if (rec_state == REC_LOOPING) {
        float sp = P[P_RSPEED] == 0 ? 0.5f : P[P_RSPEED] == 2 ? 2.0f : 1.0f, np = rec_pos + (float)CTRL * sp;
        int shift = (int)P[P_RFUNNY] - 12;
        for (;;) {
            if (rec_play_i < rec_n && (float)rec_ev[rec_play_i].t < np) {
                const rev_t *e = &rec_ev[rec_play_i++];
                int id = (SRC_LOOP << 8) | (e->note & 127);
                if (e->on)
                    play_note(SRC_LOOP, id, (int)e->note + shift, (int)((float)P[P_RVOL] * 0.9f) + 8);
                else
                    voices_release(id);
                continue;
            }
            break;
        }
        rec_pos = np;
        if (rec_pos >= (float)rec_len) {
            rec_pos -= (float)rec_len;
            rec_play_i = 0;
            voices_release_all();
        }
        rec_phase = (uint8_t)(rec_pos * 255.0f / (float)rec_len);
    }
}
static void rec_press(void)
{
    if (rec_state == REC_IDLE)
        rec_state = REC_ARMED;
    else if (rec_state == REC_ARMED)
        rec_state = REC_IDLE;
    else if (rec_state == REC_RECORDING)
        rec_finish(1);
    else {
        rec_state = REC_IDLE;
        rec_n = 0;
        voices_release_all();
    }
    rec_phase = 0;
}

/* --------------------------------------------------------------- the events --- */
static void key_on(int key)
{
    int n = key_note(key), mode = (int)P[P_ARP];
    keys_down++;
    rec_event(1, key);
    mel_follow_key(key);
    if (mode) {
        if (!P[P_STICKY] || keys_down == 1) {
            if (keys_down == 1 && P[P_STICKY])
                arp_cnt = 0;
        }
        arp_add(n);
        arp_notes = (uint8_t)arp_cnt;
        return;
    }
    play_note(SRC_KEY, (SRC_KEY << 8) | key, n, 110);
}
static void key_off(int key)
{
    int n = key_note(key);
    if (keys_down > 0)
        keys_down--;
    rec_event(0, key);
    if (P[P_ARP]) {
        if (!P[P_STICKY])
            arp_del(n);
        arp_notes = (uint8_t)arp_cnt;
        return;
    }
    voices_release((SRC_KEY << 8) | key);
}

/* USB MIDI notes from a keyboard or a computer play the sound directly (no octave, easy scale or key: the notes are the notes) */
static void midi_poll(void)
{
    uint32_t pkt;
    int guard = 0;
    while (guard++ < 16 && plat_midi_in(&pkt)) {
        int cin = (int)(pkt & 15u), st = (int)((pkt >> 8) & 0xFFu), n = (int)((pkt >> 16) & 0x7Fu), v = (int)((pkt >> 24) & 0x7Fu);
        int id = (SRC_KEY << 8) | (0x80 | (n & 0x7F));
        if (cin == 9 && (st & 0xF0) == 0x90 && v > 0)
            play_note(SRC_KEY, id, n, v);
        else if (cin == 8 || (cin == 9 && (st & 0xF0) == 0x90))
            voices_release(id);
        else if (cin == 0xB && (st & 0xF0) == 0xB0 && n == 123)   /* all notes off */
            voices_release_all();
    }
}

static void process_events(void)
{
    midi_poll();
    while (uq_r != uq_w) {
        const uev_t e = uq[uq_r & 63u];
        uq_r = (uint8_t)(uq_r + 1u);
        switch (e.type) {
        case EV_KEY_ON:
            if (e.a >= 0 && e.a < NKEYS_L)
                key_on(e.a);
            break;
        case EV_KEY_OFF:
            if (e.a >= 0 && e.a < NKEYS_L)
                key_off(e.a);
            break;
        case EV_PLAY:
            mel_start(e.a);
            if (beat_on)
                beat_on = 0;                           /* the beat starts again with the melody, together */
            break;
        case EV_STOP:
            mel_stop();
            break;
        case EV_MELODY:
            if (mel_playing)
                mel_start(e.a);
            else
                mel_idx = (uint8_t)((unsigned)e.a % NMEL);
            break;
        case EV_REC:
            rec_press();
            break;
        case EV_PANIC:
            voices_release_all();
            arp_cnt = 0;
            arp_notes = 0;
            keys_down = 0;
            break;
        case EV_ARP_CLEAR:
            arp_cnt = 0;
            arp_notes = 0;
            if (!P[P_ARP]) {
                voices_release_all();
            }
            break;
        case EV_BEAT_RESET:
            beat_on = 0;
            break;
        default:
            break;
        }
    }
}

/* -------------------------------------------------------------------- effects --- */
#define ECHO_N 22050
static int16_t echo_l[ECHO_N] X0X_POOL, echo_r[ECHO_N] X0X_POOL;
static int echo_w;
#define CH_N 2048
static float ch_buf[CH_N] X0X_POOL;
static int ch_w;
static float ch_ph1, ch_ph2;
static const uint16_t COMB_L[4] = {1116, 1188, 1277, 1356}, COMB_R[4] = {1139, 1211, 1300, 1379};
static const uint16_t AP_L[2] = {556, 441}, AP_R[2] = {579, 464};
static float rv_comb[2][4][1400] X0X_POOL;
static float rv_ap[2][2][600] X0X_POOL;
static int rv_ci[2][4], rv_ai[2][2];
static float rv_damp[2][4];
static float rob_hold, rob_ph;
static int rob_cnt;
static float master_g = 0.5f;

void engine_set_master(uint32_t q12)
{
    if (q12 > 4096u)
        q12 = 4096u;
    master_g = (float)q12 * (1.0f / 4096.0f) * MAX_GAIN;
}

/* the world's mood: DAY as it is, NIGHT darker and dreamier, MAGIC sparkly, CRAZY wild */
static void mood_fx(int mood, float *cut_shift, float *rel_mul, int *echo, int *space, int *fluff, int *robot)
{
    *cut_shift = 0.0f;
    *rel_mul = 1.0f;
    if (mood == MOOD_NIGHT) {
        *cut_shift = -0.9f;
        *rel_mul = 1.6f;
        *space += 34;
    } else if (mood == MOOD_MAGIC) {
        *cut_shift = 0.35f;
        *fluff += 44;
        *echo += 26;
        *space += 22;
    } else if (mood == MOOD_CRAZY) {
        *robot += 34;
    }
    if (*echo > 127)
        *echo = 127;
    if (*space > 127)
        *space = 127;
    if (*fluff > 127)
        *fluff = 127;
    if (*robot > 127)
        *robot = 127;
}

static void fx_block(const float *mono, const float *drums, int32_t *out, int n, int echo, int space, int fluff, int robot)
{
    int i, ci, ai, c, ech_d;
    float beat = 60.0f / (float)BPM_OF(P[P_TEMPO]), dsec = beat * 0.75f, fb, wetE, wetR, fbR, mixC, bits, levels;
    int hold_n, ring_on;
    float ring_mix;
    while (dsec > 0.5f)
        dsec *= 0.5f;
    ech_d = (int)(dsec * FS);
    if (ech_d >= ECHO_N)
        ech_d = ECHO_N - 1;
    fb = 0.28f + 0.30f * (float)echo * (1.0f / 127.0f);
    wetE = (float)echo * (1.0f / 127.0f) * 0.75f;
    fbR = 0.72f + 0.2f * (float)space * (1.0f / 127.0f);
    wetR = (float)space * (1.0f / 127.0f) * 0.45f;
    mixC = (float)fluff * (1.0f / 127.0f) * 0.7f;
    bits = 12.0f - (float)robot * (1.0f / 127.0f) * 9.0f;
    levels = fm_exp2f(bits - 1.0f);
    hold_n = 1 + (int)((float)robot * (1.0f / 127.0f) * 7.0f);
    ring_on = robot > 70;
    ring_mix = ring_on ? (float)(robot - 70) * (1.0f / 57.0f) * 0.8f : 0.0f;
    for (i = 0; i < n; i++) {
        float x = mono[i], l, r, rl, rr, e_l, e_r, in_r, d0, d1, c1, c2;
        int ri;
        if (robot > 0) {
            if (rob_cnt <= 0) {
                rob_hold = (float)(int)(x * levels + (x < 0.0f ? -0.5f : 0.5f)) / levels;
                rob_cnt = hold_n;
            }
            rob_cnt--;
            x = rob_hold;
            if (ring_on) {
                rob_ph += 170.0f * (1.0f / FS);
                if (rob_ph >= 1.0f)
                    rob_ph -= 1.0f;
                x = x * (1.0f - ring_mix) + x * sn(rob_ph) * ring_mix * 1.4f;
            }
        }
        /* chorus: two taps, one a side */
        ch_buf[ch_w] = x;
        ch_ph1 += 0.45f / FS;
        ch_ph2 += 0.61f / FS;
        if (ch_ph1 >= 1.0f)
            ch_ph1 -= 1.0f;
        if (ch_ph2 >= 1.0f)
            ch_ph2 -= 1.0f;
        l = x;
        r = x;
        if (fluff > 0) {
            float t1 = (13.0f + 4.0f * sn(ch_ph1)) * 0.001f * FS, t2 = (19.0f + 5.0f * sn(ch_ph2)) * 0.001f * FS;
            int i1 = (int)t1, i2 = (int)t2;
            float f1 = t1 - (float)i1, f2 = t2 - (float)i2;
            float a0 = ch_buf[(ch_w - i1) & (CH_N - 1)], a1 = ch_buf[(ch_w - i1 - 1) & (CH_N - 1)];
            float b0 = ch_buf[(ch_w - i2) & (CH_N - 1)], b1 = ch_buf[(ch_w - i2 - 1) & (CH_N - 1)];
            c1 = a0 + (a1 - a0) * f1;
            c2 = b0 + (b1 - b0) * f2;
            l = x * (1.0f - mixC * 0.4f) + c1 * mixC;
            r = x * (1.0f - mixC * 0.4f) + c2 * mixC;
        }
        ch_w = (ch_w + 1) & (CH_N - 1);
        /* echo: ping-pong */
        ri = echo_w - ech_d;
        if (ri < 0)
            ri += ECHO_N;
        e_l = (float)echo_l[ri] * (1.0f / 16384.0f);
        e_r = (float)echo_r[ri] * (1.0f / 16384.0f);
        in_r = (l + r) * 0.5f;
        d0 = in_r + e_r * fb;
        d1 = e_l * fb;
        if (d0 > 1.9f)
            d0 = 1.9f;
        if (d0 < -1.9f)
            d0 = -1.9f;
        if (d1 > 1.9f)
            d1 = 1.9f;
        if (d1 < -1.9f)
            d1 = -1.9f;
        echo_l[echo_w] = (int16_t)(d0 * 16384.0f);
        echo_r[echo_w] = (int16_t)(d1 * 16384.0f);
        echo_w++;
        if (echo_w >= ECHO_N)
            echo_w = 0;
        l += e_l * wetE;
        r += e_r * wetE;
        /* reverb */
        {
            float inp = (l + r) * 0.5f * 0.12f, sl = 0.0f, sr = 0.0f;
            for (c = 0; c < 4; c++) {
                float o, *bf;
                ci = rv_ci[0][c];
                bf = &rv_comb[0][c][ci];
                o = *bf;
                rv_damp[0][c] = o * 0.75f + rv_damp[0][c] * 0.25f;
                *bf = inp + rv_damp[0][c] * fbR;
                if (++rv_ci[0][c] >= (int)COMB_L[c])
                    rv_ci[0][c] = 0;
                sl += o;
                ci = rv_ci[1][c];
                bf = &rv_comb[1][c][ci];
                o = *bf;
                rv_damp[1][c] = o * 0.75f + rv_damp[1][c] * 0.25f;
                *bf = inp + rv_damp[1][c] * fbR;
                if (++rv_ci[1][c] >= (int)COMB_R[c])
                    rv_ci[1][c] = 0;
                sr += o;
            }
            for (c = 0; c < 2; c++) {
                float bo;
                ai = rv_ai[0][c];
                bo = rv_ap[0][c][ai];
                rv_ap[0][c][ai] = sl + bo * 0.5f;
                sl = bo - sl;
                if (++rv_ai[0][c] >= (int)AP_L[c])
                    rv_ai[0][c] = 0;
                ai = rv_ai[1][c];
                bo = rv_ap[1][c][ai];
                rv_ap[1][c][ai] = sr + bo * 0.5f;
                sr = bo - sr;
                if (++rv_ai[1][c] >= (int)AP_R[c])
                    rv_ai[1][c] = 0;
            }
            rl = sl * wetR;
            rr = sr * wetR;
        }
        l += rl + drums[i];
        r += rr + drums[i];
        l = sat(l * master_g * 1.0f);
        r = sat(r * master_g * 1.0f);
        out[2 * i] = (int32_t)(l * 8300000.0f);
        out[2 * i + 1] = (int32_t)(r * 8300000.0f);
        {
            float a = fm_fabsf(l) > fm_fabsf(r) ? fm_fabsf(l) : fm_fabsf(r);
            uint32_t pk = (uint32_t)(a * 255.0f);
            if (pk > vis_peak)
                vis_peak = (uint8_t)(pk > 255u ? 255u : pk);
        }
    }
}

/* ------------------------------------------------------------------- the LFO --- */
static float lfo_ph, lfo_val, lfo_sh, lfo_target_rnd;

static void controls(void)
{
    float dep = knob((const uint8_t *)P, P_LDEP), hz, l = 0.0f, bend_t, wig_t, size = ((float)P[P_SIZE] - 64.0f) * (1.0f / 64.0f);
    int shape_i = P[P_LWAVE] & 3, what = P[P_LWHAT];
    static float bend_s, wig_s, wig_t_s;
    static int wig_cnt;
    hz = 0.1f * fm_exp2f(knob((const uint8_t *)P, P_LSPD) * 7.6f);
    lfo_ph += hz * (float)CTRL * (1.0f / FS);
    if (lfo_ph >= 1.0f) {
        lfo_ph -= 1.0f;
        lfo_sh = frnd();
    }
    if (shape_i == LW_SINE)
        l = sn(lfo_ph);
    else if (shape_i == LW_TRI)
        l = tri_w(lfo_ph);
    else if (shape_i == LW_SQUARE) {
        l = sn(lfo_ph) * 5.0f;
        l = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
    } else {
        lfo_target_rnd = lfo_sh;
        lfo_val += (lfo_target_rnd - lfo_val) * 0.12f;
        l = lfo_val;
    }
    vis_lfo = (int8_t)(l * dep * 127.0f);
    ctl.lfo_pitch = (what == LT_PITCH) ? l * dep * 2.5f : 0.0f;
    ctl.lfo_filter = (what == LT_WAH) ? l * dep * 3.0f : 0.0f;
    ctl.trem = (what == LT_TREM) ? 1.0f - dep * (0.5f + 0.5f * l) : 1.0f;
    bend_t = ((float)P[P_BEND] - 64.0f) * (1.0f / 64.0f) * 12.0f - size * 4.0f;     /* BEND, and SIZE: giants sing lower */
    bend_s += (bend_t - bend_s) * 0.04f;
    ctl.bend = bend_s;
    if (--wig_cnt <= 0) {                                /* WIGGLE: a drunken drift in the pitch */
        wig_cnt = 160;
        wig_t_s = frnd();
    }
    {
        float w = (float)P[P_WIGGLE] * (1.0f / 127.0f);
        if (P[P_MOOD] == MOOD_CRAZY && w < 0.35f)
            w = 0.35f;
        wig_t = wig_t_s * w * 1.8f;
    }
    wig_s += (wig_t - wig_s) * 0.02f;
    ctl.wig = wig_s;
}

/* -------------------------------------------------------------------- render --- */
static void rebuild_sound(void)
{
    const preset_t *pr = &PRESETS[P[P_PRESET] % NPRESET];
    float shift, rel;
    int echo = P[P_ECHO], space = P[P_SPACE], fluff = P[P_FLUFF], robot = P[P_ROBOT];
    float size = ((float)P[P_SIZE] - 64.0f) * (1.0f / 64.0f);
    mood_fx(P[P_MOOD] % NMOOD, &shift, &rel, &echo, &space, &fluff, &robot);
    shift += -size * 1.4f;
    rel *= fm_exp2f(size * 1.0f);
    derive(&g_vp, (const uint8_t *)P, pr, shift, rel);
    ctl.cut_mood = 0.0f;
}

void engine_init(void)
{
    int i, m;
    for (i = 0; i <= SINE_N + 1; i++)
        sine_tab[i] = fm_sinf(FM_TWO_PI * (float)i * (1.0f / (float)SINE_N));
    melodies_parse();
    if (!P[P_TEMPO] && !P[P_PRESET] && !P[P_BEATVOL]) {   /* a first start: the defaults */
        preset_load(0);
        P[P_TEMPO] = 50;
        P[P_KEY] = 6;
        P[P_ARPSPD] = 1;
        P[P_ARPRNG] = 1;
        P[P_BEATVOL] = 90;
        P[P_MSPEED] = 64;
        P[P_RVOL] = 100;
        P[P_RSPEED] = 1;
        P[P_RFUNNY] = 12;
        P[P_RLEN] = 1;
        P[P_OCT] = 3;
        P[P_BOUNCE] = 0;
        P[P_TUNING] = 64;
        P[P_MREPEAT] = 1;
        P[P_MBAND] = 0;
        P[P_STICK] = 0;
    }
    {
        uint8_t zero[NSND];
        const preset_t *bell = &PRESETS[10];
        for (m = 0; m < NSND; m++)
            zero[m] = bell->d[m];
        for (m = 0; m < NPRESET; m++)
            derive(&zoo_vp[m], PRESETS[m].d, &PRESETS[m], 0.0f, 1.0f);
        zero[P_ATK] = 0;
        zero[P_PLUCK] = 100;
        zero[P_REL] = 70;
        zero[P_METAL] = 70;
        zero[P_WAVE] = 0;
        zero[P_CUT] = 120;
        zero[P_BIG] = 0;
        zero[P_BUDDY] = 0;
        derive(&spark_vp, zero, bell, 0.0f, 1.0f);
        spark_vp.vol *= 0.7f;
    }
    rebuild_sound();
}

void preset_load(int idx)
{
    int i;
    const preset_t *pr = &PRESETS[(unsigned)idx % NPRESET];
    for (i = 0; i < NSND; i++)
        P[i] = pr->d[i];
    P[P_PRESET] = (uint8_t)((unsigned)idx % NPRESET);
}

void engine_render(int32_t *out, uint32_t n)
{
    static float mono[256], drum[256];
    uint32_t i, nb = n & ~7u;
    int k, active = 0, echo, space, fluff, robot;
    float lvl = 0.0f, sh, rm;
    if (nb > 256u)
        nb = 256u;
    process_events();
    rebuild_sound();
    echo = P[P_ECHO];
    space = P[P_SPACE];
    fluff = P[P_FLUFF];
    robot = P[P_ROBOT];
    mood_fx(P[P_MOOD] % NMOOD, &sh, &rm, &echo, &space, &fluff, &robot);
    if (arp_last_mode != P[P_ARP]) {                      /* leaving or entering the arpeggiator */
        arp_last_mode = P[P_ARP];
        arp_cnt = 0;
        arp_notes = 0;
        arp_idx = 0;
        voices_release_all();
    }
    for (i = 0; i < nb; i += CTRL) {
        float *m8 = &mono[i];
        int s;
        for (k = 0; k < CTRL; k++) {
            m8[k] = 0.0f;
            drum[i + (uint32_t)k] = 0.0f;
        }
        controls();
        for (s = 0; s < NSCHED; s++)
            if (sched[s].used && (int32_t)(now_s - sched[s].t) >= 0) {
                sched[s].used = 0;
                if (sched[s].type == SC_ON)
                    voice_on(sched[s].src, sched[s].id, (float)sched[s].note, (float)sched[s].vel * (1.0f / 127.0f), sched[s].vp);
                else if (sched[s].type == SC_OFF)
                    voices_release(sched[s].id);
                else
                    drum_hit(sched[s].note, sched[s].vel);
            }
        arp_tick();
        mel_tick();
        beat_tick();
        rec_tick();
        for (k = 0; k < NVOICE; k++)
            if (voice[k].on)
                voice_run(&voice[k], m8);
        now_s += CTRL;
    }
    drums_run(drum, (int)nb);
    for (k = 0; k < NVOICE; k++)
        if (voice[k].on) {
            float l = voice[k].env * voice[k].vel;
            active++;
            if (l > lvl)
                lvl = l;
        }
    vis_voices = (uint8_t)active;
    vis_level = (uint8_t)(lvl * 255.0f > 255.0f ? 255.0f : lvl * 255.0f);
    fx_block(mono, drum, out, (int)nb, echo, space, fluff, robot);
    if (vis_peak > 0)
        vis_peak = (uint8_t)(vis_peak - (vis_peak >> 5) - 1);
    for (i = nb; i < n; i++) {
        out[2 * i] = 0;
        out[2 * i + 1] = 0;
    }
}
