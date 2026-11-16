#include "UI/JamHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "UI/JamHowToRules.h"
#include "UI/JamTexts.h"

// T-096: the HOW TO PLAY screen, a neon schematic drawn on the canvas (a generated picture garbles key labels): the
// whole keyboard dim, our keys lit in cyan, magenta leader lines to the labels, sized to the screen's height.

namespace
{
// Sizes on a 1080-line screen; the screen scales them by its height.
constexpr float ReferenceHeight = 1080.f;
constexpr float TitleTop = 0.07f;
constexpr float TitleScale = 3.2f;
constexpr float TaglineTop = 0.15f;
constexpr float TaglineScale = 1.3f;
constexpr float PromptTop = 0.88f;
constexpr float PromptScale = 1.6f;
constexpr float PromptHz = 1.2f;
constexpr float CapsScale = 1.5f;
constexpr float LabelScale = 1.15f;
constexpr float KeyCapScale = 1.f;
constexpr float LabelPad = 6.f;
constexpr float KeyLine = 1.5f;
constexpr float LitLine = 2.f;
constexpr float LeaderLine = 1.5f;
constexpr float GlowWidth = 3.f;
constexpr float GlowAlpha = 0.15f;
constexpr float DotSize = 5.f;
const FLinearColor Shade(0.01f, 0.012f, 0.03f, 0.94f);
const FLinearColor Cyan(0.2f, 0.9f, 1.0f);
const FLinearColor Pink(1.0f, 0.3f, 0.85f);
// The keys are filled over the leader lines, which show through them faintly: the caps stay readable.
const FLinearColor LitFill(0.03f, 0.13f, 0.18f, 0.88f);
const FLinearColor DimFill(0.02f, 0.025f, 0.05f, 0.85f);
const FLinearColor DimEdge(0.22f, 0.24f, 0.32f);
const FLinearColor DimCap(0.32f, 0.34f, 0.42f);
const FLinearColor LabelColor(0.86f, 0.88f, 0.94f);
const FLinearColor TaglineColor(0.6f, 0.62f, 0.72f);
} // namespace

void AJamHUD::DrawNeonLine(const FVector2D& From, const FVector2D& To, const FLinearColor& Color, float Thickness)
{
	FLinearColor Glow = Color;
	Glow.A = GlowAlpha;
	DrawLine(From.X, From.Y, To.X, To.Y, Glow, Thickness * GlowWidth);
	DrawLine(From.X, From.Y, To.X, To.Y, Color, Thickness);
}

void AJamHUD::DrawNeonBox(const FBox2D& Box, const FLinearColor& Color, float Thickness)
{
	const FVector2D A = Box.Min;
	const FVector2D B(Box.Max.X, Box.Min.Y);
	const FVector2D C = Box.Max;
	const FVector2D D(Box.Min.X, Box.Max.Y);
	DrawNeonLine(A, B, Color, Thickness);
	DrawNeonLine(B, C, Color, Thickness);
	DrawNeonLine(C, D, Color, Thickness);
	DrawNeonLine(D, A, Color, Thickness);
}

void AJamHUD::DrawHowToKeys(const FJamHowToLayout& Layout, float Scale)
{
	UFont* Font = GEngine->GetMediumFont();
	for (const FJamHowToKey& Key : Layout.Keys)
	{
		// The mouse's picture draws its own buttons.
		if (HowToMouse && Layout.Mouse.IsInside(Key.Box.GetCenter()))
		{
			continue;
		}
		const FVector2D Size = Key.Box.GetSize();
		DrawRect(Key.bLit ? LitFill : DimFill, Key.Box.Min.X, Key.Box.Min.Y, Size.X, Size.Y);
		if (Key.bLit)
		{
			DrawNeonBox(Key.Box, Cyan, LitLine * Scale);
		}
		else
		{
			DrawBox(Key.Box, DimEdge, KeyLine * Scale);
		}
		float CapW = 0.f;
		float CapH = 0.f;
		const float CapScale = KeyCapScale;
		GetTextSize(Key.Cap, CapW, CapH, Font, CapScale);
		// A cap wider than its key (LEFT CLICK on a mouse button) shrinks to fit.
		const float Fit = FMath::Min(1.f, 0.85f * Size.X / FMath::Max(1.f, CapW));
		const FVector2D Centre = Key.Box.GetCenter();
		DrawText(Key.Cap, Key.bLit ? Cyan : DimCap, Centre.X - 0.5f * CapW * Fit, Centre.Y - 0.5f * CapH * Fit, Font, CapScale * Fit);
	}
}

