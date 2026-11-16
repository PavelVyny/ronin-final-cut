#pragma once

#include "CoreMinimal.h"

/**
 * T-099: how big the HUD is drawn on a screen. The HUD is laid out for a 1080-line screen and grows with the real one, so it
 * reads the same at 720p, on an ultrawide and at 4K; its text is rasterised at the size it is shown at, never a small font
 * scaled up (that was soft).
 *
 * Tests: Jam.Hud.*
 */
namespace JamHudScaleRules
{
/** The screen height the HUD's pixel sizes and text scales were set on. */
inline constexpr float ReferenceHeight = 1080.f;

/** How much bigger than on the reference screen the HUD is drawn on a screen ScreenHeight pixels tall. */
JAMGAME_API float UiScale(float ScreenHeight);

/** The size a font of BaseSize is rasterised at when drawn at Scale on a screen of UiScale: never under one. */
JAMGAME_API float FontSize(float BaseSize, float Scale, float UiScale);
} // namespace JamHudScaleRules
