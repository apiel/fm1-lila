/* SPDX-License-Identifier: GPL-3.0-only */
/* Worlds 4..7: the spaceship in the galaxy, the meteor shower, the block world, the dinosaurs. */
#pragma once

/* ----------------------------------------------------------------- 4 SPACE --- */
static void planet(int x, int y, int r, uint16_t c1, uint16_t c2, uint16_t ring)
{
    fill_circle(x, y, r, c1);
    fill_ellipse(x - r / 4, y + r / 4, r * 3 / 4, r / 2, c2);
    fill_ellipse(x, y, r * 7 / 4, r / 4, ring);
    fill_circle(x, y - r / 4, r * 2 / 3, c1);
    fill_ellipse(x, y + 1, r * 7 / 4, r / 6, ring);
}
static void bg_space(void)
{
    int i, spd = 1 + sc.bounce / 90;
    vgrad(0, 240, K(6, 4, 30), K(46, 14, 80));
    fill_circle(190, 60, 46, blend(K(6, 4, 30), K(110, 40, 160), 3));      /* a nebula */
    fill_circle(60, 170, 60, blend(K(6, 4, 30), K(40, 70, 170), 3));
    for (i = 0; i < 46; i++) {                          /* three layers of stars, rushing past */
        int layer = i % 3, sp = 1 + layer * 2, y = (i * 53 + 11) % 236;
        int x = 239 - (int)(((TM / 14u) * (unsigned)(sp * spd) + (unsigned)i * 41u) % 240u);
        int len = 1 + layer * (1 + sc.lvl / 90) * spd;
        if (len > 1)
            fat_line(x, y, x + len * 3, y, 1, K(200 + layer * 25, 210 + layer * 20, 255));
        else
            cv_pset(x, y, K(220, 230, 255));
        if (layer == 2 && (i & 3) == 0)
            sparkle(x, y, 2, K(255, 255, 220));
    }
    planet(40, 56, 18, K(255, 150, 90), K(220, 100, 60), K(255, 220, 150));
    fill_circle(212, 150, 9, K(170, 190, 220));
    fill_circle(209, 148, 2, K(130, 150, 190));
}
static void act_space(int cx, int cy)
{
    int bob = isin((int)(TM / 9)) * 3 >> 7, fl = 18 + sc.lvl / 7 + (int)(urand() % 8u), ln = sc.lean / 2;
    uint16_t sil = K(236, 240, 250), red = K(240, 70, 70), shade = K(190, 200, 225), win = K(120, 210, 255);
    cy += bob;
    AT(-8, 40, 8, 40, 0, 40 + fl, K(255, 150, 40));
    AT(-4, 40, 4, 40, 0, 40 + fl * 6 / 10, K(255, 240, 120));
    AT(-18, 6, -38, 48, -17, 34, red);
    AT(18, 6, 38, 48, 17, 34, red);
    AE(0, 0, 20, 42, sil);
    AE(8, 4, 12, 36, shade);
    AE(-3, 0, 14, 40, sil);
    AT(-19, -24, 19, -24, ln, -66, red);
    AB(-20, 14, 40, 6, red);
    AE(0, -8, 12, 12, K(70, 80, 120));
    AE(0, -8, 9, 9, win);
    AE(-3, -12, 3, 3, K(220, 245, 255));
    AE(0, -5, 5, 5, K(255, 205, 175));
    AE(-2, -6, 1, 1, K(40, 30, 40));
    AE(2, -6, 1, 1, K(40, 30, 40));
    if (sc.mouth > 60)
        AE(0, -2, 2, 2, K(150, 50, 60));
    AE(-8, 26, 4, 4, K(110, 120, 150));
    AE(8, 26, 4, 4, K(110, 120, 150));
}
static void fg_space(void)
{
    if (sc.hit > 150) {                                   /* a shooting star */
        int p = 255 - sc.hit;
        fat_line(240 - p * 3, 20 + p, 270 - p * 3, 8 + p, 1, K(255, 255, 255));
    }
}

