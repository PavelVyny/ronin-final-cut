#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamHudScaleRules.h"

/**
 * T-099: the author saw soft, blurry text. The HUD now grows with the screen and its text is rasterised at the size shown.
 * A bug here looks in game like tiny text at 4K, a huge one at 720p, or text that is soft again.
 *
 * Run: Automation RunTests Jam.Hud
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudScaleTest, "Jam.Hud.TheHudGrowsWithTheScreen", JamTest::Flags)

bool FJamHudScaleTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("1080 lines: as laid out"), JamHudScaleRules::UiScale(1080.f), 1.f);
	TestEqual(TEXT("4K: twice as big"), JamHudScaleRules::UiScale(2160.f), 2.f);
	TestTrue(TEXT("an ultrawide 1440: by its height, not its width"), FMath::IsNearlyEqual(JamHudScaleRules::UiScale(1440.f), 4.f / 3.f));
	TestTrue(TEXT("720p: smaller"), FMath::IsNearlyEqual(JamHudScaleRules::UiScale(720.f), 2.f / 3.f));
	TestEqual(TEXT("no screen yet: as laid out"), JamHudScaleRules::UiScale(0.f), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudFontTest, "Jam.Hud.TextIsRasterisedAtItsShownSize", JamTest::Flags)

bool FJamHudFontTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("a 10 font at 1.4 on 1080: 14"), JamHudScaleRules::FontSize(10.f, 1.4f, 1.f), 14.f);
	TestEqual(TEXT("the same at 4K: 28"), JamHudScaleRules::FontSize(10.f, 1.4f, 2.f), 28.f);
	TestEqual(TEXT("a vanishing card is still a font"), JamHudScaleRules::FontSize(10.f, 0.01f, 1.f), 1.f);
	return true;
}

#endif
