#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# LILA in the browser: the host simulator compiled to WebAssembly (Emscripten), plus its page.
#   web/emu/build.sh  ->  build/emu/{index.html, worklet.js, lila.wasm}
# Same sources and flags as host/build_host.sh (-ffp-contract=off, like the device).
# Needs emcc: source ~/emsdk/emsdk_env.sh
set -e
cd "$(dirname "$0")/../.."
mkdir -p build/gen build/emu
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
python3 tools/gen_stories.py build/gen/story_data.h >/dev/null
emcc -O2 -ffp-contract=off -std=gnu99 -Wall -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
    -DX0X_HOST -DLILA_WEB -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    --no-entry -sSTANDALONE_WASM -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=33554432 -sFILESYSTEM=0 \
    -o build/emu/lila.wasm web/emu/lila_web.c firmware/src/lila/synth.c firmware/src/lila/data.c
cp web/emu/index.html web/emu/worklet.js build/emu/
echo "emu: build/emu ($(wc -c < build/emu/lila.wasm) B wasm)"
