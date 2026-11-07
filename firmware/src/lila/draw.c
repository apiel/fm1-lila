/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's drawing toolkit: shapes with no framebuffer. The screen is drawn in two bands (cv_oy shifts the drawing so a
 * scene is written once, in screen coordinates, and run twice). Colours go through K(), which applies the sound's look:
 * the filter dims and warms them, the key turns the hue, a ghost fades them toward the horizon.
 * Part of the unity build, after gfx.c. */
#pragma once
#include "../dsp/fastmath.h"

/* ============================================================ text helpers === */
static char *put_s(char *b, const char *s)
{
    while (*s)
        *b++ = *s++;
    *b = 0;
    return b;
}
static char *put_i(char *b, int v)
{
    char t[12];
    int n = 0, neg = v < 0;
    uint32_t u = (uint32_t)(neg ? -v : v);
    do {
        t[n++] = (char)('0' + u % 10u);
        u /= 10u;
    } while (u);
    if (neg)
        *b++ = '-';
    while (n)
        *b++ = t[--n];
    *b = 0;
    return b;
}

/* ========================================================= colours and tint === */
static uint16_t blend(uint16_t a, uint16_t b, int t)  /* a..b, t of 16 */
{
    int r = ((a >> 11) * (16 - t) + (b >> 11) * t) >> 4, g = (((a >> 5) & 63) * (16 - t) + ((b >> 5) & 63) * t) >> 4,
        bl = ((a & 31) * (16 - t) + (b & 31) * t) >> 4;
    return (uint16_t)((r << 11) | (g << 5) | bl);
}

/* the look of the sound, set once per frame by the UI (ui.c) */
static struct {
    int bri;            /* 0..256: the filter's cutoff, dark to bright */
    int sat;            /* 0..256: a metal sound is grey, a bright one vivid */
    int warm;           /* -32..32: low pass warm (+), high pass cool (-) */
    int hue;            /* 0..767: the key turns the colours */
    int fade;           /* 0..16: toward fadecol (a ghost) */
    uint16_t fadecol;
    int night;          /* 0..16: the mood darkens and blues the world */
    int flash;          /* 0..16: a white flash (the kick, a note) */
} look;

static uint16_t K(int r, int g, int b)
{
    int gray;
    if (look.hue) {                                   /* rotate the colour wheel: three 256-step thirds */
        int h = look.hue, a = h & 255, mx = r, mn = r, rr = r, gg = g, bb = b;
        int third = h >> 8;
        (void)mx;
        (void)mn;
        if (third == 1) { rr = b; gg = r; bb = g; }
        else if (third == 2) { rr = g; gg = b; bb = r; }
        if (a) {                                        /* blend toward the next rotation */
            int nr, ng, nb;
            if (third == 0) { nr = b; ng = r; nb = g; }
            else if (third == 1) { nr = g; ng = b; nb = r; }
            else { nr = r; ng = g; nb = b; }
            rr += ((nr - rr) * a) >> 8;
            gg += ((ng - gg) * a) >> 8;
            bb += ((nb - bb) * a) >> 8;
        }
        r = rr; g = gg; b = bb;
    }
    if (look.sat < 256) {
        gray = (r * 77 + g * 150 + b * 29) >> 8;
        r = gray + (((r - gray) * look.sat) >> 8);
        g = gray + (((g - gray) * look.sat) >> 8);
        b = gray + (((b - gray) * look.sat) >> 8);
    }
    if (look.warm) {
        r += look.warm;
        b -= look.warm;
    }
    if (look.bri < 256) {
        r = (r * look.bri) >> 8;
        g = (g * look.bri) >> 8;
        b = (b * look.bri) >> 8;
    }
    if (look.night) {
        r = r * (16 - look.night * 6 / 10) >> 4;
        g = g * (16 - look.night * 5 / 10) >> 4;
        b = (b * (16 - look.night * 2 / 10) >> 4) + look.night * 2;
    }
    if (look.flash) {
        r += ((255 - r) * look.flash) >> 5;
        g += ((255 - g) * look.flash) >> 5;
        b += ((255 - b) * look.flash) >> 5;
    }
    r = r < 0 ? 0 : r > 255 ? 255 : r;
    g = g < 0 ? 0 : g > 255 ? 255 : g;
    b = b < 0 ? 0 : b > 255 ? 255 : b;
    {
        uint16_t c = RGB(r, g, b);
        return look.fade ? blend(c, look.fadecol, look.fade) : c;
    }
}
static uint16_t KC(uint32_t rgb) { return K((int)((rgb >> 16) & 255u), (int)((rgb >> 8) & 255u), (int)(rgb & 255u)); }

