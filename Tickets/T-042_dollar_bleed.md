# T-042 The Publisher bleeds paper dollars

**Block:** session 5 · **Status:** done

## Goal

The author's ask (Sat 14 Nov): when hit, the Publisher bleeds paper dollars. Look picked in chat: spray and floor.

## Scope

- `JamFeedbackRules::BillCount`: one bill per 20 damage, at least 2 and at most 12 per hit, 60 on the final blow, 0
  for any other event or a hit on the hero. `BillPose`: pops up from chest height, flutters down for 1.2 s (rocking
  across the throw), lies on the floor 3 s, shrinks away in 0.5 s. Numbers in `[JamFeedback]`.
- `AJamMoneyView`: green neon stand-in cards from a pool, at most 200 alive (the oldest go first). The money is the
  Publisher's look: the hero's effects cut never hides it.

## Test seam

`Jam.Feedback.ThePublisherBleedsDollarsByTheDamage`, `Jam.Feedback.BillsFlutterDownAndFade`; red first on stubs (and
`Jam.Feedback.IniIsComplete` red on the missing lines).

## Done

156 `Jam.` tests green. Clip `Clips/t042_dollar_bleed.mp4` (30 s, bot). The final burst is not in the clip (the bot
needs about 150 s to win).
