# T-095 The author's fix list after session-5s: comic, banter, the hero's tier, the rune telegraph

**Block:** session 5, Sunday evening · **Status:** done, not yet seen in a game window

## Goal

The author played session-5s ("overall everything is good in the game") and asked for four changes in one build:

1. **The comic's first panel** holds longer and opens with a soft, slow push in on the dev at his computer, not a jerk
   to the left; mechanical keyboard typing under it, then the phone rings, and only after the ring the Publisher speaks.
2. **Banter:** when no line has played for more than 5 s, a pair plays, the Publisher first and Ronin answering; no
   repeat until every pair has played; any event line (a cut, a steal comment, the stun, the finale) cuts the banter
   off at once. Eight pairs, the author's text.
3. **The hero degrades later:** at the first cut the world degrades as before, but the hero keeps his 3D model and his
   dash's afterimages until the second cut.
4. **The slam's circles** get a top-tier telegraph before the first cut (a glowing rune ring that fills inward to the
   moment of impact, a soft inner glow, embers rising, a sharp burst when it lands), the feel of a polished action
   game's telegraph, nothing copied. The plain red circle stays for every tier after the first cut. The danger area
   must be the hit area.

## Decision

- **Comic:** panel 1 lasts 7 s, fades up from black (0.8 s) instead of sliding in, and pushes in on its centre from
  zoom 1.0 to 1.35 with no sideways move; `sfx_comic_typing_01` plays under it. Panel 2 (the call) lasts the ring
  (`sfx_comic_ring_01`, 2.5 s) and half a second, with no line; the Publisher's first line opens panel 3. The comic is
  22.5 s without the voice; the old 20 s cap of `Jam.Comic.FivePanelsTellTheCallInOrder` became 25 s for this.
- **Banter and barks share one bag:** the 8 pairs and the 4 old barks (12 fillers) are drawn without a repeat until
  all 12 have played, never the same twice running; one is drawn whenever the plain fight has been quiet for more
  than 5 s after the end of any line. So the old barks stay, at the banter's rhythm instead of their own timer. An
  event line drops the banter still queued and stops the line being said; a bark or another banter line does not.
- **The hero's art** is a layer of its own (`bHeroArt`, `HeroArtLostAt=2` in `[JamLayers]`); the hero view gets
  `JamLayerRules::HeroLayers`, which is the world's layers with his art standing for the full art.
- **The telegraph** (`JamFxRules::TelegraphLook`, `TelegraphOf`): in the full art each circle is a plane exactly the
  hit's diameter at the hit's centre, wearing `M_JamSlamRune` (the fal rune seal `T_JamSlamRune`, unlit and additive,
  `Tools/Editor/create_slam_rune.py`): its runes light from the rim inward as `Fill` (the hazard's own progress) grows,
  a hot front runs in with it and reaches the centre as the slam lands, the floor inside glows a little more as it
  closes, nothing shows outside the rim. Embers rise off the rim (`NS_JamRuneEmbers`, a copy of the charge aura's motes
  made through the editor's Niagara toolset, sized to the radius). The landing adds a hot orange spark burst
  (`NS_JamHitSparks`) to the dust's shockwave ring. The ring is a material and the embers and the burst are Niagara.

## Test seam

Red first: `Jam.Layers.TheHeroKeepsHisLookOneCutLonger`, `Jam.Comic.TheFirstPanelPushesInSlowly`,
`Jam.Comic.TheRingComesBeforeTheCall`, `Jam.Voice.BanterFillsTheSilence`, `Jam.Voice.EveryPairBeforeARepeat`,
`Jam.Voice.EventLinesCutTheBanterOff`, `Jam.Fx.TheRuneTelegraphMatchesTheHit`, `Jam.Fx.ASlamLandsWithASharpBurst`;
changed: `Jam.Layers.OneLayerPerCut`, `EachCutDropsItsLayers` (seven layers now), `Jam.Fx.SlamsAndBlastsRaiseDust...`
(the slam has a burst too), the comic's length cap. 294 tests green. The look, the timing of the banter, the comic's
sounds and fade and the hero at the first cut are not seen in a game window, by the author's ask.
