"""
KIND: offline tool (headless Blender, read only).

Measures the Publisher's mesh for the rig work: welded topology (shells, open edges) and the body profile in slices.

    blender -b --factory-startup --python Tools/Art/publisher_rig/analyse_mesh.py -- <file.fbx>
"""
import sys
from collections import Counter, defaultdict

import bmesh
import bpy


def main():
    path = sys.argv[sys.argv.index("--") + 1]
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=path)
    ob = next(o for o in bpy.data.objects if o.type == "MESH")
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bm.transform(ob.matrix_world)
    before = len(bm.verts)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-5)
    print("verts %d -> welded %d, faces %d" % (before, len(bm.verts), len(bm.faces)))
    kinds = Counter()
    for e in bm.edges:
        kinds[len(e.link_faces)] += 1
    print("edges by face count", dict(kinds))
    bm.verts.ensure_lookup_table()
    seen = set()
    shells = []
    for v in bm.verts:
        if v.index in seen:
            continue
        stack = [v]
        seen.add(v.index)
        members = []
        while stack:
            a = stack.pop()
            members.append(a)
            for e in a.link_edges:
                b = e.other_vert(a)
                if b.index not in seen:
                    seen.add(b.index)
                    stack.append(b)
        shells.append(members)
    shells.sort(key=len, reverse=True)
    print("shells", len(shells))
    for s in shells[:15]:
        lo = [min(v.co[i] for v in s) for i in range(3)]
        hi = [max(v.co[i] for v in s) for i in range(3)]
        print("  shell verts=%5d lo=(%.2f %.2f %.2f) hi=(%.2f %.2f %.2f)" % (len(s), *lo, *hi))

    print("z slices (trunk, |x|<0.75): z  xmin xmax ymin ymax n")
    rows = defaultdict(list)
    for v in bm.verts:
        if abs(v.co.x) < 0.75:
            rows[round(v.co.z / 0.1)].append(v.co)
    for k in sorted(rows):
        cs = rows[k]
        print("  z=%.1f x[%.2f %.2f] y[%.2f %.2f] n=%d" % (k * 0.1, min(c.x for c in cs), max(c.x for c in cs),
                                                         min(c.y for c in cs), max(c.y for c in cs), len(cs)))
    print("x slices (z>1.2): x  zmin zmax ymin ymax n")
    cols = defaultdict(list)
    for v in bm.verts:
        if v.co.z > 1.2:
            cols[round(v.co.x / 0.1)].append(v.co)
    for k in sorted(cols):
        cs = cols[k]
        print("  x=%.1f z[%.2f %.2f] y[%.2f %.2f] n=%d" % (k * 0.1, min(c.z for c in cs), max(c.z for c in cs),
                                                         min(c.y for c in cs), max(c.y for c in cs), len(cs)))
    print("leg gap: z  inner x of the left leg verts (x>0, z<1.2), n near x=0")
    for zi in range(0, 13):
        z = zi * 0.1
        xs = [v.co.x for v in bm.verts if abs(v.co.z - z) < 0.05]
        pos = [x for x in xs if x > 0]
        print("  z=%.1f min|x|=%.3f xmax=%.2f n=%d" % (z, min(abs(x) for x in xs) if xs else -1, max(pos) if pos else -1, len(xs)))


main()
