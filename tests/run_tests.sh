#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# LILA's tests: the audio engine on every world (no NaN, no clipping, not silent), then the whole app in the host
# simulator driven by scripts (tests/scenarios/*.lila): buttons, knobs, keys, the arpeggiator, the beat, the melodies, the looper.
# The screenshots and the audio of the scripts are in build/scen/.
set -e
cd "$(dirname "$0")/.."
mkdir -p build/t build/scen
sh host/build_host.sh
python3 tools/gen_stories.py build/gen/story_data.h >/dev/null
cc -O2 -ffp-contract=off -std=gnu99 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -DX0X_HOST -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    -o build/t/synth_test tests/synth_test.c firmware/src/lila/synth.c firmware/src/lila/data.c -lm
build/t/synth_test build/t
for s in tests/scenarios/basics.lila tests/scenarios/music.lila tests/scenarios/pages.lila tests/scenarios/worlds.lila tests/scenarios/stories.lila tests/scenarios/screens.lila tests/scenarios/pianos.lila tests/scenarios/fables.lila tests/scenarios/instruments.lila; do
    echo "== $s"
    build/host/lila_host "$s" build/scen
done
echo "all tests passed"
