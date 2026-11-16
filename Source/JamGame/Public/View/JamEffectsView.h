#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "View/JamFeedbackRules.h"
#include "View/JamFxRules.h"

#include "JamEffectsView.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * What a fight leaves in the air for a moment: the warning circles, the sweep of a katana, the flash of an explosion or
 * of a slam, the trail of a dash. Greybox shapes from a pool that only grows; the events of the simulation start the
 * effects, the world clock ends them.
 */
UCLASS()
class JAMGAME_API AJamEffectsView : public AActor
{
	GENERATED_BODY()

public:
	AJamEffectsView();

	/** Takes the reach, the arc and the timing of the katana from the balance numbers, so the blade shows what really hits,
	 * and the sizes of the hit feedback. */
	void Init(const FJamBalanceData& InBalance, const FJamFeedbackSettings& InFeedback);

	/** Starts the effects of the frame's events and draws everything that is alive at Time. */
	void Sync(const FJamFight& Fight, const TArray<FJamEvent>& Events, float Alpha, float Time);

	/** False once the Publisher took the effects: the hero's blade turns bare grey, the trail of his dash is gone. His
	 * own effects, the warnings above all, stay. */
	bool bHeroEffects = true;

	/** The full art is on (T-075): Niagara shows the hits, slams, blasts, deflects and heals, the flat discs stay off. */
	bool bArt = false;

	/** Id of the enemy shot that "LMB PARRY" points at (JamTeachRules), INDEX_NONE for none: it carries a cyan ring. */
	int32 ParryCue = INDEX_NONE;

	/** The hero's Ultimate is full and his to use: a turning ring in its colour round his feet. */
	bool bUltimateReady = false;

	/** How the hero's charge and dash show (T-072): the floor shapes go with the full art, thin out one tier down. */
	EJamChargeLook ChargeLook = EJamChargeLook::Bars;

	/** How the slam's warning circles show (the session-5s playtest): the rune seal of the full art, else the plain disc. */
	EJamTelegraphLook TelegraphLook = EJamTelegraphLook::Plain;

	/** How bright the effects shine (JamNeonSettings::EffectGlow): above 1 they bloom. */
	float EffectGlow = 1.f;

private:
	/** A short effect started by an event. */
	struct FBurst
	{
		FJamEvent Event;
		float StartTime = 0.f;
		/** Seconds it stays on screen. */
		float Life = 0.f;
	};

	/** Seconds the effect of an event lasts; 0 for the events that are not drawn here. */
	float LifeOf(const FJamEvent& Event) const;

	/** The next free shape of the pool, shown, with this mesh and colour. */
	UStaticMeshComponent& Take(UStaticMesh* Mesh, const FLinearColor& Color);

	void DrawDisc(const FVector2D& Pos, float Radius, float Height, const FLinearColor& Color);
	/** A dashed circle on the ground that turns slowly, so it reads as a warning and not as part of the floor. */
	void DrawRing(const FVector2D& Pos, float Radius, const FLinearColor& Color, float Time);
	/** The open parry window: a standing wall of glowing panels around the body, a shield and never a blade. Rise below
	 * 1 is the Publisher's stance going up (T-038): the panels grow out of the floor to their full height. */
	void DrawShield(const FVector2D& Pos, float Radius, float Rise = 1.f);
	void DrawCircles(const FJamFight& Fight, float Alpha, float Time);
	/** The rune telegraph (JamEffectsTelegraph.cpp): loads its assets, draws seal Index, hides the seals from From on. */
	void LoadRunes();
	/** False when its assets are missing: the plain disc stands in. */
	bool DrawRune(int32 Index, const FJamTelegraph& Telegraph);
	void HideRunes(int32 From);
	/** What a fighter shows around himself: the Ultimate wind-up, the shield, the tell of the Publisher's chain. */
	void DrawStances(const FJamFight& Fight, float Alpha, float Time);
	/** The Publisher's dropped heals (T-049): in flight, then floating over a pulsing ring, blinking before they go. */
	void DrawCash(const FJamFight& Fight, float Alpha, float Time);
	/** The Ultimate lane of a fighter (JamFeedbackRules::UltimateLane): the hero's while ready or winding up, the
	 * Publisher's only while he winds up. */
	void DrawLane(const FJamFight& Fight, EJamSide Side, float Alpha);
	/** A flat strip on the floor from From along Dir. */
	void DrawStrip(const FVector2D& From, const FVector2D& Dir, float Length, float Width, float Height, const FLinearColor& Color);
	/** Age runs from 0 to 1 over the life of the effect. */
	void DrawBurst(const FJamEvent& Event, const FJamFight& Fight, float Alpha, float Age);
	/** bBare: the hero lost his effects layer, the blade is a flat grey stand-in. */
	void DrawSwing(const FJamEvent& Event, const FJamFighter& Self, const FVector2D& Pos, float Age, bool bBare);
	void DrawTrail(const FJamEvent& Event, const FJamFighter& Self, const FVector2D& Pos);
	void DrawSlash(const FJamEvent& Event, float Age);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Pool;

	/** The cash stacks on the floor wear their own textures, so they are kept out of the neon pool. */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Stacks;

	/** The rune seals on the floor, their materials and the embers rising off them, one each. */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Runes;
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> RuneLooks;
	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> RuneEmbers;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> RuneMaterial;
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> RuneEmberSystem;
	UPROPERTY()
	TObjectPtr<UStaticMesh> RunePlane;

	TArray<FBurst> Bursts;
	int32 Used = 0;
	float KatanaRange = 0.f;
	float KatanaHalfArc = 0.f;
	/** The Publisher's blade sweeps through his whole wind-up: the sweep is the warning. */
	float BossSwingLife = 0.f;
	float UltimateRange = 0.f;
	float UltimateHalfWidth = 0.f;
	float ExplosionRadius = 0.f;
	/** Steps the Publisher's shield takes to go up. */
	int32 ParryRaiseTicks = 1;
	FJamFeedbackSettings Feedback;
	/** The balance numbers the Ultimate lane is measured with. */
	FJamBalanceData Balance;
};
