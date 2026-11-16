"""Creates the neon material of the stand-ins (T-027). Runs inside the editor, headless:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/create_neon_material.py"

M_JamNeon is unlit: its colour is all emission, Color times Glow, so a Glow above 1 blooms. An existing material is
left alone.
"""
import unreal

FOLDER = "/Game/Materials"
NAME = "M_JamNeon"
PATH = "%s/%s" % (FOLDER, NAME)

if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    unreal.log("create_neon_material: %s already exists" % PATH)
else:
    lib = unreal.MaterialEditingLibrary
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    color = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, 0)
    color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    glow = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 250)
    glow.set_editor_property("parameter_name", "Glow")
    glow.set_editor_property("default_value", 1.0)
    times = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    lib.connect_material_expressions(color, "", times, "A")
    lib.connect_material_expressions(glow, "", times, "B")
    lib.connect_material_property(times, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(mat)
    saved = unreal.EditorAssetLibrary.save_asset(PATH, only_if_is_dirty=False)
    unreal.log("create_neon_material: %s saved=%s" % (PATH, saved))
