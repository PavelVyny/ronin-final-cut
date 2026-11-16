#include "UI/JamHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "UI/JamHUDPixels.h"
#include "UI/JamTexts.h"

namespace
{
// T-092: the main menu between the two fighters, as fractions of the screen. Where the rows and the controls are is
// JamMenuRules::Layout's (T-094), so the mouse hits what is drawn.
constexpr float MenuTitleTop = 0.08f;
constexpr float MenuTitleScale = 4.f;
constexpr float MenuTextScale = 1.8f;
constexpr float MenuHintTop = 0.93f;
constexpr float MenuHintScale = 1.2f;
constexpr JamHudPixels::FPixels MenuEdge{ 4.f };
constexpr JamHudPixels::FPixels MenuPadding{ 16.f };
// Pixels the title's shadow sits down and right.
constexpr JamHudPixels::FPixels MenuShadow{ 3.f };
// A control's frame, the slider's track as a share of its height, its knob's width in pixels.
constexpr JamHudPixels::FPixels ControlBorder{ 2.f };
constexpr float TrackShare = 0.2f;
constexpr JamHudPixels::FPixels KnobWidth{ 10.f };
// A list's options' text at most this big, and this share of an option's height.
constexpr float OptionTextScale = 1.4f;
constexpr float OptionTextShare = 0.75f;
const FLinearColor MenuCyan(0.2f, 0.9f, 1.0f);
const FLinearColor MenuPink(1.0f, 0.3f, 0.85f);
const FLinearColor MenuShadowColor(0.f, 0.f, 0.f, 0.8f);
// The author's 6a look: the main page's bands were too see-through over the arena.
const FLinearColor MenuBand(0.01f, 0.01f, 0.03f, 0.62f);
const FLinearColor MenuBandSelected(0.01f, 0.01f, 0.03f, 0.85f);
// T-094: the settings page sits on an opaque panel: the arena showed through and the words were hard to read.
const FLinearColor MenuPanel(0.02f, 0.02f, 0.05f, 0.94f);
const FLinearColor PanelBand(0.06f, 0.07f, 0.11f, 1.f);
const FLinearColor PanelBandSelected(0.11f, 0.14f, 0.22f, 1.f);
const FLinearColor ControlFill(0.01f, 0.01f, 0.03f, 1.f);
const FLinearColor ControlFrame(0.35f, 0.37f, 0.45f, 1.f);
const FLinearColor TrackColor(0.22f, 0.23f, 0.3f, 1.f);
const FLinearColor ListFill(0.03f, 0.03f, 0.07f, 1.f);
const FLinearColor MenuIdleText(0.72f, 0.74f, 0.8f);
const FLinearColor MenuHintColor(0.6f, 0.6f, 0.7f);
} // namespace

FString AJamHUD::OptionName(EJamMenuItem Item, int32 Index) const
{
	switch (Item)
	{
		case EJamMenuItem::Quality:
			return JamTexts::QualityNames[FMath::Clamp(Index, 0, JamMenuRules::QualityCount - 1)];
		case EJamMenuItem::Window:
			return JamTexts::WindowNames[FMath::Clamp(Index, 0, JamMenuRules::WindowCount - 1)];
		case EJamMenuItem::Resolution:
			return Resolutions.IsValidIndex(Index) ? FString::Printf(TEXT("%d x %d"), Resolutions[Index].X, Resolutions[Index].Y)
												   : FString(TEXT("-"));
		case EJamMenuItem::Dlss:
			return JamTexts::DlssNames[FMath::Clamp(Index, 0, JamMenuRules::DlssCount - 1)];
		default:
			return FString();
	}
}

