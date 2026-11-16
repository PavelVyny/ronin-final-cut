#include "UI/JamHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineFontServices.h"
#include "Fonts/FontMeasure.h"
#include "Core/JamConfig.h"
#include "GlobalRenderResources.h"
#include "Sim/JamFightSubsystem.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamStealRules.h"
#include "UI/JamCardRules.h"
#include "UI/JamHUDPixels.h"
#include "UI/JamHudScaleRules.h"
#include "UI/JamMarkerRules.h"
#include "UI/JamTexts.h"

namespace
{
// Layout of the prototype HUD as fractions of the screen: presentation only, replaced by Kirill's UI in block E.
constexpr float BossBarWidth = 0.5f;
constexpr float BossBarTop = 0.04f;
constexpr float HeroBarWidth = 0.22f;
constexpr float HeroBarBottom = 0.93f;
constexpr float BarHeight = 0.022f;
constexpr float HudMargin = 0.03f;
constexpr float HudTextScale = 1.4f;
constexpr JamHudPixels::FPixels HintRowHeight{ 20.f };
constexpr JamHudPixels::FPixels LoadoutRowHeight{ 24.f };
constexpr float LoadoutScale = 1.2f;
constexpr JamHudPixels::FPixels LoadoutNameOffset{ 70.f };
// The hero's health numbers over his bar, and the loadout's bottom over them.
constexpr JamHudPixels::FPixels HeroTextLift{ 28.f };
constexpr JamHudPixels::FPixels LoadoutLift{ 6.f };
constexpr float EndingBandTop = 0.36f;
constexpr float EndingBandHeight = 0.24f;
// The intro's card sits low: the Publisher's show-off is in the middle of the screen.
constexpr float IntroBandTop = 0.7f;
constexpr float IntroBandHeight = 0.24f;
constexpr float EndingTitleScale = 3.2f;
// The steal card: a band across the screen with the options side by side and the timer under them. The band and the
// options' boxes are JamCardRules', where a click finds them.
constexpr float CardTitleScale = 2.8f;
constexpr float CardAskRow = 0.09f;
constexpr JamHudPixels::FPixels CardOptionEdge{ 4.f };
constexpr JamHudPixels::FPixels CardPadding{ 14.f };
constexpr float CardNameScale = 2.4f;
constexpr float CardNameRow = 0.055f;
constexpr float CardTimerRow = 0.34f;
constexpr float CardTimerHeight = 0.008f;
constexpr float CardHintRow = 0.37f;
// The Ultimate meter under the health bar and the banner of a full one, low in the middle of the screen.
constexpr float UltimateBarHeight = 0.012f;
constexpr JamHudPixels::FPixels UltimateBarGap{ 6.f };
constexpr float UltimateBannerTop = 0.8f;
constexpr float UltimateBannerScale = 2.4f;
constexpr float UltimatePulseHz = 1.5f;
constexpr float UltimatePulseWhite = 0.6f;
// A call over the hero's head: world height in cm, text scale.
constexpr float OverHeroHeight = 230.f;
constexpr float OverHeroScale = 1.6f;
// The arrow to the Publisher off screen: pixels from the edge, its size in pixels, the height on him it points at (cm).
constexpr JamHudPixels::FPixels BossMarkerMargin{ 48.f };
constexpr JamHudPixels::FPixels BossMarkerSize{ 34.f };
constexpr float BossMarkerHeight = 100.f;
// Pixels between the bottom of his health numbers and the arrow's edge.
constexpr JamHudPixels::FPixels BossMarkerBarGap{ 16.f };
// Pixels between his health bar and its numbers.
constexpr JamHudPixels::FPixels BossTextGap{ 4.f };
constexpr float StunBandTop = 0.14f;
constexpr float StunBandHeight = 0.13f;
// The voice-over's subtitle: under the steal card (it ends at 0.70), above the intro's band (it starts at 0.70).
constexpr float SubtitleTop = 0.74f;
constexpr float SubtitleIntroTop = 0.64f;
constexpr float SubtitleScale = 1.5f;
constexpr JamHudPixels::FPixels SubtitlePadding{ 10.f };
const FLinearColor BarBackColor(0.f, 0.f, 0.f, 0.6f);
// The Publisher's name in his gold.
const FLinearColor BossNameColor(1.0f, 0.78f, 0.25f);
const FLinearColor BossBarColor(0.9f, 0.15f, 0.15f);
const FLinearColor HeroBarColor(0.2f, 0.9f, 1.0f);
// T-053: health just lost stays white for a moment; health just gained shows light green before the bar fills to it.
const FLinearColor ChunkColor(1.f, 1.f, 1.f, 0.9f);
const FLinearColor PreviewColor(0.55f, 1.f, 0.6f, 0.85f);
const FLinearColor StolenColor(1.0f, 0.35f, 0.3f);
const FLinearColor EndingBandColor(0.f, 0.f, 0.f, 0.75f);
const FLinearColor WinColor(1.0f, 0.3f, 0.85f);
const FLinearColor LockedColor(0.45f, 0.45f, 0.45f);

/** A button of the hero, the mechanic that lives on it and the name of the key. */
struct FLoadoutRow
{
	EJamButton Button;
	EJamMechanic Mechanic;
	const TCHAR* Key;
};

const FLoadoutRow LoadoutRows[] = {
	{ EJamButton::Primary, EJamMechanic::Katana, TEXT("LMB") },
	{ EJamButton::Secondary, EJamMechanic::Shuriken, TEXT("RMB") },
	{ EJamButton::Dash, EJamMechanic::Dash, TEXT("SPACE") },
	{ EJamButton::Parry, EJamMechanic::Parry, TEXT("LMB") },
	{ EJamButton::Ultimate, EJamMechanic::Ultimate, TEXT("E") },
};

/** What stands over an option of the card: the key that gives it away, or the mark of a cut nobody asks about. */
const TCHAR* OptionKey(const FJamStealState& Steal, int32 Index)
{
	if (Steal.bTakesAll)
	{
		return JamTexts::StealAllMark;
	}
	return Index == 0 ? JamTexts::StealLeftKey : JamTexts::StealRightKey;
}

/** Color at Alpha of its own, for the words of a panel still popping in. */
FLinearColor Faded(const FLinearColor& Color, float Alpha)
{
	return FLinearColor(Color.R, Color.G, Color.B, Color.A * FMath::Clamp(Alpha, 0.f, 1.f));
}

/** Box grown from its centre by Grow (past 1 while it overshoots). */
FBox2D Grown(const FBox2D& Box, float Grow)
{
	const FVector2D Half = 0.5 * Box.GetSize() * FMath::Max(Grow, 0.f);
	return FBox2D(Box.GetCenter() - Half, Box.GetCenter() + Half);
}
} // namespace

