"""Imports HOW TO PLAY's generated mouse (Art/HowTo/howto_mouse_a.png, T-096) as the UI texture
/Game/UI/HowTo/T_HowToMouse. Runs inside the editor, headless:

UnrealEditor-Cmd.exe <abs>/JamGame.uproject -run=pythonscript -script="<abs>/Tools/Editor/import_howto.py"

UI group and no mips, like the comic's panels. Running it again replaces it.
"""
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/UI/HowTo"

task = unreal.AssetImportTask()
task.set_editor_property("filename", ROOT + "Art/HowTo/howto_mouse_a.png")
task.set_editor_property("destination_path", FOLDER)
task.set_editor_property("destination_name", "T_HowToMouse")
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", False)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset(FOLDER + "/T_HowToMouse")
texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property("never_stream", True)
texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
unreal.log("JAMHOWTO T_HowToMouse %dx%d" % (texture.blueprint_get_size_x(), texture.blueprint_get_size_y()))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