FString AJamHUD::MenuValue(EJamMenuItem Item) const
{
	const FJamMenuSettings& Settings = Menu.Settings;
	switch (Item)
	{
		case EJamMenuItem::Quality:
			return OptionName(Item, Settings.Quality);
		case EJamMenuItem::Window:
			return OptionName(Item, Settings.Window);
		case EJamMenuItem::Resolution:
			return OptionName(Item, Settings.Resolution);
		case EJamMenuItem::Dlss:
			return OptionName(Item, Settings.Dlss);
		case EJamMenuItem::Master:
			return FString::Printf(TEXT("%d%%"), FMath::RoundToInt32(100.f * JamMenuRules::Volume(Settings.Master)));
		case EJamMenuItem::Music:
			return FString::Printf(TEXT("%d%%"), FMath::RoundToInt32(100.f * JamMenuRules::Volume(Settings.Music)));
		case EJamMenuItem::Voice:
			return FString::Printf(TEXT("%d%%"), FMath::RoundToInt32(100.f * JamMenuRules::Volume(Settings.Voice)));
		case EJamMenuItem::Difficulty:
			return JamTexts::DifficultyNames[FMath::Clamp(Settings.Difficulty, 0, JamMenuRules::DifficultyCount - 1)];
		default:
			return FString();
	}
}

void AJamHUD::DrawMenuTitle()
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	UFont* Font = GEngine->GetMediumFont();
	// The title in the cover's colours: RONIN in cyan, FINAL CUT in pink.
	const FString Title = JamTexts::MenuTitle;
	FString Ronin;
	FString Cut;
	if (!Title.Split(TEXT(" "), &Ronin, &Cut))
	{
		Ronin = Title;
	}
	Ronin += TEXT(" ");
	float RoninWidth = 0.f;
	float CutWidth = 0.f;
	float TitleHeight = 0.f;
	GetTextSize(Ronin, RoninWidth, TitleHeight, Font, MenuTitleScale);
	GetTextSize(Cut, CutWidth, TitleHeight, Font, MenuTitleScale);
	const float TitleX = 0.5f * (W - RoninWidth - CutWidth);
	const float TitleY = MenuTitleTop * H;
	DrawText(Ronin, MenuShadowColor, TitleX + MenuShadow, TitleY + MenuShadow, Font, MenuTitleScale);
	DrawText(Cut, MenuShadowColor, TitleX + RoninWidth + MenuShadow, TitleY + MenuShadow, Font, MenuTitleScale);
	DrawText(Ronin, MenuCyan, TitleX, TitleY, Font, MenuTitleScale);
	DrawText(Cut, MenuPink, TitleX + RoninWidth, TitleY, Font, MenuTitleScale);
}

void AJamHUD::DrawFramedBox(const FBox2D& Box, const FLinearColor& Fill, const FLinearColor& Frame)
{
	const FVector2D Size = Box.GetSize();
	DrawRect(Frame, Box.Min.X - ControlBorder, Box.Min.Y - ControlBorder, Size.X + 2.f * ControlBorder, Size.Y + 2.f * ControlBorder);
	DrawRect(Fill, Box.Min.X, Box.Min.Y, Size.X, Size.Y);
}

void AJamHUD::DrawDropdown(EJamMenuItem Item, const FBox2D& Box, bool bSelected)
{
	UFont* Font = GEngine->GetMediumFont();
	DrawFramedBox(Box, ControlFill, bSelected ? MenuCyan : ControlFrame);
	const FString Value = MenuValue(Item);
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Value, Width, Height, Font, MenuTextScale);
	const FVector2D Size = Box.GetSize();
	DrawText(Value, bSelected ? FLinearColor::White : MenuIdleText, Box.Min.X + MenuPadding, Box.Min.Y + 0.5f * (Size.Y - Height), Font,
		MenuTextScale);
	// The arrow that says a list opens here, pointing down, at the box's right end.
	const float Half = 0.18f * Size.Y;
	const FVector2D Tip(Box.Max.X - MenuPadding - Half, Box.GetCenter().Y + 0.5f * Half);
	FCanvasTriangleItem Arrow(Tip + FVector2D(-Half, -Half), Tip + FVector2D(Half, -Half), Tip, GWhiteTexture);
	Arrow.SetColor(bSelected ? MenuCyan : ControlFrame);
	Canvas->DrawItem(Arrow);
}

