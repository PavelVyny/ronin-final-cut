#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "JamPlayerController.generated.h"

class AJamCameraRig;

/**
 * The player's hands: turns the keyboard and the mouse into the hero's intent. The camera is the top-down one only (T-018):
 * the cursor aims, the keys move in screen axes. The player has no pawn, the hero is a body of the simulation.
 *
 * Keys are polled on purpose. The prototype has a dozen fixed keys and no rebinding, so input assets would only be one
 * more thing to cook.
 */
UCLASS()
class JAMGAME_API AJamPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AJamPlayerController();

	/**
	 * Reads this frame's input and returns what the hero wants to do. CardOptions: how many choices the steal card shows
	 * (0: no card); a click on one of them answers it (T-094).
	 */
	FJamIntent ReadIntent(const AJamCameraRig& Rig, const FJamFighter& Hero, int32 CardOptions = 0);

	/** Whether any button of the keyboard or the mouse went down this frame (the HOW TO PLAY screen, T-096). */
	bool AnyKeyJustPressed() const;

	/** The mouse behaviour of the top-down camera: a visible cursor locked to the window. */
	void UseCursor();

	/** The floor point under the cursor as seen through the camera at CameraNow, after the camera moved this frame. */
	FVector2D LiveAimPoint(const FVector& CameraNow) const;

private:
	/** The last known aim: kept while the cursor ray misses the arena. */
	FJamAim Aim;

	/** Where the camera stood when the cursor was last read: the view of the last rendered frame. */
	FVector AimCamera = FVector::ZeroVector;
};
