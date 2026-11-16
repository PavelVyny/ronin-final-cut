"""
KIND: offline tool (Pillow, writes one image).

Joins rendered frames into a contact sheet, or into a GIF, with a caption under each frame.

    python Tools/Art/publisher_rig/sheet.py <out.png|out.gif|out.webp> [--cols 6] [--cell 320] [--fps 15] [--title "..."]
        <frame.png[=caption]> ...
    python Tools/Art/publisher_rig/sheet.py out.gif --pair <before_dir> <after_dir> <prefix>   (side by side GIF)
"""
import argparse
import glob
import os

from PIL import Image, ImageDraw, ImageFont


def font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("frames", nargs="*")
    ap.add_argument("--cols", type=int, default=6)
    ap.add_argument("--cell", type=int, default=320)
    ap.add_argument("--fps", type=float, default=15)
    ap.add_argument("--title", default="")
    ap.add_argument("--pair", nargs=3)
    args = ap.parse_args()

    if args.pair:
        before, after, prefix = args.pair
        a = sorted(glob.glob(os.path.join(before, prefix + "*.png")))
        b = sorted(glob.glob(os.path.join(after, prefix + "*.png")))
        frames = []
        for pa, pb in zip(a, b):
            ia = Image.open(pa).convert("RGB").resize((args.cell, args.cell), Image.LANCZOS)
            ib = Image.open(pb).convert("RGB").resize((args.cell, args.cell), Image.LANCZOS)
            im = Image.new("RGB", (args.cell * 2 + 4, args.cell), (20, 20, 24))
            im.paste(ia, (0, 0))
            im.paste(ib, (args.cell + 4, 0))
            d = ImageDraw.Draw(im)
            d.text((8, 6), "before (Meshy)", fill=(255, 120, 110), font=font(16))
            d.text((args.cell + 12, 6), "after", fill=(130, 230, 140), font=font(16))
            frames.append(im)
        save_anim(frames, args.out, args.fps)
        return

    items = [f.split("=", 1) if "=" in f else (f, "") for f in args.frames]
    if args.out.lower().endswith((".gif", ".webp")):
        save_anim([Image.open(p).convert("RGB").resize((args.cell, args.cell), Image.LANCZOS) for p, _ in items], args.out, args.fps)
        return
    cols = min(args.cols, len(items))
    rows = (len(items) + cols - 1) // cols
    cap = 22
    top = 30 if args.title else 0
    sheet = Image.new("RGB", (cols * args.cell, top + rows * (args.cell + cap)), (20, 20, 24))
    d = ImageDraw.Draw(sheet)
    if args.title:
        d.text((8, 5), args.title, fill=(240, 240, 240), font=font(18))
    for i, (p, c) in enumerate(items):
        im = Image.open(p).convert("RGB").resize((args.cell, args.cell), Image.LANCZOS)
        x, y = (i % cols) * args.cell, top + (i // cols) * (args.cell + cap)
        sheet.paste(im, (x, y))
        d.text((x + 6, y + args.cell + 3), c or os.path.basename(p)[:-4], fill=(200, 200, 200), font=font(14))
    sheet.save(args.out)
    print("[sheet]", args.out, sheet.size)


def save_anim(frames, out, fps):
    if out.lower().endswith(".gif"):
        frames = [f.quantize(colors=128, method=Image.MEDIANCUT, dither=Image.NONE) for f in frames]
    frames[0].save(out, save_all=True, append_images=frames[1:], duration=int(1000 / fps), loop=0, optimize=False)
    print("[sheet]", out, len(frames), "frames", os.path.getsize(out) // 1024, "KB")


main()
