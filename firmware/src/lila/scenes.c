/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's twelve worlds. Each is a backdrop, a character and a foreground, drawn in screen coordinates (the screen is drawn in
 * two bands, so every scene runs twice a frame). A character is drawn with its feet (or its middle, if it floats) at (cx, cy)
 * and everything is scaled by X() and Y(): the SIZE knob scales it, BOING squashes and stretches it.
 * The wrapper (world_draw) adds what the sound does to every world: ghosts for the echo, a shimmer for the chorus, twins
 * for the buddy, friends for the chord, sparkles for the resonance. Included by the unity build after draw.c. */
#pragma once

/* what the world needs to know about the sound and the music, refreshed every frame by the UI */
static struct {
    uint32_t t;                 /* ms */
    int world;
    int lvl, mouth, hit, bounce, pitch, lfo;    /* 0..255: loudness, the singing, a note just hit, the beat, the last note's height, LFO -127..127 */
    int sz, lean, sq;           /* the size in %, the lean (px), the squash in % */
    int wave, cut, reso, metal, fat, big, echo, space, fluff, robot, stars, friends, mood, grit;
    int jump;                   /* px the character is up */
    int beatph;                 /* 0..255, the metronome's phase at the tempo */
    int play, rec, arp;         /* the melody plays, the loop records, the arpeggiator holds notes */
    /* what the world's own knobs do: the pictures that go with them */
    int ctype, camt, noise, am, chat, ping, stomp, bell, vowel, form, vibd, chip, duty, pwm;
    int shake, shake_dx;        /* px the whole picture shakes (a stomp, a roar), and how far this frame */
    int stompf, bellf, blipf, pingf;     /* 0..255 flashes: a stomp, the cowbell, a blip, a ping */
    int score;                  /* notes played: the arcade's score */
    int siren;                  /* 0..255: a siren or alarm sounds (the flashing lights) */
    int chiphue;                /* the colours of the character flash through a fast chord (0..767) */
} sc;
static int g_sw = 100, g_sh = 100;
#define X(v) ((v) * g_sw / 100)
#define Y(v) ((v) * g_sh / 100)
#define AE(x, y, rx, ry, c) fill_ellipse(cx + X(x), cy + Y(y), imax(1, X(rx)), imax(1, Y(ry)), c)
#define AT(x0, y0, x1, y1, x2, y2, c) fill_tri(cx + X(x0), cy + Y(y0), cx + X(x1), cy + Y(y1), cx + X(x2), cy + Y(y2), c)
#define AL(x0, y0, x1, y1, r, c) fat_line(cx + X(x0), cy + Y(y0), cx + X(x1), cy + Y(y1), imax(1, X(r)), c)
#define AB(x, y, w, h, c) box_(cx + X(x), cy + Y(y), imax(1, X(w)), imax(1, Y(h)), c)
#define AR(x, y, r, th, c) ring_(cx + X(x), cy + Y(y), imax(1, X(r)), imax(1, th), c)
#define AQ(x0, y0, x1, y1, x2, y2, x3, y3, c) \
    fill_quad(cx + X(x0), cy + Y(y0), cx + X(x1), cy + Y(y1), cx + X(x2), cy + Y(y2), cx + X(x3), cy + Y(y3), c)
#define TM (sc.t)

typedef struct {
    const char *name;
    void (*bg)(void);
    void (*actor)(int cx, int cy);
    void (*fg)(void);
    int16_t hx, hy;                 /* where the character stands */
    uint32_t horizon;               /* the colour ghosts fade into */
    uint32_t accent;                /* the HUD's colour for this world */
} scene_t;

