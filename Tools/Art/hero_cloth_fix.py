"""
KIND: offline tool. Paints the soft cyan and red-magenta glow patches that Meshy baked into the hero's black cloth
over with the cloth's own black (the author, Sun 15 Nov: "red and light-blue patches on the black cloth of the back
and his sides"). The neon piping (bright and saturated), the mask and the face stay untouched.

Which texels: those of the cloth (faces whose skin belongs neither to the head and neck nor to the hands and feet,
found from the vertex weights), saturated, cyan or red-to-magenta in hue, and dimmer than the piping. They take the
median colour of the black cloth, keeping a little of their own shading.

Two steps, because Blender's Python has no PIL:
    blender -b --factory-startup --python Tools/Art/hero_cloth_fix.py -- <hero.glb> <protected.json>
    python Tools/Art/hero_cloth_fix.py <protected.json> <base_color.png> <out.png> [mask.png]
"""
import json
import sys

PROTECTED = ("head", "neck", "hand", "foot", "ball", "toe")
# Thresholds in 0..1: a patch is dimmer than this (the piping is brighter), at least this saturated.
MAX_VALUE = 0.62
MIN_SATURATION = 0.22
CYAN = (150.0, 230.0)
# Red to magenta wraps round 0 degrees.
RED_FROM, RED_TO = 285.0, 25.0
KEEP_SHADING = 0.35


def dump_protected(glb, out_json):
    """Blender: the UV polygons of every face skinned mostly to a protected bone."""
    import bpy

    for ob in list(bpy.data.objects):
        bpy.data.objects.remove(ob)
    bpy.ops.import_scene.gltf(filepath=glb)
    mesh = next(o for o in bpy.data.objects if o.type == "MESH" and o.parent is not None)
    me = mesh.data
    groups = {g.index: g.name.lower() for g in mesh.vertex_groups}
    uv = me.uv_layers.active.data
    polys = []
    for poly in me.polygons:
        weight = {}
        for vi in poly.vertices:
            for g in me.vertices[vi].groups:
                name = groups.get(g.group, "")
                weight[name] = weight.get(name, 0.0) + g.weight
        owner = max(weight, key=weight.get) if weight else ""
        if any(k in owner for k in PROTECTED):
            polys.append([[round(uv[li].uv[0], 5), round(uv[li].uv[1], 5)] for li in poly.loop_indices])
    with open(out_json, "w") as out:
        json.dump(polys, out)
    print("[cloth] protected faces %d of %d" % (len(polys), len(me.polygons)))


def paint(protected_json, base_path, out_path, mask_path=None):
    import numpy as np
    from PIL import Image, ImageDraw, ImageFilter

    img = Image.open(base_path).convert("RGB")
    W, H = img.size
    protect = Image.new("L", (W, H), 0)
    draw = ImageDraw.Draw(protect)
    for poly in json.load(open(protected_json)):
        draw.polygon([(u * W, (1.0 - v) * H) for u, v in poly], fill=255)
    # A margin round the protected islands: the texture bleeds past the UV edges.
    protect = protect.filter(ImageFilter.MaxFilter(9))
    if mask_path:
        protect.save(mask_path)
    rgb = np.asarray(img).astype(np.float32) / 255.0
    mx, mn = rgb.max(axis=2), rgb.min(axis=2)
    sat = np.where(mx > 1e-4, (mx - mn) / np.maximum(mx, 1e-4), 0.0)
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    delta = np.maximum(mx - mn, 1e-4)
    hue = np.where(mx == r, ((g - b) / delta) % 6, np.where(mx == g, (b - r) / delta + 2, (r - g) / delta + 4)) * 60.0
    cyan = (hue >= CYAN[0]) & (hue <= CYAN[1])
    red = (hue >= RED_FROM) | (hue <= RED_TO)
    cloth = np.asarray(protect) == 0
    patch = cloth & (mx < MAX_VALUE) & (sat > MIN_SATURATION) & (cyan | red)
    black = cloth & (mx < 0.25) & (sat < 0.25)
    tone = np.median(rgb[black], axis=0)
    shade = np.clip((mx / max(float(np.median(mx[black])), 1e-3)) ** 0.5, 0.5, 1.6)[..., None]
    fixed = rgb.copy()
    fixed[patch] = (tone * (1.0 - KEEP_SHADING + KEEP_SHADING * shade))[patch]
    Image.fromarray((np.clip(fixed, 0, 1) * 255).astype(np.uint8)).save(out_path)
    print("[cloth] cloth black %s; patch texels %d (%.1f%% of the cloth)" % (
        tuple(int(c * 255) for c in tone), int(patch.sum()), 100.0 * patch.sum() / cloth.sum()))


if "--" in sys.argv:
    dump_protected(*sys.argv[sys.argv.index("--") + 1:][:2])
elif __name__ == "__main__":
    paint(*sys.argv[1:])
