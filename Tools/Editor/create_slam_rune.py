"""Creates the slam's rune telegraph (the session-5s playtest). Runs inside the editor, headless:

UnrealEditor-Cmd.exe <abs>/JamGame.uproject -run=pythonscript -script="<abs>/Tools/Editor/create_slam_rune.py"

T_JamSlamRune is the white rune seal of Art/Concepts/r10_slam_rune.png (fal, round 10), its outer ring touching the
edges. M_JamSlamRune lays it on a plane as large as the hit's circle: unlit and additive, Color x Intensity. The
parameter Fill (0 to 1, JamFxRules::TelegraphOf) lights the runes from the rim inward, a hot front runs in with it and
the floor inside glows a little more as it closes; at Fill 1 the front reaches the centre, the moment the slam lands.
Nothing shows outside the circle, so the glow is exactly the danger area. Running it again reimports the texture;
an existing material is left alone.
"""
import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/FX"
TEXTURE = "T_JamSlamRune"
MATERIAL = "M_JamSlamRune"

task = unreal.AssetImportTask()
task.set_editor_property("filename", ROOT + "Art/Concepts/r10_slam_rune.png")
task.set_editor_property("destination_path", FOLDER)
task.set_editor_property("destination_name", TEXTURE)
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", False)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset("%s/%s" % (FOLDER, TEXTURE))
texture.set_editor_property("srgb", False)
texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_GRAYSCALE)
texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)

path = "%s/%s" % (FOLDER, MATERIAL)
if unreal.EditorAssetLibrary.does_asset_exist(path):
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    raise SystemExit("create_slam_rune: %s already exists" % path)
lib = unreal.MaterialEditingLibrary
mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(MATERIAL, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
mat.set_editor_property("two_sided", True)

uv = lib.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
sample = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -1000, 0)
sample.set_editor_property("texture", texture)
sample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE)
lib.connect_material_expressions(uv, "", sample, "UVs")
fill = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1000, 300)
fill.set_editor_property("parameter_name", "Fill")
fill.set_editor_property("default_value", 0.0)

custom = lib.create_material_expression(mat, unreal.MaterialExpressionCustom, -700, 100)
custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
custom.set_editor_property("code", "\n".join([
    "float R = distance(UV, float2(0.5, 0.5));",
    "float Inside = saturate((0.5 - R) * 300.0);",
    "float Front = 0.5 * (1.0 - saturate(Fill));",
    "float Lit = saturate((R - Front) * 60.0);",
    "float Edge = saturate(1.0 - abs(R - Front) * 25.0);",
    "float Rim = saturate(1.0 - abs(R - 0.49) * 60.0);",
    "float Glow = 0.06 + 0.22 * Lit * saturate(Fill) + 0.1 * saturate(1.0 - R * 2.0) * Fill * Fill;",
    "return Inside * (Rune * (0.3 + 0.9 * Lit) + 0.6 * Rim + 1.2 * Edge * Fill + Glow);",
]))
inputs = []
for name in ["UV", "Rune", "Fill"]:
    entry = unreal.CustomInput()
    entry.set_editor_property("input_name", name)
    inputs.append(entry)
custom.set_editor_property("inputs", inputs)
lib.connect_material_expressions(uv, "", custom, "UV")
lib.connect_material_expressions(sample, "", custom, "Rune")
lib.connect_material_expressions(fill, "", custom, "Fill")

color = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, 400)
color.set_editor_property("parameter_name", "Color")
color.set_editor_property("default_value", unreal.LinearColor(1.0, 0.22, 0.05, 1.0))
intensity = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 600)
intensity.set_editor_property("parameter_name", "Intensity")
intensity.set_editor_property("default_value", 6.0)
tint = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -400, 300)
lib.connect_material_expressions(color, "", tint, "A")
lib.connect_material_expressions(intensity, "", tint, "B")
emissive = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -200, 100)
lib.connect_material_expressions(custom, "", emissive, "A")
lib.connect_material_expressions(tint, "", emissive, "B")
lib.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(mat)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("JAMRUNE %s and %s saved" % (TEXTURE, MATERIAL))
