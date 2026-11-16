"""
KIND: offline tool (headless Blender).

Reworks the Meshy clips on the rebuilt Publisher rig and bakes them as clean per-frame keys:

  * every clip starts at the origin and stands on the ground (constant offset per clip);
  * horizontal travel goes to the `root` bone, so Unreal can use it as root motion or lock it;
  * arms are kept out of the belly, chest and hips (rotation about the shoulder, smoothed in time);
  * the belly gets a small, heavily damped follow-through, the jacket flaps hang, lag and yield to the thighs;
  * the tie hangs from its knot on two bones: it lies on the belly, follows gravity sideways and lags;
  * in the slam the raised hand turns its palm forward and stays in line with the forearm;
  * a `T_Pose` clip is added.

    blender -b --factory-startup --python Tools/Art/publisher_rig/rework_clips.py -- <rig.blend> <out.blend> [meta.json]
"""
import json
import math
import sys

import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

FPS = 30
LOOPS = ("Idle", "Slow_Orc_Walk")
TRAVEL = ("Hit_Reaction", "Knock_Down", "Kung_Fu_Punch")
HELPERS = ("root", "belly", "jacket_front_l", "jacket_front_r", "jacket_back", "tie_01", "tie_02")
ARM_CHAIN = ("upperarm", "lowerarm", "hand")
# Clips whose raised hand is turned palm forward and straightened at the wrist.
RAISED_HAND = ("Charged_Ground_Slam",)
PALM_TURN, WRIST_BEND = math.radians(150), math.radians(12)
# How far the fingers of a raised hand lean from the shoulder-wrist line toward straight up.
HAND_UP = 0.3
# Share of the elbow bend that a raised arm gives up.
ARM_STRAIGHT = 0.85
# A raised arm leans this far from straight up freely; beyond it only ARM_LEAN_KEEP of the lean is left.
ARM_LEAN, ARM_LEAN_KEEP = math.radians(12), 0.3
# Tip of the tie in the rest pose, and how far its lower half may bend against the upper one.
TIE_TIP, TIE_BEND = (0.0, -0.59, 1.51), math.radians(14)

# Torso colliders in rest space: (carrier bone, centre z, half height, |x| limit of the measured band).
COLLIDERS = (("pelvis", 1.12, 0.32, 0.80), ("spine_01", 1.47, 0.42, 0.80), ("spine_03", 1.84, 0.30, 0.68))
# Points of the arm that must stay outside: (bone, fraction along the bone to its child, radius).
ARM_POINTS = (("upperarm", 0.55, 0.11), ("lowerarm", 0.0, 0.11), ("lowerarm", 0.5, 0.09), ("hand", 0.0, 0.07),
              ("hand", 0.7, 0.05))


def gauss(values, sigma, loop):
    """Gaussian blur of a list of Vectors (edge clamped, or wrapped for a loop)."""
    n = len(values)
    r = max(1, int(sigma * 3))
    ws = [math.exp(-0.5 * (k / sigma) ** 2) for k in range(-r, r + 1)]
    out = []
    for i in range(n):
        acc = values[0] * 0.0
        tot = 0.0
        for k, w in zip(range(-r, r + 1), ws):
            j = (i + k) % (n - 1) if loop else min(n - 1, max(0, i + k))
            acc += values[j] * w
            tot += w
        out.append(acc / tot)
    return out


def sample(scene, arm, action):
    arm.animation_data.action = action
    arm.animation_data.action_slot = action.slots[0]
    a, b = int(action.frame_range[0]), int(action.frame_range[1])
    frames = list(range(a, b + 1))
    world = {pb.name: [] for pb in arm.pose.bones}
    for f in frames:
        scene.frame_set(f)
        for pb in arm.pose.bones:
            world[pb.name].append(pb.matrix.copy())
    return frames, world


def ground_level(scene, arm, mesh, frames):
    """Height the clip stands on: a low percentile of the lowest deformed vertex over the frames."""
    lows = []
    for f in frames[::3]:
        scene.frame_set(f)
        dg = bpy.context.evaluated_depsgraph_get()
        ev = mesh.evaluated_get(dg)
        me = ev.to_mesh()
        co = np.empty(len(me.vertices) * 3, dtype=np.float32)
        me.vertices.foreach_get("co", co)
        ev.to_mesh_clear()
        lows.append(float(co[2::3].min()))
    return float(np.percentile(lows, 25)), min(lows)


