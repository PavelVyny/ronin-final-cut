#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "View/JamArenaRules.h"
#include "View/JamNeonRules.h"

#include "JamArenaView.generated.h"

class UDirectionalLightComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * The arena as the player sees it, in three looks (JamArenaRules::LookFor). The shrine of the full art (T-081): a wet
 * slate rooftop with an engraved crest, lantern pillars along the edge, torii gates, stone lanterns, bonsai, a
 * railing, the city far below and rain. The bare shrine after the first cut: the slate and the pillars. The greybox
 * after the second: the engine grid and plain posts. A neon band marks the edge in every look but the grid.
 * Built in code so that the map stays an empty file.
 */
UCLASS()
class JAMGAME_API AJamArenaView : public AActor
{
	GENERATED_BODY()

public:
	AJamArenaView();

	/** Builds every look of an arena of this radius and shows the shrine. Called once. */
	void Init(float ArenaRadius, const FJamNeonSettings& InNeon);

	/** Shows one look. Cheap when nothing changes. */
	void SetLook(EJamArenaLook InLook);

	/** The beat of the music this frame (JamNeonRules::LineGlow): the edge band kicks with it, not on the grid. */
	void Pulse(float LineGlow);

	/**
	 * Fighter Who (an EJamSide) stands at At this frame: the wet floor splashes once a stride under its feet
	 * (JamArenaRules::StepSplashes), only while it rains (the shrine). A jump of more than a dash (a retry) splashes not.
	 */
	void Wade(int32 Who, const FVector& At, float Stride);

	/** The skyline standing behind the main menu's two fighters, past the rim on -Y, facing the menu camera (T-092). */
	void ShowBackdrop(bool bShown);

	/**
	 * Sizes the skyline to cover a screen of Aspect seen from Eye toward LookAt (horizontal Fov on 16:9), its top on the
	 * screen's top, so the tower and its sign stay in frame on any width (JamArenaRules::PlaceBackdrop, T-094).
	 */
	void PlaceBackdrop(const FVector& Eye, const FVector& LookAt, float Fov, float Aspect);

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Backdrop;

	/** Where a fighter stood last frame and how far it walked since its last splash. */
	struct FWader
	{
		FVector Last = FVector::ZeroVector;
		float Carry = 0.f;
		bool bSeen = false;
	};

	TMap<int32, FWader> Waders;

	/** The pieces of the arena: each shows in its own looks. */
	enum class EArenaPiece : uint8
	{
		/** Greybox posts: plain boxes along the edge. */
		Post,
		/** The neon band along the edge of the floor. */
		Edge,
		/** The slate rooftop, its tower and the lantern pillars: the shrine and the bare shrine. */
		Shrine,
		/** The torii, lanterns, bonsai, railing and the city: the shrine only. */
		Prop
	};

	/** A neon box with its centre at Location, turned by Yaw degrees. */
	void AddBox(const FVector& Location, float Yaw, const FVector& Size, const FLinearColor& Color, EArenaPiece Piece);

	/** A model standing at Location (its bottom there), turned by Yaw, scaled so its longest side is Size cm. */
	UStaticMeshComponent* AddModel(
		UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Location, float Yaw, float Size, EArenaPiece Piece);

	/** The shrine's pieces (JamArenaShrine.cpp): the slate rooftop and its tower, the props round the edge, the city
	 * below and the rain. */
	void BuildShrine(float ArenaRadius);
	void BuildRooftop(float ArenaRadius);
	void BuildRim(float ArenaRadius);
	void BuildSky();

	/** Whether Piece shows in the current look. */
	bool Shows(EArenaPiece Piece) const;

	/** Gives box Index the look now on. */
	void Dress(int32 Index);

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Floor;

	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> Sun;

	/** T-103: the menu's low light across the slate, on with the backdrop; only the floor takes its lighting channel. */
	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> RimLight;

	UPROPERTY()
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY()
	TObjectPtr<USkyLightComponent> SkyLight;

	/** Neon boxes (posts, edge) and their colours; the models are in Parts. */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Boxes;

	TArray<FLinearColor> BoxColors;
	TArray<EArenaPiece> Pieces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	TArray<EArenaPiece> PartPieces;

	/** The slate floor's own material: the crest shows only in the shrine. */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SlateFloor;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> Rain;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> SplashSystem;

	FJamNeonSettings Neon;
	EJamArenaLook Look = EJamArenaLook::Shrine;
};
