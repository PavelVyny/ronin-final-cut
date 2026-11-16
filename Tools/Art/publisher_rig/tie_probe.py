"""
KIND: offline tool (headless Blender).

Finds the tie on the rebuilt Publisher (gold vertices on the front of the chest) and writes its shape in the rest
pose: size of the connected parts, how it is joined to the shirt, its centre line by height and its weights.

    blender -b --factory-startup --python Tools/Art/publisher_rig/tie_probe.py -- <file.blend> <out.txt>
"""
import colorsys
import sys

import bpy
import numpy as np


def vertex_colours(mesh):
    me = mesh.data
    image = next(i for i in bpy.data.images if i.size[0] > 0)
    for mat in me.materials:
        if mat and mat.use_nodes:
            for node in mat.node_tree.nodes:
                if node.type == "BSDF_PRINCIPLED":
                    link = node.inputs["Base Color"].links
                    if link and link[0].from_node.type == "TEX_IMAGE":
                        image = link[0].from_node.image
    w, h = image.size
    px = np.array(image.pixels[:], dtype=np.float32).reshape(h, w, 4)
    uv = np.zeros(len(me.loops) * 2, dtype=np.float32)
    me.uv_layers.active.data.foreach_get("uv", uv)
    uv = uv.reshape(-1, 2)
    vi = np.zeros(len(me.loops), dtype=np.int64)
    me.loops.foreach_get("vertex_index", vi)
    xs = np.clip((uv[:, 0] % 1.0 * w).astype(int), 0, w - 1)
    ys = np.clip((uv[:, 1] % 1.0 * h).astype(int), 0, h - 1)
    total = np.zeros((len(me.vertices), 3))
    count = np.zeros(len(me.vertices))
    np.add.at(total, vi, px[ys, xs, :3])
    np.add.at(count, vi, 1)
    return total / np.maximum(count, 1)[:, None]


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    mesh = next(o for o in bpy.context.scene.objects if o.type == "MESH")
    me = mesh.data
    co = np.array([v.co[:] for v in me.vertices])
    hsv = np.array([colorsys.rgb_to_hsv(*c) for c in vertex_colours(mesh)])
    gold = (hsv[:, 0] > 0.07) & (hsv[:, 0] < 0.17) & (hsv[:, 1] > 0.45) & (hsv[:, 2] > 0.3)
    zone = (np.abs(co[:, 0]) < 0.32) & (co[:, 1] < 0.0) & (co[:, 2] > 1.2) & (co[:, 2] < 2.15)
    cand = gold & zone
    adj = [[] for _ in me.vertices]
    for e in me.edges:
        a, b = e.vertices
        adj[a].append(b)
        adj[b].append(a)
    lines = ["gold verts in the chest zone: %d" % cand.sum()]
    seen = np.zeros(len(co), dtype=bool)
    parts = []
    for start in np.where(cand)[0]:
        if seen[start]:
            continue
        comp, stack = [start], [start]
        seen[start] = True
        while stack:
            a = stack.pop()
            for b in adj[a]:
                if cand[b] and not seen[b]:
                    seen[b] = True
                    comp.append(b)
                    stack.append(b)
        parts.append(np.array(comp))
    parts.sort(key=len, reverse=True)
    for part in parts[:8]:
        p = co[part]
        border = sum(1 for a in part for b in adj[a] if not cand[b])
        lines.append("part %5d verts  x %.3f..%.3f  y %.3f..%.3f  z %.3f..%.3f  edges to non-gold %d" % (
            len(part), p[:, 0].min(), p[:, 0].max(), p[:, 1].min(), p[:, 1].max(), p[:, 2].min(), p[:, 2].max(),
            border))
    tie = parts[0]
    p = co[tie]
    lines.append("tie centre line (z band: n, x mean, x min..max, y min..max; nearest non-gold y at that x,z)")
    other = np.where(~cand & (co[:, 1] < 0.1))[0]
    for z0 in np.arange(p[:, 2].min(), p[:, 2].max(), 0.04):
        m = (p[:, 2] >= z0) & (p[:, 2] < z0 + 0.04)
        if not m.any():
            continue
        q = p[m]
        cx = q[:, 0].mean()
        near = other[(np.abs(co[other, 0] - cx) < 0.05) & (np.abs(co[other, 2] - z0 - 0.02) < 0.03)]
        shirt = co[near, 1].min() if len(near) else float("nan")
        lines.append("  z %.2f: %4d  x %6.3f  %6.3f..%6.3f  y %6.3f..%6.3f  shirt y %6.3f" % (
            z0, m.sum(), cx, q[:, 0].min(), q[:, 0].max(), q[:, 1].min(), q[:, 1].max(), shirt))
    island = np.zeros(len(co), dtype=bool)
    stack = [int(tie[0])]
    island[stack] = True
    while stack:
        a = stack.pop()
        for b in adj[a]:
            if not island[b]:
                island[b] = True
                stack.append(b)
    q = co[island]
    lines.append("mesh island of the tie: %d verts of %d, z %.3f..%.3f, |x| max %.3f" % (
        island.sum(), len(co), q[:, 2].min(), q[:, 2].max(), np.abs(q[:, 0]).max()))
    names = [g.name for g in mesh.vertex_groups]
    total = {}
    for i in tie:
        for g in me.vertices[int(i)].groups:
            total[names[g.group]] = total.get(names[g.group], 0.0) + g.weight
    lines.append("tie weights: " + ", ".join("%s %.0f" % kv for kv in sorted(total.items(), key=lambda kv: -kv[1])))
    with open(args[1], "w") as out:
        out.write("\n".join(lines) + "\n")


main()
