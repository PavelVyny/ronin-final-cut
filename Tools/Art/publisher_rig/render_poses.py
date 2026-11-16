"""
KIND: offline tool (headless Blender, writes PNG frames only).

Renders a rigged character in the frames of its clips with the Workbench engine: textured, or coloured by the skin
weights (each bone its own colour, blended by weight), so bad deformation and its cause sit side by side.

    blender -b --factory-startup --python Tools/Art/publisher_rig/render_poses.py -- <file.fbx|.glb|.blend> <out_dir>
        [--clips Idle,Slow_Orc_Walk] [--frames 6 | --at 1,20,40] [--views front,q3,side,back] [--mode texture|weights]
        [--size 512] [--rest] [--step 0] [--scale 3.6] [--focus neck_01]

Output: <out_dir>/<clip>_<view>_<frame:04d>.png. --rest renders the rest pose as clip "rest". --step N renders every
N-th frame of the clip instead of --frames (for a GIF). --focus centres the frame on a bone (with a small --scale: a
close-up).
"""
import argparse
import colorsys
import math
import os
import sys

import bpy
from mathutils import Vector

VIEWS = {  # the character faces -Y
    "front": (0.0, -1.0), "q3": (-0.62, -0.78), "side": (-1.0, 0.0), "back": (0.0, 1.0), "q3b": (0.62, 0.78),
    "q3r": (0.62, -0.78),
}


def parse():
    ap = argparse.ArgumentParser()
    ap.add_argument("file")
    ap.add_argument("out")
    ap.add_argument("--clips", default="")
    ap.add_argument("--frames", type=int, default=6)
    ap.add_argument("--at", default="")
    ap.add_argument("--views", default="q3")
    ap.add_argument("--mode", default="texture")
    ap.add_argument("--size", type=int, default=512)
    ap.add_argument("--rest", action="store_true")
    ap.add_argument("--step", type=int, default=0)
    ap.add_argument("--scale", type=float, default=3.6)
    ap.add_argument("--elev", type=float, default=8.0)
    ap.add_argument("--focus", default="")
    return ap.parse_args(sys.argv[sys.argv.index("--") + 1:])


def load(path):
    if path.lower().endswith(".blend"):
        bpy.ops.wm.open_mainfile(filepath=path)
        return
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if path.lower().endswith(".fbx"):
        bpy.ops.import_scene.fbx(filepath=path)
    else:
        bpy.ops.import_scene.gltf(filepath=path)


def bone_colour(name, index, count):
    """Left and right limbs get clearly different hues; the trunk chain runs red to yellow."""
    trunk = {"pelvis": 0.0, "spine_01": 0.04, "spine_02": 0.08, "spine_03": 0.13, "neck_01": 0.17, "head": 0.3,
             "belly": 0.92, "root": 0.0}
    if name in trunk:
        return colorsys.hsv_to_rgb(trunk[name], 0.9, 1.0 if name != "pelvis" else 0.7)
    h = (index * 0.618034) % 1.0
    if name.endswith("_l") or name.startswith("Left"):
        h = 0.5 + 0.2 * h
    elif name.endswith("_r") or name.startswith("Right"):
        h = 0.72 + 0.16 * h
    return colorsys.hsv_to_rgb(h, 0.75, 0.95)


def paint_weights(ob):
    me = ob.data
    names = [g.name for g in ob.vertex_groups]
    cols = [bone_colour(n, i, len(names)) for i, n in enumerate(names)]
    attr = me.color_attributes.get("WeightView") or me.color_attributes.new("WeightView", "FLOAT_COLOR", "POINT")
    for v in me.vertices:
        r = g = b = 0.0
        for e in v.groups:
            c = cols[e.group]
            r += c[0] * e.weight
            g += c[1] * e.weight
            b += c[2] * e.weight
        attr.data[v.index].color = (r, g, b, 1.0)
    me.color_attributes.active_color = attr
    me.color_attributes.render_color_index = list(me.color_attributes).index(attr)


