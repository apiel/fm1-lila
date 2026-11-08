/* SPDX-License-Identifier: GPL-3.0-only */
/* Five more instruments, each with its own picture, all lit by the keys (pk[], from scenes_piano.c): XYLO (a bear with mallets
 * and a rainbow xylophone), HARP (a fairy and a golden harp whose strings glow and shiver), UKULELE (a crab strumming on a
 * beach at sunset), FLUTE (a snake rising from its basket in the desert night), SITAR (a singing sitar in front of a turning
 * mandala). Included by scenes.c after scenes_piano.c. */
#pragma once

#define INST_WORLD0 15
static int iworld(void) { return iclamp(sc.world - INST_WORLD0, 0, 4); }
static int key_x(int k) { return 10 + k * 87 / 10; }                        /* a key's column: 10 .. 236 */

/* ---------------------------------------------------------------- 15 XYLO --- */
static void bg_xylo(void)
{
    int i;
    vgrad(0, 240, K(150, 214, 255), K(255, 236, 250));
    for (i = 0; i < 6; i++) {                                                /* a rainbow */
        static const uint8_t RC[6][3] = {{255, 70, 80}, {255, 160, 60}, {255, 230, 70}, {90, 220, 100}, {70, 170, 255}, {170, 100, 255}};
        ring_(120, 150, 118 - i * 7, 8, K(RC[i][0], RC[i][1], RC[i][2]));
    }
    box_(0, 150, 240, 100, K(190, 236, 190));
    cloud(((int)(TM / 70) + 40) % 300 - 40, 34, 9, K(255, 255, 255));
    cloud(((int)(TM / 100) + 160) % 300 - 40, 66, 7, K(252, 252, 255));
    sun(210, 36, 14, K(255, 238, 130), K(255, 226, 100));
}
static void act_xylo(int cx, int cy)                                         /* a bear: the mallets go where the last note was */
{
    int g = sc.mouth * 9 / 255, par = (sc.score / 10) & 1, tx = 20 + sc.pitch * 200 / 255 - cx, k;
    uint16_t fur = K(196, 130, 76), lt = K(250, 214, 160), ink = K(40, 28, 28), wd = K(214, 160, 90);
    AE(-22, -4, 14, 7, fur);
    AE(22, -4, 14, 7, fur);
    AE(0, -34, 28, 30, fur);
    AE(0, -28, 17, 20, lt);
    for (k = 0; k < 2; k++) {                                                /* two arms: one hits where the last note was, one waits */
        int side = k ? 1 : -1, active = (k == par), hx = active ? iclamp(tx, -100, 100) : side * 38, hy = active ? (sc.hit > 110 ? 54 : 34) : -26;
        AL(side * 22, -44, hx, hy - 16, 5, fur);
        AL(hx, hy - 16, hx, hy, 1, wd);
        AE(hx, hy, 6, 6, K(255, 120, 150));
    }
    AE(-18, -72, 9, 9, fur);
    AE(18, -72, 9, 9, fur);
    AE(-18, -72, 5, 5, lt);
    AE(18, -72, 5, 5, lt);
    AE(0, -62, 26, 23, fur);
    AE(0, -54, 12, 9 + g / 3, lt);
    AE(0, -57, 5, 3, ink);
    AE(-9, -66, 3, 4, ink);
    AE(9, -66, 3, 4, ink);
    if (g > 2)
        AE(0, -50 + g / 3, 4, 1 + g / 2, K(150, 40, 60));
}
static void fg_xylo(void)
{
    int k;
    box_(6, 168, 228, 4, K(150, 100, 60));                                   /* the rails */
    box_(6, 204, 228, 4, K(150, 100, 60));
    for (k = 0; k < NKEYS_L; k++) {
        int x = key_x(k), len = 74 - k * 17 / 10, lit = pk[k], dy = lit >> 6;
        uint16_t c = piano_col(k, 30 + lit / 2);
        fill_rbox(x - 3, 186 - len / 2 + dy, 7, len, 3, lit ? blend(c, K(255, 255, 255), lit * 10 / 255) : c);
        box_(x - 2, 186 - len / 2 + 2 + dy, 1, len - 4, blend(c, K(255, 255, 255), 9));
        if (lit > 150 && sc.metal > 20)
            sparkle(x, 186 - len / 2 - 4, 2 + sc.metal / 40, K(255, 255, 255));
    }
}

