/* SPDX-License-Identifier: GPL-3.0-only */
/* Worlds 0..3: the bird in the sun, the cow on the farm, the dog in the garden, the frog on the pond. */
#pragma once

/* a speech bubble with a word in it, its tail pointing to (tx, ty) */
static void bubble(int x, int y, int w, int h, const char *word, int tx, int ty)
{
    uint16_t white = K(255, 255, 255), ink = K(30, 30, 50);
    fill_rbox(x, y, w, h, 10, ink);
    fill_rbox(x + 2, y + 2, w - 4, h - 4, 8, white);
    fill_tri(x + w / 2 - 6, y + h - 4, x + w / 2 + 8, y + h - 4, tx, ty, ink);
    fill_tri(x + w / 2 - 4, y + h - 5, x + w / 2 + 6, y + h - 5, tx, ty - 3, white);
    text_big(x + w / 2, y + (h - 16 * 2) / 2 + 2, &FONT_B, word, ink, 2);
}

/* ------------------------------------------------------------------ 0 BIRD --- */
static void bg_bird(void)
{
    int i;
    vgrad(0, 206, K(96, 190, 255), K(210, 238, 255));
    sun(200, 44, 20, K(255, 236, 120), K(255, 222, 100));
    cloud(((int)(TM / 60) + 30) % 300 - 40, 40, 10, K(255, 255, 255));
    cloud(((int)(TM / 90) + 150) % 300 - 40, 80, 8, K(250, 252, 255));
    cloud(((int)(TM / 130) + 220) % 300 - 40, 22, 7, K(245, 250, 255));
    hills(184, 12, 170, (int)(TM / 90), 240, K(120, 205, 100));
    hills(204, 10, 120, (int)(TM / 60) + 40, 240, K(85, 180, 75));
    fat_line(-6, 160, 236, 150, 5, K(122, 82, 46));
    fat_line(190, 152, 236, 120, 3, K(122, 82, 46));
    for (i = 0; i < 7; i++) {
        int x = 12 + i * 34, sw = (isin((int)(TM / 14) + i * 40) * 2) >> 7;
        fill_ellipse(x + sw, 150 + (i & 1) * 14 - 10, 12, 6, K(70 + i * 6, 170, 70));
        fill_ellipse(x + 12 + sw, 164 - (i & 1) * 6, 10, 5, K(90, 190 - i * 4, 80));
    }
}
static void act_bird(int cx, int cy)
{
    int g = sc.mouth * 10 / 255, fl = sc.hit * isin((int)(TM / 4)) >> 12, wag = isin((int)(TM / 10)) * 3 >> 7;
    int ln = sc.lean * 3 / 4;
    uint16_t yel = K(255, 212, 58), dk = K(240, 158, 30), belly = K(255, 242, 176), org = K(255, 128, 30), ink = K(30, 24, 40);
    AL(-4, -10, -4, 0, 2, org);
    AL(9, -10, 10, 0, 2, org);
    AT(-14, -22, -52, -40 + wag, -50, -14 + wag, dk);
    AT(-14, -26, -46, -26 + wag, -44, -10 + wag, K(255, 180, 40));
    AE(0, -29, 27, 22, yel);
    AE(9, -22, 16, 14, belly);
    AE(-8 - fl / 2, -30 + fl, 16, 10 + (fl > 0 ? fl / 2 : 0), dk);
    AE(-10 - fl / 2, -31 + fl, 9, 5, K(255, 190, 60));
    AT(12 + ln, -66, 16 + ln, -84, 22 + ln, -67, dk);
    AT(18 + ln, -67, 26 + ln, -82, 29 + ln, -65, dk);
    AE(18 + ln, -54, 18, 17, yel);
    AE(19 + ln, -46, 9, 5, K(255, 236, 150));
    AE(18 + ln, -48, 5, 3, K(255, 150, 140));
    AE(25 + ln, -58, 6, 6, K(255, 255, 255));
    AE(27 + ln, -58, 3, 4, ink);
    AE(28 + ln, -60, 1, 1, K(255, 255, 255));
    AT(33 + ln, -56, 50 + ln, -52 - g / 2, 33 + ln, -50 - g / 3, org);
    AT(33 + ln, -49 + g / 3, 47 + ln, -48 + g, 33 + ln, -44 + g / 2, K(235, 100, 20));
    if (g > 3)
        AT(33 + ln, -50 - g / 3, 44 + ln, -49 - g / 4, 33 + ln, -49 + g / 3, K(120, 30, 40));
}
static void fg_bird(void)
{
    int i;
    for (i = 0; i < 6; i++) {
        int x = 20 + i * 42, h = 220 + (i * 5 % 10), sway = (isin((int)(TM / 18) + i * 50) * 3) >> 7;
        fat_line(x, 240, x + sway, h, 1, K(60, 150, 60));
        fill_circle(x + sway, h - 2, 4, K(255, 90 + i * 25, 120));
        fill_circle(x + sway, h - 2, 2, K(255, 230, 80));
    }
}

