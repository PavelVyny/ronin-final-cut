"""Creates the material of the paper props (T-062): the Publisher's memos and the dollar bills. Runs inside the editor,
headless, after import_models.py:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/create_card_material.py"

M_JamCard is unlit and two-sided: Card (the texture) times Color times Glow, so a Glow above 1 blooms like the neon of
the stand-ins. MI_MemoCard and MI_DollarCard carry the round-5 textures. Running it again rebuilds them.
"""
import unreal

FOLDER = "/Game/Props/Materials"
MASTER = "M_JamCard"
CARDS = [("MI_MemoCard", "/Game/Props/Textures/T_MemoCard"), ("MI_DollarCard", "/Game/Props/Textures/T_DollarCard")]

lib = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(text):
    unreal.log("create_card_material: " + text)


def node(mat, cls, x, y, **props):
    expr = lib.create_material_expression(mat, cls, x, y)
    for key, value in props.items():
        expr.set_editor_property(key, value)
    return expr


def binary(mat, cls, a, b, x, y):
    n = node(mat, cls, x, y)
    lib.connect_material_expressions(a, "", n, "A")
    lib.connect_material_expressions(b, "", n, "B")
    return n


path = "%s/%s" % (FOLDER, MASTER)
if unreal.EditorAssetLibrary.does_asset_exist(path):
    mat = unreal.load_asset(path)
    lib.delete_all_material_expressions(mat)
else:
    mat = tools.create_asset(MASTER, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("two_sided", True)
first = unreal.load_asset(CARDS[0][1])
card = node(mat, unreal.MaterialExpressionTextureSampleParameter2D, -900, 0, parameter_name="Card", texture=first)
rgb = node(mat, unreal.MaterialExpressionComponentMask, -650, 0, r=True, g=True, b=True, a=False)
lib.connect_material_expressions(card, "", rgb, "")
color = node(mat, unreal.MaterialExpressionVectorParameter, -900, 250, parameter_name="Color",
             default_value=unreal.LinearColor(1, 1, 1, 1))
color_rgb = node(mat, unreal.MaterialExpressionComponentMask, -650, 250, r=True, g=True, b=True, a=False)
lib.connect_material_expressions(color, "", color_rgb, "")
glow = node(mat, unreal.MaterialExpressionScalarParameter, -650, 400, parameter_name="Glow", default_value=1.0)
lit = binary(mat, unreal.MaterialExpressionMultiply, binary(mat, unreal.MaterialExpressionMultiply, rgb, color_rgb, -450, 100), glow, -250, 200)
lib.connect_material_property(lit, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat, False)

for name, texture_path in CARDS:
    mi_path = "%s/%s" % (FOLDER, name)
    mi = unreal.load_asset(mi_path) if unreal.EditorAssetLibrary.does_asset_exist(mi_path) else tools.create_asset(
        name, FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, mat)
    lib.set_material_instance_texture_parameter_value(mi, "Card", unreal.load_asset(texture_path))
    lib.update_material_instance(mi)
    unreal.EditorAssetLibrary.save_loaded_asset(mi, False)
    log("%s wears %s" % (name, texture_path))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("done")
