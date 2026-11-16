"""
KIND: offline tool (headless Blender, writes Art/Models/boss_fixed/publisher_rig.blend).

Rebuilds the Publisher's rig from the Meshy files: welds the mesh, refits the Meshy skeleton to the fat body (same bone
names and rest orientations, so the Meshy clips still play), adds a root and six helper bones (belly, three jacket
flaps, two for the tie), drops the Meshy end bones and paints new skin weights from scratch.

    blender -b --factory-startup --python Tools/Art/publisher_rig/build_rig.py

The weights are a function of the rest (T-pose) position and of the texture colour, not of bone distance: the trunk
follows pelvis and spine by height, the arm starts at the sleeve, the leg below the jacket hem, the head is the skin.
"""
import colorsys
import heapq
import statistics
import math
import os

import bmesh
import bpy
import numpy as np
from mathutils import Vector, kdtree

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
SRC = os.path.join(ROOT, "Art", "Models", "boss_clips", "result.animation_fbx_url.fbx")
OUT_DIR = os.path.join(ROOT, "Art", "Models", "boss_fixed")
OUT = os.path.join(OUT_DIR, "publisher_rig.blend")

END_BONES = ["head_end", "headfront", "LeftHand_End", "RightHand_End", "LeftToe_end", "RightToe_end"]
SHOULDER_X, ELBOW_X, WRIST_X = 0.66, 0.92, 1.13
KNEE_Z, ANKLE_Z = 0.69, 0.21
SPINE_Z = (1.46, 1.68, 1.91)
TIE_KNOT, TIE_MID = (0, -0.235, 2.10), (0, -0.46, 1.80)
TIE_GOLD_UNDER = 2.03  # above this height the shirt under the tie strip stays gold


# The free tie strip floats this far (m) off the shirt it was cut from; the shirt under it sinks by TIE_SINK.
TIE_LIFT = 0.006
# The top of the knot, painted dark and skin by Meshy: half width, front limit (y) and height range (m).
KNOT_CAP = (0.036, -0.14, 2.185, 2.24)
TIE_SINK = 0.015
# The black collar within this distance (m) of the skin moves with the skin, so the crease under the jowls never opens.
NECK_RING = 0.04
# Faces within this distance (m) of the painted skin border are cut and cleaned.
NECK_BAND = 0.10
# Width (m) of the average that draws the new skin-to-collar border; it stays within NECK_REACH of the painted one.
NECK_BLUR = 0.045
NECK_REACH = 0.06


def ss(a, b, x):
    """Smoothstep from 0 at a to 1 at b; a may be greater than b."""
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


def import_source():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=SRC)
    arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
    mesh = next(o for o in bpy.data.objects if o.type == "MESH")
    world = mesh.matrix_world.copy()
    mesh.parent = None
    mesh.matrix_world = world
    for o in (arm, mesh):
        o.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    mesh.parent = arm
    mesh.matrix_parent_inverse.identity()
    arm.name = "Armature"
    arm.data.name = "PublisherSkeleton"
    mesh.name = "SK_Publisher"
    mesh.data.name = "SK_Publisher"
    return arm, mesh


def weld(mesh):
    bm = bmesh.new()
    bm.from_mesh(mesh.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-5)
    for f in bm.faces:
        f.smooth = True
    bm.to_mesh(mesh.data)
    bm.free()
    me = mesh.data
    if me.attributes.get("custom_normal"):
        me.attributes.remove(me.attributes["custom_normal"])
    me.update()


def slice_centre(co, lo, hi, axis, side_min=0.02):
    """Bounding-box centre of the left-side vertices whose `axis` coordinate is in [lo, hi]."""
    m = (co[:, axis] >= lo) & (co[:, axis] <= hi) & (co[:, 0] > side_min)
    part = co[m]
    return (part.min(axis=0) + part.max(axis=0)) / 2.0


def measure(co):
    """Joint positions of the left side, from the body itself."""
    knee = slice_centre(co, KNEE_Z - 0.04, KNEE_Z + 0.04, 2)
    ankle = slice_centre(co, ANKLE_Z, ANKLE_Z + 0.08, 2)
    arm = slice_centre(co, 0.82, 1.02, 0)
    shoe = co[(co[:, 2] < 0.14) & (co[:, 0] > 0.02)][:, :2]
    mean = shoe.mean(axis=0)
    _, vecs = np.linalg.eigh(np.cov((shoe - mean).T))
    toe_dir = vecs[:, 1]
    if toe_dir[1] > 0:
        toe_dir = -toe_dir
    t = (shoe - mean) @ toe_dir
    heel, toe = mean + toe_dir * t.min(), mean + toe_dir * t.max()
    ball = heel + (toe - heel) * 0.66
    joints = {
        "thigh": (0.20, 0.10, 1.08),
        "calf": (knee[0], knee[1] - 0.02, KNEE_Z),
        "foot": (ankle[0], ankle[1] + 0.03, ANKLE_Z),
        "ball": (ball[0], ball[1], 0.05),
        "clavicle": (0.09, 0.17, 2.12),
        "upperarm": (SHOULDER_X, arm[1], arm[2] + 0.02),
        "lowerarm": (ELBOW_X, arm[1], arm[2]),
        "hand": (WRIST_X, arm[1], arm[2]),
    }
    print("[build] toe_dir", toe_dir, "heel", heel, "toe", toe)
    for k, v in joints.items():
        print("[build] joint %-9s %.3f %.3f %.3f" % (k, *v))
    return joints, toe_dir, np.array(joints["ball"][:2])


