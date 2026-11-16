"""Lists what import_models.py brought in: every asset under the model folders, mesh sizes, skeleton bones, clip lengths.

UnrealEditor-Cmd.exe JamGame.uproject -run=pythonscript -script="Tools/Editor/report_models.py"
"""
import unreal

FOLDERS = ["/Game/Characters", "/Game/Props"]


def log(text):
    unreal.log("report_models: " + text)


for folder in FOLDERS:
    for path in unreal.EditorAssetLibrary.list_assets(folder, recursive=True):
        asset = unreal.load_asset(path)
        kind = asset.get_class().get_name()
        line = "%s [%s]" % (path.split(".")[0], kind)
        if isinstance(asset, (unreal.SkeletalMesh, unreal.StaticMesh)):
            bounds = asset.get_bounds()
            o, e = bounds.origin, bounds.box_extent
            line += " centre (%.1f %.1f %.1f) size (%.1f %.1f %.1f)" % (o.x, o.y, o.z, 2 * e.x, 2 * e.y, 2 * e.z)
        if isinstance(asset, unreal.StaticMesh):
            line += " tris %d" % asset.get_num_triangles(0)
        if isinstance(asset, unreal.SkeletalMesh):
            line += " materials %s" % [m.get_editor_property("material_interface").get_name() if m.get_editor_property("material_interface") else None
                                       for m in asset.get_editor_property("materials")]
        if isinstance(asset, unreal.AnimSequence):
            line += " %.2f s lock %s" % (asset.get_play_length(), asset.get_editor_property("force_root_lock"))
        if isinstance(asset, unreal.Texture2D):
            line += " %dx%d" % (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())
        log(line)
for mesh_path in ["/Game/Characters/Hero/SK_Hero", "/Game/Characters/Publisher/SK_Publisher"]:
    mesh = unreal.load_asset(mesh_path)
    if mesh:
        bones = [str(mesh.get_bone_name(i)) for i in range(mesh.get_num_bones())] if hasattr(mesh, "get_num_bones") else []
        log("%s bones %d: %s" % (mesh_path, len(bones), " ".join(bones)))
