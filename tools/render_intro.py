#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Pre-render NLK boot splash (static PNG) for Trimui-Terminal.

Shell app has no render loop, so the intro is a pre-rendered 1024x768 PNG
(Brick real screen per CHANGELOG v0.2.7) shown ~1s by launch.sh via
fim/fbv/fbi (best-effort). See Music-Player/docs/NLK_INTRO_LOGO.md sec 6.

  python tools/render_intro.py

Output: files/assets/intro.png (1024x768, bg (8,8,12), red #E50914 "NLK").
Reproducible: same input font -> same PNG (no timestamps embedded).
"""
import os

W, H = 1024, 768
BG = (8, 8, 12)
RED = (229, 9, 20)          # #E50914
RED_DARK = (120, 5, 10)
SWEEP_ALPHA = 28            # subtle white sweep

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "files", "assets", "intro.png")
CANDIDATE_FONTS = [
    os.path.join(ROOT, "files", "assets", "fallback.ttf"),
    "C:/Windows/Fonts/DejaVuSans-Bold.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
    "C:/Windows/Fonts/arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
]


def find_font():
    for p in CANDIDATE_FONTS:
        if os.path.isfile(p):
            return p
    return None


def main():
    from PIL import Image, ImageDraw, ImageFont, ImageFilter

    font_path = find_font()
    print("font: %s" % (font_path or "BUILTIN (no ttf found)"))

    img = Image.new("RGB", (W, H), BG)
    text = "NLK"

    # Pick the largest font size whose width fits in W-80 (scale-to-fit).
    size = 420
    font = None
    if font_path:
        tmp_draw = ImageDraw.Draw(img)
        while size > 40:
            try:
                f = ImageFont.truetype(font_path, size)
            except Exception:
                size -= 20
                continue
            bbox = tmp_draw.textbbox((0, 0), text, font=f)
            # Pillow <10 has no letter_spacing kwarg; fall back below.
            tw = bbox[2] - bbox[0]
            if tw <= W - 80:
                font = f
                break
            size -= 20
    if font is None:
        if font_path:
            font = ImageFont.truetype(font_path, size)
        else:
            font = ImageFont.load_default()
            size = 200

    # letter spacing manual (compat across Pillow versions): draw per-letter centered.
    draw = ImageDraw.Draw(img, "RGBA")
    try:
        test_font = font
        widths, heights, bboxes = [], [], []
        for ch in text:
            bb = draw.textbbox((0, 0), ch, font=test_font)
            bboxes.append(bb)
            widths.append(bb[2] - bb[0])
            heights.append(bb[3] - bb[1])
    except Exception:
        widths = [size] * len(text)
        heights = [size] * len(text)

    spread = int(size * 0.12)  # letter gap grows with size (like _intro_spread)
    total_w = sum(widths) + spread * (len(text) - 1)
    # Final scale-to-fit guard: never overflow small screens (PNG is native 1024x768,
    # viewers use -a autoscale; this keeps the source itself inside margins).
    if total_w > W - 80:
        scale = (W - 80) / float(total_w)
        size = max(40, int(size * scale))
        if font_path:
            font = ImageFont.truetype(font_path, size)
        spread = int(size * 0.12)
        widths2 = []
        for ch in text:
            bb = draw.textbbox((0, 0), ch, font=font)
            widths2.append(bb[2] - bb[0])
        widths = widths2
        total_w = sum(widths) + spread * (len(text) - 1)

    x0 = (W - total_w) // 2
    # Vertical center using true glyph bbox of whole string.
    whole_bb = draw.textbbox((0, 0), text, font=font)
    th = whole_bb[3] - whole_bb[1]
    y0 = (H - th) // 2 - whole_bb[1]  # offset so glyph ink is centered

    # Subtle glow: dark-red copies blurred beneath.
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    gd = ImageDraw.Draw(glow)
    x = x0
    for i, ch in enumerate(text):
        for dx, dy in ((3, 6), (-3, 6), (0, 9)):
            gd.text((x + dx, y0 + dy), ch, font=font, fill=RED_DARK + (255,))
        x += widths[i] + spread
    glow = glow.filter(ImageFilter.GaussianBlur(12))
    img = img.convert("RGBA")
    img.alpha_composite(glow)

    draw = ImageDraw.Draw(img, "RGBA")
    # Subtle diagonal white sweep (single static streak, very low alpha).
    sweep = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    sd = ImageDraw.Draw(sweep)
    sd.polygon([(W * 0.55, 0), (W * 0.68, 0), (W * 0.45, H), (W * 0.32, H)],
               fill=(255, 255, 255, SWEEP_ALPHA))
    sweep = sweep.filter(ImageFilter.GaussianBlur(30))

    # Main glyphs in Netflix red.
    main = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    md = ImageDraw.Draw(main)
    x = x0
    for i, ch in enumerate(text):
        md.text((x, y0), ch, font=font, fill=RED + (255,))
        x += widths[i] + spread
    img.alpha_composite(sweep)
    img.alpha_composite(main)

    out = img.convert("RGB")
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    out.save(OUT, "PNG")
    print("wrote %s (%dx%d, %d bytes)" % (OUT, W, H, os.path.getsize(OUT)))


if __name__ == "__main__":
    main()