def refit_skeleton(arm, joints):
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    eb = arm.data.edit_bones
    for b in eb:
        b.use_connect = False
    for name in END_BONES:
        eb.remove(eb[name])

    def move(name, head):
        b = eb[name]
        d = Vector(head) - b.head
        b.head += d
        b.tail += d

    centre = {"pelvis": (0, 0.08, 1.20), "spine_01": (0, 0.10, SPINE_Z[0]), "spine_02": (0, 0.125, SPINE_Z[1]),
              "spine_03": (0, 0.15, SPINE_Z[2]), "neck_01": (0, 0.17, 2.10), "head": (0, 0.16, 2.20)}
    for name, head in centre.items():
        move(name, head)
    for key, (x, y, z) in joints.items():
        move(key + "_l", (x, y, z))
        move(key + "_r", (-x, y, z))

    def helper(name, head, parent):
        b = eb.new(name)
        b.head = Vector(head)
        b.tail = Vector(head) + Vector((0, 0.2, 0))
        b.roll = 0.0
        b.parent = eb[parent] if parent else None
        b.use_deform = True
        return b

    root = helper("root", (0, 0, 0), None)
    eb["pelvis"].parent = root
    helper("belly", (0, -0.10, 1.48), "spine_01")
    helper("jacket_front_l", (0.36, -0.38, 1.52), "spine_01")
    helper("jacket_front_r", (-0.36, -0.38, 1.52), "spine_01")
    helper("jacket_back", (0, 0.50, 1.52), "pelvis")
    helper("tie_01", TIE_KNOT, "spine_03")
    helper("tie_02", TIE_MID, "tie_01")
    bpy.ops.object.mode_set(mode="OBJECT")


def base_image(me):
    image = None
    for mat in me.materials:
        if mat and mat.use_nodes:
            for node in mat.node_tree.nodes:
                if node.type == "BSDF_PRINCIPLED":
                    link = node.inputs["Base Color"].links
                    if link and link[0].from_node.type == "TEX_IMAGE":
                        image = link[0].from_node.image
    if image is None:
        image = next(i for i in bpy.data.images if i.size[0] > 0)
    return image


def vertex_colours(mesh):
    """Mean base-colour of each vertex, sampled through its UVs."""
    me = mesh.data
    image = base_image(me)
    w, h = image.size
    px = np.array(image.pixels[:], dtype=np.float32).reshape(h, w, 4)
    uv = np.zeros(len(me.loops) * 2, dtype=np.float32)
    me.uv_layers.active.data.foreach_get("uv", uv)
    uv = uv.reshape(-1, 2)
    vi = np.zeros(len(me.loops), dtype=np.int64)
    me.loops.foreach_get("vertex_index", vi)
    xs = np.clip((uv[:, 0] % 1.0 * w).astype(int), 0, w - 1)
    ys = np.clip((uv[:, 1] % 1.0 * h).astype(int), 0, h - 1)
    rgb = px[ys, xs, :3]
    total = np.zeros((len(me.vertices), 3))
    count = np.zeros(len(me.vertices))
    np.add.at(total, vi, rgb)
    np.add.at(count, vi, 1)
    return total / np.maximum(count, 1)[:, None]


def neighbours(me):
    adj = [[] for _ in me.vertices]
    for e in me.edges:
        a, b = e.vertices
        adj[a].append(b)
        adj[b].append(a)
    return adj


def geodesic(co, adj, sources, limit):
    dist = np.full(len(co), np.inf)
    heap = []
    for s in sources:
        dist[s] = 0.0
        heap.append((0.0, int(s)))
    heapq.heapify(heap)
    while heap:
        d, a = heapq.heappop(heap)
        if d > dist[a] or d > limit:
            continue
        for b in adj[a]:
            nd = d + float(np.linalg.norm(co[a] - co[b]))
            if nd < dist[b]:
                dist[b] = nd
                heapq.heappush(heap, (nd, b))
    return dist