/* ---------------------------------------------------------------- 16 HARP --- */
static void bg_harp(void)
{
    int i;
    vgrad(0, 240, K(20, 14, 60), K(120, 70, 150));
    stars_sky(30, 130, 5, K(255, 244, 200));
    fill_circle(44, 52, 20, K(255, 246, 210));
    fill_circle(36, 46, 20, blend(K(20, 14, 60), K(60, 40, 110), 8));        /* a crescent: a dark disc bites the moon */
    cloud(((int)(TM / 90) + 80) % 300 - 40, 150, 10, K(150, 110, 190));
    for (i = 0; i < 3; i++)
        cloud(((int)(TM / (120 + i * 30)) + i * 100) % 300 - 40, 190 + i * 12, 12, K(110, 80, 160));
}
static void act_harp(int cx, int cy)                                         /* a fairy */
{
    int g = sc.mouth * 8 / 255, fl = isin((int)(TM / 6)) * 14 >> 7, ln = sc.lean / 3;
    uint16_t skin = K(255, 218, 190), hair = K(255, 214, 90), dress = K(255, 160, 220), wing = blend(K(120, 70, 150), K(200, 240, 255), 9);
    AE(-30, -62 - fl, 26, 12 + fl / 3, wing);
    AE(30, -62 - fl, 26, 12 + fl / 3, wing);
    AE(-22, -44 + fl / 2, 18, 8, blend(wing, K(255, 255, 255), 5));
    AE(22, -44 + fl / 2, 18, 8, blend(wing, K(255, 255, 255), 5));
    AT(-8, -62, 8, -62, 24, -6, dress);
    AT(-8, -62, -24, -6, 24, -6, dress);
    AL(-6, -56, -28, -74, 3, skin);
    AL(6, -56, 30 + ln, -78, 3, skin);
    AE(ln, -80, 15, 15, hair);
    AE(ln, -76, 12, 13, skin);
    AE(ln - 5, -76, 2, 3, K(40, 30, 50));
    AE(ln + 5, -76, 2, 3, K(40, 30, 50));
    AE(ln - 8, -71, 3, 2, K(255, 150, 160));
    AE(ln + 8, -71, 3, 2, K(255, 150, 160));
    if (g > 1)
        AE(ln, -69, 3, 1 + g / 3, K(150, 40, 60));
    AL(30 + ln, -78, 40 + ln, -96, 1, K(255, 230, 140));                      /* the wand */
    sparkle(40 + ln, -98, 4 + sc.hit / 60, K(255, 250, 190));
}
static void fg_harp(void)
{
    int k;
    uint16_t gold = K(236, 190, 70), dk = K(160, 110, 40);
    for (k = 0; k < NKEYS_L; k++) {                                          /* strings: C red, F blue, like a real harp */
        int x = 10 + k * 87 / 10, top = 24 + k * 4, pc = (KEY0_NOTE + k) % 12, lit = pk[k], sh = (isin((int)(TM / 3) + k * 20) * lit) >> 10;
        uint16_t c = pc == 0 ? K(255, 80, 90) : pc == 5 ? K(90, 150, 255) : K(250, 245, 230);
        if (lit)
            c = blend(c, K(255, 255, 190), 4 + lit * 12 / 255);
        fat_line(x, top, x + sh, (top + 206) / 2, lit > 100 ? 1 : 0, c);
        fat_line(x + sh, (top + 206) / 2, x, 206, lit > 100 ? 1 : 0, c);
        if (lit > 120)
            sparkle(x + sh, (top + 206) / 2, 2, K(255, 255, 220));
    }
    fat_line(4, 20, 236, 130, 5, gold);                                       /* the neck, the column, the sound board */
    fat_line(236, 130, 236, 216, 6, gold);
    fill_rbox(4, 206, 232, 20, 6, dk);
    box_(6, 208, 228, 4, gold);
    fat_line(4, 20, 4, 24, 4, gold);
}

