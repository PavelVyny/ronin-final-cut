#pragma once

#include "CoreMinimal.h"

/**
 * T-099: the HUD's sizes in pixels of the 1080-line screen it was laid out on (JamHudScaleRules), grown with the real one.
 * AJamHUD::DrawHUD sets the scale at the start of every frame; a constant of FPixels reads as the grown size wherever a float
 * is used.
 */
namespace JamHudPixels
{
/** JamHudScaleRules::UiScale of the screen the HUD draws on this frame. */
inline float UiScale = 1.f;

/** A length in pixels of the 1080-line screen. */
struct FPixels
{
	float Reference = 0.f;

	operator float() const { return Reference * UiScale; }
};
} // namespace JamHudPixels
