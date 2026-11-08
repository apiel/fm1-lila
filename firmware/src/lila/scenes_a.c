/* SPDX-License-Identifier: GPL-3.0-only */
/* Worlds 0..3: the bird in the sun, the cow on the farm, the dog in the garden, the frog on the pond. */
#pragma once

static int strlen_(const char *s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}
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
    int g = sc.mouth * 7 / 255 + (sc.mouth < 20 && isin((int)(TM / 7)) > 40 ? 2 : 0), nod = isin((int)(TM / 14)) * 2 >> 7,
        sw = isin((int)(TM / (sc.hit > 100 ? 4 : 9))) * (sc.hit > 100 ? 9 : 5) >> 7, ln = sc.lean / 2, blink = ((int)(TM / 120u) % 26) == 0;
    int bx = 34 + (isin((int)(TM / 5)) * sc.bellf >> 9), by = -46 + (icos((int)(TM / 5)) * sc.bellf >> 11);
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
    if (blink)
        AB(49 + ln, -73 + nod, 9, 2, blk);
    else {
        AE(53 + ln, -73 + nod, 4, 4, blk);
        AE(54 + ln, -74 + nod, 1, 1, wh);
    }
    AL(30, -52, 36, -50, 1, K(120, 60, 40));
    AE(bx, by, 5, 6, K(255, 205, 60));
    AE(bx, by + 4, 2, 2, K(150, 110, 20));
    if (sc.bellf > 90)
        AR(bx, by, 12, 2, K(255, 240, 120));
}
static void fg_cow(void)
{
    grass_tufts(226, 16, K(70, 160, 60), 6);
    if (sc.mouth > 70) {
        bubble(10, 44, sc.mouth > 170 ? 116 : 96, 44, sc.mouth > 170 ? "MOOO!" : "MOO!", 60, 112);
    }
}

