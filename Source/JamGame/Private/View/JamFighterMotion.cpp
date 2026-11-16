#include "View/JamFighterView.h"

#include "JamGame.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "View/JamPoseInstance.h"

// How a fighter's body follows the fight: its place and turn, and how its legs run under it (T-066, T-078).

namespace
{
// How fast the Publisher's body turns toward his facing, degrees per second (T-066).
constexpr float BossTurnRate = 540.f;
// T-078: the hips turn toward the move at most this far, the runs cross over this many degrees of the move angle,
// and the weights and the twist ease toward the angle at these rates (per second) so a flick of the cursor never pops.
constexpr float StrideMaxTwist = 60.f;
constexpr float StrideBlendDegrees = 16.f;
constexpr float StrideWeightRate = 6.f;
constexpr float StrideTwistRate = 360.f;
// Below this speed the move angle stays as it was: the legs keep their direction while they fade to the idle.
constexpr float StrideAngleFromSpeed = 20.f;
} // namespace

void AJamFighterView::InitRuns(TConstArrayView<FJamStrideClip> InDiagonals, float MoveSpeed, float MaxRate, float InUpperWalk)
{
	UpperWalk = InUpperWalk;
	Diagonals.Reset();
	for (const FJamStrideClip& Run : InDiagonals)
	{
		if (UAnimSequence* Sequence = Run.Asset ? LoadObject<UAnimSequence>(nullptr, Run.Asset) : nullptr)
		{
			Diagonals.Add(Sequence);
		}
	}
	// All four or none: a missing one would leave a hole in the circle of directions.
	if (Diagonals.Num() != InDiagonals.Num())
	{
		Diagonals.Reset();
	}
	MaxRunRate = MaxRate;
	RunSpeeds.Init(MoveSpeed, static_cast<int32>(EJamStride::Num));
	for (int32 Index = 0; Index < Diagonals.Num(); ++Index)
	{
		RunSpeeds[1 + Index] = InDiagonals[Index].Speed;
	}
	StrideWeights.Init(0.f, static_cast<int32>(EJamStride::Num));
	StrideWeights[static_cast<int32>(EJamStride::Forward)] = 1.f;
}

void AJamFighterView::Face(const FVector2D& Dir)
{
	SetActorRotation(FRotator(0.0, FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)), 0.0));
}

void AJamFighterView::Sync(const FJamFighter& Fighter, float Alpha, float Time, bool bSnap)
{
	const float Dt = FMath::Clamp(Time - LastSyncTime, 0.f, 0.1f);
	LastSyncTime = Time;
	const FVector2D Pos = FMath::Lerp(Fighter.PrevPos, Fighter.Pos, static_cast<double>(Alpha));
	const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(Fighter.Facing.Y, Fighter.Facing.X));
	// The hero turns at once to the cursor (Face); the Publisher's heavy body turns at a rate instead of snapping (T-066).
	const double LastYaw = GetActorRotation().Yaw;
	const double ShownYaw = Side == EJamSide::Boss && !bSnap ? FMath::FixedTurn(LastYaw, Yaw, BossTurnRate * Dt) : Yaw;
	SetActorLocationAndRotation(FVector(Pos.X, Pos.Y, 0.0), FRotator(0.0, ShownYaw, 0.0));
	const FVector2D Step = Fighter.Pos - Fighter.PrevPos;
	const float StepSpeed = Step.Size() * JamSim::StepsPerSecond;
	const float Moving = FMath::Clamp(StepSpeed / Speed, 0.f, 1.f);
	// The legs keep the last direction while the body slows to a stop (T-078).
	if (StepSpeed > StrideAngleFromSpeed)
	{
		StrideAngle = JamPoseRules::MoveAngle(
			Step, FVector2D(FMath::Cos(FMath::DegreesToRadians(ShownYaw)), FMath::Sin(FMath::DegreesToRadians(ShownYaw))));
	}
	const float YawRate = Dt > 0.f && !bSnap ? static_cast<float>(FMath::FindDeltaAngleDegrees(LastYaw, ShownYaw)) / Dt : 0.f;
	Animate(Moving, StrideAngle, YawRate, Time);
	if (Shown.bModel && !Shown.bArt)
	{
		AnimateRonin(Moving, Time);
	}
	UpdateGhosts(Time);
	const bool bFlashNow = Time < FlashUntil;
	if (bFlashNow != bFlashing)
	{
		bFlashing = bFlashNow;
		Repaint();
	}
}

