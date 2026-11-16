#pragma once

#include "CoreMinimal.h"
#include "Sim/JamLayerRules.h"

/** Where the menu's skyline stands (T-094): its size and the height of its centre, in cm. */
struct FJamBackdropPlace
{
	double Width = 0.0;
	double Height = 0.0;
	double CentreZ = 0.0;
};

/** The three looks of the arena, from the full rooftop shrine down to the engine grid. */
enum class EJamArenaLook : uint8
{
	/** The full art: the wet slate shrine with its crest, props, city below and rain. */
	Shrine,
	/** After the first cut: the slate floor and the lantern pillars only. */
	BareShrine,
	/** After the second cut: the engine grid and plain posts. */
	Greybox
};

/**
 * The arena's look for the layers the Publisher has left, and when the wet floor splashes under the fighters' feet.
 * Presentation only.
 * Tests: Jam.Arena.*
 */
namespace JamArenaRules
{
/** The look of the arena with these layers: the art keeps the shrine, the materials keep the bare shrine. */
JAMGAME_API EJamArenaLook LookFor(const FJamLayers& Layers);

/**
 * How many splashes a fighter makes walking Distance cm, one per Stride cm. Carry holds the distance walked since the
 * last splash between frames. No splashes for a Stride of zero or less.
 */
JAMGAME_API int32 StepSplashes(float& Carry, float Distance, float Stride);

/**
 * The menu's skyline (T-094) on an upright plane Distance cm in front of a camera at Eye looking at LookAt: scaled to
 * cover the whole view on a screen of Aspect, the picture's top at the top of the view so the tower's crown always
 * shows. Fov is the horizontal field of view at 16:9; the engine keeps the vertical one on other screens
 * (AspectRatio_MaintainYFOV).
 */
JAMGAME_API FJamBackdropPlace PlaceBackdrop(
	const FVector& Eye, const FVector& LookAt, float Fov, float Aspect, float PictureAspect, double Distance);
} // namespace JamArenaRules
