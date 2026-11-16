"""
KIND: offline tool (headless Blender).

Reads the exported Publisher FBX files back and checks them against the probe written by export_fbx.py: bone list,
frame range, unit scale, influences per vertex and the world position of every bone at a few frames.

    blender -b --factory-startup --python Tools/Art/publisher_rig/verify_fbx.py -- <fbx_dir> <probe.json> <report.txt>
"""
import json
import os
import sys

import bpy
from mathutils import Vector


def load(path):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=path, automatic_bone_orientation=False, ignore_leaf_bones=False)
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    meshes = [o for o in scene.objects if o.type == "MESH"]
    return scene, arm, meshes


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    probe = json.load(open(args[1]))
    lines, failed = [], 0

    scene, arm, meshes = load(os.path.join(args[0], "SK_Publisher.fbx"))
    mesh = meshes[0]
    bones = [b.name for b in arm.data.bones]
    most = max(len([g for g in v.groups if g.weight > 0]) for v in mesh.data.vertices)
    loose = sum(1 for v in mesh.data.vertices if abs(sum(g.weight for g in v.groups) - 1) > 0.01)
    corners = [mesh.matrix_world @ Vector(c) for c in mesh.bound_box]
    # The importer turns a centimetre file into metres with an object scale of 0.01.
    height = (max(c.z for c in corners) - min(c.z for c in corners)) * 100
    roots = [b.name for b in arm.data.bones if b.parent is None]
    lines.append("SK_Publisher.fbx: object %s, %d bones, roots %s, %d verts, %d materials, %d images, "
                 "influences max %d, not normalised %d, height %.1f cm, armature scale %.4f, actions %d"
                 % (arm.name, len(bones), roots, len(mesh.data.vertices), len(mesh.data.materials),
                    len(bpy.data.images), most, loose, height, arm.scale[0], len(bpy.data.actions)))
    lines.append("bones: " + " ".join(bones))

    for clip in sorted(probe):
        scene, arm, meshes = load(os.path.join(args[0], clip + ".fbx"))
        action = arm.animation_data.action if arm.animation_data else None
        if action is None:
            lines.append("%s.fbx: NO ACTION" % clip)
            failed += 1
            continue
        first, last = (int(round(f)) for f in action.frame_range)
        have = [b.name for b in arm.data.bones]
        same = have == bones
        if set(have) != set(bones):
            lines.append("%s.fbx: FAIL, bones missing %s, extra %s"
                         % (clip, sorted(set(bones) - set(have)), sorted(set(have) - set(bones))))
            failed += 1
            continue
        worst, where = 0.0, ""
        # The exporter writes the range from zero.
        shift = first - probe[clip]["first"]
        for frame, want in probe[clip]["frames"].items():
            scene.frame_set(int(frame) + shift)
            for name, pos in want.items():
                if name not in arm.pose.bones:
                    worst, where = 1e9, "%s is not in the file" % name
                    continue
                got = (arm.matrix_world @ arm.pose.bones[name].head) * 100
                err = (got - Vector(pos)).length
                if err > worst:
                    worst, where = err, "%s@%s" % (name, frame)
        ok = same and worst < 0.5 and last - first == probe[clip]["last"] - probe[clip]["first"] and not meshes
        failed += 0 if ok else 1
        lines.append("%s.fbx: %s, take '%s', frames %d..%d (want %d), fps %d, bones same %s, meshes %d, "
                     "worst bone error %.3f cm (%s)"
                     % (clip, "ok" if ok else "FAIL", action.name, first, last,
                        probe[clip]["last"] - probe[clip]["first"] + 1, scene.render.fps, same, len(meshes), worst,
                        where))
    lines.append("failed: %d" % failed)
    with open(args[2], "w") as out:
        out.write("\n".join(lines) + "\n")


main()