/* ------------------------------------------------------------------- 2 DOG --- */
/* a dog that howls at the moon, and does a police siren, an ambulance or a fire alarm: the whole world flashes red and blue */
static void bg_dog(void)
{
    int i, howl = sc.ctype == CT_HOWL ? sc.mouth : 0, mr = 22 + howl / 28;
    vgrad(0, 174, K(36, 30, 96), K(236, 140, 130));
    for (i = 0; i < 26; i++)
        if (isin((int)(TM / 11) + i * 31) > -60)
            sparkle((i * 89 + 7) % 240, (i * 43) % 90, 1 + (isin((int)(TM / 8) + i * 17) > 100), K(255, 250, 220));
    fill_circle(62, 56, mr + 10 + howl / 40, blend(K(36, 30, 96), K(255, 240, 190), 3 + howl / 70));     /* the moon and its glow */
    fill_circle(62, 56, mr, K(255, 248, 215));
    fill_circle(54, 50, mr / 4, K(236, 226, 190));
    fill_circle(72, 64, mr / 5, K(236, 226, 190));
    hills(150, 12, 190, 40, 240, K(70, 50, 100));
    for (i = 0; i < 16; i++) {                       /* the garden fence */
        int h = 52 + (i & 1) * 4;
        box_(i * 16, 176 - h, 13, h, K(120, 86, 84));
        fill_tri(i * 16, 176 - h, i * 16 + 13, 176 - h, i * 16 + 6, 176 - h - 8, K(120, 86, 84));
    }
    box_(0, 138, 240, 5, K(96, 66, 70));
    hat_ground(176, K(70, 130, 90), K(40, 90, 70));
    box_(188, 134, 50, 44, K(170, 70, 56));          /* the kennel */
    fill_tri(180, 136, 244, 136, 212, 106, K(120, 50, 44));
    fill_ellipse(212, 178, 14, 20, K(30, 18, 26));
}
static void act_dog(int cx, int cy)
{
    int howl = sc.ctype == CT_HOWL, tilt = sc.mouth * (howl ? 62 : 14) / 255, open = sc.mouth * 40 / 255, wag = isin((int)(TM / 6) + sc.bounce) * 12 >> 7;
    int ang = -6 + tilt, ln = sc.lean / 2, hx = 12 + ln, hy = -92 - tilt / 5, a256 = ang * 256 / 360, sx, sy, jx, jy;
    uint16_t tan = K(150, 156, 172), dk = K(96, 102, 122), lt = K(226, 230, 238), ink = K(30, 24, 30), wh = K(255, 255, 255);
    uint16_t pk = K(255, 120, 140), red = K(150, 156, 172);
    int len = 28, dirx = icos(a256), diry = isin(a256), jang = a256 - open * 256 / 360;
    AL(-34, -16, -54 + wag, -34 - wag / 2, 5, tan);                 /* the tail */
    AE(-18, -26, 24, 24, dk);                                       /* the haunch */
    AE(-22, -3, 18, 6, tan);
    AE(0, -48, 22, 38, tan);                                        /* the body, sitting up */
    AE(10, -42, 12, 26, lt);
    AB(6, -34, 11, 34, tan);                                        /* the front legs */
    AE(14, -3, 11, 5, lt);
    AL(6, -74, hx - 2, hy + 8, 11, tan);                            /* the neck */
    AB(-6, -78, 24, 6, red);                                        /* the collar */
    AT(hx - 16, hy - 8, hx - 4, hy - 12, hx - 14, hy - 34, dk);     /* the pointed ear, behind the head */
    AE(hx, hy, 18, 17, tan);
    sx = hx + dirx * len / 127;
    sy = hy + 4 - diry * len / 127;
    jx = hx + icos(jang) * (len - 4) / 127;
    jy = hy + 6 - isin(jang) * (len - 4) / 127;
    if (open > 3) {
        AL(hx + 2, hy + 8, jx, jy + 4, 5, pk);                      /* the jaw, open */
        AL(hx + 2, hy + 6, sx - 4, sy + 2, 3, K(110, 30, 44));      /* the inside of the mouth */
    }
    AL(hx + 2, hy + 3, sx, sy, 9, lt);                              /* the snout */
    AE(sx - hx + hx, sy - 1, 5, 4, ink);                            /* the nose */
    if (tilt > 28) {                                                /* howling: the eye is shut, a happy curve */
        AL(hx + 2, hy - 4, hx + 12, hy - 5, 1, ink);
    } else {
        AE(hx + 6, hy - 5, 6, 6, wh);
        AE(hx + 8, hy - 5, 3, 4, ink);
    }
    AT(hx - 6, hy - 14, hx + 2, hy - 16, hx - 8, hy - 32, dk);       /* the other ear, up */
    if (sc.ctype >= CT_WAIL && sc.ctype <= CT_ALARM) {              /* a siren light on its head */
        int ph = (int)(TM / 160) & 1;
        AB(hx - 7, hy - 26, 14, 8, ph ? K(255, 60, 60) : K(70, 120, 255));
        AE(hx, hy - 27, 8, 4, ph ? K(255, 150, 150) : K(160, 190, 255));
    }
}
static void fg_dog(void)
{
    const char *word = 0;
    int w;
    grass_tufts(224, 14, K(50, 110, 70), 7);
    if (sc.mouth > 70) {
        word = sc.ctype == CT_HOWL ? "AWOOO!" : sc.ctype == CT_WAIL ? "WEE-OOO" : sc.ctype == CT_HILO ? "NEE-NAW" : sc.ctype == CT_ALARM ? "BRRING!" : "AWOO!";
        w = (int)strlen_(word) * 16 + 20;
        bubble(124 - w / 2 + 30, 30, w, 44, word, 150, 86);
    }
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
    int g = sc.mouth * 9 / 255, sac = sc.mouth * 14 / 255 + (sc.lvl > 20 ? 2 : 0), pul = sc.am > 8 && sc.mouth > 20 ? ((isin((int)(TM / 3)) + 127) * sc.am * 8 >> 15) : 0;
    sac += pul;
    cy += pul / 4;
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
    {   /* a fly, and a tongue that catches it */
        int fx = 170 + (isin((int)(TM / 11)) * 34 >> 7), fy = 92 + (icos((int)(TM / 8)) * 20 >> 7), mx = 120, my = 176 - 32 * sc.sz / 100 - sc.jump;
        if (sc.hit > 170)
            fat_line(mx, my, mx + (fx - mx) * (sc.hit - 150) / 105, my + (fy - my) * (sc.hit - 150) / 105, 3, K(255, 110, 130));
        fill_ellipse(fx, fy, 3, 2, K(20, 20, 20));
        fat_line(fx, fy - 1, fx - 3, fy - 5 - (int)((TM / 40) & 1) * 2, 1, K(200, 230, 255));
        fat_line(fx, fy - 1, fx + 3, fy - 5 - (int)((TM / 40) & 1) * 2, 1, K(200, 230, 255));
    }
    if (sc.mouth > 70)
        bubble(124, 36, sc.am > 60 ? 112 : 100, 44, sc.am > 60 ? "RRIBBIT" : "RIBBIT", 150, 92);
}
