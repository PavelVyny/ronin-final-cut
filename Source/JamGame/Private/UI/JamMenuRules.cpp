#include "UI/JamMenuRules.h"

namespace
{
// T-094: the page on the screen, as fractions of it: where the rows start, their height, the column's width; a control
// starts ControlFrom of the way across its row, ends ControlPadding of the column short of its right edge and keeps
// ControlInset of the row's height clear above and below.
constexpr double MainTop = 0.42;
constexpr double SettingsTop = 0.25;
constexpr double RowShare = 0.062;
constexpr double MainColumn = 0.3;
constexpr double SettingsColumn = 0.46;
constexpr double ControlFrom = 0.48;
constexpr double ControlPadding = 0.03;
constexpr double ControlInset = 0.18;
// T-101: the difficulty's arrows, from just right of the row's middle (its label ends just left of it) to this far
// across, so the label and the level read as one centred line like the main page's other rows.
constexpr double SelectorFrom = 0.53;
constexpr double SelectorTo = 0.77;
// The settings panel reaches this far round its rows, and further down for the hint under them.
constexpr double PanelPadding = 0.025;
constexpr double PanelHint = 0.07;
// An open list keeps this much of the screen's height clear above and below; its options are at most this share of a
// row tall, shorter when a long list would not fit.
constexpr double ListMargin = 0.03;
constexpr double OptionShare = 0.8;

bool IsValueRow(EJamMenuItem Item)
{
	return (Item >= EJamMenuItem::Quality && Item <= EJamMenuItem::Voice) || Item == EJamMenuItem::Dlss;
}

/** Steps Value by Step round Count values. */
int32 Cycle(int32 Value, int32 Step, int32 Count)
{
	return Count > 0 ? ((Value + Step) % Count + Count) % Count : Value;
}

/** The setting a value row shows and changes. */
int32& ValueOf(FJamMenuSettings& Settings, EJamMenuItem Item)
{
	switch (Item)
	{
		case EJamMenuItem::Quality:
			return Settings.Quality;
		case EJamMenuItem::Window:
			return Settings.Window;
		case EJamMenuItem::Resolution:
			return Settings.Resolution;
		case EJamMenuItem::Master:
			return Settings.Master;
		case EJamMenuItem::Music:
			return Settings.Music;
		case EJamMenuItem::Dlss:
			return Settings.Dlss;
		default:
			return Settings.Voice;
	}
}

/** Steps a value row; Volume when a volume changed, None otherwise (the graphics wait for BACK). */
EJamMenuAction StepValue(FJamMenuSettings& Settings, EJamMenuItem Item, int32 Step)
{
	int32& Value = ValueOf(Settings, Item);
	if (JamMenuRules::IsList(Item))
	{
		Value = Cycle(Value, Step, JamMenuRules::OptionCount(Settings, Item));
		return EJamMenuAction::None;
	}
	const int32 Was = Value;
	Value = FMath::Clamp(Was + Step, 0, JamMenuRules::VolumeSteps);
	return Value == Was ? EJamMenuAction::None : EJamMenuAction::Volume;
}

/** Steps the difficulty round its levels; the game saves it. */
EJamMenuAction StepDifficulty(FJamMenuSettings& Settings, int32 Step)
{
	Settings.Difficulty = Cycle(Settings.Difficulty, Step, JamMenuRules::DifficultyCount);
	return EJamMenuAction::Difficulty;
}

/** The side arrows: a value row steps its value, the difficulty its level; other rows ignore them. */
EJamMenuAction PressSide(FJamMenuSettings& Settings, EJamMenuItem Item, int32 Step)
{
	if (JamMenuRules::IsSelector(Item))
	{
		return StepDifficulty(Settings, Step);
	}
	return IsValueRow(Item) ? StepValue(Settings, Item, Step) : EJamMenuAction::None;
}

EJamMenuAction CloseSettings(FJamMenu& Menu)
{
	Menu.Page = EJamMenuPage::Main;
	Menu.Selected = JamMenuRules::ItemsOf(EJamMenuPage::Main).IndexOfByKey(EJamMenuItem::Settings);
	Menu.bListOpen = false;
	Menu.bDragging = false;
	return EJamMenuAction::Graphics;
}

/** Opens the selected row's list on the value it has; a list without options stays shut. */
void OpenList(FJamMenu& Menu, EJamMenuItem Item)
{
	const int32 Count = JamMenuRules::OptionCount(Menu.Settings, Item);
	Menu.bListOpen = Count > 0;
	Menu.ListSelected = FMath::Clamp(ValueOf(Menu.Settings, Item), 0, FMath::Max(Count - 1, 0));
}

/** Takes option Option of the open list and closes it: a pick applies at once. */
EJamMenuAction Pick(FJamMenu& Menu, int32 Option)
{
	const EJamMenuItem Item = JamMenuRules::ItemsOf(Menu.Page)[Menu.Selected];
	ValueOf(Menu.Settings, Item) = Option;
	Menu.bListOpen = false;
	return EJamMenuAction::Graphics;
}

/** The slider's value under X on its Track; Volume when it changed. */
EJamMenuAction Slide(FJamMenuSettings& Settings, EJamMenuItem Item, const FBox2D& Track, double X)
{
	const double Width = Track.GetSize().X;
	const double Along = Width > 0.0 ? FMath::Clamp((X - Track.Min.X) / Width, 0.0, 1.0) : 0.0;
	int32& Value = ValueOf(Settings, Item);
	const int32 Was = Value;
	Value = FMath::RoundToInt32(Along * JamMenuRules::VolumeSteps);
	return Value == Was ? EJamMenuAction::None : EJamMenuAction::Volume;
}

/** The keys while a list is open: the arrows walk its options, Enter picks one, Escape shuts it. */
EJamMenuAction PressInList(FJamMenu& Menu, EJamMenuItem Item, EJamMenuKey Key)
{
	switch (Key)
	{
		case EJamMenuKey::Up:
		case EJamMenuKey::Down:
			Menu.ListSelected = Cycle(Menu.ListSelected, Key == EJamMenuKey::Down ? 1 : -1, JamMenuRules::OptionCount(Menu.Settings, Item));
			return EJamMenuAction::None;
		case EJamMenuKey::Confirm:
			return Pick(Menu, Menu.ListSelected);
		case EJamMenuKey::Back:
			Menu.bListOpen = false;
			return EJamMenuAction::None;
		default:
			return EJamMenuAction::None;
	}
}

EJamMenuAction Confirm(FJamMenu& Menu, EJamMenuItem Item)
{
	switch (Item)
	{
		case EJamMenuItem::NewGame:
			return EJamMenuAction::NewGame;
		case EJamMenuItem::Exit:
			return EJamMenuAction::Exit;
		case EJamMenuItem::Settings:
			Menu.Page = EJamMenuPage::Settings;
			Menu.Selected = 0;
			return EJamMenuAction::None;
		case EJamMenuItem::Back:
			return CloseSettings(Menu);
		case EJamMenuItem::Difficulty:
			return StepDifficulty(Menu.Settings, 1);
		default:
			if (JamMenuRules::IsList(Item))
			{
				OpenList(Menu, Item);
			}
			return EJamMenuAction::None;
	}
}

/** The row At is over, INDEX_NONE off them all. */
int32 RowUnder(const FJamMenuLayout& Layout, const FVector2D& At)
{
	return Layout.Rows.IndexOfByPredicate([&At](const FBox2D& Row) { return Row.IsInsideOrOn(At); });
}

/** The open list's options under the control at Control, kept on a screen Height tall. */
TArray<FBox2D> ListUnder(const FBox2D& Control, int32 Count, double RowHeight, double Height)
{
	TArray<FBox2D> Options;
	if (Count <= 0)
	{
		return Options;
	}
	const double OptionHeight = FMath::Min(OptionShare * RowHeight, (1.0 - 2.0 * ListMargin) * Height / Count);
	const double Top = FMath::Max(FMath::Min(Control.Max.Y, (1.0 - ListMargin) * Height - Count * OptionHeight), ListMargin * Height);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const double Y = Top + Index * OptionHeight;
		Options.Add(FBox2D(FVector2D(Control.Min.X, Y), FVector2D(Control.Max.X, Y + OptionHeight)));
	}
	return Options;
}
} // namespace

