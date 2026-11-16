"""
KIND: offline tool (headless Blender). Per clip of a character: the frame where a hand moves fastest, and where a hand
is lowest, as candidates for the impact frame the game times its hits by (JamFighterView clip table).

    blender -b --factory-startup --python Tools/Art/clip_impacts.py -- <file.blend|.glb> [fps]
"""
import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:]
source = args[0]
fps = float(args[1]) if len(args) > 1 else 30.0
scene = bpy.context.scene
scene.render.fps = int(fps)
if source.endswith(".blend"):
    bpy.ops.wm.open_mainfile(filepath=source)
elif source.endswith(".fbx"):
    bpy.ops.import_scene.fbx(filepath=source)
else:
    bpy.ops.import_scene.gltf(filepath=source)
scene = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
HANDS = [b for b in ("hand_r", "hand_l") if b in arm.pose.bones]
for action in sorted(bpy.data.actions, key=lambda a: a.name):
    if not arm.animation_data:
        arm.animation_data_create()
    arm.animation_data.action = action
    if action.slots:
        arm.animation_data.action_slot = action.slots[0]
    first, last = (int(round(f)) for f in action.frame_range)
    track = {h: [] for h in HANDS}
    for frame in range(first, last + 1):
        scene.frame_set(frame)
        for h in HANDS:
            track[h].append((arm.matrix_world @ arm.pose.bones[h].head).copy())
    line = "%-26s %3d frames" % (action.name, last - first + 1)
    for h in HANDS:
        pts = track[h]
        speeds = [(pts[i + 1] - pts[i - 1]).length / 2 * fps for i in range(1, len(pts) - 1)]
        fast = max(range(len(speeds)), key=lambda i: speeds[i]) + 1
        low = min(range(len(pts)), key=lambda i: pts[i].z)
        line += " | %s fastest f%d (%.2f s, %.1f u/s) lowest f%d (%.2f s)" % (h, fast, fast / fps, speeds[fast - 1], low, low / fps)
    print("[impact] " + line)
