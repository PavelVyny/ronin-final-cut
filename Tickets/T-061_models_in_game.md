# T-061 The 3D hero and Publisher replace the stand-ins

**Block:** session 5, models · **Status:** done

## Goal

The author, Sun 15 Nov 00:30: the approved models go into the game now. The hero (Meshy model, rig and clips) and
the Publisher (the rebuilt rig of `Art/Models/boss_fixed`, seventh export) replace the shapes; the bean, the T-pose
and the other cosmetic layers keep working; the Publisher is 2.6 m to the hero's 1.8 m; the travel of
`Hit_Reaction`, `Knock_Down` and `Kung_Fu_Punch` is locked on the root.

## Decision

- **The hero is exported like the Publisher** (`Tools/Art/hero_export.py`, Blender): one mesh FBX in centimetres and
  one FBX per clip. A `root` bone is added under `pelvis`, and in `Hit_Reaction`, `Dead` and `Roll_Dodge` the
  horizontal travel of the hips moves to it, so Force Root Lock keeps him in place while the hips still drop. The
  merged Meshy clip file gave Unreal only one of its seven takes.
- **No animation blueprint.** `UJamPoseInstance` blends by hand in C++ (its proxy's `Evaluate`): the idle, the walk
  by the speed, and one action over them, from `spine_01` up while he walks, the whole body when he stands or the
  action says so. The T-pose layer is the reference pose.
- **The simulation sets the timing.** `JamPoseRules::CueFor` picks the clip from the step's events (strike, guard,
  slam circles, throws, Ultimate, hits, the stun); `Fit` starts and speeds the clip so its impact frame (measured
  with `Tools/Art/clip_impacts.py`) shows on the step the blow lands. The Publisher staggers only under hits of 25
  and more, a hit never cuts a strike or a guard.
- **One material for both** (`M_JamCharacter`): the Meshy textures, a white flash for hits, and the saturated parts
  of the base colour glow (the hero's piping), switched off with the effects layer. Albedo is scaled to 0.3 for the
  scene's locked exposure.
- **Kung_Fu_Punch** stays out of the fight (it belongs to the intro, T-030).

## Test seam

`Jam.Pose.*` (9 tests, red first against a stub): the strike and its lead, the second slash, the guard by the
window, the slam on its first new circle, his throws and his Ultimate, hits by weight and the stun, the priority of
a step, who lies down, `Fit`, the fades. The look is checked from the game camera in `Clips/t061_models_in_game.mp4`
(`-JamDemoSkipEvery=10`: the glow, the grid, the T-pose and the bean on the 3D hero).

## As built

- Import: `Tools/Editor/import_models.py` (classic FBX importer, GLB through Interchange),
  `create_character_materials.py` (textures from `Tools/Art/save_textures.py`), `report_models.py`.
- `AJamFighterView`: the skeletal model, the katana in `hand_r`, the bean kept; `Act` starts the cues.
- `/Game/Characters` and `/Game/Props` are always cooked: the views load them by path.
- Known: the Publisher's walk is played at up to 3.5 times its speed and still slides (he moves 6 times faster than
  the clip). The FBX importer also left `*.fbm` texture folders next to the sources and two assets of the first
  try (`/Game/Props/model_glb`, `result_animation_fbx_url_Roll_Dodge`); none is committed.
