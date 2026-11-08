#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""make_video.py [OUT.mp4]: a demo video of LILA with English captions, made from the host simulator (no camera needed).

The script below drives the simulated FM-1 (host/lila_host.c) through a tour: it records the screen (`video`) and the audio (`wav`) on the
device's own clock and marks the moments the captions change (`mark`); ffmpeg then puts the 240x240 screen on a 1280x720 canvas, burns
the captions in and adds the sound. Needs ffmpeg and a built simulator (sh host/build_host.sh); the font is DejaVu Sans Bold.
It shows the screen and the sound, not the FM-1 itself: for that, film the real device with a phone."""
import subprocess
import sys
import textwrap
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "build" / "video" / "lila-demo.mp4"
WORK = ROOT / "build" / "video"
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

S, CAPS = [], []                                    # script lines; (text) per caption, started by mark cN, ended by mark eN


def cap(text):
    S.append(f"mark c{len(CAPS)}")
    CAPS.append(text)


def end():
    S.append(f"mark e{len(CAPS) - 1}")


def wait(ms):
    S.append(f"wait {ms}")


def play(k, ms, gap=50):
    S.extend([f"key {k} down", f"wait {ms}", f"key {k} up", f"wait {gap}"])


def phrase(keys, ms=280):
    for k in keys:
        play(k, ms)


def chord(keys, ms):
    S.extend(f"key {k} down" for k in keys)
    wait(ms)
    S.extend(f"key {k} up" for k in keys)
    wait(100)


def world():                                         # the next world
    S.extend(["turn PRESET 1", "wait 650"])


def both_buttons():                                  # SAVE + GLO together
    S.extend(["press SAVE", "wait 40", "press GLO", "wait 60", "release SAVE", "release GLO", "wait 200"])


# ------------------------------------------------------------------------------------------------ the tour
S.extend(["master 4096", "wav audio.wav", "video screen.rgb", "mark v0", "splash"])
cap("A toy synth for a very young musician.\nCustom firmware for the M-VAVE FM-1.")
wait(3200)
end()

cap("25 worlds. Each one is an instrument and an animated picture.")
phrase([7, 7, 14, 14, 16, 16], 300)
play(14, 700)
phrase([12, 12, 11, 11, 9, 9], 300)
play(7, 900)
end()

cap("Turn any knob and the picture changes with it: echo leaves ghosts, the robot knob makes big pixels.")
S.extend(["tap FX", "wait 300", "turn K1 14", "wait 200"])
phrase([7, 11, 14, 19], 380)
S.extend(["turn K4 22", "wait 200"])
phrase([19, 14, 11], 380)
S.extend(["turn K4 -22", "turn K1 -14", "wait 300"])
end()

cap("A cow, a wolf, a frog, a spaceship, meteors, a block world, dinosaurs, a robot, the ocean, a fairy castle and the jungle.")
world(); phrase([7, 11, 14], 330); wait(300)                    # cow
world(); play(7, 2800); wait(200)                               # wolf
world(); phrase([11, 11, 14, 11], 300); wait(200)               # frog
world(); chord([7, 14], 2000); wait(200)                        # space
world(); phrase([14, 19, 14, 12], 250); wait(200)               # meteor
world(); phrase([7, 11, 14, 19, 14, 11], 200); wait(200)        # blocks
world(); play(7, 2000); wait(200)                               # dino
world(); phrase([7, 14, 12, 19], 280); wait(200)                # robot
world(); play(11, 2400); wait(200)                              # ocean
world(); phrase([14, 16, 19, 21], 320); wait(200)               # castle
world(); play(9, 2000); wait(200)                               # jungle
end()

INSTR = [("Piano", lambda: (chord([7, 11, 14], 1100), phrase([14, 16, 19], 320))),
         ("Electric piano", lambda: (chord([9, 12, 16], 1100), phrase([16, 14, 12], 320))),
         ("Organ", lambda: chord([7, 14, 19], 2000)),
         ("Xylophone", lambda: phrase([7, 9, 11, 12, 14, 16, 18, 19], 200)),
         ("Harp", lambda: phrase([7, 11, 14, 19, 23, 19, 14, 11], 180)),
         ("Ukulele: every key strums a chord", lambda: (play(7, 900), play(12, 900), play(14, 1000))),
         ("Flute", lambda: (play(14, 800), play(16, 700), play(19, 1300))),
         ("Kalimba", lambda: phrase([11, 14, 16, 14, 19, 16, 14, 11], 230))]
for name, act in INSTR:
    cap(f"Eight instruments where the keys light up: {name}")
    world()
    act()
    wait(250)
    end()

cap("A drum beat to play along to, an arpeggiator, a looper, and nineteen songs.")
S.extend(["tap SEQ", "wait 300"])
phrase([11, 14, 16, 19, 16, 14, 11, 9, 11, 14], 300)
S.extend(["tap SEQ", "wait 300", "tap PLAY", "wait 3500", "tap PLAY", "wait 400"])
end()

cap("For a child too young for a screen: SAVE and GLO together show only a name, then turn the screen off.")
both_buttons(); wait(1500)
phrase([11, 14, 16], 400); wait(700)
both_buttons(); wait(1500)
phrase([16, 14, 11], 400); wait(900)
both_buttons(); wait(900)
end()

cap("Five stories, in French: three fables of La Fontaine, Goldilocks and Little Red Riding Hood. Each key tells one part.")
world()                                                         # the first fable
S.append("wait 300")
S.extend(["key w3 down", "wait 5600", "key w3 up", "wait 200"])
S.extend(["key w5 down", "wait 6200", "key w5 up", "wait 300"])
for _ in range(4):
    S.extend(["turn PRESET 1", "wait 450"])
S.extend(["key w8 down", "wait 5200", "key w8 up", "wait 200"])
S.extend(["key w9 down", "wait 5000", "key w9 up", "wait 300"])
end()

cap("Free and open source (GPL-3.0). Made for a very young musician.")
S.extend(["key w11 down", "wait 5600", "key w11 up", "wait 500"])
end()
S.extend(["videostop", "wavstop"])


# ------------------------------------------------------------------------------------------------ run and assemble
def main():
    WORK.mkdir(parents=True, exist_ok=True)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    script = WORK / "demo.lila"
    script.write_text("\n".join(S) + "\n")
    host = ROOT / "build" / "host" / "lila_host"
    if not host.exists():
        sys.exit("build the simulator first: sh host/build_host.sh")
    r = subprocess.run([str(host), str(script), str(WORK)], capture_output=True, text=True, check=True)
    marks = {}
    for ln in r.stdout.splitlines():
        if ln.startswith("MARK "):
            _, name, ms = ln.split()
            marks[name] = int(ms)
    t0 = marks["v0"] / 1000.0
    dur = (WORK / "screen.rgb").stat().st_size / (240 * 240 * 3) / 30.0
    print(f"{len(CAPS)} captions, {dur:.1f} s of video")
    chain = ["[0:v]scale=720:720:flags=neighbor,pad=1280:720:30:0:color=0x14122e"]
    for text, args in (("LILA", "x=790:y=40:fontsize=110:fontcolor=0xffd040"),
                       ("toy synth firmware for the M-VAVE FM-1", "x=794:y=170:fontsize=20:fontcolor=0xb8b2e8"),
                       ("GPL-3.0: free and open source", "x=794:y=670:fontsize=20:fontcolor=0x7d78b8")):
        f = WORK / f"static{abs(hash(text)) % 10000}.txt"
        f.write_text(text)
        chain.append(f"drawtext=fontfile={FONT}:textfile={f}:{args}")
    for i, text in enumerate(CAPS):
        s, e = marks[f"c{i}"] / 1000.0 - t0, marks[f"e{i}"] / 1000.0 - t0
        f = WORK / f"cap{i}.txt"
        f.write_text("\n".join(textwrap.wrap(text.replace("\n", " "), 20)))
        chain.append(f"drawtext=fontfile={FONT}:textfile={f}:x=790:y=260:fontsize=36:line_spacing=12:fontcolor=white:"
                     f"box=0:enable='between(t,{s:.3f},{e:.3f})'")
    vf = ",".join(chain)
    cmd = ["ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pixel_format", "rgb24", "-video_size", "240x240", "-framerate", "30",
           "-i", str(WORK / "screen.rgb"), "-i", str(WORK / "audio.wav"), "-filter_complex", vf + "[v]",
           "-af", "loudnorm=I=-16:TP=-1.5,afade=t=in:d=0.3,afade=t=out:st=%.2f:d=0.8" % max(0, dur - 0.8), "-map", "[v]", "-map", "1:a",
           "-c:v", "libx264", "-pix_fmt", "yuv420p", "-crf", "18", "-c:a", "aac", "-b:a", "192k", "-shortest", "-movflags", "+faststart", str(OUT)]
    subprocess.run(cmd, check=True)
    print(f"video: {OUT} ({OUT.stat().st_size // 1024} KB)")


if __name__ == "__main__":
    main()
