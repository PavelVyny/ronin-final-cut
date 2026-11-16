"""
KIND: offline tool (headless Blender, writes the FBX files of Art/Models/hero_fixed).

Exports the Meshy hero for Unreal the way the Publisher is exported (Tools/Art/publisher_rig/export_fbx.py):
SK_Hero.fbx (mesh, skeleton, embedded textures, no animation) and one armature-only FBX per clip, in centimetres.

Two changes to the Meshy data on the way:
- a `root` bone at the origin on the ground becomes the parent of `pelvis` (Meshy rigs have none);
- in the clips that travel (TRAVEL) the horizontal travel of the hips moves from `pelvis` to `root`, so Unreal's
  Force Root Lock keeps him in place while the hips still drop and sway.

    blender -b --factory-startup --python Tools/Art/hero_export.py -- <out_dir>
"""
import os
import sys

import bpy
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "publisher_rig"))
from export_fbx import export, to_centimetres  # noqa: E402

ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
MODELS = os.path.join(ROOT, "Art", "Models")
CLIPS_GLB = os.path.join(MODELS, "hero_clips", "result.animation_glb_url.glb")
EXTRA = {
    "Walk": os.path.join(MODELS, "hero_rig", "result.basic_animations.walking_glb_url.glb"),
    "Run": os.path.join(MODELS, "hero_rig", "result.basic_animations.running_glb_url.glb"),
}
# The throw (Sun 15 Nov night): Meshy's quick one-arm cast, mirrored so the left hand throws while the katana stays
# in the right. The slow two-hand throw of the same task is not used.
THROWS_GLB = os.path.join(MODELS, "hero_throws", "result.animation_glb_url.glb")
THROW_SOURCE = "mage_soell_cast"
THROW_NAME = "Throw_L"
# The directional runs (T-078): Meshy library clips 5, 6, 9 and 10 on the same rig, in place, renamed by the way they go.
DIRS_GLB = os.path.join(MODELS, "hero_dirs", "result.animation_glb_url.glb")
DIRS = {
    "BackLeft_run": "Run_BackLeft",
    "BackRight_Run": "Run_BackRight",
    "ForwardLeft_Run_Fight": "Run_ForwardLeft",
    "ForwardRight_Run_Fight": "Run_ForwardRight",
}
TRAVEL = ["Hit_Reaction", "Dead", "Roll_Dodge", "Draw"]
# The intro's draw (T-089): Meshy library clip 101, Sword_Shout, on the same rig. Its opening, from the idle stance
# with the blade at the hip, up over the head and down to a guard pointing ahead, sped up to about 1.5 s.
DRAW_GLB = os.path.join(MODELS, "hero_draw", "result.animation_glb_url.glb")
DRAW_SOURCE = "Sword_Shout"
DRAW_LAST_FRAME = 92
DRAW_SPEED = 2.0
FPS = 30


def import_glb(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=path)
    return [o for o in bpy.data.objects if o not in before]


def add_root(arm):
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    bones = arm.data.edit_bones
    root = bones.new("root")
    root.head = Vector((0, 0, 0))
    root.tail = Vector((0, 0.2, 0))
    bones["pelvis"].parent = root
    bpy.ops.object.mode_set(mode="OBJECT")


def mirror_name(name):
    for a, b in (("_l", "_r"), ("_r", "_l")):
        if name.endswith(a):
            return name[: -len(a)] + b
    return name.replace("Left", "#").replace("Right", "Left").replace("#", "Right")


