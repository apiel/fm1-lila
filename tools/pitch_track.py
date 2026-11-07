#!/usr/bin/env python3
"""pitch_track.py FILE.raw [STEP_MS]: the pitch of a mono 16-bit 44.1 kHz raw file every STEP_MS (autocorrelation), for checking contours."""
import array, sys
d = array.array('h'); d.frombytes(open(sys.argv[1], 'rb').read())
step = int(sys.argv[2]) if len(sys.argv) > 2 else 150
W = 2048
out = []
for t0 in range(0, len(d) - W - 900, step * 44):
    seg = d[t0:t0 + W]
    en = sum(abs(x) for x in seg) / W
    if en < 40:
        out.append("  -  "); continue
    best, bl = 0, 0
    for lag in range(30, 800):
        c = sum(seg[i] * seg[i + lag] for i in range(0, W - 800, 3))
        if c > best: best, bl = c, lag
    out.append("%4.0f " % (44100 / bl) if bl else "  ?  ")
print(" ".join(out))