/* ------------------------------------------------------------- 17 UKULELE --- */
static void bg_uke(void)
{
    int i;
    vgrad(0, 150, K(255, 130, 90), K(255, 214, 140));
    fill_circle(120, 142, 40, K(255, 244, 170));
    box_(0, 142, 240, 100, K(40, 150, 190));                                  /* the sea, and the sun's path */
    for (i = 0; i < 9; i++) {
        int w = 50 - i * 5 + (isin((int)(TM / 14) + i * 50) >> 4);
        box_(120 - w / 2, 146 + i * 6, w, 2, K(255, 226, 150));
        box_(((int)(TM / 40) + i * 31) % 260 - 10, 148 + i * 5, 24, 1, K(120, 210, 235));
    }
    box_(0, 176, 240, 70, K(244, 214, 150));                                  /* the sand */
    for (i = 0; i < 8; i++)                                                   /* a palm */
        fat_line(26, 190, 34 + (i - 4) * 3, 60 - (i & 1) * 4, 3, K(130, 90, 56));
    for (i = 0; i < 6; i++) {
        int a = i * 43 + 10 + (isin((int)(TM / 30) + i * 30) >> 5);
        fat_line(34, 56, 34 + icos(a) * 38 / 127, 56 + isin(a) * 22 / 127, 3, K(60, 160, 70));
    }
}
static void act_uke(int cx, int cy)                                           /* a crab with a ukulele */
{
    int g = sc.mouth * 6 / 255, par = (sc.score / 10) & 1, swing = sc.hit > 80 ? (par ? 20 : -20) * sc.hit / 255 : 0, ln = sc.lean / 3;
    uint16_t red = K(240, 80, 70), dk = K(180, 50, 50), wd = K(200, 120, 56), wd2 = K(150, 84, 40), wh = K(255, 255, 255), ink = K(30, 24, 30);
    int i;
    for (i = 0; i < 3; i++) {                                                 /* legs */
        AL(-20, -12, -40 - i * 4, -2 - i * 2, 2, dk);
        AL(20, -12, 40 + i * 4, -2 - i * 2, 2, dk);
    }
    AE(0, -22, 32, 20, red);
    AL(-8, -36, -8, -50, 2, dk);                                              /* the eyes on stalks */
    AL(8, -36, 8, -50, 2, dk);
    AE(-8 + ln / 2, -52, 6, 7, wh);
    AE(8 + ln / 2, -52, 6, 7, wh);
    AE(-8 + ln, -52, 3, 4, ink);
    AE(8 + ln, -52, 3, 4, ink);
    if (g > 1)
        AE(0, -20, 6, 1 + g / 2, K(110, 20, 40));
    else {
        AL(-5, -22, 0, -19, 1, ink);
        AL(0, -19, 5, -22, 1, ink);
    }
    AL(-30, -30, -48, -52, 4, red);                                            /* the left claw holds the neck */
    AE(-50, -56, 9, 7, red);
    AL(-6, -56 + 0, -40, -64, 3, wd2);                                         /* the neck, pointing up-left */
    AE(-44, -66, 5, 8, wd2);
    AE(2, -34, 22, 18, wd);                                                    /* the body of the ukulele: two round parts */
    AE(0, -52, 15, 14, wd);
    AE(2, -40, 7, 7, K(40, 24, 20));
    for (i = -2; i <= 2; i++)
        AL(-6 + i, -64, 4 + i * 2, -22 + swing / 6, 0, wh);
    AL(26, -34, 8, -38 + swing, 4, red);                                       /* the right claw strums */
    AE(6, -38 + swing, 9, 7, red);
}
static void fg_uke(void)
{
    int i;
    for (i = 0; i < 5; i++)                                                    /* shells */
        fill_ellipse(24 + i * 52, 214 + (i & 1) * 8, 6, 4, i & 1 ? K(255, 180, 190) : K(255, 238, 220));
    fill_circle(204, 216, 6, K(255, 140, 90));
}

