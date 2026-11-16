#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamMenuRules.h"

/**
 * NVIDIA DLSS Super Resolution in SETTINGS: OFF, QUALITY, BALANCED, PERFORMANCE, QUALITY by default on an RTX card and
 * OFF, with no other choice, on any card DLSS does not run on. A bug looks in game like "the DLSS row offers modes on a
 * card without DLSS", "a fresh install starts without DLSS on an RTX card" or "picking a mode does not apply it".
 *
 * Run: Automation RunTests Jam.Dlss
 */
namespace
{
FJamMenu OnDlss(bool bSupported)
{
	FJamMenu Menu;
	Menu.Page = EJamMenuPage::Settings;
	Menu.Selected = JamMenuRules::ItemsOf(EJamMenuPage::Settings).IndexOfByKey(EJamMenuItem::Dlss);
	Menu.Settings.bDlssSupported = bSupported;
	return Menu;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDlssRowTest, "Jam.Dlss.TheSettingsPageHasADlssRowUnderResolution", JamTest::Flags)

bool FJamDlssRowTest::RunTest(const FString& Parameters)
{
	const TArray<EJamMenuItem> Items = JamMenuRules::ItemsOf(EJamMenuPage::Settings);
	TestEqual(
		TEXT("DLSS right under RESOLUTION"), Items.IndexOfByKey(EJamMenuItem::Dlss), Items.IndexOfByKey(EJamMenuItem::Resolution) + 1);
	TestTrue(TEXT("a dropdown"), JamMenuRules::IsList(EJamMenuItem::Dlss));
	FJamMenuSettings Settings;
	Settings.bDlssSupported = true;
	TestEqual(TEXT("four modes on RTX"), JamMenuRules::OptionCount(Settings, EJamMenuItem::Dlss), JamMenuRules::DlssCount);
	Settings.bDlssSupported = false;
	TestEqual(TEXT("only OFF elsewhere"), JamMenuRules::OptionCount(Settings, EJamMenuItem::Dlss), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDlssDefaultTest, "Jam.Dlss.QualityByDefaultOnRtxOffElsewhere", JamTest::Flags)

bool FJamDlssDefaultTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("never saved, RTX: QUALITY"), JamMenuRules::DlssFor(INDEX_NONE, true), 1);
	TestEqual(TEXT("saved OFF stays OFF"), JamMenuRules::DlssFor(0, true), 0);
	TestEqual(TEXT("saved PERFORMANCE stays"), JamMenuRules::DlssFor(3, true), 3);
	TestEqual(TEXT("a value past the list is clamped"), JamMenuRules::DlssFor(9, true), JamMenuRules::DlssCount - 1);
	TestEqual(TEXT("no DLSS: OFF whatever was saved"), JamMenuRules::DlssFor(2, false), 0);
	TestEqual(TEXT("no DLSS, never saved: OFF"), JamMenuRules::DlssFor(INDEX_NONE, false), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDlssPickTest, "Jam.Dlss.AModeStepsAndAPickApplies", JamTest::Flags)

bool FJamDlssPickTest::RunTest(const FString& Parameters)
{
	FJamMenu Menu = OnDlss(true);
	Menu.Settings.Dlss = JamMenuRules::DlssCount - 1;
	JamMenuRules::Press(Menu, EJamMenuKey::Right);
	TestEqual(TEXT("PERFORMANCE wraps to OFF"), Menu.Settings.Dlss, 0);
	TestTrue(TEXT("Enter opens the list"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::None && Menu.bListOpen);
	JamMenuRules::Press(Menu, EJamMenuKey::Down);
	TestTrue(TEXT("a pick applies at once"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::Graphics);
	TestEqual(TEXT("QUALITY picked"), Menu.Settings.Dlss, 1);

	Menu = OnDlss(false);
	JamMenuRules::Press(Menu, EJamMenuKey::Right);
	TestEqual(TEXT("no DLSS: the arrows keep OFF"), Menu.Settings.Dlss, 0);
	return true;
}

#endif
