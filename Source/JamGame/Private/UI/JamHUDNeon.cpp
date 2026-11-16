// T-104: the full art's neon bars (Submission/hud_mock_neon.png, drawn by Submission/hud_mock_bars.py): parallelograms
// on a dark plate with scanlines, the fill in segments with a bright top edge, a neon frame with a soft glow. Drawn on the
// canvas, so they stay sharp at any resolution. From the first cut on the plain bars of JamHUD.cpp come back.
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GlobalRenderResources.h"
#include "Sim/JamFightSubsystem.h"
#include "UI/JamHUD.h"
#include "UI/JamHUDPixels.h"
#include "UI/JamNeonBarRules.h"
#include "UI/JamTexts.h"

namespace
{
// The mock's layout, as shares of the screen it was drawn on (a 2000x836 preview of a 3440x1440 frame).
constexpr float NeonBossFrom = 0.26f;
constexpr float NeonBossTo = 0.625f;
constexpr float NeonBossTop = 0.048f;
constexpr float NeonBossBottom = 0.067f;
constexpr float NeonHeroFrom = 0.035f;
constexpr float NeonHeroTo = 0.25f;
constexpr float NeonHeroTop = 0.9354f;
constexpr float NeonHeroBottom = 0.9522f;
constexpr float NeonUltimateTo = 0.22f;
constexpr float NeonUltimateTop = 0.9641f;
constexpr float NeonUltimateBottom = 0.9713f;
constexpr int32 NeonBossSegments = 40;
constexpr int32 NeonHeroSegments = 20;
constexpr int32 NeonUltimateSegments = 24;
// One slope for every shape of every bar (x per pixel of height, from the bar's bottom line): the author's
// play-through saw the frame and the segments lean at different angles when each got the same pixel offset.
constexpr float NeonSlope = 0.6f;
// Sizes in pixels of the 1080-line screen (the mock's pixels times 1.29).
constexpr JamHudPixels::FPixels NeonPlatePad{ 5.f };
constexpr JamHudPixels::FPixels NeonSegmentGap{ 4.f };
constexpr JamHudPixels::FPixels NeonScanlineStep{ 3.f };
constexpr JamHudPixels::FPixels NeonFrameWidth{ 2.f };
constexpr JamHudPixels::FPixels NeonTopEdgeWidth{ 1.5f };
constexpr JamHudPixels::FPixels NeonGlowWidth{ 7.f };
constexpr JamHudPixels::FPixels NeonCutReach{ 11.f };
constexpr JamHudPixels::FPixels NeonCutWidth{ 2.5f };
constexpr JamHudPixels::FPixels NeonCutLabelGap{ 3.f };
constexpr JamHudPixels::FPixels NeonTabWidth{ 250.f };
constexpr JamHudPixels::FPixels NeonTabHeight{ 28.f };
constexpr JamHudPixels::FPixels NeonTabLift{ 10.f };
constexpr JamHudPixels::FPixels NeonTabTextInset{ 18.f };
constexpr JamHudPixels::FPixels NeonNumberGap{ 22.f };
constexpr JamHudPixels::FPixels NeonHeroTextGap{ 6.f };
constexpr JamHudPixels::FPixels NeonLabelGap{ 14.f };
constexpr float NeonNameScale = 1.3f;
constexpr float NeonNumberScale = 1.3f;
constexpr float NeonSmallScale = 0.9f;
constexpr float NeonUltimatePulseHz = 1.5f;
const FLinearColor NeonPink(FColor(255, 70, 110));
const FLinearColor NeonCyan(FColor(40, 230, 255));
const FLinearColor NeonMagenta(FColor(255, 60, 220));
const FLinearColor NeonPlateColor(FColor(6, 8, 16, 215));
const FLinearColor NeonNameColor(FColor(255, 225, 230));
const FLinearColor NeonHeroNameColor(FColor(220, 250, 255));
const FLinearColor NeonChunkWhite(1.f, 1.f, 1.f, 0.9f);
const FLinearColor NeonHealPreview(0.55f, 1.f, 0.6f, 0.7f);

/** The four corners of a slanted shape on a bar whose bottom line is BottomY (JamNeonBarRules::Slanted). */
struct FNeonPara
{
	FVector2D Corners[4];