static void text_c(int cx, int y, const felucca_font_t *f, const char *s, uint16_t c) { cv_text(cx - text_w(f, s) / 2, y, f, s, c); }

/* ========================================================== sine, random === */
static int8_t sin_t[256];                              /* -127..127 for a full turn in 256 */
static void draw_init(void)
{
    int i;
    for (i = 0; i < 256; i++)
        sin_t[i] = (int8_t)(fm_sinf((float)i * (FM_TWO_PI / 256.0f)) * 127.0f + (i < 128 ? 0.4f : -0.4f));
}
static inline int isin(int a) { return sin_t[a & 255]; }
static inline int icos(int a) { return sin_t[(a + 64) & 255]; }
static uint32_t rnd_s = 0x9E3779B9u;
static inline uint32_t urand(void)
{
    rnd_s ^= rnd_s << 13;
    rnd_s ^= rnd_s >> 17;
    rnd_s ^= rnd_s << 5;
    return rnd_s;
}
static inline int rnd_range(int lo, int hi) { return lo + (int)(urand() % (uint32_t)(hi - lo + 1)); }
static inline int iabs(int v) { return v < 0 ? -v : v; }
static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int iclamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* ========================================================== shape drawing === */
static inline void hspan(int32_t x0, int32_t x1, int32_t y, uint16_t sc)   /* sc: a swapped colour */
{
    int32_t yy = y + cv_oy;
    uint16_t *p;
    if (yy < cv_y0 || yy >= cv_y1)
        return;
    if (x0 < 0)
        x0 = 0;
    if (x1 > 239)
        x1 = 239;
    if (x0 > x1)
        return;
    p = cv_px + (uint32_t)yy * 240u + (uint32_t)x0;
    for (; x0 <= x1; x0++)
        *p++ = sc;
}
static void box_(int x, int y, int w, int h, uint16_t c) { cv_rect(x, y, w, h, c); }

