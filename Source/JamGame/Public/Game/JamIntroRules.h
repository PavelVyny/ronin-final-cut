#pragma once

#include "CoreMinimal.h"

/**
 * The intro before the first fight (T-030): the Publisher shows off (his Kung_Fu_Punch clip) under a title card while
 * the fight waits, then the camera goes over to the hero. Skippable; once per launch, not on a retry.
 * Tests: Jam.Intro.*
 */
namespace JamIntroRules
{
/**
 * Whether the intro is over Age seconds in: it ran its Length, or a skip came after SkipAfter (a click still held
 * from starting the game does not skip it).
 */
JAMGAME_API bool IsOver(float Age, float Length, bool bSkip, float SkipAfter);

/** How visible the title card is: it fades in over Fade, stays, and fades out over the last Fade of Length. */
JAMGAME_API float TitleAlpha(float Age, float Length, float Fade);

/** How far the camera has gone from the Publisher to the hero: 0 until the last Return seconds, then eased to 1. */
JAMGAME_API float CameraToHero(float Age, float Length, float Return);

/** T-083: how much of the Publisher's health bar shows SinceFight seconds after the intro ended (negative: still in
 * the intro): none through the show, fading in over Fade once the fight starts. */
JAMGAME_API float BossBarAlpha(float SinceFight, float Fade);

/** T-089: when the hero starts his draw, Age seconds in: as the camera starts over to him, Return before the show ends. */
JAMGAME_API float HeroDrawStart(float Length, float Return);

/** T-089: how long the whole intro lasts: the Publisher's show, or until the hero's Draw seconds end if that is later. */
JAMGAME_API float FullLength(float Length, float Return, float Draw);

/**
 * T-094: whether the full theme plays, or the soft ambient: the theme over the menu and the fight (from a skip of the
 * intro on, bIntroDone), the ambient under the comic and the screens before the intro. The intro has IntroTheme.
 */
JAMGAME_API bool IsThemeUp(bool bMenu, bool bComic, bool bIntroDone);

/**
 * The theme's share Age seconds into the Publisher's show of Length: rising from silence under his kung fu and his
 * line, full exactly as the camera starts over to the hero, Return seconds before the show ends.
 */
JAMGAME_API float IntroTheme(float Age, float Length, float Return);

/** The theme's share of the music, 0 (all ambient) to 1, moved Dt seconds toward bUp over a crossfade of Seconds. */
JAMGAME_API float Crossfade(float Theme, bool bUp, float Dt, float Seconds);
} // namespace JamIntroRules