void AJamHUD::DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& Color)
{
	if (!bBars)
	{
		return;
	}
	DrawRect(BarBackColor, X, Y, Width, Height);
	DrawRect(Color, X, Y, Width * FMath::Clamp(Fraction, 0.f, 1.f), Height);
}

void AJamHUD::DrawBossBar(const FJamFighter& Boss)
{
	// T-083: the Publisher's bar stays away through his show-off and fades in as the fight starts.
	if (BossBarAlpha <= 0.f)
	{
		return;
	}
	const UJamFightSubsystem* Host = GetWorld()->GetSubsystem<UJamFightSubsystem>();
	if (bBars && bNeonBars && Host)
	{
		DrawNeonBoss(Boss, Host->GetBalance());
		return;
	}
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float BossX = 0.5f * (1.f - BossBarWidth) * W;
	DrawHealthBar(BossX, BossBarTop * H, BossBarWidth * W, BarHeight * H, BossBar, BossBarColor, BossBarAlpha);
	const FString BossText = JamTexts::BossHealth(FMath::CeilToInt32(Boss.Health), FMath::CeilToInt32(Boss.MaxHealth));
	DrawText(BossText, FLinearColor(1.f, 1.f, 1.f, BossBarAlpha), BossX, (BossBarTop + BarHeight) * H + BossTextGap,
		GEngine->GetMediumFont(), HudTextScale);
}