/* ---------------------------------------------------------- shared props --- */
static void cloud(int x, int y, int s, uint16_t c)
{
    fill_ellipse(x, y, s * 3, s, c);
    fill_circle(x - s, y - s / 2, s, c);
    fill_circle(x + s, y - s * 2 / 3, s * 5 / 4, c);
}
static void sun(int x, int y, int r, uint16_t c, uint16_t ray)
{
    int i, a0 = (int)(TM / 40);
    for (i = 0; i < 8; i++) {
        int a = a0 + i * 32, c1 = icos(a), s1 = isin(a), c2 = icos(a + 10), s2 = isin(a + 10), c3 = icos(a - 10), s3 = isin(a - 10);
        fill_tri(x + c1 * (r + 14 + (sc.hit >> 5)) / 127, y + s1 * (r + 14 + (sc.hit >> 5)) / 127, x + c2 * (r + 3) / 127,
                 y + s2 * (r + 3) / 127, x + c3 * (r + 3) / 127, y + s3 * (r + 3) / 127, ray);
    }
    fill_circle(x, y, r, c);
}
static void tree(int x, int y, int s, uint16_t trunk, uint16_t leaf, uint16_t leaf2)
{
    box_(x - s / 6, y - s, s / 3 + 1, s, trunk);
    fill_circle(x, y - s - s / 3, s * 2 / 3, leaf);
    fill_circle(x - s / 2, y - s, s / 2, leaf2);
    fill_circle(x + s / 2, y - s, s / 2, leaf);
}
static void grass_tufts(int y, int n, uint16_t c, int sway)
{
    int i;
    for (i = 0; i < n; i++) {
        int x = i * 240 / n + 5, h = 8 + (i * 7 % 6), w = (isin((int)(TM / 20) + i * 30) * sway) >> 7;
        fill_tri(x - 3, y, x + 3, y, x + w, y - h, c);
    }
}
static void hat_ground(int y, uint16_t top, uint16_t bot)       /* a flat ground with a lighter edge */
{
    vgrad(y, 240, top, bot);
}
static void stars_sky(int n, int ymax, int speed, uint16_t c)
{
    int i;
    for (i = 0; i < n; i++) {
        int x = (i * 97 + (int)(TM * (unsigned)speed / 200u)) % 240, y = (i * 53) % ymax, tw = isin((int)(TM / 12) + i * 40);
        if (tw > -50)
            cv_pset(x, y, c);
        if (tw > 90)
            sparkle(x, y, 2, c);
    }
}

#include "scenes_a.c"
#include "scenes_b.c"
#include "scenes_c.c"
#include "scenes_piano.c"
#include "scenes_inst.c"
static void bg_fable(void);
static void act_fable(int cx, int cy);
static void fg_fable(void);
static void bg_chap(void);
static void act_chap(int cx, int cy);
static void fg_chap(void);
static void bg_goldy(void);
static void act_goldy(int cx, int cy);
static void fg_goldy(void);

static const scene_t SCENES[NPRESET] = {
    {"BIRD", bg_bird, act_bird, fg_bird, 112, 150, 0xCDEBFF, 0xFFC83C},
    {"COW", bg_cow, act_cow, fg_cow, 120, 176, 0xB8E8FF, 0xFFFFFF},
    {"WOLF", bg_dog, act_dog, fg_dog, 120, 178, 0xBFE6FF, 0xE8A050},
    {"FROG", bg_frog, act_frog, fg_frog, 120, 176, 0x4F9A90, 0x6EDC50},
    {"SPACE", bg_space, act_space, fg_space, 120, 112, 0x10163A, 0xA070FF},
    {"METEOR", bg_meteor, act_meteor, fg_meteor, 120, 116, 0x20102A, 0xFF8030},
    {"BLOCKS", bg_blocks, act_blocks, fg_blocks, 120, 178, 0x8CCBFF, 0x60D040},
    {"DINO", bg_dino, act_dino, fg_dino, 128, 180, 0xFFC890, 0xFF6040},
    {"ROBOT", bg_robot, act_robot, fg_robot, 120, 196, 0x202838, 0x40D0FF},
    {"OCEAN", bg_ocean, act_ocean, fg_ocean, 112, 112, 0x1C6EA0, 0x40C8FF},
    {"CASTLE", bg_castle, act_castle, fg_castle, 120, 176, 0xFFC8E8, 0xFF80D0},
    {"JUNGLE", bg_jungle, act_jungle, fg_jungle, 112, 178, 0xBDE8A0, 0x80E040},
    {"PIANO", bg_piano, act_piano, fg_piano, 120, 128, 0x2A1030, 0xFFD450},
    {"E-PIANO", bg_piano, act_piano, fg_piano, 120, 128, 0x20083A, 0x50F0FF},
    {"ORGAN", bg_piano, act_piano, fg_piano, 120, 128, 0x40303C, 0xFFC080},
    {"XYLO", bg_xylo, act_xylo, fg_xylo, 120, 130, 0xFFECFA, 0xFF70A0},
    {"HARP", bg_harp, act_harp, fg_harp, 190, 112, 0x78469A, 0xFFE070},
    {"UKULELE", bg_uke, act_uke, fg_uke, 120, 196, 0xFFD68C, 0xFF8A50},
    {"FLUTE", bg_flute, act_flute, fg_flute, 150, 204, 0xFF8C5A, 0x80E0A0},
    {"KALIMBA", bg_kalimba, act_kalimba, fg_kalimba, 120, 124, 0xFFB070, 0xFFC040},
    {"CORBEAU", bg_fable, act_fable, fg_fable, 80, 200, 0xCDEBFF, 0x8080B0},
    {"BOEUF", bg_fable, act_fable, fg_fable, 120, 176, 0xB8E8FF, 0x6EDC50},
    {"LION", bg_fable, act_fable, fg_fable, 90, 190, 0xBDE8A0, 0xFFA830},
    {"BOUCLE", bg_goldy, act_goldy, fg_goldy, 56, 188, 0xD0F0D0, 0xFFD040},
    {"CHAPERON", bg_chap, act_chap, fg_chap, 56, 188, 0xD0F0D0, 0xFF4050},
};
#include "scenes_story.c"
#include "scenes_fables.c"
#include "scenes_chaperon.c"

