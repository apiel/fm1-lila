#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Random buttons, knobs, keys and even out-of-range parameter values, for a few minutes of simulated time, with the address and
# undefined-behaviour sanitizers on: the app must never read or write out of bounds. (The fastmath shift is left alone: it is
# the device's arithmetic shift of a negative exponent.)
set -e
cd "$(dirname "$0")/.."
mkdir -p build/host build/scen
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
python3 tools/gen_stories.py build/gen/story_data.h >/dev/null
cc -O1 -g -fsanitize=address,undefined -fno-sanitize=shift -fno-sanitize-recover=undefined -ffp-contract=off -std=gnu99 -DX0X_HOST \
    -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o build/host/lila_fuzz host/lila_host.c firmware/src/lila/synth.c firmware/src/lila/data.c -lm
for seed in 1 2 3 4 5 6; do
python3 - "$seed" <<'PY'
import random, sys
random.seed(int(sys.argv[1]) * 17)
btn = "FX SEL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCT- OCT+".split()
enc = "SELECT ALGO PRESET K1 K2 K3 K4".split()
names = ("ECHO SPACE FLUFF ROBOT CUT RESO FTYPE GRIT ATK PLUCK REL BOING LSPD LDEP LWHAT LWAVE WAVE BUDDY BIG METAL BEND STARS SIZE WIGGLE "
         "TEMPO KEY EASY FRIENDS ARP ARPSPD ARPRNG STICKY BEAT BEATVOL BOUNCE TUNING MSPEED MMODE MREPEAT MBAND RLEN RVOL RSPEED RFUNNY "
         "BOX ACT STICK COLOR MOOD PRESET MELODY OCT").split()
out = ["wait 400"]
for i in range(2200):
    r = random.random()
    if r < 0.14: out.append("tap %s" % random.choice(btn))
    elif r < 0.34: out.append("turn %s %d" % (random.choice(enc), random.choice([-6, -3, -2, -1, 1, 2, 3, 7, 20])))
    elif r < 0.46: out.append("set %s %d" % (random.choice(names), random.choice([0, 1, 2, 3, 5, 12, 24, 63, 64, 100, 127, 128, 200, 255])))
    elif r < 0.68: out.append("key %d down" % random.randrange(27))
    elif r < 0.86: out.append("key %d up" % random.randrange(27))
    elif r < 0.9: out.append("master %d" % random.randrange(0, 4097))
    else: out.append("wait %d" % random.choice([10, 50, 200, 700, 1500]))
    if random.random() < 0.3: out.append("wait %d" % random.choice([20, 60, 120]))
open("build/scen/fuzz.lila", "w").write("\n".join(out) + "\nwait 2000\n")
PY
build/host/lila_fuzz build/scen/fuzz.lila build/scen
done
echo "fuzz: clean"