void AJamHUD::DrawHowToLines(const FJamHowToLayout& Layout, float Scale, bool bDots)
{
	const float Dot = DotSize * Scale;
	for (const TArray<TPair<FVector2D, FVector2D>>& Lines : Layout.Lines)
	{
		for (const TPair<FVector2D, FVector2D>& Line : Lines)
		{
			if (!bDots)
			{
				DrawNeonLine(Line.Key, Line.Value, Pink, LeaderLine * Scale);
				continue;
			}
			DrawRect(Pink, Line.Key.X - 0.5f * Dot, Line.Key.Y - 0.5f * Dot, Dot, Dot);
			DrawRect(Pink, Line.Value.X - 0.5f * Dot, Line.Value.Y - 0.5f * Dot, Dot, Dot);
		}
	}
}

void AJamHUD::DrawBox(const FBox2D& Box, const FLinearColor& Color, float Thickness)
{
	DrawLine(Box.Min.X, Box.Min.Y, Box.Max.X, Box.Min.Y, Color, Thickness);
	DrawLine(Box.Max.X, Box.Min.Y, Box.Max.X, Box.Max.Y, Color, Thickness);
	DrawLine(Box.Max.X, Box.Max.Y, Box.Min.X, Box.Max.Y, Color, Thickness);
	DrawLine(Box.Min.X, Box.Max.Y, Box.Min.X, Box.Min.Y, Color, Thickness);
}

void AJamHUD::DrawHowToMouse(const FJamHowToLayout& Layout)
{
	if (!HowToMouse)
	{
		HowToMouse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/HowTo/T_HowToMouse.T_HowToMouse"));
	}
	const FVector2D Size = Layout.Mouse.GetSize();
	if (!HowToMouse)
	{
		DrawNeonBox(Layout.Mouse, Cyan, LitLine);
		return;
	}
	// Added on: the picture's black ground vanishes into the shade, its neon glows over the lines under it.
	DrawTexture(
		HowToMouse, Layout.Mouse.Min.X, Layout.Mouse.Min.Y, Size.X, Size.Y, 0.f, 0.f, 1.f, 1.f, FLinearColor::White, BLEND_Additive);
}

void AJamHUD::DrawHowTo()
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float Scale = H / ReferenceHeight;
	UFont* Font = GEngine->GetMediumFont();
	DrawRect(Shade, 0.f, 0.f, W, H);

	const TArray<FJamHowToCallout> Callouts = JamHowToRules::Callouts(HowToExplosiveEvery);
	TArray<FVector2D> Sizes;
	for (const FJamHowToCallout& Callout : Callouts)
	{
		float CapsW = 0.f;
		float CapsH = 0.f;
		float LabelW = 0.f;
		float LabelH = 0.f;
		GetTextSize(Callout.Caps, CapsW, CapsH, Font, CapsScale);
		GetTextSize(Callout.Label, LabelW, LabelH, Font, LabelScale);
		Sizes.Add(FVector2D(FMath::Max(CapsW, LabelW) + 2.f * LabelPad * Scale, CapsH + LabelH + 2.f * LabelPad * Scale));
	}
	const FJamHowToLayout Layout = JamHowToRules::Layout(FVector2D(W, H), Callouts, Sizes);
	DrawHowToLines(Layout, Scale, false);
	DrawHowToKeys(Layout, Scale);
	DrawHowToMouse(Layout);
	DrawHowToLines(Layout, Scale, true);
	for (int32 Index = 0; Index < Callouts.Num(); ++Index)
	{
		const FBox2D& Box = Layout.Labels[Index];
		float CapsW = 0.f;
		float CapsH = 0.f;
		GetTextSize(Callouts[Index].Caps, CapsW, CapsH, Font, CapsScale);
		const float Pad = LabelPad * Scale;
		DrawText(Callouts[Index].Caps, Pink, Box.Min.X + Pad, Box.Min.Y + Pad, Font, CapsScale);
		DrawText(Callouts[Index].Label, LabelColor, Box.Min.X + Pad, Box.Min.Y + Pad + CapsH, Font, LabelScale);
	}

	DrawCentered(JamTexts::HowToTitle, TitleTop * H, TitleScale, Cyan);
	DrawCentered(JamTexts::HowToTagline, TaglineTop * H, TaglineScale, TaglineColor);
	const FString After = HowToSecondsLeft >= 0 ? FString(JamTexts::HowToStartsIn).Replace(TEXT("%d"), *FString::FromInt(HowToSecondsLeft))
												: FString(JamTexts::HowToWaits);
	FLinearColor Prompt = Pink;
	Prompt.A = 0.85f + 0.15f * FMath::Cos(2.f * PI * PromptHz * GetWorld()->GetRealTimeSeconds());
	DrawCentered(FString::Printf(TEXT("%s   -   %s"), JamTexts::HowToPress, *After), PromptTop * H, PromptScale, Prompt);
}