def head_set(co, rgb, adj):
    hsv = np.array([colorsys.rgb_to_hsv(*c) for c in rgb])
    black = hsv[:, 2] < 0.18
    cream = (hsv[:, 1] < 0.28) & (hsv[:, 2] > 0.45)
    x, z = np.abs(co[:, 0]), co[:, 2]
    head = ((~black & ~cream) & (z > 2.13) & (x < 0.36)) | (z > 2.44)
    # Shaded creases of the collar and the lapels pass the colour test too: only what is joined to the skull through
    # skin is the head, otherwise the lapel tips follow the head and lift off the jacket.
    reached = np.zeros(len(co), dtype=bool)
    stack = [int(i) for i in np.where(z > 2.44)[0]]
    reached[stack] = True
    while stack:
        a = stack.pop()
        for b in adj[a]:
            if head[b] and not reached[b]:
                reached[b] = True
                stack.append(b)
    print("[build] head candidates %d, joined to the skull %d" % (head.sum(), reached.sum()))
    head = reached
    # Teeth, eyes and the top-knot: whatever is cut off from the body by the skin is the head too.
    seen = head.copy()
    for start in np.where(~head)[0]:
        if seen[start]:
            continue
        comp, stack, low = [start], [start], False
        seen[start] = True
        while stack:
            a = stack.pop()
            if co[a, 2] < 2.16:  # the loose folds of the tie knot end at 2.16 and are not the head
                low = True
            for b in adj[a]:
                if not seen[b]:
                    seen[b] = True
                    comp.append(b)
                    stack.append(b)
        if not low:
            head[comp] = True
    return head, black, cream, hsv


