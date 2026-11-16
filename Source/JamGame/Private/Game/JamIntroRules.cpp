#include "Game/JamIntroRules.h"

bool JamIntroRules::IsOver(float Age, float Length, bool bSkip, float SkipAfter)
{
	return Age >= Length || (bSkip && Age >= SkipAfter);
}

float JamIntroRules::TitleAlpha(float Age, float Length, float Fade)
{
	if (Fade <= 0.f)
	{
		return Age >= 0.f && Age < Length ? 1.f : 0.f;
	}
	return FMath::Clamp(FMath::Min(Age, Length - Age) / Fade, 0.f, 1.f);
}

float JamIntroRules::CameraToHero(float Age, float Length, float Return)
{
	const float Into = FMath::Clamp((Age - (Length - Return)) / FMath::Max(Return, UE_SMALL_NUMBER), 0.f, 1.f);
	return FMath::SmoothStep(0.f, 1.f, Into);
}

float JamIntroRules::BossBarAlpha(float SinceFight, float Fade)
{
	return FMath::Clamp(SinceFight / FMath::Max(Fade, UE_SMALL_NUMBER), 0.f, 1.f);
}

float JamIntroRules::HeroDrawStart(float Length, float Return)
{
	return FMath::Max(0.f, Length - Return);
}

float JamIntroRules::FullLength(float Length, float Return, float Draw)
{
	return FMath::Max(Length, HeroDrawStart(Length, Return) + FMath::Max(0.f, Draw));
}

bool JamIntroRules::IsThemeUp(bool bMenu, bool bComic, bool bIntroDone)
{
	return bMenu || (!bComic && bIntroDone);
}

float JamIntroRules::IntroTheme(float Age, float Length, float Return)
{
	const float Full = HeroDrawStart(Length, Return);
	if (Full <= 0.f)
	{
		return 1.f;
	}
	// Squared, so it stays quiet under the start of his show and swells towards the camera's move.
	const float Into = FMath::Clamp(Age / Full, 0.f, 1.f);
	return Into * Into;
}

float JamIntroRules::Crossfade(float Theme, bool bUp, float Dt, float Seconds)
{
	const float Target = bUp ? 1.f : 0.f;
	if (Seconds <= 0.f)
	{
		return Target;
	}
	return FMath::FInterpConstantTo(Theme, Target, Dt, 1.f / Seconds);
}
