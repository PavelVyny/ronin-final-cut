"""Imports the Publisher's base colour (Art/Models/boss_fixed/textures/base_color.png) again over
/Game/Characters/Publisher/Textures/T_Publisher_base_color, keeping the texture's settings. Runs inside the editor,
headless:

UnrealEditor-Cmd.exe <abs>/JamGame.uproject -run=pythonscript -script="<abs>/Tools/Editor/import_publisher_color.py"

For a repaint of the rig's texture (the shirt under the tie, f3a5269): import_models.py brings the mesh and the clips,
but its material keeps pointing at this texture, which it does not import again.
"""
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/Characters/Publisher/Textures"
NAME = "T_Publisher_base_color"
KEPT = ["srgb", "compression_settings", "lod_group", "mip_gen_settings", "never_stream", "address_x", "address_y"]

before = unreal.load_asset("%s/%s" % (FOLDER, NAME))
settings = {key: before.get_editor_property(key) for key in KEPT} if before else {}
task = unreal.AssetImportTask()
task.set_editor_property("filename", ROOT + "Art/Models/boss_fixed/textures/base_color.png")
task.set_editor_property("destination_path", FOLDER)
task.set_editor_property("destination_name", NAME)
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", False)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset("%s/%s" % (FOLDER, NAME))
for key, value in settings.items():
    texture.set_editor_property(key, value)
unreal.log_warning("JAMPUBCOLOR %s %dx%d %s" % (NAME, texture.blueprint_get_size_x(), texture.blueprint_get_size_y(), settings))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
