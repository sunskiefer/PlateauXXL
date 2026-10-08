#!/usr/bin/env python3
"""Replace the skin's drawn knob filmstrips with Valley's Rogan knobs, as on VCV Rack Plateau's panel.

    python3 tools/knob_art.py "<skin dir>/Plugin Skins"

gen_vst.py writes one filmstrip per knob radius (sh_knob_r<R>.png: 128 square frames of 2R+10 px, stacked down,
minimum first). layout.conf gives each knob colour its own radius, and this redraws each strip from art/valley/:
a static background layer, the knob rotated from -0.83 pi to +0.83 pi (VCV Rack's Rogan knob range) and a static
highlight on top, on the panel colour. Needs cairosvg and Pillow.
"""
import io
import math
import os
import sys

import cairosvg
from PIL import Image

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
ART = os.path.join(ROOT, "art", "valley")
FRAMES = 128
PANEL = (40, 40, 40)          # Plateau's dark panel, #282828 (layout.conf theme_panel)
SWEEP = 0.83 * math.pi        # componentlibrary Rogan: minAngle -0.83 pi, maxAngle +0.83 pi
SUPER = 4                     # render this many times larger, then scale down (smooth edges)

# radius in layout.conf -> (background, knob, highlight)
KNOBS = {
    58: ("Rogan1PSMed-bg", "Rogan1PSBlueMed", "Rogan1PSBlueMed-fg"),
    47: ("Rogan1PSMed-bg", "Rogan1PSGreenMed", "Rogan1PSGreenMed-fg"),
    46: ("Rogan1PSMed-bg", "Rogan1PSRedMed", "Rogan1PSRedMed-fg"),
    44: ("Rogan1PSMedSmall-bg", "Rogan1PSWhiteMedSmall", "Rogan1PSWhiteMedSmall-fg"),
}


def render(name, px):
    png = cairosvg.svg2png(url=os.path.join(ART, name + ".svg"), output_width=px, output_height=px)
    return Image.open(io.BytesIO(png)).convert("RGBA")


def build(path, radius):
    old = Image.open(path)
    frame = old.size[0]
    if old.size[1] != frame * FRAMES:
        raise SystemExit("knob_art: %s: expected %d square frames" % (path, FRAMES))
    bg_name, knob_name, fg_name = KNOBS[radius]
    size = (frame - 2) * SUPER            # the knob fills the frame, 1 px margin
    bg, knob, fg = render(bg_name, size), render(knob_name, size), render(fg_name, size)
    out = Image.new("RGB", (frame, frame * FRAMES), PANEL)
    for i in range(FRAMES):
        angle = -SWEEP + 2.0 * SWEEP * i / (FRAMES - 1)
        cell = Image.new("RGBA", (size, size), PANEL + (255,))
        cell.alpha_composite(bg)
        # PIL rotates counter-clockwise for positive angles; a knob turns clockwise as the value rises
        cell.alpha_composite(knob.rotate(-math.degrees(angle), resample=Image.BICUBIC))
        cell.alpha_composite(fg)
        small = cell.resize((frame - 2, frame - 2), Image.LANCZOS)
        tile = Image.new("RGB", (frame, frame), PANEL)
        tile.paste(small.convert("RGB"), (1, 1))
        out.paste(tile, (0, i * frame))
    out.save(path, optimize=True)
    print("knob_art: %s -> %s, %d px frames" % (os.path.basename(path), knob_name, frame))


def main():
    skin = sys.argv[1]
    done = set()
    for name in sorted(os.listdir(skin)):
        if name.startswith("sh_knob_r") and name.endswith(".png") and name[9:-4].isdigit():
            r = int(name[9:-4])
            if r not in KNOBS:
                raise SystemExit("knob_art: no Valley knob for radius %d (layout.conf radii: %s)" % (r, sorted(KNOBS)))
            build(os.path.join(skin, name), r)
            done.add(r)
    missing = set(KNOBS) - done
    if not done or missing:
        raise SystemExit("knob_art: filmstrips missing for radius %s in %s" % (sorted(missing) or sorted(KNOBS), skin))


if __name__ == "__main__":
    main()
