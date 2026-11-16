#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/JamIntroRules.h"
#include "Tests/JamTestHelpers.h"
#include "View/JamPoseRules.h"

/**
 * T-030: the intro before the first fight. A bug here looks in game like "the intro never ends", "the click that
 * started the game skipped it at once" or "the camera jumps to the hero".
 *
 * Run: Automation RunTests Jam.Intro
 */
namespace
{
constexpr float ShowLength = 7.3f;
constexpr float SkipGrace = 0.5f;
constexpr float TitleFade = 0.5f;
constexpr float ReturnTime = 0.8f;
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroEndTest, "Jam.Intro.EndsAfterItsShowOrOnASkip", JamTest::Flags)

bool FJamIntroEndTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("it runs at the start"), JamIntroRules::IsOver(0.f, ShowLength, false, SkipGrace));
	TestFalse(TEXT("and through the show"), JamIntroRules::IsOver(ShowLength - 0.01f, ShowLength, false, SkipGrace));
	TestTrue(TEXT("it ends with the show"), JamIntroRules::IsOver(ShowLength, ShowLength, false, SkipGrace));
	TestFalse(TEXT("a click at once does not skip it"), JamIntroRules::IsOver(0.1f, ShowLength, true, SkipGrace));
	TestTrue(TEXT("a click later does"), JamIntroRules::IsOver(SkipGrace, ShowLength, true, SkipGrace));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroTitleTest, "Jam.Intro.TheTitleFadesInAndOut", JamTest::Flags)

bool FJamIntroTitleTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("hidden at the start"), JamIntroRules::TitleAlpha(0.f, ShowLength, TitleFade), 0.f);
	TestEqual(TEXT("half way in"), JamIntroRules::TitleAlpha(TitleFade / 2.f, ShowLength, TitleFade), 0.5f, 1e-4f);
	TestEqual(TEXT("whole in the middle"), JamIntroRules::TitleAlpha(ShowLength / 2.f, ShowLength, TitleFade), 1.f);
	TestEqual(TEXT("half way out"), JamIntroRules::TitleAlpha(ShowLength - TitleFade / 2.f, ShowLength, TitleFade), 0.5f, 1e-4f);
	TestEqual(TEXT("gone at the end"), JamIntroRules::TitleAlpha(ShowLength + 1.f, ShowLength, TitleFade), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroCameraTest, "Jam.Intro.TheCameraGoesOverToTheHeroAtTheEnd", JamTest::Flags)

bool FJamIntroCameraTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("on the Publisher through the show"),
		JamIntroRules::CameraToHero(ShowLength - ReturnTime - 0.01f, ShowLength, ReturnTime), 0.f);
	const float Mid = JamIntroRules::CameraToHero(ShowLength - ReturnTime / 2.f, ShowLength, ReturnTime);
	TestTrue(TEXT("on its way half through the return"), Mid > 0.f && Mid < 1.f);
	TestEqual(TEXT("on the hero at the end"), JamIntroRules::CameraToHero(ShowLength, ShowLength, ReturnTime), 1.f);
	TestEqual(TEXT("and after it"), JamIntroRules::CameraToHero(ShowLength + 2.f, ShowLength, ReturnTime), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroBarTest, "Jam.Intro.TheBossBarFadesInWhenTheFightStarts", JamTest::Flags)

