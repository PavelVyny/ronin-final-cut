"""
KIND: offline tool (headless Blender). Renders a model's base colour flat (no light) from four sides, for checking
a texture: front, right, back, left side by side in one PNG. Optional third argument: a base-colour PNG to put on
the model instead of its own (to compare a repainted texture).

    blender -b --factory-startup --python Tools/Art/flat_views.py -- <model.glb> <out.png> [base_color.png]
"""
import math
import os
import sys

import bpy
from mathutils import Vector

args = sys.argv[sys.argv.index("--") + 1:]
src, out = args[0], args[1]
swap = args[2] if len(args) > 2 else None
for ob in list(bpy.data.objects):
    bpy.data.objects.remove(ob)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.data.objects if o.type == "MESH" and o.parent is not None]
if swap:
    image = bpy.data.images.load(swap)
    for mesh in meshes:
        for slot in mesh.material_slots:
            for node in slot.material.node_tree.nodes:
                feeds = [l.to_socket.name for out in node.outputs for l in out.links]
                if node.type == "TEX_IMAGE" and "Base Color" in feeds:
                    node.image = image
                    print("[views] base colour swapped on %s" % slot.material.name)
scene = bpy.context.scene
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.light = "FLAT"
scene.display.shading.color_type = "TEXTURE"
scene.render.resolution_x, scene.render.resolution_y = 700, 1100
scene.render.film_transparent = False
scene.world = bpy.data.worlds.new("w") if not scene.world else scene.world
scene.display.shading.background_type = "VIEWPORT"
bpy.context.view_layer.update()
pts = [m.matrix_world @ Vector(c) for m in meshes for c in m.bound_box]
lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
centre = (lo + hi) / 2
cam_data = bpy.data.cameras.new("cam")
cam_data.type = "ORTHO"
cam_data.ortho_scale = (hi.z - lo.z) * 1.05
cam = bpy.data.objects.new("cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
frames = []
# glTF forward is +Y after import into Blender's Z-up? The model faces -Y: front camera sits at -Y.
for name, yaw in (("front", 0), ("right", 90), ("back", 180), ("left", 270)):
    a = math.radians(yaw)
    d = 10.0
    cam.location = centre + Vector((d * math.sin(a), -d * math.cos(a), 0))
    cam.rotation_euler = (math.radians(90), 0, a)
    path = out.replace(".png", "_%s.png" % name)
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    frames.append(path)
print("[views] " + " ".join(frames))