def measure_colliders(mesh):
    co = np.array([v.co[:] for v in mesh.data.vertices])
    out = []
    for carrier, cz, hz, xlim in COLLIDERS:
        band = co[(np.abs(co[:, 2] - cz) < 0.07) & (np.abs(co[:, 0]) < xlim)]
        y0, y1 = band[:, 1].min(), band[:, 1].max()
        out.append((carrier, Vector((0, (y0 + y1) / 2, cz)), Vector((np.abs(band[:, 0]).max(), (y1 - y0) / 2, hz))))
    return out


def arm_points(world, rest, side, i):
    pts = []
    order = ARM_CHAIN + ("hand_end",)
    for bone, t, radius in ARM_POINTS:
        m = world["%s_%s" % (bone, side)][i]
        head = m.translation
        k = ARM_CHAIN.index(bone)
        if k < 2:
            nxt = world["%s_%s" % (order[k + 1], side)][i].translation
        else:
            nxt = head + m.to_3x3() @ Vector((0, 0.2, 0))
        pts.append((head.lerp(nxt, t), radius))
    return pts


def clamp_arms(world, rest, colliders, n, loop, log):
    """Rotates each arm about its shoulder until it clears the torso, then smooths the correction in time."""
    for side in ("l", "r"):
        names = ["%s_%s" % (b, side) for b in ARM_CHAIN]
        corr = []
        worst = 0.0
        for i in range(n):
            q = Quaternion()
            sh = world[names[0]][i].translation.copy()
            points = arm_points(world, rest, side, i)
            frames_of = []
            for carrier, centre, radii in colliders:
                d = world[carrier][i] @ rest[carrier].inverted()
                frames_of.append((d, d.inverted(), centre, radii))
            for _ in range(48):
                push = None
                depth = 0.0
                for p0, radius in points:
                    p = sh + q @ (p0 - sh)
                    for d, inv, centre, radii in frames_of:
                        loc = inv @ p - centre
                        rr = Vector((radii.x + radius, radii.y + radius, radii.z + radius))
                        e = (loc.x / rr.x) ** 2 + (loc.y / rr.y) ** 2 + (loc.z / rr.z) ** 2
                        if e < 1.0 and 1.0 - e > depth:
                            depth = 1.0 - e
                            normal = d.to_3x3() @ Vector((loc.x / rr.x ** 2, loc.y / rr.y ** 2, loc.z / rr.z ** 2))
                            push = (p, normal.normalized())
                if push is None:
                    break
                worst = max(worst, depth)
                lever = push[0] - sh
                axis = lever.cross(push[1])
                if axis.length < 1e-6:
                    break
                q = Quaternion(axis.normalized(), math.radians(1.5)) @ q
            corr.append(q)
        # Smooth as 4-vectors on one hemisphere.
        vecs = [Vector((c.w, c.x, c.y, c.z)) for c in corr]
        vecs = gauss(vecs, 2.0, loop)
        used = 0.0
        for i in range(n):
            v = vecs[i].normalized()
            q = Quaternion((v[0], v[1], v[2], v[3]))
            used = max(used, math.degrees(q.angle))
            sh = world[names[0]][i].translation.copy()
            fix = Matrix.Translation(sh) @ q.to_matrix().to_4x4() @ Matrix.Translation(-sh)
            for name in names:
                world[name][i] = fix @ world[name][i]
        log("    arm_%s: deepest overlap %.2f, largest correction %.0f deg" % (side, worst, used))


def spring(targets, omega, zeta, loop, sub=4):
    """A damped particle following the target positions (list of Vectors), one output per frame."""
    dt = 1.0 / (FPS * sub)
    n = len(targets)
    x = targets[0].copy()
    v = x * 0.0
    out = [None] * n
    passes = 3 if loop else 1
    for p in range(passes):
        for i in range(n):
            a = targets[i - 1] if i else (targets[n - 2] if loop else targets[0])
            b = targets[i]
            for s in range(sub):
                t = a.lerp(b, (s + 1) / sub)
                acc = omega * omega * (t - x) - 2.0 * zeta * omega * v
                v = v + acc * dt
                x = x + v * dt
            out[i] = x.copy()
    return out


