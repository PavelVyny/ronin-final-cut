#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * Where a fighter aims, whatever the camera is: a cursor ray on the arena plane (top-down) or a look yaw (third and
 * first person). The simulation only ever sees the resulting FJamAim.
 * Tests: Jam.Aim.*
 */
namespace JamAimRules
{
/**
 * Aim at the point where a ray meets the plane Z = PlaneZ. Returns false and leaves InOutAim untouched when the ray never
 * meets the plane. A point on the fighter itself updates Point but keeps the last direction.
 */
JAMGAME_API bool FromRay(const FVector& Origin, const FVector& Dir, float PlaneZ, const FVector2D& From, FJamAim& InOutAim);

/** Aim along a yaw, with the point Reach ahead of the fighter. */
JAMGAME_API FJamAim FromYaw(float YawDeg, const FVector2D& From, float Reach);

/** The way a fighter drawn at From faces a cursor at Point; Keep when the cursor is on him. */
JAMGAME_API FVector2D FacingTo(const FVector2D& Point, const FVector2D& From, const FVector2D& Keep);

/**
 * The floor point under a still cursor after a camera that only slides (the top-down rig) moved from CameraWas to
 * CameraNow: the cursor is read through last frame's camera, the hero is drawn under this frame's.
 */
JAMGAME_API FVector2D FollowCamera(const FVector2D& Point, const FVector& CameraWas, const FVector& CameraNow);
} // namespace JamAimRules
