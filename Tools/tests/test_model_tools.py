"""Tests for the pure parts of the model tools: GLB stats, the files of a result (Tools/Art/model_files.py) and the
Meshy credit plan and cap (Tools/Art/meshy_models.py)."""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "Art"))

import meshy_models  # noqa: E402
import model_files  # noqa: E402


def make_glb(doc):
    body = json.dumps(doc).encode("utf-8")
    body += b" " * (-len(body) % 4)
    chunk = struct.pack("<II", len(body), 0x4E4F534A) + body
    return struct.pack("<III", 0x46546C67, 2, 12 + len(chunk)) + chunk


def test_glb_stats_counts_triangles_vertices_textures_and_rig():
    doc = {
        "accessors": [{"count": 30}, {"count": 12}, {"count": 9}, {"count": 4}],
        "meshes": [
            {"primitives": [{"attributes": {"POSITION": 1}, "indices": 0}]},
            {"primitives": [{"attributes": {"POSITION": 3}}]},
        ],
        "images": [{}, {}],
        "skins": [{"joints": [0, 1, 2]}],
        "animations": [{}],
    }
    stats = model_files.glb_stats(make_glb(doc))
    # 30 indices = 10 triangles; the unindexed one has 4 positions = 1 triangle (rounded down).
    assert stats == {"triangles": 11, "vertices": 16, "textures": 2, "joints": 3, "animations": 1}


def test_glb_stats_rejects_what_is_not_a_glb():
    try:
        model_files.glb_stats(b"not a glb at all")
    except ValueError:
        return
    raise AssertionError("expected ValueError")


def test_files_in_finds_fal_file_objects_with_their_path_names():
    result = {
        "model_glb": {"url": "https://x/a/model.glb"},
        "model_urls": {"glb": {"url": "https://x/b/m.glb"}, "fbx": {"url": "https://x/c/m.fbx"}},
        "animations": [{"url": "https://x/d/clip.glb"}],
        "seed": 4,
    }
    assert model_files.files_in(result) == [
        ("model_glb", "https://x/a/model.glb", "model_glb.glb"),
        ("model_urls.glb", "https://x/b/m.glb", "model_urls.glb.glb"),
        ("model_urls.fbx", "https://x/c/m.fbx", "model_urls.fbx.fbx"),
        ("animations.0", "https://x/d/clip.glb", "animations.0.glb"),
    ]


def test_files_in_finds_meshy_plain_url_strings():
    task = {
        "id": "abc",
        "model_urls": {"glb": "https://assets.meshy.ai/t/model.glb?Expires=1", "usdz": ""},
        "thumbnail_url": "https://assets.meshy.ai/t/preview.png?Expires=1",
        "texture_urls": [{"base_color": "https://assets.meshy.ai/t/tex.png"}],
        "task_error": None,
    }
    assert model_files.files_in(task) == [
        ("model_urls.glb", "https://assets.meshy.ai/t/model.glb?Expires=1", "model_urls.glb.glb"),
        ("thumbnail_url", "https://assets.meshy.ai/t/preview.png?Expires=1", "thumbnail_url.png"),
        ("texture_urls.0.base_color", "https://assets.meshy.ai/t/tex.png", "texture_urls.0.base_color.png"),
    ]


def test_shrink_textures_makes_a_smaller_glb_with_the_same_mesh():
    import io

    from PIL import Image

    png = io.BytesIO()
    Image.effect_noise((512, 512), 64).convert("RGB").save(png, "PNG")
    image = png.getvalue()
    geometry = bytes(range(48))
    doc = {
        "asset": {"version": "2.0"},
        "buffers": [{"byteLength": len(image) + len(geometry)}],
        "bufferViews": [{"buffer": 0, "byteOffset": 0, "byteLength": len(image)},
                        {"buffer": 0, "byteOffset": len(image), "byteLength": len(geometry)}],
        "images": [{"bufferView": 0, "mimeType": "image/png"}],
        "accessors": [{"bufferView": 1, "count": 3}, {"bufferView": 1, "count": 4}],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 1}, "indices": 0}]}],
    }
    body = json.dumps(doc).encode("utf-8")
    body += b" " * (-len(body) % 4)
    blob = image + geometry
    blob += b"\0" * (-len(blob) % 4)
    chunks = struct.pack("<II", len(body), 0x4E4F534A) + body + struct.pack("<II", len(blob), 0x004E4942) + blob
    glb = struct.pack("<III", 0x46546C67, 2, 12 + len(chunks)) + chunks

    small = model_files.shrink_textures(glb, 128)
    assert len(small) < len(glb) / 4
    assert model_files.glb_stats(small) == model_files.glb_stats(glb)
    length = struct.unpack_from("<I", small, 12)[0]
    out = json.loads(small[20:20 + length])
    view = out["bufferViews"][1]
    bin_start = 20 + length + 8
    assert small[bin_start + view["byteOffset"]:bin_start + view["byteOffset"] + view["byteLength"]] == geometry
    shrunk = out["bufferViews"][0]
    assert Image.open(io.BytesIO(small[bin_start + shrunk["byteOffset"]:bin_start + shrunk["byteOffset"] + shrunk["byteLength"]])).size == (128, 128)


def test_meshy_credits_follow_the_price_page():
    textured = {"kind": "image-to-3d", "input": {}}
    assert meshy_models.planned_credits(textured) == 30
    assert meshy_models.planned_credits({"kind": "image-to-3d", "input": {"should_texture": False}}) == 20
    assert meshy_models.planned_credits({"kind": "image-to-3d", "input": {"texture_resolution": "8k"}}) == 35
    assert meshy_models.planned_credits({"kind": "image-to-3d", "input": {"geometry_resolution": "2k"}}) == 35
    assert meshy_models.planned_credits({"kind": "rigging"}) == 5
    assert meshy_models.planned_credits({"kind": "animations", "input": {"action_ids": [0, 8, 178]}}) == 9


def test_a_round_over_the_credit_cap_or_the_balance_is_refused():
    assert meshy_models.refusal(booked=150, planned=60, cap=200, balance=640)
    assert meshy_models.refusal(booked=0, planned=60, cap=200, balance=50)
    assert meshy_models.refusal(booked=100, planned=60, cap=200, balance=640) == ""
