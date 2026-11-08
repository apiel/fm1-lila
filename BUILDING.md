# Building LILA

The firmware build makes three files in `build/`:

| File | What |
| --- | --- |
| `x0x.bin` | the firmware app |
| `loader/ota.bin` | the update loader (Felucca's, unchanged) |
| `lila.fwsc` | the installable package (app + loader) |

## Try it without an FM-1

Only a C compiler, Python 3 with Pillow (`pip3 install Pillow`) and, for the window with sound, SFML 2.x (`libsfml-dev`, `pkg-config`).

```
tests/run_tests.sh                       # engine test on every world + the scripted scenarios; screenshots in build/scen/
build/host/lila_host SCRIPT OUTDIR       # one script (the commands are listed at the top of host/lila_host.c)
desktop/build.sh && build/desktop/lila   # the window: click or drag the knobs, the computer's keys play
build/desktop/lila --shot panel.png      # no window: two seconds of the screen to a PNG
build/desktop/lila --reset               # forget the saved boxes
```

In the window: `Q` `W` `E` ... and `J` `K` `L` ... are the white keys, `1`-`0` and `-` the black keys, `A S D F G H` are
FX SEL ENV LFO EDIT GLO, `Z X C V B N` are HOME SAVE ARP SEQ PLAY REC, `Left` / `Right` are OCT- / OCT+, the up and down
arrows turn SELECT. `Shift`-click or right-click a button to latch it.

## The firmware

The build needs (on Linux x86-64 they run natively; elsewhere in Docker, see `tools/build.py`):

- the JieLi toolchain (clang 4.0.1 for pi32v2): `tools/get_toolchain.sh` installs it in `~/.jieli/toolchain`;
- the JieLi AC79 SDK (Apache-2.0), of which the package uses three files (`uboot.boot`, `cfg_tool.bin`, `eq_cfg_hw.bin`):

  ```
  git clone --depth 1 --branch AC79NN_SDK_V1.2.1_2023-12-13 \
      https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK.git ~/fw-AC79_AIoT_SDK
  ```

```
JIELI_TOOLCHAIN=~/.jieli/toolchain AC79_SDK=~/fw-AC79_AIoT_SDK python3 tools/build.py
```

The build checks the image: no soft-double routine linked (a `double` crept in), RAM and pool sizes, register access only in
`firmware/hal/`. Last build: image 122 KB (of 580 KB), RAM 43 KB (of 96 KB), pool 266 KB (of 344 KB).

`python3 tools/build.py --release 1.0-beta` makes `build/lila-1.0-beta.fwsc` with the identity `FM-1_9010000`.

## Install

From the command line (needs `pip3 install mido python-rtmidi`, `tools/flash.sh` makes its own venv):

```
tools/flash.sh --build            # build, and check that the FM-1 answers: writes nothing
tools/flash.sh --yes              # install build/lila.fwsc
python3 tools/fm1_install.py --info
```

Installing firmware is at your own risk. If the FM-1 no longer starts but reaches the chip's update mode (4C4A:8057 on USB),
`tools/fm1_rescue.py` puts the stock firmware back (it needs M-VAVE's FM-1 V15 file; `tools/fm1_rescue.sh` fetches it for macOS).

## How the code is laid out

| | |
|---|---|
| `firmware/hal/`, `firmware/loader/`, `firmware/src/{usb,ota,storage,lcd,gfx,libc,midi_uart}.c`, `firmware/src/app/` | the FM-1's hardware layer, USB, update loader, storage, boot and audio interrupt (Felucca / X0X) |
| `firmware/src/lila/lila.h` | the state (`P[]`, one small integer per knob), the events and the numbers the screen reads from the audio |
| `firmware/src/lila/synth.c` | the audio engine, in the audio interrupt: voices, effects, arpeggiator, melody player, beat, looper |
| `firmware/src/lila/data.c` | the twelve sounds, the twelve melodies, the eight beats |
| `firmware/src/lila/draw.c`, `scenes*.c` | the drawing toolkit (two 240x120 bands, double buffered) and the twelve worlds |
| `firmware/src/lila/ui.c`, `splash.c` | the pages, the knobs, the HUD, the lights, the start-up screen |
| `host/lila_host.c`, `desktop/` | the simulator (scripts, screenshots, WAV) and the window with sound |

To add a world: a sound row in `data.c`, three drawing functions (backdrop, character, foreground) in a `scenes_*.c` file and one row in
the `SCENES` table; raise `NPRESET`. To add a melody: a line in `data.c` (the notation is explained there) and raise `NMEL`.

## The website (GitHub Pages)

`.github/workflows/pages.yml` runs the tests, builds the browser emulator with Emscripten, and publishes a landing page, the manual and
the emulator on every push to `main`. One-time setup: repository **Settings > Pages > Source: GitHub Actions**.

Locally: `source ~/emsdk/emsdk_env.sh && web/emu/build.sh && node tests/web_emu_test.mjs`, then (with `pip install markdown`)
`python3 tools/make_pages.py build/pages` and `cd build/pages && python3 -m http.server 8000`. The web installer
(`install/`) writes the newest `dist/lila-*.fwsc`. The Action does not build the firmware (it needs the JieLi toolchain and SDK), so to publish
a new version: `python3 tools/build.py --release 1.1-beta`, copy `build/lila-1.1-beta.fwsc` to `dist/`, delete the old one and commit.

## The stories

A story is a text, `assets/stories/NAME.txt`: one line per key (up to 27), `SCENE|text`. `tools/story_make.py` speaks each line with Piper,
codes it (`tools/lpc_enc.c`: 12th-order LPC at 11025 Hz, 50 frames of 10 bytes a second, about 0.5 KB a second of speech) and writes
`NAME.lpc`, which is committed; the build turns both into C (`tools/gen_stories.py`). Piper is only needed to remake the speech:

```
python3 -m venv ~/piper && ~/piper/bin/pip install piper-tts
curl -LO https://huggingface.co/rhasspy/piper-voices/resolve/v1.0.0/fr/fr_FR/siwis/medium/fr_FR-siwis-medium.onnx      # and its .onnx.json
python3 tools/story_make.py assets/stories/corbeau_fr.txt fr_FR-siwis-medium.onnx --piper ~/piper/bin/piper
```

`build/t/lpc_enc -d NAME.lpc out.raw` decodes a coding with a plain pulse, to listen to it alone (11025 Hz mono 16-bit).
A new story also needs its row in `tools/gen_stories.py` (STORIES, SCENES), a preset in `data.c` (its `story` field) and a scene in
`firmware/src/lila/scenes_story.c`.
