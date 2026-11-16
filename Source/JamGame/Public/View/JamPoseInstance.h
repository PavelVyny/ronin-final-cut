#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"

#include "JamPoseInstance.generated.h"

class UAnimSequence;

/** What a fighter's body shows on this frame: the view decides it, the pose instance only blends it. */
struct FJamPoseFrame
{
	/** The rest pose: for the hero the T-pose, what a character is before anybody animates it. */
	bool bRefPose = false;

	const UAnimSequence* Idle = nullptr;
	float IdleTime = 0.f;

	/**
	 * The directional runs (T-078) by EJamStride, blended by RunWeights into the walk, which goes over the idle by
	 * MoveWeight. Back is the forward run played backwards: the view sets its clip and its time.
	 */
	static constexpr int32 RunCount = 6;
	const UAnimSequence* Runs[RunCount] = {};
	float RunTimes[RunCount] = {};
	float RunWeights[RunCount] = {};
	float MoveWeight = 0.f;
	/** How much of the walk the upper body (UpperBodyBone and under) shows; the rest stays the idle's. */
	float UpperWalk = 1.f;

	/**
	 * Degrees the hips and the legs turn toward the move (positive: to the right); the spine turns back as much, so
	 * the chest keeps facing the aim (T-078).
	 */
	float Twist = 0.f;
	/** Degrees the upper body leans to the right into a turn (T-078); negative to the left. */
	float TurnLean = 0.f;

	/** The action a new one cut short (T-066): it fades out under the new one instead of popping. */
	const UAnimSequence* Fading = nullptr;
	float FadingTime = 0.f;
	float FadingWeight = 0.f;
	bool bFadingFullBody = false;

	/** A one-shot action over both, ActionWeight of the way. Only from the spine up while he walks, unless bFullBody. */
	const UAnimSequence* Action = nullptr;
	float ActionTime = 0.f;
	float ActionWeight = 0.f;
	bool bFullBody = false;

	/**
	 * The hit lean (T-064): the upper body turns by how Lean at LeanTime differs from its first frame, LeanWeight of
	 * the way, on top of everything else. It rotates only: the root and the legs never move for it.
	 */
	const UAnimSequence* Lean = nullptr;
	float LeanTime = 0.f;
	float LeanWeight = 0.f;
};

/**
 * The animation of the 3D fighters without an animation blueprint: the idle, the walk and one action blended by hand
 * in C++, on the animation thread (FJamPoseProxy). The sequences must outlive the frame: the view keeps them.
 */
UCLASS(Transient, NotBlueprintable)
class JAMGAME_API UJamPoseInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** The upper body is this bone and everything under it. */
	static inline const FName UpperBodyBone = TEXT("spine_01");

	FJamPoseFrame Frame;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