def mirror_action(scene, arm, source, name):
    """A new action that plays source with left and right swapped, mirrored across the armature's X (sideways)
    axis. Each bone takes its partner's change from rest, reflected, on its own rest, parents first.

    Not the partner's reflected pose itself: the Meshy rig's spine bones have their X axis up, not sideways, so a
    reflected pose turned every bone about 180 degrees about its own length and the torso folded in a throw (T-086)."""
    from mathutils import Matrix

    flip = Matrix.Scale(-1.0, 4, (1.0, 0.0, 0.0))
    rest(arm)
    arm.animation_data.action = source
    arm.animation_data.action_slot = source.slots[0]
    first, last = (int(round(f)) for f in source.frame_range)
    poses = {}
    for frame in range(first, last + 1):
        scene.frame_set(frame)
        poses[frame] = {pb.name: pb.matrix.copy() for pb in arm.pose.bones}
    target = bpy.data.actions.new(name)
    target.use_fake_user = True
    rest(arm)
    arm.animation_data.action = target
    if target.slots:
        arm.animation_data.action_slot = target.slots[0]
    order = sorted(arm.pose.bones, key=lambda pb: len(pb.parent_recursive))
    rests = {b.name: b.matrix_local.copy() for b in arm.data.bones}
    for frame, pose in poses.items():
        for pb in order:
            partner = mirror_name(pb.name)
            pb.matrix = flip @ pose[partner] @ rests[partner].inverted() @ flip @ rests[pb.name]
            bpy.context.view_layer.update()
        for pb in order:
            pb.keyframe_insert("location", frame=frame)
            pb.keyframe_insert("rotation_quaternion", frame=frame)
    if not arm.animation_data.action_slot and target.slots:
        arm.animation_data.action_slot = target.slots[0]
    print("[hero] %s: %s mirrored, frames %d..%d" % (name, source.name, first, last))


def import_draw():
    """The draw: Sword_Shout's frames 0..DRAW_LAST_FRAME, played DRAW_SPEED times as fast, renamed Draw."""
    known = set(bpy.data.actions)
    extra = import_glb(DRAW_GLB)
    for action in [a for a in bpy.data.actions if a not in known]:
        if action.name != DRAW_SOURCE:
            bpy.data.actions.remove(action)
    for name in [o.name for o in extra]:
        if name in bpy.data.objects:
            bpy.data.objects.remove(bpy.data.objects[name])
    action = bpy.data.actions[DRAW_SOURCE]
    action.name = "Draw"
    for curve in fcurves_of(action):
        # From the end: a removal shifts the keys after it.
        for index in reversed(range(len(curve.keyframe_points))):
            if curve.keyframe_points[index].co.x > DRAW_LAST_FRAME + 0.5:
                curve.keyframe_points.remove(curve.keyframe_points[index])
        for key in curve.keyframe_points:
            key.co.x /= DRAW_SPEED
            key.handle_left.x /= DRAW_SPEED
            key.handle_right.x /= DRAW_SPEED
        curve.update()
    print("[hero] Draw: %s frames 0..%d at %.1fx" % (DRAW_SOURCE, DRAW_LAST_FRAME, DRAW_SPEED))


def fcurves_of(action):
    """The action's curves: Blender 5 keeps them in layered channelbags, older versions on the action."""
    if hasattr(action, "fcurves") and len(action.fcurves):
        return list(action.fcurves)
    return [c for layer in action.layers for strip in layer.strips for bag in strip.channelbags for c in bag.fcurves]


def rest(arm):
    """Every bone back to its rest pose. Blender keeps an unkeyed bone where the last action left it: without this the
    root of the travel clips leaked into every clip evaluated after them (the hero stood 1 to 4 m off his place)."""
    for pb in arm.pose.bones:
        pb.matrix_basis.identity()
    bpy.context.view_layer.update()


def move_travel(scene, arm, action):
    """Keys root with the horizontal travel of pelvis from the first frame, and pelvis without it."""
    rest(arm)
    arm.animation_data.action = action
    arm.animation_data.action_slot = action.slots[0]
    first, last = (int(round(f)) for f in action.frame_range)
    root, pelvis = arm.pose.bones["root"], arm.pose.bones["pelvis"]
    scene.frame_set(first)
    start = pelvis.matrix.translation.copy()
    frames = {}
    for frame in range(first, last + 1):
        scene.frame_set(frame)
        frames[frame] = pelvis.matrix.copy()
    for frame, hips in frames.items():
        travel = hips.translation - start
        travel.z = 0.0
        root.matrix = Matrix.Translation(travel)
        bpy.context.view_layer.update()
        pelvis.matrix = hips
        bpy.context.view_layer.update()
        root.keyframe_insert("location", frame=frame)
        pelvis.keyframe_insert("location", frame=frame)
        pelvis.keyframe_insert("rotation_quaternion", frame=frame)
    travel = (frames[last].translation - start).to_2d()
    seconds = (last - first) / FPS
    print("[hero] %s: travel %.3f m (x %.3f, y %.3f) in %.2f s moved to root" % (action.name, travel.length, travel.x, travel.y, seconds))