def paint(mesh, arm, toe_dir, ball_xy):
    me = mesh.data
    n = len(me.vertices)
    co = np.zeros(n * 3)
    me.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    x, y, z = co[:, 0], co[:, 1], co[:, 2]
    ax = np.abs(x)
    adj = neighbours(me)
    rgb = vertex_colours(mesh)
    head, black, cream, hsv = head_set(co, rgb, adj)
    red = ((hsv[:, 0] < 0.03) | (hsv[:, 0] > 0.95)) & (hsv[:, 1] > 0.6)
    print("[build] head verts %d, black %d, cream %d" % (head.sum(), black.sum(), cream.sum()))

    names = [b.name for b in arm.data.bones]
    w = {name: np.zeros(n) for name in names}

    # Trunk: a partition by height.
    h = 0.14
    s1, s2, s3 = (ss(c - h, c + h, z) for c in SPINE_Z)
    w["pelvis"] = 1 - s1
    w["spine_01"] = s1 * (1 - s2)
    w["spine_02"] = s2 * (1 - s3)
    w["spine_03"] = s3
    trunk = ["pelvis", "spine_01", "spine_02", "spine_03"]

    def scale(keys, factor):
        for k in keys:
            w[k] = w[k] * factor

    # Belly: the front bulge, shirt and jacket front alike.
    belly = 0.75 * ss(0.08, -0.30, y) * ss(1.95, 1.58, z) * ss(0.98, 1.14, z) * ss(0.72, 0.42, ax)
    scale(trunk, 1 - belly)
    w["belly"] = belly

    # Jacket flaps: the cream skirt of the jacket from the hem up to the waist.
    jacket = (~black & ~red & (z > 1.15)).astype(float)
    flap = 0.85 * ss(1.50, 1.24, z) * jacket
    theta = np.abs(np.arctan2(x, -y))
    back = ss(math.radians(80), math.radians(120), theta)
    scale(trunk + ["belly"], 1 - flap)
    w["jacket_back"] = flap * back
    w["jacket_front_l"] = flap * (1 - back) * (x > 0)
    w["jacket_front_r"] = flap * (1 - back) * (x <= 0)
    body = trunk + ["belly", "jacket_back", "jacket_front_l", "jacket_front_r"]

    # Legs: below a hip line that runs from the crotch up to the outside of the hip, never into the jacket.
    zc = 0.90 + 0.17 * ss(0.0, 0.35, ax)
    leg = ss(zc + 0.13, zc - 0.13, z)
    leg = np.where(black & (z > 0.9), 0.0, leg)
    leg = np.where(z > 1.165, 0.0, leg)
    scale(body, 1 - leg)
    left = ss(-0.05, 0.05, x)
    knee = ss(KNEE_Z + 0.10, KNEE_Z - 0.10, z)
    foot = ss(0.27, 0.15, z)
    for side, share, sx in (("l", left, 1.0), ("r", 1 - left, -1.0)):
        t = (x * sx - ball_xy[0]) * toe_dir[0] + (y - ball_xy[1]) * toe_dir[1]
        toe = ss(-0.04, 0.04, t) * ss(0.16, 0.10, z)
        w["thigh_" + side] = leg * share * (1 - knee)
        w["calf_" + side] = leg * share * knee * (1 - foot)
        w["foot_" + side] = leg * share * knee * foot * (1 - toe)
        w["ball_" + side] = leg * share * knee * foot * toe

    # Arms: the sleeve from the shoulder seam outwards; the jacket under the armpit stays on the spine.
    gate = ss(1.80, 1.93, z)
    arm_m = ss(0.60, 0.82, ax) * gate
    clav = 0.6 * ss(0.14, 0.42, ax) * ss(1.88, 2.08, z) * (1 - arm_m)
    scale(body, 1 - arm_m - clav)
    elbow = ss(ELBOW_X - 0.07, ELBOW_X + 0.07, ax)
    wrist = ss(WRIST_X - 0.04, WRIST_X + 0.04, ax)
    for side, m in (("l", x > 0), ("r", x <= 0)):
        w["clavicle_" + side] = clav * m
        w["upperarm_" + side] = arm_m * (1 - elbow) * m
        w["lowerarm_" + side] = arm_m * elbow * (1 - wrist) * m
        w["hand_" + side] = arm_m * wrist * m

    # Head and neck: the skin is the head; the twist is spread over the jowls and a short ring of the collar.
    out = geodesic(co, adj, np.where(head)[0], 0.4)
    inside = geodesic(co, adj, np.where(~head)[0], 0.4)
    # The skin border lies in the crease under the jowls and its texture is ragged: the first ring of black collar
    # moves with the skin, so the stretch falls on plain black shirt further out.
    reach = np.where(black, np.maximum(out - NECK_RING, 0.0), out)
    hn = np.where(head, 1.0, 1 - ss(0.0, 0.12, reach))
    w_head = np.where(head, 0.6 + 0.4 * ss(0.0, 0.10, inside), 0.6 * (1 - ss(0.0, 0.07, reach)))
    for k in names:
        w[k] = w[k] * (1 - hn)
    w["head"] = w_head
    w["neck_01"] = np.maximum(hn - w_head, 0.0)

    # Tie: the gold strip on the shirt hangs from the knot on its own two bones.
    gold = (hsv[:, 0] > 0.07) & (hsv[:, 0] < 0.17) & (hsv[:, 1] > 0.45) & (hsv[:, 2] > 0.3)
    tie = gold & ~head & (ax < 0.12) & (y < -0.15) & (z > 1.45) & (z < 2.16)
    # Dark texels inside the gold would leave holes in the strip: whatever lies inside its outline is tie as well.
    rows = np.clip(((z - 1.45) / 0.02).astype(int), 0, 35)
    half = np.zeros(36)
    np.maximum.at(half, rows[tie], ax[tie])
    half = np.maximum(half, np.maximum(np.roll(half, 1), np.roll(half, -1)))
    tie = tie | (~head & (y < -0.15) & (z > 1.45) & (z < 2.16) & (ax < half[rows] - 0.005))
    # Notches in the outline go the same way: a vert with most of its neighbours in the tie is tie.
    for _ in range(2):
        share = np.array([tie[nb].mean() if len(nb) else 0.0 for nb in adj])
        tie = tie | (~head & (y < -0.15) & (z > 1.45) & (z < 2.16) & (share > 0.5))
    # The knot itself stays on the chest; the faces that join it to the strip must stay gold.
    knot = gold & ~tie & ~head & (ax < 0.12) & (y < -0.15) & (z > 2.0)
    hang = np.where(tie, ss(2.12, 2.00, z), 0.0)
    lower = ss(1.88, 1.72, z)
    for k in names:
        w[k] = w[k] * (1 - hang)
    w["tie_01"] = hang * (1 - lower)
    w["tie_02"] = hang * lower
    # The knot sits on the chest: it must not follow the head.
    for k in ("head", "neck_01"):
        w["spine_03"] = w["spine_03"] + np.where(tie, w[k], 0.0)
        w[k] = np.where(tie, 0.0, w[k])
    print("[build] tie verts %d" % tie.sum())

    W = np.stack([w[k] for k in names], axis=1)
    W = W / W.sum(axis=1, keepdims=True)

    # Smooth over the surface, then put the rigid parts back.
    rigid = (head & (inside > 0.08)) | (ax > WRIST_X + 0.08) | (z < 0.12) | tie
    fixed = W.copy()
    idx = np.array([i for i, nb in enumerate(adj) for _ in nb])
    nbr = np.array([b for nb in adj for b in nb])
    deg = np.array([max(len(nb), 1) for nb in adj], dtype=float)
    for _ in range(6):
        avg = np.zeros_like(W)
        np.add.at(avg, idx, W[nbr])
        avg /= deg[:, None]
        W = 0.5 * W + 0.5 * avg
        W[rigid] = fixed[rigid]

    # Layers that lie on each other (lapel on jacket, jacket edge on shirt, tie on shirt) are not neighbours on the
    # surface, so the smoothing drives them apart: average the torso over space as well.
    # The tie is left out: it must not stick to the shirt under it.
    layered = np.where(~head & ~tie & (z > 1.15) & (ax < ELBOW_X))[0]
    tree = kdtree.KDTree(len(layered))
    for k, i in enumerate(layered):
        tree.insert(co[i], k)
    tree.balance()
    near = [[layered[k] for _, k, _ in tree.find_range(co[i], 0.03)] for i in layered]
    for _ in range(3):
        W[layered] = np.array([W[group].mean(axis=0) for group in near])

    # At most four influences, normalised.
    order = np.argsort(-W, axis=1)
    keep = np.zeros_like(W, dtype=bool)
    np.put_along_axis(keep, order[:, :4], True, axis=1)
    W = np.where(keep & (W > 0.01), W, 0.0)
    W = W / W.sum(axis=1, keepdims=True)

    mesh.vertex_groups.clear()
    for j, name in enumerate(names):
        g = mesh.vertex_groups.new(name=name)
        col = W[:, j]
        for i in np.where(col > 0)[0]:
            g.add([int(i)], float(col[i]), "REPLACE")
    used = (W > 0).sum(axis=1)
    print("[build] influences per vertex: max %d, mean %.2f" % (used.max(), used.mean()))
    for j, name in enumerate(names):
        print("[build] group %-16s verts=%5d weight=%8.1f" % (name, (W[:, j] > 0).sum(), W[:, j].sum()))
    split_tie(mesh, co, W, names, tie, knot, black & ~tie & ~head)
    # Skin spilled onto the collar has the hue of the gold: only the gold around the knot is kept out of the border.
    by_knot = gold & (ax < 0.15) & (y < -0.10) & (z < 2.25)
    neck_border(mesh, co, adj, rgb, head, ~cream & ~tie & ~head & ~by_knot, tie | by_knot, black & ~tie & ~head,
                inside, out)
    gild_knot(mesh)


