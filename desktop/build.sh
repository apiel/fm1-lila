#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# LILA, native: the host simulator in a window with sound (SFML 2.x), on the desktop.
#   desktop/build.sh  ->  build/desktop/lila      run it: build/desktop/lila [--reset] [--data DIR]
# Same sources and flags as host/build_host.sh (-ffp-contract=off, like the device). Needs libsfml-dev.
set -e
cd "$(dirname "$0")/.."
ROOT=$(pwd)
CC="${CC:-cc}"
mkdir -p build/gen build/desktop
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
U="firmware/src/lila/synth.c firmware/src/lila/data.c"
FLAGS="-O2 -ffp-contract=off -std=gnu99 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -DX0X_HOST -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen"
# shellcheck disable=SC2086
$CC $FLAGS -c desktop/lila_core.c -o build/desktop/lila_core.o
OBJS="build/desktop/lila_core.o"
for f in $U; do
    o="build/desktop/$(basename "$f" .c).o"
    # shellcheck disable=SC2086
    $CC $FLAGS -c "$f" -o "$o"
    OBJS="$OBJS $o"
done
# shellcheck disable=SC2046
c++ -O2 -std=c++17 -Wall -Wextra -DLILA_FONT="\"$ROOT/assets/fonts/BarlowSemiCondensed-SemiBold.ttf\"" \
    desktop/lila_desktop.cpp $OBJS \
    $(pkg-config --cflags --libs sfml-graphics sfml-audio sfml-window sfml-system) -lm -o build/desktop/lila
echo "desktop: build/desktop/lila"
