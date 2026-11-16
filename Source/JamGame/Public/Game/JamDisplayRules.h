#pragma once

#include "CoreMinimal.h"

/** The window modes in the order of the menu's WINDOW row (JamTexts::WindowNames). */
enum class EJamWindowMode : uint8
{
	Fullscreen,
	Borderless,
	Windowed
};

/** What the game shows the picture in: the window, its size in pixels, and the share of it the scene renders at. */
struct FJamDisplay
{
	EJamWindowMode Mode = EJamWindowMode::Borderless;
	FIntPoint Resolution = FIntPoint::ZeroValue;
	/** sg.ResolutionQuality: 0 leaves the engine's own default, which renders a 1440p screen at about two thirds. */
	float RenderScale = 0.f;
};

/**
 * T-102: the author's first launch was blurry. The picture a run starts with: a first launch is borderless at the
 * desktop's size with the scene at full size, a borderless window always covers the desktop, a window fits it, and no
 * saved setting renders the scene smaller (an RTX card's DLSS mode sets its own screen percentage after this).
 * Tests: Jam.Display.*
 */
namespace JamDisplayRules
{
/** The scene's share of the window: always all of it. */
inline constexpr float FullRenderScale = 100.f;

/**
 * The display a run starts with, from what was saved (bSaved false on a first launch, nothing in the user's
 * GameUserSettings.ini) on a desktop of Desktop pixels (none known: the saved size is kept).
 */
JAMGAME_API FJamDisplay Settle(const FJamDisplay& Saved, bool bSaved, const FIntPoint& Desktop);
} // namespace JamDisplayRules