def add_belly(world, rest, n, loop):
    carrier = "spine_01"
    centre = rest["belly"].translation + Vector((0, -0.25, 0))
    deltas = [world[carrier][i] @ rest[carrier].inverted() for i in range(n)]
    targets = [deltas[i] @ centre for i in range(n)]
    sim = spring(targets, 2 * math.pi * 3.4, 0.5, loop)
    peak = 0.0
    world["belly"] = []
    for i in range(n):
        off = deltas[i].to_3x3().inverted() @ (sim[i] - targets[i]) * 0.8
        if off.length > 1e-6:
            off = off.normalized() * 0.05 * math.tanh(off.length / 0.05)
        peak = max(peak, off.length)
        world["belly"].append(deltas[i] @ Matrix.Translation(off) @ rest["belly"])
    return peak


def add_flap(world, rest, name, carrier, outward, thighs, n, loop):
    """outward: -1 for the front flaps (they swing to -Y), +1 for the back flap."""
    head = rest[name].translation
    length = 0.34
    deltas = [world[carrier][i] @ rest[carrier].inverted() for i in range(n)]
    hang = []
    for i in range(n):
        down = deltas[i].to_3x3().inverted() @ Vector((0, 0, -1))
        theta = max(0.0, min(math.radians(35), 0.5 * math.atan2(outward * down.y, -down.z)))
        if down.z > 0:
            theta = math.radians(35) if outward * down.y > 0 else 0.0
        phi = max(-0.2, min(0.2, 0.5 * math.atan2(down.x, max(1e-3, -down.z))))
        hang.append((theta, phi))
    tips = []
    for i in range(n):
        theta, phi = hang[i]
        local = Vector((math.sin(phi), outward * math.sin(theta), -math.cos(theta))).normalized() * length
        tips.append(deltas[i] @ (head + local))
    sim = spring(tips, 2 * math.pi * 1.9, 0.5, loop)
    peak = 0.0
    world[name] = []
    for i in range(n):
        v = deltas[i].inverted() @ sim[i] - head
        theta = math.atan2(outward * v.y, -v.z)
        phi = max(-0.25, min(0.25, math.atan2(v.x, max(1e-3, -v.z))))
        pushed = 0.0
        for thigh, calf in thighs:
            u = deltas[i].to_3x3().inverted() @ (world[calf][i].translation - world[thigh][i].translation)
            flex = math.atan2(outward * u.y, -u.z)
            pushed = max(pushed, min(math.radians(24), 0.35 * max(0.0, flex - math.radians(15))))
        theta = max(theta, pushed, 0.0)
        theta = min(theta, math.radians(38))
        peak = max(peak, theta)
        # About +X the hanging direction -Z turns to +Y, so the front flaps need the negative angle.
        rot = Matrix.Rotation(phi, 4, "Y") @ Matrix.Rotation(outward * theta, 4, "X")
        swing = Matrix.Translation(head) @ rot @ Matrix.Translation(-head)
        world[name].append(deltas[i] @ swing @ rest[name])
    return math.degrees(peak)


def smooth01(a, b, x):
    t = max(0.0, min(1.0, (x - a) / (b - a)))
    return t * t * (3.0 - 2.0 * t)


def about(point, axis, angle):
    return Matrix.Translation(point) @ Matrix.Rotation(angle, 4, axis) @ Matrix.Translation(-point)