def set_action(arm, action):
    if arm.animation_data is None:
        arm.animation_data_create()
    arm.animation_data.action = action
    if action is not None and hasattr(arm.animation_data, "action_slot") and getattr(action, "slots", None):
        arm.animation_data.action_slot = action.slots[0]


def main():
    args = parse()
    load(os.path.abspath(args.file))
    os.makedirs(args.out, exist_ok=True)
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    meshes = [o for o in scene.objects if o.type == "MESH" and any(m.type == "ARMATURE" for m in o.modifiers)]
    for o in scene.objects:
        if o.type == "MESH" and o not in meshes:
            o.hide_render = True
    arm.hide_render = True

    scene.render.engine = "BLENDER_WORKBENCH"
    scene.render.resolution_x = scene.render.resolution_y = args.size
    scene.render.film_transparent = False
    scene.render.image_settings.file_format = "PNG"
    sh = scene.display.shading
    sh.light = "STUDIO"
    sh.show_cavity = True
    sh.cavity_type = "BOTH"
    sh.show_shadows = False
    sh.show_specular_highlight = False
    sh.background_type = "WORLD" if scene.world else "THEME"
    if args.mode == "weights":
        for ob in meshes:
            paint_weights(ob)
        sh.color_type = "VERTEX"
    else:
        sh.color_type = "TEXTURE"
    if scene.world is None:
        scene.world = bpy.data.worlds.new("W")
    scene.world.color = (0.16, 0.17, 0.2)
    sh.background_type = "WORLD"
    scene.view_settings.view_transform = "Standard"

    cam_data = bpy.data.cameras.new("PoseCam")
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = args.scale
    cam_data.clip_end = 100
    cam = bpy.data.objects.new("PoseCam", cam_data)
    scene.collection.objects.link(cam)
    scene.camera = cam

    wanted = [c for c in args.clips.split(",") if c]
    jobs = []
    if args.rest:
        jobs.append(("rest", None))
    for a in bpy.data.actions:
        short = a.name.split("|")[-1] if a.name.count("|") == 1 else a.name
        if "rest" in a.name.lower() and "|" in a.name:
            continue
        if not wanted or short in wanted:
            jobs.append((short, a))

    pelvis = arm.pose.bones.get("pelvis")
    for short, action in jobs:
        if action is None:
            set_action(arm, None)
            for pb in arm.pose.bones:
                pb.matrix_basis.identity()
            frames = [0]
        else:
            set_action(arm, action)
            a, b = int(round(action.frame_range[0])), int(round(action.frame_range[1]))
            if args.at:
                frames = [int(f) for f in args.at.split(",")]
            elif args.step:
                frames = list(range(a, b + 1, args.step))
            else:
                n = max(2, args.frames)
                frames = [round(a + (b - a) * i / (n - 1)) for i in range(n)] if n > 1 else [a]
                frames = [min(f, b - 1) if i == n - 1 and short in ("Idle", "Slow_Orc_Walk") else f for i, f in enumerate(frames)]
        for f in frames:
            scene.frame_set(f)
            bpy.context.view_layer.update()
            centre = (arm.matrix_world @ pelvis.head) if pelvis else Vector((0, 0, 1.2))
            target = Vector((centre.x, centre.y, 1.25))
            if args.focus:
                target = arm.matrix_world @ arm.pose.bones[args.focus].head
            for view in args.views.split(","):
                dx, dy = VIEWS[view]
                el = math.radians(args.elev)
                d = Vector((dx * math.cos(el), dy * math.cos(el), math.sin(el))).normalized()
                cam.location = target + d * 12.0
                cam.rotation_euler = (-d).to_track_quat("-Z", "Y").to_euler()
                scene.render.filepath = os.path.join(args.out, "%s_%s_%04d.png" % (short, view, f))
                bpy.ops.render.render(write_still=True)
    print("[render_poses] done", args.out)


main()
