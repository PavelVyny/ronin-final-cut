"""
KIND: offline tool (headless Blender, writes the FBX files of Art/Models/boss_fixed).

Exports the reworked Publisher for Unreal: SK_Publisher.fbx (mesh, skeleton, embedded textures, no animation) and one
armature-only FBX per clip, named as the clip. The data is converted to centimetres first (rest bones, mesh and
location keys times 100, scene unit 0.01), so the file has unit scale 1 and no scaled armature node: root motion
comes into Unreal in true size whatever the importer does with units.

    blender -b --factory-startup --python Tools/Art/publisher_rig/export_fbx.py -- <clips.blend> <out_dir> [probe.json]

probe.json receives world positions (cm) of every bone at a few frames of every clip, for verify_fbx.py.
"""
import json
import os
import sys

import bpy

SCALE = 100.0
MESH_FILE = "SK_Publisher.fbx"


def fcurves(action):
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                yield from bag.fcurves


def to_centimetres(scene, arm, mesh):
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0 / SCALE
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.matrix_basis.identity()
    bpy.ops.object.select_all(action="DESELECT")
    # The mesh is a child of the armature and takes its scale from it.
    assert mesh.parent == arm
    height = mesh.dimensions.z
    arm.scale = (SCALE, SCALE, SCALE)
    for ob in (arm, mesh):
        ob.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.context.view_layer.update()
    assert abs(mesh.dimensions.z - height * SCALE) < 0.01 * height * SCALE, mesh.dimensions.z
    assert all(abs(s - 1) < 1e-6 for ob in (arm, mesh) for s in ob.matrix_world.to_scale())
    for action in bpy.data.actions:
        for fc in fcurves(action):
            if fc.data_path.endswith(".location"):
                for key in fc.keyframe_points:
                    key.co[1] *= SCALE
                    key.handle_left[1] *= SCALE
                    key.handle_right[1] *= SCALE
                fc.update()


def export(path, objects, animated):
    bpy.ops.object.select_all(action="DESELECT")
    for ob in objects:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.export_scene.fbx(
        filepath=path, use_selection=True, object_types={"ARMATURE", "MESH"}, global_scale=1.0,
        apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", axis_forward="-Z", axis_up="Y",
        add_leaf_bones=False, primary_bone_axis="Y", secondary_bone_axis="X", use_armature_deform_only=False,
        armature_nodetype="NULL", mesh_smooth_type="FACE", use_tspace=False, use_mesh_modifiers=True,
        bake_anim=animated, bake_anim_use_all_bones=True, bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False, bake_anim_force_startend_keying=True, bake_anim_step=1.0,
        bake_anim_simplify_factor=0.0, path_mode="COPY", embed_textures=not animated)


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    out_dir = args[1]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    mesh = next(o for o in scene.objects if o.type == "MESH")
    arm.name = "Armature"
    mesh.name = "SK_Publisher"
    to_centimetres(scene, arm, mesh)
    scene.render.fps = 30

    export(os.path.join(out_dir, MESH_FILE), [arm, mesh], False)
    print("[fbx] %s: %d bones, %d verts" % (MESH_FILE, len(arm.data.bones), len(mesh.data.vertices)))

    probe = {}
    for action in sorted(bpy.data.actions, key=lambda a: a.name):
        arm.animation_data.action = action
        arm.animation_data.action_slot = action.slots[0]
        first, last = (int(round(f)) for f in action.frame_range)
        scene.frame_start, scene.frame_end = first, last
        scene.name = action.name
        export(os.path.join(out_dir, action.name + ".fbx"), [arm], True)
        probe[action.name] = {"first": first, "last": last, "frames": {}}
        for frame in sorted({first, (first + last) // 2, last}):
            scene.frame_set(frame)
            probe[action.name]["frames"][str(frame)] = {
                pb.name: [round(c, 3) for c in (arm.matrix_world @ pb.head)] for pb in arm.pose.bones}
        print("[fbx] %s.fbx: frames %d..%d" % (action.name, first, last))
    if len(args) > 2:
        with open(args[2], "w") as out:
            json.dump(probe, out)


if __name__ == "__main__":
    main()
