#!/usr/bin/env python3
"""Skin touch-up run by gen_vst.py (vst.json "skin_post"): section titles in Titillium Web, like the lettering on the VCV
panel, and the name plate in the OUTPUT frame with a small credit line under it. The frames in layout.conf have no
titles of their own (the generator's title sits on the frame's rule); the titles here are placed by frame.

    python3 tools/post_skin.py "<skin dir>"     (the "<vendor> - VST - Plateau" folder or its "Plugin Skins")
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
FONT = os.path.join(ROOT, "art", "fonts", "TitilliumWeb-SemiBold.ttf")
Y_OFF = 86             # layout.conf (Force Shadow) y minus this = skin y
PLATE_CX = 1168        # centre of the OUTPUT frame (layout.conf: x=1066 w=204)
TITLE_CY = 612
CREDIT_CY = 660
# layout.conf frames (x, y) -> title
TITLES = [((10, 92), "REVERB"), ((462, 92), "INPUT"), ((914, 92), "TANK"),
          ((10, 400), "INPUT FILTER  /  REVERB FILTER"), ((614, 400), "MODULATION"), ((1066, 400), "OUTPUT")]


def centred(dr, cx, cy, text, size, fill):
    font = ImageFont.truetype(FONT, size)
    b = dr.textbbox((0, 0), text, font=font)
    dr.text((cx - (b[2] - b[0]) / 2 - b[0], cy - Y_OFF - (b[3] - b[1]) / 2 - b[1]), text, font=font, fill=fill)


def main():
    skin = sys.argv[1]
    if os.path.isdir(os.path.join(skin, "Plugin Skins")):
        skin = os.path.join(skin, "Plugin Skins")
    bg = os.path.join(skin, "sh_bg_0.png")
    if not os.path.exists(bg):
        raise SystemExit("post_skin: no page background %s" % bg)
    im = Image.open(bg).convert("RGB")
    dr = ImageDraw.Draw(im)
    font = ImageFont.truetype(FONT, 22)
    for (x, y), title in TITLES:
        b = dr.textbbox((0, 0), title, font=font)
        dr.text((x + 18 - b[0], y - Y_OFF + 30 - b[3]), title, font=font, fill="#ffffff")
    centred(dr, PLATE_CX, TITLE_CY, "Plateau", 46, "#ffffff")
    centred(dr, PLATE_CX, CREDIT_CY, "after Valley Audio", 17, "#8a8a8a")
    im.save(bg, optimize=True)
    print("post_skin: titles and name plate drawn on " + os.path.basename(bg))


if __name__ == "__main__":
    main()