/* --------------------------------------------------------------- 5 METEOR --- */
static void bg_meteor(void)
{
    int i;
    vgrad(0, 212, K(20, 8, 40), K(140, 50, 60));
    for (i = 0; i < 30; i++)
        if (isin((int)(TM / 10) + i * 30) > -40)
            cv_pset((i * 79) % 240, (i * 41) % 150, K(255, 230, 220));
    fill_circle(196, 52, 22, K(255, 190, 130));                 /* a far moon */
    fill_circle(204, 46, 5, K(230, 160, 110));
    for (i = 0; i < 5; i++) {                                   /* meteors across the sky */
        int p = (int)((TM / 9u + (unsigned)i * 97u) % 330u), x = 260 - p - i * 14, y = p * 2 / 3 - 70 + i * 24;
        fat_line(x + 46, y - 30, x, y, 1, K(255, 190, 90));
        fat_line(x + 22, y - 14, x, y, 2, K(255, 120, 50));
        fill_circle(x, y, 4, K(255, 240, 200));
    }
    hills(196, 6, 120, 30, 240, K(90, 50, 70));
    hills(210, 5, 90, 90, 240, K(60, 30, 50));
    fill_ellipse(40, 214, 26, 6, K(40, 20, 36));
    fill_ellipse(200, 222, 32, 7, K(40, 20, 36));
    fill_ellipse(120, 232, 40, 6, K(40, 20, 36));
}
static void act_meteor(int cx, int cy)
{
    int wob = isin((int)(TM / 11)) * 4 >> 7, fl = 24 + sc.lvl / 6 + (int)(urand() % 10u), g = sc.mouth * 14 / 255;
    uint16_t rock = K(140, 104, 90), dk = K(96, 66, 60), lt = K(190, 150, 120), ink = K(30, 20, 30);
    cy += wob;
    AT(10, -24, 60 + fl, -64 - fl / 3, 28, -4, K(255, 110, 40));
    AT(14, -14, 46 + fl * 7 / 10, -48 - fl / 4, 28, 6, K(255, 180, 60));
    AT(20, -8, 36 + fl / 2, -30, 30, 10, K(255, 240, 150));
    AE(0, 0, 40, 38, rock);
    AE(-10, -10, 26, 22, lt);
    AE(0, 0, 40, 38, rock);
    AE(-10, -12, 24, 20, K(166, 128, 104));
    AE(-26, 18, 9, 7, dk);
    AE(24, -22, 7, 6, dk);
    AE(26, 18, 6, 5, dk);
    AE(-28, -20, 5, 4, dk);
    AE(-12, -8, 9, 10, K(255, 255, 255));
    AE(12, -8, 9, 10, K(255, 255, 255));
    AE(-11 + (sc.lean >> 3), -7, 4, 6, ink);
    AE(13 + (sc.lean >> 3), -7, 4, 6, ink);
    if (g > 2)
        AE(0, 14, 11, 4 + g / 2, K(120, 30, 50));
    else
        AB(-8, 14, 16, 2, ink);
    AE(-22, 6, 5, 3, K(255, 140, 140));
    AE(22, 6, 5, 3, K(255, 140, 140));
}
static void fg_meteor(void)
{
    if (sc.hit > 120) {                                         /* a meteor lands: a flash and a ring of fire */
        int x = 20 + sc.pitch * 200 / 255, r = (255 - sc.hit) / 2 + 4;
        ring_(x, 222, r, 3, K(255, 160, 60));
        fill_circle(x, 222, 4 + sc.hit / 40, K(255, 240, 160));
    }
}

