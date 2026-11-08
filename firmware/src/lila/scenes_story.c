/* SPDX-License-Identifier: GPL-3.0-only */
/* The two story worlds. Goldilocks has her own forest, house, kitchen, chairs and bedroom, and the
 * three bears. The picture is the part being told (story_part), or the last one told. Included by scenes.c after SCENES. */
#pragma once

static int story_last[5];                               /* per story: the part on screen when none is being told */
static int spart(int s) { return story_part >= 0 ? (story_last[s] = story_part) : story_last[s]; }

/* a little girl, feet at (cx, cy): Goldilocks (golden curls, a blue dress; the other look is unused now) */
static void girl(int cx, int cy, int golden)
{
    int g = sc.mouth * 8 / 255, wave = sc.mouth > 60 ? isin((int)(TM / 4)) * 10 >> 7 : 0;
    uint16_t skin = K(255, 214, 180), hair = golden ? K(255, 210, 70) : K(110, 64, 34), dress = golden ? K(80, 140, 240) : K(250, 110, 170);
    uint16_t ink = K(40, 30, 40), shoe = golden ? K(200, 60, 60) : K(130, 60, 200), wh = K(255, 255, 255);
    AB(-9, -24, 6, 24, skin);
    AB(3, -24, 6, 24, skin);
    AE(-6, -2, 6, 3, shoe);
    AE(6, -2, 6, 3, shoe);
    AL(-10, -54, -22, -36 - wave, 3, skin);                      /* the arms: one waves while she talks */
    AL(10, -54, 22, -38, 3, skin);
    AQ(-10, -60, 10, -60, 20, -22, -20, -22, dress);
    AB(-20, -24, 40, 3, blend(dress, K(255, 255, 255), 6));
    if (golden) {                                                /* curls all around */
        AE(-15, -70, 9, 12, hair);
        AE(15, -70, 9, 12, hair);
        AE(-12, -60, 7, 8, hair);
        AE(12, -60, 7, 8, hair);
    } else {                                                     /* two pigtails with ribbons */
        AE(-22, -72, 7, 10, hair);
        AE(22, -72, 7, 10, hair);
        AE(-17, -80, 4, 3, K(255, 80, 120));
        AE(17, -80, 4, 3, K(255, 80, 120));
    }
    AE(0, -74, 16, 16, skin);
    AE(0, -84, 17, 9, hair);                                     /* the fringe */
    AE(-6, -74, 2, 3, ink);
    AE(6, -74, 2, 3, ink);
    AE(-7, -75, 1, 1, wh);
    AE(5, -75, 1, 1, wh);
    AE(-10, -68, 3, 2, K(255, 160, 160));
    AE(10, -68, 3, 2, K(255, 160, 160));
    if (g > 1)
        AE(0, -66 + g / 3, 4, 1 + g / 2, K(150, 40, 60));
    else {                                                       /* a smile */
        AL(-5, -68, 0, -65, 1, ink);
        AL(0, -65, 5, -68, 1, ink);
    }
}