TArray<EJamMenuItem> JamMenuRules::ItemsOf(EJamMenuPage Page)
{
	if (Page == EJamMenuPage::Main)
	{
		return { EJamMenuItem::NewGame, EJamMenuItem::Difficulty, EJamMenuItem::Settings, EJamMenuItem::Exit };
	}
	return { EJamMenuItem::Quality, EJamMenuItem::Window, EJamMenuItem::Resolution, EJamMenuItem::Dlss, EJamMenuItem::Master,
		EJamMenuItem::Music, EJamMenuItem::Voice, EJamMenuItem::Back };
}

EJamMenuAction JamMenuRules::Press(FJamMenu& Menu, EJamMenuKey Key)
{
	const TArray<EJamMenuItem> Items = ItemsOf(Menu.Page);
	Menu.Selected = FMath::Clamp(Menu.Selected, 0, Items.Num() - 1);
	const EJamMenuItem Item = Items[Menu.Selected];
	if (Menu.bListOpen)
	{
		return PressInList(Menu, Item, Key);
	}
	switch (Key)
	{
		case EJamMenuKey::Up:
			Menu.Selected = Cycle(Menu.Selected, -1, Items.Num());
			return EJamMenuAction::None;
		case EJamMenuKey::Down:
			Menu.Selected = Cycle(Menu.Selected, 1, Items.Num());
			return EJamMenuAction::None;
		case EJamMenuKey::Left:
		case EJamMenuKey::Right:
			return PressSide(Menu.Settings, Item, Key == EJamMenuKey::Right ? 1 : -1);
		case EJamMenuKey::Confirm:
			return Confirm(Menu, Item);
		default:
			return Menu.Page == EJamMenuPage::Settings ? CloseSettings(Menu) : EJamMenuAction::None;
	}
}