void AJamHUD::DrawHealthBar(float X, float Y, float Width, float Height, const FJamBarState& Bar, const FLinearColor& Color, float Alpha)
{
	if (!bBars || Alpha <= 0.f)
	{
		return;
	}
	const FLinearColor Fade(1.f, 1.f, 1.f, Alpha);
	DrawRect(BarBackColor * Fade, X, Y, Width, Height);
	DrawRect(ChunkColor * Fade, X + Width * Bar.Fill, Y, Width * (Bar.Chunk - Bar.Fill), Height);
	DrawRect(PreviewColor * Fade, X + Width * Bar.Fill, Y, Width * (Bar.Preview - Bar.Fill), Height);
	DrawRect(Color * Fade, X, Y, Width * Bar.Fill, Height);
}

void AJamHUD::DrawLoadout(const FJamLedger& Ledger, float Meter, float X, float Bottom)
{
	UFont* Font = GEngine->GetMediumFont();
	const int32 RowCount = UE_ARRAY_COUNT(LoadoutRows);
	for (int32 Index = 0; Index < RowCount; ++Index)
	{
		const FLoadoutRow& Row = LoadoutRows[Index];
		const float Y = Bottom - (RowCount - Index) * LoadoutRowHeight;
		const TCHAR* Name = JamLoadoutRules::Name(Row.Mechanic);
		if (Ledger.Has(EJamSide::Hero, Row.Mechanic))
		{
			DrawText(Row.Key, HeroBarColor, X, Y, Font, LoadoutScale);
			const FString Label = Row.Mechanic == EJamMechanic::Ultimate ? JamTexts::UltimateMeter(Name, Meter) : FString(Name);
			DrawText(Label, HeroBarColor, X + LoadoutNameOffset, Y, Font, LoadoutScale);
			continue;
		}
		const bool bBall = JamLoadoutRules::ActionFor(Row.Button, EJamSide::Hero, Ledger) == EJamAction::Ball;
		DrawText(Row.Key, StolenColor, X, Y, Font, LoadoutScale);
		DrawText(JamTexts::LoadoutStolen(bBall, Name), StolenColor, X + LoadoutNameOffset, Y, Font, LoadoutScale);
	}
}

void AJamHUD::DrawHint(const FString& Text, int32 Row)
{
	UFont* Font = GEngine->GetMediumFont();
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Text, Width, Height, Font, 1.f);
	DrawText(Text, FLinearColor::White, (1.f - HudMargin) * Canvas->ClipX - Width, HeroBarBottom * Canvas->ClipY - Row * HintRowHeight,
		Font, 1.f);
}

void AJamHUD::DrawCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color)
{
	UFont* Font = GEngine->GetMediumFont();
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Text, Width, Height, Font, Scale);
	DrawText(Text, Color, 0.5f * (Canvas->ClipX - Width), Y, Font, Scale);
}

void AJamHUD::DrawStealCard(const FJamFight& Fight, float Age)
{
	const FJamStealState& Steal = Fight.Steal;
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float Top = JamCardRules::CardTop * H;
	const float Grow = JamCardRules::Pop(Age);
	DrawBand(Top, JamCardRules::CardHeight * H, Grow);
	DrawCentered(
		Steal.bTakesAll ? JamTexts::StealAllTitle : JamTexts::StealTitle, Top + 0.02f * H, CardTitleScale, Faded(StolenColor, Grow));
	DrawCentered(
		Steal.bTakesAll ? JamTexts::StealAllAsk : JamTexts::StealAsk, Top + CardAskRow * H, HudTextScale, Faded(FLinearColor::White, Grow));
	const FVector2D Screen(W, H);
	const int32 Count = Steal.Offer.Num();
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float OptionGrow = JamCardRules::Pop(Age, JamCardRules::OptionDelay * (Index + 1));
		DrawStealOption(Fight, Index, Grown(JamCardRules::OptionBox(Index, Count, Screen), OptionGrow), OptionGrow);
	}
	const float TimerWidth = (2.f * JamCardRules::OptionWidth + JamCardRules::OptionGap) * W;
	const float TimeLeft = JamStealRules::TimeLeft(Steal, static_cast<float>(Fight.Tick));
	if (Grow >= 1.f)
	{
		DrawBar(0.5f * (W - TimerWidth), Top + CardTimerRow * H, TimerWidth, CardTimerHeight * H, TimeLeft, StolenColor);
	}
	DrawCentered(Steal.bTakesAll ? JamTexts::StealAllKey : JamTexts::StealDefault, Top + CardHintRow * H, HudTextScale,
		Faded(FLinearColor::White, Grow));
}