/* ------------------------------------------------------------------- 1 COW --- */
static void bg_cow(void)
{
    int i;
    vgrad(0, 168, K(130, 200, 255), K(215, 238, 255));
    sun(40, 40, 16, K(255, 240, 140), K(255, 230, 120));
    cloud(((int)(TM / 70) + 120) % 300 - 40, 36, 9, K(255, 255, 255));
    cloud(((int)(TM / 110) + 40) % 300 - 40, 64, 7, K(250, 252, 255));
    hills(150, 8, 200, 20, 240, K(130, 205, 90));
    /* the barn */
    box_(160, 90, 72, 70, K(200, 50, 40));
    fill_tri(154, 92, 238, 92, 196, 58, K(150, 30, 28));
    box_(180, 112, 32, 48, K(250, 240, 230));
    fill_quad(180, 112, 184, 112, 212, 160, 208, 160, K(200, 50, 40));
    fill_quad(212, 112, 208, 112, 180, 160, 184, 160, K(200, 50, 40));
    box_(180, 112, 32, 3, K(200, 50, 40));
    fill_circle(196, 78, 6, K(250, 240, 230));
    hills(164, 6, 160, 60, 240, K(110, 190, 80));
    hat_ground(180, K(100, 180, 75), K(70, 150, 60));
    for (i = 0; i < 8; i++) {                         /* the fence */
        box_(4 + i * 32, 138, 5, 30, K(250, 245, 235));
    }
    box_(0, 146, 240, 4, K(250, 245, 235));
    box_(0, 158, 240, 4, K(250, 245, 235));
}
static void act_cow(int cx, int cy)
{
    int g = sc.mouth * 7 / 255, nod = isin((int)(TM / 14)) * 2 >> 7, sw = isin((int)(TM / 9)) * 5 >> 7, ln = sc.lean / 2;
    uint16_t wh = K(252, 252, 252), blk = K(40, 36, 44), pk = K(255, 176, 190), pk2 = K(232, 130, 150);
    int i;
    for (i = 0; i < 4; i++) {
        int x = (i == 0 ? -38 : i == 1 ? -24 : i == 2 ? 14 : 28);
        AB(x, -34, 11, 34, i & 1 ? K(226, 226, 232) : wh);
        AB(x, -5, 11, 5, blk);
    }
    AL(-44, -74, -54 - sw, -48, 3, wh);
    AE(-54 - sw, -43, 5, 9, blk);
    fill_rbox(cx + X(-50), cy + Y(-86), X(100), Y(56), X(24), wh);
    AE(-28, -66, 13, 9, blk);
    AE(2, -50, 15, 10, blk);
    AE(26, -70, 9, 7, blk);
    AE(-16, -34, 11, 7, pk);
    AE(46 + ln, -66 + nod, 24, 21, wh);
    AT(38 + ln, -84 + nod, 35 + ln, -100 + nod, 46 + ln, -86 + nod, K(240, 220, 170));
    AE(31 + ln, -82 + nod, 12, 6, blk);
    AE(33 + ln, -81 + nod, 7, 3, pk);
    AE(62 + ln, -58 + nod, 15, 11, pk);
    AE(62 + ln, -52 + g + nod, 12, 6, pk2);
    if (g > 2)
        AE(62 + ln, -54 + nod + g / 2, 8, 2, K(110, 30, 50));
    AE(67 + ln, -62 + nod, 2, 2, K(150, 70, 90));
    AE(53 + ln, -73 + nod, 4, 4, blk);
    AE(54 + ln, -74 + nod, 1, 1, wh);
    AE(34, -46, 5, 6, K(255, 205, 60));
}
static void fg_cow(void)
{
    grass_tufts(226, 16, K(70, 160, 60), 6);
    if (sc.mouth > 70) {
        bubble(10, 44, 96, 44, "MOO!", 60, 112);
    }
}