void JamMenuRules::Point(FJamMenu& Menu, int32 Row)
{
	if (Row >= 0 && Row < ItemsOf(Menu.Page).Num())
	{
		Menu.Selected = Row;
	}
}

int32 JamMenuRules::RowAt(float Y, float Top, float RowHeight, int32 Count)
{
	if (RowHeight <= 0.f || Y < Top)
	{
		return INDEX_NONE;
	}
	const int32 Row = FMath::FloorToInt32((Y - Top) / RowHeight);
	return Row < Count ? Row : INDEX_NONE;
}

float JamMenuRules::Volume(int32 Steps)
{
	return static_cast<float>(FMath::Clamp(Steps, 0, VolumeSteps)) / VolumeSteps;
}

bool JamMenuRules::IsList(EJamMenuItem Item)
{
	return Item == EJamMenuItem::Quality || Item == EJamMenuItem::Window || Item == EJamMenuItem::Resolution || Item == EJamMenuItem::Dlss;
}

bool JamMenuRules::IsSlider(EJamMenuItem Item)
{
	return Item == EJamMenuItem::Master || Item == EJamMenuItem::Music || Item == EJamMenuItem::Voice;
}

int32 JamMenuRules::DlssFor(int32 Saved, bool bSupported)
{
	if (!bSupported)
	{
		return 0;
	}
	return Saved == INDEX_NONE ? 1 : FMath::Clamp(Saved, 0, DlssCount - 1);
}

bool JamMenuRules::IsSelector(EJamMenuItem Item)
{
	return Item == EJamMenuItem::Difficulty;
}

int32 JamMenuRules::OptionCount(const FJamMenuSettings& Settings, EJamMenuItem Item)
{
	switch (Item)
	{
		case EJamMenuItem::Quality:
			return QualityCount;
		case EJamMenuItem::Window:
			return WindowCount;
		case EJamMenuItem::Resolution:
			return Settings.ResolutionCount;
		case EJamMenuItem::Dlss:
			return Settings.bDlssSupported ? DlssCount : 1;
		default:
			return 0;
	}
}

FJamMenuLayout JamMenuRules::Layout(const FJamMenu& Menu, const FVector2D& Screen)
{
	const TArray<EJamMenuItem> Items = ItemsOf(Menu.Page);
	const bool bSettings = Menu.Page == EJamMenuPage::Settings;
	const double Column = (bSettings ? SettingsColumn : MainColumn) * Screen.X;
	const double Left = 0.5 * (Screen.X - Column);
	const double Top = (bSettings ? SettingsTop : MainTop) * Screen.Y;
	const double Row = RowShare * Screen.Y;
	FJamMenuLayout Out;
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		const double RowTop = Top + Index * Row;
		Out.Rows.Add(FBox2D(FVector2D(Left, RowTop), FVector2D(Left + Column, RowTop + Row)));
		const bool bSelector = IsSelector(Items[Index]);
		const bool bControl = IsList(Items[Index]) || IsSlider(Items[Index]) || bSelector;
		const double From = bSelector ? SelectorFrom : ControlFrom;
		const double To = bSelector ? SelectorTo : 1.0 - ControlPadding;
		Out.Controls.Add(bControl ? FBox2D(FVector2D(Left + From * Column, RowTop + ControlInset * Row),
										FVector2D(Left + To * Column, RowTop + (1.0 - ControlInset) * Row))
								  : FBox2D(ForceInit));
	}
	if (bSettings)
	{
		const double Pad = PanelPadding * Screen.Y;
		Out.Panel =
			FBox2D(FVector2D(Left - Pad, Top - Pad), FVector2D(Left + Column + Pad, Top + Items.Num() * Row + PanelHint * Screen.Y));
	}
	if (Menu.bListOpen && Out.Controls.IsValidIndex(Menu.Selected))
	{
		Out.Options = ListUnder(Out.Controls[Menu.Selected], OptionCount(Menu.Settings, Items[Menu.Selected]), Row, Screen.Y);
	}
	return Out;
}