void AJamHUD::DrawSlider(EJamMenuItem Item, const FBox2D& Track, bool bSelected)
{
	UFont* Font = GEngine->GetMediumFont();
	const FJamMenuSettings& Settings = Menu.Settings;
	const int32 Steps = Item == EJamMenuItem::Master ? Settings.Master : (Item == EJamMenuItem::Music ? Settings.Music : Settings.Voice);
	const float Fraction = JamMenuRules::Volume(Steps);
	const FVector2D Size = Track.GetSize();
	const float TrackThickness = FMath::Max(4.f, TrackShare * Size.Y);
	const float TrackTop = Track.GetCenter().Y - 0.5f * TrackThickness;
	DrawRect(TrackColor, Track.Min.X, TrackTop, Size.X, TrackThickness);
	DrawRect(bSelected ? MenuCyan : MenuIdleText, Track.Min.X, TrackTop, Fraction * Size.X, TrackThickness);
	const float KnobX = Track.Min.X + Fraction * Size.X - 0.5f * KnobWidth;
	DrawRect(bSelected ? FLinearColor::White : MenuIdleText, KnobX, Track.Min.Y, KnobWidth, Size.Y);
	// The percentage just left of the track, where the label's half of the row ends.
	const FString Value = MenuValue(Item);
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Value, Width, Height, Font, MenuTextScale);
	DrawText(Value, bSelected ? MenuCyan : MenuIdleText, Track.Min.X - MenuPadding - KnobWidth - Width, Track.GetCenter().Y - 0.5f * Height,
		Font, MenuTextScale);
}

float AJamHUD::FittedMenuScale(const FString& Text, float Room)
{
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Text, Width, Height, GEngine->GetMediumFont(), MenuTextScale);
	return Width > Room && Width > 0.f ? MenuTextScale * FMath::Max(Room, 1.f) / Width : MenuTextScale;
}

void AJamHUD::DrawSelector(EJamMenuItem Item, const FBox2D& Box, bool bSelected)
{
	UFont* Font = GEngine->GetMediumFont();
	// No frame: on the main page it reads as the row's text, < EASY >, after its label. The arrows are what the mouse
	// clicks (JamMenuRules::Click: the box's left half steps back, the right half on).
	const FVector2D Size = Box.GetSize();
	const float Half = 0.26f * Size.Y;
	const float Y = Box.GetCenter().Y;
	const float LeftTip = Box.Min.X + MenuPadding;
	const float RightTip = Box.Max.X - MenuPadding;
	FCanvasTriangleItem Back(
		FVector2D(LeftTip, Y), FVector2D(LeftTip + Half, Y - Half), FVector2D(LeftTip + Half, Y + Half), GWhiteTexture);
	FCanvasTriangleItem On(
		FVector2D(RightTip, Y), FVector2D(RightTip - Half, Y + Half), FVector2D(RightTip - Half, Y - Half), GWhiteTexture);
	Back.SetColor(bSelected ? MenuCyan : MenuIdleText);
	On.SetColor(bSelected ? MenuCyan : MenuIdleText);
	Canvas->DrawItem(Back);
	Canvas->DrawItem(On);
	const FString Value = MenuValue(Item);
	const float Scale = FittedMenuScale(Value, Size.X - 2.f * (MenuPadding + Half + MenuPadding));
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Value, Width, Height, Font, Scale);
	DrawText(Value, bSelected ? MenuCyan : MenuIdleText, Box.GetCenter().X - 0.5f * Width, Y - 0.5f * Height, Font, Scale);
}

void AJamHUD::DrawMenuList(const FJamMenuLayout& Layout)
{
	UFont* Font = GEngine->GetMediumFont();
	const EJamMenuItem Item = JamMenuRules::ItemsOf(Menu.Page)[Menu.Selected];
	const FBox2D Whole(Layout.Options[0].Min, Layout.Options.Last().Max);
	DrawFramedBox(Whole, ListFill, MenuCyan);
	float Unused = 0.f;
	float LineHeight = 0.f;
	GetTextSize(TEXT("X"), Unused, LineHeight, Font, 1.f);
	const float OptionHeight = Layout.Options[0].GetSize().Y;
	const float Scale = FMath::Min(OptionTextScale, OptionTextShare * OptionHeight / FMath::Max(LineHeight, 1.f));
	const int32 Current = Item == EJamMenuItem::Quality ? Menu.Settings.Quality
		: Item == EJamMenuItem::Window                  ? Menu.Settings.Window
		: Item == EJamMenuItem::Dlss                    ? Menu.Settings.Dlss
														: Menu.Settings.Resolution;
	for (int32 Index = 0; Index < Layout.Options.Num(); ++Index)
	{
		const FBox2D& Option = Layout.Options[Index];
		if (Index == Menu.ListSelected)
		{
			DrawRect(PanelBandSelected, Option.Min.X, Option.Min.Y, Option.GetSize().X, OptionHeight);
		}
		const FLinearColor Color = Index == Current ? MenuCyan : (Index == Menu.ListSelected ? FLinearColor::White : MenuIdleText);
		DrawText(OptionName(Item, Index), Color, Option.Min.X + MenuPadding, Option.Min.Y + 0.5f * (OptionHeight - Scale * LineHeight),
			Font, Scale);
	}
}

