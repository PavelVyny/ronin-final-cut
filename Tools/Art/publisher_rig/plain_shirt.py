"""
KIND: offline tool (headless Blender, changes the UVs and the base colour of a Publisher .blend).

Makes the shirt under the tie plain black. When the tie was cut loose (build_rig.py, split_tie) the shirt faces it
lay on were folded onto one texel of "plain black" (plain_black_uv). That texel lies on a black sliver two or three
texels wide between texels of the cream jacket, so as soon as the texture is seen at a lower mip level the whole
tie-shaped patch takes the average of the two and shows as a grey imprint of the tie. This tool

- moves the UVs of those folded faces, and of the folded shirt faces around the tie that point at the same texel, to
  the centre of the widest square of plain shirt in the texture (base colour
  at the black of the shirt beside the tie, a flat normal, the shirt's roughness, no metal, used by shirt faces of the
  front of the chest only), so every mip level sees black there. No texel changes for this;
- paints the grey texels of the unfolded shirt faces under the tie black, never a texel that any other face (the tie
  strip, the knot, the collar, the skin, the jacket) uses, nor a coloured one (the gold of the knot).

Folded faces outside the chest under the tie (the collar of neck_border, the gold under the knot) are left alone.

    blender -b --factory-startup --python Tools/Art/publisher_rig/plain_shirt.py -- <file.blend> [--write] [report.txt]

Without --write it only measures. The report compares the colour the shirt under the tie shows before and after, at
full size and averaged over squares of 4 to 32 texels (what the lower mip levels see).
"""
import sys

import bpy
import numpy as np
from mathutils import kdtree

# The chest under the tie, in Blender metres (he faces -Y). Above TOP the knot and the collar begin.
BOX_X, BOX_Y, BOTTOM, TOP = 0.16, -0.12, 1.30, 2.12
# A shirt face is under the tie if its centre lies this close (m) to a vert of the tie strip.
MARGIN = 0.04
# The shirt beside the tie that gives the black: |x| from, |x| to, z from, z to.
SIDE = (0.16, 0.26, 1.55, 2.00)
# The front of the chest between the lapels: the new spot is used by faces of it only.
CHEST_X = 0.40
# A texel is off the shirt if a channel of a map differs by more than this from the shirt beside the tie.
PLAIN = {"base": 0.012, "normal": 0.03, "rough": 0.05, "metal": 0.05}
# A texel is grey (not the gold of the knot) if its channels differ by less than this.
GREY = 0.05
# Half sizes of the squares averaged in the report, like mip levels 2 to 5.
MIPS = (2, 4, 8, 16)


def images(me):
    """The four textures of the material by their input on the shader."""
    found = {}
    for node in me.materials[0].node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            for name, socket in (("base", "Base Color"), ("rough", "Roughness"), ("metal", "Metallic")):
                link = node.inputs[socket].links
                if link and link[0].from_node.type == "TEX_IMAGE":
                    found[name] = link[0].from_node.image
        if node.type == "NORMAL_MAP":
            link = node.inputs["Color"].links
            if link and link[0].from_node.type == "TEX_IMAGE":
                found["normal"] = link[0].from_node.image
    if set(found) != {"base", "rough", "metal", "normal"}:
        raise RuntimeError("textures found: %s" % sorted(found))
    return found


def coverage(t, w, h):
    """Texels whose centre lies in a UV triangle given in texel units, plus a half-texel rim."""
    x0, y0 = np.floor(t.min(axis=0)).astype(int) - 1
    x1, y1 = np.ceil(t.max(axis=0)).astype(int) + 1
    x0, y0, x1, y1 = max(x0, 0), max(y0, 0), min(x1, w - 1), min(y1, h - 1)
    d = (t[1, 1] - t[2, 1]) * (t[0, 0] - t[2, 0]) + (t[2, 0] - t[1, 0]) * (t[0, 1] - t[2, 1])
    if x1 < x0 or y1 < y0 or abs(d) < 1e-9:
        return None
    xs, ys = np.meshgrid(np.arange(x0, x1 + 1), np.arange(y0, y1 + 1))
    b0 = ((t[1, 1] - t[2, 1]) * (xs - t[2, 0]) + (t[2, 0] - t[1, 0]) * (ys - t[2, 1])) / d
    b1 = ((t[2, 1] - t[0, 1]) * (xs - t[2, 0]) + (t[0, 0] - t[2, 0]) * (ys - t[2, 1])) / d
    tol = min(0.5, 1.0 / max(x1 - x0, y1 - y0, 1))
    return slice(y0, y1 + 1), slice(x0, x1 + 1), (b0 >= -tol) & (b1 >= -tol) & (1 - b0 - b1 >= -tol)


