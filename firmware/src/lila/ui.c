/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's user interface. The rules:
 *   - every button has one page, and the four knobs (K1..K4) always do something on it;
 *   - the keys only play, OCT- / OCT+ only change the octave;
 *   - MASTER is the volume, SELECT picks the melody (while it plays), PRESETS picks the world, ALGORITHM the mood;
 *   - whatever is turned, the world changes with it (scenes.c, world_draw and look).
 * Included by the unity build after draw.c and scenes.c. */
#include "lila.h"
#include "../app/plat.h"

enum { PG_HOME, PG_FX, PG_SEL, PG_ENV, PG_LFO, PG_EDIT, PG_GLO, PG_SAVE, PG_ARP, PG_SEQ, PG_PLAY, PG_REC, NPAGES };
enum { FM_NONE, FM_BPM, FM_KEY, FM_SEMI, FM_WAVE, FM_BEND, FM_BARS };

typedef struct {
    const char *name;                   /* <= 8 characters */
    uint8_t id, max;                    /* the parameter (P_*), its top value; a switch has names */
    uint8_t fmt;                        /* FM_*: a number in words */
    const char *const *names;
    const uint8_t *vals;                /* a switch whose choices are not 0, 1, 2 ...: the value of each */
} knobdef_t;
typedef struct {
    const char *title;
    uint8_t button;
    uint32_t color;
    knobdef_t k[4];
} page_t;