/* --------------------------------------------------------------- 18 FLUTE --- */
static void bg_flute(void)
{
    vgrad(0, 200, K(30, 20, 80), K(255, 140, 90));
    stars_sky(26, 100, 6, K(255, 244, 210));
    fill_circle(190, 62, 18, K(255, 240, 200));
    fill_tri(10, 190, 80, 190, 45, 120, K(206, 140, 90));                       /* pyramids */
    fill_tri(60, 190, 190, 190, 125, 98, K(190, 120, 80));
    fill_tri(125, 98, 190, 190, 150, 190, K(150, 90, 66));
    fill_tri(170, 190, 240, 190, 214, 140, K(206, 140, 90));
    box_(0, 190, 240, 60, K(236, 190, 120));
    fill_ellipse(60, 214, 100, 14, K(244, 206, 140));
}
static void act_flute(int cx, int cy)                                          /* a snake rising from a basket, dancing to the flute */
{
    int i, h = 40 + sc.lvl / 8 + sc.mouth / 12, g = sc.mouth * 5 / 255, ln = sc.lean / 2;
    uint16_t sk = K(90, 190, 90), sk2 = K(250, 230, 120), ink = K(30, 24, 30);
    for (i = 14; i >= 0; i--) {
        int t = i * h / 14, x = isin((int)(TM / 11) + i * 14) * (6 + i * 2) / 127 + ln * i / 14, r = 11 - i / 2 + (i > 12 ? -1 : 0);
        AE(x, -16 - t, imax(4, r), imax(4, r), (i & 1) ? sk : K(70, 160, 80));
        AE(x, -12 - t, imax(2, r / 2), imax(2, r / 2), sk2);
    }
    {
        int hx = isin((int)(TM / 11)) * 6 / 127 + ln, hy = -16 - h - 8;
        AE(hx, hy, 15, 12, sk);
        AE(hx - 6, hy - 3, 4, 5, K(255, 255, 255));
        AE(hx + 6, hy - 3, 4, 5, K(255, 255, 255));
        AE(hx - 6, hy - 3, 2, 3, ink);
        AE(hx + 6, hy - 3, 2, 3, ink);
        if (g > 1)
            AE(hx, hy + 6, 4, 1 + g / 2, K(150, 30, 50));
        if ((int)(TM / 200u) & 1)                                              /* the tongue flicks */
            AL(hx, hy + 8, hx + 2, hy + 18, 0, K(255, 60, 90));
    }
    AE(0, -6, 34, 14, K(200, 140, 70));                                          /* the basket */
    for (i = -3; i <= 3; i++)
        AB(i * 9 - 3, -10, 3, 12, K(150, 96, 44));
    AB(-34, -14, 68, 4, K(230, 170, 90));
}
static void fg_flute(void)
{
    int i, held = 0;
    for (i = 0; i < NKEYS_L; i++)
        if (pk[i] > 120)
            held++;
    fat_line(10, 222, 112, 188, 5, K(200, 210, 230));                           /* the flute, with six holes that light up */
    fat_line(10, 220, 112, 186, 2, K(240, 245, 255));
    for (i = 0; i < 6; i++) {
        int x = 34 + i * 14, y = 214 - i * 5 + (i >= 3 ? 0 : 0);
        fill_circle(x, y, 3, ((held * 2 + 1) >> 1) > i || pk[(i * 4) % NKEYS_L] > 100 ? K(255, 230, 90) : K(60, 70, 100));
    }
    if (sc.hit > 80)
        for (i = 0; i < 3; i++)                                                 /* notes come out of the end */
            text_out(120 + i * 18 + sc.hit / 8, 176 - i * 12 - sc.hit / 10, &FONT_B, i == 1 ? "~" : "o", K(255, 255, 200), K(80, 40, 60));
}

