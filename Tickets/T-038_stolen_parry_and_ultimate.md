# T-038 The Publisher uses the Parry and the Ultimate he stole

**Block:** session 5, fight · **Status:** done

## Goal

Today a stolen Parry or Ultimate only leaves the hero's hands: the Publisher never uses them. The author's answer
(Sat 14 Nov 18:30, option "Stance + timed line"):

- **Parry: a shield stance.** He raises a visible shield (0.3 s raise, the telegraph), holds it 1.2 s; every hero
  shot that touches him in that time comes back at the hero (who can parry it again). Cooldown 4 s. He raises it
  when hero shots fly at him.
- **Ultimate: a timed line.** His meter would never fill from damage dealt (a 100 HP hero), so he gets it on a timer:
  every 14 s, a 0.9 s wind-up with his red lane drawn (T-046), 35 damage, the line locked at the press (no tracking).

## Scope

- Balance in `[JamBalance]`: `BossParryRaise=0.3`, `BossParryHold=1.2`, `BossParryCooldown=4`,
  `BossUltimateInterval=14`, `BossUltimateWindUp=0.9`, `BossUltimateDamage=35`.
- The stance is the same `FJamParryState` and the same deflect code: in his hands the window opens after the raise
  and lasts the hold. He stands while raising and holding.
- His meter: full when the interval since he stole it (or since his last slash) has passed; the HUD may show it.
- The bot hero: does not shoot into a raised shield, steps off a locked lane.
- View: the shield cage on him (T-035 look) from the raise on; his lane red while he winds up (exists).

## Test seam

`Jam.Boss.RaisesTheShieldAtIncomingShots`, `Jam.Boss.ShieldSendsShotsBack` (only after the raise),
`Jam.Boss.NeverParriesWithoutTheMechanic`, `Jam.Boss.UltimateOnItsInterval`, `Jam.Boss.UltimateLineIsLocked`,
`Jam.Boss.UltimateHurtsByItsOwnDamage`. Bot table stays 60 of 60.

## As built

- `FJamParryState::ActiveFromTick`, `JamParryRules::Start(Tick, Balance, RaiseSeconds)`, `IsRaising`, `ShotWithin`.
  In his hands (`InHandsOf`) the window is the hold, his cooldown, his damage scale (1), the return speed x0.8.
- `JamBossRules::Decide`: the shield first (a hero shot reaches him within raise + hold), then the Ultimate (full
  meter, hero within the line), then the old order. `IsBusy` holds him through the stance and the wind-up.
- `JamUltimateRules::Charge`: only the hero's hits fill a meter; his fills by the clock while he owns it.
- Only the hero's own wind-up slows the world. The strike-parry (T-044) is the hero's only.
- The shield goes up out of the floor during the raise (`DrawShield` rise), then shrinks over the hold.
- Bot: holds fire into a raised shield, walks off his locked line sideways, dashes at the last moment.

## Test seam (as built)

`Jam.Boss.RaisesTheShieldAtIncomingShots`, `Jam.Boss.ShieldSendsShotsBack`, `Jam.Boss.NeverParriesWithoutTheMechanic`,
`Jam.Boss.UltimateOnItsInterval`, `Jam.Boss.SlashesWhenHisMeterIsFull`, `Jam.Boss.UltimateLineIsLocked`,
`Jam.Bot.StepsOffHisUltimateLine`, `Jam.Bot.HoldsFireIntoHisShield`. Red first: 5 of 6 boss tests on stubs (the
guard `NeverParriesWithoutTheMechanic` was green from the start).

## Bot table

60 of 60, median 108 s (65 to 162; T-055: 81 s), closest win at 66 health (T-055: 28). His stances stop him, so the
bot's fight is longer and safer; T-056 puts a fan right after the shield drops. The in-game demo bot is hit much
harder than the table shows (12 health at 20 s in the clip), so a person's run is the real check.

## Done

193 `Jam.` tests green, code check without warnings. Clip `Clips/t038_shield_and_line.mp4` (90 s, bot gives away
Parry and Ultimate on the cards): his red lane at about 22 s, the shield cage after the FINAL NOTE at about 80 s.
