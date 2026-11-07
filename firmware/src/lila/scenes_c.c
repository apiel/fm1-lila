/* SPDX-License-Identifier: GPL-3.0-only */
/* Worlds 8..11: the robot workshop, the ocean, the fairy castle, the jungle. */
#pragma once

/* ---------------------------------------------------------------- 8 ROBOT --- */
static void gear(int x, int y, int r, int a0, uint16_t c)
{
    int i;
    for (i = 0; i < 8; i++) {
        int a = a0 + i * 32;
        fill_circle(x + icos(a) * (r + 3) / 127, y + isin(a) * (r + 3) / 127, 4, c);
    }
    fill_circle(x, y, r, c);
    fill_circle(x, y, r / 3, K(30, 36, 52));
}
static void bg_robot(void)
{
    int i, x;
    vgrad(0, 240, K(30, 40, 66), K(14, 18, 34));
    for (x = 0; x < 240; x += 24)
        box_(x, 0, 1, 240, K(40, 52, 84));
    for (i = 0; i < 240; i += 24)
        box_(0, i, 240, 1, K(40, 52, 84));
    gear(26, 40, 20, (int)(TM / 14), K(70, 90, 130));
    gear(66, 66, 12, 16 - (int)(TM / 10), K(90, 110, 150));
    gear(214, 50, 24, -(int)(TM / 18), K(70, 90, 130));
    for (i = 0; i < 16; i++) {                                   /* the equalizer on the wall */
        int h = 8 + ((isin((int)(TM / 6) + i * 37) + 127) * (20 + sc.lvl / 4) >> 8);
        uint16_t c = i < 5 ? K(80, 230, 120) : i < 11 ? K(250, 220, 70) : K(255, 90, 80);
        box_(52 + i * 8, 140 - h, 6, h, c);
    }
    box_(0, 190, 240, 50, K(60, 70, 100));
    box_(0, 190, 240, 4, K(120, 140, 190));
    for (x = 0; x < 240; x += 40)
        box_(x, 200, 20, 6, K(80, 92, 126));
}
static void act_robot(int cx, int cy)
{
    int arm = isin((int)(TM / 9)) * 6 >> 7, bl = ((int)(TM / 400u) & 7) == 0, n, ln = sc.lean / 3;
    uint16_t bd = K(160, 180, 210), bd2 = K(120, 140, 175), dk = K(40, 50, 80), eye = K(80, 240, 255), red = K(255, 70, 70);
    AB(-26, -22, 16, 22, bd2);
    AB(10, -22, 16, 22, bd2);
    AB(-32, -6, 26, 8, dk);
    AB(6, -6, 26, 8, dk);
    fill_rbox(cx + X(-34), cy + Y(-76), X(68), Y(58), X(8), bd);
    AB(-24, -64, 48, 30, dk);
    for (n = 0; n < 24; n++) {                                   /* the chest screen: the sound's wave */
        int y = isin(n * (14 + sc.wave * 9) + (int)(TM / 3)) * (2 + sc.lvl / 20) >> 7;
        AB(-22 + n * 2, -49 + y, 2, 3, n > 17 ? red : K(100, 255, 140));
    }
    AB(-18, -28, 36, 6, bd2);
    AL(-34, -66, -52, -42 + arm, 5, bd);
    AL(34, -66, 52, -42 - arm, 5, bd);
    AE(-52, -40 + arm, 7, 7, bd2);
    AE(52, -40 - arm, 7, 7, bd2);
    fill_rbox(cx + X(-30) + ln, cy + Y(-130), X(60), Y(46), X(10), bd);
    AB(-24 + ln / 2, -122, 48, 30, dk);
    if (!bl) {
        AE(-12 + ln / 2, -108, 8, 8, eye);
        AE(12 + ln / 2, -108, 8, 8, eye);
        AE(-12 + ln / 2, -108, 3, 3, K(255, 255, 255));
        AE(12 + ln / 2, -108, 3, 3, K(255, 255, 255));
    } else {
        AB(-20 + ln / 2, -108, 16, 3, eye);
        AB(4 + ln / 2, -108, 16, 3, eye);
    }
    AB(-14 + ln / 2, -96 + 0, 28, 3 + sc.mouth / 40, K(40, 255, 160));
    AL(0, -130, ln / 2, -146, 2, bd2);
    AE(ln / 2, -150, 6, 6, (((int)(TM / 250u)) & 1) ? red : K(150, 40, 40));
    if (sc.blipf > 40) {                                         /* chatter: sparks at the antenna */
        AL(ln / 2, -154, ln / 2 + 8, -164, 1, K(255, 255, 120));
        AL(ln / 2 + 8, -164, ln / 2 + 2, -170, 1, K(255, 255, 120));
        AL(ln / 2, -154, ln / 2 - 8, -163, 1, K(255, 255, 120));
    }
}
static void fg_robot(void)
{
    static const char *const SAY[6] = {"BEEP", "BOOP", "ZZZT", "BLIP", "BZZZ", "BING"};
    if (sc.hit > 160 || sc.blipf > 60)
        text_out(40 + sc.pitch / 2 + (sc.blipf > 60 ? 20 : 0), 110 - sc.hit / 10 - (sc.blipf > 60 ? 20 : 0), &FONT_B, SAY[(sc.score / 10 + (sc.blipf > 60 ? 3 : 0)) % 6],
                 K(120, 255, 160), K(0, 40, 30));
}

