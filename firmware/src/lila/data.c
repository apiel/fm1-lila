/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's data: the twelve worlds' sounds, the melodies and the beats. */
#include "lila.h"

/* A world's sound is the neutral sound (preset_load: every parameter 0, except BEND, SIZE, DUTY, VOWEL, VIBS, AMR, CTIME, FMR, NZDEC, PINGT and
 * CHATR, which have a middle) plus the parameters listed here. Pitch contours: CTYPE says which (CT_*), CAMT how far it goes (0..127 is
 * 0..24 semitones), CTIME how long (or, for a siren, how slow). The knobs that show these are in ui.c, under each world's own names. */

static const pv_t SET_BIRD[] = {
    {P_ECHO, 26}, {P_SPACE, 40}, {P_FLUFF, 24}, {P_CUT, 112}, {P_RESO, 18}, {P_ATK, 3}, {P_PLUCK, 78}, {P_REL, 36},
    {P_BOING, 30}, {P_LSPD, 104}, {P_LDEP, 46}, {P_WAVE, 8}, {P_METAL, 34}, {P_FMR, 42}, {P_NOISE, 8}, {P_CTYPE, 6},
    {P_CAMT, 32}, {P_CTIME, 28},
};
static const pv_t SET_COW[] = {
    {P_ECHO, 12}, {P_SPACE, 34}, {P_FLUFF, 10}, {P_CUT, 52}, {P_RESO, 60}, {P_GRIT, 6}, {P_ATK, 34}, {P_PLUCK, 8},
    {P_REL, 50}, {P_BOING, 62}, {P_LSPD, 38}, {P_WAVE, 92}, {P_BUDDY, 24}, {P_BIG, 44}, {P_FMR, 17}, {P_NOISE, 10},
    {P_NZDEC, 60}, {P_CTYPE, 1}, {P_CAMT, 16}, {P_CTIME, 91}, {P_FORM, 96}, {P_VOWEL, 16}, {P_VIBD, 34}, {P_VIBS, 40},
    {P_VIBDLY, 70}, {P_BELL, 40},
};
static const pv_t SET_DOG[] = {
    {P_ECHO, 74}, {P_SPACE, 72}, {P_CUT, 88}, {P_RESO, 26}, {P_ATK, 44}, {P_REL, 88}, {P_BOING, 34}, {P_LSPD, 30},
    {P_LWAVE, 2}, {P_WAVE, 44}, {P_BUDDY, 22}, {P_BIG, 16}, {P_FMR, 8}, {P_CTYPE, 1}, {P_CAMT, 64}, {P_CTIME, 107},
    {P_VIBD, 46}, {P_VIBS, 70}, {P_VIBDLY, 62}, {P_VOWEL, 10}, {P_FORM, 84}, {P_AMR, 70},
};
static const pv_t SET_FROG[] = {
    {P_ECHO, 18}, {P_SPACE, 54}, {P_CUT, 70}, {P_RESO, 56}, {P_GRIT, 10}, {P_PLUCK, 88}, {P_REL, 26}, {P_BOING, 40},
    {P_LWHAT, 2}, {P_LWAVE, 2}, {P_WAVE, 84}, {P_FMR, 17}, {P_NOISE, 8}, {P_NZDEC, 20}, {P_CTYPE, 8}, {P_CAMT, 37},
    {P_CTIME, 62}, {P_AMR, 83}, {P_AMD, 104}, {P_VOWEL, 34}, {P_FORM, 84},
};
static const pv_t SET_SPACE[] = {
    {P_ECHO, 66}, {P_SPACE, 96}, {P_FLUFF, 64}, {P_CUT, 58}, {P_RESO, 46}, {P_ATK, 74}, {P_REL, 88}, {P_BOING, 46},
    {P_LSPD, 18}, {P_LDEP, 62}, {P_LWHAT, 1}, {P_WAVE, 84}, {P_BUDDY, 84}, {P_BIG, 44}, {P_METAL, 8}, {P_NZDEC, 127},
    {P_CTYPE, 5}, {P_CTIME, 30},
};
static const pv_t SET_METEOR[] = {
    {P_ECHO, 62}, {P_SPACE, 54}, {P_CUT, 104}, {P_RESO, 40}, {P_GRIT, 10}, {P_PLUCK, 100}, {P_REL, 70}, {P_BOING, 58},
    {P_LSPD, 60}, {P_WAVE, 22}, {P_BUDDY, 20}, {P_BIG, 20}, {P_METAL, 112}, {P_FMR, 51}, {P_NOISE, 56}, {P_NZDEC, 93},
    {P_CTYPE, 5}, {P_CAMT, 74}, {P_CTIME, 45}, {P_STOMP, 56}, {P_CHAT, 40}, {P_CHATR, 50},
};
static const pv_t SET_BLOCKS[] = {
    {P_ECHO, 10}, {P_SPACE, 10}, {P_ROBOT, 44}, {P_CUT, 122}, {P_GRIT, 14}, {P_PLUCK, 76}, {P_REL, 18}, {P_LSPD, 70},
    {P_LDEP, 14}, {P_LWAVE, 2}, {P_WAVE, 127}, {P_FMR, 8}, {P_DUTY, 44}, {P_PWMD, 48}, {P_CHIP, 1}, {P_CHIPS, 100},
    {P_CTYPE, 5}, {P_CAMT, 21}, {P_CTIME, 39},
};
static const pv_t SET_DINO[] = {
    {P_ECHO, 34}, {P_SPACE, 40}, {P_FLUFF, 20}, {P_CUT, 40}, {P_RESO, 62}, {P_GRIT, 70}, {P_ATK, 8}, {P_PLUCK, 28},
    {P_REL, 60}, {P_BOING, 94}, {P_LSPD, 88}, {P_LDEP, 74}, {P_LWHAT, 2}, {P_LWAVE, 1}, {P_WAVE, 90}, {P_BUDDY, 100},
    {P_BIG, 112}, {P_METAL, 18}, {P_FMR, 8}, {P_NOISE, 48}, {P_NZDEC, 100}, {P_CTYPE, 5}, {P_CAMT, 42}, {P_CTIME, 100},
    {P_AMR, 66}, {P_AMD, 72}, {P_VOWEL, 58}, {P_FORM, 72}, {P_STOMP, 64},
};
static const pv_t SET_ROBOT[] = {
    {P_ECHO, 26}, {P_SPACE, 20}, {P_ROBOT, 82}, {P_CUT, 88}, {P_RESO, 62}, {P_GRIT, 20}, {P_PLUCK, 72}, {P_REL, 18},
    {P_BOING, 56}, {P_LSPD, 96}, {P_LDEP, 50}, {P_LWHAT, 1}, {P_LWAVE, 2}, {P_WAVE, 100}, {P_BUDDY, 30}, {P_BIG, 20},
    {P_METAL, 26}, {P_CHAT, 52}, {P_CHATR, 70}, {P_VOWEL, 84}, {P_FORM, 58},
};
static const pv_t SET_OCEAN[] = {
    {P_ECHO, 74}, {P_SPACE, 100}, {P_FLUFF, 92}, {P_CUT, 78}, {P_RESO, 24}, {P_ATK, 2}, {P_PLUCK, 100}, {P_REL, 52},
    {P_BOING, 30}, {P_LSPD, 26}, {P_LDEP, 40}, {P_BUDDY, 22}, {P_BIG, 30}, {P_METAL, 36}, {P_FMR, 8}, {P_CTYPE, 6},
    {P_CAMT, 37}, {P_CTIME, 104}, {P_VIBD, 22}, {P_VIBS, 30}, {P_VIBDLY, 84}, {P_PING, 44}, {P_PINGT, 64},
    {P_CHAT, 30}, {P_CHATR, 30},
};
static const pv_t SET_CASTLE[] = {
    {P_ECHO, 60}, {P_SPACE, 84}, {P_FLUFF, 40}, {P_CUT, 118}, {P_RESO, 14}, {P_PLUCK, 110}, {P_REL, 90}, {P_BOING, 24},
    {P_LSPD, 44}, {P_LDEP, 20}, {P_LWHAT, 1}, {P_WAVE, 4}, {P_BUDDY, 28}, {P_METAL, 64}, {P_STARS, 38},
};
static const pv_t SET_JUNGLE[] = {
    {P_ECHO, 22}, {P_SPACE, 40}, {P_FLUFF, 30}, {P_CUT, 56}, {P_RESO, 44}, {P_GRIT, 14}, {P_ATK, 44}, {P_REL, 54},
    {P_BOING, 76}, {P_LSPD, 74}, {P_WAVE, 88}, {P_BUDDY, 54}, {P_BIG, 52}, {P_FMR, 8}, {P_NOISE, 16}, {P_NZDEC, 34},
    {P_CTYPE, 6}, {P_CAMT, 26}, {P_CTIME, 62}, {P_VOWEL, 60}, {P_FORM, 52}, {P_VIBD, 22}, {P_VIBS, 64}, {P_VIBDLY, 60},
    {P_STOMP, 34},
};

