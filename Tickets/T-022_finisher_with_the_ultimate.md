# T-022 The finisher with the Ultimate

**Block:** D · **Status:** done

## Goal

The end of the road: at zero the Publisher is stunned, everything returns at once (mechanics, layers, music), the
hero's meter is full, and only the Ultimate lands the final blow.

## Scope

- The stun (`JamOutcomeRules`, built in T-015) also fills the hero's meter.
- The win needs the final blow: a hit reported with `JamUltimateRules::SlashId`. A katana or ball hit on the stunned
  boss is a hit, fills the meter, and ends nothing; a slash that misses can be earned again that way.
- The stun card reads `E: the Ultimate lands the final blow.`

## Test seam

`Jam.Outcome.TheStunFillsTheMeter` (new), `Jam.Outcome.TheUltimateLandsTheFinalBlow` (was
`AHitOnTheStunnedBossWins`: the katana now hits without winning), `Jam.Outcome.EveryCutComesBeforeTheStun` (the hero
also presses E). Red first: 2 failing.

## Done

128 `Jam.` tests green, 0 code check warnings. Bot: 60 of 60. Clip of a whole fight down to the bean and back:
`Clips/t022_road_down_and_back.mp4`. Shipping package of block D.
