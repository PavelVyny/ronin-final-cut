# T-100: the hero's spotlight, an easier fight, the old cash stack, the comic's glass

Status: done (Sun 15 Nov 2026, session-5y; the glass in session-5z)

## Asked

The author's last list: the hero is too dark in the full art and gets his own spotlight, split off the Publisher's in
the intro; the fight easier (a fifth more health, harder hits after the first cut, checked with the bot); the heal back
to the Hunyuan3D cash stack, Rodin's read as a torn banknote.

## Done

- Spotlight: `AJamLightView` has a second, smaller and dimmer warm key light on the hero (450 cd against the
  Publisher's 1100, cones 6 and 14 degrees), with the full art only like the other. In the intro it starts inside the
  Publisher's beam, dark, and rides with the camera onto the hero (`JamSpotRules::SplitOff`, `SplitShare`: full light a
  quarter of the way over); from then on it trails him on the same spring and sways like the Publisher's, out of step.
  A skip lets the spring carry it to him. Test: `Jam.Spot.TheHerosSpotSplitsOffThePublishers`, red first.
- Balance: `HeroHealth` 100 to 120; `HeroCutDamageScale` 1.2 to 1.4. The bot (60 fights, three ways of answering the
  cards) wins 60 of 60 before and after; median fight 80 to 69 s, closest win 63 to 72 health (at 120 health and 1.2:
  closest 63). 1.5 and 1.6 took the mixed answers under the 65 s room of `Jam.Bot.EveryPolicyLeavesRoomAboveTheFloor`,
  so 1.4 is the most the author's floor allows. Test: `Jam.Bot.TheFinalBuildIsEasier`, red first.
  `Jam.Balance.ThePublisherIsHeavy` asked a slam to take a third of the hero; at 120 health it takes 28%, so it now asks
  for over a quarter.
- Cash stack: back to `/Game/Props/CashStack/model_glb` (Hunyuan3D), its old size and centre; the Rodin stone lanterns
  stay. THIRD_PARTY and the itch page say so.

- Comic glass (section four, asked after session-5y was pushed): the packaged build had no `sfx_comic_shatter_01`.
  The sounds are loaded by path, nothing on the map references them, and `/Game/Audio` was not in
  `DirectoriesToAlwaysCook`; of the 66 audio assets only the glass (added in T-097) was missing from the cooked asset
  registry of session-5x. `/Game/Audio` is now always cooked, and the glass plays once with the gauntlet's panel at 0.6
  (`JamComicRules::SoundVolume`), under the Publisher's line that starts with it (voice boost 1.3). Tests:
  `Jam.Comic.TheGlassIsHeardUnderTheVoice`, `Jam.Comic.ItsSoundsAreCooked`, red first.

## Not checked

Not run in a game window (the author validates in play): the hero's spot in the intro and the fight. The Rodin cash
stack's imported assets (`/Game/Props/CashStack/model`) and the unused `Art/Models/rodin/base_basic_shaded.glb` were
deleted on the author's yes (6 files, 42.6 MB, after a dry run).
