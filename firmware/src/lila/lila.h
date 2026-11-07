/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA: a polyphonic toy synth for a young child, on the M-VAVE FM-1.
 *
 * Twelve worlds (presets): every one has its own instrument and its own animated world. Every button of the
 * panel has exactly one page and the four knobs always do something. The state is a flat array of small
 * integers (P[]): the UI (main loop) writes it, the audio ISR (synth.c) reads it. Keys, buttons that act and
 * melody changes travel through a small event queue; what the screen needs to dance to comes back through
 * the vis_* variables and the note ring. */
#pragma once
#include <stdint.h>

#define NVOICE 10
#define NPRESET 12
#define NMEL 12
#define NKEYS_L 27                     /* F3 .. G5, as the panel's keys */
#define KEY0_NOTE 53                   /* the lowest key plays F3 (MIDI 53) */

/* ------------------------------------------------------------ parameters --- */
/* 0..127 unless the page says otherwise (switches count from 0). The first NSND belong to the sound: a
 * preset loads them; the rest are the player's settings and survive a preset change. */
enum {
    P_ECHO, P_SPACE, P_FLUFF, P_ROBOT,            /* FX   */
    P_CUT, P_RESO, P_FTYPE, P_GRIT,               /* SEL  (the filter) */
    P_ATK, P_PLUCK, P_REL, P_BOING,               /* ENV  (and what it does to the filter) */
    P_LSPD, P_LDEP, P_LWHAT, P_LWAVE,             /* LFO  */
    P_WAVE, P_BUDDY, P_BIG, P_METAL,              /* EDIT */
    P_BEND, P_STARS, P_SIZE, P_WIGGLE,            /* the magic: universal fun */
    /* what makes a world its own: the pages show some of these, under the world's own names (ui.c, WORLD_KNOBS) */
    P_CTYPE, P_CAMT, P_CTIME,                     /* a note's pitch contour (CT_*), how far it goes, how fast */
    P_VIBD, P_VIBS, P_VIBDLY,                     /* vibrato: depth, speed, how late it sets in (a howl, a whale) */
    P_AMR, P_AMD,                                 /* a pulsing volume: rate and depth (a croak, a growl) */
    P_DUTY, P_PWMD,                               /* pulse width and how much it sweeps (arcade) */
    P_CHIP, P_CHIPS,                              /* a fast arpeggio inside every note (arcade): chord and speed */
    P_VOWEL, P_FORM,                              /* a throat: which vowel, and how much of it (a moo, a roar, a robot) */
    P_NOISE, P_NZDEC,                             /* breath, a crash: how much, how long */
    P_STOMP, P_BELL,                              /* a thump and a cowbell with every note */
    P_CHAT, P_CHATR,                              /* a chatter of little blips after a note: how many, how far apart in pitch */
    P_PING, P_PINGT,                              /* the same note again and again, softer (sonar): how often, how far apart */
    P_FMR,                                        /* the FM partner's ratio */
    NSND,
    P_TEMPO = NSND, P_KEY, P_EASY, P_FRIENDS,     /* GLO  */
    P_ARP, P_ARPSPD, P_ARPRNG, P_STICKY,          /* ARP  */
    P_BEAT, P_BEATVOL, P_BOUNCE, P_TUNING,        /* SEQ  */
    P_MSPEED, P_MMODE, P_MREPEAT, P_MBAND,        /* PLAY */
    P_RLEN, P_RVOL, P_RSPEED, P_RFUNNY,           /* REC  */
    P_BOX, P_ACT, P_STICK, P_COLOR,               /* SAVE */
    P_MOOD, P_PRESET, P_MELODY, P_OCT,            /* ALGO, PRESETS, SELECT, OCT +/- */
    NP
};
extern volatile uint8_t P[NP];

#define BPM_OF(v) (50 + (int)(v))                  /* P_TEMPO 0..127 -> 50 .. 177 beats a minute */

enum { CT_NONE, CT_HOWL, CT_WAIL, CT_HILO, CT_ALARM, CT_DIVE, CT_SWOOP, CT_COIN, CT_RIBBIT, NCT };
enum { LW_SINE, LW_TRI, LW_SQUARE, LW_BUMPY };
enum { LT_PITCH, LT_WAH, LT_TREM };
enum { MOOD_DAY, MOOD_NIGHT, MOOD_MAGIC, MOOD_CRAZY, NMOOD };

