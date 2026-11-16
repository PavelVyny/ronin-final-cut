#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

#include "JamFeedbackRules.generated.h"

/**
 * Numbers of the hit feedback and the camera shake. The values live in the [JamFeedback] section of
 * Config/DefaultGame.ini, one line per field. Distances in cm, times in seconds.
 */
USTRUCT()
struct JAMGAME_API FJamFeedbackSettings
{
	GENERATED_BODY()

	/** How far the camera is thrown by each kind of blow. */
	UPROPERTY()
	float SlamShake = 0.f;

	UPROPERTY()
	float UltimateShake = 0.f;

	UPROPERTY()
	float ExplosionShake = 0.f;

	UPROPERTY()
	float HeroHurtShake = 0.f;

	/** Seconds a shake takes to die out. */
	UPROPERTY()
	float ShakeSeconds = 0.f;

	/** Swings of the camera per second while it shakes. */
	UPROPERTY()
	float ShakeFrequency = 0.f;

	/** The spark where a blow lands: its radius per point of damage, never smaller or larger than the limits. */
	UPROPERTY()
	float SparkPerDamage = 0.f;

	UPROPERTY()
	float SparkMinRadius = 0.f;

	UPROPERTY()
	float SparkMaxRadius = 0.f;

	/** A hit on the Publisher of this damage or more flashes him fully white; a smaller one only tints him. */
	UPROPERTY()
	float BossFlashFullDamage = 0.f;

	/** The tint of the smallest hit on the Publisher, 0 to 1. */
	UPROPERTY()
	float BossFlashMin = 0.f;

	/** The parry ring opens this wide (in body radii) and closes to the end size when the window shuts. */
	UPROPERTY()
	float ParryRingStart = 0.f;

	UPROPERTY()
	float ParryRingEnd = 0.f;

	/** The Publisher bleeds paper dollars when hit: one bill per this much damage, never fewer or more than the limits. */
	UPROPERTY()
	float DamagePerBill = 0.f;

	UPROPERTY()
	int32 BillsPerHitMin = 0;

	UPROPERTY()
	int32 BillsPerHitMax = 0;

	/** The burst of the final blow. */
	UPROPERTY()
	int32 BillsFinalBlow = 0;

	/** A bill flutters down this long, lies on the floor this long, then shrinks away. */
	UPROPERTY()
	float BillFlutterSeconds = 0.f;

	UPROPERTY()
	float BillFloorSeconds = 0.f;

	UPROPERTY()
	float BillFadeSeconds = 0.f;

	/** How high above the hit point a bill pops before it falls, in cm. */
	UPROPERTY()
	float BillPopHeight = 0.f;

	/** How far a falling bill swings to the side, in cm, and how often it rocks per second. */
	UPROPERTY()
	float BillSway = 0.f;

	UPROPERTY()
	float BillFlutterHz = 0.f;
};

/** Where a paper dollar leaves the Publisher and how far it flies. */
struct FJamSpill
{
	FVector2D From = FVector2D::ZeroVector;
	FVector2D Kick = FVector2D::ZeroVector;
};

/** Where a paper dollar is at one moment of its life. */
struct FJamBillPose
{
	FVector Pos = FVector::ZeroVector;
	/** Tip of the bill round the way it flies, in degrees: it rocks in the air and lies flat on the floor. */
	float Roll = 0.f;
	/** 1 while it lives, down to 0 as it fades. */
	float Scale = 1.f;
};

/** The lane on the floor that shows where an Ultimate cuts and when (T-046). */
struct FJamUltimateLane
{
	bool bShow = false;
	/** Pressed: the lane is locked on its way and fills up to the cut. Before that it follows the aim. */
	bool bArmed = false;
	/** The other fighter stands in the lane: the cut would land on him. */
	bool bOnTarget = false;
	FVector2D Origin = FVector2D::ZeroVector;
	FVector2D Dir = FVector2D(1.0, 0.0);
	float Length = 0.f;
	float HalfWidth = 0.f;
	/** 0 at the press, 1 on the step the cut lands. */
	float Fill = 0.f;
};

