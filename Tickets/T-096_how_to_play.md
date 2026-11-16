# T-096: HOW TO PLAY, the itch page images, last look fixes

Status: done (Sun 15 Nov 2026, session-5u)

## Asked

- A HOW TO PLAY screen in the neon style (cyan and magenta on dark), drawn by the HUD: a stylised keyboard with only
  our keys lit and a mouse, thin leader lines to the labels like a schematic. The labels come from the real bindings.
- Shown once after the comic, before the intro, with PRESS ANY KEY and an auto-start countdown; F1 shows it again
  over the fight, which waits under it.
- The same screen as `Submission/howtoplay.png` (1920x1080) for the itch page.
- The itch page: the Publisher's letter as the hook, the steal paragraph and the tiers, the katana fact, the controls
  image, a trailer line; a banner from real frames and the cover's logo; five real screenshots.
- The author's later look: the drawn mouse looked odd (a generated neon mouse, the labels native, as in the comic); the
  comic's first panel a bit shorter; the theme in the middle of the Publisher's kung fu, not after his line; the
  Publisher imported again for the shirt fix (f3a5269).

## Done

- `Core/JamKeys.h`: every binding the game reads, shared by the controller, the game mode and the screen.
- `JamHowToRules`: the callouts from the keys (`CapOf`), the board, the mouse's parts, the labels in rows above and
  below the board and in a column by the mouse, one leader line per row of keys nearest the label; the countdown
  and the key that closes it. Tests: `Jam.HowTo.*` (four).
- `JamHUDHowTo.cpp`: lines under the keys, lit keys in cyan with a glow, the rest dim without one, the generated mouse
  (`/Game/UI/HowTo/T_HowToMouse`, `Art/HowTo/howto_mouse_a.png`) added on over its box, the labels and the prompt.
- The game mode: the screen after the comic with a 15 s countdown, F1 over the fight, the closing key swallowed;
  `-JamDemoHowTo` holds it for a capture.
- The comic's first panel 7 s to 5 s; the theme cue at half of the Publisher's show (`JamIntroRules::IsThemeCued`).
- The Publisher's mesh, clips and base colour imported again (`Tools/Editor/import_publisher_color.py` for the
  repainted texels, which `import_models.py` does not bring).
- `Submission/`: `howtoplay.png`, `banner.png` (`Tools/Art/make_banner.py`), `screens/1..5`, `itch_page.md`.

## Not checked in a game window

The screen's countdown variant after the comic, F1 over a running fight, the shorter first panel, the theme's new
cue and the Publisher's shirt: covered by tests where they are rules, not watched.