bool FJamIntroBarTest::RunTest(const FString& Parameters)
{
	// T-083, the author: the Publisher's HP bar must not show during the intro; it fades in when the fight starts.
	TestEqual(TEXT("hidden through the show"), JamIntroRules::BossBarAlpha(-3.f, 0.6f), 0.f);
	TestEqual(TEXT("still hidden the moment it ends"), JamIntroRules::BossBarAlpha(0.f, 0.6f), 0.f);
	const float Half = JamIntroRules::BossBarAlpha(0.3f, 0.6f);
	TestTrue(TEXT("fading in"), Half > 0.2f && Half < 0.8f);
	TestEqual(TEXT("whole after the fade"), JamIntroRules::BossBarAlpha(0.6f, 0.6f), 1.f);
	TestEqual(TEXT("and stays"), JamIntroRules::BossBarAlpha(60.f, 0.6f), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroDrawTest, "Jam.Intro.TheHeroDrawsAsTheCameraComesToHim", JamTest::Flags)

bool FJamIntroDrawTest::RunTest(const FString& Parameters)
{
	// T-089, the author: when the camera turns from the Publisher to the hero, the hero slowly draws his katana and
	// takes control only when the draw ends.
	constexpr float Draw = 1.5f;
	TestEqual(
		TEXT("he draws as the camera starts over to him"), JamIntroRules::HeroDrawStart(ShowLength, ReturnTime), ShowLength - ReturnTime);
	const float Full = JamIntroRules::FullLength(ShowLength, ReturnTime, Draw);
	TestEqual(TEXT("the intro lasts until his draw ends"), Full, ShowLength - ReturnTime + Draw);
	TestFalse(TEXT("no control while he draws"), JamIntroRules::IsOver(Full - 0.01f, Full, false, SkipGrace));
	TestTrue(TEXT("control when the draw ends"), JamIntroRules::IsOver(Full, Full, false, SkipGrace));
	TestEqual(TEXT("a draw shorter than the camera's way does not cut the show"), JamIntroRules::FullLength(ShowLength, ReturnTime, 0.2f),
		ShowLength);
	TestEqual(TEXT("no draw clip: the show alone"), JamIntroRules::FullLength(ShowLength, ReturnTime, 0.f), ShowLength);
	TestEqual(TEXT("a show shorter than the camera's way: he draws at once"), JamIntroRules::HeroDrawStart(0.5f, ReturnTime), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroTurnTest, "Jam.Intro.ThePublisherTurnsWithHisShowOff", JamTest::Flags)

bool FJamIntroTurnTest::RunTest(const FString& Parameters)
{
	// T-090, the author: his show-off ends looking away from the hero; the whole clip turns so he ends facing him.
	constexpr float Turn = -120.f;
	TestEqual(TEXT("the whole turn while the show-off plays"), JamPoseRules::ShowOffTurn(Turn, 1.f), Turn);
	TestEqual(TEXT("none without it"), JamPoseRules::ShowOffTurn(Turn, 0.f), 0.f);
	TestEqual(TEXT("eased out as the clip fades"), JamPoseRules::ShowOffTurn(Turn, 0.5f), 0.5f * Turn);
	TestEqual(TEXT("never past the turn"), JamPoseRules::ShowOffTurn(Turn, 1.5f), Turn);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroThemeRiseTest, "Jam.Intro.TheThemeRisesUnderHisShow", JamTest::Flags)

bool FJamIntroThemeRiseTest::RunTest(const FString& Parameters)
{
	// The author's play-through: the theme fades in quietly while he does his kung fu and talks, and is full exactly
	// as the camera goes over to the hero (Return seconds before his show of Length ends).
	const float Length = 4.f;
	const float Return = 0.8f;
	TestTrue(TEXT("silent as he starts"), JamIntroRules::IntroTheme(0.f, Length, Return) <= 0.01f);
	TestTrue(TEXT("quiet in the first part"), JamIntroRules::IntroTheme(0.5f, Length, Return) < 0.3f);
	const float Early = JamIntroRules::IntroTheme(1.f, Length, Return);
	TestTrue(TEXT("rising"), Early > 0.f && JamIntroRules::IntroTheme(2.f, Length, Return) > Early);
	TestTrue(TEXT("not full before the camera moves"), JamIntroRules::IntroTheme(Length - Return - 0.1f, Length, Return) < 1.f);
	TestEqual(TEXT("full as the camera moves"), JamIntroRules::IntroTheme(Length - Return, Length, Return), 1.f);
	TestEqual(TEXT("and after"), JamIntroRules::IntroTheme(Length, Length, Return), 1.f);
	TestEqual(TEXT("no show: full"), JamIntroRules::IntroTheme(0.f, 0.f, Return), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamIntroThemeTest, "Jam.Intro.TheAmbientGivesWayToTheTheme", JamTest::Flags)

bool FJamIntroThemeTest::RunTest(const FString& Parameters)
{
	// T-094: the comic played under the full fight music; it gets a soft ambient. The intro's rise is IntroTheme's.
	TestTrue(TEXT("the menu has the theme"), JamIntroRules::IsThemeUp(true, false, false));
	TestFalse(TEXT("the comic the ambient"), JamIntroRules::IsThemeUp(false, true, false));
	TestFalse(TEXT("so does the screen before the intro"), JamIntroRules::IsThemeUp(false, false, false));
	TestTrue(TEXT("the fight the theme, from the skip on too"), JamIntroRules::IsThemeUp(false, false, true));
	float Theme = 0.f;
	Theme = JamIntroRules::Crossfade(Theme, true, 1.f, 2.f);
	TestTrue(TEXT("halfway after half the crossfade"), FMath::IsNearlyEqual(Theme, 0.5f, 1e-4f));
	Theme = JamIntroRules::Crossfade(Theme, true, 5.f, 2.f);
	TestEqual(TEXT("then all theme"), Theme, 1.f);
	TestEqual(TEXT("down to the ambient"), JamIntroRules::Crossfade(1.f, false, 5.f, 2.f), 0.f);
	TestEqual(TEXT("no crossfade time switches at once"), JamIntroRules::Crossfade(0.f, true, 0.01f, 0.f), 1.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