/* --------------------------------------------------------------- presets --- */
/* a world's sound: the parameters that differ from neutral (preset_load starts from neutral), and its own trims */
typedef struct { uint8_t id, v; } pv_t;
enum { BL_NONE, BL_BEEP, BL_BUBBLE, BL_CHIRP, BL_TWINKLE, NBLIP };      /* what the chatter's blips are */
typedef struct {
    const char *name;                 /* the world's name, <= 8 characters */
    const char *voice;                /* what it sounds like: "TWEET", "MOO" */
    const pv_t *set;                  /* the sound: parameters that are not neutral */
    uint8_t nset;
    int8_t oct;                       /* the instrument's own octave shift */
    uint8_t vol;                      /* level trim, 100 = 1.0 */
    uint8_t blip;                     /* BL_*: the chatter's blips */
} preset_t;
extern const preset_t PRESETS[NPRESET];

/* melodies: text in data.c, parsed by the engine into note / length tables */
typedef struct { const char *name; const char *notes; } melody_t;
extern const melody_t MELODIES[NMEL];

/* the beat patterns: 16 steps of kick, snare, hat, clap as bit masks (bit 0 = first step) */
#define NBEAT 8
typedef struct { const char *name; uint16_t kick, snare, hat, clap; } beat_t;
extern const beat_t BEATS[NBEAT];

/* ---------------------------------------------------------- UI -> audio --- */
enum { EV_KEY_ON, EV_KEY_OFF, EV_PLAY, EV_STOP, EV_MELODY, EV_REC, EV_PANIC, EV_ARP_CLEAR, EV_BEAT_RESET };
void ev_post(int type, int a, int b);                 /* main loop */

/* ---------------------------------------------------------- audio -> UI --- */
enum { VE_NOTE, VE_MELODY_NOTE, VE_ARP, VE_LOOP, VE_DRUM, VE_BLIP };   /* a blip: src is what made it (0x40 fairy dust, 0x42 cowbell, 0x43 chatter, 0x44 ping) */
typedef struct { uint8_t type, note, vel, src; } vev_t;
int vis_pop(vev_t *e);                                /* the next note that sounded, 1 = got one */
extern volatile uint8_t vis_level;                    /* the loudest voice's envelope * velocity, 0..255 */
extern volatile int8_t vis_lfo;                       /* the LFO now, -127..127 */
extern volatile uint8_t vis_peak;                     /* the output's level, 0..255, decays */
extern volatile uint8_t vis_voices;                   /* voices sounding */
extern volatile uint32_t vis_beat_n;                  /* the beat's 16th counter */
extern volatile uint8_t vis_beat_hit;                 /* bits: 1 kick 2 snare 4 hat 8 clap, set by the beat, cleared by the UI */
extern volatile uint8_t mel_playing, mel_idx;
extern volatile uint8_t mel_pos, mel_len;             /* note index and the number of notes of the melody */
extern volatile int8_t mel_expect;                    /* follow-me: the key (0..26) to press next, -1 none */
extern volatile uint8_t mel_done;                     /* counts melody ends (the UI cheers) */
extern volatile uint8_t rec_state, rec_phase;         /* REC_*, and where the loop is, 0..255 */
enum { REC_IDLE, REC_ARMED, REC_RECORDING, REC_LOOPING };
extern volatile uint8_t arp_notes;                    /* notes the arpeggiator holds */
extern volatile uint8_t arp_cur;                      /* the arpeggiator's note now (0 = none) */

/* ---------------------------------------------------------------- engine --- */
void engine_init(void);
void engine_load(uint32_t pct, uint32_t frames);
void engine_render(int32_t *out_lr, uint32_t n);      /* ISR: n stereo frames (n a multiple of 8), 24-bit in int32 */
void engine_set_master(uint32_t q12);                 /* the MASTER pot, 0 .. 4096 */
extern volatile uint32_t eng_load_pct;
const char *melody_name(int i);
int melody_count(void);
int melody_notes(int i, const uint8_t **notes, const uint8_t **lens);   /* the parsed tables */

/* a sound the way the UI wants it: load a preset's sound into P[] */
void preset_load(int idx);