/* ---------------------------------------------------------------- 9 OCEAN --- */
static void bg_ocean(void)
{
    int i;
    vgrad(0, 240, K(90, 210, 230), K(10, 50, 120));
    for (i = 0; i < 5; i++) {                                    /* the sun's rays through the water */
        int x = 30 + i * 52 + (isin((int)(TM / 40) + i * 50) >> 4);
        fill_tri(x, 0, x + 26, 0, x - 20 + (i * 20), 200, blend(K(90, 210, 230), K(220, 255, 255), 3));
    }
    for (i = 0; i < 5; i++) {                                    /* little fish */
        int dir = (i & 1) ? 1 : -1, x = (int)(((TM / (unsigned)(18 + i * 6)) + (unsigned)i * 53u) % 300u), y = 40 + i * 28;
        int fx = dir > 0 ? x - 30 : 270 - x, tail = isin((int)(TM / 5) + i * 40) >> 4;
        uint16_t c = K(255, 120 + i * 24, 60 + i * 30);
        fill_ellipse(fx, y, 10, 6, c);
        fill_tri(fx - dir * 8, y, fx - dir * 17, y - 6 + tail / 2, fx - dir * 17, y + 6 + tail / 2, c);
        cv_pset(fx + dir * 5, y - 2, K(0, 0, 0));
    }
    for (i = 0; i < 3; i++) {                                    /* jellyfish, glowing */
        int jx = 30 + i * 90 + (isin((int)(TM / 30) + i * 70) >> 3), jy = 150 + (isin((int)(TM / 22) + i * 90) >> 2) - i * 14, pulse = isin((int)(TM / 9) + i * 50) >> 5, j;
        uint16_t jc = blend(K(90, 210, 230), K(255, 150, 220), 9 + i);
        for (j = -2; j <= 2; j++)
            fat_line(jx + j * 4, jy, jx + j * 5 + (isin((int)(TM / 14) + j * 40 + i * 30) >> 4), jy + 22 + pulse, 1, jc);
        fill_ellipse(jx, jy - 2, 14 + pulse / 2, 10 - pulse / 2, jc);
        fill_ellipse(jx - 3, jy - 5, 5, 3, blend(jc, K(255, 255, 255), 8));
    }
    hills(214, 8, 150, 20, 240, K(222, 200, 140));
    for (i = 0; i < 7; i++) {                                    /* seaweed */
        int x = 12 + i * 36, h = 40 + (i * 13 % 30), j;
        for (j = 0; j < 5; j++) {
            int sw = isin((int)(TM / 14) + i * 40 + j * 30) * (j + 1) >> 6;
            fat_line(x + (j ? isin((int)(TM / 14) + i * 40 + (j - 1) * 30) * j >> 6 : 0), 226 - j * h / 5, x + sw, 226 - (j + 1) * h / 5, 2, K(40, 160 + i * 6, 90));
        }
    }
    fill_circle(30, 226, 7, K(255, 170, 190));
    fill_circle(204, 230, 6, K(255, 240, 200));
    for (i = 0; i < 3; i++) {
        int b = (int)((TM / 20u + (unsigned)i * 70u) % 220u);
        ring_(40 + i * 80 + (isin((int)(TM / 17) + i * 60) >> 4), 230 - b, 3 + i, 1, K(220, 250, 255));
    }
}
static void act_ocean(int cx, int cy)
{
    int bob = isin((int)(TM / 12)) * 5 >> 7, g = sc.mouth * 12 / 255, tail = isin((int)(TM / 6)) * 8 >> 7, ln = sc.lean / 3;
    uint16_t bl = K(80, 140, 230), dk = K(50, 100, 190), lt = K(210, 235, 255), ink = K(20, 30, 60);
    cy += bob;
    AT(-40, 0, -86, -30 + tail, -70, -4 + tail / 2, dk);
    AT(-40, 4, -86, 34 + tail, -66, 10 + tail / 2, dk);
    AE(0, 0, 56, 38, bl);
    AE(6, 14, 44, 22, lt);
    AE(-6, -20, 40, 14, K(110, 170, 240));
    AT(0, 22, 18, 44, 24, 16, dk);
    AE(30 + ln, -6, 7, 8, K(255, 255, 255));
    AE(32 + ln, -6, 3, 5, ink);
    AE(26 + ln, 8, 6, 3, K(255, 150, 160));
    if (g > 2)
        AE(34 + ln, 14 + g / 3, 12, 2 + g / 2, K(120, 30, 60));
    else
        AL(24 + ln, 16, 44 + ln, 14, 1, ink);
    AL(-4, -38, -4, -52, 2, K(170, 220, 255));
    AL(-4, -52, -14, -62, 2, K(170, 220, 255));
    AL(-4, -52, 6, -62, 2, K(170, 220, 255));
}
static void fg_ocean(void)
{
    static uint32_t last, last2;
    if (sc.mouth > 90 && sc.t - last > 120u) {                   /* the whale's spout makes bubbles */
        last = sc.t;
        part_add(PK_BUBBLE, 116 + rnd_range(-8, 8), 56, rnd_range(-4, 4), rnd_range(-30, -18), 50, rnd_range(3, 7), RGB(220, 250, 255), 0);
    }
    if (sc.mouth > 60 && sc.t - last2 > 260u) {                  /* the song: rings that grow from its mouth */
        last2 = sc.t;
        part_add(PK_RING, 112 + 36 * sc.sz / 100, 118 - sc.jump, 0, 0, 34, 12, RGB(190, 235, 255), 0);
    }
}

