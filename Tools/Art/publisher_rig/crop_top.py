"""
KIND: offline tool. Crops every render of a folder to the top part of the figure (a raised hand, the head) and
writes them as one sheet.

    python Tools/Art/publisher_rig/crop_top.py <renders_dir> <out.png> [share_of_height=0.45] [cols=4]
"""
import glob
import os
import sys

from PIL import Image, ImageChops, ImageDraw


def main():
    src, out = sys.argv[1], sys.argv[2]
    share = float(sys.argv[3]) if len(sys.argv) > 3 else 0.45
    cols = int(sys.argv[4]) if len(sys.argv) > 4 else 4
    cells = []
    for path in sorted(glob.glob(os.path.join(src, "*.png"))):
        im = Image.open(path).convert("RGB")
        back = Image.new("RGB", im.size, im.getpixel((2, 2)))
        box = ImageChops.difference(im, back).point(lambda v: 255 if v > 12 else 0).getbbox()
        left, top, right, bottom = box
        cut = im.crop((left, top, right, top + int((bottom - top) * share)))
        cut.thumbnail((560, 560))
        cell = Image.new("RGB", (580, 600), (20, 20, 26))
        cell.paste(cut, (10, 10))
        ImageDraw.Draw(cell).text((10, 582), os.path.basename(path), fill=(230, 230, 230))
        cells.append(cell)
    rows = (len(cells) + cols - 1) // cols
    sheet = Image.new("RGB", (580 * cols, 600 * rows), (20, 20, 26))
    for i, cell in enumerate(cells):
        sheet.paste(cell, (580 * (i % cols), 600 * (i // cols)))
    sheet.save(out)
    print("[crop]", out, sheet.size)


main()
