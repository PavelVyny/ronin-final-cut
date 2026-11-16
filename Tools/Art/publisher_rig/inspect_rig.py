"""
KIND: offline tool (headless Blender, read only).

Prints what a Meshy file holds: objects, mesh size, loose parts, the skeleton, the clips and the skin-weight statistics.

    blender -b --factory-startup --python Tools/Art/publisher_rig/inspect_rig.py -- <file.fbx|file.glb>
"""
import sys
from collections import Counter

import bpy


def load(path):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if path.lower().endswith(".fbx"):
        bpy.ops.import_scene.fbx(filepath=path)
    else:
        bpy.ops.import_scene.gltf(filepath=path)


def islands(mesh):
    parent = list(range(len(mesh.vertices)))

    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a

    for e in mesh.edges:
        a, b = find(e.vertices[0]), find(e.vertices[1])
        if a != b:
            parent[a] = b
    return Counter(find(i) for i in range(len(mesh.vertices)))


def main():
    path = sys.argv[sys.argv.index("--") + 1]
    load(path)
    print("== FILE", path)
    print("fps", bpy.context.scene.render.fps, "unit scale", bpy.context.scene.unit_settings.scale_length)
    for ob in bpy.data.objects:
        print("OBJ %-30s %-9s parent=%s loc=%s rot=%s scale=%s" % (
            ob.name, ob.type, ob.parent.name if ob.parent else None,
            tuple(round(v, 3) for v in ob.location), tuple(round(v, 3) for v in ob.rotation_euler),
            tuple(round(v, 3) for v in ob.scale)))
    for ob in bpy.data.objects:
        if ob.type == "MESH":
            me = ob.data
            ws = [ob.matrix_world @ v.co for v in me.vertices]
            lo = [min(c[i] for c in ws) for i in range(3)]
            hi = [max(c[i] for c in ws) for i in range(3)]
            print("MESH %s verts=%d edges=%d polys=%d tris=%d mats=%s uv=%s" % (
                ob.name, len(me.vertices), len(me.edges), len(me.polygons),
                sum(len(p.vertices) - 2 for p in me.polygons), [m.name for m in me.materials], [u.name for u in me.uv_layers]))
            print("  bbox world lo=%s hi=%s" % ([round(v, 3) for v in lo], [round(v, 3) for v in hi]))
            isl = islands(me)
            print("  islands=%d largest=%s" % (len(isl), sorted(isl.values(), reverse=True)[:12]))
            names = [g.name for g in ob.vertex_groups]
            count = Counter()
            total = Counter()
            unweighted = 0
            bad_sum = 0
            per_group = Counter()
            for v in me.vertices:
                gs = [g for g in v.groups if g.weight > 1e-4]
                count[len(gs)] += 1
                s = sum(g.weight for g in gs)
                if not gs:
                    unweighted += 1
                elif abs(s - 1.0) > 0.01:
                    bad_sum += 1
                for g in gs:
                    per_group[names[g.group]] += 1
                    total[names[g.group]] += g.weight
            print("  influences per vertex:", dict(sorted(count.items())), "unweighted", unweighted, "sum!=1", bad_sum)
            for n in names:
                print("  group %-22s verts=%6d weight=%9.1f" % (n, per_group[n], total[n]))
            print("  modifiers", [(m.type, getattr(m, "object", None) and m.object.name) for m in ob.modifiers])
        if ob.type == "ARMATURE":
            print("ARMATURE %s bones=%d" % (ob.name, len(ob.data.bones)))
            for b in ob.data.bones:
                h = ob.matrix_world @ b.head_local
                t = ob.matrix_world @ b.tail_local
                print("  bone %-20s parent=%-18s head=(%.3f %.3f %.3f) tail=(%.3f %.3f %.3f) deform=%s" % (
                    b.name, b.parent.name if b.parent else "-", h.x, h.y, h.z, t.x, t.y, t.z, b.use_deform))
    for a in bpy.data.actions:
        print("ACTION %-40s range=%s" % (a.name, tuple(a.frame_range)))


main()