void AJamHUD::DrawStealOption(const FJamFight& Fight, int32 Index, const FBox2D& Box, float Grow)
{
	if (Grow <= 0.f)
	{
		return;
	}
	const FJamStealState& Steal = Fight.Steal;
	UFont* Font = GEngine->GetMediumFont();
	// The options stay dim until the card takes an answer: a key held from the fight must not look like a choice.
	const FLinearColor Color = Faded(Fight.Tick >= Steal.OpenTick ? HeroBarColor : LockedColor, Grow);
	const FVector2D Size = Box.GetSize();
	DrawRect(BarBackColor, Box.Min.X, Box.Min.Y, Size.X, Size.Y);
	DrawRect(Color, Box.Min.X, Box.Min.Y, Size.X, CardOptionEdge);
	const float Scale = FMath::Max(Grow, 0.01f);
	const float Left = Box.Min.X + CardPadding * Scale;
	DrawText(OptionKey(Steal, Index), Color, Left, Box.Min.Y + CardPadding * Scale, Font, HudTextScale * Scale);
	DrawText(JamLoadoutRules::Name(Steal.Offer[Index]), Color, Left, Box.Min.Y + CardNameRow * Canvas->ClipY * Scale, Font,
		CardNameScale * Scale);
}

void AJamHUD::DrawBand(float Top, float Height, float Grow)
{
	const float Shown = Height * FMath::Max(Grow, 0.f);
	DrawRect(EndingBandColor, 0.f, Top + 0.5f * (Height - Shown), Canvas->ClipX, Shown);
}

void AJamHUD::DrawStun(float Grow)
{
	const float H = Canvas->ClipY;
	DrawBand(StunBandTop * H, StunBandHeight * H, Grow);
	DrawCentered(JamTexts::StunTitle, (StunBandTop + 0.015f) * H, CardTitleScale, Faded(WinColor, Grow));
	DrawCentered(JamTexts::StunAsk, (StunBandTop + 0.085f) * H, HudTextScale, Faded(FLinearColor::White, Grow));
}

void AJamHUD::DrawEnding(bool bWon, float Grow)
{
	const float H = Canvas->ClipY;
	DrawBand(EndingBandTop * H, EndingBandHeight * H, Grow);
	DrawCentered(bWon ? JamTexts::WinTitle : JamTexts::LossTitle, (EndingBandTop + 0.06f) * H, EndingTitleScale,
		Faded(bWon ? WinColor : FLinearColor::White, Grow));
	DrawCentered(
		bWon ? JamTexts::WinHint : JamTexts::LossHint, (EndingBandTop + 0.17f) * H, HudTextScale, Faded(FLinearColor::White, Grow));
}

void AJamHUD::DrawIntro()
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float A = FMath::Clamp(IntroAlpha, 0.f, 1.f);
	DrawRect(EndingBandColor * FLinearColor(1.f, 1.f, 1.f, A), 0.f, IntroBandTop * H, W, IntroBandHeight * H);
	DrawCentered(JamTexts::IntroTitle, (IntroBandTop + 0.06f) * H, EndingTitleScale, WinColor * FLinearColor(1.f, 1.f, 1.f, A));
	DrawCentered(JamTexts::IntroLine, (IntroBandTop + 0.17f) * H, HudTextScale, FLinearColor(1.f, 1.f, 1.f, A));
	DrawCentered(JamTexts::IntroSkip, (IntroBandTop + 0.22f) * H, HudTextScale, FLinearColor(0.6f, 0.6f, 0.7f, A));
}

