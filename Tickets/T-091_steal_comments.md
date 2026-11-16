# T-091 The Publisher comments on the move he stole

**Block:** session 5, Sunday evening, build session-5r · **Status:** done

## Goal

The author: after a cut, once the crack has finished and both lines of the cut's card have played, the Publisher says
one line about the move he stole that time (the Dash's gets Ronin's answer). No overlap with any line, barks wait; once
per steal; the Katana has no line. Voiced with ElevenLabs like the approved lines, in VO_LINES.md, with subtitles.

## Decision

- Five lines spoken by `eleven_audio.py speak` with the saved Publisher and Ronin voices (`eleven_vo_lines.json`),
  levels within a dB of the approved lines, imported to `/Game/Audio/Voice/vo_steal_*`; subtitles in `JamTexts`.
- `JamVoiceRules::CommentsOf`: a StealResolved to the Publisher gives the move's comment; the Katana none, so the last
  cut (the Katana and the one move left) comments on the move.
- `JamVoiceRules::Release`: the comment is held while the crack is open, then queued after the card's lines still being
  said. `MayBark` is false while a comment is held, and the bark clock restarts after it.
- The cue names are the author's: three Publisher lines do not end in `_pub`, so the voice test's file-name check now
  checks only what `SpeakerOf` reads (`_ron` is the hero).

## Test seam

`Jam.Voice.ThePublisherCommentsOnTheMoveHeStole`, `Jam.Voice.TheCommentWaitsForTheCrackAndTheCard` (red first). Not
heard in a game window.