static int rows_lo(int cy, int r)                      /* first visible row offset of a shape around cy, radius r */
{
    int lo = cv_y0 - cv_oy - cy;
    return lo > -r ? lo : -r;
}
static int rows_hi(int cy, int r)
{
    int hi = cv_y1 - 1 - cv_oy - cy;
    return hi < r ? hi : r;
}
static void fill_ellipse(int cx, int cy, int rx, int ry, uint16_t c)
{
    int dy, lo, hi;
    uint16_t sc = swap16(c);
    if (rx < 1 || ry < 1)
        return;
    lo = rows_lo(cy, ry);
    hi = rows_hi(cy, ry);
    for (dy = lo; dy <= hi; dy++) {
        float f = 1.0f - (float)(dy * dy) / (float)(ry * ry);
        int w;
        if (f < 0.0f)
            continue;
        w = (int)(fm_sqrtf(f) * (float)rx + 0.5f);
        hspan(cx - w, cx + w, cy + dy, sc);
    }
}
static void fill_circle(int cx, int cy, int r, uint16_t c) { fill_ellipse(cx, cy, r, r, c); }
static void ring_(int cx, int cy, int r, int th, uint16_t c)       /* an outline of thickness th */
{
    int dy, lo, hi;
    uint16_t sc = swap16(c);
    int ri = r - th;
    if (r < 1)
        return;
    if (ri < 0)
        ri = 0;
    lo = rows_lo(cy, r);
    hi = rows_hi(cy, r);
    for (dy = lo; dy <= hi; dy++) {
        int wo = (int)(fm_sqrtf((float)(r * r - dy * dy)) + 0.5f), wi = -1;
        if (iabs(dy) < ri)
            wi = (int)(fm_sqrtf((float)(ri * ri - dy * dy)) + 0.5f);
        if (wi < 0)
            hspan(cx - wo, cx + wo, cy + dy, sc);
        else {
            hspan(cx - wo, cx - wi, cy + dy, sc);
            hspan(cx + wi, cx + wo, cy + dy, sc);
        }
    }
}
/* a filled polygon (any simple one), up to 12 corners, by scanlines and even-odd */
static void fill_poly(const int16_t *pt, int n, uint16_t c)
{
    int y, ymin = 10000, ymax = -10000, i;
    uint16_t sc = swap16(c);
    for (i = 0; i < n; i++) {
        if (pt[2 * i + 1] < ymin) ymin = pt[2 * i + 1];
        if (pt[2 * i + 1] > ymax) ymax = pt[2 * i + 1];
    }
    if (ymin < cv_y0 - cv_oy)
        ymin = cv_y0 - cv_oy;
    if (ymax > cv_y1 - 1 - cv_oy)
        ymax = cv_y1 - 1 - cv_oy;
    for (y = ymin; y <= ymax; y++) {
        int xs[12], k = 0, a, b;
        for (i = 0; i < n; i++) {
            int x0 = pt[2 * i], y0 = pt[2 * i + 1], x1 = pt[2 * ((i + 1) % n)], y1 = pt[2 * ((i + 1) % n) + 1];
            if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y)) {
                if (k < 12)
                    xs[k++] = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            }
        }
        for (a = 1; a < k; a++) {                       /* sort */
            int v = xs[a];
            for (b = a - 1; b >= 0 && xs[b] > v; b--)
                xs[b + 1] = xs[b];
            xs[b + 1] = v;
        }
        for (a = 0; a + 1 < k; a += 2)
            hspan(xs[a], xs[a + 1], y, sc);
    }
}
static void fill_tri(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c)
{
    int16_t p[6] = {(int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2};
    fill_poly(p, 3, c);
}
static void fill_quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c)
{
    int16_t p[8] = {(int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2, (int16_t)x3, (int16_t)y3};
    fill_poly(p, 4, c);
}
/* a thick line with round ends */
static void fat_line(int x0, int y0, int x1, int y1, int r, uint16_t c)
{
    int dx = x1 - x0, dy = y1 - y0, nx, ny;
    float l = fm_sqrtf((float)(dx * dx + dy * dy));
    if (l < 1.0f) {
        fill_circle(x0, y0, r, c);
        return;
    }
    nx = (int)((float)(-dy) * (float)r / l);
    ny = (int)((float)dx * (float)r / l);
    fill_quad(x0 + nx, y0 + ny, x1 + nx, y1 + ny, x1 - nx, y1 - ny, x0 - nx, y0 - ny, c);
    fill_circle(x0, y0, r, c);
    fill_circle(x1, y1, r, c);
}
/* a rounded rectangle */
static void fill_rbox(int x, int y, int w, int h, int r, uint16_t c)
{
    if (r * 2 > h)
        r = h / 2;
    if (r * 2 > w)
        r = w / 2;
    box_(x + r, y, w - 2 * r, h, c);
    box_(x, y + r, w, h - 2 * r, c);
    fill_circle(x + r, y + r, r, c);
    fill_circle(x + w - r - 1, y + r, r, c);
    fill_circle(x + r, y + h - r - 1, r, c);
    fill_circle(x + w - r - 1, y + h - r - 1, r, c);
}
/* a four-point sparkle */
static void sparkle(int x, int y, int r, uint16_t c)
{
    if (r < 1) {
        cv_pset(x, y, c);
        return;
    }
    fill_tri(x - r, y, x, y - r / 3 - 1, x, y + r / 3 + 1, c);
    fill_tri(x + r, y, x, y - r / 3 - 1, x, y + r / 3 + 1, c);
    fill_tri(x, y - r, x - r / 3 - 1, y, x + r / 3 + 1, y, c);
    fill_tri(x, y + r, x - r / 3 - 1, y, x + r / 3 + 1, y, c);
}
/* a vertical gradient over rows y0 .. y1 */
static void vgrad(int y0, int y1, uint16_t c0, uint16_t c1)
{
    int y, h = y1 - y0;
    if (h < 1)
        return;
    for (y = y0; y < y1; y++) {
        int yy = y + cv_oy;
        if (yy < cv_y0 || yy >= cv_y1)
            continue;
        hspan(0, 239, y, swap16(blend(c0, c1, (y - y0) * 16 / h)));
    }
}
/* a vertical run of pixels in column x, rows y0 .. y1 */
static inline void vspan(int x, int y0, int y1, uint16_t sc)
{
    int ys = y0 + cv_oy, ye = y1 + cv_oy;
    uint16_t *p;
    if ((uint32_t)x >= 240u)
        return;
    if (ys < cv_y0)
        ys = cv_y0;
    if (ye >= cv_y1)
        ye = cv_y1 - 1;
    p = cv_px + (uint32_t)ys * 240u + (uint32_t)x;
    for (; ys <= ye; ys++, p += 240)
        *p = sc;
}
/* a rolling hill: the ground under a sine, from base down to the bottom row */
static void hills(int base, int amp, int period, int phase, int bottom, uint16_t c)
{
    int x;
    uint16_t sc = swap16(c);
    for (x = 0; x < 240; x++)
        vspan(x, base - (isin(x * 256 / period + phase) * amp >> 7), bottom - 1, sc);
}

/* text magnified k times, centred at cx, top at y (the band's canvas shows only its rows) */
static void text_big(int cx, int y, const felucca_font_t *f, const char *s, uint16_t c, int k)
{
    int32_t w = text_w(f, s), x0 = cx - w * k / 2, x = 0;
    for (; *s; s++) {
        uint32_t gi = glyph(f, (uint8_t)*s), gx, gy, gw = f->bw[gi], bpr = (gw + 1u) / 2u;
        const uint8_t *gd = f->data + f->off[gi];
        for (gy = 0; gy < f->h; gy++)
            for (gx = 0; gx < gw; gx++) {
                uint32_t a = gd[gy * bpr + gx / 2u];
                a = (gx & 1u) ? (a & 15u) : (a >> 4);
                if (a >= 8u)
                    box_(x0 + (x - f->pad + (int32_t)gx) * k, y + (int32_t)gy * k, k, k, c);
            }
        x += f->adv[gi];
    }
}
/* text with an outline, for words on a busy picture */
static void text_out(int cx, int y, const felucca_font_t *f, const char *s, uint16_t c, uint16_t o)
{
    int w = text_w(f, s), x = cx - w / 2, dx, dy;
    for (dy = -1; dy <= 1; dy++)
        for (dx = -1; dx <= 1; dx++)
            if (dx || dy)
                cv_text(x + dx, y + dy, f, s, o);
    cv_text(x, y, f, s, c);
}

/* ============================================================== particles === */
enum { PK_NONE, PK_NOTE, PK_STAR, PK_RING, PK_BUBBLE, PK_DOT, PK_HEART, PK_CONF, PK_SPARK, PK_ZZZ };
typedef struct {
    int16_t x, y, vx, vy;           /* 1/16 pixel */
    uint8_t kind, life, max, size;
    uint16_t col;                    /* RGB565 */
    uint8_t shape;
} part_t;
#define NPART 56
static part_t parts[NPART];
static uint8_t part_n;

static void part_add(int kind, int x, int y, int vx, int vy, int life, int size, uint16_t col, int shape)
{
    int i, slot = -1, oldest = 255, k;
    for (k = 0; k < NPART; k++) {
        i = (part_n + k) % NPART;
        if (parts[i].kind == PK_NONE) {
            slot = i;
            break;
        }
        if (parts[i].life < oldest) {
            oldest = parts[i].life;
            slot = i;
        }
    }
    part_n = (uint8_t)((slot + 1) % NPART);
    parts[slot].x = (int16_t)(x * 16);
    parts[slot].y = (int16_t)(y * 16);
    parts[slot].vx = (int16_t)vx;
    parts[slot].vy = (int16_t)vy;
    parts[slot].kind = (uint8_t)kind;
    parts[slot].life = parts[slot].max = (uint8_t)life;
    parts[slot].size = (uint8_t)size;
    parts[slot].col = col;
    parts[slot].shape = (uint8_t)shape;
}
static void parts_step(void)                           /* once per frame */
{
    int i;
    for (i = 0; i < NPART; i++) {
        part_t *p = &parts[i];
        if (p->kind == PK_NONE)
            continue;
        if (p->life == 0) {
            p->kind = PK_NONE;
            continue;
        }
        p->life--;
        p->x = (int16_t)(p->x + p->vx);
        p->y = (int16_t)(p->y + p->vy);
        if (p->kind == PK_CONF)
            p->vy = (int16_t)(p->vy + 3);
        if (p->kind == PK_NOTE || p->kind == PK_HEART || p->kind == PK_ZZZ)
            p->vx = (int16_t)(p->vx + (isin((p->max - p->life) * 9 + i * 40) >> 5));
    }
}
/* a note: a glyph in the shape of the sound's wave (circle, triangle, saw, square) */
static void note_glyph(int x, int y, int s, uint16_t c, int shape)
{
    switch (shape & 3) {
    case 0: fill_circle(x, y, s, c); break;
    case 1: fill_tri(x - s, y + s, x + s, y + s, x, y - s, c); break;
    case 2: fill_tri(x - s, y + s, x + s, y + s, x + s, y - s, c); break;
    default: box_(x - s, y - s, s * 2 + 1, s * 2 + 1, c); break;
    }
    box_(x + s - 1, y - s * 3, 2, s * 3, c);                      /* the stem */
    fill_tri(x + s, y - s * 3, x + s + s + 1, y - s * 2, x + s, y - s * 2 + 1, c);   /* the flag */
}
static void heart(int x, int y, int s, uint16_t c)
{
    fill_circle(x - s / 2, y - s / 3, s / 2 + 1, c);
    fill_circle(x + s / 2, y - s / 3, s / 2 + 1, c);
    fill_tri(x - s, y - s / 6, x + s, y - s / 6, x, y + s, c);
}
static void parts_draw(void)
{
    int i;
    for (i = 0; i < NPART; i++) {
        const part_t *p = &parts[i];
        int x = p->x >> 4, y = p->y >> 4, s, t;
        if (p->kind == PK_NONE)
            continue;
        t = p->life * 255 / (p->max ? p->max : 1);          /* 255 young .. 0 old */
        switch (p->kind) {
        case PK_NOTE:
            s = 2 + p->size * (t > 200 ? 255 - t + 100 : 255) / 255 / 4;
            note_glyph(x, y, s > 1 ? s : 2, p->col, p->shape);
            break;
        case PK_STAR:
            s = (p->size * (t < 100 ? t + 60 : 255) / 255) + (((p->life >> 1) & 1) ? 1 : 0);
            sparkle(x, y, s, p->col);
            break;
        case PK_RING:
            s = p->size + (255 - t) * p->size / 128;
            ring_(x, y, s, 2, blend(p->col, look.fadecol ? look.fadecol : 0, 16 - t * 16 / 255));
            break;
        case PK_BUBBLE:
            ring_(x, y, p->size, 2, p->col);
            cv_pset(x - p->size / 3, y - p->size / 3, C_WHITE);
            break;
        case PK_DOT:
            fill_circle(x, y, p->size * t / 255 + 1, p->col);
            break;
        case PK_HEART:
            heart(x, y, p->size, p->col);
            break;
        case PK_CONF:
            box_(x, y, 4, 3, p->col);
            break;
        case PK_SPARK:
            fill_circle(x, y, 1 + p->size * t / 255, p->col);
            break;
        case PK_ZZZ:
            text_big(x, y, &FONT_B, "Z", p->col, 1 + (255 - t) / 128);
            break;
        default:
            break;
        }
    }
}
