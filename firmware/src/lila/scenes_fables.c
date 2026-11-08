/* SPDX-License-Identifier: GPL-3.0-only */
/* The three fables of La Fontaine, one preset each: the raven (on his branch) and the fox, the frog who swells next to the ox,
 * the lion and the rat. The picture follows the part being told (nine parts a fable). Included by scenes.c after the table. */
#pragma once

#define FABLE_WORLD0 20

/* a fox, feet at (cx, cy), facing right: up = how high it looks (0..100), mouth = how open, cheese = in its mouth */
static void fox_(int cx, int cy, int up, int mouth, int cheese)
{
    uint16_t og = K(236, 118, 34), dk = K(58, 36, 30), wh = K(255, 246, 230), ink = K(30, 20, 20);
    int wag = isin((int)(TM / 8)) * 5 >> 7, hx = 34, hy = -54 - up / 4, ty = hy + 3 - up / 2, g = mouth * 12 / 255;
    AE(-44, -36 + wag / 2, 24, 12, og);                         /* the big tail, white at its tip */
    AE(-62, -40 + wag, 11, 8, wh);
    AB(-22, -16, 9, 16, dk);
    AB(-8, -16, 9, 16, dk);
    AE(-6, -30, 32, 17, og);
    AE(10, -24, 20, 10, wh);
    AB(14, -22, 8, 22, dk);
    AB(26, -20, 8, 20, dk);
    AL(22, -34, hx - 2, hy + 8, 8, og);                          /* the neck */
    AT(hx - 10, hy - 8, hx - 2, hy - 10, hx - 8, hy - 32, og);   /* the pointed ears */
    AT(hx - 8, hy - 12, hx - 3, hy - 12, hx - 8, hy - 25, dk);
    AT(hx + 2, hy - 10, hx + 10, hy - 8, hx + 9, hy - 30, og);
    AT(hx + 4, hy - 12, hx + 8, hy - 11, hx + 9, hy - 24, dk);
    AE(hx, hy, 16, 13, og);
    AE(hx + 2, hy + 6, 11, 6, wh);
    if (g > 2)
        AT(hx + 2, hy + 4, hx + 22, ty + 2, hx + 12, hy + 10 + g, K(120, 30, 40));
    AT(hx + 2, hy - 7, hx + 2, hy + 9, hx + 26, ty, og);          /* the long snout */
    AT(hx + 2, hy + 3, hx + 2, hy + 10, hx + 24, ty + 3, wh);
    AE(hx + 26, ty, 3, 3, ink);
    AE(hx + 7, hy - 4, 4, 4, wh);
    AE(hx + 8, hy - 4, 2, 3, ink);
    AL(hx + 3, hy - 9, hx + 12, hy - 7, 1, dk);                   /* a sly eyebrow */
    if (cheese)
        AE(hx + 22, ty + 5, 8, 6, K(255, 208, 56));
}

/* a raven on a branch, feet at (cx, cy), facing left: ob = the beak's opening, cheese in the beak, sad */
static void raven_(int cx, int cy, int ob, int cheese, int sad)
{
    uint16_t ink = K(26, 26, 44), hi = K(78, 78, 112), bk = K(150, 120, 70), wh = K(255, 255, 255);
    AL(-4, -8, -4, 0, 2, bk);
    AL(8, -8, 8, 0, 2, bk);
    AT(20, -30, 54, -20, 22, -10, ink);
    AE(6, -26, 26, 18, ink);
    AE(10, -26, 18, 10, hi);
    AE(-18, -48 + (sad ? 4 : 0), 14, 13, ink);
    AE(-24, -51 + (sad ? 4 : 0), 4, 4, wh);
    AE(-25, -51 + (sad ? 4 : 0), 2, 2, ink);
    if (sad)
        AE(-27, -42, 1, 4, K(120, 200, 255));
    AT(-28, -55, -52, -46, -28, -46, bk);
    if (ob > 3) {
        AT(-28, -46, -50, -46, -28, -43, K(150, 30, 40));
        AT(-28, -44, -48, -43 + ob, -28, -40, bk);
    } else
        AT(-28, -46, -49, -45, -28, -41, K(120, 94, 54));
    if (cheese) {
        AE(-48, -45 + ob / 2, 9, 7, K(255, 208, 56));
        AE(-50, -46 + ob / 2, 2, 2, K(235, 160, 30));
        AE(-45, -43 + ob / 2, 2, 2, K(235, 160, 30));
    }
}

static void cheese_(int x, int y)
{
    fill_ellipse(x, y, 9, 7, K(255, 208, 56));
    fill_circle(x - 2, y - 1, 2, K(235, 160, 30));
    fill_circle(x + 4, y + 2, 2, K(235, 160, 30));
}

/* a lion, feet at (cx, cy), facing the viewer a little to the right */
static void lion_(int cx, int cy, int asleep, int mouth, int roar)
{
    uint16_t ta = K(232, 172, 72), ma = K(168, 90, 30), ink = K(40, 24, 24), wh = K(255, 255, 255);
    int g = mouth * 12 / 255 + (roar ? 8 : 0), hy = asleep ? -40 : -50;
    AL(-44, -20, -64, -44, 3, ta);
    AE(-64, -48, 6, 9, ma);
    AE(-8, -30, 42, 25, ta);
    AB(-34, -14, 13, 14, ta);
    AB(12, -14, 13, 14, ta);
    AE(-28, -1, 9, 4, K(250, 200, 120));
    AE(18, -1, 9, 4, K(250, 200, 120));
    AE(36, hy, 31, 31, ma);
    AE(20, hy - 28, 8, 8, ta);
    AE(52, hy - 28, 8, 8, ta);
    AE(36, hy + 2, 21, 20, ta);
    AE(36, hy + 8, 12, 8, K(250, 224, 170));
    if (asleep) {
        AL(24, hy - 4, 32, hy - 2, 1, ink);
        AL(40, hy - 2, 48, hy - 4, 1, ink);
    } else {
        AE(28, hy - 4, 5, 6, wh);
        AE(44, hy - 4, 5, 6, wh);
        AE(29, hy - 4, 2, 3, ink);
        AE(45, hy - 4, 2, 3, ink);
        AL(22, hy - 11, 31, hy - 8, 2, ink);                     /* angry eyebrows */
        AL(50, hy - 11, 41, hy - 8, 2, ink);
    }
    AT(32, hy + 4, 40, hy + 4, 36, hy + 10, K(130, 50, 56));
    if (g > 2)
        AE(36, hy + 14 + g / 4, 7, 2 + g / 2, K(150, 30, 50));
    else
        AL(30, hy + 14, 36, hy + 11, 1, ink);
}

