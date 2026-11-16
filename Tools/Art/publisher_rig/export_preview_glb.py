"""
KIND: offline tool (headless Blender).

Exports the reworked Publisher with all clips as one GLB for the review page (textures reduced to 1024 px), plus the
same file as base64 text, because the artifact host does not serve .glb.

    blender -b --factory-startup --python Tools/Art/publisher_rig/export_preview_glb.py -- <clips.blend> <out.glb>
"""
import base64
import sys

import bpy


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    for image in bpy.data.images:
        if image.size[0] > 1024:
            image.scale(1024, 1024)
    arm = next(o for o in bpy.context.scene.objects if o.type == "ARMATURE")
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.matrix_basis.identity()
    bpy.ops.export_scene.gltf(filepath=args[1], export_format="GLB", export_animations=True,
                              export_animation_mode="ACTIONS", export_force_sampling=True, export_frame_step=1,
                              export_image_format="JPEG", export_jpeg_quality=85, export_skins=True,
                              export_optimize_animation_size=True, export_yup=True)
    with open(args[1], "rb") as src, open(args[1] + ".txt", "w") as dst:
        dst.write(base64.b64encode(src.read()).decode("ascii"))
    print("done", args[1])


main()