/* the two story worlds: a clear voice, a little room */
static const pv_t SET_LILA[] = {
    {P_CUT, 112}, {P_RESO, 8}, {P_REL, 34}, {P_SPACE, 34}, {P_ECHO, 8}, {P_CTIME, 64}, {P_BUDDY, 0}, {P_WAVE, 0}, {P_PINGT, 30},
};
static const pv_t SET_GOLDY[] = {
    {P_CUT, 112}, {P_RESO, 8}, {P_REL, 34}, {P_SPACE, 26}, {P_ECHO, 6}, {P_CTIME, 64}, {P_BUDDY, 0}, {P_WAVE, 0}, {P_PINGT, 30},
};

#define SET(n) SET_##n, (uint8_t)(sizeof SET_##n / sizeof SET_##n[0])
const preset_t PRESETS[NPRESET] = {
    {"BIRD", "TWEET", SET(BIRD), 1, 95, 3, -1},
    {"COW", "MOO", SET(COW), -1, 60, 0, -1},
    {"DOG", "WOOF", SET(DOG), 0, 126, 0, -1},
    {"FROG", "RIBBIT", SET(FROG), -1, 94, 2, -1},
    {"SPACE", "WHOOSH", SET(SPACE), 0, 77, 1, -1},
    {"METEOR", "ZAP", SET(METEOR), 0, 54, 4, -1},
    {"BLOCKS", "BLOOP", SET(BLOCKS), 0, 55, 1, -1},
    {"DINO", "ROAR", SET(DINO), -2, 101, 0, -1},
    {"ROBOT", "BEEP", SET(ROBOT), 0, 111, 1, -1},
    {"OCEAN", "BLUB", SET(OCEAN), 0, 90, 2, -1},
    {"CASTLE", "TINKLE", SET(CASTLE), 1, 105, 4, -1},
    {"JUNGLE", "TRUMPET", SET(JUNGLE), -1, 62, 3, -1},
    {"LILA", "UNE HISTOIRE", SET(LILA), 0, 75, 0, 0},
    {"GOLDILOCKS", "A STORY", SET(GOLDY), 0, 80, 0, 1},
};

