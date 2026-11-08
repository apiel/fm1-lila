#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""make_pages.py OUTDIR: the GitHub Pages site: a landing page, the manual, the browser emulator (build/emu, from web/emu/build.sh) and
the web installer (web/lila_installer.html with fm1pkg.js, fm1ota.js and the package's identity inlined; Chrome or Edge, Web MIDI).
The firmware package is the newest dist/lila-*.fwsc (a release build: python3 tools/build.py --release X.Y-beta, copied to dist/ and
committed, because the toolchain that builds it is not on GitHub's runners). Needs `pip install markdown`."""
import json
import os
import re
import shutil
import sys
from pathlib import Path

import markdown

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "web"))
from make_site import product_of, strip_module  # noqa: E402  (Felucca's: the package format)
out = Path(sys.argv[1] if len(sys.argv) > 1 else ROOT / "build" / "pages")
if out.exists():
    shutil.rmtree(out)
(out / "manual").mkdir(parents=True)
shutil.copytree(ROOT / "build" / "emu", out / "emulator")
shutil.copytree(ROOT / "docs" / "img", out / "img")


def installer():
    pkgs = sorted((ROOT / "dist").glob("lila-*.fwsc"), key=lambda p: [int(n) for n in re.findall(r"\d+", p.name)])
    if not pkgs:
        print("no dist/lila-*.fwsc: the site has no installer")
        return None
    pkg = pkgs[-1]
    raw = pkg.read_bytes()
    product = product_of(raw)
    if not re.fullmatch(r"FM-1_9\d{2,}", product) or b"FELUCCA-LOADER-1" not in raw:
        raise SystemExit(f"{pkg}: not a LILA package (identity {product!r})")
    version = pkg.stem[len("lila-"):]
    page = (ROOT / "web" / "lila_installer.html").read_text(encoding="utf-8")
    lib = strip_module((ROOT / "web" / "fm1pkg.js").read_text(encoding="utf-8")) + "\n" + strip_module((ROOT / "web" / "fm1ota.js").read_text(encoding="utf-8"))
    meta = json.dumps({"version": version, "product": product, "pkg": "../firmware/" + pkg.name})
    for mark in ("/*LIB*/", "/*META*/"):
        if page.count(mark) != 1:
            raise SystemExit(f"lila_installer.html must contain {mark} once")
    repo = "https://github.com/" + os.environ["GITHUB_REPOSITORY"] if os.environ.get("GITHUB_REPOSITORY") else "https://github.com"
    page = page.replace("/*LIB*/", lib).replace("/*META*/", meta).replace("__REPO_URL__", repo)
    (out / "install").mkdir()
    (out / "firmware").mkdir()
    (out / "install" / "index.html").write_text(page, encoding="utf-8")
    shutil.copy(pkg, out / "firmware" / pkg.name)
    return version, pkg.name


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

inst = installer()
landing = """<p><img src="img/splash.png" width="240" alt="LILA"></p>
<h1>LILA</h1>
<p>A polyphonic toy synth for a very young musician, as custom firmware for the M-VAVE FM-1. Twenty-five animated worlds, each its own
instrument and whatever knob is turned, the picture changes with it:</p>
<ul><li><b>twelve animals and places</b>: a bird, a cow, a wolf, a frog, a spaceship, meteors, a block world, dinosaurs, a robot, the ocean, a fairy castle and the jungle;</li>
<li><b>eight instruments</b> whose keys light up on the screen: a piano, an electric piano, an organ, a xylophone, a harp, a ukulele, a flute and a kalimba;</li>
<li><b>five stories in French</b>, told by the synth: three fables of La Fontaine, <i>Boucle d'or</i> and <i>Le Petit Chaperon rouge</i>.</li></ul>
<p>For a child too young for a screen, SAVE + GLO together switch the screen to a single name, or off.</p>
<p><a class="go" href="emulator/">Try it in the browser</a><a class="go" href="install/">Install on an FM-1</a><a class="go" href="manual/">Manual</a></p>
<p><img src="img/worlds.png" alt="The 25 worlds"></p>
<p>The emulator runs the same code as the FM-1 (compiled to WebAssembly). The installer runs in Chrome or Edge over USB and can put the stock firmware back.
Installing firmware is at your own risk; LILA is a beta. GPL-3.0-only; M-VAVE and FM-1 are trademarks of their owners.</p>"""
(out / "index.html").write_text(page("LILA", landing))
(out / ".nojekyll").write_text("")
print(f"site: {out}" + (f", installer {inst[0]}" if inst else ""))
