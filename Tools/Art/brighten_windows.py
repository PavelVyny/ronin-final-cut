"""Brighten the warm window texels of a lantern's base colour so M_JamArenaProp's glow lights them (T-099).

The arena prop material glows texels brighter than GlowFrom (0.6 luma). The Rodin stone lantern's orange windows sit at
about 0.67, so they barely glowed; its stone stays under 0.6. Warm texels are scaled up until their luma is about 0.85.

    python -I Tools/Art/brighten_windows.py <base_color.png>
"""

import sys

import numpy
from PIL import Image

TARGET_LUMA = 0.85

path = sys.argv[1]
rgb = numpy.asarray(Image.open(path).convert("RGB")).astype(numpy.float32) / 255.0
r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
luma = 0.3 * r + 0.59 * g + 0.11 * b
warm = (r > 0.7) & (r - b > 0.35) & (g > 0.35)
gain = numpy.where(warm, TARGET_LUMA / numpy.maximum(luma, 1e-3), 1.0)[..., None]
out = numpy.clip(rgb * gain, 0.0, 1.0)
Image.fromarray((out * 255.0 + 0.5).astype(numpy.uint8)).save(path)
print("brighten_windows: %.2f%% of texels brightened" % (100.0 * warm.mean()))