	FNeonPara(float X0, float Y0, float X1, float Y1, float BottomY, float Slope)
	{
		const FJamSlantQuad Quad = JamNeonBarRules::Slanted(X0, Y0, X1, Y1, BottomY, Slope);
		for (int32 Index = 0; Index < 4; ++Index)
		{
			Corners[Index] = Quad.Corners[Index];
		}
	}
};

FLinearColor NeonWithAlpha(const FLinearColor& Color, float Alpha)
{
	return FLinearColor(Color.R, Color.G, Color.B, Color.A * Alpha);
}

void NeonFill(FCanvas& Canvas, const FNeonPara& P, const FLinearColor& Color, ESimpleElementBlendMode Blend = SE_BLEND_Translucent)
{
	for (int32 Second = 1; Second <= 2; ++Second)
	{
		FCanvasTriangleItem Triangle(P.Corners[0], P.Corners[Second], P.Corners[Second + 1], GWhiteTexture);
		Triangle.SetColor(Color);
		Triangle.BlendMode = Blend;
		Canvas.DrawItem(Triangle);
	}
}

/** A line as a thin quad: the canvas' line items drew a faint colour opaque (premultiplied), the triangles blend. */
void NeonLine(FCanvas& Canvas, const FVector2D& A, const FVector2D& B, const FLinearColor& Color, float Width,
	ESimpleElementBlendMode Blend = SE_BLEND_Translucent)
{
	const FVector2D Along = (B - A).GetSafeNormal();
	const FVector2D Half = FVector2D(-Along.Y, Along.X) * (0.5f * FMath::Max(Width, 1.f));
	FNeonPara Quad(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	Quad.Corners[0] = A + Half;
	Quad.Corners[1] = B + Half;
	Quad.Corners[2] = B - Half;
	Quad.Corners[3] = A - Half;
	NeonFill(Canvas, Quad, Color, Blend);
}

void NeonOutline(
	FCanvas& Canvas, const FNeonPara& P, const FLinearColor& Color, float Width, ESimpleElementBlendMode Blend = SE_BLEND_Translucent)
{
	for (int32 Index = 0; Index < 4; ++Index)
	{
		NeonLine(Canvas, P.Corners[Index], P.Corners[(Index + 1) % 4], Color, Width, Blend);
	}
}

/** The part of Segment from From to To (shares of the bar), if any, filled in Color. */
void NeonSegmentPart(FCanvas& Canvas, const FJamBarSegment& Segment, float From, float To, float X0, float Length, float Y0, float Y1,
	float Slope, const FLinearColor& Color)
{
	const float End = FMath::Min(Segment.To, To);
	if (End > From)
	{
		NeonFill(Canvas, FNeonPara(X0 + Length * From, Y0, X0 + Length * End, Y1, Y1, Slope), Color);
	}
}

/** The neon frame: a soft glow of wide faint passes under a thin bright line. */
void NeonFrame(FCanvas& Canvas, const FNeonPara& P, const FLinearColor& Color, float Alpha, float Glow)
{
	// Translucent, not additive: the canvas' additive blend ignores alpha and floods the plate. Each pass is centred on
	// the frame, so at most half of the widest one reaches into the plate's pad.
	const float Passes[][2] = { { 1.4f, 0.05f }, { 0.9f, 0.09f }, { 0.5f, 0.16f } };
	for (const auto& Pass : Passes)
	{
		NeonOutline(Canvas, P, NeonWithAlpha(Color, FMath::Min(1.f, Alpha * Glow * Pass[1])), NeonGlowWidth * Pass[0]);
	}
	NeonOutline(Canvas, P, NeonWithAlpha(Color, Alpha), NeonFrameWidth);
}
} // namespace

void AJamHUD::DrawNeonBar(const FJamNeonBar& Bar)
{
	FCanvas& C = *Canvas->Canvas;
	const float X0 = Bar.Box.Min.X;
	const float Y0 = Bar.Box.Min.Y;
	const float X1 = Bar.Box.Max.X;
	const float Y1 = Bar.Box.Max.Y;
	const float Length = X1 - X0;
	const float Slope = Bar.Slope;
	const FNeonPara Plate(X0 - NeonPlatePad, Y0 - NeonPlatePad, X1 + NeonPlatePad, Y1 + NeonPlatePad, Y1, Slope);
	NeonFill(C, Plate, NeonWithAlpha(NeonPlateColor, Bar.Alpha));
	for (float Y = Y0; Y < Y1; Y += NeonScanlineStep)
	{
		const float Shift = (Y1 - Y) * Slope;
		NeonLine(C, FVector2D(X0 + Shift, Y), FVector2D(X1 + Shift, Y), FLinearColor(1.f, 1.f, 1.f, 0.04f * Bar.Alpha), 1.f);
	}
	// The lit part's own glow under the segments.
	NeonFill(C, FNeonPara(X0 - 0.5f * NeonPlatePad, Y0 - 0.5f * NeonPlatePad, X0 + Length * Bar.Fill, Y1 + 0.5f * NeonPlatePad, Y1, Slope),
		NeonWithAlpha(Bar.Color, FMath::Min(1.f, 0.12f * Bar.Alpha * Bar.Glow)));
	const FLinearColor Lit = FMath::Lerp(Bar.Color, FLinearColor::White, Bar.Whiten);
	for (const FJamBarSegment& Segment : JamNeonBarRules::Segments(Bar.Segments, NeonSegmentGap / FMath::Max(1.f, Length), Bar.Fill))
	{
		const float From = X0 + Length * Segment.From;
		if (!Segment.IsLit())
		{
			NeonOutline(C, FNeonPara(From, Y0, X0 + Length * Segment.To, Y1, Y1, Slope), NeonWithAlpha(Bar.Color, 0.25f * Bar.Alpha), 1.f);
		}
		else
		{
			const float To = X0 + Length * Segment.FillTo;
			NeonFill(C, FNeonPara(From, Y0, To, Y1, Y1, Slope), NeonWithAlpha(Lit, 0.92f * Bar.Alpha));
			const float TopShift = (Y1 - Y0) * Slope;
			NeonLine(C, FVector2D(From + TopShift, Y0), FVector2D(To + TopShift, Y0), FLinearColor(1.f, 1.f, 1.f, 0.67f * Bar.Alpha),
				NeonTopEdgeWidth);
		}
		// T-053's chunk of health just lost in bright white, and a heal's light preview, cut like the fill.
		NeonSegmentPart(
			C, Segment, FMath::Max(Segment.From, Bar.Fill), Bar.Chunk, X0, Length, Y0, Y1, Slope, NeonWithAlpha(NeonChunkWhite, Bar.Alpha));
		NeonSegmentPart(C, Segment, FMath::Max(Segment.From, Bar.Fill), Bar.Preview, X0, Length, Y0, Y1, Slope,
			NeonWithAlpha(NeonHealPreview, Bar.Alpha));
	}
	NeonFrame(C, Plate, Bar.Color, Bar.Alpha, Bar.Glow);
}

void AJamHUD::DrawNeonBoss(const FJamFighter& Boss, const FJamBalanceData& Balance)
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	FJamNeonBar Bar;
	Bar.Box = FBox2D(FVector2D(NeonBossFrom * W, NeonBossTop * H), FVector2D(NeonBossTo * W, NeonBossBottom * H));
	Bar.Slope = NeonSlope;
	Bar.Color = NeonPink;
	Bar.Segments = NeonBossSegments;
	Bar.Fill = BossBar.Fill;
	Bar.Chunk = BossBar.Chunk;
	Bar.Preview = BossBar.Preview;
	Bar.Alpha = BossBarAlpha;
	DrawNeonBar(Bar);
	UFont* Font = GEngine->GetMediumFont();
	const FLinearColor Fade(1.f, 1.f, 1.f, BossBarAlpha);

