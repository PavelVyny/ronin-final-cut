#include "UI/JamHudScaleRules.h"

float JamHudScaleRules::UiScale(float ScreenHeight)
{
	return ScreenHeight > 0.f ? ScreenHeight / ReferenceHeight : 1.f;
}

float JamHudScaleRules::FontSize(float BaseSize, float Scale, float UiScale)
{
	return FMath::Max(1.f, BaseSize * Scale * UiScale);
}