def border_distance(me, co, adj, head, signed):
    """Distance (m) of each vert from a smooth skin-to-collar border, positive on the skin.

    Close to the painted border the skin flag is averaged over the surface by area; across a straight border that
    average is the normal distribution of the distance, so its inverse is a distance again, and one that is linear
    along an edge however long the edge is. Zero of that field does not follow the edges of the mesh.
    """
    area = np.zeros(len(co))
    me.calc_loop_triangles()
    for lt in me.loop_triangles:
        for k in lt.vertices:
            if k < len(co):
                area[k] += lt.area / 3
    span = [[float(np.linalg.norm(co[a] - co[b])) for b in nb] for a, nb in enumerate(adj)]
    limit = 2.5 * NECK_BLUR
    side = head.astype(float)
    close = np.abs(signed) < NECK_REACH
    for i in np.where(close)[0]:
        dist = {int(i): 0.0}
        heap = [(0.0, int(i))]
        while heap:
            d, a = heapq.heappop(heap)
            if d > dist[a]:
                continue
            for b, step in zip(adj[a], span[a]):
                nd = d + step
                if nd < limit and nd < dist.get(b, limit):
                    dist[b] = nd
                    heapq.heappush(heap, (nd, b))
        near = np.fromiter(dist.keys(), dtype=np.int64)
        weight = area[near] * np.exp(-0.5 * (np.fromiter(dist.values(), dtype=float) / NECK_BLUR) ** 2)
        side[i] = (weight * head[near]).sum() / weight.sum()
    normal = statistics.NormalDist()
    smooth = np.array([NECK_BLUR * normal.inv_cdf(min(max(s, 0.005), 0.995)) for s in side])
    smooth = np.where(close, smooth, signed)
    # A vert right on the border would make faces that lie on both sides.
    return np.where(np.abs(smooth) < 1e-4, 1e-4, smooth)


def plain_black_uv(bm, uv, shirt):
    """One texel of plain black shirt, far from any seam."""
    donor = next(f for f in bm.faces if all(v.index < len(shirt) and shirt[v.index] and abs(v.co.x) > 0.15
                                             and v.co.y < -0.2 and 1.6 < v.co.z < 1.9 for v in f.verts))
    return sum((l[uv].uv for l in donor.loops), Vector((0, 0))) / len(donor.loops)


def covered(p, w, h, tol):
    """Texels under a UV triangle given in texel units: the slices of the block and the mask inside it."""
    x0, y0 = np.floor(p.min(axis=0)).astype(int) - 1
    x1, y1 = np.ceil(p.max(axis=0)).astype(int) + 1
    x0, y0, x1, y1 = max(x0, 0), max(y0, 0), min(x1, w - 1), min(y1, h - 1)
    d = (p[1, 1] - p[2, 1]) * (p[0, 0] - p[2, 0]) + (p[2, 0] - p[1, 0]) * (p[0, 1] - p[2, 1])
    if x1 < x0 or y1 < y0 or abs(d) < 1e-9:
        return None
    xs, ys = np.meshgrid(np.arange(x0, x1 + 1), np.arange(y0, y1 + 1))
    b0 = ((p[1, 1] - p[2, 1]) * (xs - p[2, 0]) + (p[2, 0] - p[1, 0]) * (ys - p[2, 1])) / d
    b1 = ((p[2, 1] - p[0, 1]) * (xs - p[2, 0]) + (p[0, 0] - p[2, 0]) * (ys - p[2, 1])) / d
    tol = min(0.5, tol / max(x1 - x0, y1 - y0, 1))
    return slice(y0, y1 + 1), slice(x0, x1 + 1), (b0 >= -tol) & (b1 >= -tol) & (1 - b0 - b1 >= -tol)