void JamMenuRules::Hover(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At)
{
	if (Menu.bDragging)
	{
		return;
	}
	if (Menu.bListOpen)
	{
		const int32 Option = Layout.Options.IndexOfByPredicate([&At](const FBox2D& Box) { return Box.IsInsideOrOn(At); });
		Menu.ListSelected = Option != INDEX_NONE ? Option : Menu.ListSelected;
		return;
	}
	Point(Menu, RowUnder(Layout, At));
}

EJamMenuAction JamMenuRules::Click(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At)
{
	if (Menu.bListOpen)
	{
		const int32 Option = Layout.Options.IndexOfByPredicate([&At](const FBox2D& Box) { return Box.IsInsideOrOn(At); });
		if (Option != INDEX_NONE)
		{
			return Pick(Menu, Option);
		}
		Menu.bListOpen = false;
		return EJamMenuAction::None;
	}
	const TArray<EJamMenuItem> Items = ItemsOf(Menu.Page);
	const int32 Row = RowUnder(Layout, At);
	if (!Items.IsValidIndex(Row))
	{
		return EJamMenuAction::None;
	}
	Menu.Selected = Row;
	const EJamMenuItem Item = Items[Row];
	if (IsSlider(Item))
	{
		if (!Layout.Controls.IsValidIndex(Row) || !Layout.Controls[Row].IsInsideOrOn(At))
		{
			return EJamMenuAction::None;
		}
		Menu.bDragging = true;
		return Slide(Menu.Settings, Item, Layout.Controls[Row], At.X);
	}
	// T-101: the selector's left half is its < arrow, the right half its >; the label steps it on like Enter.
	if (IsSelector(Item) && Layout.Controls.IsValidIndex(Row) && Layout.Controls[Row].IsInsideOrOn(At))
	{
		return StepDifficulty(Menu.Settings, At.X < Layout.Controls[Row].GetCenter().X ? -1 : 1);
	}
	return Confirm(Menu, Item);
}

EJamMenuAction JamMenuRules::Drag(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At)
{
	const TArray<EJamMenuItem> Items = ItemsOf(Menu.Page);
	if (!Menu.bDragging || !Items.IsValidIndex(Menu.Selected) || !IsSlider(Items[Menu.Selected]) ||
		!Layout.Controls.IsValidIndex(Menu.Selected))
	{
		return EJamMenuAction::None;
	}
	return Slide(Menu.Settings, Items[Menu.Selected], Layout.Controls[Menu.Selected], At.X);
}

void JamMenuRules::Release(FJamMenu& Menu)
{
	Menu.bDragging = false;
}

FIntPoint JamMenuRules::FitWindow(const FIntPoint& Size, const FIntPoint& Desktop)
{
	// A window takes at most this share of the desktop each way, so its title bar and the taskbar stay on the screen.
	constexpr double FitWindowShare = 0.9;
	if (Desktop.X <= 0 || Desktop.Y <= 0 || Size.X <= 0 || Size.Y <= 0)
	{
		return Size;
	}
	const double Scale = FMath::Min3(1.0, FitWindowShare * Desktop.X / Size.X, FitWindowShare * Desktop.Y / Size.Y);
	if (Scale >= 1.0 && Size.X < Desktop.X && Size.Y < Desktop.Y)
	{
		return Size;
	}
	return FIntPoint(FMath::FloorToInt32(Size.X * Scale), FMath::FloorToInt32(Size.Y * Scale));
}
