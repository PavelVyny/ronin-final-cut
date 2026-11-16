# T-037 A full Ultimate is called out

**Block:** session 4 (playtest fixes) · **Status:** done

## Goal

The author's playtest: "I never found the Ultimate in three fights. One line of HUD text 'E ULTIMATE 64%' is easy
to miss." Option taken in chat: a meter bar and a ready banner.

## Scope

- Simulation: `EJamEvent::UltimateReady` on the step a meter fills (`JamUltimateRules::Charge`), once per filling.
- Sound: cue 14, `sfx_ultimate_ready_01` (a rising chime stub, `Tools/Audio/make_stub_sfx.py`), heard for the hero's
  meter only. One name more than the 13 approved in Q8: Kirill gets it on the list.
- `JamTeachRules::UltimateCallFor`: None, Banner or BannerAndPrompt. A call while the hero owns the Ultimate, the
  meter is full, it is not winding up and the fight runs or the Publisher is stunned; the call over his head only
  until he has used it once in the session (`FJamLessons::Ultimates`).
- HUD: a magenta meter bar under the health bar with "ULTIMATE  E"; the pulsing "E   ULTIMATE READY" banner low in
  the middle; "E  ULTIMATE: CUT ACROSS THE ARENA" over the hero (the parry call wins when both want the spot).
  Effects: a turning dashed ring at the hero's feet while it is ready.
- Demo: `-JamDemoHoldUltimate` (through `JAM_DEMO_ARGS` of `demo_clip.sh`) keeps the bot's hands off E for a clip.

## Test seam

`Jam.Ultimate.AFullMeterIsAnnouncedOnce`, `Jam.Sound.TheHeroHearsHisUltimateFill`,
`Jam.Teach.AFullUltimateIsCalledOut`: red first (3 failing on stubs).

## Done

153 `Jam.` tests green, 0 code check warnings. Clip `Clips/t037_ultimate_ready.mp4` (40 s, bot holding E): from
26 s the banner, the call over the hero, the ring and the full bar. A first clip without the switch showed nothing:
the bot presses E the step the meter fills, and the call is off during the wind-up, as the rule says.
