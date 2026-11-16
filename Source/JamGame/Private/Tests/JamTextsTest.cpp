#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/ConfigCacheIni.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamTexts.h"

/**
 * T-040: the game is called RONIN: FINAL CUT (QUESTIONS.md Q1: NEON RONIN is taken on Steam). A bug looks like "the
 * win title still says NEON RONIN" or "the window title says JamGame".
 *
 * Run: Automation RunTests Jam.Hud
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTextsNameTest, "Jam.Hud.TheGameIsCalledRoninFinalCut", JamTest::Flags)

bool FJamTextsNameTest::RunTest(const FString& Parameters)
{
	const FString Name = TEXT("RONIN: FINAL CUT");
	TestEqual(TEXT("the win title is the name"), FString(JamTexts::WinTitle), Name);
	FString Window;
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectDisplayedTitle"), Window, GGameIni);
	TestTrue(TEXT("and so is the window title"), Window.Contains(Name));
	TestFalse(TEXT("the old name is gone"), FString(JamTexts::WinTitle).Contains(TEXT("NEON")) || Window.Contains(TEXT("NEON")));
	return true;
}

#endif
