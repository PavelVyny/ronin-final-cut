#include "UI/JamHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "UI/JamHUDPixels.h"
#include "UI/JamComicRules.h"
#include "UI/JamTexts.h"

namespace
{
// T-093: the opening comic's lettering, as fractions of the screen or in pixels.
constexpr JamHudPixels::FPixels GutterWidth{ 12.f };
constexpr int32 BalloonChars = 26;
constexpr float BalloonScale = 1.7f;
constexpr JamHudPixels::FPixels BalloonPadding{ 16.f };
constexpr JamHudPixels::FPixels BalloonBorder{ 4.f };
constexpr JamHudPixels::FPixels BalloonTail{ 26.f };
constexpr float CaptionLeft = 0.04f;
constexpr float CaptionTop = 0.05f;
constexpr float CaptionScale = 1.6f;
constexpr JamHudPixels::FPixels CaptionPadding{ 12.f };
constexpr float SoundScale = 4.5f;
constexpr JamHudPixels::FPixels SoundOutline{ 4.f };
constexpr float SkipScale = 1.1f;
// Where each panel's sound word lands, as fractions of the screen; unused where the panel has none.
const FVector2D SoundAt[] = { FVector2D(0.5, 0.5), FVector2D(0.74, 0.2), FVector2D(0.5, 0.5), FVector2D(0.62, 0.8), FVector2D(0.5, 0.5) };
static_assert(UE_ARRAY_COUNT(SoundAt) == JamComicRules::PanelCount, "one place per panel");
const FLinearColor Ink(0.02f, 0.02f, 0.04f);
const FLinearColor Paper(0.97f, 0.96f, 0.92f);
const FLinearColor CaptionYellow(1.f, 0.86f, 0.3f);
const FLinearColor SoundGold(1.f, 0.8f, 0.15f);
const FLinearColor SkipColor(0.7f, 0.7f, 0.8f, 0.8f);
} // namespace

void AJamHUD::LoadComic()
{
	ComicPictures.Reset();
	for (int32 Index = 1; Index <= JamComicRules::PanelCount; ++Index)
	{
		ComicPictures.Add(LoadObject<UTexture2D>(nullptr, *FString::Printf(TEXT("/Game/UI/Comic/T_Comic_%d.T_Comic_%d"), Index, Index)));
	}
}

void AJamHUD::DrawComicPanel(int32 Index, float Age, float Offset, const FBox2D& Frame)
{
	UTexture2D* Picture = ComicPictures.IsValidIndex(Index) ? ComicPictures[Index].Get() : nullptr;
	const FVector2D FrameSize = Frame.GetSize();
	const float Shown = FrameSize.X - Offset;
	if (!Picture || Shown <= 0.f)
	{
		return;
	}
	const FBox2D Crop = JamComicRules::Crop(JamComicRules::PanelOf(Index), Age, FrameSize.X / FMath::Max(FrameSize.Y, 1.0));
	const FVector2D Size = Crop.GetSize();
	// Only the part already inside the frame: the panel's left side, as wide as it has slid in.
	const float Part = Shown / FMath::Max(FrameSize.X, 1.0);
	const float Left = Frame.Min.X + Offset;
	DrawTexture(
		Picture, Left, Frame.Min.Y, Shown, FrameSize.Y, Crop.Min.X, Crop.Min.Y, Part * Size.X, Size.Y, FLinearColor::White, BLEND_Opaque);
	// The gutter between two panels, on the edge of the one sliding in.
	if (Offset > 0.f)
	{
		DrawRect(Ink, Left - GutterWidth, Frame.Min.Y, GutterWidth, FrameSize.Y);
	}
}

void AJamHUD::DrawBalloon(const FString& Words, const FVector2D& At, const FBox2D& Frame)
{
	UFont* Font = GEngine->GetMediumFont();
	const TArray<FString> Rows = JamComicRules::Wrap(Words.ToUpper(), BalloonChars);
	float Width = 0.f;
	float RowHeight = 0.f;
	for (const FString& Row : Rows)
	{
		float RowWidth = 0.f;
		GetTextSize(Row, RowWidth, RowHeight, Font, BalloonScale);
		Width = FMath::Max(Width, RowWidth);
	}
	const float BoxW = Width + 2.f * BalloonPadding;
	const float BoxH = Rows.Num() * RowHeight + 2.f * BalloonPadding;
	const FVector2D Centre = Frame.Min + At * Frame.GetSize();
	const float X = Centre.X - 0.5f * BoxW;
	const float Y = Centre.Y - 0.5f * BoxH;
	// The tail first, so the box's border closes over its root.
	const FVector2D Root(X + 0.5f * BoxW, Y + BoxH);
	FCanvasTriangleItem Tail(Root + FVector2D(-0.5f * BalloonTail - BalloonBorder, -BalloonBorder),
		Root + FVector2D(0.5f * BalloonTail + BalloonBorder, -BalloonBorder), Root + FVector2D(0.f, BalloonTail + BalloonBorder),
		GWhiteTexture);
	Tail.SetColor(Ink);
	Canvas->DrawItem(Tail);
	FCanvasTriangleItem Fill(Root + FVector2D(-0.5f * BalloonTail, -BalloonBorder), Root + FVector2D(0.5f * BalloonTail, -BalloonBorder),
		Root + FVector2D(0.f, BalloonTail), GWhiteTexture);
	Fill.SetColor(Paper);
	DrawRect(Ink, X - BalloonBorder, Y - BalloonBorder, BoxW + 2.f * BalloonBorder, BoxH + 2.f * BalloonBorder);
	DrawRect(Paper, X, Y, BoxW, BoxH);
	Canvas->DrawItem(Fill);
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		float RowWidth = 0.f;
		float Unused = 0.f;
		GetTextSize(Rows[Index], RowWidth, Unused, Font, BalloonScale);
		DrawText(Rows[Index], Ink, X + 0.5f * (BoxW - RowWidth), Y + BalloonPadding + Index * RowHeight, Font, BalloonScale);
	}
}