static void rat_(int cx, int cy, int dir)                       /* a mouse-like rat, feet at (cx, cy); dir 1: faces right */
{
    uint16_t gr = K(170, 170, 184), pk = K(255, 160, 176), ink = K(30, 24, 30);
    int w = isin((int)(TM / 6)) * 3 >> 7;
    AL(-dir * 12, -6, -dir * 28, -10 + w, 1, pk);
    AE(0, -8, 12, 8, gr);
    AE(dir * 13, -9, 7, 6, gr);
    AE(dir * 9, -15, 3, 4, pk);
    AE(dir * 20, -9, 2, 2, pk);
    AE(dir * 15, -11, 1, 1, ink);
    AL(dir * 18, -8, dir * 27, -10, 1, ink);
    AB(-7, -2, 4, 2, pk);
    AB(5, -2, 4, 2, pk);
}

static void bg_fable(void)
{
    int w = sc.world - FABLE_WORLD0;
    if (w == 0)
        bg_bird();
    else if (w == 1)
        bg_cow();
    else
        bg_jungle();
}

static void act_fable(int cx0, int cy0)
{
    int w = sc.world - FABLE_WORLD0, part = iclamp(spart(w), 0, 8), cx = cx0, cy = cy0, g = sc.mouth, sw = g_sw, sh = g_sh;
    if (w == 0) {                                                /* the fox below, the raven on his branch */
        int up = (part >= 2 && part <= 5) ? 100 : 0, talk = (part == 3 || part == 4 || part == 6 || part == 7) ? g : 0;
        if (part >= 2)
            fox_(cx, cy, up, talk, part >= 6);
        {
            int rx = cx + X(106), ry = cy + Y(-48), ob = part == 5 ? 8 : 0;
            raven_(rx, ry, ob, part <= 4, part >= 6);
        }
    } else if (w == 1) {                                         /* the ox on the left, the frog who swells on the right */
        static const uint8_t SZ[9] = {55, 55, 62, 80, 80, 100, 118, 40, 55};
        g_sw = sw * 70 / 100;
        g_sh = sh * 70 / 100;
        SCENES[1].actor(cx - X(52), cy);
        g_sw = sw * SZ[part] / 100;
        g_sh = sh * SZ[part] / 100;
        SCENES[3].actor(cx + X(60), cy);
        g_sw = sw;
        g_sh = sh;
    } else {                                                     /* the lion and the rat */
        int asleep = part <= 1, roar = part == 5, rx = cx + X(130), ry = cy;
        int mouth = (part == 3 || part == 4 || part == 5 || part == 7 || part == 8) ? g : (part == 2 ? 120 : 0);
        lion_(cx, cy, asleep, mouth + (part == 4 ? 60 + (isin((int)(TM / 40)) >> 2) : 0), roar);
        if (part == 1) {
            rx = cx + X(0) + (isin((int)(TM / 70)) >> 4);
            ry = cy + Y(-58);
            rat_(rx, ry, 1);
        } else if (part == 2 || part == 3)
            rat_(cx + X(72), cy + Y(-34), -1);
        else if (part == 4)
            rat_(cx + X(110) + (int)((TM / 8u) % 70u), cy, 1);
        else if (part == 5)
            rat_(cx + X(150), cy, -1);
        else if (part == 6)
            rat_(cx + X(46) + (isin((int)(TM / 30)) >> 5), cy + Y(-4), -1);
        else if (part >= 7)
            rat_(cx + X(86), cy, -1);
    }
}

static void fg_fable(void)
{
    int w = sc.world - FABLE_WORLD0, part = iclamp(spart(w), 0, 8), i;
    if (w == 0 && part == 5)                                     /* the cheese falls, straight into the fox's mouth */
        cheese_(138, 106 + (int)((TM / 9u) % 36u));
    if (w == 1 && part == 7) {                                   /* pouf! */
        text_out(168, 100, &FONT_B, "POUF!", K(255, 255, 255), K(200, 40, 60));
        for (i = 0; i < 6; i++)
            sparkle(168 + icos(i * 43 + (int)(TM / 20)) / 3, 150 + isin(i * 43 + (int)(TM / 20)) / 4, 3, K(255, 240, 150));
    }
    if (w == 2 && (part == 5 || part == 6)) {                    /* the hunters' net */
        for (i = 0; i < 9; i++) {
            box_(10 + i * 18, 80, 1, 130, K(150, 110, 60));
            box_(10, 80 + i * 16, 150, 1, K(150, 110, 60));
        }
    } else if (w == 2 && part >= 7) {                            /* cut cords on the ground */
        for (i = 0; i < 5; i++)
            fat_line(20 + i * 30, 214 + (i & 1) * 6, 38 + i * 30, 208, 1, K(150, 110, 60));
    }
}