void AJamHUD::DrawPhase(const FJamFight& Fight)
{
	// T-094: a panel pops in when its phase begins; real seconds, so the Ultimate's slow motion does not slow the pop.
	const float Now = GetWorld()->GetRealTimeSeconds();
	if (Fight.Phase != ShownPhase)
	{
		ShownPhase = Fight.Phase;
		PhaseStart = Now;
	}
	const float Age = Now - PhaseStart;
	switch (Fight.Phase)
	{
		case EJamPhase::Steal:
			DrawStealCard(Fight, Age);
			break;
		case EJamPhase::Stunned:
			DrawStun(JamCardRules::Pop(Age));
			break;
		case EJamPhase::HeroDead:
		case EJamPhase::Won:
			DrawEnding(Fight.Phase == EJamPhase::Won, JamCardRules::Pop(Age));
			break;
		default:
			break;
	}
}

void AJamHUD::BeginPlay()
{
	Super::BeginPlay();
	BarSettings = JamConfig::Load<FJamBarSettings>(JamBarRules::Section);
}

void AJamHUD::DrawText(const FString& Text, FLinearColor Color, float ScreenX, float ScreenY, UFont* Font, float Scale, bool bScalePosition)
{
	if (!Canvas)
	{
		return;
	}
	const FVector2D At = bScalePosition ? FVector2D(ScreenX, ScreenY) * Scale : FVector2D(ScreenX, ScreenY);
	FSlateFontInfo Info = (Font ? Font : GEngine->GetMediumFont())->GetLegacySlateFontInfo();
	Info.Size = JamHudScaleRules::FontSize(Info.Size, Scale, JamHudPixels::UiScale);
	FCanvasTextItem Item(At, FText::FromString(Text), Info, Color);
	Canvas->DrawItem(Item);
}

void AJamHUD::GetTextSize(const FString& Text, float& OutWidth, float& OutHeight, UFont* Font, float Scale) const
{
	FSlateFontInfo Info = (Font ? Font : GEngine->GetMediumFont())->GetLegacySlateFontInfo();
	Info.Size = JamHudScaleRules::FontSize(Info.Size, Scale, JamHudPixels::UiScale);
	const TSharedPtr<FSlateFontMeasure> Measure = FEngineFontServices::Get().GetFontMeasure();
	const FVector2D Size = Measure.IsValid() ? FVector2D(Measure->Measure(Text, Info)) : FVector2D::ZeroVector;
	OutWidth = Size.X;
	OutHeight = Size.Y;
}

void AJamHUD::DrawHUD()
{
	Super::DrawHUD();
	// T-099: the HUD grows with the screen, its pixel sizes and its text alike.
	if (Canvas)
	{
		JamHudPixels::UiScale = JamHudScaleRules::UiScale(Canvas->ClipY);
	}
	// A whole-screen page over everything: HOW TO PLAY (T-096), the comic or the menu.
	if ((bHowTo || bMenu || bComic) && Canvas)
	{
		bHowTo ? DrawHowTo() : bComic ? DrawComic() : DrawMenu();
		return;
	}
	const UJamFightSubsystem* Host = GetWorld()->GetSubsystem<UJamFightSubsystem>();
	if (!Canvas || !Host)
	{
		return;
	}
	const FJamFight& Fight = Host->GetFight();
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	// The crack's old picture goes under the HUD: the bars and the texts stay readable over it.
	DrawCrack();
	const float Delta = GetWorld()->GetDeltaSeconds();
	JamBarRules::Track(BossBar, Boss.Health / FMath::Max(1.f, Boss.MaxHealth), Delta, BarSettings);
	JamBarRules::Track(HeroBar, Hero.Health / FMath::Max(1.f, Hero.MaxHealth), Delta, BarSettings);

	DrawBossBar(Boss);

	DrawHeroBar(Fight);
#if WITH_EDITOR
	DrawHint(JamTexts::DebugHint, 0);
#endif
	// One call over the head at a time: the shot about to land comes first.
	if (bParryPrompt)
	{
		DrawOverHero(Hero, JamTexts::ParryPrompt, HeroBarColor);
	}
	else if (UltimateCall == EJamUltimateCall::BannerAndPrompt)
	{
		DrawOverHero(Hero, JamTexts::UltimatePrompt, WinColor);
	}
	if (Fight.Phase == EJamPhase::Fight || Fight.Phase == EJamPhase::Stunned)
	{
		DrawBossMarker(Boss);
	}
	DrawPhase(Fight);
	if (IntroAlpha > 0.f)
	{
		DrawIntro();
	}
	if (bSubtitle)
	{
		DrawSubtitle();
	}
}