void AJamHUD::DrawComicWords(int32 Index, const FBox2D& Frame)
{
	UFont* Font = GEngine->GetMediumFont();
	const float W = Frame.GetSize().X;
	const float H = Frame.GetSize().Y;
	const FString Caption = JamTexts::ComicCaptions[Index];
	if (!Caption.IsEmpty())
	{
		float Width = 0.f;
		float Height = 0.f;
		GetTextSize(Caption, Width, Height, Font, CaptionScale);
		const float X = Frame.Min.X + CaptionLeft * W;
		const float Y = Frame.Min.Y + CaptionTop * H;
		DrawRect(Ink, X - BalloonBorder, Y - BalloonBorder, Width + 2.f * (CaptionPadding + BalloonBorder),
			Height + 2.f * (CaptionPadding + BalloonBorder));
		DrawRect(CaptionYellow, X, Y, Width + 2.f * CaptionPadding, Height + 2.f * CaptionPadding);
		DrawText(Caption, Ink, X + CaptionPadding, Y + CaptionPadding, Font, CaptionScale);
	}
	const FString Sound = JamTexts::ComicSounds[Index];
	if (!Sound.IsEmpty())
	{
		const float X = Frame.Min.X + SoundAt[Index].X * W;
		const float Y = Frame.Min.Y + SoundAt[Index].Y * H;
		for (const FVector2D Offset : { FVector2D(-1, -1), FVector2D(1, -1), FVector2D(-1, 1), FVector2D(1, 1) })
		{
			DrawText(Sound, Ink, X + SoundOutline * Offset.X, Y + SoundOutline * Offset.Y, Font, SoundScale);
		}
		DrawText(Sound, SoundGold, X, Y, Font, SoundScale);
	}
}

void AJamHUD::DrawComic()
{
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	// The black under everything is also the bars at the sides of a screen wider than the comic (T-094).
	DrawRect(Ink, 0.f, 0.f, W, H);
	if (ComicPictures.Num() != JamComicRules::PanelCount)
	{
		LoadComic();
	}
	const FBox2D Frame = JamComicRules::Frame(FVector2D(W, H));
	const float Slide = JamComicRules::SlidesIn(ComicPanel) ? JamComicRules::SlideIn(ComicPanelAge) : 1.f;
	if (Slide < 1.f && ComicPanel > 0)
	{
		// The last panel stays where its move ended while the new one slides over it.
		DrawComicPanel(ComicPanel - 1, JamComicRules::PanelOf(ComicPanel - 1).Seconds, 0.f, Frame);
	}
	DrawComicPanel(ComicPanel, ComicPanelAge, (1.f - Slide) * Frame.GetSize().X, Frame);
	// The session-5s playtest: the first panel fades up from black instead of sliding in.
	const float Fade = JamComicRules::FadeIn(ComicPanel, ComicPanelAge);
	if (Fade < 1.f)
	{
		DrawRect(FLinearColor(Ink.R, Ink.G, Ink.B, 1.f - Fade), 0.f, 0.f, W, H);
	}
	if (Slide < 1.f)
	{
		return;
	}
	DrawComicWords(ComicPanel, Frame);
	if (bComicBalloon)
	{
		DrawBalloon(JamTexts::VoiceLine(static_cast<int32>(ComicLine)), JamComicRules::BalloonAt(ComicLine), Frame);
	}
	float Width = 0.f;
	float Height = 0.f;
	UFont* Font = GEngine->GetMediumFont();
	GetTextSize(JamTexts::ComicSkip, Width, Height, Font, SkipScale);
	DrawText(JamTexts::ComicSkip, SkipColor, Frame.Max.X - Width - CaptionPadding, Frame.Max.Y - Height - CaptionPadding, Font, SkipScale);
}
