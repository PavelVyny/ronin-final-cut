"""
KIND: offline tool (headless Blender, read only).

Prints, per clip of the rig file, what the motion does: pelvis travel, trunk lean and yaw, arm angles, foot heights.

    blender -b --factory-startup --python Tools/Art/publisher_rig/clip_stats.py -- <file.blend> [every=10]
"""
import math
import sys

import bpy
from mathutils import Vector


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    every = int(args[1]) if len(args) > 1 else 10
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    pb = arm.pose.bones
    for action in bpy.data.actions:
        arm.animation_data.action = action
        if action.slots:
            arm.animation_data.action_slot = action.slots[0]
        a, b = int(action.frame_range[0]), int(action.frame_range[1])
        print("== %s %d..%d" % (action.name, a, b))
        print("   f   pelvis(x y z)      yaw  lean | spine3 lean | armL dn/fw  armR dn/fw | footL z  footR z | head yaw")
        for f in list(range(a, b + 1, every)) + ([b] if (b - a) % every else []):
            scene.frame_set(f)
            p = pb["pelvis"]
            def delta(name):
                return (pb[name].matrix @ arm.data.bones[name].matrix_local.inverted()).to_3x3()

            m = delta("pelvis")
            up = m @ Vector((0, 0, 1))
            back = m @ Vector((0, 1, 0))
            yaw = math.degrees(math.atan2(-back.x, back.y))
            lean = math.degrees(math.acos(max(-1, min(1, up.z))))
            up3 = delta("spine_03") @ Vector((0, 0, 1))
            lean3 = math.degrees(math.acos(max(-1, min(1, up3.z))))
            arms = []
            for s in ("l", "r"):
                d = (pb["lowerarm_" + s].head - pb["upperarm_" + s].head).normalized()
                arms.append((math.degrees(math.asin(max(-1, min(1, -d.z)))), -d.y))
            hb = delta("head") @ Vector((0, 1, 0))
            print("%4d  %6.2f %6.2f %5.2f  %5.0f %5.0f | %5.0f | %4.0f %5.2f  %4.0f %5.2f | %5.2f %5.2f | %5.0f" % (
                f, p.head.x, p.head.y, p.head.z, yaw, lean, lean3, arms[0][0], arms[0][1], arms[1][0], arms[1][1],
                pb["foot_l"].head.z, pb["foot_r"].head.z, math.degrees(math.atan2(-hb.x, hb.y))))


main()
