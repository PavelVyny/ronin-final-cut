"""Imports the rooftop shrine of the full art (T-081) and builds its materials. Runs inside the editor, headless:

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/create_arena_assets.py"

- The five Meshy props (Art/Models/arena_*) as static meshes in /Game/Arena/Props, with their PBR maps, on
  M_JamArenaProp: the Meshy look scaled to the dark scene, plus Glow on the bright parts of the base colour (the
  lantern glass) in GlowColor. The lantern pillars get a cyan and a magenta instance.
- M_JamArenaFloor: the PBRgen slate (Art/Materials/slate) tiled in world space, wet (roughness scaled down, the low
  spots of the height map darker and glossy as puddles), with the engraved crest (r7_floor_crest) over the centre.
  Its relief: the normal's tilt scaled (NormalStrength), the gaps and chips darkened (CavityDarken), and a world-space
  parallax from the height map (ParallaxHeight).
  CrestRadius is set by the game to the arena radius.
- M_JamArenaCity: the city far below (r7_city_backdrop), unlit.
- M_JamMenuBackdrop: the skyline behind the two fighters on the main menu (r9_menu_backdrop_wide, r8_menu_backdrop_b outpainted to 3:1 in T-094), unlit (T-092).
Running it again rebuilds everything over itself.
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/Arena"
PROPS = ["lantern_pillar", "torii", "stone_lantern", "bonsai", "railing"]
# T-099: the stone lantern is Hyper3D Rodin Gen-2.5's, decimated to ~9k triangles by Tools/Art/decimate_glb.py in the same layout.
PROP_SOURCES = {"stone_lantern": "Art/Models/rodin/stone_lantern_lod"}
MAPS = ["base_color", "normal", "metallic", "roughness"]
# The scene is lit for the dark stand-ins (as M_JamCharacter): the Meshy albedo is scaled down to match.
ALBEDO = 0.35
# The lantern glass and paper: base colour above this luminance glows.
GLOW_FROM = 0.6
PILLAR_GLOWS = {"Cyan": (0.0, 0.8, 1.0), "Magenta": (1.0, 0.1, 0.6)}
STONE_LANTERN_GLOW = (1.0, 0.45, 0.12)
# PBRgen export, Unreal preset, renamed into Art/Materials/slate: BC as color, N as normal, H as height and the packed
# ORM as roughness (its G channel is the roughness).
SLATE = "Art/Materials/slate"
SLATE_MAPS = {"color": "T_Slate_Color", "normal": "T_Slate_Normal", "roughness": "T_Slate_Roughness", "height": "T_Slate_Height"}
# One slate tile repeat in world cm.
SLATE_TILE = 700.0
# The slate's relief (the author's "the floor reads flat", Mon 16 Nov): the normal's tilt, the gaps darkened, the
# parallax shift in tile units at a grazing view.
NORMAL_STRENGTH = 2.5
CAVITY_DARKEN = 0.6
PARALLAX_HEIGHT = 0.02

lib = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(text):
    unreal.log("create_arena_assets: " + text)


def import_file(path, folder, name, options=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(ROOT, path))
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", False)
    if options:
        task.set_editor_property("options", options)
    tools.import_asset_tasks([task])
    return unreal.load_asset("%s/%s" % (folder, name))


def import_texture(path, folder, name, data=False, normal=False):
    texture = import_file(path, folder, name)
    if normal:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        texture.set_editor_property("srgb", False)
    elif data:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("srgb", False)
    return texture


def mesh_options():
    ui = unreal.FbxImportUI()
    ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", False)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    data = ui.get_editor_property("static_mesh_import_data")
    data.set_editor_property("combine_meshes", True)
    data.set_editor_property("auto_generate_collision", False)
    data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
    return ui


def fresh_material(name, folder=FOLDER + "/Materials"):
    path = "%s/%s" % (folder, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mat = unreal.load_asset(path)
        lib.delete_all_material_expressions(mat)
        return mat
    return tools.create_asset(name, folder, unreal.Material, unreal.MaterialFactoryNew())


def node(mat, cls, x, y):
    return lib.create_material_expression(mat, cls, x, y)


def scalar(mat, name, value, x, y):
    n = node(mat, unreal.MaterialExpressionScalarParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", value)
    return n


def vector(mat, name, value, x, y):
    n = node(mat, unreal.MaterialExpressionVectorParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", unreal.LinearColor(*value, 1.0))
    return n


def texture_param(mat, name, texture, x, y, sampler=None):
    n = node(mat, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("texture", texture)
    if sampler is not None:
        n.set_editor_property("sampler_type", sampler)
    return n


def binary(mat, cls, a, b, x, y, a_out="", b_out=""):
    n = node(mat, cls, x, y)
    lib.connect_material_expressions(a, a_out, n, "A")
    lib.connect_material_expressions(b, b_out, n, "B")
    return n


def constant(mat, value, x, y):
    n = node(mat, unreal.MaterialExpressionConstant, x, y)
    n.set_editor_property("r", value)
    return n


def build_prop_master(textures):
    mat = fresh_material("M_JamArenaProp")
    base = texture_param(mat, "BaseColor", textures["base_color"], -1200, -300)
    normal = texture_param(mat, "Normal", textures["normal"], -1200, 0, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    metallic = texture_param(mat, "Metallic", textures["metallic"], -1200, 250, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
    rough = texture_param(mat, "Roughness", textures["roughness"], -1200, 500, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
    albedo = binary(mat, unreal.MaterialExpressionMultiply, base, scalar(mat, "Albedo", ALBEDO, -1000, -150), -700, -300, "RGB")
    lib.connect_material_property(albedo, "", unreal.MaterialProperty.MP_BASE_COLOR)
    lib.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    lib.connect_material_property(metallic, "R", unreal.MaterialProperty.MP_METALLIC)
    # The rain: everything outside is a little wetter than Meshy painted it.
    wet = binary(mat, unreal.MaterialExpressionMultiply, rough, scalar(mat, "Wetness", 0.7, -1000, 650), -700, 500, "R")
    lib.connect_material_property(wet, "", unreal.MaterialProperty.MP_ROUGHNESS)
    # Glow: the bright parts of the base colour (lantern glass and paper) shine in GlowColor.
    luma = node(mat, unreal.MaterialExpressionDesaturation, -900, -500)
    lib.connect_material_expressions(base, "RGB", luma, "")
    smooth = node(mat, unreal.MaterialExpressionSmoothStep, -700, -550)
    lib.connect_material_expressions(scalar(mat, "GlowFrom", GLOW_FROM, -900, -650), "", smooth, "Min")
    lib.connect_material_expressions(constant(mat, 0.95, -900, -720), "", smooth, "Max")
    lib.connect_material_expressions(luma, "", smooth, "Value")
    tint = binary(mat, unreal.MaterialExpressionMultiply, vector(mat, "GlowColor", (1.0, 1.0, 1.0), -700, -750),
                  scalar(mat, "Glow", 0.0, -700, -820), -500, -750)
    emissive = binary(mat, unreal.MaterialExpressionMultiply, smooth, tint, -300, -600)
    lib.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
    return mat


def prop_instance(master, name, textures, glow=0.0, color=(1.0, 1.0, 1.0)):
    folder = FOLDER + "/Materials"
    path = "%s/%s" % (folder, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = tools.create_asset(name, folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(mi, master)
    for param, key in (("BaseColor", "base_color"), ("Normal", "normal"), ("Metallic", "metallic"), ("Roughness", "roughness")):
        lib.set_material_instance_texture_parameter_value(mi, param, textures[key])
    lib.set_material_instance_scalar_parameter_value(mi, "Glow", glow)
    lib.set_material_instance_vector_parameter_value(mi, "GlowColor", unreal.LinearColor(*color, 1.0))
    lib.update_material_instance(mi)
    unreal.EditorAssetLibrary.save_loaded_asset(mi, False)
    return mi


def import_props():
    master = None
    # JAM_IMPORT_PROP=stone_lantern limits the run to that prop.
    only = os.environ.get("JAM_IMPORT_PROP", "")
    for prop in [p for p in PROPS if not only or p == only]:
        source = PROP_SOURCES.get(prop, "Art/Models/arena_%s" % prop)
        textures = {}
        for m in MAPS:
            textures[m] = import_texture("%s/texture_urls.0.%s.png" % (source, m), FOLDER + "/Props/Textures", "T_%s_%s" % (prop, m),
                                         data=m in ("metallic", "roughness"), normal=m == "normal")
        mesh = import_file("%s/model_urls.fbx.fbx" % source, FOLDER + "/Props", "SM_Arena_%s" % prop, mesh_options())
        if not mesh:
            log("FAILED SM_Arena_%s" % prop)
            continue
        if master is None:
            master = build_prop_master(textures)
        if prop == "lantern_pillar":
            mis = [prop_instance(master, "MI_Arena_%s_%s" % (prop, tint), textures, 4.0, color) for tint, color in PILLAR_GLOWS.items()]
            mi = mis[0]
        elif prop == "stone_lantern":
            mi = prop_instance(master, "MI_Arena_%s" % prop, textures, 3.0, STONE_LANTERN_GLOW)
        else:
            mi = prop_instance(master, "MI_Arena_%s" % prop, textures)
        mesh.set_material(0, mi)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh, False)
        e = mesh.get_bounds().box_extent
        log("SM_Arena_%s: size (%.0f %.0f %.0f), %d slot(s)" % (prop, 2 * e.x, 2 * e.y, 2 * e.z, len(mesh.get_editor_property("static_materials"))))


def world_uv(mat, size_param, x, y):
    """World XY divided by a size parameter: texture coordinates that tile the same on any mesh."""
    pos = node(mat, unreal.MaterialExpressionWorldPosition, x - 400, y)
    mask = node(mat, unreal.MaterialExpressionComponentMask, x - 200, y)
    mask.set_editor_property("r", True)
    mask.set_editor_property("g", True)
    lib.connect_material_expressions(pos, "", mask, "")
    return binary(mat, unreal.MaterialExpressionDivide, mask, size_param, x, y)


def mask(mat, source, channels, x, y, out=""):
    n = node(mat, unreal.MaterialExpressionComponentMask, x, y)
    for channel in "rgba":
        n.set_editor_property(channel, channel in channels)
    lib.connect_material_expressions(source, out, n, "")
    return n


def parallax(mat, uv, height_texture):
    """The slate's tops shift with the camera, its gaps lag behind: an offset along the view in world XY (the floor
    tiles in world space, so the mesh's tangents do not follow the slate). Nearly none from the top-down fight camera,
    most at the menu's low one. ParallaxHeight is in tile units at a fully grazing view."""
    probe = texture_param(mat, "SlateHeight", height_texture, -1500, 1100, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
    lib.connect_material_expressions(uv, "", probe, "UVs")
    relief = binary(mat, unreal.MaterialExpressionSubtract, probe, constant(mat, 0.5, -1500, 1300), -1300, 1100, "R")
    amount = binary(mat, unreal.MaterialExpressionMultiply, relief, scalar(mat, "ParallaxHeight", PARALLAX_HEIGHT, -1300, 1300), -1150, 1100)
    view = mask(mat, node(mat, unreal.MaterialExpressionCameraVectorWS, -1500, 1400), "rg", -1300, 1400)
    shift = binary(mat, unreal.MaterialExpressionMultiply, view, amount, -1000, 1200)
    return binary(mat, unreal.MaterialExpressionAdd, uv, shift, -900, 1000)


def stronger_normal(mat, normal):
    """The slate's normal with its tilt (RG) scaled by NormalStrength and renormalised: the stones' relief under the
    flat neon light."""
    tilt = binary(mat, unreal.MaterialExpressionMultiply, mask(mat, normal, "rg", -900, -150, "RGB"),
                  scalar(mat, "NormalStrength", NORMAL_STRENGTH, -900, -50), -750, -150)
    whole = binary(mat, unreal.MaterialExpressionAppendVector, tilt, mask(mat, normal, "b", -900, 50, "RGB"), -600, -100)
    unit = node(mat, unreal.MaterialExpressionNormalize, -500, -100)
    lib.connect_material_expressions(whole, "", unit, "")
    return unit


def cavity(mat, height):
    """Darkens the low spots of the height map (the gaps between the stones and the chips) by up to CavityDarken, so
    the stones read as separate blocks. Height 0.25 and below is a full gap, 0.6 and above a stone's top."""
    low = node(mat, unreal.MaterialExpressionSmoothStep, -900, -650)
    lib.connect_material_expressions(constant(mat, 0.6, -1100, -700), "", low, "Min")
    lib.connect_material_expressions(constant(mat, 0.25, -1100, -640), "", low, "Max")
    lib.connect_material_expressions(height, "R", low, "Value")
    kept = node(mat, unreal.MaterialExpressionOneMinus, -900, -550)
    lib.connect_material_expressions(scalar(mat, "CavityDarken", CAVITY_DARKEN, -1100, -550), "", kept, "")
    shade = node(mat, unreal.MaterialExpressionLinearInterpolate, -750, -600)
    lib.connect_material_expressions(constant(mat, 1.0, -900, -480), "", shade, "A")
    lib.connect_material_expressions(kept, "", shade, "B")
    lib.connect_material_expressions(low, "", shade, "Alpha")
    return shade


def build_floor(crest):
    have_slate = all(os.path.exists(os.path.join(ROOT, SLATE, "%s.png" % key)) for key in SLATE_MAPS)
    mat = fresh_material("M_JamArenaFloor")
    if have_slate:
        tex = {key: import_texture("%s/%s.png" % (SLATE, key), FOLDER + "/Floor", name, data=key in ("roughness", "height"),
                                   normal=key == "normal") for key, name in SLATE_MAPS.items()}
    uv = world_uv(mat, scalar(mat, "TileSize", SLATE_TILE, -1800, 0), -1500, 0)
    # The crest: centred on the arena's middle, CrestRadius across half of it.
    crest_uv_scale = binary(mat, unreal.MaterialExpressionMultiply, scalar(mat, "CrestRadius", 1500.0, -1800, 600), constant(mat, 2.0, -1800, 680), -1600, 600)
    crest_uv = world_uv(mat, crest_uv_scale, -1400, 600)
    centred = binary(mat, unreal.MaterialExpressionAdd, crest_uv, constant(mat, 0.5, -1400, 700), -1200, 600)
    crest_tex = texture_param(mat, "Crest", crest, -1000, 600)
    lib.connect_material_expressions(centred, "", crest_tex, "UVs")
    if have_slate:
        uv = parallax(mat, uv, tex["height"])
        color = texture_param(mat, "SlateColor", tex["color"], -1200, -400)
        normal = texture_param(mat, "SlateNormal", tex["normal"], -1200, -100, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        rough = texture_param(mat, "SlateRoughness", tex["roughness"], -1200, 200, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        height = texture_param(mat, "SlateHeight", tex["height"], -1200, 400, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        for sampler in (color, normal, rough, height):
            lib.connect_material_expressions(uv, "", sampler, "UVs")
        base_rgb, rough_r = color, rough
    else:
        base_rgb = vector(mat, "SlateFallback", (0.03, 0.035, 0.05), -1200, -400)
        rough_r = constant(mat, 0.5, -1200, 200)
        normal = height = None
    # Puddles: the low spots of the height map (below PuddleLevel) turn dark and mirror-glossy.
    if height is not None:
        puddle = node(mat, unreal.MaterialExpressionSmoothStep, -900, 400)
        lib.connect_material_expressions(scalar(mat, "PuddleLevel", 0.35, -1100, 500), "", puddle, "Min")
        lib.connect_material_expressions(constant(mat, 0.2, -1100, 560), "", puddle, "Max")
        lib.connect_material_expressions(height, "R", puddle, "Value")
    else:
        puddle = constant(mat, 0.0, -900, 400)
    # Stone: the colour scaled to the dark scene, the crest mixed in by its own brightness.
    stone = binary(mat, unreal.MaterialExpressionMultiply, base_rgb, scalar(mat, "Albedo", 0.2, -1000, -300), -800, -400, "RGB" if height is not None else "")
    if height is not None:
        stone = binary(mat, unreal.MaterialExpressionMultiply, stone, cavity(mat, height), -700, -450)
    crest_mix = node(mat, unreal.MaterialExpressionLinearInterpolate, -600, -200)
    lib.connect_material_expressions(stone, "", crest_mix, "A")
    crest_scaled = binary(mat, unreal.MaterialExpressionMultiply, crest_tex, scalar(mat, "CrestAlbedo", 0.5, -800, 700), -800, 600, "RGB")
    lib.connect_material_expressions(crest_scaled, "", crest_mix, "B")
    # Only the bright bronze lines of the crest are laid into the slate; its own dark stone and the black outside stay out.
    crest_luma = node(mat, unreal.MaterialExpressionDesaturation, -900, 850)
    lib.connect_material_expressions(crest_tex, "RGB", crest_luma, "")
    crest_lines = node(mat, unreal.MaterialExpressionSmoothStep, -800, 900)
    lib.connect_material_expressions(constant(mat, 0.18, -1000, 950), "", crest_lines, "Min")
    lib.connect_material_expressions(constant(mat, 0.4, -1000, 1000), "", crest_lines, "Max")
    lib.connect_material_expressions(crest_luma, "", crest_lines, "Value")
    crest_alpha = binary(mat, unreal.MaterialExpressionMultiply, crest_lines, scalar(mat, "CrestStrength", 0.8, -800, 800), -700, 750)
    lib.connect_material_expressions(crest_alpha, "", crest_mix, "Alpha")
    dark = binary(mat, unreal.MaterialExpressionMultiply, crest_mix, constant(mat, 0.35, -500, -50), -400, -100)
    base = node(mat, unreal.MaterialExpressionLinearInterpolate, -200, -300)
    lib.connect_material_expressions(crest_mix, "", base, "A")
    lib.connect_material_expressions(dark, "", base, "B")
    lib.connect_material_expressions(puddle, "", base, "Alpha")
    lib.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    wet = binary(mat, unreal.MaterialExpressionMultiply, rough_r, scalar(mat, "Wetness", 0.45, -1000, 250), -800, 200, "G" if height is not None else "")
    glossy = node(mat, unreal.MaterialExpressionLinearInterpolate, -200, 200)
    lib.connect_material_expressions(wet, "", glossy, "A")
    lib.connect_material_expressions(constant(mat, 0.04, -400, 300), "", glossy, "B")
    lib.connect_material_expressions(puddle, "", glossy, "Alpha")
    lib.connect_material_property(glossy, "", unreal.MaterialProperty.MP_ROUGHNESS)
    lib.connect_material_property(constant(mat, 0.6, -200, 400), "", unreal.MaterialProperty.MP_SPECULAR)
    if normal is not None:
        # Puddles are flat water: the stone's normal fades out in them.
        flat = node(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
        flat.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 1.0, 1.0))
        water = node(mat, unreal.MaterialExpressionLinearInterpolate, -200, 0)
        lib.connect_material_expressions(stronger_normal(mat, normal), "", water, "A")
        lib.connect_material_expressions(flat, "", water, "B")
        lib.connect_material_expressions(puddle, "", water, "Alpha")
        lib.connect_material_property(water, "", unreal.MaterialProperty.MP_NORMAL)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
    log("M_JamArenaFloor built, slate maps %s" % ("found" if have_slate else "MISSING (flat fallback)"))


def build_city(city, name="M_JamArenaCity"):
    mat = fresh_material(name)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    sample = texture_param(mat, "City", city, -600, 0)
    lit = binary(mat, unreal.MaterialExpressionMultiply, sample, scalar(mat, "Glow", 1.0, -600, 250), -300, 0, "RGB")
    lib.connect_material_property(lit, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, False)


unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
ONLY = os.environ.get("JAM_IMPORT_ONLY", "")
if not ONLY or ONLY == "Props":
    import_props()
if not ONLY or ONLY == "Floor":
    crest_texture = import_texture("Art/Concepts/r7_floor_crest.png", FOLDER + "/Floor", "T_ArenaCrest")
    crest_texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
    crest_texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    build_floor(crest_texture)
    build_city(import_texture("Art/Concepts/r7_city_backdrop.png", FOLDER + "/Floor", "T_ArenaCity"))
if not ONLY or ONLY == "Menu":
    backdrop = import_texture("Art/Concepts/r9_menu_backdrop_wide.png", FOLDER + "/Floor", "T_MenuBackdrop")
    backdrop.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
    backdrop.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    build_city(backdrop, "M_JamMenuBackdrop")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("done")