	// His name on its own slanted tab over the bar's left end.
	const float TabBottom = Bar.Box.Min.Y - NeonTabLift;
	const float Bottom = Bar.Box.Max.Y;
	const FNeonPara Tab(
		Bar.Box.Min.X - NeonPlatePad, TabBottom - NeonTabHeight, Bar.Box.Min.X + NeonTabWidth, TabBottom, Bottom, NeonSlope);
	NeonFill(*Canvas->Canvas, Tab, NeonWithAlpha(NeonPlateColor, BossBarAlpha));
	NeonFrame(*Canvas->Canvas, Tab, NeonPink, BossBarAlpha, 1.f);
	float TextW = 0.f;
	float TextH = 0.f;
	GetTextSize(JamTexts::BossName, TextW, TextH, Font, NeonNameScale);
	DrawText(JamTexts::BossName, NeonNameColor * Fade, Bar.Box.Min.X + (Bottom - TabBottom) * NeonSlope + NeonTabTextInset,
		TabBottom - 0.5f * (NeonTabHeight + TextH), Font, NeonNameScale);

	// Where he steals a feature: white cuts across the bar, the share under each.
	for (const float Mark : JamNeonBarRules::CutMarks(Balance))
	{
		const float X = Bar.Box.Min.X + Bar.Box.GetSize().X * Mark;
		const float CutTop = Bar.Box.Min.Y - NeonCutReach;
		const float CutBottom = Bottom + NeonCutReach;
		NeonLine(*Canvas->Canvas, FVector2D(X + (Bottom - CutTop) * NeonSlope, CutTop),
			FVector2D(X + (Bottom - CutBottom) * NeonSlope, CutBottom), FLinearColor(1.f, 1.f, 1.f, 0.9f * BossBarAlpha), NeonCutWidth);
		const FString Label = FString::Printf(TEXT("%d%%"), FMath::RoundToInt32(Mark * 100.f));
		GetTextSize(Label, TextW, TextH, Font, NeonSmallScale);
		DrawText(Label, FLinearColor(1.f, 1.f, 1.f, 0.67f * BossBarAlpha), X - 0.5f * TextW, Bar.Box.Max.Y + NeonCutReach + NeonCutLabelGap,
			Font, NeonSmallScale);
	}