/* the panel's keys: key 0 = F3 ... 26 = G5 (the simulators use these tables) */
__attribute__((unused)) static const uint8_t WHITE_K[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
__attribute__((unused)) static const uint8_t BLACK_K[11] = {1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25};

static const char *const N_TYPE[] = {"SOFT", "TALKY", "THIN"};
static const char *const N_WHAT[] = {"SIREN", "WAH", "TREMBLE"};
static const char *const N_LWAVE[] = {"SMOOTH", "ZIGZAG", "CHOPPY", "BUMPY"};
static const char *const N_EASY[] = {"ANY", "PENTA", "MAJOR"};
static const char *const N_FRIENDS[] = {"ALONE", "TWINS", "POWER", "FAMILY"};
static const char *const N_ARP[] = {"OFF", "UP", "DOWN", "UP-DOWN", "RANDOM"};
static const char *const N_ARPSPD[] = {"SNAIL", "TURTLE", "RABBIT", "CHEETAH"};
static const char *const N_ARPRNG[] = {"1 OCT", "2 OCT", "3 OCT"};
static const char *const N_YESNO[] = {"NO", "YES"};
static const char *const N_BEAT[] = {"OFF", "MARCH", "ROCK", "DISCO", "REGGAE", "HIP HOP", "POLKA", "ROBOT", "PARTY"};
static const char *const N_MMODE[] = {"LISTEN", "FOLLOW"};
static const char *const N_MREP[] = {"ONCE", "REPEAT", "ALL"};
static const char *const N_MBAND[] = {"SOLO", "ZOO"};
static const char *const N_ACT[] = {"SAVE", "LOAD", "TRASH"};
static const char *const N_RSPD[] = {"HALF", "NORMAL", "DOUBLE"};
static const char *const N_BOX[] = {"BOX 1", "BOX 2", "BOX 3", "BOX 4", "BOX 5", "BOX 6", "BOX 7", "BOX 8"};
static const char *const N_STICK[] = {"STAR", "HEART", "MOON", "SUN", "FLOWER", "NOTE", "DROP", "BOLT", "FISH", "CROWN", "LEAF", "SMILE"};
static const char *const N_COLOR[] = {"RED", "ORANGE", "YELLOW", "GREEN", "TEAL", "BLUE", "PURPLE", "PINK"};

static const page_t PAGES[NPAGES] = {
    {"MAGIC", B_HOME, 0xFFD040, {{"BEND", P_BEND, 127, FM_BEND, 0}, {"STARS", P_STARS, 127, 0, 0}, {"SIZE", P_SIZE, 127, 0, 0}, {"WIGGLE", P_WIGGLE, 127, 0, 0}}},
    {"FX", B_FX, 0xFF70C0, {{"ECHO", P_ECHO, 127, 0, 0}, {"SPACE", P_SPACE, 127, 0, 0}, {"FLUFFY", P_FLUFF, 127, 0, 0}, {"ROBOT", P_ROBOT, 127, 0, 0}}},
    {"FILTER", B_SEL, 0x50D0FF, {{"BRIGHT", P_CUT, 127, 0, 0}, {"WOW", P_RESO, 127, 0, 0}, {"TYPE", P_FTYPE, 2, 0, N_TYPE}, {"GRIT", P_GRIT, 127, 0, 0}}},
    {"ENVELOPE", B_ENV, 0x70E070, {{"FADE IN", P_ATK, 127, 0, 0}, {"PLUCK", P_PLUCK, 127, 0, 0}, {"FADE OUT", P_REL, 127, 0, 0}, {"BOING", P_BOING, 127, 0, 0}}},
    {"WOBBLE", B_LFO, 0xFFA040, {{"SPEED", P_LSPD, 127, 0, 0}, {"WOBBLE", P_LDEP, 127, 0, 0}, {"WHAT", P_LWHAT, 2, 0, N_WHAT}, {"WAVE", P_LWAVE, 3, 0, N_LWAVE}}},
    {"SOUND", B_EDIT, 0xB080FF, {{"SHAPE", P_WAVE, 127, FM_WAVE, 0}, {"BUDDY", P_BUDDY, 127, 0, 0}, {"BIG", P_BIG, 127, 0, 0}, {"METAL", P_METAL, 127, 0, 0}}},
    {"PARTY", B_GLO, 0x60FFB0, {{"TEMPO", P_TEMPO, 127, FM_BPM, 0}, {"KEY", P_KEY, 12, FM_KEY, 0}, {"EASY", P_EASY, 2, 0, N_EASY}, {"FRIENDS", P_FRIENDS, 3, 0, N_FRIENDS}}},
    {"TREASURE", B_SAVE, 0xFFE070, {{"BOX", P_BOX, 7, 0, N_BOX}, {"DO", P_ACT, 2, 0, N_ACT}, {"STICKER", P_STICK, 11, 0, N_STICK}, {"COLOR", P_COLOR, 7, 0, N_COLOR}}},
    {"ARPEGGIO", B_ARP, 0xFF8060, {{"MODE", P_ARP, 4, 0, N_ARP}, {"SPEED", P_ARPSPD, 3, 0, N_ARPSPD}, {"RANGE", P_ARPRNG, 2, 0, N_ARPRNG}, {"STICKY", P_STICKY, 1, 0, N_YESNO}}},
    {"BEAT", B_SEQ, 0xFF5050, {{"STYLE", P_BEAT, 8, 0, N_BEAT}, {"LOUD", P_BEATVOL, 127, 0, 0}, {"BOUNCE", P_BOUNCE, 127, 0, 0}, {"TUNING", P_TUNING, 127, 0, 0}}},
    {"SONG", B_PLAY, 0x80FF80, {{"SPEED", P_TEMPO, 127, FM_BPM, 0}, {"MODE", P_MMODE, 1, 0, N_MMODE}, {"REPEAT", P_MREPEAT, 2, 0, N_MREP}, {"BAND", P_MBAND, 1, 0, N_MBAND}}},
    {"LOOP", B_REC, 0xFF4040, {{"LENGTH", P_RLEN, 3, FM_BARS, 0}, {"LOUD", P_RVOL, 127, 0, 0}, {"SPEED", P_RSPEED, 2, 0, N_RSPD}, {"FUNNY", P_RFUNNY, 24, FM_SEMI, 0}}},
};
static const char *const N_MOOD[NMOOD] = {"SUNNY", "NIGHT", "MAGIC", "CRAZY"};

/* ---------------------------------------------------------------- each world's own knobs --- */
/* A world takes over some knobs of the generic pages and gives them its own names and meanings: a dog's HOME page has its siren and
 * its howl, a frog's EDIT page its croak, a robot's HOME page its chatter. (page, knob 0..3, definition) */
static const char *const N_SIREN[] = {"WOLF", "POLICE", "HI-LO", "ALARM"};
static const uint8_t V_SIREN[] = {CT_HOWL, CT_WAIL, CT_HILO, CT_ALARM};
static const char *const N_FROGSTYLE[] = {"RIBBIT", "PEEP", "BOING", "PLAIN"};
static const uint8_t V_FROGSTYLE[] = {CT_RIBBIT, CT_SWOOP, CT_DIVE, CT_NONE};
static const char *const N_GAME[] = {"PEW", "COIN", "JUMP", "PLAIN"};
static const uint8_t V_GAME[] = {CT_DIVE, CT_COIN, CT_SWOOP, CT_NONE};
static const char *const N_CHIP[] = {"OFF", "MAJOR", "MINOR", "OCTAVE", "POWER", "SPOOKY"};
static const char *const N_RSIREN[] = {"OFF", "BEEP-BOP", "SWEEP", "ALARM"};
static const uint8_t V_RSIREN[] = {CT_NONE, CT_HILO, CT_WAIL, CT_ALARM};
typedef struct { uint8_t page, k; knobdef_t d; } wk_t;
#define WK(pg, k, ...) {pg, k, {__VA_ARGS__}}
static const wk_t WK_BIRD[] = {
    WK(PG_HOME, 3, "TWEETS", P_CHAT, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "CHIRP", P_CAMT, 127, 0, 0, 0), WK(PG_EDIT, 2, "LENGTH", P_CTIME, 127, 0, 0, 0)};
static const wk_t WK_COW[] = {
    WK(PG_HOME, 0, "MOO", P_VOWEL, 127, 0, 0, 0), WK(PG_HOME, 1, "BELL", P_BELL, 127, 0, 0, 0), WK(PG_HOME, 3, "WOBBLY", P_WIGGLE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "THROAT", P_FORM, 127, 0, 0, 0), WK(PG_EDIT, 2, "DEEP", P_BIG, 127, 0, 0, 0), WK(PG_EDIT, 3, "BREATH", P_NOISE, 127, 0, 0, 0),
    WK(PG_ENV, 3, "SWAY", P_VIBD, 127, 0, 0, 0),
    WK(PG_FX, 0, "BARN", P_ECHO, 127, 0, 0, 0), WK(PG_FX, 1, "FIELD", P_SPACE, 127, 0, 0, 0)};
static const wk_t WK_DOG[] = {
    WK(PG_HOME, 0, "SIREN", P_CTYPE, 3, 0, N_SIREN, V_SIREN), WK(PG_HOME, 1, "HOWL", P_CAMT, 127, 0, 0, 0), WK(PG_HOME, 2, "SLOW", P_CTIME, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "OOOH", P_VOWEL, 127, 0, 0, 0), WK(PG_EDIT, 2, "THROAT", P_FORM, 127, 0, 0, 0), WK(PG_EDIT, 3, "SHAKE", P_VIBD, 127, 0, 0, 0),
    WK(PG_FX, 0, "CANYON", P_ECHO, 127, 0, 0, 0), WK(PG_FX, 1, "NIGHT", P_SPACE, 127, 0, 0, 0), WK(PG_FX, 3, "TINNY", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_FROG[] = {
    WK(PG_HOME, 0, "STYLE", P_CTYPE, 3, 0, N_FROGSTYLE, V_FROGSTYLE), WK(PG_HOME, 1, "GAP", P_CTIME, 127, 0, 0, 0), WK(PG_HOME, 2, "JUMP", P_CAMT, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "CROAK", P_AMD, 127, 0, 0, 0), WK(PG_EDIT, 2, "RATTLE", P_AMR, 127, 0, 0, 0), WK(PG_EDIT, 3, "THROAT", P_VOWEL, 127, 0, 0, 0),
    WK(PG_FX, 0, "POND", P_ECHO, 127, 0, 0, 0), WK(PG_FX, 1, "MARSH", P_SPACE, 127, 0, 0, 0), WK(PG_FX, 2, "BUBBLES", P_CHAT, 127, 0, 0, 0),
    WK(PG_FX, 3, "CRUNCH", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_SPACE[] = {
    WK(PG_HOME, 3, "ALIEN", P_VIBD, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "CREW", P_BUDDY, 127, 0, 0, 0), WK(PG_EDIT, 2, "ENGINE", P_NOISE, 127, 0, 0, 0), WK(PG_EDIT, 3, "LASER", P_CAMT, 127, 0, 0, 0),
    WK(PG_FX, 1, "GALAXY", P_SPACE, 127, 0, 0, 0), WK(PG_FX, 3, "STATIC", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_METEOR[] = {
    WK(PG_HOME, 0, "PEW", P_CAMT, 127, 0, 0, 0), WK(PG_HOME, 1, "BOOM", P_NOISE, 127, 0, 0, 0), WK(PG_HOME, 2, "CRASH", P_STOMP, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SPARKS", P_CHAT, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "ROCK", P_FMR, 127, 0, 0, 0), WK(PG_EDIT, 3, "SPEED", P_CTIME, 127, 0, 0, 0),
    WK(PG_FX, 2, "SMOKE", P_FLUFF, 127, 0, 0, 0)};
static const wk_t WK_BLOCKS[] = {
    WK(PG_HOME, 0, "GAME", P_CTYPE, 3, 0, N_GAME, V_GAME), WK(PG_HOME, 1, "CHORD", P_CHIP, 5, 0, N_CHIP, 0), WK(PG_HOME, 2, "FAST", P_CHIPS, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 0, "DUTY", P_DUTY, 127, 0, 0, 0), WK(PG_EDIT, 1, "PWM", P_PWMD, 127, 0, 0, 0), WK(PG_EDIT, 2, "SHAPE", P_WAVE, 127, FM_WAVE, 0, 0),
    WK(PG_EDIT, 3, "BOOM", P_NOISE, 127, 0, 0, 0),
    WK(PG_FX, 3, "8-BIT", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_DINO[] = {
    WK(PG_HOME, 0, "GROWL", P_AMD, 127, 0, 0, 0), WK(PG_HOME, 1, "RUMBLE", P_AMR, 127, 0, 0, 0), WK(PG_HOME, 2, "STOMP", P_STOMP, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "SCREAM", P_VOWEL, 127, 0, 0, 0), WK(PG_EDIT, 2, "PACK", P_BUDDY, 127, 0, 0, 0), WK(PG_EDIT, 3, "SPIT", P_NOISE, 127, 0, 0, 0),
    WK(PG_FX, 0, "CANYON", P_ECHO, 127, 0, 0, 0), WK(PG_FX, 3, "SNARL", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_ROBOT[] = {
    WK(PG_HOME, 0, "CHATTER", P_CHAT, 127, 0, 0, 0), WK(PG_HOME, 1, "RANGE", P_CHATR, 127, 0, 0, 0), WK(PG_HOME, 2, "VOICE", P_VOWEL, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "SIREN", P_CTYPE, 3, 0, N_RSIREN, V_RSIREN), WK(PG_EDIT, 2, "SLOW", P_CTIME, 127, 0, 0, 0), WK(PG_EDIT, 3, "FM", P_METAL, 127, 0, 0, 0),
    WK(PG_FX, 2, "STUTTER", P_AMD, 127, 0, 0, 0), WK(PG_FX, 3, "CRUSH", P_ROBOT, 127, 0, 0, 0)};
static const wk_t WK_OCEAN[] = {
    WK(PG_HOME, 0, "BUBBLES", P_CHAT, 127, 0, 0, 0), WK(PG_HOME, 1, "SONAR", P_PING, 127, 0, 0, 0), WK(PG_HOME, 2, "WHALE", P_CAMT, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "SINGING", P_VIBD, 127, 0, 0, 0), WK(PG_EDIT, 2, "DREAMY", P_VIBDLY, 127, 0, 0, 0),
    WK(PG_FX, 1, "DEEP", P_SPACE, 127, 0, 0, 0), WK(PG_FX, 2, "CURRENT", P_FLUFF, 127, 0, 0, 0), WK(PG_FX, 3, "PING GAP", P_PINGT, 127, 0, 0, 0)};
static const wk_t WK_CASTLE[] = {
    WK(PG_HOME, 1, "DUST", P_STARS, 127, 0, 0, 0), WK(PG_HOME, 3, "WAND", P_CHIP, 5, 0, N_CHIP, 0),
    WK(PG_EDIT, 2, "CHIME", P_PING, 127, 0, 0, 0)};
static const wk_t WK_JUNGLE[] = {
    WK(PG_HOME, 0, "TRUMPET", P_CAMT, 127, 0, 0, 0), WK(PG_HOME, 1, "BLOW", P_NOISE, 127, 0, 0, 0), WK(PG_HOME, 2, "STOMP", P_STOMP, 127, 0, 0, 0),
    WK(PG_HOME, 3, "SIZE", P_SIZE, 127, 0, 0, 0),
    WK(PG_EDIT, 1, "HONK", P_VOWEL, 127, 0, 0, 0), WK(PG_EDIT, 2, "HERD", P_BUDDY, 127, 0, 0, 0)};
#define WKS(t) {t, (uint8_t)(sizeof t / sizeof t[0])}
static const struct { const wk_t *t; uint8_t n; } WORLD_KNOBS[NPRESET] = {
    WKS(WK_BIRD), WKS(WK_COW), WKS(WK_DOG), WKS(WK_FROG), WKS(WK_SPACE), WKS(WK_METEOR), WKS(WK_BLOCKS), WKS(WK_DINO), WKS(WK_ROBOT),
    WKS(WK_OCEAN), WKS(WK_CASTLE), WKS(WK_JUNGLE)};

/* the knob a page shows for this world: the world's own, or the generic one */
static const knobdef_t *kdef(int page, int k)
{
    int i, w = P[P_PRESET] % NPRESET;
    for (i = 0; i < WORLD_KNOBS[w].n; i++)
        if (WORLD_KNOBS[w].t[i].page == page && WORLD_KNOBS[w].t[i].k == k)
            return &WORLD_KNOBS[w].t[i].d;
    return &PAGES[page].k[k];
}
/* a switch's position for the value it holds */
static int kindex(const knobdef_t *k, int v)
{
    int i;
    if (!k->vals)
        return v > k->max ? k->max : v;
    for (i = 0; i <= k->max; i++)
        if (k->vals[i] == v)
            return i;
    return 0;
}

static struct {
    uint8_t page;
    int8_t touched;                     /* the knob last turned, -1 none */
    uint32_t touch_ms;                  /* when anything last happened on the panel (the HUD stays up 4 s) */
    uint32_t bprev, kprev;
    int hud_y;                          /* the HUD's top row: 240 hidden .. 196 shown */
    char toast[32];
    char toast2[32];
    uint32_t toast_until, toast_t0;
    uint16_t toast_col;
    uint32_t frame;
    uint32_t trans_t0;                  /* a world change: the iris closes and opens */
    int trans_to, trans_done;
    uint8_t drum_f[4];
    uint8_t mel_done_seen;
    uint8_t arp_seen;
    uint32_t held_k;                    /* keys held now */
    int8_t pulse[4];
    uint8_t save_pop;                   /* the treasure boxes' celebration */
    int8_t beat_wave;
    uint32_t last_ms;
    int level_prev;
    uint8_t oct_seen;
} ui;

/* a saved sound: the box on the TREASURE page */
typedef struct {
    uint8_t used, preset, sticker, color;
    uint8_t snd[NSND];
} box_t;
#define NBOX 8
static box_t boxes[NBOX];
typedef struct {
    uint32_t magic;
    uint8_t p[NP];
    box_t box[NBOX];
} lila_save_t;
#define LILA_MAGIC 0x4C494C32u          /* "LIL2": the sound grew to NSND parameters */

static void toast(const char *a, const char *b, uint16_t col)
{
    int i;
    for (i = 0; a[i] && i < 30; i++)
        ui.toast[i] = a[i];
    ui.toast[i] = 0;
    for (i = 0; b && b[i] && i < 30; i++)
        ui.toast2[i] = b[i];
    ui.toast2[b ? i : 0] = 0;
    ui.toast_t0 = plat_ms();
    ui.toast_until = ui.toast_t0 + 1600u;
    ui.toast_col = col;
}
static uint16_t rgb24(uint32_t c) { return RGB((c >> 16) & 255u, (c >> 8) & 255u, c & 255u); }
void ui_say(const char *a, const char *b) { toast(a, b, RGB(255, 255, 255)); }

/* ------------------------------------------------------------ the saved state --- */
static void lila_save_now(void)
{
    static lila_save_t s;
    int i;
    s.magic = LILA_MAGIC;
    for (i = 0; i < NP; i++)
        s.p[i] = P[i];
    for (i = 0; i < NBOX; i++)
        s.box[i] = boxes[i];
    plat_store_save(OBJ_SET, &s, sizeof s);
}
static void lila_load(void)
{
    static lila_save_t s;
    int i;
    if (plat_store_load(OBJ_SET, &s, sizeof s) == (int)sizeof s && s.magic == LILA_MAGIC) {
        for (i = 0; i < NP; i++)
            P[i] = s.p[i];
        for (i = 0; i < NBOX; i++)
            boxes[i] = s.box[i];
        if (P[P_PRESET] >= NPRESET)
            P[P_PRESET] = 0;
        for (i = 0; i < NP; i++)               /* a damaged record must not leave a knob out of range */
            if (i >= P_TEMPO && P[i] > 127)
                P[i] = 127;
        P[P_BEAT] = P[P_BEAT] > 8 ? 0 : P[P_BEAT];
        P[P_ARP] = P[P_ARP] > 4 ? 0 : P[P_ARP];
        P[P_OCT] = P[P_OCT] > 6 ? 3 : P[P_OCT];
        P[P_KEY] = P[P_KEY] > 12 ? 6 : P[P_KEY];
        P[P_RFUNNY] = P[P_RFUNNY] > 24 ? 12 : P[P_RFUNNY];
        P[P_RLEN] &= 3;
        P[P_BEAT] = 0;                         /* a beat or an arpeggio that starts by itself would be a surprise */
        P[P_ARP] = 0;
        P[P_MOOD] &= 3;
    }
    /* else: engine_init() sets the defaults */
}

/* ----------------------------------------------------------------- the knobs --- */
static void fmt_value(const knobdef_t *k, int v, char *out)
{
    switch (k->fmt) {
    case FM_BPM:
        put_i(out, BPM_OF(v));
        break;
    case FM_KEY:
    case FM_SEMI: {
        int s = k->fmt == FM_KEY ? v - 6 : v - 12;
        char *p = out;
        if (s > 0)
            *p++ = '+';
        put_i(p, s);
        break;
    }
    case FM_BEND: {
        int s = (v - 64) * 12 / 64;
        char *p = out;
        if (s > 0)
            *p++ = '+';
        put_i(p, s);
        break;
    }
    case FM_WAVE: {
        static const char *const WN[4] = {"SINE", "TRIANGLE", "SAW", "SQUARE"};
        put_s(out, WN[iclamp((v * 3 + 64) / 127, 0, 3)]);
        break;
    }
    case FM_BARS: {
        static const char *const BN[4] = {"1 BAR", "2 BARS", "4 BARS", "8 BARS"};
        put_s(out, BN[v & 3]);
        break;
    }
    default:
        out[0] = 0;
    }
}

static void world_change(int idx, int immediate);
static void box_action(void);

static void knob_turn(int i, int d)
{
    const knobdef_t *k = kdef(ui.page, i);
    int v = P[k->id], sw = k->names != 0 || k->max < 40, cur = kindex(k, v), step, nv, mag = d < 0 ? -d : d, pv;
    if (sw)
        step = d;                                          /* a switch: one click, one step */
    else
        step = d * (mag >= 3 ? 8 : mag == 2 ? 4 : 2);     /* a number: two per click, more when it spins */
    nv = cur + step;
    nv = nv < 0 ? 0 : nv > k->max ? k->max : nv;
    pv = sw && k->vals ? k->vals[nv] : nv;
    if (pv != v || sw) {
        P[k->id] = (uint8_t)pv;
        if (k->id == P_ARP && v == 0 && pv)
            ev_post(EV_ARP_CLEAR, 0, 0);
        if (k->id == P_BEAT && !pv && v)
            ev_post(EV_BEAT_RESET, 0, 0);
    }
    if (ui.page == PG_SAVE && i == 0 && pv != v)
        ui.save_pop = 0;
    ui.touched = (int8_t)i;
    ui.pulse[i] = 8;
    ui.touch_ms = plat_ms();
}

/* ------------------------------------------------------------------- worlds --- */
static void world_change(int idx, int immediate)
{
    idx = (idx % NPRESET + NPRESET) % NPRESET;
    preset_load(idx);
    if (immediate) {
        sc.world = idx;
        ui.trans_to = idx;
        ui.trans_done = 1;
        ui.trans_t0 = 0;
    } else {
        ui.trans_to = idx;
        ui.trans_done = 0;
        ui.trans_t0 = plat_ms();
    }
    toast(PRESETS[idx].name, PRESETS[idx].voice, rgb24(SCENES[idx].accent));
    ui.touch_ms = plat_ms();
}

static void box_action(void)
{
    int b = P[P_BOX], act = P[P_ACT], i;
    box_t *bx = &boxes[b & 7];
    if (act == 0) {
        bx->used = 1;
        bx->preset = P[P_PRESET];
        bx->sticker = P[P_STICK];
        bx->color = P[P_COLOR];
        for (i = 0; i < NSND; i++)
            bx->snd[i] = P[i];
        lila_save_now();
        toast("SAVED!", N_BOX[b & 7], RGB(255, 230, 90));
        ui.save_pop = 30;
    } else if (act == 1) {
        if (!bx->used) {
            toast("EMPTY BOX", 0, RGB(255, 150, 150));
            return;
        }
        if (bx->preset != P[P_PRESET])
            world_change(bx->preset, 0);
        for (i = 0; i < NSND; i++)
            P[i] = bx->snd[i];
        P[P_PRESET] = bx->preset;
        toast("OPENED!", PRESETS[bx->preset % NPRESET].name, RGB(150, 255, 150));
        ui.save_pop = 30;
    } else {
        bx->used = 0;
        lila_save_now();
        toast("EMPTY NOW", N_BOX[b & 7], RGB(200, 200, 255));
    }
}

/* ------------------------------------------------------------------- input --- */
static const uint8_t PAGE_OF_BTN[NB] = {PG_FX, PG_SEL, PG_ENV, PG_LFO, PG_EDIT, PG_GLO, PG_HOME, PG_SAVE, PG_ARP, PG_SEQ, PG_PLAY, PG_REC, 255, 255};

static void goto_page(int pg)
{
    if (ui.page != pg)
        ui.touched = -1;
    ui.page = (uint8_t)pg;
    ui.touch_ms = plat_ms();
    toast(PAGES[pg].title, 0, rgb24(PAGES[pg].color));
    ui.toast_until = plat_ms() + 900u;
}

static void input(void)
{
    uint32_t b = plat_buttons(), k = plat_keys(), bd = b & ~ui.bprev, kd = k & ~ui.kprev, ku = ui.kprev & ~k;
    int i, d;
    uint32_t now = plat_ms();
    engine_set_master(plat_master());
    for (i = 0; i < NB; i++) {
        if (!((bd >> i) & 1u))
            continue;
        if (i == B_OCTDN || i == B_OCTUP) {
            int o = (int)P[P_OCT] + (i == B_OCTUP ? 1 : -1);
            if (o >= 0 && o <= 6) {
                P[P_OCT] = (uint8_t)o;
                ev_post(EV_PANIC, 0, 0);
            }
            {
                char t[16], *q = t;
                q = put_s(q, "OCTAVE ");
                if (P[P_OCT] > 3)
                    *q++ = '+';
                put_i(q, (int)P[P_OCT] - 3);
                toast(t, 0, RGB(255, 255, 255));
            }
            ui.touch_ms = now;
            continue;
        }
        {
            int pg = PAGE_OF_BTN[i];
            int same = ui.page == pg;
            if (pg == PG_PLAY) {
                if (mel_playing) {
                    ev_post(EV_STOP, 0, 0);
                    toast("STOP", 0, RGB(255, 200, 120));
                } else {
                    ev_post(EV_PLAY, mel_idx, 0);
                    toast(melody_name(mel_idx), "PLAYING", RGB(150, 255, 150));
                }
                ui.page = PG_PLAY;
                ui.touched = -1;
                ui.touch_ms = now;
                continue;
            }
            if (pg == PG_REC) {
                ev_post(EV_REC, 0, 0);
                ui.page = PG_REC;
                ui.touched = -1;
                ui.touch_ms = now;
                continue;
            }
            if (pg == PG_SAVE && same) {
                box_action();
                ui.touch_ms = now;
                continue;
            }
            if (pg == PG_ARP) {
                if (same)
                    P[P_ARP] = P[P_ARP] ? 0 : 1;
                else if (!P[P_ARP])
                    P[P_ARP] = 1;
                if (P[P_ARP] && !same)
                    ev_post(EV_ARP_CLEAR, 0, 0);
            }
            if (pg == PG_SEQ) {
                if (same)
                    P[P_BEAT] = P[P_BEAT] ? 0 : 1;
                else if (!P[P_BEAT])
                    P[P_BEAT] = 1;
                if (!P[P_BEAT])
                    ev_post(EV_BEAT_RESET, 0, 0);
            }
            goto_page(pg);
            if (pg == PG_ARP)
                toast(PAGES[pg].title, P[P_ARP] ? "ON" : "OFF", rgb24(PAGES[pg].color));
            if (pg == PG_SEQ)
                toast(PAGES[pg].title, P[P_BEAT] ? N_BEAT[P[P_BEAT] % 9] : "OFF", rgb24(PAGES[pg].color));
            if (pg == PG_SAVE)
                toast(PAGES[pg].title, "PRESS AGAIN", rgb24(PAGES[pg].color));
        }
    }
    for (i = 0; i < NKEYS_L; i++) {
        if ((kd >> i) & 1u) {
            ev_post(EV_KEY_ON, i, 0);
            ui.touch_ms = now;
        }
        if ((ku >> i) & 1u)
            ev_post(EV_KEY_OFF, i, 0);
    }
    ui.held_k = k;
    for (i = 0; i < 4; i++) {
        d = plat_enc(EN_K1 + i);
        if (d)
            knob_turn(i, d);
    }
    d = plat_enc(EN_PRESET);
    if (d)
        world_change((int)P[P_PRESET] + (d > 0 ? 1 : -1), 0);
    d = plat_enc(EN_ALGO);
    if (d) {
        int m = ((int)P[P_MOOD] + (d > 0 ? 1 : -1) + NMOOD) % NMOOD;
        P[P_MOOD] = (uint8_t)m;
        toast(N_MOOD[m], "MOOD", RGB(255, 255, 255));
        ui.touch_ms = now;
        {
            int n;
            for (n = 0; n < 10; n++)
                part_add(PK_STAR, rnd_range(10, 230), rnd_range(10, 150), rnd_range(-10, 10), rnd_range(-14, -2), 28, rnd_range(3, 7),
                         m == MOOD_NIGHT ? RGB(200, 210, 255) : m == MOOD_CRAZY ? RGB(255, rnd_range(40, 255), rnd_range(40, 255)) : RGB(255, 240, 150), 0);
        }
    }
    d = plat_enc(EN_SELECT);
    if (d) {
        if (mel_playing) {
            int m = ((int)mel_idx + (d > 0 ? 1 : -1) + NMEL) % NMEL;
            ev_post(EV_MELODY, m, 0);
            toast(melody_name(m), "PLAYING", RGB(150, 255, 150));
        } else
            toast("PRESS PLAY", "TO HEAR A SONG", RGB(180, 255, 180));
        ui.touch_ms = now;
    }
    ui.bprev = b;
    ui.kprev = k;
}
void ui_input_only(void) { input(); }

/* ------------------------------------------------------------ the sound's look --- */
static void look_update(void)
{
    int hue = ((int)P[P_KEY] - 6) * 64, mood = P[P_MOOD] & 3;
    int lfo_t = P[P_LWHAT];
    look.bri = 118 + (int)P[P_CUT] * 138 / 127;
    if (lfo_t == LT_WAH && P[P_LDEP])
        look.bri = iclamp(look.bri + (int)sc.lfo * 70 / 127, 90, 256);
    look.sat = 256 - (int)P[P_METAL] * 150 / 127;
    look.warm = P[P_FTYPE] == 0 ? 8 : P[P_FTYPE] == 2 ? -12 : 0;
    if (mood == MOOD_CRAZY)
        hue += (int)(plat_ms() / 14u);
    while (hue < 0)
        hue += 768;
    look.hue = hue % 768;
    look.night = mood == MOOD_NIGHT ? 9 : 0;
    look.flash = sc.bounce > 220 ? 1 : 0;
    look.fade = 0;
    look.wr = look.wb = 0;
    if (sc.siren > 40) {                                 /* the siren's lights wash the whole picture: red, blue, red ... or an alarm's red */
        int ph = (int)(plat_ms() / 190u) & 1;
        if (sc.ctype == CT_ALARM)
            look.wr = ph ? 110 : 0;
        else if (ph)
            look.wr = 100;
        else
            look.wb = 100;
    }
}

/* ---------------------------------------------------- the world follows the sound --- */
static void note_origin(int *x, int *y)
{
    const scene_t *w = &SCENES[sc.world % NPRESET];
    *x = w->hx + 18 * sc.sz / 100;
    *y = w->hy - 70 * sc.sz / 100 - sc.jump;
}

static void world_events(void)
{
    vev_t e;
    int guard = 0;
    uint32_t now = plat_ms();
    while (vis_pop(&e) && guard++ < 16) {
        int x, y;
        uint16_t col;
        note_origin(&x, &y);
        if (e.type == VE_BLIP) {                         /* a little sound the world adds: fairy dust, the cowbell, a chatter, a ping */
            uint16_t bc = e.src == 0x42 ? RGB(255, 210, 60) : e.src == 0x44 ? RGB(150, 230, 255) : RGB(255, 255, 255);
            if (e.src == 0x42) {
                sc.bellf = 255;
            } else if (e.src == 0x44) {
                sc.pingf = 255;
                part_add(PK_RING, x - 20, y + 30, 0, 0, 30, 14, bc, 0);
            } else if (e.src == 0x43) {
                sc.blipf = 255;
                if (sc.world == 3 || sc.world == 9)              /* a frog or the sea: bubbles */
                    part_add(PK_BUBBLE, x + rnd_range(-50, 40), y + rnd_range(10, 70), rnd_range(-4, 4), rnd_range(-34, -18), 44, rnd_range(3, 7), RGB(220, 250, 255), 0);
                else if (sc.world == 5)                          /* a meteor: sparks of debris */
                    part_add(PK_SPARK, rnd_range(20, 220), rnd_range(190, 225), rnd_range(-20, 20), rnd_range(-50, -20), 26, 5, RGB(255, rnd_range(120, 220), 60), 0);
                else if (sc.world == 8 || sc.world == 6)         /* a robot, an arcade: square pixels */
                    part_add(PK_CONF, x + rnd_range(-50, 50), y + rnd_range(0, 50), rnd_range(-10, 10), rnd_range(-30, -10), 30, 2,
                             RGB(rnd_range(80, 255), rnd_range(200, 255), rnd_range(80, 255)), 0);
                else
                    part_add(PK_NOTE, x + rnd_range(-50, 50), y + rnd_range(-20, 40), rnd_range(-10, 10), rnd_range(-34, -18), 40, 3,
                             RGB(rnd_range(120, 255), rnd_range(160, 255), rnd_range(120, 255)), P[P_WAVE] * 3 / 127);
            }
            continue;
        }
        sc.hit = 255;
        sc.score += 10;
        if (P[P_STOMP] > 8) {
            sc.stompf = 255;
            sc.shake = imax(sc.shake, 2 + P[P_STOMP] / 20);
        }
        sc.pitch = iclamp(((int)e.note - 40) * 255 / 50, 0, 255);
        col = RGB(iclamp(120 + (e.note * 53) % 136, 0, 255), iclamp(110 + (e.note * 91) % 146, 0, 255), iclamp(130 + (e.note * 37) % 126, 0, 255));
        if (e.type != VE_ARP || (ui.frame & 1u))
            part_add(PK_NOTE, x + rnd_range(-8, 14), y + rnd_range(-10, 6), rnd_range(-6, 14), rnd_range(-34, -18), 46, 4, col, P[P_WAVE] * 3 / 127);
        if (P[P_SPACE] > 24 && e.type != VE_ARP)
            part_add(PK_RING, SCENES[sc.world % NPRESET].hx, SCENES[sc.world % NPRESET].hy - sc.jump - (SCENES[sc.world % NPRESET].hy > 150 ? 4 : 0), 0, 0, 26, 10 + P[P_SPACE] / 9, col, 0);
        if (P[P_STARS] > 10)
            part_add(PK_STAR, x + rnd_range(-50, 50), y + rnd_range(-30, 50), rnd_range(-6, 6), rnd_range(-10, 0), 24, 3 + P[P_STARS] / 30,
                     RGB(255, 245, 160), 0);
        (void)now;
    }
    if (vis_beat_hit) {
        uint8_t h = vis_beat_hit;
        vis_beat_hit = 0;
        if (h & 1) {
            ui.drum_f[0] = 255;
            sc.bounce = 255;
        }
        if (h & 2)
            ui.drum_f[1] = 255;
        if (h & 4)
            ui.drum_f[2] = 255;
        if (h & 8)
            ui.drum_f[3] = 255;
    }
    if (mel_done != ui.mel_done_seen) {                  /* a melody ended: confetti */
        int n;
        ui.mel_done_seen = mel_done;
        for (n = 0; n < 40; n++)
            part_add(PK_CONF, rnd_range(10, 230), rnd_range(-20, 20), rnd_range(-12, 12), rnd_range(8, 30), 60,
                     2, RGB(rnd_range(80, 255), rnd_range(80, 255), rnd_range(80, 255)), 0);
        toast("HOORAY!", 0, RGB(255, 230, 90));
    }
}

static void sc_update(void)
{
    int lvl = vis_level, target, lfo = vis_lfo, i;
    uint32_t t = plat_ms();
    sc.t = t;
    if (ui.trans_t0 && !ui.trans_done && t - ui.trans_t0 > 260u) {      /* the iris is closed: the new world */
        sc.world = ui.trans_to;
        ui.trans_done = 1;
    }
    sc.lvl = lvl;
    target = lvl;
    sc.mouth += (target - sc.mouth) * (target > sc.mouth ? 3 : 1) / 4;
    if (lvl > 20)
        sc.mouth = iclamp(sc.mouth * (86 + (isin((int)(t / 7)) * 14 >> 7)) / 100, 0, 255);
    sc.hit -= sc.hit / 6 + 4;
    if (sc.hit < 0)
        sc.hit = 0;
    sc.bounce -= sc.bounce / 5 + 4;
    if (sc.bounce < 0)
        sc.bounce = 0;
    sc.lfo = lfo;
    sc.wave = imin(P[P_WAVE], 127) * 3 / 127;
    sc.cut = imin(P[P_CUT], 127);
    sc.reso = imin(P[P_RESO], 127);
    sc.metal = imin(P[P_METAL], 127);
    sc.fat = imin(P[P_BUDDY], 127);
    sc.big = imin(P[P_BIG], 127);
    sc.echo = imin(P[P_ECHO], 127);
    sc.space = imin(P[P_SPACE], 127);
    sc.fluff = imin(P[P_FLUFF], 127);
    sc.robot = imin(P[P_ROBOT], 127);
    sc.stars = imin(P[P_STARS], 127);
    sc.friends = P[P_FRIENDS] & 3;
    sc.mood = P[P_MOOD];
    sc.grit = imin(P[P_GRIT], 127);
    sc.ctype = P[P_CTYPE];
    sc.camt = imin(P[P_CAMT], 127);
    sc.noise = imin(P[P_NOISE], 127);
    sc.am = imin(P[P_AMD], 127);
    sc.chat = imin(P[P_CHAT], 127);
    sc.ping = imin(P[P_PING], 127);
    sc.stomp = imin(P[P_STOMP], 127);
    sc.bell = imin(P[P_BELL], 127);
    sc.vowel = imin(P[P_VOWEL], 127);
    sc.form = imin(P[P_FORM], 127);
    sc.vibd = imin(P[P_VIBD], 127);
    sc.chip = P[P_CHIP];
    sc.duty = imin(P[P_DUTY], 127);
    sc.pwm = imin(P[P_PWMD], 127);
    sc.siren = (P[P_CTYPE] >= CT_WAIL && P[P_CTYPE] <= CT_ALARM && vis_level > 12) ? 255 : sc.siren - sc.siren / 4;
    sc.stompf -= sc.stompf / 4 + (sc.stompf ? 1 : 0);
    sc.bellf -= sc.bellf / 4 + (sc.bellf ? 1 : 0);
    sc.blipf -= sc.blipf / 4 + (sc.blipf ? 1 : 0);
    sc.pingf -= sc.pingf / 6 + (sc.pingf ? 1 : 0);
    sc.shake -= (sc.shake > 0) + (sc.shake > 4);
    if (sc.shake < 0)
        sc.shake = 0;
    sc.shake_dx = sc.shake ? (int)(urand() % (uint32_t)(sc.shake * 2 + 1)) - sc.shake : 0;
    sc.sz = 58 + (int)P[P_SIZE] * 112 / 127 + (int)P[P_BIG] * 14 / 127;
    if (P[P_LWHAT] == LT_TREM && P[P_LDEP])
        sc.sz += (int)sc.lfo * sc.sz / 380;
    if (sc.form > 20)                                    /* the throat's vowel: "oo" is a small round mouth, "ah" is wide open, "ee" a thin smile */
        sc.mouth = sc.mouth * (46 + (127 - iabs(sc.vowel * 2 - 127)) * 54 / 127) / 100;
    sc.lean = ((int)P[P_BEND] - 64) * 26 / 64;
    if (sc.vibd > 4)                                     /* vibrato: the singer sways */
        sc.lean += isin((int)(t / 18)) * sc.vibd * (sc.mouth + 40) >> 15;
    sc.sq = 100 + ((int)P[P_BOING] * lvl >> 11) - (sc.hit * (6 + (int)P[P_BOING] / 12) >> 8);
    sc.jump = (sc.hit * 10 >> 8) + (sc.bounce * (6 + (int)P[P_BOUNCE] / 20) >> 8) + (((int)P[P_BEND] - 64) * 22 / 64);
    if (P[P_LWHAT] == LT_PITCH && P[P_LDEP])
        sc.jump += (int)sc.lfo * 14 / 127;
    sc.beatph = (int)(((t * (uint32_t)(BPM_OF(P[P_TEMPO]))) * 256u / 60000u) & 255u);
    sc.play = mel_playing;
    sc.rec = rec_state;
    sc.arp = arp_notes;
    if (P[P_BEAT] == 0 && ((sc.beatph + 8) & 255) < 16 && (ui.frame & 3u) == 0)
        sc.bounce = imax(sc.bounce, 70);                 /* with no beat, a soft metronome: the world sways at the tempo */
    if (P[P_WIGGLE] > 90 || (P[P_MOOD] == MOOD_CRAZY && P[P_WIGGLE] > 20))
        sc.jump += isin((int)(t / 8)) / 40;
    for (i = 0; i < 4; i++)
        ui.drum_f[i] = (uint8_t)(ui.drum_f[i] - (ui.drum_f[i] >> 2) - (ui.drum_f[i] ? 1 : 0));
    if (P[P_RESO] > 40 && (urand() & 7u) < (uint32_t)(P[P_RESO] - 40) / 18u) {       /* the resonance sparkles */
        int x, y;
        note_origin(&x, &y);
        part_add(PK_STAR, x - 30 + rnd_range(-50, 50), y + 20 + rnd_range(-50, 60), 0, rnd_range(-6, 2), 18, 3 + P[P_RESO] / 40, RGB(255, 255, 255), 0);
    }
    if (P[P_STARS] > 0 && (urand() % 100u) < (uint32_t)P[P_STARS] / 10u + 1u)
        part_add(PK_STAR, rnd_range(4, 236), rnd_range(4, 190), 0, rnd_range(-4, 4), 30, 2 + P[P_STARS] / 36, RGB(255, 245, 170), 0);
    if (P[P_MOOD] == MOOD_MAGIC && (urand() & 3u) == 0)
        part_add(PK_STAR, rnd_range(4, 236), rnd_range(4, 190), 0, rnd_range(-6, 2), 26, rnd_range(2, 5), RGB(255, rnd_range(120, 255), 255), 0);
    if (mel_playing && P[P_MMODE] == 0 && (urand() & 3u) == 0) {      /* a song: a stream of notes */
        int x, y;
        note_origin(&x, &y);
        part_add(PK_NOTE, x + rnd_range(-60, 60), y + 60 + rnd_range(-20, 40), rnd_range(-4, 4), rnd_range(-30, -14), 50, 3,
                 RGB(rnd_range(120, 255), rnd_range(120, 255), rnd_range(120, 255)), sc.wave);
    }
    if (sc.chip && sc.mouth > 20) {                      /* a chord played in a flash: the colours flash through it */
        static const uint8_t CN[6] = {1, 3, 3, 2, 4, 4};
        int idx = (int)((t * (uint32_t)(8 + (int)P[P_CHIPS] / 3)) / 1000u) % CN[sc.chip % 6];
        sc.chiphue = idx * 140;
    } else
        sc.chiphue = 0;
    if (sc.noise > 20 && sc.mouth > 40 && (urand() & 3u) == 0) {      /* breath, a crash, spit: puffs from the mouth */
        int x, y;
        note_origin(&x, &y);
        part_add(PK_DOT, x + 20 + rnd_range(-6, 6), y + 40 + rnd_range(-6, 6), rnd_range(6, 18), rnd_range(-10, 4), 22, 3 + sc.noise / 30, RGB(200, 200, 210), 0);
    }
    look_update();
    sc.rec = rec_state;
    ui.level_prev = lvl;
}

/* -------------------------------------------------------------------- the HUD --- */
#define HUD_H 42
static void hud_cell(int i, int x, int y)
{
    const page_t *pg = &PAGES[ui.page];
    const knobdef_t *k = kdef(ui.page, i);
    uint16_t col = rgb24(pg->color), dark = RGB(14, 16, 34), hot = RGB(255, 255, 255);
    int v = kindex(k, P[k->id]), w = 56, h = HUD_H - 4, touched = ui.touched == i, pulse = ui.pulse[i];
    char buf[16];
    if (v > k->max)
        v = k->max;                                    /* a damaged value must not index past a name list */
    fill_rbox(x, y, w, h, 7, touched ? hot : col);
    fill_rbox(x + 2, y + 2, w - 4, h - 4, 6, dark);
    text_c(x + w / 2, y + 4, &FONT_XS, k->name, touched ? hot : col);
    if (k->names || k->fmt) {
        const char *s;
        if (k->names)
            s = k->names[v];
        else {
            fmt_value(k, v, buf);
            s = buf;
        }
        text_c(x + w / 2, y + 20, &FONT_XS, s, hot);
    } else {
        int bw = w - 12, fw = bw * v / (k->max ? k->max : 1);
        fill_rbox(x + 6, y + 21, bw, 9 + (pulse > 4 ? 2 : 0), 4, RGB(46, 48, 78));
        if (fw > 3)
            fill_rbox(x + 6, y + 21, fw, 9 + (pulse > 4 ? 2 : 0), 4, col);
        else
            fill_circle(x + 9, y + 25, 3, col);
        fill_circle(x + 6 + fw, y + 25, 3 + (pulse > 0 ? 1 : 0), hot);
    }
}
static void hud_draw(void)
{
    int i, y = ui.hud_y;
    if (y >= 240)
        return;
    for (i = 0; i < 4; i++)
        hud_cell(i, 4 + i * 59, y);
}

/* the treasure boxes' stickers: twelve little pictures, s is their radius */
static void sticker(int x, int y, int s, int idx, uint16_t c, uint16_t ink)
{
    int i;
    switch (idx % 12) {
    case 0:  sparkle(x, y, s + 3, c); fill_circle(x, y, s / 2, c); break;                           /* star */
    case 1:  heart(x, y + 1, s + 1, c); break;
    case 2:  fill_circle(x, y, s, c); fill_circle(x + s / 2 + 1, y - s / 4, s - 1, ink); break;      /* moon */
    case 3:  fill_circle(x, y, s * 2 / 3, c);
             for (i = 0; i < 8; i++)
                 fill_circle(x + icos(i * 32) * (s + 1) / 127, y + isin(i * 32) * (s + 1) / 127, 1, c);
             break;                                                                                  /* sun */
    case 4:  for (i = 0; i < 5; i++)
                 fill_circle(x + icos(i * 51) * s / 2 / 127 * 1 + 0, y + isin(i * 51) * s / 2 / 127, s / 2, c);
             fill_circle(x, y, s / 3, RGB(255, 230, 80)); break;                                      /* flower */
    case 5:  note_glyph(x - 2, y + s / 2, s / 2 + 1, c, 0); break;                                   /* a note */
    case 6:  fill_tri(x, y - s - 1, x - s + 1, y + 1, x + s - 1, y + 1, c); fill_circle(x, y + 2, s - 1, c); break;   /* a drop */
    case 7:  fill_poly((const int16_t[]){(int16_t)(x + 2), (int16_t)(y - s - 1), (int16_t)(x - s / 2 - 1), (int16_t)(y + 1), (int16_t)x, (int16_t)(y + 1),
                       (int16_t)(x - 2), (int16_t)(y + s + 1), (int16_t)(x + s / 2 + 1), (int16_t)(y - 1), (int16_t)x, (int16_t)(y - 1)}, 6, c); break;   /* a bolt */
    case 8:  fill_ellipse(x, y, s, s * 2 / 3, c); fill_tri(x + s - 2, y, x + s + 4, y - s / 2, x + s + 4, y + s / 2, c); cv_pset(x - s / 2, y - 1, ink); break;   /* a fish */
    case 9:  fill_poly((const int16_t[]){(int16_t)(x - s), (int16_t)(y + s / 2), (int16_t)(x - s), (int16_t)(y - s / 2), (int16_t)(x - s / 2), (int16_t)y,
                       (int16_t)x, (int16_t)(y - s), (int16_t)(x + s / 2), (int16_t)y, (int16_t)(x + s), (int16_t)(y - s / 2), (int16_t)(x + s), (int16_t)(y + s / 2)}, 7, c); break;   /* a crown */
    case 10: fill_ellipse(x, y, s, s / 2 + 1, c); cv_pset(x, y, ink); fat_line(x - s, y + s / 2, x + s, y - s / 2, 1, ink); break;   /* a leaf */
    default: fill_circle(x, y, s, c); fill_circle(x - s / 3, y - s / 4, 1, ink); fill_circle(x + s / 3, y - s / 4, 1, ink);
             ring_(x, y + 1, s / 2 + 1, 1, ink); box_(x - s, y - s, s * 2 + 1, s - 1, c); fill_circle(x, y, s, c);
             fill_circle(x - s / 3, y - s / 4, 1, ink); fill_circle(x + s / 3, y - s / 4, 1, ink); box_(x - s / 3, y + s / 3, s * 2 / 3 + 1, 1, ink); break;   /* a smile */
    }
}
static const uint32_t BOX_COL[8] = {0xFF5050, 0xFF9A30, 0xFFE040, 0x60DD60, 0x40D0C0, 0x5090FF, 0xA070FF, 0xFF80C8};

/* the TREASURE page: eight boxes on shelves; the one the knob picks bounces, a filled one shows its sticker */
static void treasure_draw(void)
{
    int i;
    uint32_t t = plat_ms();
    fill_rbox(8, 22, 224, 150, 12, RGB(30, 20, 60));
    fill_rbox(10, 24, 220, 146, 10, RGB(60, 40, 110));
    for (i = 0; i < NBOX; i++) {
        int col = i & 3, row = i >> 2, x = 14 + col * 54, y = 32 + row * 70, sel = (i == (int)P[P_BOX]);
        int bob = sel ? (isin((int)(t / 6)) * 3 >> 7) - 2 : 0;
        uint16_t base = rgb24(BOX_COL[(boxes[i].used ? boxes[i].color : i) & 7]);
        if (sel)
            fill_rbox(x - 3, y - 3 + bob, 54, 64, 10, RGB(255, 255, 255));
        fill_rbox(x, y + bob, 48, 58, 8, boxes[i].used ? base : RGB(40, 30, 80));
        if (!boxes[i].used) {
            ring_(x + 24, y + 28 + bob, 15, 2, RGB(110, 90, 170));
            text_c(x + 24, y + 20 + bob, &FONT_B, "?", RGB(130, 110, 190));
        } else {
            box_(x, y + 40 + bob, 48, 5, blend(base, RGB(0, 0, 0), 5));
            sticker(x + 24, y + 22 + bob, 11, boxes[i].sticker, RGB(255, 255, 255), base);
            text_c(x + 24, y + 44 + bob, &FONT_XS, PRESETS[boxes[i].preset % NPRESET].name, RGB(255, 255, 255));
        }
        if (sel) {                                         /* what turning the other knobs will give it: the sticker and the colour */
            uint16_t pc = rgb24(BOX_COL[P[P_COLOR] & 7]);
            fill_rbox(x + 30, y + 40 + bob, 16, 16, 4, pc);
            sticker(x + 38, y + 48 + bob, 5, P[P_STICK], RGB(255, 255, 255), pc);
        }
    }
    text_c(120, 176, &FONT_B, N_ACT[P[P_ACT] % 3], RGB(255, 240, 120));
}

/* the things the music adds on top: the song, the loop, the beat, the arpeggio */
static void overlays_draw(void)
{
    int i;
    if (ui.page == PG_SAVE && plat_ms() - ui.touch_ms < 12000u)
        treasure_draw();
    if (mel_playing) {                                   /* song progress and, in FOLLOW, the hint */
        int n = mel_len ? mel_len : 1, w = 150 * mel_pos / n;
        fill_rbox(45, 6, 150, 7, 3, RGB(30, 40, 60));
        if (w > 4)
            fill_rbox(45, 6, w, 7, 3, RGB(120, 255, 140));
        fill_circle(45 + w, 9, 5, RGB(255, 255, 255));
        if (P[P_MMODE] == 1)
            text_out(120, 16, &FONT_B, "PRESS THE LIGHT!", RGB(255, 255, 120), RGB(40, 30, 0));
        else
            text_out(120, 16, &FONT_S, melody_name(mel_idx), RGB(255, 255, 255), RGB(20, 30, 60));
    }
    if (sc.rec == REC_ARMED || sc.rec == REC_RECORDING) {
        int blink = (plat_ms() / 300u) & 1u;
        fill_circle(16, 16, blink ? 9 : 6, RGB(255, 40, 40));
        text_out(48, 9, &FONT_B, sc.rec == REC_ARMED ? "PLAY!" : "REC", RGB(255, 255, 255), RGB(120, 0, 0));
        if (sc.rec == REC_RECORDING)
            box_(0, 0, rec_phase * 240 / 255, 4, RGB(255, 60, 60));
    } else if (sc.rec == REC_LOOPING) {
        int px = rec_phase * 240 / 255;
        box_(0, 0, 240, 4, RGB(20, 60, 30));
        box_(0, 0, px, 4, RGB(90, 255, 120));
        fill_circle(px, 2, 5, RGB(255, 255, 255));
    }
    if (P[P_BEAT]) {                                     /* the drum pads: they light as they hit */
        static const uint16_t DC[4] = {RGB(255, 90, 70), RGB(255, 200, 60), RGB(90, 220, 255), RGB(220, 120, 255)};
        for (i = 0; i < 4; i++) {
            int r = 4 + (int)P[P_TUNING] / 40 + (ui.drum_f[i] >> 5);
            fill_circle(214, 24 + i * 18 - 2, r, blend(RGB(40, 44, 70), DC[i], 4 + ui.drum_f[i] * 12 / 255));
        }
    }
    if (P[P_ARP] && sc.arp) {
        int a = (int)(plat_ms() / 4u);
        for (i = 0; i < 3; i++)
            fill_circle(28 + icos(a + i * 85) * 12 / 127, 40 + isin(a + i * 85) * 12 / 127, 3, RGB(255, 170 + i * 30, 90));
    }
    if (ui.toast_until && plat_ms() < ui.toast_until) {   /* the toast: a chip near the top */
        uint32_t age = plat_ms() - ui.toast_t0, left = ui.toast_until - plat_ms();
        int w = imax(text_w(&FONT_B, ui.toast) * 2 + 24, ui.toast2[0] ? text_w(&FONT_S, ui.toast2) + 24 : 0), h = ui.toast2[0] ? 60 : 40;
        int y = 6 - (left < 250u ? (int)(250u - left) / 4 : 0) - (age < 120u ? (int)(120u - age) / 2 : 0);
        w = imin(w, 232);
        if (left < 250u || age < 100u)
            y += 0;
        fill_rbox(120 - w / 2, y, w, h, 12, RGB(10, 12, 30));
        fill_rbox(120 - w / 2 + 2, y + 2, w - 4, h - 4, 10, RGB(250, 250, 255));
        {
            uint16_t c = blend(ui.toast_col, RGB(0, 0, 40), 7);
            text_big(120, y + 5, &FONT_B, ui.toast, c, 2);
            if (ui.toast2[0])
                text_c(120, y + 40, &FONT_S, ui.toast2, RGB(60, 60, 90));
        }
    }
}

static int iris_radius(void)                              /* -1: no transition */
{
    uint32_t t = plat_ms(), el;
    if (!ui.trans_t0)
        return -1;
    el = t - ui.trans_t0;
    if (el > 560u) {
        ui.trans_t0 = 0;
        return -1;
    }
    return el < 280u ? (int)(180 - el * 180 / 280u) : (int)((el - 280u) * 190 / 280u);
}
static void iris_draw(int r)
{
    int y, cx = 120, cy = 100;
    uint16_t sc2 = swap16(C_BLACK);
    for (y = 0; y < 240; y++) {
        int yy = y + cv_oy, w;
        if (yy < cv_y0 || yy >= cv_y1)
            continue;
        if (iabs(y - cy) >= r) {
            hspan(0, 239, y, sc2);
            continue;
        }
        w = (int)fm_sqrtf((float)(r * r - (y - cy) * (y - cy)));
        hspan(0, cx - w - 1, y, sc2);
        hspan(cx + w + 1, 239, y, sc2);
    }
}

/* ---------------------------------------------------------------- post effects --- */
static void post_fx(void)
{
    int r, x, y, b;
    if (P[P_WIGGLE] > 4 || P[P_MOOD] == MOOD_CRAZY) {      /* the jelly: every row slides */
        int amp = (int)P[P_WIGGLE] * 9 / 127 + (P[P_MOOD] == MOOD_CRAZY ? 3 : 0);
        for (r = 0; r < (int)cv_h; r++) {
            int d = (isin(((r - cv_oy) * 5) + (int)(sc.t / 9)) * amp) >> 7;
            uint16_t *row = cv_px + (uint32_t)r * 240u;
            if (!d)
                continue;
            if (d > 0) {
                for (x = 239; x >= d; x--)
                    row[x] = row[x - d];
            } else {
                for (x = 0; x < 240 + d; x++)
                    row[x] = row[x - d];
            }
        }
    }
    if (sc.shake > 0) {                                  /* a stomp, a roar: the whole picture jumps */
        int d = sc.shake_dx;
        for (r = 0; r < (int)cv_h && d; r++) {
            uint16_t *row = cv_px + (uint32_t)r * 240u;
            if (d > 0) {
                for (x = 239; x >= d; x--)
                    row[x] = row[x - d];
            } else {
                for (x = 0; x < 240 + d; x++)
                    row[x] = row[x - d];
            }
        }
    }
    if (sc.robot > 5) {                                  /* the robot: the world turns to big square pixels */
        static const uint8_t BS[8] = {1, 2, 3, 4, 5, 6, 8, 8};
        b = BS[sc.robot * 7 / 127];
        if (b > 1) {
            for (y = 0; y < (int)cv_h; y += b) {
                uint16_t *row = cv_px + (uint32_t)y * 240u;
                for (x = 0; x < 240; x += b) {
                    uint16_t p = row[x];
                    int xx, yy;
                    for (xx = 1; xx < b && x + xx < 240; xx++)
                        row[x + xx] = p;
                    for (yy = 1; yy < b && y + yy < (int)cv_h; yy++)
                        for (xx = 0; xx < b && x + xx < 240; xx++)
                            cv_px[(uint32_t)(y + yy) * 240u + (uint32_t)(x + xx)] = p;
                }
            }
        }
    }
    if (sc.grit > 6) {                                   /* the grit: static */
        int n = sc.grit * sc.grit / 200;
        while (n--) {
            uint32_t rr = urand();
            cv_px[(rr >> 8) % (cv_w * cv_h)] = (rr & 1u) ? 0xFFFFu : 0x0000u;
        }
    }
}

static void draw_frame(void)
{
    int band, irisr = iris_radius();
    for (band = 0; band < 2; band++) {
        cv_px = cv_buf[band];
        cv_begin(240, 120, C_BLACK);
        cv_oy = -120 * band;
        world_draw();
        parts_draw();
        post_fx();
        if (irisr >= 0)
            iris_draw(irisr);
        overlays_draw();
        hud_draw();
        cv_blit(0, (uint32_t)(band * 120));
        if (!band)
            input();                                     /* the keys are read between the bands too: half the latency of a frame */
    }
}

/* ---------------------------------------------------------------------- LEDs --- */
static void leds(void)
{
    uint32_t b = 0, bd = 0, k = 0, kd = 0, kf = 0;
    int i;
    uint32_t t = plat_ms();
    for (i = 0; i < NB; i++)
        if (PAGE_OF_BTN[i] == ui.page)
            b |= 1u << i;
    if (mel_playing)
        b |= 1u << B_PLAY;
    if (P[P_ARP])
        b |= 1u << B_ARP;
    if (P[P_BEAT] && ((t / 250u) & 1u))
        b |= 1u << B_SEQ;
    else if (P[P_BEAT])
        bd |= 1u << B_SEQ;
    if (rec_state == REC_RECORDING || (rec_state == REC_ARMED && ((t / 250u) & 1u)))
        b |= 1u << B_REC;
    else if (rec_state == REC_LOOPING)
        bd |= 1u << B_REC;
    k = ui.held_k;
    if (mel_playing && mel_expect >= 0) {
        if (P[P_MMODE] == 1 && ((t / 200u) & 1u))
            k |= 1u << mel_expect;                         /* FOLLOW: the next key blinks */
        else if (P[P_MMODE] == 1)
            kd |= 1u << mel_expect;
        else
            k |= 1u << mel_expect;
    }
    if (P[P_EASY]) {                                     /* the scale's keys are faintly lit: they all sound right */
        static const uint16_t PENTA = 0x295, MAJOR = 0xAB5;   /* C D E G A; C D E F G A B */
        uint16_t m = P[P_EASY] == 1 ? PENTA : MAJOR;
        for (i = 0; i < NKEYS_L; i++) {
            int n = ((KEY0_NOTE + i + 12 * ((int)P[P_OCT] - 3)) % 12 + 12) % 12;
            if ((m >> n) & 1u)
                kf |= 1u << i;
        }
    }
    plat_glow(1);
    plat_play_red(rec_state == REC_RECORDING);
    plat_buttons_dim(bd & ~b);
    plat_keys_dim(kd & ~k);
    plat_keys_faint(kf & ~k & ~kd);
    plat_leds(b, k);
}

/* ------------------------------------------------------------------- entry --- */
void ui_init(void)
{
    palette_set(0);
    draw_init();
    ui.page = PG_HOME;
    ui.touched = -1;
    ui.hud_y = 240;
    ui.touch_ms = plat_ms();
    sc.world = P[P_PRESET] % NPRESET;
    sc.sz = 100;
    sc.sq = 100;
    ui.trans_to = sc.world;
    ui.trans_done = 1;
    ui.bprev = plat_buttons();
    ui.kprev = plat_keys();
    ui.mel_done_seen = mel_done;
    toast(PRESETS[sc.world].name, PRESETS[sc.world].voice, rgb24(SCENES[sc.world].accent));
}

void ui_frame(void)
{
    int i;
    uint32_t t = plat_ms();
    input();
    world_events();
    sc_update();
    parts_step();
    for (i = 0; i < 4; i++)
        if (ui.pulse[i] > 0)
            ui.pulse[i]--;
    if (ui.save_pop > 0) {                               /* a box was filled or opened: a shower of stars */
        int n;
        for (n = 0; n < 2; n++)
            part_add(PK_STAR, rnd_range(10, 230), rnd_range(0, 40), rnd_range(-8, 8), rnd_range(12, 28), 40, rnd_range(3, 7),
                     RGB(255, rnd_range(180, 255), rnd_range(60, 200)), 0);
        ui.save_pop--;
    }
    {   /* the HUD slides up when the panel is touched, and down after a while */
        int want = (t - ui.touch_ms < 4500u) ? 240 - HUD_H - 2 : 240;
        ui.hud_y += (want - ui.hud_y) / 3 + (want > ui.hud_y ? 1 : -1) * (iabs(want - ui.hud_y) > 0);
        if (iabs(want - ui.hud_y) < 2)
            ui.hud_y = want;
    }
    draw_frame();
    leds();
    ui.frame++;
}
