# Polish plan, Sun 15 Nov night

The author, 02:40: polish the game as a partner, special attention to smooth movement, animations and their
transitions, the details (the katana in the hand in every pose, shurikens leaving the throwing hand), richer effects
for the full version so the fall to medium and greybox reads better, and a crack transition when a stage changes.
Answers to the grill (same night): the crack keeps the old look as a frozen frame; Niagara for the rich effects once
the Unreal MCP is on (code-driven particles otherwise); three tiers, and after cut 1 the hero goes back to the shape
ronin of before the models; time is not the constraint.

## Order

1. **T-065 Hands and blade.** The katana sits in the right hand in every pose (idle, run, strikes, parry, throws),
   measured from close frames; shurikens and balls leave from the throwing hand (the view starts them at the hand
   and joins the simulation's path within a few frames; the simulation keeps its body-centre spawn). The hero gets
   a throw gesture (a short arm flick from the slash clip, upper body only).
2. **T-066 Smooth movement and transitions.** Facing turns smoothly between steps where it snaps; the walk/run
   blend follows speed without popping; strike chains blend into each other; the Publisher's slide is reduced
   (stride matched to his speed where the clip allows); the dash gets a lean.
3. **T-067 Three tiers of the hero.** Cut 1 brings back the shape ronin of before the models (its walk, its blade,
   its T-pose) with the medium effects; cut 2 turns everything grey with bare effects; cut 3 the T-pose; cut 4 the
   bean. Tests on the tier rule.
4. **T-068 The crack.** When the layers change, the last frame of the old look is captured; the new look switches on
   under it; a glowing jagged vertical crack opens in the middle and runs to both edges over about 0.7 s while the
   fight holds. Rules (crack position, hold, timing) tested; the drawing in the HUD.
5. **T-069 Rich effects for the full tier.** Niagara systems if the MCP is up (hit sparks, slash ribbons, slam dust
   and debris, shuriken trails, money bursts, footstep dust, the Ultimate's line), otherwise a pooled code-driven
   particle system. Medium keeps today's effects, bare draws flat grey shapes without glow; his warnings stay
   readable in every tier.

Each step: tests first where there is a decision, its own commit, frames checked from the game camera, a DEVLOG
line. A packaged Shipping build at the end (pushed only on the author's word).