def neck_border(mesh, co, adj, rgb, head, shirt, other, black_shirt, inside, out):
    """Cut a clean skin-to-collar border into the mesh.

    Meshy paints that border across the faces, ragged, with black spilled onto the jowls and skin onto the collar,
    and the faces in the crease are slivers with a few stretched texels each: it shows as a hairy fringe as soon as
    the head turns. The faces near the border are cut along a smooth line over the surface; on the collar side they
    turn plain black, on the skin side whatever is not skin is painted over. A face that reaches the jacket keeps
    its texture, with the skin on its collar side painted black. Faces with a vert of `other` are left alone.
    """
    me = mesh.data
    n = len(head)
    band = np.where(head, inside, out) < NECK_BAND
    side = border_distance(me, co, adj, head, np.where(head, inside, -out))
    near_skin = head & (inside < 0.10) & (rgb.max(axis=1) > 0.4)
    tone = np.median(rgb[near_skin], axis=0)

    def is_skin(c, colour=0.2):
        hi, lo = c.max(axis=-1), c.min(axis=-1)
        return (hi > 0.5) & ((hi - lo) > colour * hi) & (c[..., 0] >= c[..., 1])

    bm = bmesh.new()
    bm.from_mesh(me)
    bm.verts.ensure_lookup_table()
    uv = bm.loops.layers.uv.active
    black_uv = plain_black_uv(bm, uv, black_shirt)
    faces = [f for f in bm.faces if all(v.index < n and not other[v.index] for v in f.verts)
             and any(band[v.index] for v in f.verts)]
    value = {v: float(side[v.index]) for f in faces for v in f.verts}
    mixed = {v for v in value if not (head[v.index] or shirt[v.index])}
    fresh = set()
    for e in {e for f in faces for e in f.edges}:
        a, b = e.verts
        sa, sb = value[a], value[b]
        if sa * sb >= 0:
            continue
        t = min(max(sa / (sa - sb), 0.05), 0.95)
        where = a.co.lerp(b.co, t)
        v = bmesh.utils.edge_split(e, a, t)[1]
        if (v.co - where).length > 1e-5:
            raise RuntimeError("edge_split measures from the other end")
        value[v] = 0.0
        fresh.add(v)
    # Each cut face gets an edge along the border; a neighbour that only received a vert is closed with a fan.
    for f in list({f for v in fresh for f in v.link_faces}):
        on = [v for v in f.verts if v in fresh]
        if len(on) == 2:
            bmesh.utils.face_split(f, on[0], on[1])
        elif len(on) == 1 and len(f.verts) == 4:
            ring = list(f.verts)
            bmesh.utils.face_split(f, on[0], ring[(ring.index(on[0]) + 2) % 4])
    bmesh.ops.triangulate(bm, faces=list({f for v in fresh for f in v.link_faces if len(f.verts) > 3}))

    image = base_image(me)
    w, h = image.size
    px = np.array(image.pixels[:], dtype=np.float32).reshape(h, w, 4)
    size = np.array((w, h))
    skin, collar = [], []
    for f in {f for v in value for f in v.link_faces}:
        if all(v in value for v in f.verts) and any(v in fresh or band[v.index] for v in f.verts):
            (skin if sum(value[v] for v in f.verts) > 0 else collar).append(f)
    texels = 0
    black_px = np.array((0.015, 0.015, 0.015))
    for f in collar:
        hit = covered(np.array([l[uv].uv[:] for l in f.loops]) * size - 0.5, w, h, 1.5)
        if any(v in mixed for v in f.verts):
            if hit:
                block = px[hit[0], hit[1], :3]
                block[hit[2] & is_skin(block, 0.3)] = black_px
            continue
        # These faces leave their texels; next to the border those texels still bleed into the skin faces.
        if hit and any(v in fresh for v in f.verts):
            block = px[hit[0], hit[1], :3]
            block[hit[2]] = tone
        for l in f.loops:
            l[uv].uv = black_uv
    for f in skin:
        hit = covered(np.array([l[uv].uv[:] for l in f.loops]) * size - 0.5, w, h, 1.5)
        if hit:
            block = px[hit[0], hit[1], :3]
            own = block[hit[2] & is_skin(block)]
            if any(v in mixed for v in f.verts):
                wrong = hit[2] & (block.max(axis=-1) < 0.25)
            else:
                wrong = hit[2] & ~is_skin(block)
            block[wrong] = np.median(own, axis=0) if len(own) >= 4 else tone
            texels += int(wrong.sum())
    first = len(me.vertices)
    bm.to_mesh(me)
    bm.free()
    image.pixels.foreach_set(px.ravel())
    image.update()
    image.pack()

    # A vert made by the cut carries the weights of both ends of its edge: back to four influences.
    for i in range(first, len(me.vertices)):
        groups = sorted(me.vertices[i].groups, key=lambda g: -g.weight)
        total = sum(g.weight for g in groups[:4])
        for k, g in enumerate([(g.group, g.weight) for g in groups]):
            if k < 4:
                mesh.vertex_groups[g[0]].add([i], g[1] / total, "REPLACE")
            else:
                mesh.vertex_groups[g[0]].remove([i])
    print("[build] neck border: %d verts cut in, %d skin faces, %d collar faces turned black, %d texels to skin, "
          "tone %s, %d verts in all" % (len(me.vertices) - first, len(skin), len(collar), texels, tone.round(3),
                                        len(me.vertices)))


