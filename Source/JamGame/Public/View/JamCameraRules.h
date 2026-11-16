#pragma once

#include "CoreMinimal.h"

#include "JamCameraRules.generated.h"

/**
 * Numbers of the three candidate cameras. The values live in the [JamCamera] section of Config/DefaultGame.ini, one line
 * per field. Angles in degrees, distances in cm.
 */
USTRUCT()
struct JAMGAME_API FJamCameraSettings
{
	GENERATED_BODY()

	/** Top-down 3/4: a fixed direction, the camera only follows the hero. */
	UPROPERTY()
	float TopDownYaw = 0.f;

	UPROPERTY()
	float TopDownPitch = 0.f;

	UPROPERTY()
	float TopDownDistance = 0.f;

	UPROPERTY()
	float TopDownFov = 0.f;

	/** Third person: behind the hero, above and to the right of him. */
	UPROPERTY()
	float ShoulderDistance = 0.f;

	UPROPERTY()
	float ShoulderHeight = 0.f;

	UPROPERTY()
	float ShoulderSide = 0.f;

	UPROPERTY()
	float ShoulderFov = 0.f;

	/** First person. */
	UPROPERTY()
	float EyeHeight = 0.f;

	UPROPERTY()
	float FirstPersonFov = 0.f;

	/** Limits of looking down and up in the mouse-look cameras. */
	UPROPERTY()
	float MinPitch = 0.f;

	UPROPERTY()
	float MaxPitch = 0.f;

	/** Degrees of turn per unit of mouse movement. */
	UPROPERTY()
	float MouseSensitivity = 0.f;
};

/** The three cameras the prototype is judged in, on F1, F2 and F3. */
enum class EJamCameraMode : uint8
{
	TopDown,
	Shoulder,
	FirstPerson
};

struct FJamView
{
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	float Fov = 90.f;
};

/**
 * Where each camera stands. The camera is presentation only: the simulation gets a move direction and an aim and never
 * learns which mode produced them.
 * Tests: Jam.Camera.*
 */
namespace JamCameraRules
{
/** Section with the camera numbers (FJamCameraSettings). */
inline const TCHAR* const Section = TEXT("JamCamera");

/** HeroPos is the hero's feet in the world. The look angles matter only in the mouse-look modes. */
JAMGAME_API FJamView ViewFor(
	EJamCameraMode Mode, const FVector& HeroPos, float LookYaw, float LookPitch, const FJamCameraSettings& Settings);

/** Yaw that "forward" on the keyboard means in this mode: the screen's up in top-down, the look direction otherwise. */
JAMGAME_API float MoveBasisYaw(EJamCameraMode Mode, float LookYaw, const FJamCameraSettings& Settings);

JAMGAME_API float ClampPitch(float Pitch, const FJamCameraSettings& Settings);
} // namespace JamCameraRules