/* --------------------------------------------------------------- 10 CASTLE --- */
static void bg_castle(void)
{
    int i;
    static const uint32_t RB[7] = {0xFF5050, 0xFFA040, 0xFFE050, 0x60E060, 0x50C0FF, 0x7070FF, 0xC060FF};
    vgrad(0, 216, K(120, 90, 200), K(255, 200, 220));
    for (i = 0; i < 24; i++)
        if (isin((int)(TM / 12) + i * 29) > -30)
            sparkle((i * 83) % 240, (i * 47) % 110, 1 + (isin((int)(TM / 9) + i * 11) > 90), K(255, 250, 220));
    for (i = 0; i < 7; i++)                                       /* a rainbow behind the hills */
        ring_(100, 200, 150 - i * 7, 8, KC(RB[i]));
    box_(86, 100, 68, 90, K(240, 220, 250));                      /* the castle */
    box_(60, 120, 30, 70, K(230, 205, 245));
    box_(150, 120, 30, 70, K(230, 205, 245));
    box_(106, 70, 28, 40, K(250, 232, 255));
    fill_tri(54, 122, 96, 122, 75, 84, K(240, 100, 160));
    fill_tri(144, 122, 186, 122, 165, 84, K(240, 100, 160));
    fill_tri(100, 72, 140, 72, 120, 30, K(130, 90, 230));
    fill_tri(86, 100, 154, 100, 120, 80, K(240, 100, 160));
    box_(120, 14, 2, 16, K(255, 255, 255));
    fill_tri(122, 14, 138, 19, 122, 24, K(255, 90, 120));
    for (i = 0; i < 3; i++) {
        int on = isin((int)(TM / 60) + i * 80) > -90;
        box_(72 + i * 0, 134, 6, 10, on ? K(255, 230, 120) : K(120, 90, 150));
        break;
    }
    box_(164, 134, 6, 10, K(255, 230, 120));
    box_(116, 90, 8, 12, K(255, 230, 120));
    fill_ellipse(120, 188, 14, 20, K(130, 80, 90));
    hills(196, 10, 160, 40, 240, K(250, 180, 220));
    hills(212, 8, 110, 130, 240, K(240, 150, 210));
}
static void act_castle(int cx, int cy)
{
    int g = sc.mouth * 6 / 255, nod = isin((int)(TM / 12)) * 2 >> 7, tw = isin((int)(TM / 10)) * 5 >> 7, ln = sc.lean / 2, i;
    static const uint32_t RB[6] = {0xFF5050, 0xFFA040, 0xFFE050, 0x60E060, 0x50C0FF, 0xC060FF};
    uint16_t wh = K(255, 255, 255), sh = K(225, 220, 245), ink = K(40, 30, 70);
    for (i = 0; i < 6; i++)
        AL(-44, -66 + i * 3, -68 - i * 2 + tw, -40 + i * 7, 3, KC(RB[i]));
    for (i = 0; i < 4; i++) {
        int x = (i == 0 ? -34 : i == 1 ? -20 : i == 2 ? 14 : 28);
        AB(x - 1, -35, 12, 36, K(170, 150, 200));
        AB(x, -34, 10, 34, i & 1 ? sh : wh);
        AB(x, -5, 10, 5, K(255, 215, 80));
    }
    fill_rbox(cx + X(-50), cy + Y(-86), X(100), Y(58), X(24), K(170, 150, 200));
    fill_rbox(cx + X(-48), cy + Y(-84), X(96), Y(54), X(22), wh);
    AE(-10, -44, 30, 8, sh);
    AL(30, -76, 50 + ln, -92 + nod, 12, wh);
    AE(54 + ln, -98 + nod, 19, 17, K(170, 150, 200));
    AE(54 + ln, -98 + nod, 17, 15, wh);
    AE(68 + ln, -92 + nod + g / 2, 12, 8 + g / 2, K(255, 215, 235));
    AT(54 + ln, -108 + nod, 62 + ln, -112 + nod, 62 + ln, -142 + nod, K(255, 215, 80));
    AT(54 + ln, -108 + nod, 62 + ln, -112 + nod, 56 + ln, -140 + nod, K(255, 235, 140));
    AE(48 + ln, -106 + nod, 8, 6, wh);
    for (i = 0; i < 6; i++)
        AL(40 + ln, -110 + nod + i * 4, 26 - i * 3 + ln - tw / 2, -104 + i * 8 + nod, 3, KC(RB[i]));
    AE(58 + ln, -102 + nod, 4, 5, ink);
    AE(59 + ln, -103 + nod, 1, 1, wh);
    AE(74 + ln, -92 + nod, 1, 1, K(200, 100, 140));
    AE(46 + ln, -92 + nod, 5, 3, K(255, 170, 200));
}
static void fg_castle(void)
{
    int i;
    for (i = 0; i < 4; i++) {
        int x = 20 + i * 62 + (isin((int)(TM / 18) + i * 60) >> 3), y = 224 + (icos((int)(TM / 15) + i * 40) >> 4);
        fill_circle(x, y, 3, K(255, 250, 160));
        sparkle(x, y, 5, K(255, 255, 200));
    }
}

