# T-023 Cosmetic layers on stand-ins

**Block:** E · **Status:** done

## Goal

The hook of the game: each of the Publisher's cuts strips a layer of polish from the hero and the world, down to a
grey bean on the engine grid; the stun gives everything back at once. The boss stays polished all the way. Built on
stand-ins, so the real art only replaces shapes and materials later.

## Scope

- `JamLayerRules::For(CutsDone, Phase, Settings)`: which of the five layers are left. `[JamLayers]` in
  `Config/DefaultGame.ini` says after which cut each one goes: effects 1, materials 2, animation, model and HUD 3.
  With four cuts (T-021) it becomes one per cut by editing the ini.
- Effects: the hero's glow light, the sweep of his blade and the trail of his dash. The boss's effects and every
  warning stay.
- Materials: the hero and the arena switch to the engine's `WorldGridMaterial`; the neon floor lines go.
- Animation: the stand-in ronin walks (bob, lean, arm swing); without the layer he stands in a T-pose.
- Model: the ronin of basic shapes (body, head, eyes, kasa hat, arms, blade) turns into the bean with a visor.
- HUD: the health bars go, the plain text stays.

## Test seam

`Jam.Layers.FullBeforeTheFirstCut`, `Jam.Layers.EachCutDropsItsLayers`, `Jam.Layers.BeanAfterTheLastCut`,
`Jam.Layers.EverythingReturnsAtTheStun`, `Jam.Layers.IniWalksTheWholeRoad` (the real ini: every cut takes something,
the last cut leaves nothing, at least four layers: the "never cut" line of the plan). Red first with a stub rule
(3 failing), then green.

## Done

Build and 112 `Jam.` tests green, 0 code check warnings. Clip of a whole bot fight
`Clips/t023_road_to_the_bean.mp4`, Shipping package `Saved/Packaged/Windows`.
