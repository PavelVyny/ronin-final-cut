#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/JamLayerRules.h"
#include "View/JamPoseRules.h"

#include "JamFighterView.generated.h"

class UAnimSequence;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class UPoseableMeshComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/** A directional run of a figure (T-078): the asset and the cm per second its feet cover at rate 1. */
struct FJamStrideClip
{
	const TCHAR* Asset = nullptr;
	float Speed = 1.f;
};

/**
 * A fighter as the player sees it: the hero or the Publisher, a 3D model playing its clips (T-061).
 *
 * The cosmetic layers of FJamLayers come off the hero one by one (T-067): the full art first (the 3D model gives way
 * to the shape ronin of before the models, with its blade and its walk), then the glow, the materials (the grey
 * grid), the animation (he stands in the T-pose) and the model (a bean with a visor). The Publisher keeps all of them.
 */
UCLASS()
class JAMGAME_API AJamFighterView : public AActor
{
	GENERATED_BODY()

public:
	AJamFighterView();

	/** Dresses the view as Side: its model and clips, the bean around a body of this radius, the glow colour. */
	void Init(EJamSide InSide, float Radius, float MaxSpeed, const FLinearColor& Color);

	/** Switches the look to these layers; nothing happens when they did not change. */
	void SetLayers(const FJamLayers& InLayers);

	/** The fighter lost health at Time: the body goes white for a moment, Weight (0 to 1) of the way. */
	void Flash(float Time, float Weight);

	/** Starts what the steps of this frame call for: a strike, a guard, a hit, the fall at the end. */
	void Act(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, float Time);

	/**
	 * Places the view between the last two simulation steps and moves the body. bSnap: it faces Fighter's way at once,
	 * the Publisher too, with no turn lean (the menu's pose, already right on its first frame).
	 */
	void Sync(const FJamFighter& Fighter, float Alpha, float Time, bool bSnap = false);

	/** Plays Clip whole on the body from Time, outside the fight: the Publisher's show-off of the intro (T-030). */
	void Perform(EJamClip Clip, float Time);

	/** Seconds Clip lasts at its own speed; 0 when the character has none. */
	float LengthOf(EJamClip Clip) const;

	/** Freezes the body in Clip at ClipTime, whole: the pose gallery of the demo (-JamPoseGallery). */
	void HoldPose(EJamClip Clip, float ClipTime, float Time);

	/** Where this fighter's throws leave from now (T-065): the hero's left hand, the Publisher's right; zero when no model shows. */
	FVector ThrowingHand() const;

	/** The right hand's bone, the fingertip end and the body's facing as text, for working out the katana's grip. */
	FString DescribeHand() const;

	/** Where the feet are and how the hips and the chest are turned, in the world: the stride gallery's trace (T-078). */
	FString DescribeStride() const;

	/** Fades the action playing now out from Time: a skipped intro. */
	void Interrupt(float Time);

	/** Turns the body along Dir now, between simulation steps: the hero follows the live cursor on every frame. */
	void Face(const FVector2D& Dir);

private:
	/** An action playing over the walk. */
	struct FAction
	{
		EJamClip Clip = EJamClip::None;
		float StartedAt = 0.f;
		FJamPlayback Playback;
		/** Clip seconds where it ends. */
		float End = 0.f;
		bool bFullBody = false;
		bool bHold = false;
	};

	/** Paints the model in its textures, white while flashing, or the grid when the materials are gone. */
	void Repaint();

	/**
	 * Fills the pose of this frame: the runs by the speed and by MoveAngle (JamPoseRules::MoveAngle) off the facing,
	 * the lean into a turn of YawRate degrees per second, the action by its clock.
	 */
	void Animate(float Moving, float MoveAngle, float YawRate, float Time);

	/** Loads the diagonal runs (all or none) and keeps how fast each run's feet go (T-078). */
	void InitRuns(TConstArrayView<FJamStrideClip> InDiagonals, float MoveSpeed, float MaxRate, float InUpperWalk);

	/** The legs' directional runs, their clock and the hips' twist toward the move (T-078). */
	void AnimateStride(struct FJamPoseFrame& Out, float Moving, float MoveAngle, float Dt);

