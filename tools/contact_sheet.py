#!/usr/bin/env python3
"""contact_sheet.py OUT.png COLUMNS IMG...: the screenshots side by side (for looking at all the worlds at once)."""
import sys
from PIL import Image
out, cols, imgs = sys.argv[1], int(sys.argv[2]), sys.argv[3:]
ims = [Image.open(p).convert("RGB") for p in imgs]
w, h = ims[0].size
rows = (len(ims) + cols - 1) // cols
sheet = Image.new("RGB", (cols * w, rows * h))
for i, im in enumerate(ims):
    sheet.paste(im, ((i % cols) * w, (i // cols) * h))
sheet.save(out)
