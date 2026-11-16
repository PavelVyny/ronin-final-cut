# T-074 The hero throws 30% more shurikens for the same damage a second

**Block:** session 5, Sunday afternoon · **Status:** done

## Goal

The author: the hero's shurikens 30% more per second, the same damage per second (cooldown 0.4 s to about 0.31 s,
damage 9 to about 6.9, DPS within 2%). The hero only: the Publisher's stolen shuriken keeps exactly his current
damage, spacing and speed. Answer to the question about the explosive shuriken (Sun 15 Nov): keep the explosions a
second the same, every 4th instead of every 3rd.

## Decision

- The simulation runs at 60 steps a second: 0.31 s rounds to 19 steps (+26%), 0.3 s is 18 steps (+33%); 0.3 s is the
  closer to +30%. `ShurikenCooldown=0.3`, `ShurikenDamage=6.75`: 22.5 damage a second, exactly as before.
  `ShurikenExplosiveEvery=4`: 0.83 blasts a second, as before. The Ultimate meter fills by damage, so its pace stays.
- His copy got its own base keys, `BossShurikenCooldown=0.4` and `BossShurikenDamage=9`, scaled as before by
  `BossStolenCooldown`, `BossShurikenCooldownScale` (T-070) and `BossStolenDamage`: 0.52 s, 8.1 damage, speed
  2400 x 0.8 = 1920. His burst gap (`ComboBurstGap`) never used the hero's cooldown.

## Test seam

`Jam.ShurikenBalance.HisStolenCopyKeepsItsNumbers` pins his numbers from the game's own [JamBalance] (green before the
change, green after); `Jam.ShurikenBalance.HeroThrowsFasterForTheSameDps` red first (0.4 s), green after.
`Jam.Loadout.HisShurikensComeSlowerThanTheHeros` now feeds his own base key.