/* the world and everything the sound does to it, into the band that is set up */
static void world_draw(void)
{
    const scene_t *w = &SCENES[sc.world % NPRESET];
    int cx = w->hx, cy = w->hy - sc.jump, i, nb = sc.fat > 104 ? 2 : sc.fat > 58 ? 1 : 0, nf = sc.friends, ghosts = sc.echo > 110 ? 3 : sc.echo > 78 ? 2 : sc.echo > 45 ? 1 : 0;
    uint16_t hz;
    int sw0, sh0, hue0;
    g_sh = sc.sz * sc.sq / 100;                          /* squash and stretch keep the area */
    g_sw = sc.sz * 100 / (sc.sq > 20 ? sc.sq : 100);
    sw0 = g_sw;
    sh0 = g_sh;
    look.fade = 0;
    hz = KC(w->horizon);
    w->bg();
    hue0 = look.hue;
    look.hue = (hue0 + sc.chiphue) % 768;                    /* a fast chord turns the colours of the character, not of the sky */
    /* the ground rumbles for a big sound: lines that jump */
    if (sc.big > 40 && sc.hit > 100)
        for (i = 0; i < 3; i++)
            ring_(cx, w->hy + 6, 30 + (255 - sc.hit) / 3 + i * 18, 2, K(255, 255, 255));
    /* friends stand beside it, a little smaller, each dancing to its own beat */
    for (i = 0; i < nf; i++) {
        int side = (i & 1) ? 1 : -1, ox = side * (64 + (i >> 1) * 38), jmp = (isin((int)(TM / 6) + i * 70) > 100) ? 6 : 0;
        g_sw = sw0 * 7 / 10;
        g_sh = sh0 * 7 / 10;
        look.fade = 2;
        look.fadecol = hz;
        w->actor(cx + ox, w->hy + (w->hy > 150 ? 0 : 6) - jmp - (sc.hit > 150 ? 8 : 0) + (i + 1) * 0);
    }
    g_sw = sw0 * 9 / 10;
    g_sh = sh0 * 9 / 10;
    look.fadecol = hz;
    for (i = nb; i > 0; i--) {                           /* the buddy's twins, behind and wider */
        look.fade = 3 + i;
        w->actor(cx - 18 * i + (isin((int)(TM / 9) + i * 80) * sc.fat >> 9), cy - 4 * i);
        w->actor(cx + 18 * i - (isin((int)(TM / 9) + i * 80) * sc.fat >> 9), cy - 4 * i);
    }
    for (i = ghosts; i > 0; i--) {                       /* the echo: older and fainter copies, further away */
        int d = i * (22 + sc.echo / 6);
        g_sw = sw0 * (100 - i * 12) / 100;
        g_sh = sh0 * (100 - i * 12) / 100;
        look.fade = 6 + i * 3;
        w->actor(cx + (i & 1 ? d : -d) + (isin((int)(TM / 8) + i * 50) >> 4), cy - i * 3);
    }
    g_sw = sw0;
    g_sh = sh0;
    look.fade = 0;
    if (sc.fluff > 34) {                                 /* the chorus: a shimmering double */
        int d = (isin((int)(TM / 5)) * (2 + sc.fluff / 12)) >> 7;
        look.fade = 5;
        look.fadecol = hz;
        w->actor(cx + d, cy);
        look.fade = 0;
    }
    w->actor(cx, cy);
    w->fg();
    look.hue = hue0;
}
