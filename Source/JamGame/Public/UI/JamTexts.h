#pragma once

#include "CoreMinimal.h"

/**
 * Every line of text the player reads on the prototype HUD, in one place: the voice of the game is edited here and
 * nowhere else. Names of the mechanics live with the mechanics, in JamLoadoutRules::Name.
 */
namespace JamTexts
{
// The steal card.
// T-096: the HOW TO PLAY screen; its labels come from JamHowToRules::Callouts, with the keys.
inline const TCHAR* const HowToTitle = TEXT("HOW TO PLAY");
inline const TCHAR* const HowToTagline = TEXT("Beat the Publisher before he cuts your game to the bone.");
inline const TCHAR* const HowToPress = TEXT("PRESS ANY KEY");
inline const TCHAR* const HowToStartsIn = TEXT("THE FIGHT STARTS IN %d");
inline const TCHAR* const HowToWaits = TEXT("THE FIGHT WAITS");
inline const TCHAR* const StealTitle = TEXT("THE PUBLISHER HAS NOTES");
inline const TCHAR* const StealAsk = TEXT("Two features are on his desk. Give one away: he will use it against you.");
inline const TCHAR* const StealLeftKey = TEXT("[A]");
inline const TCHAR* const StealRightKey = TEXT("[D]");
inline const TCHAR* const StealDefault = TEXT("No answer: he takes the left one.");
inline const TCHAR* const StealAllTitle = TEXT("FINAL NOTE: CUT THE REST");
inline const TCHAR* const StealAllAsk = TEXT("He takes what you have left. You keep the ball.");
inline const TCHAR* const StealAllMark = TEXT("CUT");
inline const TCHAR* const StealAllKey = TEXT("[A] hand it over");

// The stun before the final blow.
inline const TCHAR* const ParryPrompt = TEXT("LMB  PARRY: SEND IT BACK");
inline const TCHAR* const UltimateBanner = TEXT("E   ULTIMATE READY");
inline const TCHAR* const UltimatePrompt = TEXT("E  ULTIMATE: CUT ACROSS THE ARENA");
inline const TCHAR* const UltimateBarLabel = TEXT("ULTIMATE  E");
inline const TCHAR* const StunTitle = TEXT("HE IS OUT OF NOTES");
inline const TCHAR* const StunAsk = TEXT("Everything is yours again. E: the Ultimate lands the final blow.");

// The intro before the first fight (T-030).
inline const TCHAR* const IntroTitle = TEXT("THE PUBLISHER");
inline const TCHAR* const IntroLine = TEXT("He has notes on your game. He will take what he likes.");
inline const TCHAR* const IntroSkip = TEXT("LMB / SPACE  skip");

// The endings.
inline const TCHAR* const WinTitle = TEXT("RONIN: FINAL CUT");
inline const TCHAR* const WinHint = TEXT("The Publisher is out of the building.   R: play again");
inline const TCHAR* const LossTitle = TEXT("SHIPPED AS PROOF OF CONCEPT");
inline const TCHAR* const LossHint = TEXT("The Publisher shipped it without you.   R: retry from the last cut");

// The subtitles of the voice-over, in the order of EJamLine (Audio/Eleven/Voice/VO_LINES.md), and who says them.
inline const TCHAR* const VoiceLines[] = {
	TEXT("Ooh, shiny! Love the passion, kid. Now let's trim the fat so we can actually ship this thing."),
	TEXT("This is my vision, my game! And I will fight for it."),
	TEXT("Focus group says the art is distracting. Minimalism is trending anyway!"),
	TEXT("Good thing the hitboxes still work."),
	TEXT("Neon? In this economy? Grey-box is the new gritty!"),
	TEXT("It's a PC game, man."),
	TEXT("Animation is pure vanity. Call it... avant-garde!"),
	TEXT("You are joking, right? It is not what I wanted to create!"),
	TEXT("Pure mechanics! Zero bloat! Behold... the Minimum Viable Product!"),
	TEXT("You can't patch out muscle memory, fat guy."),
	TEXT("Wait, wait! Keep the neon! Keep the sword! Full creative control, I swear!"),
	TEXT("Patch notes: you're out!"),
	TEXT("My... bonus..."),
	TEXT("Per my last email!"),
	TEXT("Let's pivot!"),
	TEXT("Too niche!"),
	TEXT("Think of the shareholders!"),
	TEXT("Ranged damage. Finally, a feature I can monetise."),
	TEXT("Blocking? I've been blocking ideas for twenty years."),
	TEXT("Dash? It'll be available behind a paywall."),
	TEXT("It's a basic mechanic, bro."),
	TEXT("Your big finale is now my DLC."),
	TEXT("Great news, kid! We ship TODAY!"),
	TEXT("Today?! It's not finished!"),
	TEXT("Enough! No time, no budget. I'll cut what we don't need... personally."),
	TEXT("Then I'll fight for every pixel."),
	// The session-5s playtest: banter for the quiet moments of the fight, the Publisher first, Ronin answering.
	TEXT("Just put a \"Day One Patch\" on the roadmap!"),
	TEXT("No."),
	TEXT("Why are you dodging? Hit your KPIs!"),
	TEXT("I'm fixing your bugs."),
	TEXT("Hear me out... what if we add a battle pass?"),
	TEXT("Over my dead build."),
	TEXT("Have you considered... NFTs?"),
	TEXT("Don't finish that sentence."),
	TEXT("Needs more loot boxes! Players love surprises!"),
	TEXT("You will be surprised soon."),
	TEXT("Stop parrying! That's not in the design doc!"),
	TEXT("I rewrote it."),
	TEXT("You're making this way too hard!"),
	TEXT("Skill issue."),
	TEXT("You can't fight market trends!"),
	TEXT("Watch me."),
};
// T-093: the opening comic's caption box and sound word on each panel; empty for none.
inline const TCHAR* const ComicCaptions[] = {
	TEXT("3:47 AM. DAY 847. ALMOST THERE."),
	TEXT("INCOMING CALL: THE PUBLISHER"),
	TEXT(""),
	TEXT(""),
	TEXT(""),
};
inline const TCHAR* const ComicSounds[] = { TEXT(""), TEXT("BZZZT!"), TEXT(""), TEXT("KRSSH!"), TEXT("") };
inline const TCHAR* const ComicSkip = TEXT("Any key to skip");
// T-092: the main menu, its rows in the order of EJamMenuItem, and the names of the settings' values.
inline const TCHAR* const MenuTitle = TEXT("RONIN: FINAL CUT");
inline const TCHAR* const MenuItems[] = {
	TEXT("NEW GAME"),
	TEXT("SETTINGS"),
	TEXT("EXIT"),
	TEXT("GRAPHICS"),
	TEXT("WINDOW"),
	TEXT("RESOLUTION"),
	TEXT("MASTER VOLUME"),
	TEXT("MUSIC VOLUME"),
	TEXT("VOICE VOLUME"),
	TEXT("BACK"),
	TEXT("DIFFICULTY"),
	TEXT("NVIDIA DLSS"),
};
inline const TCHAR* const QualityNames[] = { TEXT("LOW"), TEXT("MEDIUM"), TEXT("HIGH"), TEXT("EPIC") };
inline const TCHAR* const WindowNames[] = { TEXT("FULLSCREEN"), TEXT("BORDERLESS"), TEXT("WINDOWED") };
inline const TCHAR* const DifficultyNames[] = { TEXT("EASY"), TEXT("NORMAL"), TEXT("HARD") };
inline const TCHAR* const DlssNames[] = { TEXT("OFF"), TEXT("QUALITY"), TEXT("BALANCED"), TEXT("PERFORMANCE") };
inline const TCHAR* const MenuHint = TEXT("Arrows or mouse to choose, Enter to pick, Esc to go back");
inline const TCHAR* const SettingsHint = TEXT("Graphics apply on BACK");

/** The label of menu row Index; empty past the end. */
inline const TCHAR* MenuItem(int32 Index)
{
	return Index >= 0 && Index < static_cast<int32>(UE_ARRAY_COUNT(MenuItems)) ? MenuItems[Index] : TEXT("");
}

inline const TCHAR* const SpeakerBoss = TEXT("PUBLISHER");
inline const TCHAR* const SpeakerHero = TEXT("RONIN");

/** The subtitle of line number Index; empty past the end. */
inline const TCHAR* VoiceLine(int32 Index)
{
	return Index >= 0 && Index < static_cast<int32>(UE_ARRAY_COUNT(VoiceLines)) ? VoiceLines[Index] : TEXT("");
}

inline const TCHAR* const DebugHint = TEXT("Debug: F5 skip to the next cut   F6 restart the fight");

/** The Publisher's name on the neon bar's tab (T-104). */
inline const TCHAR* const BossName = TEXT("THE PUBLISHER");

inline FString BossHealth(int32 Now, int32 Max)
{
	return FString::Printf(TEXT("THE PUBLISHER  %d / %d"), Now, Max);
}

inline FString HeroHealth(int32 Now)
{
	return FString::Printf(TEXT("RONIN  %d"), Now);
}

/** What a button is left with after a steal: the weak ball on the attack buttons, nothing on the dash. */
inline FString LoadoutStolen(bool bBall, const TCHAR* Mechanic)
{
	return FString::Printf(TEXT("%s    (%s is the Publisher's now)"), bBall ? TEXT("BALL") : TEXT("nothing"), Mechanic);
}
/** The Ultimate's row: how full the meter is, or that it is ready. */
inline FString UltimateMeter(const TCHAR* Mechanic, float Meter)
{
	return Meter >= 1.f ? FString::Printf(TEXT("%s  READY"), Mechanic)
						: FString::Printf(TEXT("%s  %d%%"), Mechanic, FMath::FloorToInt32(Meter * 100.f));
}
} // namespace JamTexts