/* a melody is a list of notes: letter, optional # or b, octave (c4 = middle C), length. Length: 0 a sixteenth, 1 an eighth,
 * 2 a quarter, 3 a dotted quarter, 4 a half, 5 a dotted eighth, 6 a dotted half, 8 a whole. r is a rest. */
const melody_t MELODIES[NMEL] = {
    {"TWINKLE STAR",
     "c42 c42 g42 g42 a42 a42 g44  f42 f42 e42 e42 d42 d42 c44  g42 g42 f42 f42 e42 e42 d44  g42 g42 f42 f42 e42 e42 d44  "
     "c42 c42 g42 g42 a42 a42 g44  f42 f42 e42 e42 d42 d42 c44"},
    {"FRERE JACQUES",
     "c42 d42 e42 c42  c42 d42 e42 c42  e42 f42 g44  e42 f42 g44  g41 a41 g41 f41 e42 c42  g41 a41 g41 f41 e42 c42  "
     "c42 g32 c44  c42 g32 c44"},
    {"ODE TO JOY",
     "e42 e42 f42 g42  g42 f42 e42 d42  c42 c42 d42 e42  e43 d41 d44  e42 e42 f42 g42  g42 f42 e42 d42  "
     "c42 c42 d42 e42  d43 c41 c44"},
    {"MARY'S LAMB",
     "e42 d42 c42 d42  e42 e42 e44  d42 d42 d44  e42 g42 g44  e42 d42 c42 d42  e42 e42 e42 e42  d42 d42 e42 d42  c48"},
    {"OLD MACDONALD",
     "c42 c42 c42 g32  a32 a32 g34  e42 e42 d42 d42 c46 g31 g31  c42 c42 c42 g32  a32 a32 g34  e42 e42 d42 d42 c48"},
    {"AU CLAIR LUNE",
     "c42 c42 c42 d42  e44 d44  c42 e42 d42 d42  c48  c42 c42 c42 d42  e44 d44  c42 e42 d42 d42  c48  "
     "d42 d42 d42 d42  a34 a34  d42 c42 b32 a32  g38  c42 c42 c42 d42  e44 d44  c42 e42 d42 d42  c48"},
    {"JINGLE BELLS",
     "e42 e42 e44  e42 e42 e44  e42 g42 c43 d41  e48  f42 f42 f43 f41  f42 e42 e42 e41 e41  e42 d42 d42 e42  d44 g44  "
     "e42 e42 e44  e42 e42 e44  e42 g42 c43 d41  e48  f42 f42 f42 f41  f42 e42 e42 e41 e41  g42 g42 f42 d42  c48"},
    {"HAPPY BIRTHDAY",
     "g35 g30 a32 g32 c42 b34  g35 g30 a32 g32 d42 c44  g35 g30 g42 e42 c42 b32 a32  f45 f40 e42 c42 d42 c46"},
    {"LONDON BRIDGE",
     "g43 a41 g42 f42 e42 f42 g42  d42 e42 f44  e42 f42 g44  g43 a41 g42 f42 e42 f42 g42  d42 g42 e42 c44"},
    {"HOT CROSS BUNS",
     "e42 d42 c44  e42 d42 c44  c41 c41 c41 c41 d41 d41 d41 d41  e42 d42 c44"},
    {"ROW YOUR BOAT",
     "c42 c42 c43 d41 e42  e43 d41 e43 f41 g46  c51 c51 c51 g41 g41 g41 e41 e41 e41 c41 c41 c41  g43 f41 e43 d41 c46"},
    {"STAIRS",
     "c42 d42 e42 f42 g42 a42 b42 c52  c52 b42 a42 g42 f42 e42 d42 c44"},
};

