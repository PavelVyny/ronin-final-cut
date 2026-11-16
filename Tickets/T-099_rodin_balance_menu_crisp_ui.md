# T-099: Rodin props, a kinder fight after the first cut, the menu's first frame, crisp UI

Status: done (Sun 15 Nov 2026, session-5x)

## Asked

The author's list: the two approved Hyper3D Rodin props in the game; the fight too hard after the first cut; the hero
and the Publisher turning to face each other as the menu opens; the UI soft and blurry, to be crisp at any resolution up
to ultrawide and 4K.

## Done

- Rodin props: `Tools/Art/decimate_glb.py` (headless Blender 5.2) decimates `Art/Models/rodin/cash_stack.glb` (500k to
  8.8k triangles, a glb with its textures) and `stone_lantern.glb` (1M to 9.4k, the FBX and PBR maps of a Meshy prop),
  textures at 2048. The cash stack replaces the heal's model (`JamShapes::CashStack`, shown 92 cm long as before); the
  stone lantern replaces `SM_Arena_stone_lantern`, the eight lanterns of the full-art arena. Its orange windows were
  just above the prop material's glow threshold, so `Tools/Art/brighten_windows.py` brightens them. Imports:
  `JAM_IMPORT_PROP` limits `import_models.py` and `create_arena_assets.py` to one prop. THIRD_PARTY and the itch page
  name Rodin.
- Balance: from the first cut on, every hit on the Publisher (all the hero's moves) is scaled by
  `HeroCutDamageScale=1.2` (DefaultGame.ini, `JamHealthRules::HeroDamageScale`). Before it nothing changes. Test:
  `Jam.Steal.TheHeroHitsHarderAfterTheFirstCut`, red first.
- Menu: the fighters' views snap to their facing on every menu frame (`AJamFighterView::Sync`'s `bSnap`): no
  Publisher turn and no turn lean from where they were spawned.
- Crisp UI: the canvas drew every text with the engine's 10-point Roboto stretched by its scale, so it was soft.
  `AJamHUD::DrawText` and `GetTextSize` now hide AHUD's: the glyphs are rasterised at the size shown. The whole HUD
  grows with the screen height over 1080 (`JamHudScaleRules`), its text and its pixel sizes (`JamHUDPixels.h`) alike, so
  it reads the same at 720p, ultrawide and 4K. Panels, bars and lines were already vector; the cursor is a hardware
  cursor at native pixels. Tests: `Jam.Hud.TheHudGrowsWithTheScreen`, `Jam.Hud.TextIsRasterisedAtItsShownSize`, red
  first.

## Not checked

Not run in a game window (the author validates in play): the new props' look and the lantern glow, and the text at
other resolutions. `Art/Models/rodin/base_basic_shaded.glb` (unused) waits for the author's yes to be deleted.
