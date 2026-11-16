"""
KIND: offline tool (headless Blender). Saves the textures of a character's material as PNG files for Unreal, named by
the socket they feed (base_color, normal, metallic, roughness): the FBX importer does not take the embedded ones.

    blender -b --factory-startup --python Tools/Art/save_textures.py -- <source.blend|.glb> <out_dir>
"""
import os
import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:]
source, out_dir = args[0], args[1]
os.makedirs(out_dir, exist_ok=True)
if source.endswith(".blend"):
    bpy.ops.wm.open_mainfile(filepath=source)
else:
    bpy.ops.import_scene.gltf(filepath=source)


def feeds(node, seen=None):
    """The Principled BSDF inputs this node's output reaches, through normal-map and separate nodes."""
    seen = seen or set()
    names = set()
    for out in node.outputs:
        for link in out.links:
            to = link.to_node
            if to.type == "BSDF_PRINCIPLED":
                names.add(link.to_socket.name)
            elif to.name not in seen:
                seen.add(to.name)
                names |= feeds(to, seen)
    return names


SOCKETS = {"Base Color": "base_color", "Normal": "normal", "Metallic": "metallic", "Roughness": "roughness"}
for mesh in [o for o in bpy.data.objects if o.type == "MESH" and o.parent and o.parent.type == "ARMATURE"]:
    for slot in mesh.material_slots:
        for node in slot.material.node_tree.nodes:
            if node.type != "TEX_IMAGE" or not node.image:
                continue
            for socket in sorted(feeds(node)):
                if socket not in SOCKETS:
                    continue
                path = os.path.join(out_dir, "%s.png" % SOCKETS[socket])
                image = node.image.copy()
                image.filepath_raw = path
                image.file_format = "PNG"
                image.save()
                print("[tex] %s <- %s (%dx%d)" % (path, node.image.name, image.size[0], image.size[1]))