/* ------------------------------------------------------------- Goldilocks --- */
enum { GS_FOREST, GS_HOUSE, GS_KITCHEN, GS_CHAIRS, GS_BEDROOM };
static void forest(void)
{
    int i;
    vgrad(0, 180, K(120, 200, 255), K(210, 240, 220));
    sun(200, 36, 15, K(255, 240, 150), K(255, 230, 120));
    cloud(((int)(TM / 80) + 40) % 300 - 40, 40, 8, K(255, 255, 255));
    hills(170, 10, 160, 30, 240, K(110, 190, 90));
    for (i = 0; i < 6; i++)
        tree(18 + i * 44 + (i & 1) * 8, 168 + (i & 1) * 6, 34 + (i * 7 % 12), K(120, 80, 50), K(60, 150, 70), K(80, 170, 80));
    hat_ground(180, K(100, 180, 80), K(70, 150, 60));
    fill_ellipse(110, 214, 70, 14, K(220, 190, 130));            /* the path */
}
static void room(uint16_t wall, uint16_t wall2)
{
    int x;
    vgrad(0, 176, wall, wall2);
    for (x = 0; x < 240; x += 30)                                 /* the wallpaper's stripes */
        box_(x, 0, 12, 176, blend(wall, wall2, 6));
    box_(14, 40, 50, 44, K(130, 90, 60));                         /* a window on the forest */
    box_(18, 44, 42, 36, K(140, 210, 255));
    fill_circle(30, 70, 9, K(70, 160, 80));
    fill_circle(48, 66, 12, K(80, 170, 90));
    box_(37, 44, 3, 36, K(130, 90, 60));
    for (x = 0; x < 240; x += 24)                                 /* the floor boards */
        box_(x, 176, 23, 64, K(170 + (x / 24 % 2) * 12, 120, 80));
}
static void bear(int x, int y, int s, uint16_t fur, int bow)    /* feet at (x, y), s in % */
{
    int cx = x, cy = y, sw = g_sw, sh = g_sh, g = sc.mouth * 8 / 255;
    uint16_t lt = blend(fur, K(255, 240, 210), 7), ink = K(30, 20, 20);
    g_sw = sw * s / 100;
    g_sh = sh * s / 100;
    AE(-14, -10, 12, 9, fur);
    AE(14, -10, 12, 9, fur);
    AE(0, -40, 26, 32, fur);
    AE(0, -36, 16, 22, lt);
    AE(-24, -48, 8, 14, fur);
    AE(24, -48, 8, 14, fur);
    AE(-17, -92, 9, 9, fur);
    AE(17, -92, 9, 9, fur);
    AE(-17, -92, 5, 5, lt);
    AE(17, -92, 5, 5, lt);
    AE(0, -78, 22, 19, fur);
    AE(0, -71, 10, 8 + g / 3, lt);
    AE(0, -75, 4, 3, ink);
    AE(-8, -83, 3, 3, ink);
    AE(8, -83, 3, 3, ink);
    if (g > 1)
        AE(0, -67 + g / 3, 4, 1 + g / 2, K(150, 40, 60));
    if (bow) {
        AT(10, -96, 22, -102, 22, -90, K(255, 90, 150));
        AT(10, -96, -2, -102, -2, -90, K(255, 90, 150));
    }
    g_sw = sw;
    g_sh = sh;
}
static void bg_goldy(void)
{
    int code = story_scene(3, spart(3)) & 0x7F, part = spart(3), i;
    if (code == GS_FOREST) {
        forest();
    } else if (code == GS_HOUSE) {
        forest();
        box_(140, 110, 90, 72, K(190, 120, 70));                  /* the bears' cottage */
        fill_tri(130, 112, 240, 112, 185, 66, K(170, 60, 50));
        box_(205, 70, 12, 26, K(120, 70, 50));
        for (i = 0; i < 3; i++)
            fill_circle(211 + (isin((int)(TM / 30) + i * 60) >> 5), 60 - i * 12 - (int)((TM / 60u) % 12u), 5 + i * 2, K(230, 230, 235));
        box_(152, 130, 26, 52, K(60, 40, 30));                    /* the open door */
        fill_quad(152, 130, 166, 136, 166, 186, 152, 182, K(150, 90, 50));
        box_(194, 128, 24, 20, K(255, 230, 140));
        box_(205, 128, 2, 20, K(150, 90, 50));
    } else if (code == GS_KITCHEN) {
        room(K(250, 220, 170), K(240, 200, 150));
        box_(100, 132, 132, 10, K(150, 95, 55));                  /* the table */
        box_(108, 142, 8, 40, K(130, 80, 45));
        box_(216, 142, 8, 40, K(130, 80, 45));
        {   /* three bowls: big, middle, little (empty once she has eaten it) */
            static const int BX[3] = {130, 172, 208}, BR[3] = {16, 12, 9};
            for (i = 0; i < 3; i++) {
                fill_ellipse(BX[i], 128, BR[i], BR[i] / 2 + 2, K(250, 250, 255));
                fill_ellipse(BX[i], 124, BR[i] - 2, 3, (i == 2 && part >= 9) ? K(230, 230, 240) : K(240, 220, 160));
                if (i == 0 && part < 10)                       /* the big one is hot */
                    fat_line(BX[i] + (isin((int)(TM / 20)) >> 5), 116, BX[i] + 3, 100 - (int)((TM / 50u) % 6u), 1, K(255, 255, 255));
            }
        }
    } else if (code == GS_CHAIRS) {
        static const int CX[3] = {128, 172, 210}, CH[3] = {60, 46, 32};
        room(K(220, 240, 200), K(200, 225, 180));
        for (i = 0; i < 3; i++) {                                 /* three chairs; the little one broken from part 13 on */
            int x = CX[i], h = CH[i], w = h * 2 / 3;
            if (i == 2 && part >= 12) {
                fill_quad(x - 14, 182, x + 4, 176, x + 6, 182, x - 12, 186, K(170, 110, 60));
                fill_quad(x + 2, 186, x + 20, 180, x + 22, 186, x + 4, 190, K(150, 95, 50));
                box_(x - 4, 172, 4, 14, K(170, 110, 60));
                continue;
            }
            box_(x - w / 2, 182 - h / 2, w, 6, K(170, 110, 60));
            box_(x - w / 2, 182 - h / 2, 4, h / 2, K(150, 95, 50));
            box_(x + w / 2 - 4, 182 - h / 2, 4, h / 2, K(150, 95, 50));
            box_(x + w / 2 - 6, 182 - h, 6, h / 2, K(170, 110, 60));
            box_(x + w / 2 - 6 - w / 2, 182 - h, w / 2 + 6, 5, K(170, 110, 60));
        }
    } else {
        static const int DX[3] = {124, 172, 212}, DW[3] = {44, 38, 28}, DH[3] = {34, 26, 20};
        room(K(190, 210, 250), K(170, 190, 240));
        for (i = 0; i < 3; i++) {                                 /* three beds: big, middle, little */
            int x = DX[i], w = DW[i], h = DH[i];
            box_(x - w / 2, 182 - h - 14, 5, h + 14, K(150, 95, 55));
            box_(x - w / 2 + 2, 182 - h, w - 2, h - 6, K(240, 120 + i * 40, 140));
            fill_ellipse(x - w / 2 + 12, 182 - h + 2, 8, 5, K(255, 255, 255));
            box_(x + w / 2 - 3, 182 - h - 4, 5, h + 4, K(150, 95, 55));
        }
        if (part >= 16 && part <= 23) {                           /* Goldilocks asleep in the little bed */
            fill_circle(DX[2] - DW[2] / 2 + 12, 182 - DH[2] - 4, 8, K(255, 210, 70));
            fill_circle(DX[2] - DW[2] / 2 + 12, 182 - DH[2] - 2, 6, K(255, 214, 180));
            for (i = 0; i < 2; i++)
                text_big(DX[2] + 6 + i * 10, 182 - DH[2] - 30 - i * 12 - (int)((TM / 120u) % 8u), &FONT_B, "Z", K(255, 255, 255), 1);
        }
    }
}
static void act_goldy(int cx, int cy)
{
    int code = story_scene(3, spart(3)), part = spart(3);
    if (code & 0x80) {                                            /* the three bears */
        int far = (code & 0x7F) == GS_HOUSE || (code & 0x7F) == GS_FOREST;
        bear(150, 186, far ? 70 : 84, K(140, 90, 50), 0);
        bear(186, 186, far ? 60 : 72, K(180, 120, 70), 1);
        bear(216, 186, far ? 44 : 52, K(200, 140, 80), 0);
    }
    if (!((code & 0x7F) == GS_BEDROOM && part >= 16 && part <= 23))
        girl(cx, cy, 1);
}
static void fg_goldy(void) {}
