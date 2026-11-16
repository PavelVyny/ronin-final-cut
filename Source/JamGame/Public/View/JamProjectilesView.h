#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JamProjectilesView.generated.h"

class UStaticMeshComponent;

/**
 * Every projectile of the fight, drawn from a pool of spheres. The pool only grows: a fight has tens of projectiles at
 * most, and a sphere that is not needed this frame is hidden.
 */
UCLASS()
class JAMGAME_API AJamProjectilesView : public AActor
{
	GENERATED_BODY()

public:
	AJamProjectilesView();

	/** Places a shape per projectile, between the last two simulation steps; an explosive one pulses with Time. */
	void Sync(const TArray<FJamProjectile>& Projectiles, float Alpha, float Time);

	/**
	 * Where each fighter's throwing hand is now (T-065): a new shot is drawn leaving it and joins its flight line within
	 * a few frames. The simulation still throws from the body; only the picture starts at the hand.
	 */
	TJamPerSide<FVector> Hands;

	/** The full art is on (T-069): shurikens leave a neon ribbon. */
	bool bArt = true;

private:
	/** When each projectile was first drawn, by its id: its first frames start at the hand. */
	TMap<int32, float> FirstSeen;

	/** The ribbon behind each shuriken, by its id: it follows the drawn shot and fades out when the shot is gone. */
	UPROPERTY()
	TMap<int32, TObjectPtr<class UNiagaraComponent>> Trails;

	UPROPERTY()
	TObjectPtr<class UNiagaraSystem> TrailSystem;

	/** Starts, moves or lets go of the ribbon of one shot drawn at Location this frame. */
	void Trail(const FJamProjectile& Projectile, const FVector& Location, TSet<int32>& Kept);

	/** Puts the floor lights of the full art on the newest shurikens, drawn at DrawnAt this frame (T-088). */
	void LightShots(const TArray<FJamProjectile>& Projectiles);

	/** Where each projectile was drawn this frame, by its index in the fight. */
	TArray<FVector> DrawnAt;

	UPROPERTY()
	TArray<TObjectPtr<class UPointLightComponent>> ShotLights;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Pool;
};
