The author played build session-5m: "it plays great, it is exactly what the game was meant to be". Four small fixes. First, you may push what is packaged now to itch.

0. Push now. Build `Saved/Packaged/Pavyny_RoninFinalCut_Session5m.zip` (Sun 04:04) is approved: push it to itch as `session-5m` with butler, the same way as before. Do not wait for the fixes below. After the fixes, package and push a new build too.

1. His stolen shurikens come too fast. When the Publisher has stolen the shuriken he throws them one after another and they are almost impossible to dodge; the author restarted about 5 times because of it. Make the time between his throws 30% longer. This applies to the boss only: the hero's `ShurikenCooldown=0.4` stays. `BossStolenCooldown` scales every stolen move, so do not change it; add a shuriken-only key in `[JamBalance]` (for example `BossShurikenCooldownScale=1.3`) or whatever the code really uses for his throw spacing (check whether his combo or AI fires faster than the cooldown). Test first, red then green.

2. The ultimate charge and the dash look low-poly in the full-art tier. At the moment both show only a plain red bar under the hero. In the full-art tier (3D models and Niagara):
   - remove the bar for the ultimate and for the dash;
   - the ultimate charge becomes a Niagara effect on the hero that grows as the charge builds and clearly reads as "ready" when full;
   - the dash already reads well through its afterimages, so it only loses the bar (if its cooldown needs a cue, keep it subtle and on the hero).
   These effects degrade with the look, like everything else: the bars come back two tiers down (greybox). One tier down (shape ronin) gets a simpler version, your call. Put the tier choice in the pure rules with a test.

3. The degradation crack is too fast. Right now the screen splits in two and slides apart before you can even see what happened. Make it slow enough to feel: about 2 to 3 times longer, and tune it by eye (the author has to see clearly which half went where). The split line must not be straight: make it a jagged crack, a fault line with a random-looking path across the screen, glowing neon along the edges. The fight keeps holding while it plays.

4. When done: run the tests, record a short clip showing the stolen shurikens, the ultimate charge in the full-art tier and one crack, package Shipping, push to itch, and update DEVLOG and the ticket index. Ask before anything you are unsure about, using interactive questions.
