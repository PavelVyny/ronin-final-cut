# T-070 His stolen shurikens come 30% slower

**Block:** session 5, Sunday feedback · **Status:** done

## Goal

The author played session-5m, Sun 15 Nov: once the Publisher has stolen the shuriken he throws them one after
another and they are almost impossible to dodge (about five restarts). The time between his throws becomes 30%
longer; the hero's `ShurikenCooldown=0.4` stays.

## Decision

- He throws shurikens two ways, and both were too quick: his plain AI fires the stolen button whenever its cooldown
  allows (`ShurikenCooldown × BossStolenCooldown` = 0.4 s), and the FanBurst chain throws three on its own clock
  (`ComboBurstGap` = 0.2 s, faster than his cooldown on purpose).
- A new key `[JamBalance] BossShurikenCooldownScale=1.3` scales both: his cooldown becomes 0.52 s and the burst's gap
  0.26 s. `BossStolenCooldown` (every stolen move) is not touched, so his dash keeps its timing.

## Test seam

`Jam.Loadout.HisShurikensComeSlowerThanTheHeros` and `Jam.Combo.HisBurstIsSpacedLikeHisShurikens`, red first
(0.4 instead of 0.52; 12 ticks instead of 16).
