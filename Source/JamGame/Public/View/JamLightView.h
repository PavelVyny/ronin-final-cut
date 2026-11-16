#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "View/JamNeonRules.h"
#include "View/JamSpotRules.h"

#include "JamLightView.generated.h"

class UDirectionalLightComponent;
class UPointLightComponent;

/**
 * The neon light of the full-art tier (T-077): cyan and magenta lights round the edge of the arena that throw their
 * colour onto the floor, the bodies and the effects. The lower tiers turn them off (JamNeonRules::LightFor); the
 * camera's post process carries the rest of the tier's look (AJamCameraRig::SetLight).
 */
UCLASS()
class JAMGAME_API AJamLightView : public AActor
{
	GENERATED_BODY()

public:
	AJamLightView();

	/**
	 * Places the lights round an arena of this radius, in step with its posts, and takes the sun of Stage (the actor
	 * that lights the arena) to dim it at night. Called once.
	 */
	void Init(float ArenaRadius, const AActor& Stage);

	/** The lights shine and the sun dims with the full art only. Cheap when nothing changes. */
	void SetLight(EJamLight InLight);

	/** The full art's rain haze and depth fog (T-094); a zero density hides the fog. */
	void SetHaze(const FJamFilmLook& Look);

	/**
	 * Keeps the key light on the Publisher standing at BossAt (T-082), trailing him like a hand would (T-087), and the
	 * hero's smaller one on him at HeroAt (T-100). ToHero below 1 is the intro's camera on its way over to the hero: his
	 * spot splits off the Publisher's beam and rides with it (JamSpotRules::SplitOff).
	 */
	void Follow(const FVector& BossAt, const FVector& HeroAt, float Time, float ToHero = 1.f);

private:
	UPROPERTY()
	TArray<TObjectPtr<UPointLightComponent>> Lamps;

	/** The stage's sun, not owned here: dimmed to a moon while the full art is on. */
	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY()
	TObjectPtr<class USpotLightComponent> Key;

	/** The hero's own key light (T-100). */
	UPROPERTY()
	TObjectPtr<class USpotLightComponent> HeroKey;

	UPROPERTY()
	TObjectPtr<class UExponentialHeightFogComponent> Haze;

	/** Where the key light's spot is on the floor, and when it last moved (negative: not placed yet). */
	FJamSpot Spot;
	FJamSpot HeroSpot;
	float SpotTime = -1.f;

	float DaySun = 0.f;
	EJamLight Light = EJamLight::Neon;

	void DimSun();

	/** A warm key light with these cones, shown with the full art only. */
	class USpotLightComponent* MakeKey(float Candela, float InnerCone, float OuterCone);

	/** Points Beam from above and the camera's side down at the spot At, at AimHeight over the floor. */
	static void Aim(class USpotLightComponent& Beam, const FVector2D& At, float AimHeight);
};
