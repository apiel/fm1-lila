#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# LILA's tests: the audio engine on every world (no NaN, no clipping, not silent), then the whole app in the host
# simulator driven by scripts (tests/scenarios/*.lila): buttons, knobs, keys, the arpeggiator, the beat, the melodies, the looper.
# The screenshots and the audio of the scripts are in build/scen/.
set -e
cd "$(dirname "$0")/.."
mkdir -p build/t build/scen
sh host/build_host.sh
cc -O2 -ffp-contract=off -std=gnu99 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -DX0X_HOST -Ifirmware/src -Ifirmware/src/dsp \
    -o build/t/synth_test tests/synth_test.c firmware/src/lila/synth.c firmware/src/lila/data.c -lm
build/t/synth_test build/t
for s in tests/scenarios/basics.lila tests/scenarios/music.lila tests/scenarios/pages.lila tests/scenarios/worlds.lila; do
    echo "== $s"
    build/host/lila_host "$s" build/scen
done
echo "all tests passed"