def relax_raised_hands(world, rest, n, loop, log):
    """A raised arm turns its palm forward and keeps the hand in line with the forearm.

    The turn is shared by the whole arm (about the shoulder-wrist line, so the wrist stays in place), the forearm and
    the hand; the bend of the wrist is cut down to WRIST_BEND while the arm is up.
    """
    forward = Vector((0.0, -1.0, -0.3)).normalized()
    for side in ("l", "r"):
        up, lo, ha = ("%s_%s" % (b, side) for b in ARM_CHAIN)
        palm_local = rest[ha].to_3x3().inverted() @ Vector((0, 0, -1))
        hand_rel = rest[lo].inverted() @ rest[ha]
        raised, turns = [], []
        prev = 0.0
        for i in range(n):
            sh, el, wr = (world[b][i].translation for b in (up, lo, ha))
            axis = (wr - el).normalized()
            palm = world[ha][i].to_3x3() @ palm_local
            p = palm - axis * palm.dot(axis)
            t = forward - axis * forward.dot(axis)
            r = smooth01(0.15, 0.55, (wr - sh).normalized().z) * smooth01(0.25, 0.55, t.length)
            a = prev
            if r > 0.0 and p.length > 1e-4:
                a = math.atan2(axis.dot(p.cross(t)), p.dot(t))
                while a - prev > math.pi:
                    a -= 2 * math.pi
                while a - prev < -math.pi:
                    a += 2 * math.pi
            elif r == 0.0:
                a = 0.0
            prev = a
            raised.append(r)
            turns.append(r * max(-PALM_TURN, min(PALM_TURN, a)))
        turns = gauss(turns, 2.0, loop)
        raised = gauss(raised, 2.0, loop)
        peak_turn = peak_bend = 0.0
        for i in range(n):
            a, r = turns[i], raised[i]
            if r < 1e-4:
                continue
            peak_turn = max(peak_turn, abs(a))
            sh, wr = world[up][i].translation.copy(), world[ha][i].translation.copy()
            fix = about(sh, (wr - sh).normalized(), 0.5 * a)
            for b in (up, lo, ha):
                world[b][i] = fix @ world[b][i]
            el, wr = world[lo][i].translation.copy(), world[ha][i].translation.copy()
            axis = (wr - el).normalized()
            world[lo][i] = about(el, axis, 0.3 * a) @ world[lo][i]
            world[ha][i] = about(wr, axis, 0.2 * a) @ about(el, axis, 0.3 * a) @ world[ha][i]
            # Shoulder: the fat shoulder stays upright with the torso, so an arm that leans far out reads as bent in
            # the middle of the sleeve. Bring the raised arm closer to straight up.
            sh = world[up][i].translation.copy()
            line = (world[ha][i].translation - sh).normalized()
            lean = line.angle(Vector((0, 0, 1)))
            if lean > ARM_LEAN:
                axis = line.cross(Vector((0, 0, 1))).normalized()
                fix = about(sh, axis, r * (1 - ARM_LEAN_KEEP) * (lean - ARM_LEAN))
                for b in (up, lo, ha):
                    world[b][i] = fix @ world[b][i]
            # Elbow: the short forearm hides in the sleeve, so a bent elbow reads as a curved arm. Lay both arm bones
            # on the shoulder-wrist line.
            line = (world[ha][i].translation - world[up][i].translation).normalized()
            for chain in ((up, lo, ha), (lo, ha)):
                a0 = world[chain[0]][i].translation.copy()
                bone = (world[chain[1]][i].translation - a0).normalized()
                turn = bone.rotation_difference(line)
                fix = about(a0, turn.axis, r * ARM_STRAIGHT * turn.angle)
                for b in chain:
                    world[b][i] = fix @ world[b][i]
            # Wrist: split the hand's rotation into a twist about the bone and the bend that is left; cut the bend.
            loc, q, _ = (hand_rel.inverted() @ world[lo][i].inverted() @ world[ha][i]).decompose()
            twist = Quaternion((q.w, 0.0, q.y, 0.0))
            if twist.magnitude > 1e-8:
                twist.normalize()
                swing = q @ twist.inverted()
                if swing.w < 0:
                    swing.negate()
                bend = swing.angle
                peak_bend = max(peak_bend, bend)
                if bend > WRIST_BEND:
                    swing = Quaternion(swing.axis, bend - r * (bend - WRIST_BEND))
                    basis = Matrix.Translation(loc) @ (swing @ twist).to_matrix().to_4x4()
                    world[ha][i] = world[lo][i] @ hand_rel @ basis
            # The wide sleeve reads steeper than the bones under it: point the fingers along the arm, a bit upward.
            sh, wr = world[up][i].translation, world[ha][i].translation.copy()
            fingers = (world[ha][i].to_3x3() @ Vector((0, 1, 0))).normalized()
            aim = (wr - sh).normalized().lerp(Vector((0, 0, 1)), HAND_UP).normalized()
            lift = fingers.rotation_difference(aim)
            world[ha][i] = about(wr, lift.axis, r * lift.angle) @ world[ha][i]
        log("    hand_%s: raised in %d frames, palm turned up to %.0f deg, wrist bend was up to %.0f deg" % (
            side, sum(1 for r in raised if r > 0.5), math.degrees(peak_turn), math.degrees(peak_bend)))