	/** Puts the action that was cut short into the pose while it fades out (T-066). */
	void ShowFading(struct FJamPoseFrame& Out, float Time);

	/** Turns the model by its figure's EntranceTurn as far as the Entrance clip shows (T-090). */
	void TurnForShowOff(float Time);

	/** Where an action is in its clip at Time, and how much of it shows. */
	float ClipTimeOf(const FAction& Of, float Time) const;
	float WeightOf(const FAction& Of, float Time) const;

	void Start(EJamClip Clip, float Lead, bool bFullBody, float Time);

	/** The shape ronin's walk: bob, lean and arm swing by how fast the body moves; the T-pose without animation. */
	void AnimateRonin(float Moving, float Time);

	UStaticMeshComponent* AddPart(USceneComponent* Parent, const TCHAR* Name, UStaticMesh* Mesh);

	/** The shape ronin's parts (constructor only), their sizes around a body of Radius, their colours. */
	void ConstructRonin();

	/** Starts or stops the blade's ribbon. */
	void SetTrail(bool bOn);

	/** The hero's own: the katana in his hand and the afterimage meshes. */
	void InitHeroGear();
	void InitRonin(float Radius);
	void PaintRonin(bool bGrid);

	/** The hero's afterimages after a hit (T-064): left one by one where he was, each fading out. */
	void UpdateGhosts(float Time);

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Visor;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Figure;

	/** The shape ronin of before the models (T-067): it hangs off this pivot at the feet, so a lean turns all of it. */
	UPROPERTY()
	TObjectPtr<USceneComponent> Ronin;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Torso;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Head;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Eyes;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Hat;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShoulderL;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShoulderR;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmL;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmR;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Blade;

	/** The hero's blade, in his right hand. */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Katana;

	/** A neon ribbon off the blade's tip while it swings, in the full art (T-069). */
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> Trail;

	bool bTrailOn = false;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Skin;

	/** Frozen neon copies of the hero's pose, left behind after a hit. */
	UPROPERTY()
	TArray<TObjectPtr<UPoseableMeshComponent>> Ghosts;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> GhostSkins;

	/** The clips by EJamClip; empty where the character has none. */
	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> Clips;

	EJamSide Side = EJamSide::Hero;
	FJamLayers Shown;
	FAction Action;
	/** The action a new one cut short, fading out from FadingWeight at FadingAt (T-066). */
	FAction Fading;
	float FadingWeight = 0.f;
	float FadingAt = 0.f;
	FLinearColor BaseColor = FLinearColor::White;
	float Speed = 1.f;
	float BodyRadius = 0.f;
	float NeonGlow = 0.f;
	float MoveWeight = 0.f;
	/** The diagonal runs by EJamStride ForwardLeft to BackRight (T-078); none for the Publisher. */
	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> Diagonals;
	/** Where the legs are in their run cycle, 0 to 1, shared by every directional run so they step together. */
	float StridePhase = 0.f;
	/** cm per second each run's feet cover at rate 1, by EJamStride, and the fastest a run may play. */
	TArray<float> RunSpeeds;
	float MaxRunRate = 1.f;
	/** How much of the walk the upper body shows (FJamPoseFrame::UpperWalk). */
	float UpperWalk = 1.f;
	/** The yaw the model is turned by now, on top of its fixed one (TurnForShowOff). */
	float ModelTurn = 0.f;
	/** The runs' weights by EJamStride, eased toward what the move angle asks. */
	TArray<float> StrideWeights;
	float StrideTwist = 0.f;
	/** The move's angle off the facing the legs run at (JamPoseRules::MoveAngle), kept while the body stands. */
	float StrideAngle = 0.f;
	float TurnLeanShown = 0.f;
	float LastTime = 0.f;
	/** When the last shaking hit came; negative before the first. */
	float HitAt = -1.f;
	/** When the last afterimages began: a shaking hit or a dash of the hero's. Negative before the first. */
	float GhostAt = -1.f;
	float LastSyncTime = 0.f;
	/** Afterimages of that hit left so far, and when each was left. */
	int32 GhostsLeft = 0;
	TArray<float> GhostBornAt;
	float FlashUntil = 0.f;
	float FlashWeight = 1.f;
	bool bFlashing = false;
};
