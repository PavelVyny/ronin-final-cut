# T-062 The generated props replace the greybox shapes

**Block:** session 5, models · **Status:** done

## Goal

The author, Sun 15 Nov 00:30: the approved props go into the game with the characters. The shuriken, the cash
stack, the paper dollar and the memo card replace their greybox shapes; the katana is already in the hero's hand
(T-061).

## Decision

- **Shuriken:** the Hunyuan3D model, laid flat and spinning, worn in the neon of its side (the hero's cyan, the
  Publisher's red, explosive yellow): whose it is and whether it explodes must read at a glance, so the colour stays
  the code's, the shape is the model's.
- **Memo and bill:** cards in `M_JamCard` (unlit, two-sided, texture times colour times glow) with the round-5
  textures `r5_memo_card` and `r5_dollar_card`.
- **Cash stack (the heal of T-049):** the model with its own textures, 90 cm long, still bobbing and turning, the
  pulsing ring kept. It has its own small pool in the effects view: the neon pool repaints everything it hands out.
- **The hit flash of the characters** was turned down (FlashGlow 2 to 0.4): the Publisher is hit all the time and
  read as a white shape for whole seconds.

## Test seam

Presentation only, no new decision: checked from the game camera in the clip of the milestone. The 218 `Jam.` tests
stay green.

## As built

- `Tools/Editor/create_card_material.py`; `JamShapes::Shuriken`, `CashStack`, `MemoCard`, `DollarCard`, `Card`.
- `AJamProjectilesView` (shuriken, memo), `AJamMoneyView` (bills; its unused glow parameter removed),
  `AJamEffectsView::DrawCash`.
- A unity-build clash of names from T-061 fixed on the way (`StaggerHit`, `MinClipRate`, `MaxClipRate`).
