"""Creates the material of the 3D characters (the game loads MI_Hero and MI_Publisher by path: the mesh slots set below
do not survive the save, checked Sun 15 Nov). Runs inside the editor, headless, after
import_models.py:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/create_character_materials.py"

M_JamCharacter is the PBR look of the Meshy textures plus two things the game drives:
- Flash (0 to 1) turns the body white and bright for a hit, FlashColor times FlashGlow in emission;
- the neon: the saturated parts of the base colour (the hero's cyan and magenta piping) shine NeonGlow times.
Metallic and roughness come from any channel of their textures (the hero has them packed in one glTF image, the
Publisher in two grey ones): MetallicChannel and RoughnessChannel pick it.
The textures come from Art/Models/<x>_fixed/textures (Tools/Art/save_textures.py). Running it again rebuilds the
material and the instances.
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/Characters/Materials"
MASTER = "M_JamCharacter"
CHARACTERS = [
    # name, textures folder, mesh, metallic channel, roughness channel, neon glow
    ("Hero", "Art/Models/hero_fixed/textures", "/Game/Characters/Hero/SK_Hero", (0, 0, 1), (0, 1, 0), 3.0),
    ("Publisher", "Art/Models/boss_fixed/textures", "/Game/Characters/Publisher/SK_Publisher", (1, 0, 0), (1, 0, 0), 0.0),
]
MAPS = ["base_color", "normal", "metallic", "roughness"]
# The scene is lit for the dark stand-ins (exposure locked at 1, JamCameraRig): the Meshy albedo is scaled down to match.
ALBEDO = 0.3
# The Publisher is hit all the time: a strong flash kept him white for whole seconds (Sun 15 Nov clip).
FLASH_GLOW = 0.4
# How far a full flash turns the base colour white: all the way blanked the cream-suited Publisher.
FLASH_MIX = 0.35

lib = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(text):
    unreal.log("create_character_materials: " + text)


def import_texture(path, folder, name):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(ROOT, path))
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", False)
    tools.import_asset_tasks([task])
    texture = unreal.load_asset("%s/%s" % (folder, name))
    if name.endswith("_normal"):
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        texture.set_editor_property("srgb", False)
    elif not name.endswith("_base_color"):
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("srgb", False)
    return texture


def node(mat, cls, x, y, **props):
    expr = lib.create_material_expression(mat, cls, x, y)
    for key, value in props.items():
        expr.set_editor_property(key, value)
    return expr


def mask(mat, source, x, y, r=False, g=False, b=False):
    m = node(mat, unreal.MaterialExpressionComponentMask, x, y, r=r, g=g, b=b, a=False)
    lib.connect_material_expressions(source, "", m, "")
    return m


def binary(mat, cls, a, b, x, y):
    n = node(mat, cls, x, y)
    lib.connect_material_expressions(a, "", n, "A")
    lib.connect_material_expressions(b, "", n, "B")
    return n


def scalar(mat, name, value, x, y):
    return node(mat, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)


def vector(mat, name, value, x, y):
    return node(mat, unreal.MaterialExpressionVectorParameter, x, y, parameter_name=name,
                default_value=unreal.LinearColor(*value))


def sample(mat, name, texture, sampler, x, y):
    return node(mat, unreal.MaterialExpressionTextureSampleParameter2D, x, y, parameter_name=name, texture=texture,
                sampler_type=sampler)


def channel(mat, texture_name, texture, channel_name, x, y):
    """dot(texture.rgb, channel.rgb): one channel of a mask texture, picked by a vector parameter."""
    tex = sample(mat, texture_name, texture, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, x, y)
    pick = mask(mat, vector(mat, channel_name, (1, 0, 0, 0), x, y + 220), x + 250, y + 220, True, True, True)
    rgb = mask(mat, tex, x + 250, y, True, True, True)
    return binary(mat, unreal.MaterialExpressionDotProduct, rgb, pick, x + 450, y)


def build_master(textures):
    path = "%s/%s" % (FOLDER, MASTER)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = tools.create_asset(MASTER, FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("used_with_skeletal_mesh", True)

    base = sample(mat, "BaseColor", textures["base_color"], unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, -1600, -400)
    base_rgb = mask(mat, base, -1350, -400, True, True, True)
    flash = scalar(mat, "Flash", 0.0, -1350, -250)
    flash_color = vector(mat, "FlashColor", (1, 1, 1, 1), -1350, -150)
    flash_rgb = mask(mat, flash_color, -1150, -150, True, True, True)
    color = node(mat, unreal.MaterialExpressionLinearInterpolate, -800, -400)
    lit = binary(mat, unreal.MaterialExpressionMultiply, base_rgb, scalar(mat, "Albedo", ALBEDO, -1150, -330), -1000, -400)
    lib.connect_material_expressions(lit, "", color, "A")
    lib.connect_material_expressions(flash_rgb, "", color, "B")
    flash_mix = binary(mat, unreal.MaterialExpressionMultiply, flash, scalar(mat, "FlashMix", FLASH_MIX, -1150, -230), -1000, -250)
    lib.connect_material_expressions(flash_mix, "", color, "Alpha")
    lib.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    normal = sample(mat, "Normal", textures["normal"], unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, -800, 0)
    lib.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    metallic = channel(mat, "Metallic", textures["metallic"], "MetallicChannel", -1600, 250)
    lib.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    rough = channel(mat, "Roughness", textures["roughness"], "RoughnessChannel", -1600, 600)
    lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    # Neon: saturation = max(r, g, b) - min(r, g, b), ramped from NeonFrom over 1 / NeonSharp.
    r = mask(mat, base, -1350, -900, r=True)
    g = mask(mat, base, -1350, -820, g=True)
    b = mask(mat, base, -1350, -740, b=True)
    hi = binary(mat, unreal.MaterialExpressionMax, r, binary(mat, unreal.MaterialExpressionMax, g, b, -1150, -800), -1000, -900)
    lo = binary(mat, unreal.MaterialExpressionMin, r, binary(mat, unreal.MaterialExpressionMin, g, b, -1150, -700), -1000, -760)
    sat = binary(mat, unreal.MaterialExpressionSubtract, hi, lo, -850, -850)
    ramp = binary(mat, unreal.MaterialExpressionMultiply,
                  binary(mat, unreal.MaterialExpressionSubtract, sat, scalar(mat, "NeonFrom", 0.35, -850, -700), -700, -850),
                  scalar(mat, "NeonSharp", 4.0, -700, -700), -550, -850)
    neon_mask = node(mat, unreal.MaterialExpressionSaturate, -400, -850)
    lib.connect_material_expressions(ramp, "", neon_mask, "")
    neon = binary(mat, unreal.MaterialExpressionMultiply,
                  binary(mat, unreal.MaterialExpressionMultiply, base_rgb, neon_mask, -250, -850),
                  scalar(mat, "NeonGlow", 0.0, -250, -700), -100, -850)
    flash_light = binary(mat, unreal.MaterialExpressionMultiply,
                         binary(mat, unreal.MaterialExpressionMultiply, flash_rgb, flash, -800, -150),
                         scalar(mat, "FlashGlow", FLASH_GLOW, -800, -60), -600, -150)
    emissive = binary(mat, unreal.MaterialExpressionAdd, neon, flash_light, 100, -500)
    lib.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
    return mat


def build_instance(master, name, textures, metallic_channel, rough_channel, neon):
    path = "%s/MI_%s" % (FOLDER, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = tools.create_asset("MI_" + name, FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, master)
    for param, key in (("BaseColor", "base_color"), ("Normal", "normal"), ("Metallic", "metallic"), ("Roughness", "roughness")):
        lib.set_material_instance_texture_parameter_value(mi, param, textures[key])
    lib.set_material_instance_vector_parameter_value(mi, "MetallicChannel", unreal.LinearColor(*metallic_channel, 0))
    lib.set_material_instance_vector_parameter_value(mi, "RoughnessChannel", unreal.LinearColor(*rough_channel, 0))
    lib.set_material_instance_scalar_parameter_value(mi, "NeonGlow", neon)
    lib.update_material_instance(mi)
    unreal.EditorAssetLibrary.save_loaded_asset(mi, False)
    return mi


master = None
for name, source, mesh_path, metallic_channel, rough_channel, neon in CHARACTERS:
    textures = {m: import_texture("%s/%s.png" % (source, m), "/Game/Characters/%s/Textures" % name, "T_%s_%s" % (name, m)) for m in MAPS}
    if master is None:
        master = build_master(textures)
    mi = build_instance(master, name, textures, metallic_channel, rough_channel, neon)
    mesh = unreal.load_asset(mesh_path)
    slots = mesh.get_editor_property("materials")
    for slot in slots:
        slot.set_editor_property("material_interface", mi)
    mesh.set_editor_property("materials", slots)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, False)
    log("%s: %d slot(s) wear %s" % (mesh_path, len(slots), mi.get_name()))


def build_ghost():
    """M_JamGhost (T-064): the hero's neon afterimages. Unlit and additive: Color times Glow times Fade, brightest at
    the rim (a Fresnel term), so a ghost reads as a glowing outline of the pose."""
    path = "%s/M_JamGhost" % FOLDER
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
    else:
        mat = tools.create_asset("M_JamGhost", FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    mat.set_editor_property("used_with_skeletal_mesh", True)
    color = mask(mat, vector(mat, "Color", (0.25, 0.95, 1.0, 1.0), -900, 0), -700, 0, True, True, True)
    rim = node(mat, unreal.MaterialExpressionFresnel, -900, 200)
    rim.set_editor_property("exponent", 2.0)
    lit = binary(mat, unreal.MaterialExpressionAdd, rim, scalar(mat, "Fill", 0.25, -900, 350), -700, 250)
    glow = binary(mat, unreal.MaterialExpressionMultiply, scalar(mat, "Glow", 3.0, -700, 400), scalar(mat, "Fade", 1.0, -700, 500), -500, 450)
    out = binary(mat, unreal.MaterialExpressionMultiply, binary(mat, unreal.MaterialExpressionMultiply, color, lit, -500, 100), glow, -300, 250)
    lib.connect_material_property(out, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
    log("M_JamGhost built")


build_ghost()
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("done")