/** How an effect is drawn once the hero may have lost his effects layer. */
enum class EJamEffectLook : uint8
{
	Hidden,
	/** The unfinished game: a flat grey stand-in, so the blow is still seen. */
	Bare,
	Neon,
};

/**
 * What the player is told by the picture: which blows shake the camera and how much, how a hit looks, which
 * projectile carries a warning and how the parry window shows the time it has left. Presentation only: the simulation
 * never asks.
 * Tests: Jam.Feedback.*
 */
namespace JamFeedbackRules
{
/** Section with the feedback numbers (FJamFeedbackSettings). */
inline const TCHAR* const Section = TEXT("JamFeedback");

/** How far an event throws the camera, in cm; 0 for the events that do not shake it. */
JAMGAME_API float ShakeKick(const FJamEvent& Event, const FJamFeedbackSettings& Settings);

/** What is left of a shake of strength Kick after Elapsed seconds: it dies out at ShakeSeconds. */
JAMGAME_API float ShakeLeft(float Kick, float Elapsed, const FJamFeedbackSettings& Settings);

/** Where the shaking camera is pushed at Time, on the ground plane: never further than Amplitude. */
JAMGAME_API FVector2D ShakeOffset(float Amplitude, float Time, const FJamFeedbackSettings& Settings);

/** True when a projectile shows the circle of its blast in flight: the explosive ones flying at the hero. */
JAMGAME_API bool ShowsBlastRing(const FJamProjectile& Projectile);

/** Radius of the spark where a blow of this damage lands. */
JAMGAME_API float SparkRadius(float Damage, const FJamFeedbackSettings& Settings);

/** How white a hit turns the fighter, 0 to 1: the hero always fully, the Publisher by the size of the blow. */
JAMGAME_API float FlashWeight(EJamSide Side, float Damage, const FJamFeedbackSettings& Settings);

/** Radius of the open parry shield in body radii; WindowLeft runs from 1 when it opens to 0 when it shuts. */
JAMGAME_API float ParryRingScale(float WindowLeft, const FJamFeedbackSettings& Settings);

/**
 * How an event of Side is drawn while the hero has (bHeroEffects) or has lost his effects layer. His flourishes (the
 * dash trail, the spark of his hit on the Publisher) go with it; his blade turns bare but stays.
 */
JAMGAME_API EJamEffectLook LookOf(EJamEvent Type, EJamSide Side, bool bHeroEffects);

/**
 * How many paper dollars a hit on the Publisher sprays (bFinalBlow: the blow that ends the fight); 0 for any other
 * event. The money is his look, not the hero's effects: it never goes with the effects cut.
 */
JAMGAME_API int32 BillCount(const FJamEvent& Event, bool bFinalBlow, const FJamFeedbackSettings& Settings);

/**
 * A bill thrown from From with a sideways Kick (cm it travels in the air) lands on the floor at FloorZ. Phase (0 to 1)
 * sets where its rocking starts, so a spray does not rock in step; Age is seconds since the hit.
 */
JAMGAME_API FJamBillPose BillPose(
	const FVector& From, const FVector2D& Kick, float FloorZ, float Phase, float Age, const FJamFeedbackSettings& Settings);

/** Seconds from the hit until a bill is gone. */
JAMGAME_API float BillLife(const FJamFeedbackSettings& Settings);

/**
 * A bill spilling out of a body of BodyRadius at Centre (T-054): it leaves the body's edge at Degrees and lands Reach
 * cm past it, never inside him. The start and the throw for BillPose.
 */
JAMGAME_API FJamSpill Spill(const FVector2D& Centre, float BodyRadius, float Degrees, float Reach);

/**
 * The Ultimate lane of Side at the picture between the last two steps (Alpha 0 to 1): shown while he owns a full meter
 * or winds up, along his aim until the press and along the locked way after it.
 */
JAMGAME_API FJamUltimateLane UltimateLane(const FJamFight& Fight, EJamSide Side, float Alpha, const FJamBalanceData& Balance);
} // namespace JamFeedbackRules
