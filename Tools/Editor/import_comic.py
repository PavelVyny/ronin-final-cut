"""Imports the opening comic's five panels (Art/Comic/comic_p*.png, T-093) as UI textures in /Game/UI/Comic, named
T_Comic_1 to T_Comic_5 in the order the comic shows them. Runs inside the editor, headless:

UnrealEditor-Cmd.exe <abs>/JamGame.uproject -run=pythonscript -script="<abs>/Tools/Editor/import_comic.py"

UI group and no mips: the canvas draws them at full size, a streamed-out mip would blur the ink lines.
Running it again replaces them.
"""
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/UI/Comic"
PANELS = ["comic_p1_room", "comic_p2_call", "comic_p3_ship_today", "comic_p4_gauntlet", "comic_p5_reflection"]

tasks = []
for index, panel in enumerate(PANELS):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", ROOT + "Art/Comic/" + panel + ".png")
    task.set_editor_property("destination_path", FOLDER)
    task.set_editor_property("destination_name", "T_Comic_%d" % (index + 1))
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for index in range(len(PANELS)):
    texture = unreal.load_asset("%s/T_Comic_%d" % (FOLDER, index + 1))
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("never_stream", True)
    texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
    texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.log("JAMCOMIC T_Comic_%d %dx%d" % (index + 1, texture.blueprint_get_size_x(), texture.blueprint_get_size_y()))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
