#include "UI/JamComicRules.h"

namespace
{
FJamComicPanel MakePanel(float Seconds, float Aspect, FVector2D From, float FromZoom, FVector2D To, float ToZoom, TArray<EJamLine> Lines,
	EJamComicSound Sound = EJamComicSound::None)
{
	FJamComicPanel Panel;
	Panel.Seconds = Seconds;
	Panel.Aspect = Aspect;
	Panel.From = From;
	Panel.FromZoom = FromZoom;
	Panel.To = To;
	Panel.ToZoom = ToZoom;
	Panel.Lines = MoveTemp(Lines);
	Panel.Sound = Sound;
	return Panel;
}

// The five pictures of Art/Comic, in order. The moves follow COMIC_SCRIPT.md: a slow push in on the dev at his desk
// (the session-5s playtest: it went by too fast and jerked sideways), a punch-in on the call, a pan from his face to the
// dev's, a pull back from the fist, a push to the reflection's eyes.
const FJamComicPanel Panels[] = {
	MakePanel(2.f, 1.f, FVector2D(0.5, 0.52), 1.f, FVector2D(0.5, 0.47), 1.15f, {}, EJamComicSound::Typing),
	MakePanel(JamComicRules::RingSeconds + 0.5f, 1.5f, FVector2D(0.55, 0.52), 1.f, FVector2D(0.6, 0.48), 1.3f, {}, EJamComicSound::Ring),
	MakePanel(5.f, 1.5f, FVector2D(0.28, 0.48), 1.3f, FVector2D(0.72, 0.48), 1.3f, { EJamLine::ComicBoss1, EJamLine::ComicHero1 }),
	MakePanel(4.f, 1.5f, FVector2D(0.58, 0.42), 1.7f, FVector2D(0.5, 0.5), 1.f, { EJamLine::ComicBoss2 }, EJamComicSound::Shatter),
	MakePanel(3.5f, 1.5f, FVector2D(0.5, 0.5), 1.f, FVector2D(0.27, 0.45), 1.8f, { EJamLine::ComicHero2 }),
};
static_assert(UE_ARRAY_COUNT(Panels) == JamComicRules::PanelCount, "one picture per panel");
} // namespace

const FJamComicPanel& JamComicRules::PanelOf(int32 Index)
{
	return Panels[FMath::Clamp(Index, 0, PanelCount - 1)];
}

float JamComicRules::SoundVolume(EJamComicSound Sound)
{
	switch (Sound)
	{
		case EJamComicSound::None:
			return 0.f;
		case EJamComicSound::Shatter:
			// Its burst peaks at full scale: at 0.6 it stays under the Publisher's boosted line and still cuts through.
			return 0.6f;
		default:
			return 1.f;
	}
}

bool JamComicRules::IsPanelDone(int32 Index, float PanelAge, bool bSpeaking)
{
	return PanelAge >= PanelOf(Index).Seconds && !bSpeaking;
}

FBox2D JamComicRules::Crop(const FJamComicPanel& Panel, float PanelAge, float ScreenAspect)
{
	const float T = Panel.Seconds > 0.f ? FMath::SmoothStep(0.f, 1.f, FMath::Clamp(PanelAge / Panel.Seconds, 0.f, 1.f)) : 1.f;
	const double Zoom = FMath::Max(1.0, FMath::Lerp(static_cast<double>(Panel.FromZoom), static_cast<double>(Panel.ToZoom), T));
	// The largest part of the picture with the screen's shape: its full width when the screen is wider, else its height.
	const double Aspect = FMath::Max(Panel.Aspect, 0.01f);
	const double Screen = FMath::Max(ScreenAspect, 0.01f);
	FVector2D Size = Screen >= Aspect ? FVector2D(1.0, Aspect / Screen) : FVector2D(Screen / Aspect, 1.0);
	Size /= Zoom;
	const FVector2D Want = FMath::Lerp(Panel.From, Panel.To, static_cast<double>(T));
	const FVector2D Centre(FMath::Clamp(Want.X, 0.5 * Size.X, 1.0 - 0.5 * Size.X), FMath::Clamp(Want.Y, 0.5 * Size.Y, 1.0 - 0.5 * Size.Y));
	return FBox2D(Centre - 0.5 * Size, Centre + 0.5 * Size);
}

float JamComicRules::SlideIn(float PanelAge)
{
	const float T = FMath::Clamp(PanelAge / SlideSeconds, 0.f, 1.f);
	// Fast in, eased to a stop, like a page slapped down.
	return 1.f - FMath::Square(1.f - T);
}

bool JamComicRules::MaySkip(float Age)
{
	return Age >= SkipAfter;
}

TArray<FString> JamComicRules::Wrap(const FString& Text, int32 MaxChars)
{
	TArray<FString> Words;
	Text.ParseIntoArrayWS(Words);
	TArray<FString> Rows;
	FString Row;
	for (const FString& Word : Words)
	{
		if (!Row.IsEmpty() && Row.Len() + 1 + Word.Len() > MaxChars)
		{
			Rows.Add(Row);
			Row.Reset();
		}
		Row += (Row.IsEmpty() ? TEXT("") : TEXT(" ")) + Word;
	}
	if (!Row.IsEmpty())
	{
		Rows.Add(Row);
	}
	return Rows;
}

FVector2D JamComicRules::BalloonAt(EJamLine Line)
{
	switch (Line)
	{
		case EJamLine::ComicBoss1:
			return FVector2D(0.27, 0.14);
		case EJamLine::ComicHero1:
			return FVector2D(0.73, 0.14);
		case EJamLine::ComicBoss2:
			return FVector2D(0.32, 0.14);
		case EJamLine::ComicHero2:
			return FVector2D(0.72, 0.14);
		default:
			return FVector2D(0.5, 0.14);
	}
}

FBox2D JamComicRules::Frame(const FVector2D& Screen)
{
	const double Width = FMath::Min(Screen.X, Screen.Y * FrameAspect);
	const double Left = 0.5 * (Screen.X - Width);
	return FBox2D(FVector2D(Left, 0.0), FVector2D(Left + Width, Screen.Y));
}

bool JamComicRules::SlidesIn(int32 Index)
{
	return Index > 0;
}

float JamComicRules::FadeIn(int32 Index, float PanelAge)
{
	return SlidesIn(Index) ? 1.f : FMath::Clamp(PanelAge / FadeSeconds, 0.f, 1.f);
}