def add_tie(world, rest, n, loop):
    """The tie hangs from its knot: it lies on the belly, swings sideways with gravity and lags behind the body."""
    knot, mid = rest["tie_01"].translation.copy(), rest["tie_02"].translation.copy()
    tip = Vector(TIE_TIP)
    length = (tip - knot).length
    along = (tip - knot).normalized()
    side = Vector((1, 0, 0))
    out = Vector((0, along.z, -along.y))

    def angles(v):
        return math.atan2(v.dot(side), max(1e-3, v.dot(along))), math.atan2(v.dot(out), max(1e-3, v.dot(along)))

    def direction(phi, theta):
        return (along + side * math.tan(phi) + out * math.tan(theta)).normalized()

    mid_rest = angles(mid - knot)[1]
    chest, floor, targets = [], [], []
    for i in range(n):
        d = world["spine_03"][i] @ rest["spine_03"].inverted()
        inv = d.inverted()
        belly = world["belly"][i] @ rest["belly"].inverted()
        waist = world["spine_01"][i] @ rest["spine_01"].inverted()
        ribs = world["spine_02"][i] @ rest["spine_02"].inverted()
        # Where the body under the tie is now: the tie may lift off it, never sink into it.
        under_tip = inv @ ((belly @ tip) * 0.75 + (waist @ tip) * 0.25) - knot
        under_mid = inv @ ((belly @ mid) * 0.3 + (ribs @ mid) * 0.7) - knot
        low = max(angles(under_tip)[1], angles(under_mid)[1] - mid_rest)
        down = d.to_3x3().inverted() @ Vector((0, 0, -1))
        phi, theta = angles(down)
        phi = max(-0.5, min(0.5, 0.8 * phi))
        theta = min(math.radians(60), max(low, theta))
        chest.append(d)
        floor.append(low)
        targets.append(d @ (knot + direction(phi, theta) * length))
    upper = spring(targets, 2 * math.pi * 2.4, 0.6, loop)
    lower = spring(targets, 2 * math.pi * 1.6, 0.45, loop)
    world["tie_01"], world["tie_02"] = [], []
    peak_side = peak_lift = 0.0
    for i in range(n):
        dirs = []
        for sim in (upper, lower):
            phi, theta = angles(chest[i].inverted() @ sim[i] - knot)
            phi = max(-0.55, min(0.55, phi))
            theta = min(math.radians(60), max(floor[i], theta))
            dirs.append(direction(phi, theta))
            peak_side = max(peak_side, abs(phi))
            peak_lift = max(peak_lift, theta - floor[i])
        first = along.rotation_difference(dirs[0])
        second = dirs[0].rotation_difference(dirs[1])
        if second.angle > TIE_BEND:
            second = Quaternion(second.axis, TIE_BEND)
        hang = chest[i] @ Matrix.Translation(knot) @ first.to_matrix().to_4x4() @ Matrix.Translation(-knot)
        bend = Matrix.Translation(mid) @ second.to_matrix().to_4x4() @ Matrix.Translation(-mid)
        world["tie_01"].append(hang @ rest["tie_01"])
        world["tie_02"].append(hang @ bend @ rest["tie_02"])
    return math.degrees(peak_side), math.degrees(peak_lift)


