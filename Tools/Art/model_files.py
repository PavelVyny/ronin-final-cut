"""
KIND: library for the offline model tools (meshy_models.py, fal_models.py): reads GLB stats and lists the files a
generation result points at. No network, no writes.
"""
import json
import os
import struct


def glb_stats(data):
    """Triangles, vertices, textures, joints and clips of a GLB, read from its JSON chunk."""
    if len(data) < 20 or struct.unpack_from("<I", data, 0)[0] != 0x46546C67:
        raise ValueError("not a GLB")
    length, kind = struct.unpack_from("<II", data, 12)
    if kind != 0x4E4F534A:
        raise ValueError("the first GLB chunk is not JSON")
    doc = json.loads(data[20:20 + length].decode("utf-8"))
    accessors = doc.get("accessors", [])
    triangles = vertices = 0
    for mesh in doc.get("meshes", []):
        for prim in mesh.get("primitives", []):
            positions = accessors[prim["attributes"]["POSITION"]]["count"]
            vertices += positions
            corners = accessors[prim["indices"]]["count"] if "indices" in prim else positions
            triangles += corners // 3
    return {
        "triangles": triangles,
        "vertices": vertices,
        "textures": len(doc.get("images", [])),
        "joints": max((len(s.get("joints", [])) for s in doc.get("skins", [])), default=0),
        "animations": len(doc.get("animations", [])),
    }


def shrink_textures(data, max_side):
    """The same GLB with every embedded texture scaled down to fit max_side (PNG, for a light preview); the mesh,
    rig and clips are copied byte for byte. Only single-buffer GLBs, as the generators write them."""
    import io

    from PIL import Image

    length = struct.unpack_from("<I", data, 12)[0]
    doc = json.loads(data[20:20 + length].decode("utf-8"))
    bin_start = 20 + length + 8
    views = doc.get("bufferViews", [])
    parts = [data[bin_start + v.get("byteOffset", 0):bin_start + v.get("byteOffset", 0) + v["byteLength"]] for v in views]
    for image in doc.get("images", []):
        if "bufferView" not in image:
            continue
        picture = Image.open(io.BytesIO(parts[image["bufferView"]]))
        picture.thumbnail((max_side, max_side))
        out = io.BytesIO()
        picture.save(out, "PNG", optimize=True)
        parts[image["bufferView"]] = out.getvalue()
        image["mimeType"] = "image/png"
    blob = b""
    for view, part in zip(views, parts):
        blob += b"\0" * (-len(blob) % 4)
        view["byteOffset"] = len(blob)
        view["byteLength"] = len(part)
        blob += part
    blob += b"\0" * (-len(blob) % 4)
    doc["buffers"] = [{"byteLength": len(blob)}]
    body = json.dumps(doc, separators=(",", ":")).encode("utf-8")
    body += b" " * (-len(body) % 4)
    chunks = struct.pack("<II", len(body), 0x4E4F534A) + body + struct.pack("<II", len(blob), 0x004E4942) + blob
    return struct.pack("<III", 0x46546C67, 2, 12 + len(chunks)) + chunks


def _entry(path, url):
    ext = os.path.splitext(url.split("?")[0])[1] or ".bin"
    return (path, url, path + ext)


def files_in(result, path=""):
    """(path in the result, url, local file name) of every file a result points at, in result order: fal's
    {"url": ...} objects and Meshy's plain https strings."""
    found = []
    if isinstance(result, str):
        return [_entry(path, result)] if result.startswith("https://") and path else []
    if isinstance(result, dict):
        if isinstance(result.get("url"), str) and path:
            return [_entry(path, result["url"])]
        for key, value in result.items():
            found += files_in(value, path + "." + key if path else key)
    elif isinstance(result, list):
        for index, value in enumerate(result):
            found += files_in(value, "%s.%d" % (path, index))
    return found


def print_stats(folder):
    for dirpath, _, names in sorted(os.walk(folder)):
        for file_name in sorted(n for n in names if n.endswith(".glb")):
            path = os.path.join(dirpath, file_name)
            with open(path, "rb") as h:
                stats = glb_stats(h.read())
            print("[stats] %s: %s, %.1f MB" % (os.path.relpath(path, folder), stats, os.path.getsize(path) / 1e6))