	// The number bright, the whole dim, right of the bar.
	const FString Now = FString::FromInt(FMath::CeilToInt32(Boss.Health));
	GetTextSize(Now, TextW, TextH, Font, NeonNumberScale);
	const float NumberX = Bar.Box.Max.X + (Bottom - Bar.Box.Min.Y + NeonPlatePad) * NeonSlope + NeonNumberGap;
	const float NumberY = Bar.Box.GetCenter().Y - 0.5f * TextH;
	DrawText(Now, FLinearColor(1.f, 1.f, 1.f, 0.95f * BossBarAlpha), NumberX, NumberY, Font, NeonNumberScale);
	DrawText(FString::Printf(TEXT("/%d"), FMath::CeilToInt32(Boss.MaxHealth)), FLinearColor(1.f, 1.f, 1.f, 0.55f * BossBarAlpha),
		NumberX + TextW + 0.25f * NeonNumberGap, NumberY + 0.2f * TextH, Font, NeonSmallScale);
}

float AJamHUD::DrawNeonHero(const FJamFight& Fight)
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	FJamNeonBar Bar;
	Bar.Box = FBox2D(FVector2D(NeonHeroFrom * W, NeonHeroTop * H), FVector2D(NeonHeroTo * W, NeonHeroBottom * H));
	Bar.Slope = NeonSlope;
	Bar.Color = NeonCyan;
	Bar.Segments = NeonHeroSegments;
	Bar.Fill = HeroBar.Fill;
	Bar.Chunk = HeroBar.Chunk;
	Bar.Preview = HeroBar.Preview;
	DrawNeonBar(Bar);
	UFont* Font = GEngine->GetMediumFont();
	float TextW = 0.f;
	float TextH = 0.f;
	GetTextSize(JamTexts::SpeakerHero, TextW, TextH, Font, NeonNameScale);
	const float TextY = Bar.Box.Min.Y - NeonPlatePad - NeonHeroTextGap - TextH;
	DrawText(JamTexts::SpeakerHero, NeonHeroNameColor, Bar.Box.Min.X, TextY, Font, NeonNameScale);
	DrawText(FString::FromInt(FMath::CeilToInt32(Hero.Health)), FLinearColor(1.f, 1.f, 1.f, 0.95f), Bar.Box.Min.X + TextW + NeonNumberGap,
		TextY, Font, NeonNumberScale);

	if (Fight.Ledger.Has(EJamSide::Hero, EJamMechanic::Ultimate))
	{
		const float Meter = Hero.Ultimate.Meter;
		const float Pulse = JamNeonBarRules::UltimatePulse(Meter, GetWorld()->GetRealTimeSeconds(), NeonUltimatePulseHz);
		FJamNeonBar Ultimate;
		Ultimate.Box = FBox2D(FVector2D(NeonHeroFrom * W, NeonUltimateTop * H), FVector2D(NeonUltimateTo * W, NeonUltimateBottom * H));
		Ultimate.Slope = NeonSlope;
		Ultimate.Color = NeonMagenta;
		Ultimate.Segments = NeonUltimateSegments;
		Ultimate.Fill = Ultimate.Chunk = Ultimate.Preview = FMath::Clamp(Meter, 0.f, 1.f);
		Ultimate.Whiten = 0.6f * Pulse;
		Ultimate.Glow = 1.f + 2.f * Pulse;
		DrawNeonBar(Ultimate);
		GetTextSize(JamTexts::UltimateBarLabel, TextW, TextH, Font, NeonSmallScale);
		DrawText(JamTexts::UltimateBarLabel, FMath::Lerp(NeonMagenta, FLinearColor::White, 0.6f * Pulse), Ultimate.Box.Max.X + NeonLabelGap,
			Ultimate.Box.GetCenter().Y - 0.5f * TextH, Font, NeonSmallScale);
	}
	return TextY;
}
