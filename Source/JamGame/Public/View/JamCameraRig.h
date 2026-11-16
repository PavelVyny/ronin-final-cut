#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "View/JamCameraRules.h"
#include "View/JamFeedbackRules.h"
#include "View/JamNeonRules.h"

#include "JamCameraRig.generated.h"

class UCameraComponent;

/**
 * The game camera: one actor that stands where JamCameraRules puts it for the current mode. It owns the mode and the look
 * angles of the mouse-look modes; the player controller writes into it and asks it how to turn the move input.
 */
UCLASS()
class JAMGAME_API AJamCameraRig : public AActor
{
	GENERATED_BODY()

public:
	AJamCameraRig();

	virtual void PostInitializeComponents() override;

	void SetMode(EJamCameraMode InMode) { Mode = InMode; }

	EJamCameraMode GetMode() const { return Mode; }

	/** Mouse movement of the frame, in the units of the player input. */
	void AddLook(float MouseX, float MouseY);

	/** Points the look along a yaw, level with the ground: used when a mouse-look mode is entered. */
	void SetLook(float Yaw);

	float GetLookYaw() const { return LookYaw; }

	/** Yaw that "forward" on the keyboard means now. */
	float GetMoveBasisYaw() const;

	/** How much the bright neon blooms (JamNeonRules::Bloom). */
	void SetBloom(float Intensity);

	/**
	 * The post process of a tier (T-077, JamNeonRules::LightFor): Lumen light and reflections, a vignette and a grade
	 * for the full art; plain screen-space reflections for the shape ronin; nothing for the greybox.
	 */
	void SetLight(EJamLight Light);

	/**
	 * The full art's film look on the camera (T-094): grain, a colour fringe toward the edges and a soft far distance
	 * (JamNeonRules::FilmLookFor). All zero turns it off.
	 */
	void SetFilm(const FJamFilmLook& Look);

	/** A blow of this strength (cm) at Time shakes the camera; a weaker one never cuts a stronger shake short. */
	void Kick(float Strength, float Time);

	/** Places the camera for the hero's feet at HeroPos, shaken as it is at Time. Called every frame after the views. */
	void Sync(const FVector& HeroPos, float Time);

	/**
	 * Renders what the camera sees now into a texture and returns it (T-068): the old look the crack opens through.
	 * Called before the new look is applied, so the picture is the old one.
	 */
	class UTextureRenderTarget2D* Freeze();

	/** Puts the camera at Location looking at Target: the close views of the pose gallery (-JamPoseGallery). */
	void Frame(const FVector& Location, const FVector& Target, float Fov);

private:
	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	/** Renders the frozen frame of the crack; idle otherwise. */
	UPROPERTY()
	TObjectPtr<class USceneCaptureComponent2D> Capture;

	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> Frozen;

	FJamCameraSettings Settings;
	FJamFeedbackSettings Feedback;
	/** The shake running now: its strength at KickTime. */
	float KickStrength = 0.f;
	float KickTime = 0.f;
	EJamCameraMode Mode = EJamCameraMode::TopDown;
	float LookYaw = 0.f;
	float LookPitch = 0.f;
};