def gild_knot(mesh):
    """Paint the top of the knot gold.

    Meshy left it dark, with a dot of skin, under two tips of the collar: it reads as a hole in the knot. Every texel
    whose place on the surface lies in the box of KNOT_CAP takes the gold of the knot below it, so the edge of the
    paint is straight whatever the faces are. A face whose texture was already folded into one texel moves whole.
    """
    me = mesh.data
    half, front, low, high = KNOT_CAP
    image = base_image(me)
    w, h = image.size
    px = np.array(image.pixels[:], dtype=np.float32).reshape(h, w, 4)
    size = np.array((w, h))
    uv = me.uv_layers.active.data
    co = np.array([v.co[:] for v in me.vertices])
    me.calc_loop_triangles()

    def in_cap(p):
        return (np.abs(p[..., 0]) < half) & (p[..., 1] < front) & (p[..., 2] > low) & (p[..., 2] < high)

    # The gold of the knot right under the cap, and one texel that has it.
    samples = []
    for lt in me.loop_triangles:
        p = co[list(lt.vertices)]
        if np.all((np.abs(p[:, 0]) < 0.03) & (p[:, 1] < -0.15) & (p[:, 2] > 2.12) & (p[:, 2] < low)):
            spot = sum((uv[l].uv for l in lt.loops), Vector((0, 0))) / 3
            c = px[min(int(spot.y * h), h - 1), min(int(spot.x * w), w - 1), :3]
            if c[0] > 0.5 and c[1] > 0.6 * c[0] and c[2] < 0.6 * c[0]:
                samples.append((c.copy(), spot))
    gold = np.median([c for c, _ in samples], axis=0)
    gold_uv = min(samples, key=lambda s: float(np.abs(s[0] - gold).sum()))[1]

    texels = moved = 0
    for lt in me.loop_triangles:
        p = co[list(lt.vertices)]
        if not (p[:, 0].min() < half and p[:, 0].max() > -half and p[:, 1].min() < front
                and p[:, 2].min() < high and p[:, 2].max() > low):
            continue
        t = np.array([uv[l].uv[:] for l in lt.loops]) * size - 0.5
        hit = covered(t, w, h, 1.5)
        if hit is None or np.ptp(t, axis=0).max() < 0.5:
            if in_cap(p.mean(axis=0)):
                for l in lt.loops:
                    uv[l].uv = gold_uv
                moved += 1
            continue
        ys, xs = np.mgrid[hit[0], hit[1]]
        d = (t[1, 1] - t[2, 1]) * (t[0, 0] - t[2, 0]) + (t[2, 0] - t[1, 0]) * (t[0, 1] - t[2, 1])
        b0 = ((t[1, 1] - t[2, 1]) * (xs - t[2, 0]) + (t[2, 0] - t[1, 0]) * (ys - t[2, 1])) / d
        b1 = ((t[2, 1] - t[0, 1]) * (xs - t[2, 0]) + (t[0, 0] - t[2, 0]) * (ys - t[2, 1])) / d
        place = b0[..., None] * p[0] + b1[..., None] * p[1] + (1 - b0 - b1)[..., None] * p[2]
        mask = hit[2] & in_cap(place)
        block = px[hit[0], hit[1], :3]
        block[mask] = gold
        texels += int(mask.sum())
    image.pixels.foreach_set(px.ravel())
    image.update()
    image.pack()
    print("[build] knot cap: %d texels to gold %s, %d folded faces moved" % (texels, gold.round(3), moved))


