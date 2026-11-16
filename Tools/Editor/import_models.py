"""Imports the approved 3D models into the project. Runs inside the editor, headless:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/import_models.py"

The hero (Art/Models/hero_fixed, exported by Tools/Art/hero_export.py), the Publisher (Art/Models/boss_fixed) and the
props. Both characters come as one mesh FBX in centimetres plus one FBX per clip; FBX goes through the classic
importer (its options are stable from a script), GLB through Interchange. An asset that exists is imported again
over itself, so the script can be run after every new export.
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
ART = os.path.join(ROOT, "Art")

CHARACTERS = [
    {
        "folder": "/Game/Characters/Hero",
        "source": "Models/hero_fixed",
        "mesh": "SK_Hero",
        "prefix": "A_Hero_",
        "clips": ["Idle", "Walk", "Run", "Right_Hand_Sword_Slash", "Left_Slash", "Sword_Parry", "Hit_Reaction", "Dead",
                  "Roll_Dodge", "Throw_L", "Draw", "Run_ForwardLeft", "Run_ForwardRight", "Run_BackLeft", "Run_BackRight"],
        # Clips whose travel is on root (hero_export.py, boss_fixed/README.md): the game keeps the fighter in place.
        "locked": ["Hit_Reaction", "Dead", "Roll_Dodge", "Draw"],
    },
    {
        "folder": "/Game/Characters/Publisher",
        "source": "Models/boss_fixed",
        "mesh": "SK_Publisher",
        "prefix": "A_Publisher_",
        "clips": ["Idle", "Slow_Orc_Walk", "Heavy_Hammer_Swing", "Charged_Ground_Slam", "Kung_Fu_Punch", "Hit_Reaction",
                  "Knock_Down", "T_Pose"],
        "locked": ["Hit_Reaction", "Knock_Down", "Kung_Fu_Punch"],
    },
]
PROP_MODELS = [
    ("Models/katana/model_glb.glb", "Katana", "SM_Katana"),
    ("Models/shuriken/model_glb.glb", "Shuriken", "SM_Shuriken"),
    # T-100: back to the Hunyuan3D stack; Rodin's (T-099) read as a torn banknote.
    ("Models/cash_stack/model_glb.glb", "CashStack", "SM_CashStack"),
]
PROP_TEXTURES = [
    ("Concepts/r5_dollar_card.png", "T_DollarCard"),
    ("Concepts/r5_memo_card.png", "T_MemoCard"),
]


def log(text):
    unreal.log("import_models: " + text)


def run(task):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def task_for(source, folder, name, options=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(ART, source))
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    if options:
        task.set_editor_property("options", options)
    return task


def fbx_options(skeleton=None):
    ui = unreal.FbxImportUI()
    ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION if skeleton else unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("import_mesh", skeleton is None)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_animations", skeleton is not None)
    ui.set_editor_property("import_materials", skeleton is None)
    ui.set_editor_property("import_textures", skeleton is None)
    ui.set_editor_property("create_physics_asset", False)
    if skeleton:
        ui.set_editor_property("skeleton", skeleton)
        anim = ui.get_editor_property("anim_sequence_import_data")
        anim.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
        anim.set_editor_property("import_bone_tracks", True)
        anim.set_editor_property("remove_redundant_keys", False)
    mesh = ui.get_editor_property("skeletal_mesh_import_data")
    mesh.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
    return ui


def size_of(asset):
    bounds = asset.get_bounds()
    e = bounds.box_extent
    return "size (%.0f %.0f %.0f) cm" % (2 * e.x, 2 * e.y, 2 * e.z)


def import_character(c):
    folder = c["folder"]
    # JAM_IMPORT_CLIPS=Run_BackLeft,Run_BackRight imports only those clips onto the mesh already in the project.
    only_clips = [name for name in os.environ.get("JAM_IMPORT_CLIPS", "").split(",") if name]
    if not only_clips:
        run(task_for("%s/%s.fbx" % (c["source"], c["mesh"]), folder, c["mesh"], fbx_options()))
    mesh = unreal.load_asset("%s/%s" % (folder, c["mesh"]))
    skeleton = mesh.get_editor_property("skeleton") if mesh else None
    if not skeleton:
        log("FAILED %s" % c["mesh"])
        return
    log("%s: %s, skeleton %s" % (c["mesh"], size_of(mesh), skeleton.get_name()))
    for clip in [clip for clip in c["clips"] if not only_clips or clip in only_clips]:
        name = c["prefix"] + clip
        run(task_for("%s/%s.fbx" % (c["source"], clip), folder + "/Anims", name, fbx_options(skeleton)))
        seq = unreal.load_asset("%s/Anims/%s" % (folder, name))
        if not isinstance(seq, unreal.AnimSequence):
            log("FAILED %s" % name)
            continue
        if clip in c["locked"]:
            seq.set_editor_property("force_root_lock", True)
            seq.set_editor_property("root_motion_root_lock", unreal.RootMotionRootLock.REF_POSE)
        log("%s: %.2f s, root lock %s" % (name, seq.get_play_length(), seq.get_editor_property("force_root_lock")))


def import_props():
    # JAM_IMPORT_PROP=CashStack (a prop's folder) limits the run to that prop and skips the cards' textures.
    only = os.environ.get("JAM_IMPORT_PROP", "")
    for source, folder, name in [p for p in PROP_MODELS if not only or p[1] == only]:
        run(task_for(source, "/Game/Props/" + folder, name))
        mesh = unreal.load_asset("/Game/Props/%s/%s" % (folder, name))
        log("%s: %s" % (name, size_of(mesh) if mesh else "FAILED"))
    for source, name in PROP_TEXTURES if not only else []:
        log("%s: %s" % (name, run(task_for(source, "/Game/Props/Textures", name))))


unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
# JAM_IMPORT_ONLY=Hero (or Publisher, Props) limits the run to one part.
ONLY = os.environ.get("JAM_IMPORT_ONLY", "")
for character in CHARACTERS:
    if not ONLY or character["mesh"].endswith(ONLY):
        import_character(character)
if not ONLY or ONLY == "Props":
    import_props()
# The task saves only what it imported; the skeletons, materials and textures made on the way are saved here.
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("done")