/* ---------------------------------------------------------------- 6 BLOCKS --- */
static void blk(int x, int y, int s, uint16_t c)            /* a lit cube face */
{
    box_(x, y, s, s, c);
    box_(x, y, s, 2, blend(c, K(255, 255, 255), 4));
    box_(x, y + s - 2, s, 2, blend(c, K(0, 0, 0), 4));
}
static void bg_blocks(void)
{
    int i, x;
    vgrad(0, 180, K(110, 190, 255), K(190, 228, 255));
    box_(190, 22, 28, 28, K(255, 240, 120));                    /* a square sun */
    box_(186, 18, 36, 36, blend(K(255, 240, 120), K(150, 210, 255), 10));
    box_(190, 22, 28, 28, K(255, 244, 150));
    for (i = 0; i < 3; i++) {
        int cxp = ((int)(TM / 70) + i * 100) % 320 - 60;
        box_(cxp, 40 + i * 18, 52, 12, K(255, 255, 255));
        box_(cxp + 10, 32 + i * 18, 28, 10, K(255, 255, 255));
    }
    for (x = 0; x < 240; x += 16) {                             /* far hills in steps */
        int h = 20 + ((x / 16 * 7) % 5) * 8;
        box_(x, 150 - h, 16, h + 30, K(90, 170, 90));
        box_(x, 150 - h, 16, 4, K(110, 195, 100));
    }
    for (x = 0; x < 240; x += 16)                               /* the ground, in cubes */
        for (i = 0; i < 4; i++) {
            int y = 172 + i * 16;
            uint16_t c = i == 0 ? K(100 + (x / 16 % 3) * 10, 190, 70) : K(140 + ((x / 16 + i) % 3) * 8, 100, 64);
            blk(x, y, 16, c);
        }
    box_(26, 118, 10, 40, K(110, 80, 50));                      /* a tree */
    box_(10, 82, 42, 24, K(60, 150, 60));
    box_(18, 62, 26, 24, K(70, 165, 70));
    box_(204, 138, 8, 34, K(110, 80, 50));
    box_(190, 110, 36, 22, K(60, 150, 60));
}
static void act_blocks(int cx, int cy)
{
    int g = sc.mouth * 8 / 255, leg = isin((int)(TM / 8)) > 60 ? 3 : 0, ln = sc.lean / 3;
    uint16_t pk = K(255, 160, 180), pk2 = K(240, 120, 150), sn = K(255, 190, 200), ink = K(30, 24, 40);
    AB(-30, -24, 12, 24, pk2);
    AB(-8, -24, 12, 24 - leg, pk2);
    AB(14, -24, 12, 24, pk2);
    AB(32, -24, 12, 24 - leg, pk2);
    AB(-42, -62, 94, 42, pk);
    AB(-42, -62, 94, 6, K(255, 190, 205));
    AB(-34, -36, 20, 16, K(255, 130, 160));
    AB(-8, -52, 18, 14, K(255, 130, 160));
    AB(-52, -64, 12, 10, pk2);
    AB(30 + ln, -90, 46, 44, pk);
    AB(30 + ln, -90, 46, 6, K(255, 190, 205));
    AB(36 + ln, -84 + 0, 8, 10, K(255, 255, 255));
    AB(58 + ln, -84, 8, 10, K(255, 255, 255));
    AB(40 + ln, -82, 4, 8, ink);
    AB(62 + ln, -82, 4, 8, ink);
    AB(40 + ln, -72 + g / 2, 26, 22 + g / 2, sn);
    AB(46 + ln, -66 + g / 2, 6, 6, pk2);
    AB(56 + ln, -66 + g / 2, 6, 6, pk2);
    if (g > 2)
        AB(44 + ln, -56 + g / 2, 18, 3 + g / 2, K(120, 30, 50));
    AB(32 + ln, -96, 10, 8, pk2);
    AB(66 + ln, -96, 10, 8, pk2);
}
static void fg_blocks(void)
{
    if (sc.hit > 100) {                                           /* a block pops out of the ground */
        int x = 8 + (sc.pitch * 200 / 255 & ~15), y = 156 - sc.hit / 8;
        blk(x, y, 16, K(250, 200 - sc.pitch / 3, 60));
    }
}

