#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Build the LILA host simulator (the whole app on this machine) into build/host/lila_host.
# Same sources as the firmware, -ffp-contract=off like the device (no fused multiply-add).
set -e
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
mkdir -p build/gen build/host
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
U="firmware/src/lila/synth.c firmware/src/lila/data.c"
# shellcheck disable=SC2086
$CC -O2 -ffp-contract=off -std=gnu99 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers \
    -DX0X_HOST -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen -o ${LILA_HOST_OUT:-build/host/lila_host} host/lila_host.c $U -lm
echo "host: build/host/lila_host"
