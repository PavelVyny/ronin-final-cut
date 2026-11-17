# How the idea came

> Written on 2026-11-17, after the jam, from the log of the planning session. That session is not published in
> `_AgentLogs/`: the author pasted API keys into its chat, so the raw log stays local. The quotes below are the
> author's messages from it, translated from Russian. Times are UTC.

## The planning session (11 Nov 22:47 to 12 Nov 11:32)

The theme, Director's Cut, was announced on 11 November. That night the author opened one Claude Code session with
the rules, the kickoff stream and the itch page, and asked for three steps: a rules brief (`JAM_BRIEF.md`), a grill of
the team, then a plan. The instruction for the grill: "Ask questions one at a time, wait for the answer, push on the
weak spots. Do not agree out of politeness: if the idea is too big for 4 days or the theme is only there for show, say
so."

The brief already set the bar for the concept: "this is my long-held idea" is the reading most teams will take, and a
judge cannot see it in the game. Stronger are concepts where editing, cut content or the "final version" is the
mechanic.

## Four concepts

In the second round of the grill the agent offered four concepts:

- **STUDIO NOTES** (recommended): a top-down arena; after each wave the studio sends a note ("Too long. CUT one of
  three.") and the player cuts a feature that really disappears; the finale restores the player's picks.
- **TAKE TWO**: a staging puzzle; the player sets marks and cues for a fight scene, shouts CUT and reshoots until the
  studio's demands are met, then watches the film.
- **FINAL CUT**: a timeline puzzle where what you cut "did not happen". The freshest, but likely clever and not fun.
- **CUT CONTENT**: a platformer as a cut character in a greybox level. Cheap, and the obvious take.

The author: "studio notes is good but I don't fully see yet how it could work, take two also sounds very interesting".
They chose STUDIO NOTES and asked to grill the details.

## The author's turn: the game cuts itself down to a bean

Asked what happens when the player cuts a feature, the author answered with their own design:

> "What if the game itself decides what to cut? We control the difficulty curve, which rises as the strongest mechanics
> are cut (but the player gets better each round and makes up for the losses with skill, and has to adapt every
> round), with no rewards for pretty eyes: the reward is beating obstacles of rising difficulty itself. And you can see
> the game getting closer to the budget (the final budget is reached when almost no mechanics are left and the player
> controls a bean shooting primitive balls at enemy beans: the joke is that they cut so much to fit the budget that only
> a PoC is left). Or is that terrible game design?"

The agent's answer: "Your idea is not terrible, it is better than mine for production." It kept one objection: if
only the game cuts, the theme's verb leaves the player, so the player still picks one of two mechanics to lose, while
the cosmetics are cut on a script.

The same round settled the rest: the finale is a fight with the producer ("Boss fight with the producer"), and the
fiction is "a game about a game, but not grey boxes: we pick a style and a setting, greyboxes only when the visuals are
cut". The setting became a neon samurai, the inner game got the working title NEON RONIN, and its title on a win gains
": DIRECTOR'S CUT", on a loss ": PROOF OF CONCEPT".

## Plan v1 (before the start)

The first `JAM_PLAN.md`, committed before the jam window in `pre-jam: tooling and plan` (12 Nov 01:49), was STUDIO
NOTES as a 7-minute run: a full game with robot enemies, a note every 50 seconds forcing one of two mechanics out,
the studio stripping effects, textures, animations and music layers on its own, a bean against beans by minute four,
then a rebellion where everything returns, and a boss, the Publisher, who strips the game again before the final blow.
Five mechanics, four choices, two robot types with ragdolls, and the boss as the last thing to cut if time ran short.

This repository is a single-commit export, so plan v1 itself is not in it; it stays in the author's working
repository, and this section is its summary.

## The morning simplification (12 Nov 10:57, inside the jam window)

Before any game code was written, the author cut the plan down:

> "I'd like to simplify the idea and do one more grill round so we're on the same page. To start with, there will be
> only one level, no waves at all. There will be one enemy, the publisher himself. [...] He shows up and says you are
> making the game too expensive and too slow, time to do something about it. The game itself and the base concept is a
> boss fight right away, with a boss who can steal and take your mechanics."

The agent agreed and pushed on three points, which shaped the final game:

1. **Keep the choice.** If the boss just takes mechanics on a script, the theme shrinks to a title card. So the
   Publisher grabs two mechanics and the player decides which one to give up.
2. **Stolen, not just cut.** A mechanic is a component with an owner, so a theft is a change of owner: take the katana
   and he slashes you with it. The choice becomes double: what you lose and what you will be hit with.
3. **No backup ending any more.** The whole game now rests on the feel of one fight, so the first thing built was the
   fight on beans, with a bot player for balance.

That became `JAM_PLAN.md` v2 and the first jam session started from it at 11:32. Later in the jam the team shrank
to one person, the game got its final name RONIN: FINAL CUT, and the tiers became full art, shape ronin, greybox and
the bean; the dev log (`DEVLOG.md`) has the rest.
