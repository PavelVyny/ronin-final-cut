#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JamFxView.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * The rich effects of the full-art tier (T-069): Niagara sparks, dust and cash bursts on the events JamFxRules picks,
 * and the neon embers that drift over the arena while the art is whole. The first cut takes all of it; the plain
 * effects of AJamEffectsView stay.
 */
UCLASS()
class JAMGAME_API AJamFxView : public AActor
{
	GENERATED_BODY()

public:
	AJamFxView();

	/** Loads the systems and starts the embers over the arena. */
	void Init();

	/**
	 * Starts the effects of this frame's events while bArt; the embers run while bArt and fade when it goes. The
	 * Ultimate's charge aura (T-072) burns round the hero, standing at HeroAt, while bArt.
	 */
	void Sync(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, bool bArt, const FVector& HeroAt);

private:
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> Sparks;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> Dust;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> Cash;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> Embers;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> Charge;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> Ambient;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ChargeAura;

	bool bAmbientOn = false;
	bool bChargeOn = false;
	bool bWasReady = false;

	/** Keeps the charge aura on the hero, sized by his meter, with a burst the moment it is full. */
	void SyncCharge(const FJamFight& Fight, bool bArt, const FVector& HeroAt);
};
