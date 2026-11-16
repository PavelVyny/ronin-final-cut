# T-044 Parry on the strike (LMB)

**Block:** session 5 · **Status:** done

## Goal

The author's playtest (Sat 14 Nov 14:20): no separate parry key. LMB is the melee strike all the time; when an enemy
shot is about to land, the same press parries it.

## Scope

- `JamParryRules::ShotInReach`: an enemy projectile whose path over one parry window (0.25 s) touches the fighter.
  No new number: the parry catches exactly what the strike decided to parry.
- `JamFightRules`: the strike opens the parry instead of swinging when the hero owns the Parry, it is ready and a shot
  is in reach; otherwise it swings (or shoots the ball). `bParry` stays in the intent for the Publisher's brain (T-038).
- Q unbound. HUD row "LMB  PARRY", prompt "LMB  PARRY: SEND IT BACK".
- The hero bot parries with its strike, like a person.

## Test seam

`Jam.Parry.TheStrikeParriesAShotAboutToLand`, `Jam.Parry.TheStrikeSwingsWithNoShotNear`,
`Jam.Parry.TheStrikeOnlySwingsWithoutTheParry`; red first on a stub (2 of 3). `Jam.Bot.ParriesAMemoBeforeDashing`
now expects the strike instead of the parry key (the design changed on the author's word).

## Bot table

60 of 60 won, median 92 s (59 to 185), closest win at 16 health (was 87 before the change).

## Done

160 `Jam.` tests green. Clip `Clips/t044_parry_on_strike.mp4` (30 s, bot).