def main():
    out_dir = sys.argv[sys.argv.index("--") + 1]
    os.makedirs(out_dir, exist_ok=True)
    scene = bpy.context.scene
    scene.render.fps = FPS
    for ob in list(bpy.data.objects):
        bpy.data.objects.remove(ob)
    objects = import_glb(CLIPS_GLB)
    arm = next(o for o in objects if o.type == "ARMATURE")
    mesh = next(o for o in objects if o.type == "MESH" and o.parent == arm)
    for ob in objects:
        if ob.type == "MESH" and ob.parent is None:
            bpy.data.objects.remove(ob)  # the bone shape the glTF importer adds
    for name, path in EXTRA.items():
        known = set(bpy.data.actions)
        extra = import_glb(path)
        new = [a for a in bpy.data.actions if a not in known]
        assert len(new) == 1, [a.name for a in new]
        new[0].name = name
        for name in [o.name for o in extra]:
            if name in bpy.data.objects:
                bpy.data.objects.remove(bpy.data.objects[name])
    known = set(bpy.data.actions)
    extra = import_glb(THROWS_GLB)
    for action in [a for a in bpy.data.actions if a not in known]:
        if action.name != THROW_SOURCE:
            bpy.data.actions.remove(action)
    for name in [o.name for o in extra]:
        if name in bpy.data.objects:
            bpy.data.objects.remove(bpy.data.objects[name])
    known = set(bpy.data.actions)
    extra = import_glb(DIRS_GLB)
    for action in [a for a in bpy.data.actions if a not in known]:
        if action.name in DIRS:
            action.name = DIRS[action.name]
        else:
            bpy.data.actions.remove(action)
    for name in [o.name for o in extra]:
        if name in bpy.data.objects:
            bpy.data.objects.remove(bpy.data.objects[name])
    import_draw()
    for action in bpy.data.actions:
        action.use_fake_user = True
    print("[hero] actions: %s" % sorted(a.name for a in bpy.data.actions))
    assert mesh.parent == arm
    # The glTF armature is turned 90 degrees (Y up): made the rest pose, armature space becomes Z-up world space.
    arm.animation_data.action = None
    bpy.ops.object.select_all(action="DESELECT")
    for ob in (arm, mesh):
        ob.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    add_root(arm)
    mirror_action(scene, arm, bpy.data.actions[THROW_SOURCE], THROW_NAME)
    bpy.data.actions.remove(bpy.data.actions[THROW_SOURCE])
    for name in TRAVEL:
        move_travel(scene, arm, bpy.data.actions[name])
    arm.name = "Armature"
    mesh.name = "SK_Hero"
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.matrix_basis.identity()
    bpy.context.view_layer.update()
    zs = [(mesh.matrix_world @ v.co).z for v in mesh.data.vertices]
    print("[hero] rest pose: feet %.3f m, top %.3f m" % (min(zs), max(zs)))
    to_centimetres(scene, arm, mesh)
    export(os.path.join(out_dir, "SK_Hero.fbx"), [arm, mesh], False)
    print("[hero] SK_Hero.fbx: %d bones, %d verts, %.1f cm tall" % (len(arm.data.bones), len(mesh.data.vertices), mesh.dimensions.z))
    for action in sorted(bpy.data.actions, key=lambda a: a.name):
        rest(arm)
        arm.animation_data.action = action
        arm.animation_data.action_slot = action.slots[0]
        first, last = (int(round(f)) for f in action.frame_range)
        scene.frame_start, scene.frame_end = first, last
        export(os.path.join(out_dir, action.name + ".fbx"), [arm], True)
        print("[hero] %s.fbx: frames %d..%d" % (action.name, first, last))


main()