/* bit 0 is the first of sixteen steps; written as strings of 16 steps for the eye */
#define S(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    ((uint16_t)((a) | (b) << 1 | (c) << 2 | (d) << 3 | (e) << 4 | (f) << 5 | (g) << 6 | (h) << 7 | (i) << 8 | (j) << 9 | \
                (k) << 10 | (l) << 11 | (m) << 12 | (n) << 13 | (o) << 14 | (p) << 15))
const beat_t BEATS[NBEAT] = {
    {"MARCH",   S(1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0),
                S(1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0), 0},
    {"ROCK",    S(1,0,0,0, 0,0,1,0, 1,0,1,0, 0,0,0,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0),
                S(1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0), 0},
    {"DISCO",   S(1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0),
                S(0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0)},
    {"REGGAE",  S(0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0), S(0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0),
                S(1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0), 0},
    {"HIP HOP", S(1,0,0,0, 0,0,0,1, 0,0,1,0, 0,0,0,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,1),
                S(1,0,1,1, 1,0,1,0, 1,0,1,1, 1,0,1,0), 0},
    {"POLKA",   S(1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0), S(0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0),
                S(1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0), 0},
    {"ROBOT",   S(1,0,0,1, 0,0,1,0, 1,0,0,1, 0,0,1,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,1),
                S(1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1), S(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0)},
    {"PARTY",   S(1,0,0,1, 0,0,1,0, 0,1,0,0, 1,0,1,0), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0),
                S(0,1,1,1, 0,1,1,1, 0,1,1,1, 0,1,1,1), S(0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0)},
};