void AJamHUD::DrawHeroBar(const FJamFight& Fight)
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	if (bBars && bNeonBars)
	{
		DrawLoadout(Fight.Ledger, Hero.Ultimate.Meter, HudMargin * W, DrawNeonHero(Fight) - LoadoutLift);
		DrawUltimate(Fight, HudMargin * W, HeroBarWidth * W);
		return;
	}
	DrawHealthBar(HudMargin * W, HeroBarBottom * H, HeroBarWidth * W, BarHeight * H, HeroBar, HeroBarColor);
	const float HeroTextY = HeroBarBottom * H - HeroTextLift;
	DrawText(JamTexts::HeroHealth(FMath::CeilToInt32(Hero.Health)), FLinearColor::White, HudMargin * W, HeroTextY, GEngine->GetMediumFont(),
		HudTextScale);
	DrawLoadout(Fight.Ledger, Hero.Ultimate.Meter, HudMargin * W, HeroTextY - LoadoutLift);
	DrawUltimate(Fight, HudMargin * W, HeroBarWidth * W);
}

void AJamHUD::DrawSubtitle()
{
	UFont* Font = GEngine->GetMediumFont();
	const FString Name = FString(SubtitleSpeaker == EJamSide::Hero ? JamTexts::SpeakerHero : JamTexts::SpeakerBoss) + TEXT("  ");
	const FString Words = JamTexts::VoiceLine(SubtitleLine);
	float NameWidth = 0.f;
	float WordsWidth = 0.f;
	float Height = 0.f;
	GetTextSize(Name, NameWidth, Height, Font, SubtitleScale);
	GetTextSize(Words, WordsWidth, Height, Font, SubtitleScale);
	// Over the intro's band during the show, under the steal card in the fight.
	const float Y = (IntroAlpha > 0.f ? SubtitleIntroTop : SubtitleTop) * Canvas->ClipY;
	const float X = 0.5f * (Canvas->ClipX - NameWidth - WordsWidth);
	DrawRect(BarBackColor, X - SubtitlePadding, Y - 0.5f * SubtitlePadding, NameWidth + WordsWidth + 2.f * SubtitlePadding,
		Height + SubtitlePadding);
	DrawText(Name, SubtitleSpeaker == EJamSide::Hero ? HeroBarColor : BossNameColor, X, Y, Font, SubtitleScale);
	DrawText(Words, FLinearColor::White, X + NameWidth, Y, Font, SubtitleScale);
}