def split_tie(mesh, co, W, names, tie, knot, shirt):
    """Cut the tie loose from the shirt it is modelled into.

    The tie faces are copied as a free two-sided strip that keeps the tie weights; the faces they came from stay on
    the body as plain black shirt, so nothing stretches between the two when the tie swings away. The faces that
    join the knot to the strip turn plain gold and stay where they are: black or sunk, they show as a wedge under
    the knot.
    """
    me = mesh.data
    bm = bmesh.new()
    bm.from_mesh(me)
    bm.verts.ensure_lookup_table()
    bm.normal_update()
    uv = bm.loops.layers.uv.active
    faces = [f for f in bm.faces if all(tie[v.index] for v in f.verts)]
    join = [f for f in bm.faces if any(tie[v.index] for v in f.verts) and any(knot[v.index] for v in f.verts)]
    held = {v for f in join for v in f.verts}
    touching = [f for f in bm.faces if any(tie[v.index] for v in f.verts) and f not in join]
    black_uv = plain_black_uv(bm, uv, shirt)
    donor = min(faces, key=lambda f: (f.calc_center_median() - Vector(TIE_MID)).length)
    gold_uv = sum((l[uv].uv for l in donor.loops), Vector((0, 0))) / len(donor.loops)

    def outward(v):
        """Off the chest, never sideways: the folds of the knot have walls that would slide across the strip."""
        d = Vector((0.0, min(v.normal.y, -0.3), v.normal.z))
        return d.normalized()

    front = bmesh.ops.duplicate(bm, geom=faces)["geom"]
    back = bmesh.ops.duplicate(bm, geom=faces)["geom"]
    for geom, lift in ((front, TIE_LIFT), (back, 0.5 * TIE_LIFT)):
        for v in geom:
            if isinstance(v, bmesh.types.BMVert):
                v.co += outward(v) * lift
    bmesh.ops.reverse_faces(bm, faces=[f for f in back if isinstance(f, bmesh.types.BMFace)])
    # The shirt under the strip sinks, so a breathing belly does not come through the tie.
    for v in {v for f in faces for v in f.verts} - held:
        v.co -= outward(v) * TIE_SINK
    # Right under the knot the strip hardly leaves the chest, and a crack between its folds must not show black.
    under = [f for f in faces if min(v.co.z for v in f.verts) > TIE_GOLD_UNDER]
    touching = [f for f in touching if f not in under]
    for group, spot in ((touching, black_uv), (join + under, gold_uv)):
        for f in group:
            for l in f.loops:
                l[uv].uv = spot
    bm.to_mesh(me)
    bm.free()

    # The verts left behind move like the shirt around them.
    body = np.where(shirt)[0]
    tree = kdtree.KDTree(len(body))
    for k, i in enumerate(body):
        tree.insert(co[i], k)
    tree.balance()
    groups = [mesh.vertex_groups[name] for name in names]
    for i in np.where(tie)[0]:
        near = [body[k] for _, k, _ in tree.find_n(co[i], 6)]
        row = W[near].mean(axis=0)
        row = np.where(row >= np.sort(row)[-4], row, 0.0)
        row /= row.sum()
        for g, value in zip(groups, row):
            if value > 0:
                g.add([int(i)], float(value), "REPLACE")
            else:
                g.remove([int(i)])
    print("[build] tie split: %d faces copied twice, %d turned black, %d gold under the knot, %d verts in all" % (
        len(faces), len(touching), len(join), len(me.vertices)))


def clean_actions(arm):
    """Meshy names are 'Armature|Idle'; curves of the dropped bones and of the armature object itself go."""
    bones = {b.name for b in arm.data.bones}
    for action in list(bpy.data.actions):
        action.name = action.name.split("|")[-1]
        removed = 0
        for layer in action.layers:
            for strip in layer.strips:
                for slot in action.slots:
                    bag = strip.channelbag(slot)
                    if bag is None:
                        continue
                    for fc in list(bag.fcurves):
                        path = fc.data_path
                        name = path.split('"')[1] if path.startswith("pose.bones[") else None
                        if name is None or name not in bones:
                            bag.fcurves.remove(fc)
                            removed += 1
        action.use_fake_user = True
        print("[build] action %-22s frames %s, dropped %d curves" % (action.name, tuple(action.frame_range), removed))


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    arm, mesh = import_source()
    weld(mesh)
    me = mesh.data
    co = np.zeros(len(me.vertices) * 3)
    me.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    print("[build] welded verts %d, bbox %s %s" % (len(co), co.min(axis=0).round(3), co.max(axis=0).round(3)))
    joints, toe_dir, ball_xy = measure(co)
    refit_skeleton(arm, joints)
    paint(mesh, arm, toe_dir, ball_xy)
    clean_actions(arm)
    # The Meshy clips keyed the armature object itself at 90 degrees; with those curves gone it stays upright.
    arm.animation_data.action = None
    arm.location = (0, 0, 0)
    arm.rotation_euler = (0, 0, 0)
    arm.rotation_quaternion = (1, 0, 0, 0)
    arm.scale = (1, 1, 1)
    for pb in arm.pose.bones:
        pb.matrix_basis.identity()
    bpy.context.scene.render.fps = 30
    try:
        bpy.ops.file.pack_all()
    except RuntimeError as e:
        print("[build] pack_all:", e)
    bpy.ops.wm.save_as_mainfile(filepath=OUT)
    print("[build] saved", OUT)


main()