/* ------------------------------------------------------------------- 2 DOG --- */
static void bg_dog(void)
{
    int i;
    vgrad(0, 174, K(140, 205, 255), K(220, 240, 255));
    sun(196, 36, 15, K(255, 238, 130), K(255, 228, 110));
    cloud(((int)(TM / 80) + 20) % 300 - 40, 50, 9, K(255, 255, 255));
    for (i = 0; i < 16; i++) {                       /* the garden fence */
        int h = 70 + (i & 1) * 4;
        box_(i * 16, 150 - h + 20, 13, h, K(210 + (i % 3) * 6, 160, 100));
        fill_tri(i * 16, 150 - h + 20, i * 16 + 13, 150 - h + 20, i * 16 + 6, 150 - h + 12, K(210 + (i % 3) * 6, 160, 100));
    }
    box_(0, 112, 240, 6, K(170, 120, 70));
    box_(0, 140, 240, 6, K(170, 120, 70));
    hat_ground(172, K(110, 195, 85), K(75, 155, 65));
    box_(180, 124, 52, 48, K(200, 90, 60));                   /* the kennel */
    fill_tri(172, 126, 240, 126, 206, 92, K(150, 60, 40));
    fill_ellipse(206, 172, 15, 22, K(50, 30, 30));
    fill_ellipse(206, 176, 11, 12, K(30, 20, 20));
    /* the bone */
    fill_circle(34, 206, 5, K(255, 252, 240));
    fill_circle(34, 214, 5, K(255, 252, 240));
    box_(32, 206, 36, 8, K(255, 252, 240));
    fill_circle(68, 206, 5, K(255, 252, 240));
    fill_circle(68, 214, 5, K(255, 252, 240));
}
static void act_dog(int cx, int cy)
{
    int g = sc.mouth * 14 / 255, wag = isin((int)(TM / 6) + sc.bounce) * 14 >> 7, tilt = sc.lean / 2;
    uint16_t tan = K(214, 150, 80), dk = K(150, 90, 40), lt = K(246, 214, 160), ink = K(30, 24, 30), wh = K(255, 255, 255);
    int ear = isin((int)(TM / 11)) * 2 >> 7;
    AL(26, -16, 44 + wag, -40, 5, tan);
    AE(0, -26, 32, 27, tan);
    AE(0, -18, 18, 17, lt);
    AE(-14, -4, 12, 6, lt);
    AE(14, -4, 12, 6, lt);
    AE(tilt, -66, 32, 29, tan);
    AE(-34 + tilt, -62 + ear, 12, 24, dk);
    AE(34 + tilt, -62 - ear, 12, 24, dk);
    AE(-14 + tilt, -74, 5, 8, K(236, 180, 110));
    AE(tilt, -52, 17, 14 + g / 3, lt);
    AE(-13 + tilt, -73, 8, 9, wh);
    AE(13 + tilt, -73, 8, 9, wh);
    AE(-12 + tilt, -72, 4, 5, ink);
    AE(14 + tilt, -72, 4, 5, ink);
    AE(-11 + tilt, -74, 1, 1, wh);
    AE(15 + tilt, -74, 1, 1, wh);
    if (g > 2) {
        AE(tilt, -45 + g / 2, 12, 3 + g / 2, K(120, 30, 40));
        AE(tilt, -42 + g, 6, 4 + g / 3, K(255, 100, 120));
    } else
        AL(-8 + tilt, -45, 8 + tilt, -45, 1, ink);
    AE(tilt, -57, 7, 5, ink);
    AE(tilt - 2, -59, 2, 1, wh);
    AB(-18, -39, 36, 5, K(220, 40, 50));
    AE(0, -33, 4, 5, K(255, 210, 60));
}
static void fg_dog(void)
{
    grass_tufts(224, 14, K(60, 150, 55), 7);
    if (sc.mouth > 70)
        bubble(20, 38, 108, 44, "WOOF!", 78, 78);
}

