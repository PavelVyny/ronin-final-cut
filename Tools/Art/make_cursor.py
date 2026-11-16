"""Draws the neon mouse pointer of the game (T-034) into Content/Slate/Cursors.

A hardware cursor: Windows draws it, so it never trails the mouse. An arrow, not a crosshair (Hades 2 is the
reference). The engine picks the PNG nearest the screen's DPI scale: NeonPointer.png at 1x, NeonPointer@2x.png.

Run: python Tools/Art/make_cursor.py
"""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

OUT = Path(__file__).resolve().parents[2] / "Content" / "Slate" / "Cursors"

# The arrow on a 32 px grid, tip at (1, 1): the hot spot is the top left corner.
ARROW = [(1, 1), (1, 23), (7, 17.5), (11, 27), (15, 25.3), (11, 16), (19, 16)]
CYAN = (63, 230, 240, 255)
MAGENTA = (255, 63, 174, 255)
DARK = (13, 11, 23, 255)


def draw(scale: int) -> Image.Image:
    size = 32 * scale
    points = [(x * scale, y * scale) for x, y in ARROW]
    # A magenta glow under the arrow, so it reads on the dark arena and on the grey grid alike.
    glow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(glow).polygon(points, fill=MAGENTA, outline=MAGENTA, width=2 * scale)
    glow = glow.filter(ImageFilter.GaussianBlur(1.2 * scale))
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    image.alpha_composite(glow)
    pen = ImageDraw.Draw(image)
    pen.polygon(points, fill=DARK, outline=CYAN, width=2 * scale)
    return image


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    draw(1).save(OUT / "NeonPointer.png")
    draw(2).save(OUT / "NeonPointer@2x.png")
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
