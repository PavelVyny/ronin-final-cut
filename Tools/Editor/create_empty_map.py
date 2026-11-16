"""Creates an empty map asset. Runs inside the editor, headless:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/create_empty_map.py"

The arena is spawned by the game mode at runtime, so the map only has to exist. An existing map is left alone.
"""
import unreal

FOLDER = "/Game/Maps"
NAME = "Arena"
PATH = "%s/%s" % (FOLDER, NAME)

if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    unreal.log("create_empty_map: %s already exists" % PATH)
else:
    world = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, FOLDER, unreal.World, unreal.WorldFactory())
    saved = world is not None and unreal.EditorAssetLibrary.save_asset(PATH, only_if_is_dirty=False)
    unreal.log("create_empty_map: %s saved=%s" % (PATH, saved))
