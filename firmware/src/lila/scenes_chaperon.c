/* SPDX-License-Identifier: GPL-3.0-only */
/* Le Petit Chaperon rouge (story 4): the forest, the grandmother's house and her bedroom, the girl in her red hood and the wolf.
 * The picture follows the part being told (twelve parts). Included by scenes.c after scenes_fables.c. */
#pragma once

#define CH_STORY 4

/* the wolf, feet (or, in bed, the middle of the pillow) at (cx, cy): bed = in grandmother's nightcap; open = how wide the mouth is */
static void wolf_(int cx, int cy, int bed, int open, int teeth)
{
    uint16_t gr = K(120, 124, 140), lt = K(206, 210, 222), dk = K(70, 72, 90), ink = K(30, 24, 30), wh = K(255, 255, 255);
    int o = open * 12 / 255 + (teeth ? 10 : 0);
    if (!bed) {
        AL(-22, -16, -48, -40 + (isin((int)(TM / 9)) >> 5), 4, gr);
        AE(-10, -3, 10, 5, gr);
        AE(10, -3, 10, 5, gr);
        AE(0, -34, 24, 30, gr);
        AE(0, -28, 14, 22, lt);
    }
    AT(-22, -62, -8, -68, -20, -96, gr);                                   /* the pointed ears */
    AT(22, -62, 8, -68, 20, -96, gr);
    AT(-18, -68, -10, -70, -19, -88, dk);
    AT(18, -68, 10, -70, 19, -88, dk);
    AE(0, -62, 24, 22, gr);
    AE(0, -52, 14, 11, lt);
    AE(-10, -66, 5, 5, K(255, 230, 90));
    AE(10, -66, 5, 5, K(255, 230, 90));
    AE(-10, -66, 1, 4, ink);
    AE(10, -66, 1, 4, ink);
    AE(0, -57, 5, 4, ink);
    if (o > 2)
        AE(0, -46 + o / 4, 9, 2 + o / 2, K(150, 30, 50));
    if (teeth) {
        AT(-6, -48, -2, -48, -4, -41, wh);
        AT(2, -48, 6, -48, 4, -41, wh);
    }
    if (bed) {                                                             /* a nightcap with a bobble */
        AT(-20, -80, 20, -80, 6, -112, K(255, 250, 250));
        AE(8, -112, 5, 5, K(255, 120, 150));
        AB(-22, -82, 44, 5, K(255, 120, 150));
    }
}

/* the girl in her red hood and cape, with a basket */
static void chaperon_(int cx, int cy)
{
    uint16_t red = K(230, 40, 50), dk = K(170, 20, 40);
    girl(cx, cy, 0);
    AT(-16, -62, 16, -62, 26, -22, red);
    AT(-16, -62, -26, -22, 26, -22, red);
    AB(-26, -24, 52, 3, dk);
    AE(0, -80, 19, 14, red);
    AE(0, -86, 17, 8, dk);
    AE(0, -74, 17, 8, K(255, 214, 180));
    AE(-6, -74, 2, 3, K(40, 30, 40));
    AE(6, -74, 2, 3, K(40, 30, 40));
    AE(26, -42, 9, 7, K(214, 160, 90));                                    /* the basket */
    AB(18, -50, 16, 2, K(150, 100, 50));
}

static void house_(int x, int y)                                             /* the grandmother's cottage, its door at x + 20 */
{
    box_(x, y, 90, 72, K(236, 214, 170));
    fill_tri(x - 10, y + 2, x + 100, y + 2, x + 45, y - 44, K(176, 70, 56));
    box_(x + 64, y - 36, 12, 24, K(120, 70, 50));
    box_(x + 14, y + 22, 26, 50, K(120, 70, 50));
    box_(x + 54, y + 20, 22, 20, K(150, 210, 255));
    box_(x + 64, y + 20, 2, 20, K(120, 70, 50));
}

static void bg_chap(void)
{
    int part = iclamp(spart(CH_STORY), 0, 11);
    if (part >= 6 && part <= 9) {                                            /* her bedroom, and the bed */
        room(K(255, 230, 200), K(240, 196, 170));
        box_(130, 138, 104, 46, K(150, 100, 60));
        box_(128, 128, 8, 62, K(120, 76, 44));
        box_(226, 128, 8, 62, K(120, 76, 44));
        box_(150, 124, 56, 26, K(250, 250, 255));
        return;
    }
    forest();
    if (part == 4 || part == 5 || part == 10)
        house_(140, 110);
}
static void act_chap(int cx0, int cy0)
{
    int part = iclamp(spart(CH_STORY), 0, 11), cx = cx0, cy = cy0, sw = g_sw, sh = g_sh;
    if (part <= 1 || part == 11)
        chaperon_(cx, cy);
    else if (part == 2 || part == 3) {
        chaperon_(cx - X(10), cy);
        wolf_(cx + X(110), cy, 0, part == 3 ? sc.mouth : 0, 0);
    } else if (part == 4 || part == 5)
        wolf_(cx + X(110), cy - Y(0), 0, sc.mouth, 0);
    else if (part == 6)
        wolf_(cx + X(116), cy - Y(24), 1, 0, 0);
    else if (part >= 7 && part <= 9) {
        chaperon_(cx - X(10), cy);
        wolf_(cx + X(116), cy - Y(24), 1, part == 9 ? 255 : part == 8 ? 40 : sc.mouth, part == 9);
    } else if (part == 10)
        wolf_(cx + X(110) + (int)((TM / 6u) % 140u), cy, 0, 200, 1);
    g_sw = sw;
    g_sh = sh;
}
static void fg_chap(void)
{
    int part = iclamp(spart(CH_STORY), 0, 11);
    if (part >= 6 && part <= 9) {                                            /* the quilt covers the wolf up to its chin */
        box_(130, 150, 104, 36, K(220, 80, 90));
        box_(130, 150, 104, 4, K(255, 150, 160));
    }
    if (part == 5)                                                           /* toc, toc, toc! */
        text_out(70, 100, &FONT_B, "TOC TOC!", K(255, 255, 255), K(80, 40, 20));
    if (part == 10)
        text_out(60, 60, &FONT_B, "OUF!", K(255, 255, 255), K(80, 40, 20));
}
