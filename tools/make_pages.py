#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""make_pages.py OUTDIR: the GitHub Pages site: a landing page, the manual and the browser emulator (build/emu, from web/emu/build.sh).
Needs `pip install markdown`."""
import shutil
import sys
from pathlib import Path

import markdown

ROOT = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1] if len(sys.argv) > 1 else ROOT / "build" / "pages")
if out.exists():
    shutil.rmtree(out)
(out / "manual").mkdir(parents=True)
shutil.copytree(ROOT / "build" / "emu", out / "emulator")
shutil.copytree(ROOT / "docs" / "img", out / "img")

CSS = """body{margin:0;background:#14122e;color:#f3f0ff;font:17px/1.55 system-ui,sans-serif}
main{max-width:860px;margin:0 auto;padding:24px 16px 64px}a{color:#ffd040}h1,h2,h3{line-height:1.2}
h1{font-size:2.4em;margin:.2em 0}img{max-width:100%;height:auto;image-rendering:pixelated;border-radius:8px}
table{border-collapse:collapse;display:block;overflow-x:auto}td,th{border:1px solid #3a3670;padding:6px 9px;text-align:left;vertical-align:top}
th{background:#241f52}code{background:#241f52;padding:1px 5px;border-radius:4px}
.go{display:inline-block;background:#ffd040;color:#14122e;padding:12px 22px;border-radius:12px;font-weight:700;text-decoration:none;margin:6px 8px 6px 0}"""

def page(title, body, back=""):
    return (f'<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
            f'<title>{title}</title><style>{CSS}</style></head><body><main>{back}{body}</main></body></html>')

manual = markdown.markdown((ROOT / "docs" / "MANUAL.md").read_text(), extensions=["tables"])
(out / "manual" / "index.html").write_text(page("LILA: the manual", manual, '<p><a href="../">&larr; LILA</a></p>'))

landing = """<p><img src="img/splash.png" width="240" alt="LILA"></p>
<h1>LILA</h1>
<p>A polyphonic toy synth for a very young musician, as custom firmware for the M-VAVE FM-1. Twelve animated worlds: a bird, a cow, a dog, a frog,
a spaceship, meteors, a block world, dinosaurs, a robot, the ocean, a fairy castle and the jungle. Each is its own instrument, and whatever knob is
turned, the picture changes with it.</p>
<p><a class="go" href="emulator/">Try it in the browser</a><a class="go" href="manual/">Manual</a></p>
<p><img src="img/worlds.png" alt="The twelve worlds"></p>
<p>The emulator runs the same code as the FM-1 (compiled to WebAssembly). Source and firmware downloads: see the GitHub repository's releases.
Installing firmware is at your own risk. GPL-3.0-only; M-VAVE and FM-1 are trademarks of their owners.</p>"""
(out / "index.html").write_text(page("LILA", landing))
(out / ".nojekyll").write_text("")
print(f"site: {out}")
