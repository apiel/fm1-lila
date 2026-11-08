/* SPDX-License-Identifier: GPL-3.0-only */
/* The three piano worlds: PIANO (a concert hall), E-PIANO (a neon synthwave night), ORGAN (a cathedral). They share one
 * picture: the panel's 27 keys drawn as a real keyboard along the bottom, every key sinking and glowing in its own colour
 * when it sounds (pressed, or played by a song, the arpeggiator or the loop), a note rising from it as a coloured bar, and
 * a little singing note floating above, who takes the usual dressing of every world (size, boing, buddy twins, echo ghosts,
 * chorus shimmer ...). Included by scenes.c before the table. */
#pragma once

#define PIANO_WORLD0 12                     /* the first piano's index among the presets */
#define PK_Y 148                            /* the keyboard's top edge */
#define PK_BARS 20

static uint8_t pk[NKEYS_L];                 /* 0..255: how lit each key is */
static struct { int16_t x, y; uint8_t w, key, h, life; } pbar[PK_BARS];
static int8_t pk_x[NKEYS_L], pk_black[NKEYS_L];
static uint8_t pk_ready;

static void piano_layout(void)
{
    int k, white = 0;
    for (k = 0; k < NKEYS_L; k++) {
        int pc = (KEY0_NOTE + k) % 12;
        pk_black[k] = (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10);
        if (pk_black[k])
            pk_x[k] = (int8_t)(white * 15 - 4 - 120);       /* centred on the edge between two white keys (stored from the middle) */
        else {
            pk_x[k] = (int8_t)(white * 15 - 120);
            white++;
        }
    }
    pk_ready = 1;
}

/* a key's colour: a rainbow along the keyboard; lit = 0..255 mixes it with white */
static uint16_t piano_col(int k, int bright)
{
    int a = k * 9;
    return K(iclamp(120 + (isin(a) * 120 >> 7) + bright / 3, 0, 255), iclamp(120 + (isin(a + 85) * 120 >> 7) + bright / 3, 0, 255),
             iclamp(120 + (isin(a + 170) * 120 >> 7) + bright / 3, 0, 255));
}

/* a key sounded: light it and send a bar up from it (the UI calls this for every note that sounds) */
static void piano_note(int note, int vel)
{
    int k = note - KEY0_NOTE - 12 * ((int)P[P_OCT] - 3), i, best = 0;
    if (!pk_ready)
        piano_layout();
    if (k < 0 || k >= NKEYS_L)
        return;
    pk[k] = 255;
    for (i = 0; i < PK_BARS; i++)
        if (!pbar[i].life) {
            best = i;
            break;
        }
    pbar[best].x = (int16_t)(120 + pk_x[k]);
    pbar[best].y = PK_Y;
    pbar[best].w = pk_black[k] ? 9 : 14;
    pbar[best].key = (uint8_t)k;
    pbar[best].h = (uint8_t)(10 + vel / 6);
    pbar[best].life = 1;
}

/* once a frame: held keys stay lit, the others fade; the bars rise */
static void piano_step(uint32_t held)
{
    int k, i;
    if (!pk_ready)
        piano_layout();
    for (k = 0; k < NKEYS_L; k++) {
        if ((held >> k) & 1u)
            pk[k] = 255;
        else if (pk[k])
            pk[k] = (uint8_t)(pk[k] > 40 ? pk[k] - pk[k] / 6 - 8 : 0);
    }
    for (i = 0; i < PK_BARS; i++)
        if (pbar[i].life) {
            pbar[i].y -= (int16_t)(3 + sc.space / 40);
            if (pbar[i].y < -pbar[i].h || ++pbar[i].life > 120)
                pbar[i].life = 0;
        }
}

static int pworld(void) { return iclamp(sc.world - PIANO_WORLD0, 0, 2); }

/* ---------------------------------------------------------------- the rooms --- */
static void bg_piano(void)
{
    int w = pworld(), i;
    if (w == 0) {                                                /* a concert hall: red curtains, a spotlight, a chandelier */
        vgrad(0, 240, K(26, 16, 44), K(74, 30, 66));
        fill_tri(120 - 6, 0, 120 + 6, 0, 120 + 74, PK_Y, blend(K(26, 16, 44), K(255, 240, 200), 2 + sc.lvl / 90));
        fill_tri(120 - 6, 0, 120 + 6, 0, 120 - 74, PK_Y, blend(K(26, 16, 44), K(255, 240, 200), 2 + sc.lvl / 90));
        for (i = 0; i < 5; i++) {
            int sw = isin((int)(TM / 30) + i * 40) >> 5;
            box_(i * 9 + sw / 2, 0, 8, 130 - i * 6, K(150 - i * 6, 20, 40));
            box_(240 - 9 - i * 9 + sw / 2, 0, 8, 130 - i * 6, K(150 - i * 6, 20, 40));
        }
        box_(0, 0, 240, 10, K(110, 14, 30));
        stars_sky(18, 80, 6, K(255, 240, 180));
    } else if (w == 1) {                                         /* neon: a striped sun and a glowing grid */
        vgrad(0, 240, K(14, 0, 40), K(130, 22, 110));
        fill_circle(120, 96, 46, K(255, 150, 60));
        fill_circle(120, 96, 38, K(255, 90, 140));
        for (i = 0; i < 6; i++)
            box_(70, 92 + i * 8, 100, 2 + i, K(60, 6, 80));
        box_(0, 104, 240, 46, K(50, 6, 80));
        for (i = 0; i < 8; i++) {
            int x = 120 + (i - 4) * 36;
            fat_line(120 + (x - 120) / 6, 104, 120 + (x - 120) * 3, PK_Y, 1, K(255, 70, 200));
        }
        for (i = 0; i < 5; i++) {
            int y = 106 + (i * i * 2) + (int)((TM / 60u) % 8u);
            if (y < PK_Y)
                box_(0, y, 240, 1, K(255, 70, 200));
        }
        stars_sky(24, 60, 8, K(200, 220, 255));
    } else {                                                     /* a cathedral: a rose window and organ pipes that glow */
        vgrad(0, 240, K(40, 30, 60), K(110, 80, 70));
        fill_circle(120, 62, 40, K(40, 30, 50));
        for (i = 0; i < 12; i++) {
            int a = i * 21 + (int)(TM / 90);
            fill_tri(120, 62, 120 + icos(a) * 36 / 127, 62 + isin(a) * 36 / 127, 120 + icos(a + 21) * 36 / 127, 62 + isin(a + 21) * 36 / 127,
                     piano_col(i * 2, 40 + sc.lvl / 2));
        }
        fill_circle(120, 62, 8, K(255, 240, 200));
        for (i = 0; i < 13; i++) {
            int h = 56 + (i < 7 ? i : 12 - i) * 11, x = 6 + i * 18, gl = sc.lvl / 3 + (i & 1 ? sc.hit / 4 : 0);
            box_(x, 0, 14, h, K(160 + gl / 3, 150 + gl / 3, 120 + gl / 4));
            box_(x + 2, 0, 3, h, K(230, 220, 190));
            box_(x, h, 14, 5, K(90, 70, 50));
            fill_ellipse(x + 7, h + 12, 4, 5, K(40, 30, 30));
        }
    }
}

