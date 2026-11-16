#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamDifficultyRules.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamMenuRules.h"
#include "UI/JamTexts.h"

/**
 * T-092: the main menu before the intro: NEW GAME, SETTINGS, EXIT, and the settings page (graphics quality, window mode,
 * resolution, three volumes). A bug looks in game like "Enter starts the wrong thing", "the arrows run off the list",
 * "a setting goes past its last value" or "the mouse clicks a row it is not over".
 *
 * Run: Automation RunTests Jam.Menu
 */
namespace
{
FJamMenu MenuOn(EJamMenuPage Page, EJamMenuItem Item)
{
	FJamMenu Menu;
	Menu.Page = Page;
	Menu.Selected = JamMenuRules::ItemsOf(Page).IndexOfByKey(Item);
	Menu.Settings.ResolutionCount = 3;
	return Menu;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuMainTest, "Jam.Menu.TheMainPageStartsSetsUpOrExits", JamTest::Flags)

bool FJamMenuMainTest::RunTest(const FString& Parameters)
{
	// T-101: the author put DIFFICULTY under NEW GAME and moved SETTINGS a row down.
	TestTrue(TEXT("new game, difficulty, settings, exit"),
		JamMenuRules::ItemsOf(EJamMenuPage::Main) ==
			TArray<EJamMenuItem>({ EJamMenuItem::NewGame, EJamMenuItem::Difficulty, EJamMenuItem::Settings, EJamMenuItem::Exit }));
	FJamMenu Menu;
	TestTrue(TEXT("Enter on the first row starts the game"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::NewGame);
	JamMenuRules::Press(Menu, EJamMenuKey::Up);
	TestEqual(TEXT("up from the top wraps to EXIT"), Menu.Selected, 3);
	TestTrue(TEXT("Enter on EXIT quits"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::Exit);
	JamMenuRules::Press(Menu, EJamMenuKey::Down);
	TestEqual(TEXT("down from the bottom wraps to the top"), Menu.Selected, 0);
	TestTrue(TEXT("Escape on the main page does nothing"), JamMenuRules::Press(Menu, EJamMenuKey::Back) == EJamMenuAction::None);
	TestTrue(TEXT("nor do the side arrows"), JamMenuRules::Press(Menu, EJamMenuKey::Right) == EJamMenuAction::None);
	TestTrue(TEXT("still on the main page"), Menu.Page == EJamMenuPage::Main);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuPagesTest, "Jam.Menu.SettingsOpenAndApplyOnTheWayBack", JamTest::Flags)

bool FJamMenuPagesTest::RunTest(const FString& Parameters)
{
	FJamMenu Menu = MenuOn(EJamMenuPage::Main, EJamMenuItem::Settings);
	TestTrue(TEXT("SETTINGS opens its page"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::None);
	TestTrue(TEXT("the settings page"), Menu.Page == EJamMenuPage::Settings);
	TestEqual(TEXT("on its first row"), Menu.Selected, 0);
	TestTrue(TEXT("its rows"),
		JamMenuRules::ItemsOf(EJamMenuPage::Settings) ==
			TArray<EJamMenuItem>({ EJamMenuItem::Quality, EJamMenuItem::Window, EJamMenuItem::Resolution, EJamMenuItem::Dlss,
				EJamMenuItem::Master, EJamMenuItem::Music, EJamMenuItem::Voice, EJamMenuItem::Back }));
	TestTrue(TEXT("Escape goes back and applies the graphics"), JamMenuRules::Press(Menu, EJamMenuKey::Back) == EJamMenuAction::Graphics);
	TestTrue(TEXT("to the main page"), Menu.Page == EJamMenuPage::Main);
	TestEqual(TEXT("on SETTINGS"), Menu.Selected, JamMenuRules::ItemsOf(EJamMenuPage::Main).IndexOfByKey(EJamMenuItem::Settings));
	Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Back);
	TestTrue(TEXT("BACK does the same"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::Graphics);
	TestTrue(TEXT("back on the main page"), Menu.Page == EJamMenuPage::Main);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuValuesTest, "Jam.Menu.SettingsStepThroughTheirValues", JamTest::Flags)

bool FJamMenuValuesTest::RunTest(const FString& Parameters)
{
	FJamMenu Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Quality);
	Menu.Settings.Quality = JamMenuRules::QualityCount - 1;
	TestTrue(TEXT("a graphics change waits for BACK"), JamMenuRules::Press(Menu, EJamMenuKey::Right) == EJamMenuAction::None);
	TestEqual(TEXT("quality wraps from EPIC to LOW"), Menu.Settings.Quality, 0);
	JamMenuRules::Press(Menu, EJamMenuKey::Left);
	TestEqual(TEXT("and back"), Menu.Settings.Quality, JamMenuRules::QualityCount - 1);
	// T-094: Enter opens a graphics setting's list instead of stepping it.
	TestTrue(TEXT("Enter opens the list"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::None && Menu.bListOpen);
	TestEqual(TEXT("on the value it has"), Menu.ListSelected, Menu.Settings.Quality);

	Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Window);
	Menu.Settings.Window = 0;
	JamMenuRules::Press(Menu, EJamMenuKey::Left);
	TestEqual(TEXT("the window mode wraps"), Menu.Settings.Window, JamMenuRules::WindowCount - 1);

	Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Resolution);
	Menu.Settings.Resolution = 2;
	JamMenuRules::Press(Menu, EJamMenuKey::Right);
	TestEqual(TEXT("the resolution wraps over the screen's list"), Menu.Settings.Resolution, 0);
	Menu.Settings.ResolutionCount = 0;
	Menu.Settings.Resolution = 0;
	JamMenuRules::Press(Menu, EJamMenuKey::Right);
	TestEqual(TEXT("no list, no change"), Menu.Settings.Resolution, 0);

	Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Music);
	Menu.Settings.Music = JamMenuRules::VolumeSteps;
	TestTrue(TEXT("full volume stays full"), JamMenuRules::Press(Menu, EJamMenuKey::Right) == EJamMenuAction::None);
	TestEqual(TEXT("no wrap to silence"), Menu.Settings.Music, JamMenuRules::VolumeSteps);
	TestTrue(TEXT("a volume change is heard at once"), JamMenuRules::Press(Menu, EJamMenuKey::Left) == EJamMenuAction::Volume);
	TestEqual(TEXT("one step down"), Menu.Settings.Music, JamMenuRules::VolumeSteps - 1);
	Menu.Settings.Music = 0;
	JamMenuRules::Press(Menu, EJamMenuKey::Left);
	TestEqual(TEXT("silence stays silent"), Menu.Settings.Music, 0);
	TestEqual(TEXT("the volume of a step"), JamMenuRules::Volume(5), 0.5f);
	TestEqual(TEXT("full"), JamMenuRules::Volume(JamMenuRules::VolumeSteps), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuMouseTest, "Jam.Menu.TheMousePicksTheRowUnderIt", JamTest::Flags)

bool FJamMenuMouseTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("over the first row"), JamMenuRules::RowAt(105.f, 100.f, 40.f, 3), 0);
	TestEqual(TEXT("over the last"), JamMenuRules::RowAt(219.f, 100.f, 40.f, 3), 2);
	TestEqual(TEXT("above the list"), JamMenuRules::RowAt(99.f, 100.f, 40.f, 3), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("below the list"), JamMenuRules::RowAt(220.f, 100.f, 40.f, 3), static_cast<int32>(INDEX_NONE));
	FJamMenu Menu;
	JamMenuRules::Point(Menu, 2);
	TestEqual(TEXT("hovering selects"), Menu.Selected, 2);
	JamMenuRules::Point(Menu, INDEX_NONE);
	TestEqual(TEXT("off the list keeps the selection"), Menu.Selected, 2);
	JamMenuRules::Point(Menu, 7);
	TestEqual(TEXT("a row the page has not"), Menu.Selected, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuTextsTest, "Jam.Menu.EveryRowHasItsWords", JamTest::Flags)

bool FJamMenuTextsTest::RunTest(const FString& Parameters)
{
	for (int32 Index = 0; Index < JamMenuRules::ItemCount; ++Index)
	{
		TestFalse(TEXT("a label"), FString(JamTexts::MenuItem(Index)).IsEmpty());
	}
	TestEqual(TEXT("one label per row"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::MenuItems)), JamMenuRules::ItemCount);
	TestEqual(TEXT("one name per quality"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::QualityNames)), JamMenuRules::QualityCount);
	TestEqual(TEXT("one name per window mode"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::WindowNames)), JamMenuRules::WindowCount);
	TestEqual(
		TEXT("one name per difficulty"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::DifficultyNames)), JamMenuRules::DifficultyCount);
	TestEqual(TEXT("as many as the fight has levels"), JamMenuRules::DifficultyCount, JamDifficultyRules::Count);
	return true;
}

