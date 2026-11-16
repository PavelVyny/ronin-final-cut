"""
KIND: offline tool (headless Blender).

Measures the wrists of every clip: the bend of the hand against the forearm (swing) and its turn about the forearm
(twist), relative to the rest pose. A wrist bends about 70 degrees and, with no forearm twist bone, any twist beyond
that reads as a broken hand.

    blender -b --factory-startup --python Tools/Art/publisher_rig/wrist_stats.py -- <file.blend> <out.txt> [clip]

With a clip name the per-frame values of that clip are written as well.
"""
import math
import sys

import bpy
from mathutils import Vector


def swing_twist(q):
    """Splits a rotation into a twist about the bone axis (Y) and the swing that is left."""
    twist = q.copy()
    twist.x, twist.z = 0.0, 0.0
    if twist.magnitude < 1e-8:
        return math.pi, 0.0
    twist.normalize()
    swing = q @ twist.inverted()
    angle = 2 * math.atan2(twist.y, twist.w)
    angle = (angle + math.pi) % (2 * math.pi) - math.pi
    return 2 * math.acos(max(-1.0, min(1.0, abs(swing.w)))), angle


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    detail = args[2] if len(args) > 2 else None
    bones = arm.data.bones
    rest = {s: (bones["lowerarm_" + s].matrix_local.inverted() @ bones["hand_" + s].matrix_local).to_quaternion()
            for s in "lr"}
    lines = []
    for action in sorted(bpy.data.actions, key=lambda a: a.name):
        arm.animation_data.action = action
        arm.animation_data.action_slot = action.slots[0]
        first, last = (int(round(f)) for f in action.frame_range)
        peak = {s: [0.0, 0, 0.0, 0] for s in "lr"}
        rows = []
        for frame in range(first, last + 1):
            scene.frame_set(frame)
            row = [frame]
            for s in "lr":
                fore, hand = arm.pose.bones["lowerarm_" + s], arm.pose.bones["hand_" + s]
                q = rest[s].inverted() @ (fore.matrix.inverted() @ hand.matrix).to_quaternion()
                swing, twist = swing_twist(q)
                swing, twist = math.degrees(swing), math.degrees(twist)
                if swing > peak[s][0]:
                    peak[s][0], peak[s][1] = swing, frame
                if abs(twist) > abs(peak[s][2]):
                    peak[s][2], peak[s][3] = twist, frame
                row += [swing, twist]
            rows.append(row)
        lines.append("%-22s left swing %5.1f@%-3d twist %6.1f@%-3d | right swing %5.1f@%-3d twist %6.1f@%-3d"
                     % (action.name, *peak["l"], *peak["r"]))
        if action.name == detail:
            lines.append("  frame  l_swing l_twist  r_swing r_twist")
            lines += ["  %5d  %7.1f %7.1f  %7.1f %7.1f" % tuple(r) for r in rows]
    with open(args[1], "w") as out:
        out.write("\n".join(lines) + "\n")


main()