def bake(arm, name, frames, world, rest_rel, parents):
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    slot = action.slots.new(id_type="OBJECT", name="Armature")
    layer = action.layers.new("Layer")
    strip = layer.strips.new(type="KEYFRAME")
    bag = strip.channelbag(slot, ensure=True)
    n = len(frames)
    for bone in arm.data.bones:
        b = bone.name
        locs, quats = [], []
        prev = None
        for i in range(n):
            if parents[b]:
                basis = rest_rel[b].inverted() @ world[parents[b]][i].inverted() @ world[b][i]
            else:
                basis = rest_rel[b].inverted() @ world[b][i]
            loc, q, _ = basis.decompose()
            if prev is not None and prev.dot(q) < 0:
                q.negate()
            prev = q
            locs.append(loc)
            quats.append(q)
        path = 'pose.bones["%s"].' % b
        for prop, rows, width in (("location", locs, 3), ("rotation_quaternion", quats, 4)):
            for k in range(width):
                fc = bag.fcurves.new(path + prop, index=k, group_name=b)
                fc.keyframe_points.add(n)
                flat = []
                for i in range(n):
                    flat += [frames[i], rows[i][k]]
                fc.keyframe_points.foreach_set("co", flat)
                for kp in fc.keyframe_points:
                    kp.interpolation = "LINEAR"
                fc.update()
    return action


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    bpy.ops.wm.open_mainfile(filepath=args[0])
    scene = bpy.context.scene
    arm = next(o for o in scene.objects if o.type == "ARMATURE")
    mesh = next(o for o in scene.objects if o.type == "MESH")
    lines = []

    def log(s):
        print(s)
        lines.append(s)

    for pb in arm.pose.bones:
        pb.rotation_mode = "QUATERNION"
    rest = {b.name: b.matrix_local.copy() for b in arm.data.bones}
    parents = {b.name: (b.parent.name if b.parent else None) for b in arm.data.bones}
    rest_rel = {b: (rest[parents[b]].inverted() @ rest[b] if parents[b] else rest[b]) for b in rest}
    colliders = measure_colliders(mesh)
    for carrier, centre, radii in colliders:
        log("collider %s centre (%.2f %.2f %.2f) radii (%.2f %.2f %.2f)" % (carrier, *centre, *radii))

    sources = sorted((a for a in bpy.data.actions), key=lambda a: a.name)
    meta = {}
    baked = []
    for src in sources:
        name = src.name
        loop = name in LOOPS
        frames, world = sample(scene, arm, src)
        n = len(frames)
        ground, lowest = ground_level(scene, arm, mesh, frames)
        pelvis = [world["pelvis"][i].translation.copy() for i in range(n)]
        if name == "Slow_Orc_Walk":
            origin = sum(pelvis, Vector()) / n
        else:
            origin = pelvis[0].copy()
        origin.z = ground
        shift = Matrix.Translation(-origin)
        for b in world:
            world[b] = [shift @ m for m in world[b]]
        log("== %s: %d frames, origin shift (%.2f %.2f), ground %.3f (lowest vertex %.3f)" % (
            name, n, origin.x, origin.y, ground, lowest))

        travel = Vector((0, 0, 0))
        if name in TRAVEL:
            flat = [Vector((p.x - origin.x, p.y - origin.y, 0)) for p in pelvis]
            smooth = gauss(flat, 5.0, False)
            roots = [s - smooth[0] for s in smooth]
            travel = roots[-1]
        else:
            roots = [Vector((0, 0, 0))] * n
        world["root"] = [Matrix.Translation(r) @ rest["root"] for r in roots]

        clamp_arms(world, rest, colliders, n, loop, log)
        if name in RAISED_HAND:
            relax_raised_hands(world, rest, n, loop, log)
        peak = add_belly(world, rest, n, loop)
        tie_side, tie_lift = add_tie(world, rest, n, loop)
        log("    tie: swings sideways up to %.0f deg, lifts off the belly up to %.0f deg" % (tie_side, tie_lift))
        legs = {s: ("thigh_" + s, "calf_" + s) for s in ("l", "r")}
        fl = add_flap(world, rest, "jacket_front_l", "spine_01", -1, [legs["l"]], n, loop)
        fr = add_flap(world, rest, "jacket_front_r", "spine_01", -1, [legs["r"]], n, loop)
        bk = add_flap(world, rest, "jacket_back", "pelvis", 1, [legs["l"], legs["r"]], n, loop)
        log("    belly follow-through peak %.1f cm, flaps peak front %.0f/%.0f deg, back %.0f deg" % (
            peak * 100, fl, fr, bk))

        info = {"frames": n, "seconds": round((n - 1) / FPS, 2), "loop": loop,
                "root_travel_m": [round(travel.x, 3), round(travel.y, 3)]}
        if name == "Slow_Orc_Walk":
            # Speed that keeps the planted foot still: mean backward speed of the lower foot.
            speeds = []
            for i in range(1, n):
                low = min(("foot_l", "foot_r"), key=lambda b: world[b][i].translation.z)
                speeds.append((world[low][i].translation.y - world[low][i - 1].translation.y) * FPS)
            info["walk_speed_m_s"] = round(float(np.median(speeds)), 3)
            log("    walk speed for no foot slide: %.2f m/s" % info["walk_speed_m_s"])
        meta[name] = info
        baked.append((name, frames, world))

    for src in sources:
        bpy.data.actions.remove(src)
    for name, frames, world in baked:
        bake(arm, name, frames, world, rest_rel, parents)
    tpose = {b: [rest[b], rest[b]] for b in rest}
    bake(arm, "T_Pose", [1, 2], tpose, rest_rel, parents)
    meta["T_Pose"] = {"frames": 2, "seconds": 0.03, "loop": True, "root_travel_m": [0, 0]}

    first = bpy.data.actions["Idle"]
    arm.animation_data.action = first
    arm.animation_data.action_slot = first.slots[0]
    scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=args[1])
    if len(args) > 2:
        with open(args[2], "w") as out:
            json.dump(meta, out, indent=2)
    if len(args) > 3:
        with open(args[3], "w") as out:
            out.write("\n".join(lines) + "\n")
    print("done", args[1])


main()