void AJamHUD::DrawMenuRow(EJamMenuItem Item, const FBox2D& Row, const FBox2D& Control, bool bSelected)
{
	UFont* Font = GEngine->GetMediumFont();
	const bool bSettings = Menu.Page == EJamMenuPage::Settings;
	const FVector2D Size = Row.GetSize();
	const FLinearColor Band = bSettings ? (bSelected ? PanelBandSelected : PanelBand) : (bSelected ? MenuBandSelected : MenuBand);
	DrawRect(Band, Row.Min.X, Row.Min.Y + 0.5f * MenuEdge, Size.X, Size.Y - MenuEdge);
	if (bSelected)
	{
		DrawRect(MenuCyan, Row.Min.X, Row.Min.Y + 0.5f * MenuEdge, MenuEdge, Size.Y - MenuEdge);
	}
	const FLinearColor Color = bSelected ? FLinearColor::White : MenuIdleText;
	const FString Label = JamTexts::MenuItem(static_cast<int32>(Item));
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Label, Width, Height, Font, MenuTextScale);
	const float TextY = Row.GetCenter().Y - 0.5f * Height;
	if (!Control.bIsValid)
	{
		DrawText(Label, Color, Row.GetCenter().X - 0.5f * Width, TextY, Font, MenuTextScale);
		return;
	}
	// A label longer than the room left of its control is drawn smaller rather than under it. The difficulty's ends just
	// left of its arrows, so the two read as one centred line.
	const bool bSelector = JamMenuRules::IsSelector(Item);
	const float RowLabelScale = FittedMenuScale(Label, Control.Min.X - Row.Min.X - 2.f * MenuPadding);
	GetTextSize(Label, Width, Height, Font, RowLabelScale);
	const float LabelX = bSelector ? Control.Min.X - MenuPadding - Width : Row.Min.X + MenuPadding;
	DrawText(Label, Color, LabelX, Row.GetCenter().Y - 0.5f * Height, Font, RowLabelScale);
	if (JamMenuRules::IsList(Item))
	{
		DrawDropdown(Item, Control, bSelected);
	}
	else if (JamMenuRules::IsSelector(Item))
	{
		DrawSelector(Item, Control, bSelected);
	}
	else
	{
		DrawSlider(Item, Control, bSelected);
	}
}

void AJamHUD::DrawMenu()
{
	const FVector2D Screen(Canvas->ClipX, Canvas->ClipY);
	DrawMenuTitle();
	const FJamMenuLayout Layout = JamMenuRules::Layout(Menu, Screen);
	const TArray<EJamMenuItem> Items = JamMenuRules::ItemsOf(Menu.Page);
	if (Layout.Panel.bIsValid)
	{
		DrawFramedBox(Layout.Panel, MenuPanel, ControlFrame);
	}
	for (int32 Index = 0; Index < FMath::Min(Items.Num(), Layout.Rows.Num()); ++Index)
	{
		DrawMenuRow(Items[Index], Layout.Rows[Index], Layout.Controls[Index], Index == Menu.Selected);
	}
	if (Menu.Page == EJamMenuPage::Settings && !Layout.Rows.IsEmpty())
	{
		DrawCentered(JamTexts::SettingsHint, Layout.Rows.Last().Max.Y + MenuPadding, MenuHintScale, MenuHintColor);
	}
	// The open list goes over everything else on the page.
	if (!Layout.Options.IsEmpty())
	{
		DrawMenuList(Layout);
	}
	DrawCentered(JamTexts::MenuHint, MenuHintTop * Screen.Y, MenuHintScale, MenuHintColor);
}