/* ------------------------------------------------------------------ 7 DINO --- */
static void bg_dino(void)
{
    int i;
    vgrad(0, 196, K(255, 150, 100), K(255, 214, 140));
    fill_circle(70, 120, 36, K(255, 230, 150));                 /* a low sun */
    fill_circle(70, 120, 28, K(255, 246, 190));
    fill_tri(150, 190, 240, 190, 214, 60, K(110, 70, 60));      /* the volcano */
    fill_tri(150, 190, 240, 190, 200, 80, K(130, 84, 70));
    fill_tri(196, 62, 232, 62, 214, 90, K(255, 90, 40));
    for (i = 0; i < 4; i++) {                                   /* lava and smoke */
        int p = (int)((TM / 12u + (unsigned)i * 60u) % 150u), x = 214 + (isin(i * 70) * p >> 9), y = 62 - p + (p * p / 260);
        fill_circle(x, y, 4 + (sc.hit >> 6), K(255, 160 + i * 20, 40));
    }
    for (i = 0; i < 3; i++) {
        int p = (int)((TM / 30u + (unsigned)i * 40u) % 120u);
        fill_circle(214 + isin(i * 90 + (int)(TM / 20)) / 8, 50 - p / 2, 8 + p / 12, blend(K(120, 100, 100), K(255, 200, 150), 8 + p / 20));
    }
    {   /* a pterodactyl glides by */
        int x = (int)(TM / 18u % 320u) - 40, y = 50 + (isin((int)(TM / 25)) >> 3), f = isin((int)(TM / 6)) >> 3;
        fill_tri(x, y, x - 22, y - 10 - f, x - 8, y + 4, K(90, 60, 80));
        fill_tri(x, y, x + 20, y - 10 + f, x + 8, y + 4, K(90, 60, 80));
        fill_ellipse(x, y + 2, 7, 4, K(120, 80, 100));
        fill_tri(x + 6, y + 1, x + 20, y + 3, x + 6, y + 5, K(255, 190, 70));
    }
    hills(186, 6, 130, 10, 240, K(126, 150, 70));
    hat_ground(190, K(120, 140, 66), K(86, 100, 50));
    for (i = 0; i < 4; i++) {                                   /* ferns */
        int x = 20 + i * 62, sw = isin((int)(TM / 16) + i * 50) * 4 >> 7;
        fat_line(x, 200, x + sw, 170, 1, K(50, 110, 50));
        fat_line(x, 188, x - 14 + sw, 180, 1, K(60, 130, 55));
        fat_line(x, 188, x + 14 + sw, 180, 1, K(60, 130, 55));
        fat_line(x, 178, x - 10 + sw, 172, 1, K(70, 140, 60));
        fat_line(x, 178, x + 10 + sw, 172, 1, K(70, 140, 60));
    }
}
static void act_dino(int cx, int cy)
{
    int g = sc.mouth * 14 / 255, st = isin((int)(TM / 7)) > 80 ? 2 : 0, tw = isin((int)(TM / 12)) * 4 >> 7, ln = sc.lean / 2;
    uint16_t gr = K(100, 190, 90), dk = K(60, 140, 70), lt = K(210, 235, 140), ink = K(30, 30, 30), wh = K(255, 255, 255);
    int i;
    AT(-30, -70, -92, -26 + tw, -50, -30, dk);
    AT(-30, -64, -80, -22 + tw, -44, -44, gr);
    AE(-6, -62, 44, 34, gr);
    AE(6, -50, 32, 22, lt);
    for (i = 0; i < 5; i++)
        AT(-34 + i * 16, -90 + i * 3, -26 + i * 16, -102 + i * 3, -20 + i * 16, -88 + i * 3, K(230, 120, 60));
    AE(-12, -22, 18, 24, dk);
    AE(-10, -2 - st, 18, 8, dk);
    AE(26, -22, 14, 20, gr);
    AE(26, -2, 15, 6, gr);
    AE(52 + ln, -92, 32, 24, gr);
    AE(40 + ln, -80, 18, 16, gr);
    AE(66 + ln, -92 + g / 2, 18, 12, gr);
    AE(64 + ln, -78 + g, 22, 8, gr);
    if (g > 2) {
        AE(66 + ln, -82 + g / 2, 20, g / 2 + 1, K(140, 40, 60));
        for (i = 0; i < 4; i++)
            AT(52 + ln + i * 8, -86, 56 + ln + i * 8, -86, 54 + ln + i * 8, -80, wh);
    }
    AE(52 + ln, -100, 6, 6, wh);
    AE(54 + ln, -100, 3, 4, ink);
    AB(18, -68, 6, 4, dk);
    AB(24, -64, 6, 4, dk);
    AE(80 + ln, -96, 2, 2, ink);
}
static void fg_dino(void)
{
    if (sc.mouth > 70)
        bubble(8, 40, 96, 44, "ROAR!", 70, 100);
    if (sc.hit > 160)
        ring_(120, 188, (255 - sc.hit) + 20, 2, K(255, 240, 200));
}
