#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/JamDisplayRules.h"
#include "Misc/ConfigCacheIni.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamMenuRules.h"

/**
 * T-102: the picture a run starts with. A bug here looks in game like "the first launch is blurry and Fullscreen is
 * sharper": a borderless window smaller than the desktop stretched over it, or the scene rendered below the window.
 *
 * Run: Automation RunTests Jam.Display
 */
namespace
{
// The author's screen: 3440x1440 at 125% in Windows, which a game without DPI awareness is told is 2752x1152.
const FIntPoint Desktop(3440, 1440);

FJamDisplay Display(EJamWindowMode Mode, const FIntPoint& Resolution, float RenderScale)
{
	FJamDisplay Out;
	Out.Mode = Mode;
	Out.Resolution = Resolution;
	Out.RenderScale = RenderScale;
	return Out;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDisplayFirstTest, "Jam.Display.AFirstLaunchIsBorderlessAtTheDesktop", JamTest::Flags)

bool FJamDisplayFirstTest::RunTest(const FString& Parameters)
{
	// The engine's first launch: a 1280x720 window, the scene at its own default (0).
	const FJamDisplay First = JamDisplayRules::Settle(Display(EJamWindowMode::Windowed, FIntPoint(1280, 720), 0.f), false, Desktop);
	TestTrue(TEXT("borderless"), First.Mode == EJamWindowMode::Borderless);
	TestEqual(TEXT("at the desktop's size"), First.Resolution, Desktop);
	TestEqual(TEXT("with the scene at full size"), First.RenderScale, JamDisplayRules::FullRenderScale);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDisplaySavedTest, "Jam.Display.NoSavedSettingRendersSmaller", JamTest::Flags)

bool FJamDisplaySavedTest::RunTest(const FString& Parameters)
{
	// The author's saved file: borderless at 2752x1152, 80% of the desktop.
	const FJamDisplay Borderless = JamDisplayRules::Settle(Display(EJamWindowMode::Borderless, FIntPoint(2752, 1152), 0.f), true, Desktop);
	TestTrue(TEXT("a borderless window stays borderless"), Borderless.Mode == EJamWindowMode::Borderless);
	TestEqual(TEXT("and covers the desktop"), Borderless.Resolution, Desktop);
	TestEqual(TEXT("the engine's default scale becomes full"), Borderless.RenderScale, JamDisplayRules::FullRenderScale);
	for (const float Picked : { 50.f, 71.f, 87.f })
	{
		TestEqual(TEXT("so does a quality's lower scale"),
			JamDisplayRules::Settle(Display(EJamWindowMode::Fullscreen, Desktop, Picked), true, Desktop).RenderScale,
			JamDisplayRules::FullRenderScale);
	}
	const FJamDisplay Full = JamDisplayRules::Settle(Display(EJamWindowMode::Fullscreen, FIntPoint(2560, 1080), 100.f), true, Desktop);
	TestTrue(TEXT("fullscreen keeps its picked size"), Full.Mode == EJamWindowMode::Fullscreen && Full.Resolution == FIntPoint(2560, 1080));
	const FJamDisplay Window = JamDisplayRules::Settle(Display(EJamWindowMode::Windowed, FIntPoint(5120, 2160), 100.f), true, Desktop);
	TestEqual(TEXT("a window too big for the desktop fits it"), Window.Resolution, JamMenuRules::FitWindow(FIntPoint(5120, 2160), Desktop));
	const FJamDisplay Unknown =
		JamDisplayRules::Settle(Display(EJamWindowMode::Borderless, FIntPoint(2752, 1152), 100.f), true, FIntPoint::ZeroValue);
	TestEqual(TEXT("no desktop known: the saved size is kept"), Unknown.Resolution, FIntPoint(2752, 1152));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDisplayDpiTest, "Jam.Display.TheGameSeesTheRealDesktop", JamTest::Flags)

bool FJamDisplayDpiTest::RunTest(const FString& Parameters)
{
	// Without DPI awareness Windows tells the game a 125% desktop is 2752x1152 and stretches its picture to 3440x1440.
	bool bAllowed = false;
	GConfig->GetBool(TEXT("/Script/Engine.UserInterfaceSettings"), TEXT("bAllowHighDPIInGameMode"), bAllowed, GEngineIni);
	TestTrue(TEXT("the game window is DPI aware"), bAllowed);
	return true;
}

#endif