void AJamHUD::DrawBossMarker(const FJamFighter& Boss)
{
	const FVector Screen = Project(FVector(Boss.Pos.X, Boss.Pos.Y, BossMarkerHeight));
	// At the top the arrow stays under his health bar and its numbers, with a clear gap (T-047).
	float TextWidth = 0.f;
	float TextHeight = 0.f;
	GetTextSize(TEXT("0"), TextWidth, TextHeight, GEngine->GetMediumFont(), HudTextScale);
	const float BarTextBottom = (BossBarTop + BarHeight) * Canvas->ClipY + BossTextGap + TextHeight;
	const float TopInset = BarTextBottom + BossMarkerBarGap + 0.5f * BossMarkerSize;
	const FJamEdgeMarker Marker =
		JamMarkerRules::EdgeMarker(FVector2D(Screen.X, Screen.Y), FVector2D(Canvas->ClipX, Canvas->ClipY), BossMarkerMargin, TopInset);
	if (!Marker.bShow)
	{
		return;
	}
	// A wedge pointing at him, its tip on the marker: the colour of his health bar, so it reads as "the boss".
	const FVector2D Ahead = FVector2D(1.0, 0.0).GetRotated(Marker.Angle);
	const FVector2D Across(-Ahead.Y, Ahead.X);
	const FVector2D Tip = Marker.Pos + Ahead * (0.5 * BossMarkerSize);
	const FVector2D Back = Marker.Pos - Ahead * (0.5 * BossMarkerSize);
	FCanvasTriangleItem Wedge(Tip, Back + Across * (0.5 * BossMarkerSize), Back - Across * (0.5 * BossMarkerSize), GWhiteTexture);
	Wedge.SetColor(BossBarColor);
	Canvas->DrawItem(Wedge);
}

void AJamHUD::DrawUltimate(const FJamFight& Fight, float X, float Width)
{
	const float H = Canvas->ClipY;
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	// The neon Ultimate bar is DrawNeonHero's; the banner stays here for both looks.
	if (!(bBars && bNeonBars) && Fight.Ledger.Has(EJamSide::Hero, EJamMechanic::Ultimate))
	{
		const float Top = (HeroBarBottom + BarHeight) * H + UltimateBarGap;
		DrawBar(X, Top, Width, UltimateBarHeight * H, Hero.Ultimate.Meter, WinColor);
		if (bBars)
		{
			DrawText(JamTexts::UltimateBarLabel, WinColor, X + Width + UltimateBarGap, Top - 0.25f * UltimateBarHeight * H,
				GEngine->GetMediumFont(), 1.f);
		}
	}
	if (UltimateCall == EJamUltimateCall::None)
	{
		return;
	}
	// The banner breathes, so the eye catches it at the edge of the fight.
	const float Pulse = 0.5f + 0.5f * FMath::Sin(UE_TWO_PI * UltimatePulseHz * GetWorld()->GetRealTimeSeconds());
	const FLinearColor Color = FMath::Lerp(WinColor, FLinearColor::White, Pulse * UltimatePulseWhite);
	UFont* Font = GEngine->GetMediumFont();
	float TextWidth = 0.f;
	float TextHeight = 0.f;
	GetTextSize(JamTexts::UltimateBanner, TextWidth, TextHeight, Font, UltimateBannerScale);
	const float Y = UltimateBannerTop * H;
	DrawRect(BarBackColor, 0.5f * (Canvas->ClipX - TextWidth) - CardPadding, Y, TextWidth + 2.f * CardPadding, TextHeight);
	DrawRect(Color, 0.5f * (Canvas->ClipX - TextWidth) - CardPadding, Y + TextHeight, TextWidth + 2.f * CardPadding, CardOptionEdge);
	DrawCentered(JamTexts::UltimateBanner, Y, UltimateBannerScale, Color);
}

void AJamHUD::DrawOverHero(const FJamFighter& Hero, const FString& Text, const FLinearColor& Color)
{
	// Above the ronin's head, so the call is read where the eyes already are.
	const FVector Screen = Project(FVector(Hero.Pos.X, Hero.Pos.Y, OverHeroHeight));
	UFont* Font = GEngine->GetMediumFont();
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Text, Width, Height, Font, OverHeroScale);
	DrawRect(BarBackColor, Screen.X - 0.5f * Width - CardPadding * 0.5f, Screen.Y - Height, Width + CardPadding, Height);
	DrawText(Text, Color, Screen.X - 0.5f * Width, Screen.Y - Height, Font, OverHeroScale);
}