/* the singing note: a round face on a stem, with a flag */
static void act_piano(int cx, int cy)
{
    int g = sc.mouth * 9 / 255, w = pworld(), ln = sc.lean / 3, blink = ((int)(TM / 300u) % 14) == 0;
    uint16_t body = w == 0 ? K(255, 214, 80) : w == 1 ? K(120, 240, 255) : K(255, 190, 120), dark = blend(body, K(0, 0, 0), 6), ink = K(40, 30, 50);
    AL(18 + ln, -30, 18 + ln / 2, -94, 3, ink);
    AT(18 + ln / 2, -94, 46 + ln / 2, -74 + (isin((int)(TM / 6)) >> 6), 18 + ln / 2, -66, ink);
    AE(ln, -24, 26, 22, dark);
    AE(ln, -26, 26, 22, body);
    AE(ln - 8, -34, 9, 6, blend(body, K(255, 255, 255), 8));
    if (!blink) {
        AE(ln - 9, -30, 5, 6, K(255, 255, 255));
        AE(ln + 9, -30, 5, 6, K(255, 255, 255));
        AE(ln - 9, -30, 2, 3, ink);
        AE(ln + 9, -30, 2, 3, ink);
    } else {
        AB(ln - 14, -30, 10, 2, ink);
        AB(ln + 4, -30, 10, 2, ink);
    }
    AE(ln - 16, -20, 4, 3, K(255, 140, 150));
    AE(ln + 16, -20, 4, 3, K(255, 140, 150));
    if (g > 1)
        AE(ln, -14 + g / 4, 6, 2 + g / 2, K(150, 30, 60));
    else {
        AL(ln - 5, -16, ln, -13, 1, ink);
        AL(ln, -13, ln + 5, -16, 1, ink);
    }
}

/* the keyboard (the foreground), and the notes that rise from it */
static void fg_piano(void)
{
    int k, i, w = pworld(), echo = sc.echo > 40;
    uint16_t wood = w == 0 ? K(30, 22, 28) : w == 1 ? K(60, 20, 90) : K(100, 66, 40);
    for (i = 0; i < PK_BARS; i++)                                /* the rising notes, with a fainter echo of each below it */
        if (pbar[i].life) {
            int y = pbar[i].y, h = pbar[i].h;
            uint16_t c = piano_col(pbar[i].key, 90);
            if (echo)
                box_(pbar[i].x + 1, y + 20 + sc.echo / 5, pbar[i].w - 2, h * 2 / 3, blend(K(30, 20, 50), c, 5));
            fill_rbox(pbar[i].x, y - h, pbar[i].w, h, 3, c);
            box_(pbar[i].x + 2, y - h + 1, 2, h - 2, blend(c, K(255, 255, 255), 8));
        }
    box_(0, PK_Y - 6, 240, 6, wood);                             /* the fallboard */
    if (w == 1)
        box_(0, PK_Y - 7, 240, 1, K(255, 70, 200));
    for (k = 0; k < NKEYS_L; k++)                                /* white keys first */
        if (!pk_black[k]) {
            int x = 120 + pk_x[k], lit = pk[k], dy = lit >> 6;
            uint16_t c = lit ? blend(K(250, 250, 245), piano_col(k, lit), 3 + lit * 11 / 255) : K(250, 250, 245);
            box_(x + 1, PK_Y + dy, 14, 92 - dy, c);
            box_(x + 1, PK_Y + 86, 14, 6, blend(c, K(120, 120, 130), 6));
            if (lit > 120 && sc.metal > 20)
                sparkle(x + 7, PK_Y - 4 - ((255 - lit) >> 3), 2 + sc.metal / 40, K(255, 255, 255));
        }
    for (k = 0; k < NKEYS_L; k++)
        if (pk_black[k]) {
            int x = 120 + pk_x[k], lit = pk[k], dy = lit >> 7;
            uint16_t c = lit ? blend(K(24, 24, 32), piano_col(k, lit), 4 + lit * 8 / 255) : K(24, 24, 32);
            box_(x, PK_Y, 9, 54 - dy, c);
            box_(x + 1, PK_Y + 44 - dy, 7, 3, blend(c, K(255, 255, 255), 3));
        }
    box_(0, PK_Y + 92, 240, 4, wood);
}
