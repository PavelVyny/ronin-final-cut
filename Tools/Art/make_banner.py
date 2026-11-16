"""
KIND: offline tool (composites the itch banner from real game frames and the cover's logo, no generation).

    python Tools/Art/make_banner.py <full-art frame> <greybox frame> <cover> <out.png> [--size 960x300] [--full-box x,y,w] [--grey-box x,y,w]

The left of the banner is a full-art fight frame, the right a greybox frame of the same fight, split by a jagged neon
crack (cyan edge over magenta glow) like the game's cut. The RONIN: FINAL CUT logo is keyed out of the cover (its
bright neon letters over the dark sky) and laid over the full-art side.
"""
import argparse
import random

from PIL import Image, ImageChops, ImageDraw, ImageFilter

# The logo's box on the cover, as shares of its size, and how much of the banner's height it takes.
LOGO_BOX = (0.03, 0.0, 0.64, 0.30)
LOGO_HEIGHT = 0.7
# How bright and how coloured a cover pixel has to be to count as logo, ramped over these spans (0-255).
KEY_VALUE = (150, 215)
KEY_SATURATION = (70, 140)
# The logo's two hues (PIL's 0-255 hue): cyan and magenta; the gold of the helmet beside it stays out.
KEY_HUES = ((105, 150), (195, 245))
CYAN = (60, 235, 255)
PINK = (255, 80, 215)


def ramp(value, span):
    lo, hi = span
    return max(0.0, min(1.0, (value - lo) / (hi - lo)))


def cover_crop(path, size, box=None):
    """The frame cropped to box (x,y,width in its pixels, the height from the banner's shape) or else scaled to cover
    size and cropped round its centre."""
    im = Image.open(path).convert("RGB")
    if box:
        x, y, w = box
        return im.crop((x, y, x + w, y + round(w * size[1] / size[0]))).resize(size, Image.LANCZOS)
    scale = max(size[0] / im.width, size[1] / im.height)
    im = im.resize((round(im.width * scale), round(im.height * scale)), Image.LANCZOS)
    left, top = (im.width - size[0]) // 2, (im.height - size[1]) // 2
    return im.crop((left, top, left + size[0], top + size[1]))


def crack(width, height, seed=7):
    """A jagged line from top to bottom just right of the middle, leaning a little, as a list of points."""
    rng = random.Random(seed)
    points, steps = [], 14
    for i in range(steps + 1):
        y = height * i / steps
        x = width * (0.66 - 0.08 * i / steps) + (rng.uniform(-1, 1) * width * 0.018 if 0 < i < steps else 0)
        points.append((x, y))
    return points


def logo(cover_path):
    cover = Image.open(cover_path).convert("RGB")
    w, h = cover.size
    box = tuple(round(v) for v in (LOGO_BOX[0] * w, LOGO_BOX[1] * h, LOGO_BOX[2] * w, LOGO_BOX[3] * h))
    part = cover.crop(box)
    hsv = part.convert("HSV")
    alpha = Image.new("L", part.size)
    alpha.putdata([round(255 * ramp(v, KEY_VALUE) * ramp(s, KEY_SATURATION)) if any(lo <= h <= hi for lo, hi in KEY_HUES) else 0
                   for (h, s, v) in hsv.get_flattened_data()])
    alpha = alpha.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.GaussianBlur(0.8))
    part.putalpha(alpha)
    return part.crop(alpha.getbbox())


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("full")
    parser.add_argument("grey")
    parser.add_argument("cover")
    parser.add_argument("out")
    parser.add_argument("--size", default="960x300")
    parser.add_argument("--full-box", help="x,y,width of the full-art frame to use")
    parser.add_argument("--grey-box", help="x,y,width of the greybox frame to use")
    args = parser.parse_args()
    size = tuple(int(v) for v in args.size.split("x"))
    boxes = [tuple(int(v) for v in b.split(",")) if b else None for b in (args.full_box, args.grey_box)]
    full, grey = cover_crop(args.full, size, boxes[0]), cover_crop(args.grey, size, boxes[1])

    line = crack(*size)
    mask = Image.new("L", size, 0)
    ImageDraw.Draw(mask).polygon(line + [(size[0], size[1]), (size[0], 0)], fill=255)
    banner = Image.composite(grey, full, mask)

    glow = Image.new("RGB", size, "black")
    ImageDraw.Draw(glow).line(line, fill=PINK, width=14)
    banner = ImageChops.add(banner, glow.filter(ImageFilter.GaussianBlur(9)))
    edge = Image.new("RGB", size, "black")
    draw = ImageDraw.Draw(edge)
    draw.line(line, fill=PINK, width=5)
    draw.line(line, fill=CYAN, width=2)
    banner = ImageChops.lighter(banner, edge)

    # A soft shade on the left so the logo reads over the fight.
    shade = Image.new("L", size, 0)
    ImageDraw.Draw(shade).rectangle((0, 0, round(size[0] * 0.42), size[1]), fill=120)
    shade = shade.filter(ImageFilter.GaussianBlur(size[0] * 0.06))
    banner = Image.composite(Image.new("RGB", size, (6, 6, 16)), banner, shade)

    mark = logo(args.cover)
    height = round(size[1] * LOGO_HEIGHT)
    mark = mark.resize((round(mark.width * height / mark.height), height), Image.LANCZOS)
    at = (round(size[0] * 0.03), (size[1] - height) // 2)
    banner.paste(mark, at, mark)
    banner.save(args.out)
    print("banner", size, "logo", mark.size, "at", at)


if __name__ == "__main__":
    main()