void AJamFighterView::AnimateStride(FJamPoseFrame& Out, float Moving, float MoveAngle, float Dt)
{
	const bool bDiagonals = !Diagonals.IsEmpty();
	const FJamStride Target = JamPoseRules::StrideFor(MoveAngle, bDiagonals, StrideMaxTwist, StrideBlendDegrees);
	UAnimSequence* Forward = Clips[static_cast<int32>(EJamClip::Move)];
	const UAnimSequence* Runs[FJamPoseFrame::RunCount] = { Forward, nullptr, nullptr, nullptr, nullptr, Forward };
	for (int32 Index = 0; Index < Diagonals.Num(); ++Index)
	{
		Runs[1 + Index] = Diagonals[Index];
	}
	// The weights ease toward the angle's, then sum to one again; the legs run one shared cycle, its pace the blend
	// of what each run needs to keep up with the ground (stride warping by play rate).
	float Sum = 0.f;
	for (int32 Run = 0; Run < FJamPoseFrame::RunCount; ++Run)
	{
		StrideWeights[Run] = Runs[Run] ? FMath::FInterpConstantTo(StrideWeights[Run], Target.Weights[Run], Dt, StrideWeightRate) : 0.f;
		Sum += StrideWeights[Run];
	}
	const float GroundSpeed = Moving * Speed;
	float CyclesPerSecond = 0.f;
	for (int32 Run = 0; Run < FJamPoseFrame::RunCount; ++Run)
	{
		StrideWeights[Run] = Sum > 0.f ? StrideWeights[Run] / Sum : (Run == 0 ? 1.f : 0.f);
		const float Length = Runs[Run] ? Runs[Run]->GetPlayLength() : 0.f;
		if (Length > 0.f)
		{
			CyclesPerSecond += StrideWeights[Run] * JamPoseRules::StrideRate(GroundSpeed, RunSpeeds[Run], MaxRunRate) / Length;
		}
	}
	StridePhase = FMath::Fmod(StridePhase + Dt * CyclesPerSecond, 1.f);
	for (int32 Run = 0; Run < FJamPoseFrame::RunCount; ++Run)
	{
		Out.Runs[Run] = Runs[Run];
		// The run backwards plays its clock the other way.
		const float Phase = Run == static_cast<int32>(EJamStride::Back) ? 1.f - StridePhase : StridePhase;
		Out.RunTimes[Run] = Runs[Run] ? Phase * Runs[Run]->GetPlayLength() : 0.f;
		Out.RunWeights[Run] = StrideWeights[Run];
	}
	StrideTwist = FMath::FInterpConstantTo(StrideTwist, Target.Twist, Dt, StrideTwistRate);
	// A full-body action (the roll, the stun) turns the whole body its own way: the twist gives way to it.
	const float FullBody = Action.Clip != EJamClip::None && Action.bFullBody ? WeightOf(Action, LastTime) : 0.f;
	Out.Twist = StrideTwist * MoveWeight * (1.f - FullBody);
	Out.UpperWalk = UpperWalk;
	UE_LOG(LogJamGame, VeryVerbose,
		TEXT("Stride %s at %.2f s: angle %.0f, twist %.0f, weights F %.2f FL %.2f FR %.2f BL %.2f BR %.2f B %.2f"),
		Side == EJamSide::Hero ? TEXT("Hero") : TEXT("Publisher"), LastTime, MoveAngle, Out.Twist, StrideWeights[0], StrideWeights[1],
		StrideWeights[2], StrideWeights[3], StrideWeights[4], StrideWeights[5]);
}

FString AJamFighterView::DescribeStride() const
{
	const FVector FootL = Figure->GetSocketLocation(TEXT("foot_l"));
	const FVector FootR = Figure->GetSocketLocation(TEXT("foot_r"));
	const double Hips = Figure->GetSocketRotation(TEXT("pelvis")).Yaw;
	const double Chest = Figure->GetSocketRotation(TEXT("spine_03")).Yaw;
	return FString::Printf(TEXT("footl %.1f %.1f %.1f footr %.1f %.1f %.1f hips %.1f chest %.1f"), FootL.X, FootL.Y, FootL.Z, FootR.X,
		FootR.Y, FootR.Z, Hips, Chest);
}
