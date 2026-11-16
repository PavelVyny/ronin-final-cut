# T-093 The opening comic: the call that starts the fight

**Block:** session 5, Sunday evening · **Status:** done, not yet seen in a game window

## Goal

The author: a short skippable comic, 10 to 20 seconds, between NEW GAME and the fight. Pictures with comic-style camera
work (zooms and pans), only the exposition and the conflict: the developer at his computer late at night, the
Publisher calls in and interrupts, says the game ships today with no time or budget, the developer says it is not
ready, the Publisher says he will come into the game and take what he likes. Picked: voiced lines, balloons drawn by
the game.

## Script (approved, with the author's "Enough!")

| Panel | Picture | Camera | Words |
|---|---|---|---|
| 1 | The dev's room at 3:47 AM, the game on his screen | push from the room to the screen | caption "3:47 AM. DAY 847. ALMOST THERE." |
| 2 | The Publisher's face fills the screen, an incoming call | punch-in | caption "INCOMING CALL: THE PUBLISHER", "BZZZT!" |
| 3 | The Publisher on the screen, the dev in front of it | pan from his face to the dev's | Publisher: "Great news, kid! We ship TODAY!" Dev: "Today?! It's not finished!" |
| 4 | The Publisher's gold-ringed fist reaching out of the screen | pull back from the fist | Publisher: "Enough! No time, no budget. I'll cut what we don't need... personally." "KRSSH!" |
| 5 | The dev's face, Ronin reflected in the screen | push to the reflection's eyes | Dev: "Then I'll fight for every pixel." |

## Decision

- Five pictures by fal GPT Image 2.6 (`Art/Comic/comic_jobs_*.json`): panel 1 first, then panels 2 to 5 as edits with
  panel 1 and the two character sheets as references, so the room and both faces stay on model. Imported to
  `/Game/UI/Comic` as UI textures without mips (`Tools/Editor/import_comic.py`); `/Game/UI` is always cooked.
- Four lines voiced with ElevenLabs on the saved voices (`vo_comic_*`), with the other lines in `VO_LINES.md`.
- `JamComicRules`: the panels, their time and lines, the camera's crop of each picture (the screen's shape, never past
  the picture's edge, smoothstep ease, holds still once the move is done), the slide-in, the balloon's word wrap and
  place. A panel waits for its lines to end. 18.5 s of pictures, plus the voice.
- The HUD draws it on the canvas: the panel slides in from the right over the last one, then the caption box, the
  sound word, the balloon of the line being said and "Any key to skip". A click, Space, Enter or Esc skips it, not
  in its first 0.3 s, so the Enter that chose NEW GAME does not; a skip hushes the voice. Then the intro runs.

## Test seam

`Jam.Comic.*` (five tests, red first). The look of the pictures, the balloons and the moves is not checked by eye.