/* --------------------------------------------------------------- 19 SITAR --- */
static void bg_sitar(void)
{
    int i, a0 = (int)(TM / 70);
    vgrad(0, 240, K(120, 20, 50), K(40, 10, 60));
    for (i = 0; i < 2; i++) {                                                    /* a palace's pillars */
        box_(8 + i * 216, 24, 16, 176, K(190, 60, 70));
        box_(4 + i * 216, 20, 24, 8, K(255, 200, 100));
        box_(4 + i * 216, 192, 24, 8, K(255, 200, 100));
    }
    fill_circle(120, 96, 66, K(70, 16, 70));                                    /* a mandala that turns */
    for (i = 0; i < 12; i++) {
        int a = a0 + i * 21;
        fill_tri(120 + icos(a) * 20 / 127, 96 + isin(a) * 20 / 127, 120 + icos(a - 6) * 62 / 127, 96 + isin(a - 6) * 62 / 127,
                 120 + icos(a + 6) * 62 / 127, 96 + isin(a + 6) * 62 / 127, piano_col(i * 2, 40 + sc.lvl / 2));
    }
    ring_(120, 96, 64, 3, K(255, 210, 110));
    ring_(120, 96, 30, 2, K(255, 210, 110));
    fill_circle(120, 96, 10, K(255, 220, 120));
    box_(0, 196, 240, 50, K(150, 40, 50));
    for (i = 0; i < 8; i++)
        box_(i * 32, 200, 16, 4, K(255, 200, 100));
}
static void act_sitar(int cx, int cy)                                           /* a sitar that sings: a gourd with a face and a long neck */
{
    int g = sc.mouth * 8 / 255, ln = sc.lean / 3, i, v = sc.lvl / 40;
    uint16_t gd = K(214, 120, 50), gd2 = K(160, 80, 36), wd = K(120, 70, 36), gold = K(255, 214, 110), wh = K(255, 255, 255), ink = K(40, 24, 24);
    AL(0, -50, 8 + ln, -150, 7, wd);                                                 /* the neck */
    for (i = 0; i < 9; i++)                                                           /* the frets */
        AB(-4 + ln * i / 9 + i * 8 / 9, -62 - i * 9, 12, 2, gold);
    AE(10 + ln, -156, 11, 12, gd);                                                   /* the little gourd at the top, and the pegs */
    AL(8 + ln, -168, 8 + ln, -176, 2, wd);
    AE(8 + ln, -178, 3, 3, gold);
    for (i = -2; i <= 2; i++) {                                                        /* the strings shiver with the sound */
        int sh = (isin((int)(TM / 2) + i * 40) * v) >> 8;
        AL(i * 2 + sh, -34, 9 + ln + i * 2 + sh, -146, 0, wh);
    }
    AE(0, -34, 40, 36, gd2);                                                          /* the gourd */
    AE(0, -36, 36, 32, gd);
    AE(-12, -50, 11, 7, blend(gd, wh, 6));
    AE(-12, -40, 7, 8, wh);
    AE(12, -40, 7, 8, wh);
    AE(-12, -40, 3, 4, ink);
    AE(12, -40, 3, 4, ink);
    AE(-22, -26, 6, 4, K(255, 150, 130));
    AE(22, -26, 6, 4, K(255, 150, 130));
    if (g > 1)
        AE(0, -22, 7, 1 + g / 2, K(130, 30, 40));
    else {
        AL(-6, -24, 0, -20, 1, ink);
        AL(0, -20, 6, -24, 1, ink);
    }
    AB(-22, -6, 44, 4, gold);
}
static void fg_sitar(void)
{
    int i;
    for (i = 0; i < 5; i++) {                                                         /* marigold petals float up on each note */
        int y = 230 - (int)((TM / 18u + (unsigned)i * 47u) % 200u), x = 20 + i * 50 + (isin((int)(TM / 20) + i * 60) >> 4);
        if (sc.lvl > 30)
            fill_ellipse(x, y, 4, 3, i & 1 ? K(255, 160, 40) : K(255, 200, 60));
    }
}
