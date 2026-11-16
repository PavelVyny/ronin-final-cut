"""Decimate a generated .glb to a triangle budget, keeping its look (T-099).

Runs inside Blender, headless:
    blender --background --factory-startup --python Tools/Art/decimate_glb.py -- <in.glb> <out dir> <triangles> <glb|fbx> [max texture]

glb: writes <out dir>/model.glb with the textures inside (for import_models.py).
fbx: writes <out dir>/model_urls.fbx.fbx and texture_urls.0.<base_color|normal|metallic|roughness>.png, the layout of a
     Meshy prop (for create_arena_assets.py).
Textures larger than max texture (default 2048) are halved until they fit.
"""

import os
import sys

import bpy
import numpy

args = sys.argv[sys.argv.index("--") + 1:]
SOURCE, OUT, BUDGET, MODE = args[0], args[1], int(args[2]), args[3]
MAX_TEXTURE = int(args[4]) if len(args) > 4 else 2048


def triangles(objects):
    return sum(len(p.vertices) - 2 for o in objects for p in o.data.polygons)


def joined_mesh():
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for o in meshes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    return bpy.context.view_layer.objects.active


def decimate(mesh):
    before = triangles([mesh])
    if before > BUDGET:
        modifier = mesh.modifiers.new("Decimate", "DECIMATE")
        modifier.decimate_type = "COLLAPSE"
        modifier.ratio = BUDGET / before
        modifier.use_collapse_triangulate = True
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    print("decimate_glb: %s triangles %d -> %d" % (os.path.basename(SOURCE), before, triangles([mesh])))


def fit(image):
    w, h = image.size
    while max(w, h) > MAX_TEXTURE:
        w, h = w // 2, h // 2
    if (w, h) != tuple(image.size):
        image.scale(w, h)
    return image


def save_pixels(image, path, channel=None):
    """Saves image (or one channel of it, as grey) as a PNG without any colour conversion."""
    fit(image)
    w, h = image.size
    pixels = numpy.empty(w * h * 4, dtype=numpy.float32)
    image.pixels.foreach_get(pixels)
    if channel is not None:
        rgba = pixels.reshape(-1, 4)
        grey = rgba[:, channel].copy()
        rgba[:, 0] = rgba[:, 1] = rgba[:, 2] = grey
        rgba[:, 3] = 1.0
    out = bpy.data.images.new(os.path.basename(path), w, h, alpha=True)
    out.colorspace_settings.name = "Non-Color"
    out.pixels.foreach_set(pixels)
    out.filepath_raw = path
    out.file_format = "PNG"
    out.save()
    print("decimate_glb: %s %dx%d" % (os.path.basename(path), w, h))


def maps(mesh):
    """The base colour, normal and metallic-roughness images of the mesh's first material, as the glTF importer wired them."""
    found = {}
    tree = mesh.data.materials[0].node_tree
    for node in tree.nodes:
        if node.type != "TEX_IMAGE" or not node.image:
            continue
        for link in node.outputs["Color"].links:
            target = link.to_node
            if target.type == "BSDF_PRINCIPLED" and link.to_socket.name == "Base Color":
                found["base_color"] = node.image
            elif target.type == "NORMAL_MAP":
                found["normal"] = node.image
            elif target.type in ("SEPARATE_COLOR", "SEPRGB"):
                found["orm"] = node.image
    return found


bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=SOURCE)
os.makedirs(OUT, exist_ok=True)
mesh = joined_mesh()
decimate(mesh)
if MODE == "glb":
    for image in bpy.data.images:
        if image.size[0] > 0:
            fit(image)
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, "model.glb"), export_format="GLB")
else:
    found = maps(mesh)
    print("decimate_glb: maps %s" % sorted(found))
    save_pixels(found["base_color"], os.path.join(OUT, "texture_urls.0.base_color.png"))
    save_pixels(found["normal"], os.path.join(OUT, "texture_urls.0.normal.png"))
    # glTF packs roughness in green and metallic in blue.
    save_pixels(found["orm"], os.path.join(OUT, "texture_urls.0.roughness.png"), 1)
    save_pixels(found["orm"], os.path.join(OUT, "texture_urls.0.metallic.png"), 2)
    bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, "model_urls.fbx.fbx"), object_types={"MESH"}, path_mode="STRIP",
                             mesh_smooth_type="FACE")
print("decimate_glb: done")