def widest_square(ok):
    """Centre (y, x) and half size of the largest square of True texels."""
    h, w = ok.shape
    run = np.zeros((h + 1, w + 1), dtype=np.int32)
    best, where = 0, (0, 0)
    for y in range(h):
        row = ok[y]
        for x in np.where(row)[0]:
            run[y + 1, x + 1] = 1 + min(run[y, x], run[y, x + 1], run[y + 1, x])
        k = int(run[y + 1].max())
        if k > best:
            best = k
            x = int(run[y + 1].argmax()) - 1
            where = (y - k // 2, x - k // 2)
    return where, (best - 1) // 2


def shown(base, faces, size):
    """Mean colour the faces show: per face the texels under it, or for a folded face its one texel, with the
    average over the squares of MIPS around the place it samples."""
    h, w = base.shape[:2]
    full, mips = [], {r: [] for r in MIPS}
    for t in faces:
        x, y = (min(max(int(round(c)), 0), n - 1) for c, n in zip((t * size - 0.5).mean(axis=0), (w, h)))
        full.append(base[y, x])
        for r in MIPS:
            mips[r].append(base[max(y - r, 0):y + r, max(x - r, 0):x + r].reshape(-1, 3).mean(axis=0))
    out = ["full %s" % np.round(np.mean(full, axis=0), 3).tolist()]
    out += ["%dx%d %s" % (2 * r, 2 * r, np.round(np.mean(mips[r], axis=0), 3).tolist()) for r in MIPS]
    return ", ".join(out)


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    path, write = args[0], "--write" in args
    report = next((a for a in args[1:] if a != "--write"), None)
    bpy.ops.wm.open_mainfile(filepath=path)
    mesh = next(o for o in bpy.context.scene.objects if o.type == "MESH")
    me = mesh.data
    maps = images(me)
    w, h = maps["base"].size
    px = {k: np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4)[..., :3] for k, im in maps.items()}
    base = px["base"]
    size = np.array((w, h))
    co = np.array([v.co[:] for v in me.vertices])
    uv = me.uv_layers.active.data
    tie_groups = {g.index for g in mesh.vertex_groups if g.name.startswith("tie_")}
    on_tie = np.array([sum(g.weight for g in v.groups if g.group in tie_groups) > 0.5 for v in me.vertices])
    strip = kdtree.KDTree(int(on_tie.sum()))
    for k, i in enumerate(np.where(on_tie)[0]):
        strip.insert(co[i], k)
    strip.balance()

    shirt = np.zeros((h, w), dtype=bool)    # unfolded shirt faces under the tie
    beside = np.zeros((h, w), dtype=bool)   # shirt faces beside the tie
    other = np.zeros((h, w), dtype=bool)    # every other face
    front = np.zeros((h, w), dtype=bool)    # faces of the front of the chest
    foreign = np.zeros((h, w), dtype=bool)  # faces off the front of the chest
    folded = []                             # folded faces of the front of the chest: (polygon, uv in texels)
    for poly in me.polygons:
        vs = list(poly.vertices)
        p = co[vs]
        t = np.array([uv[l].uv[:] for l in poly.loop_indices]) * size - 0.5
        body = not on_tie[vs].any()
        chest = (body and np.all(p[:, 1] < BOX_Y) and np.all((p[:, 2] > BOTTOM) & (p[:, 2] < TOP)))
        under = chest and np.all(np.abs(p[:, 0]) < BOX_X) and strip.find(p.mean(axis=0))[2] < MARGIN
        on_front = chest and np.all(np.abs(p[:, 0]) < CHEST_X)
        if np.ptp(t, axis=0).max() < 0.5:
            x, y = (min(max(int(round(c)), 0), n - 1) for c, n in zip(t.mean(axis=0), (w, h)))
            # A shirt face next to the strip may share a vert with it: folded, it is shirt all the same.
            if (on_front or not body) and np.all(p[:, 1] < BOX_Y) and np.all((p[:, 2] > BOTTOM) & (p[:, 2] < TOP)) \
                    and np.all(np.abs(p[:, 0]) < CHEST_X):
                folded.append((poly, t))
            else:
                other[y, x] = True
            (front if on_front else foreign)[y, x] = True
            continue
        for k in range(1, len(vs) - 1):
            cover = coverage(t[[0, k, k + 1]], w, h)
            if cover is None:
                continue
            (shirt if under else other)[cover[0], cover[1]] |= cover[2]
            (front if on_front else foreign)[cover[0], cover[1]] |= cover[2]
            if body and np.all((np.abs(p[:, 0]) > SIDE[0]) & (np.abs(p[:, 0]) < SIDE[1])) and chest \
                    and np.all((p[:, 2] > SIDE[2]) & (p[:, 2] < SIDE[3])):
                beside[cover[0], cover[1]] |= cover[2]

    # The shirt beside the tie: its black, normal, roughness and metal.
    dark = beside & ~shirt & (base.max(axis=-1) < 0.15)
    look = {k: np.median(px[k][dark], axis=0) for k in px}
    black = look["base"]
    lines = ["%s" % path, "shirt beside the tie, %d texels (spread %.4f): %s" % (
        dark.sum(), base[dark].std(axis=0).max(),
        ", ".join("%s %s" % (k, np.round(v, 3).tolist()) for k, v in look.items()))]

    # The folded faces of the chest that point at a grey texel move; the gold ones under the knot stay.
    moving = [(poly, t) for poly, t in folded
              if np.ptp(base[min(max(int(round(t[:, 1].mean())), 0), h - 1),
                             min(max(int(round(t[:, 0].mean())), 0), w - 1)]) < GREY]
    spots = sorted({(int(round(t[:, 0].mean())), int(round(t[:, 1].mean()))) for _, t in moving})
    rest = [poly for poly in me.polygons if (int(round(np.mean([uv[l].uv[0] for l in poly.loop_indices]) * w - 0.5)),
                                              int(round(np.mean([uv[l].uv[1] for l in poly.loop_indices]) * h - 0.5)))
            in spots and poly.index not in {p.index for p, _ in moving}]
    if rest:
        z = np.array([co[list(p.vertices)].mean(axis=0) for p in rest])
        lines.append("other folded faces on the same grey texels, left alone: %d, z %.2f..%.2f, |x| up to %.2f" % (
            len(rest), z[:, 2].min(), z[:, 2].max(), np.abs(z[:, 0]).max()))
        low = [p for p in rest if co[list(p.vertices)][:, 2].max() < TOP]
        lines.append("  below the knot: %d, of them with a tie vert %d, behind y %.2f %d" % (
            len(low), sum(on_tie[list(p.vertices)].any() for p in low),
            BOX_Y, sum((co[list(p.vertices)][:, 1] >= BOX_Y).any() for p in low)))
    plain = front & ~foreign & ~shirt
    for k, limit in PLAIN.items():
        plain &= np.abs(px[k] - look[k]).max(axis=-1) <= limit
    (cy, cx), half = widest_square(plain)
    lines.append("folded faces of the chest below the knot: %d, %d of them on grey texels %s; the gold ones under the knot stay"
                 % (len(folded), len(moving), spots))
    lines.append("new spot (%d, %d): the centre of %dx%d texels of plain shirt; base %s normal %s rough %.3f" % (
        cx, cy, 2 * half + 1, 2 * half + 1, np.round(base[cy, cx], 3).tolist(),
        np.round(px["normal"][cy, cx], 3).tolist(), px["rough"][cy, cx, 0]))

    grey = shirt & ~other & (np.ptp(base, axis=-1) < GREY) & (base.max(axis=-1) < 0.5)
    paint = grey & (np.abs(base - black).max(axis=-1) > PLAIN["base"])
    lines.append("unfolded shirt under the tie: %d texels (%d shared with other faces and left alone), %d grey ones "
                 "painted black" % ((shirt & ~other).sum(), (shirt & other).sum(), paint.sum()))
    faces = [t / size for _, t in moving]
    lines.append("before: the moving faces show " + shown(base, [f + 0.5 / size for f in faces], size))

    if write:
        if half < max(MIPS):
            raise RuntimeError("no square of plain shirt as wide as the largest mip square")
        spot = ((cx + 0.5) / w, (cy + 0.5) / h)
        for poly, _ in moving:
            for l in poly.loop_indices:
                uv[l].uv = spot
        base[paint] = black
        full = np.array(maps["base"].pixels[:], dtype=np.float32).reshape(h, w, 4)
        full[..., :3] = base
        maps["base"].pixels.foreach_set(full.ravel())
        maps["base"].update()
        maps["base"].pack()
        after = [np.array([spot])] * len(moving)
        lines.append("after:  the moving faces show " + shown(base, after, size))
        bpy.ops.wm.save_mainfile()
        lines.append("saved")
    print("\n".join("[shirt] " + l for l in lines))
    if report:
        with open(report, "w") as out:
            out.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
