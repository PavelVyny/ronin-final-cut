"""
KIND: offline tool (headless Blender, read only).

Writes, per clip, where the body parts face (yaw of pelvis, chest, head, feet) and where the feet stand, at a few frames.

    blender -b --factory-startup --python Tools/Art/publisher_rig/clip_facing.py -- <file.blend> <out.txt> [samples=5]
"""
import math
import sys

import bpy
from mathutils import Vector


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    samples = int(args[2]) if len(args) > 2 else 5
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    pb = arm.pose.bones

    def yaw(name):
        d = (pb[name].matrix @ arm.data.bones[name].matrix_local.inverted()).to_3x3() @ Vector((0, -1, 0))
        return math.degrees(math.atan2(d.x, -d.y))

    lines = []
    for action in bpy.data.actions:
        arm.animation_data.action = action
        if action.slots:
            arm.animation_data.action_slot = action.slots[0]
        a, b = int(action.frame_range[0]), int(action.frame_range[1])
        lines.append("== %s %d..%d  (yaw: 0 faces -Y, positive turns to +X)" % (action.name, a, b))
        lines.append("    f  pelvis chest  head footL footR | footL x y z | footR x y z | handL x y z | handR x y z")
        for i in range(samples):
            f = round(a + (b - a) * i / (samples - 1))
            scene.frame_set(f)
            row = "%5d %6.0f %5.0f %5.0f %5.0f %5.0f" % (f, yaw("pelvis"), yaw("spine_03"), yaw("head"), yaw("foot_l"),
                                                     yaw("foot_r"))
            for n in ("foot_l", "foot_r", "hand_l", "hand_r"):
                h = pb[n].head
                row += " | %5.2f %5.2f %5.2f" % (h.x, h.y, h.z)
            lines.append(row)
    with open(args[1], "w") as out:
        out.write("\n".join(lines) + "\n")


main()
