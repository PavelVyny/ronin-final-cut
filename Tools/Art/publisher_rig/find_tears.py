"""
KIND: offline tool (headless Blender).

Finds where the skin tears: vertices that touch at rest (closer than 2.5 cm) but move apart in a pose. Writes the worst
pairs with their weights, so a layer that leaves the layer under it (a lapel, a tie) shows up with its cause.

    blender -b --factory-startup --python Tools/Art/publisher_rig/find_tears.py -- <file.blend> <out.txt> <clip> <frame>
"""
import sys

import bpy
import numpy as np
from mathutils import kdtree


def weights(mesh, i):
    groups = mesh.vertex_groups
    return " ".join("%s=%.2f" % (groups[g.group].name, g.weight)
                    for g in sorted(mesh.data.vertices[i].groups, key=lambda g: -g.weight) if g.weight > 0.01)


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    mesh = next(o for o in scene.objects if o.type == "MESH")
    n = len(mesh.data.vertices)
    rest = np.array([v.co[:] for v in mesh.data.vertices])
    action = bpy.data.actions[args[2]]
    arm.animation_data.action = action
    arm.animation_data.action_slot = action.slots[0]
    scene.frame_set(int(args[3]))
    deps = bpy.context.evaluated_depsgraph_get()
    posed = np.array([v.co[:] for v in mesh.evaluated_get(deps).data.vertices])
    assert len(posed) == n
    tree = kdtree.KDTree(n)
    for i, c in enumerate(rest):
        tree.insert(c, i)
    tree.balance()
    worst = np.zeros(n)
    mate = np.full(n, -1)
    for i in range(n):
        for _, j, d in tree.find_range(rest[i], 0.025):
            if j == i:
                continue
            gap = float(np.linalg.norm(posed[i] - posed[j])) - d
            if gap > worst[i]:
                worst[i], mate[i] = gap, j
    order = np.argsort(-worst)
    with open(args[1], "w") as out:
        out.write("%s frame %s: %d verts, %d open more than 1 cm, %d more than 2 cm\n"
                  % (args[2], args[3], n, (worst > 0.01).sum(), (worst > 0.02).sum()))
        # Histogram of the open vertices by height and side, to see where the tears are.
        for z0 in np.arange(0.0, 2.8, 0.2):
            band = (rest[:, 2] >= z0) & (rest[:, 2] < z0 + 0.2) & (worst > 0.01)
            if band.any():
                out.write("z %.1f..%.1f: %5d open, worst %.3f\n" % (z0, z0 + 0.2, band.sum(), worst[band].max()))
        seen = []
        for i in order:
            if worst[i] < 0.01 or len(seen) >= 40:
                break
            if any(np.linalg.norm(rest[i] - rest[s]) < 0.06 for s in seen):
                continue
            seen.append(i)
            j = mate[i]
            out.write("\ngap %.3f at rest (%.2f %.2f %.2f)\n  a %s\n  b %s\n"
                      % (worst[i], rest[i][0], rest[i][1], rest[i][2], weights(mesh, i), weights(mesh, j)))


main()
