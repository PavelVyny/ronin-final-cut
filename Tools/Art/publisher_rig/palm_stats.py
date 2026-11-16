"""
KIND: offline tool (headless Blender).

Writes, per frame of one clip and per arm, where the arm points and where the palm faces, in the frame of the chest
(right, forward, up of spine_03), plus the elbow bend and the bend of the hand against the forearm. The palm is taken
as facing down in the rest pose (arms out).

    blender -b --factory-startup --python Tools/Art/publisher_rig/palm_stats.py -- <file.blend> <out.txt> <clip> [step]
"""
import math
import sys

import bpy
from mathutils import Vector


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    step = int(args[3]) if len(args) > 3 else 1
    bones = arm.data.bones
    action = bpy.data.actions[args[2]]
    arm.animation_data.action = action
    arm.animation_data.action_slot = action.slots[0]
    palm_local = {s: bones["hand_" + s].matrix_local.to_3x3().inverted() @ Vector((0, 0, -1)) for s in "lr"}
    chest_rest = bones["spine_03"].matrix_local.to_3x3()
    first, last = (int(round(f)) for f in action.frame_range)
    lines = ["chest frame: x = his left, y = back, z = up (as at rest). dir = shoulder to wrist, palm = palm normal",
             "frame | L dir(x y z) palm(x y z) elbow hand | R dir(x y z) palm(x y z) elbow hand"]
    for frame in range(first, last + 1, step):
        scene.frame_set(frame)
        chest = (arm.pose.bones["spine_03"].matrix.to_3x3() @ chest_rest.inverted()).inverted()
        row = "%5d" % frame
        for s in "lr":
            upper, fore, hand = (arm.pose.bones[n + s] for n in ("upperarm_", "lowerarm_", "hand_"))
            direction = chest @ (hand.head - upper.head).normalized()
            palm = chest @ (hand.matrix.to_3x3() @ palm_local[s])
            elbow = math.degrees((fore.head - upper.head).angle(hand.head - fore.head))
            bend = math.degrees((hand.head - fore.head).angle(hand.matrix.to_3x3() @ Vector((0, 1, 0))))
            row += " | %5.2f %5.2f %5.2f  %5.2f %5.2f %5.2f  %4.0f %4.0f" % (*direction, *palm, elbow, bend)
        lines.append(row)
    with open(args[1], "w") as out:
        out.write("\n".join(lines) + "\n")


main()