/* -------------------------------------------------------------- 11 JUNGLE --- */
static void bg_jungle(void)
{
    int i;
    vgrad(0, 200, K(150, 220, 150), K(230, 250, 170));
    sun(196, 40, 17, K(255, 245, 160), K(255, 230, 120));
    for (i = 0; i < 4; i++) {                                    /* palm trees */
        int x = 30 + i * 62, h = 90 + (i * 29 % 40), sw = isin((int)(TM / 25) + i * 70) * 3 >> 7;
        fat_line(x, 190, x + 6 + sw / 2, 190 - h, 4, K(130, 90, 50));
        fat_line(x + 6 + sw / 2, 190 - h, x + 38 + sw, 190 - h + 20, 3, K(50, 140, 60));
        fat_line(x + 6 + sw / 2, 190 - h, x - 28 + sw, 190 - h + 20, 3, K(50, 140, 60));
        fat_line(x + 6 + sw / 2, 190 - h, x + 24 + sw, 190 - h - 12, 3, K(70, 160, 70));
        fat_line(x + 6 + sw / 2, 190 - h, x - 14 + sw, 190 - h - 12, 3, K(70, 160, 70));
    }
    hills(190, 6, 140, 10, 240, K(80, 170, 70));
    hat_ground(194, K(70, 150, 60), K(110, 90, 50));
    for (i = 0; i < 3; i++) {                                    /* vines */
        int x = 20 + i * 100, j;
        for (j = 0; j < 6; j++)
            fat_line(x + (isin((int)(TM / 22) + i * 60 + j * 20) * j >> 6), j * 14, x + (isin((int)(TM / 22) + i * 60 + (j + 1) * 20) * (j + 1) >> 6),
                     (j + 1) * 14, 1, K(40, 110, 50));
    }
    fill_ellipse(-6, 20, 40, 18, K(40, 120, 50));
    fill_ellipse(246, 14, 40, 20, K(40, 120, 50));
}
static void act_jungle(int cx, int cy)
{
    int g = sc.mouth, up = g * 34 / 255 + sc.camt / 7, nod = isin((int)(TM / 13)) * 2 >> 7, ln = sc.lean / 2, ear = isin((int)(TM / 8)) * 3 >> 7;
    uint16_t gy = K(150, 160, 190), dk = K(116, 126, 160), pk = K(255, 190, 200), wh = K(255, 252, 240), ink = K(30, 30, 40);
    int i;
    for (i = 0; i < 4; i++) {
        int x = (i == 0 ? -40 : i == 1 ? -22 : i == 2 ? 16 : 34);
        AB(x, -34, 15, 34, i & 1 ? dk : gy);
        AB(x, -6, 15, 6, K(210, 200, 190));
    }
    AL(-52, -74, -60, -50, 2, dk);
    fill_rbox(cx + X(-54), cy + Y(-92), X(112), Y(64), X(28), gy);
    AE(46 + ln, -72 + nod, 28, 26, gy);
    AE(26 + ln, -66 + nod + ear, 22, 28, dk);
    AE(28 + ln, -66 + nod + ear, 14, 20, pk);
    AE(56 + ln, -76 + nod, 5, 5, wh);
    AE(57 + ln, -76 + nod, 3, 3, ink);
    for (i = 0; i < 6; i++) {                                    /* the trunk hangs, and curls up when it sings */
        int k0 = i * 10, k1 = (i + 1) * 10;
        int tx0 = 68 + (i * 3) + (i * i * up) / 22, ty0 = -62 + k0 - (i * i * up) / 9;
        int tx1 = 68 + ((i + 1) * 3) + ((i + 1) * (i + 1) * up) / 22, ty1 = -62 + k1 - ((i + 1) * (i + 1) * up) / 9;
        AL(tx0 + ln, ty0 + nod, tx1 + ln, ty1 + nod, 11 - i, gy);
    }
    AT(60 + ln, -52 + nod, 74 + ln, -42 + nod, 68 + ln, -60 + nod, wh);
    AE(44 + ln, -88 + nod, 5, 3, K(255, 160, 170));
}
static void fg_jungle(void)
{
    static uint32_t last;
    if (sc.mouth > 100 && sc.t - last > 90u) {                    /* a trumpet: water from the trunk */
        last = sc.t;
        part_add(PK_BUBBLE, 130 + rnd_range(-4, 4), 108, rnd_range(4, 20), rnd_range(-20, -8), 30, rnd_range(2, 4), RGB(190, 230, 255), 0);
    }
    grass_tufts(228, 12, K(50, 140, 55), 7);
}