namespace
{
const FVector2D FullHd(1920.0, 1080.0);

int32 RowOf(EJamMenuPage Page, EJamMenuItem Item)
{
	return JamMenuRules::ItemsOf(Page).IndexOfByKey(Item);
}

/** A point Along of the way across Box, at its middle height. */
FVector2D Across(const FBox2D& Box, double Along)
{
	return FVector2D(FMath::Lerp(Box.Min.X, Box.Max.X, Along), Box.GetCenter().Y);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuSliderTest, "Jam.Menu.TheDrawnSlidersTakeTheClicks", JamTest::Flags)

bool FJamMenuSliderTest::RunTest(const FString& Parameters)
{
	// T-094: the author clicked the drawn arrows and nothing happened; only the label took the click.
	FJamMenu Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Quality);
	FJamMenuLayout Layout = JamMenuRules::Layout(Menu, FullHd);
	TestEqual(TEXT("a band per row"), Layout.Rows.Num(), JamMenuRules::ItemsOf(EJamMenuPage::Settings).Num());
	if (!TestEqual(TEXT("a control per row"), Layout.Controls.Num(), Layout.Rows.Num()) || Layout.Rows.IsEmpty())
	{
		return false;
	}
	const int32 Master = RowOf(EJamMenuPage::Settings, EJamMenuItem::Master);
	const FBox2D Track = Layout.Controls[Master];
	TestTrue(TEXT("the slider has a track"), Track.bIsValid && Track.GetSize().X > 0.0);
	TestTrue(TEXT("a click 30% along it"), JamMenuRules::Click(Menu, Layout, Across(Track, 0.3)) == EJamMenuAction::Volume);
	TestEqual(TEXT("sets 30%"), Menu.Settings.Master, 3);
	TestEqual(TEXT("and selects the row"), Menu.Selected, Master);
	TestTrue(TEXT("the slider follows the held button"), Menu.bDragging);
	TestTrue(TEXT("dragged past its left end"), JamMenuRules::Drag(Menu, Layout, Across(Track, -0.5)) == EJamMenuAction::Volume);
	TestEqual(TEXT("silence"), Menu.Settings.Master, 0);
	JamMenuRules::Release(Menu);
	TestTrue(TEXT("let go, it stays"),
		JamMenuRules::Drag(Menu, Layout, Across(Track, 1.0)) == EJamMenuAction::None && Menu.Settings.Master == 0);
	const int32 Music = RowOf(EJamMenuPage::Settings, EJamMenuItem::Music);
	Menu.Settings.Music = 4;
	JamMenuRules::Click(Menu, Layout, Across(Layout.Controls[Music], 1.0));
	TestEqual(TEXT("its right end is full"), Menu.Settings.Music, JamMenuRules::VolumeSteps);
	JamMenuRules::Release(Menu);
	TestTrue(TEXT("a click right of the screen's sliders is none of them"),
		JamMenuRules::Click(Menu, Layout, FVector2D(FullHd.X - 2.0, Layout.Rows[Music].GetCenter().Y)) == EJamMenuAction::None);
	const int32 Back = RowOf(EJamMenuPage::Settings, EJamMenuItem::Back);
	TestFalse(TEXT("BACK has no control"), Layout.Controls[Back].bIsValid);
	TestTrue(
		TEXT("a click on BACK goes back"), JamMenuRules::Click(Menu, Layout, Layout.Rows[Back].GetCenter()) == EJamMenuAction::Graphics);
	TestTrue(TEXT("to the main page"), Menu.Page == EJamMenuPage::Main);
	Layout = JamMenuRules::Layout(Menu, FullHd);
	if (Layout.Rows.Num() != JamMenuRules::ItemsOf(EJamMenuPage::Main).Num())
	{
		return false;
	}
	TestTrue(TEXT("a click on NEW GAME starts"),
		JamMenuRules::Click(Menu, Layout, Layout.Rows[RowOf(EJamMenuPage::Main, EJamMenuItem::NewGame)].GetCenter()) ==
			EJamMenuAction::NewGame);
	TestTrue(TEXT("a click off the rows does nothing"), JamMenuRules::Click(Menu, Layout, FVector2D(5.0, 5.0)) == EJamMenuAction::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuListTest, "Jam.Menu.DropdownsOpenAndPick", JamTest::Flags)

bool FJamMenuListTest::RunTest(const FString& Parameters)
{
	// T-094: no more stepping through every resolution: a list opens on a click and a pick applies.
	FJamMenu Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Quality);
	Menu.Settings.ResolutionCount = 12;
	Menu.Settings.Resolution = 1;
	const int32 Resolution = RowOf(EJamMenuPage::Settings, EJamMenuItem::Resolution);
	FJamMenuLayout Layout = JamMenuRules::Layout(Menu, FullHd);
	if (!TestEqual(TEXT("a control per row"), Layout.Controls.Num(), JamMenuRules::ItemsOf(EJamMenuPage::Settings).Num()))
	{
		return false;
	}
	TestTrue(TEXT("no list while closed"), Layout.Options.IsEmpty());
	TestTrue(TEXT("a click on the box opens it"),
		JamMenuRules::Click(Menu, Layout, Layout.Controls[Resolution].GetCenter()) == EJamMenuAction::None);
	TestTrue(TEXT("open"), Menu.bListOpen && Menu.Selected == Resolution);
	TestEqual(TEXT("on the current value"), Menu.ListSelected, 1);
	Layout = JamMenuRules::Layout(Menu, FullHd);
	if (!TestEqual(TEXT("one option per resolution"), Layout.Options.Num(), 12))
	{
		return false;
	}
	JamMenuRules::Hover(Menu, Layout, Layout.Options[7].GetCenter());
	TestEqual(TEXT("the pointer marks an option"), Menu.ListSelected, 7);
	TestTrue(
		TEXT("a click picks it and applies"), JamMenuRules::Click(Menu, Layout, Layout.Options[5].GetCenter()) == EJamMenuAction::Graphics);
	TestTrue(TEXT("picked, closed"), Menu.Settings.Resolution == 5 && !Menu.bListOpen);
	Layout = JamMenuRules::Layout(Menu, FullHd);
	JamMenuRules::Click(Menu, Layout, Across(Layout.Rows[Resolution], 0.1));
	TestTrue(TEXT("the label opens it too"), Menu.bListOpen);
	Layout = JamMenuRules::Layout(Menu, FullHd);
	TestTrue(TEXT("a click elsewhere closes it"), JamMenuRules::Click(Menu, Layout, FVector2D(5.0, 5.0)) == EJamMenuAction::None);
	TestTrue(TEXT("unchanged"), !Menu.bListOpen && Menu.Settings.Resolution == 5);

	Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Window);
	Menu.Settings.Window = 0;
	JamMenuRules::Press(Menu, EJamMenuKey::Confirm);
	JamMenuRules::Press(Menu, EJamMenuKey::Down);
	JamMenuRules::Press(Menu, EJamMenuKey::Down);
	TestEqual(TEXT("the arrows walk the list"), Menu.ListSelected, 2);
	TestEqual(TEXT("not the rows"), Menu.Selected, RowOf(EJamMenuPage::Settings, EJamMenuItem::Window));
	TestTrue(TEXT("Enter picks and applies"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::Graphics);
	TestTrue(TEXT("picked, closed"), Menu.Settings.Window == 2 && !Menu.bListOpen);
	JamMenuRules::Press(Menu, EJamMenuKey::Confirm);
	JamMenuRules::Press(Menu, EJamMenuKey::Up);
	TestTrue(TEXT("Escape closes the list"), JamMenuRules::Press(Menu, EJamMenuKey::Back) == EJamMenuAction::None);
	TestTrue(
		TEXT("without a change, still on the page"), !Menu.bListOpen && Menu.Settings.Window == 2 && Menu.Page == EJamMenuPage::Settings);
	TestEqual(TEXT("four qualities"), JamMenuRules::OptionCount(Menu.Settings, EJamMenuItem::Quality), JamMenuRules::QualityCount);
	TestEqual(TEXT("a slider has no list"), JamMenuRules::OptionCount(Menu.Settings, EJamMenuItem::Voice), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFitTest, "Jam.Menu.TheLayoutFitsAnyScreen", JamTest::Flags)

bool FJamMenuFitTest::RunTest(const FString& Parameters)
{
	for (const FVector2D Screen : { FVector2D(1280.0, 720.0), FullHd, FVector2D(3440.0, 1440.0), FVector2D(1024.0, 768.0) })
	{
		const FBox2D OnScreen(FVector2D(0.0, 0.0), Screen);
		for (const EJamMenuPage Page : { EJamMenuPage::Main, EJamMenuPage::Settings })
		{
			FJamMenu Menu = MenuOn(Page, JamMenuRules::ItemsOf(Page)[0]);
			const FJamMenuLayout Layout = JamMenuRules::Layout(Menu, Screen);
			TestEqual(TEXT("a control per row"), Layout.Controls.Num(), Layout.Rows.Num());
			for (int32 Row = 0; Row < FMath::Min(Layout.Rows.Num(), Layout.Controls.Num()); ++Row)
			{
				TestTrue(TEXT("each row on the screen"), OnScreen.IsInside(Layout.Rows[Row]));
				TestTrue(TEXT("its control inside it"), !Layout.Controls[Row].bIsValid || Layout.Rows[Row].IsInside(Layout.Controls[Row]));
				TestTrue(TEXT("on the panel"), Page == EJamMenuPage::Main || Layout.Panel.IsInside(Layout.Rows[Row]));
			}
		}
		FJamMenu Menu = MenuOn(EJamMenuPage::Settings, EJamMenuItem::Resolution);
		Menu.Settings.ResolutionCount = 40;
		JamMenuRules::Press(Menu, EJamMenuKey::Confirm);
		const FJamMenuLayout Layout = JamMenuRules::Layout(Menu, Screen);
		TestEqual(TEXT("a long list"), Layout.Options.Num(), 40);
		for (int32 Option = 0; Option < Layout.Options.Num(); ++Option)
		{
			TestTrue(TEXT("every option on the screen"), OnScreen.IsInside(Layout.Options[Option]));
			TestTrue(TEXT("tall enough to hit"), Layout.Options[Option].GetSize().Y >= 0.012 * Screen.Y);
			TestTrue(TEXT("below the one before"), Option == 0 || Layout.Options[Option].Min.Y >= Layout.Options[Option - 1].Max.Y - 0.01);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFitWindowTest, "Jam.Menu.AWindowFitsTheDesktop", JamTest::Flags)

bool FJamMenuFitWindowTest::RunTest(const FString& Parameters)
{
	// The author's 5v run: the game opened windowed at 3840x2160 and only the window's top-left corner was on the screen.
	const FIntPoint Desktop(3440, 1440);
	TestEqual(TEXT("a small window stays as it is"), JamMenuRules::FitWindow(FIntPoint(1280, 720), Desktop), FIntPoint(1280, 720));
	const FIntPoint Big = JamMenuRules::FitWindow(FIntPoint(3840, 2160), Desktop);
	TestTrue(TEXT("a big one fits the desktop"), Big.X < Desktop.X && Big.Y < Desktop.Y);
	TestTrue(TEXT("with room for the title bar and the taskbar"), Big.Y <= Desktop.Y - 60);
	TestTrue(TEXT("its shape kept"), FMath::IsNearlyEqual(double(Big.X) / Big.Y, 16.0 / 9.0, 0.01));
	TestTrue(TEXT("and as big as it can be"), Big.Y >= Desktop.Y * 0.8);
	const FIntPoint Same = JamMenuRules::FitWindow(Desktop, Desktop);
	TestTrue(TEXT("the desktop's own size fits too"), Same.X < Desktop.X && Same.Y < Desktop.Y);
	TestEqual(TEXT("no desktop: left alone"), JamMenuRules::FitWindow(FIntPoint(3840, 2160), FIntPoint::ZeroValue), FIntPoint(3840, 2160));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuDifficultyTest, "Jam.Menu.TheDifficultyRowCyclesItsLevels", JamTest::Flags)

bool FJamMenuDifficultyTest::RunTest(const FString& Parameters)
{
	// T-101: DIFFICULTY  < NORMAL > under NEW GAME; the side arrows, Enter and the drawn arrows step it round.
	FJamMenu Menu;
	TestEqual(TEXT("NORMAL by default"), Menu.Settings.Difficulty, 1);
	TestEqual(TEXT("right under NEW GAME"), RowOf(EJamMenuPage::Main, EJamMenuItem::Difficulty), 1);
	TestTrue(TEXT("a selector"), JamMenuRules::IsSelector(EJamMenuItem::Difficulty) && !JamMenuRules::IsSelector(EJamMenuItem::NewGame));
	JamMenuRules::Press(Menu, EJamMenuKey::Down);
	TestTrue(TEXT("right steps it and asks to save it"), JamMenuRules::Press(Menu, EJamMenuKey::Right) == EJamMenuAction::Difficulty);
	TestEqual(TEXT("HARD"), Menu.Settings.Difficulty, 2);
	JamMenuRules::Press(Menu, EJamMenuKey::Right);
	TestEqual(TEXT("past HARD it comes round to EASY"), Menu.Settings.Difficulty, 0);
	TestTrue(TEXT("left steps back"), JamMenuRules::Press(Menu, EJamMenuKey::Left) == EJamMenuAction::Difficulty);
	TestEqual(TEXT("before EASY is HARD"), Menu.Settings.Difficulty, 2);
	TestTrue(TEXT("Enter steps it on"), JamMenuRules::Press(Menu, EJamMenuKey::Confirm) == EJamMenuAction::Difficulty);
	TestEqual(TEXT("to EASY"), Menu.Settings.Difficulty, 0);
	TestTrue(TEXT("still on the main page"), Menu.Page == EJamMenuPage::Main);

	const FJamMenuLayout Layout = JamMenuRules::Layout(Menu, FullHd);
	const int32 Row = RowOf(EJamMenuPage::Main, EJamMenuItem::Difficulty);
	if (!TestTrue(TEXT("the row has its arrows' box"), Layout.Controls.IsValidIndex(Row) && Layout.Controls[Row].bIsValid))
	{
		return false;
	}
	TestTrue(TEXT("the box is inside its row"), Layout.Rows[Row].IsInside(Layout.Controls[Row]));
	TestTrue(TEXT("a click on the right arrow"),
		JamMenuRules::Click(Menu, Layout, Across(Layout.Controls[Row], 0.95)) == EJamMenuAction::Difficulty);
	TestEqual(TEXT("steps it on"), Menu.Settings.Difficulty, 1);
	JamMenuRules::Click(Menu, Layout, Across(Layout.Controls[Row], 0.05));
	TestEqual(TEXT("one on the left steps it back"), Menu.Settings.Difficulty, 0);
	TestFalse(TEXT("NEW GAME has no box"), Layout.Controls[RowOf(EJamMenuPage::Main, EJamMenuItem::NewGame)].bIsValid);
	// The author's 6a look: the label at the left edge and a framed box at the right did not fit the centred main page.
	// The label now ends at the row's middle and the arrows start right after it, so the two read as one centred line.
	const FBox2D& Band = Layout.Rows[Row];
	const FBox2D& Arrows = Layout.Controls[Row];
	TestTrue(TEXT("the arrows start right of the row's middle"), Arrows.Min.X > Band.GetCenter().X);
	TestTrue(TEXT("and end well short of its right edge"), Arrows.Max.X <= Band.Min.X + 0.8 * Band.GetSize().X);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
