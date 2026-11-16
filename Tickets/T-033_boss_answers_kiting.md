# T-033 The Publisher answers kiting: a volley that leads its target

**Block:** K1 · **Status:** done

## Goal

A hero who runs in a circle at range is hit unless he reacts. Found by the bot pass (T-016): with the straight volley
the worst moment of 60 won fights was 74 of 100 health, the Publisher never lands anything on a circling hero.

## Scope

- `JamBossRules::LeadDirection(From, TargetPos, TargetVelocity, ProjectileSpeed)`: the direction in which a projectile
  meets a target that keeps its course. A target that cannot be caught is aimed at directly.
- The volley is aimed with it: the middle memo meets a hero who keeps running, the fan covers a small change of pace.
  The answer of the player: change direction when he fires, or dash through.
- The bot gets the same answer: when its course runs into a projectile, it turns the circle the other way; the dash
  stays the last resort.
- The stolen shuriken stays straight for now; decided again after the numbers.

## Test seam

- `Jam.Boss.LeadsARunningTarget`: the lead direction meets a target that keeps its course; a standing target is aimed
  at directly; a target faster than the projectile is aimed at directly.
- `Jam.Boss.VolleyLeadsTheHero`: a hero who runs sideways and never turns is hit by the volley.
- `Jam.Bot.TurnsItsCircleBeforeAMemo`: a memo on the bot's course turns the circle; with a clear course it keeps it.
- The full-fight tests of T-016 stay green on the numbers of the game; the table goes to `DEVLOG.md`.

## Done

Tests green, the bot table shows that won fights now cost health.

## As built

107 `Jam.` tests green, the code check has no warnings. On the numbers of the game the bot wins 60 of 60 fights in
all six orders, the median won fight is 167 seconds (126 to 176), and the closest win ends with 5 of 100 health
(74 before this ticket).

- `LeadDirection` takes a head start too: a memo appears in front of the Publisher, not inside him, and without that
  term the lead missed by a constant.
- The stolen shuriken is not thrown on the step of a volley: the two share one aim, and that aim is led
  (`Jam.Boss.StolenShurikenIsNotLed`).
- The bot needed more than the turn of its circle, and each addition has its test:
  `Jam.Bot.KeepsOffTheWall` (the closer the wall, the more it turns back into the open) and
  `Jam.Bot.KeepsOutOfABlast` (an explosive shuriken counts where it ends: at the wall or at the end of its life).

What the lost fights showed on the way (all 60 lost at first, the hero dead with the boss at 340 to 1460 health):

1. One shuriken per volley was led by accident, through the shared aim. Fixed in the brain.
2. The bot backed into the wall and lost its speed there. Sliding along the wall gave 52 of 60.
3. Almost all the remaining damage was the blast of the explosive shuriken that flew past the bot and went off on
   the wall behind it. No dodge helps there, only room: with it the bot wins every order.

Open: a hero at the wall is punished hard by the stolen shuriken (every third one explodes there). It reads as a
fair rule, "do not let him pin you", but a person has to be able to see it coming: the explosive shuriken needs
its own look in the hands of the boss (block D or the balance ticket T-031).