/* ------------------------------------------------------------------ 3 FROG --- */
static void bg_frog(void)
{
    int i;
    vgrad(0, 140, K(70, 150, 150), K(170, 215, 170));
    sun(190, 42, 17, K(255, 245, 190), K(255, 235, 150));
    for (i = 0; i < 5; i++) {                         /* reeds with cattails */
        int x = 12 + i * 20 + (i & 1) * 6, sw = isin((int)(TM / 16) + i * 50) * 3 >> 7;
        fat_line(x, 160, x + sw, 70 + i * 6, 2, K(60, 120, 60));
        fill_ellipse(x + sw, 66 + i * 6, 4, 12, K(130, 80, 50));
    }
    vgrad(140, 240, K(60, 150, 150), K(24, 90, 110));
    for (i = 0; i < 4; i++) {                         /* ripples */
        int p = (int)((TM / 30 + i * 40) % 160);
        ring_(120, 188, 20 + p, 1, K(150, 215, 215));
    }
    fill_ellipse(30, 200, 26, 7, K(70, 170, 90));
    fill_ellipse(214, 214, 22, 6, K(60, 160, 80));
    fill_circle(208, 208, 4, K(255, 150, 190));
    fill_ellipse(120, 182, 62, 14, K(60, 150, 70));
    fill_tri(120, 182, 178, 172, 178, 186, K(40, 120, 120));
    fill_ellipse(120, 180, 54, 11, K(80, 180, 90));
}
static void act_frog(int cx, int cy)
{
    int g = sc.mouth * 9 / 255, sac = sc.mouth * 14 / 255 + (sc.lvl > 20 ? 2 : 0);
    uint16_t gr = K(110, 210, 70), dk = K(60, 150, 50), lt = K(200, 240, 130), ink = K(30, 40, 30), wh = K(255, 255, 255);
    AE(-38, -14, 18, 12, dk);
    AE(38, -14, 18, 12, dk);
    AE(-30, -20, 16, 20, gr);
    AE(30, -20, 16, 20, gr);
    AE(0, -28, 38, 28, gr);
    AE(0, -18, 24, 17, lt);
    AE(-14, -38, 8, 5, dk);
    AE(16, -42, 7, 4, dk);
    AE(0, -20 + sac / 3, 18 + sac / 2, 7 + sac, K(255, 170, 140));
    AE(-24, -22, 8, 10, gr);
    AE(24, -22, 8, 10, gr);
    AE(-16, -50, 14, 14, gr);
    AE(16, -50, 14, 14, gr);
    AE(-16, -52, 11, 11, wh);
    AE(16, -52, 11, 11, wh);
    AE(-16 + (sc.lean >> 3), -52, 5, 7, ink);
    AE(16 + (sc.lean >> 3), -52, 5, 7, ink);
    AL(-26, -30, 26, -30, 1, ink);
    if (g > 2)
        AE(0, -30 + g / 2, 22, g / 2 + 1, K(140, 40, 60));
    AE(-9, -36, 2, 1, ink);
    AE(9, -36, 2, 1, ink);
}
static void fg_frog(void)
{
    int i;
    for (i = 0; i < 3; i++) {                         /* a firefly */
        int x = 40 + i * 70 + (isin((int)(TM / 20) + i * 80) >> 2), y = 70 + i * 30 + (icos((int)(TM / 17) + i * 60) >> 2);
        fill_circle(x, y, 2, K(255, 255, 140));
        fill_circle(x, y, 4, blend(K(255, 255, 140), K(70, 150, 150), 13));
    }
    if (sc.mouth > 70)
        bubble(130, 36, 100, 44, "RIBBIT", 150, 92);
}
