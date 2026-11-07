#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""story_make.py STORY.txt VOICE.onnx [--piper PIPER] [--slow 1.15]: a story's speech, coded for the FM-1.

Each line of STORY.txt (SCENE|text, # comments) is spoken by Piper (a free text-to-speech program, https://github.com/OHF-Voice/piper1-gpl),
resampled to 11025 Hz with sox, trimmed, and LPC-coded by tools/lpc_enc.c. The frames of all the lines go into STORY.lpc next to the text:

  "LPC1", u16 parts, then for each part: u32 first frame, u16 frames; then the 10-byte frames.

The .lpc files are committed (the build does not need Piper); tools/gen_stories.py turns them and the texts into C at build time.
The voices used: en_US-ljspeech-high (LJ Speech, public domain), fr_FR-siwis-medium (SIWIS, CC BY 4.0)."""
import argparse
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def lines_of(path):
    out = []
    for ln in Path(path).read_text(encoding="utf-8").splitlines():
        if not ln.strip() or ln.startswith("#"):
            continue
        scene, text = ln.split("|", 1)
        out.append((scene.strip(), text.strip()))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("story")
    ap.add_argument("voice")
    ap.add_argument("--piper", default="piper")
    ap.add_argument("--slow", type=float, default=1.15, help="Piper's length scale: > 1 speaks more slowly")
    a = ap.parse_args()
    enc = ROOT / "build" / "t" / "lpc_enc"
    enc.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["cc", "-O2", "-o", str(enc), str(ROOT / "tools" / "lpc_enc.c"), "-lm"], check=True)
    parts = lines_of(a.story)
    frames = []
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        for i, (_, text) in enumerate(parts):
            wav, raw, lpc = td / f"{i}.wav", td / f"{i}.raw", td / f"{i}.lpc"
            subprocess.run([a.piper, "-m", a.voice, "-f", str(wav), "--length-scale", str(a.slow), "--sentence-silence", "0.25"],
                           input=text.encode("utf-8"), check=True, capture_output=True)
            # 11025 Hz mono, a high pass, 4 dB of headroom, the silence at both ends cut down to 0.1 s
            subprocess.run(["sox", str(wav), "-r", "11025", "-c", "1", "-b", "16", "-e", "signed", str(raw),
                            "highpass", "70", "gain", "-4", "silence", "1", "0.02", "0.5%", "reverse", "silence", "1", "0.02", "0.5%",
                            "reverse", "pad", "0.05", "0.15"], check=True)
            subprocess.run([str(enc), str(raw), str(lpc)], check=True, capture_output=True)
            frames.append(lpc.read_bytes())
            print(f"  {i + 1:2d}/{len(parts)} {len(frames[-1]) // 10 / 50:4.1f} s  {text[:60]}")
    head = b"LPC1" + struct.pack("<H", len(parts))
    pos, table = 0, b""
    for fr in frames:
        n = len(fr) // 10
        table += struct.pack("<IH", pos, n)
        pos += n
    out = Path(a.story).with_suffix(".lpc")
    out.write_bytes(head + table + b"".join(frames))
    print(f"{out}: {len(parts)} parts, {pos / 50:.0f} s of speech, {out.stat().st_size} bytes")


if __name__ == "__main__":
    sys.exit(main())
