#include "View/JamPoseInstance.h"

#include "View/JamPoseRules.h"

#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimSequence.h"
#include "AnimationRuntime.h"

namespace
{
void Extract(const UAnimSequence& Sequence, float Time, bool bLoop, FPoseContext& Into)
{
	FAnimationPoseData Data(Into);
	Sequence.GetAnimationPose(Data, FAnimExtractContext(static_cast<double>(Time), false, FDeltaTimeRecord(), bLoop));
}

const FName PoseHipsBone = TEXT("pelvis");
const FName PoseChestBone = TEXT("spine_03");

struct FJamPoseProxy : public FAnimInstanceProxy
{
	explicit FJamPoseProxy(UAnimInstance* Instance)
		: FAnimInstanceProxy(Instance)
	{
	}

	FJamPoseFrame Frame;

	virtual void PreUpdate(UAnimInstance* Instance, float DeltaSeconds) override
	{
		FAnimInstanceProxy::PreUpdate(Instance, DeltaSeconds);
		Frame = CastChecked<UJamPoseInstance>(Instance)->Frame;
	}

	virtual bool Evaluate(FPoseContext& Output) override
	{
		if (Frame.bRefPose || !Frame.Idle)
		{
			Output.ResetToRefPose();
			return true;
		}
		Extract(*Frame.Idle, Frame.IdleTime, true, Output);
		if (Frame.MoveWeight > 0.f)
		{
			BlendRuns(Output);
		}
		if (Frame.Fading && Frame.FadingWeight > 0.f)
		{
			FPoseContext Fading(Output);
			Extract(*Frame.Fading, Frame.FadingTime, false, Fading);
			BlendAction(Output, Fading, Frame.FadingWeight, Frame.bFadingFullBody);
		}
		if (Frame.Action && Frame.ActionWeight > 0.f)
		{
			FPoseContext Action(Output);
			Extract(*Frame.Action, Frame.ActionTime, false, Action);
			BlendAction(Output, Action, Frame.ActionWeight, Frame.bFullBody);
		}
		if (Frame.Lean && Frame.LeanWeight > 0.f)
		{
			AddLean(Output);
		}
		if (Frame.MoveWeight > 0.f || !FMath::IsNearlyZero(Frame.TurnLean))
		{
			TwistAndLean(Output);
		}
		return true;
	}

	/**
	 * Degrees (about Z, in component space) the walk's hips and chest are turned off the plain forward run's: a
	 * diagonal run turns the whole body its way, and the chest has to come back to the aim (TwistAndLean).
	 */
	float HipsOffRun = 0.f;
	float ChestOffRun = 0.f;

	/** The yaw that brings Bone of Pose to where it is in Reference, in degrees. */
	static float YawTo(const FCompactPose& Reference, const FCompactPose& Pose, FCompactPoseBoneIndex Bone)
	{
		if (!Bone.IsValid())
		{
			return 0.f;
		}
		const FQuat Delta = ComponentRotation(Reference, Bone) * ComponentRotation(Pose, Bone).Inverse();
		return FMath::RadiansToDegrees(FMath::UnwindRadians(static_cast<float>(Delta.GetTwistAngle(FVector::UpVector))));
	}

	/** The directional runs, each by its weight, into one walk over the idle by MoveWeight (T-078). */
	void BlendRuns(FPoseContext& Output)
	{
		HipsOffRun = 0.f;
		ChestOffRun = 0.f;
		FPoseContext Walk(Output);
		float Taken = 0.f;
		for (int32 Run = 0; Run < FJamPoseFrame::RunCount; ++Run)
		{
			const float Weight = Frame.RunWeights[Run];
			if (!Frame.Runs[Run] || Weight <= 0.f)
			{
				continue;
			}
			if (Taken <= 0.f)
			{
				Extract(*Frame.Runs[Run], Frame.RunTimes[Run], true, Walk);
			}
			else
			{
				FPoseContext Next(Output);
				Extract(*Frame.Runs[Run], Frame.RunTimes[Run], true, Next);
				FAnimationPoseData Into(Walk);
				FAnimationRuntime::BlendTwoPosesTogetherInPlace(Into, FAnimationPoseData(Next), Taken / (Taken + Weight));
			}
			Taken += Weight;
		}
		if (Taken > 0.f && Frame.Runs[0] && Frame.RunWeights[0] < 1.f)
		{
			FPoseContext Forward(Output);
			Extract(*Frame.Runs[0], Frame.RunTimes[0], true, Forward);
			HipsOffRun = YawTo(Forward.Pose, Walk.Pose, BoneNamed(Output, PoseHipsBone));
			ChestOffRun = YawTo(Forward.Pose, Walk.Pose, BoneNamed(Output, PoseChestBone));
		}
		if (Taken > 0.f)
		{
			const TArray<bool, TInlineAllocator<64>> Upper = UpperBody(Output);
			for (const FCompactPoseBoneIndex Bone : Output.Pose.ForEachBoneIndex())
			{
				Output.Pose[Bone].BlendWith(Walk.Pose[Bone], Frame.MoveWeight * (Upper[Bone.GetInt()] ? Frame.UpperWalk : 1.f));
			}
		}
	}

	static FCompactPoseBoneIndex BoneNamed(const FPoseContext& Output, FName Name)
	{
		const FBoneContainer& Bones = Output.Pose.GetBoneContainer();
		const int32 MeshIndex = Bones.GetPoseBoneIndexForBoneName(Name);
		return MeshIndex == INDEX_NONE ? FCompactPoseBoneIndex(INDEX_NONE) : Bones.MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
	}

	/** The bone's rotation in component space, from the local rotations of its chain. */
	static FQuat ComponentRotation(const FCompactPose& Pose, FCompactPoseBoneIndex Bone)
	{
		FQuat Rotation = FQuat::Identity;
		for (FCompactPoseBoneIndex At = Bone; At.IsValid(); At = Pose.GetParentBoneIndex(At))
		{
			Rotation = Pose[At].GetRotation() * Rotation;
		}
		return Rotation;
	}

	/** Turns Bone, and everything under it, by Turn in component space about the bone's own joint. */
	static void TurnInComponent(FCompactPose& Pose, FCompactPoseBoneIndex Bone, const FQuat& Turn)
	{
		const FCompactPoseBoneIndex Parent = Pose.GetParentBoneIndex(Bone);
		const FQuat ParentRotation = Parent.IsValid() ? ComponentRotation(Pose, Parent) : FQuat::Identity;
		const FQuat Local = ParentRotation.Inverse() * Turn * ParentRotation * Pose[Bone].GetRotation();
		Pose[Bone].SetRotation(Local.GetNormalized());
	}

	/**
	 * T-078: the hips and the legs turn toward the move, the spine turns back over its bones so the chest keeps
	 * facing the aim; then the upper body leans into the turn. The model faces +Y in component space, Z up.
	 */
	void TwistAndLean(FPoseContext& Output) const
	{
		static const FName Spine[] = { TEXT("spine_01"), TEXT("spine_02"), PoseChestBone };
		FCompactPose& Pose = Output.Pose;
		TArray<FCompactPoseBoneIndex, TInlineAllocator<3>> SpineBones;
		for (const FName& Name : Spine)
		{
			const FCompactPoseBoneIndex Bone = BoneNamed(Output, Name);
			if (Bone.IsValid())
			{
				SpineBones.Add(Bone);
			}
		}
		const FCompactPoseBoneIndex Pelvis = BoneNamed(Output, PoseHipsBone);
		if (!Pelvis.IsValid() || SpineBones.IsEmpty())
		{
			return;
		}
		// T-084: an action owns the chest (a throw while backpedalling twisted it away): the warp fades as it comes in.
		const float Warp = JamPoseRules::WarpWeight(Frame.ActionWeight, Frame.bFullBody);
		const float TwistRadians = FMath::DegreesToRadians(Frame.Twist) * Warp;
		TurnInComponent(Pose, Pelvis, FQuat(FVector::UpVector, TwistRadians));
		// The chest comes back from the twist and from the diagonal run's own turn.
		const float OffRun = Frame.MoveWeight * ChestOffRun * Warp;
		const FQuat Back(FVector::UpVector, (FMath::DegreesToRadians(OffRun) - TwistRadians) / SpineBones.Num());
		for (const FCompactPoseBoneIndex Bone : SpineBones)
		{
			TurnInComponent(Pose, Bone, Back);
		}
		// A lean to the right brings the head toward component -X (the model's right) about its forward +Y axis.
		TurnInComponent(Pose, SpineBones[0], FQuat(FVector::RightVector, -FMath::DegreesToRadians(Frame.TurnLean)));
	}

	/** Which bones are the upper body: UpperBodyBone and everything under it. Parents come first in a compact pose. */
	static TArray<bool, TInlineAllocator<64>> UpperBody(const FPoseContext& Output)
	{
		const FBoneContainer& Bones = Output.Pose.GetBoneContainer();
		const FReferenceSkeleton& RefSkeleton = Bones.GetReferenceSkeleton();
		TArray<bool, TInlineAllocator<64>> Upper;
		Upper.SetNumZeroed(Output.Pose.GetNumBones());
		for (const FCompactPoseBoneIndex Bone : Output.Pose.ForEachBoneIndex())
		{
			const FCompactPoseBoneIndex Parent = Output.Pose.GetParentBoneIndex(Bone);
			const FName Name = RefSkeleton.GetBoneName(Bones.MakeMeshPoseIndex(Bone).GetInt());
			Upper[Bone.GetInt()] = Name == UJamPoseInstance::UpperBodyBone || (Parent.IsValid() && Upper[Parent.GetInt()]);
		}
		return Upper;
	}

	/** The hit lean: the upper body turns by the clip's peak against its first frame, an additive rotation only. */
	void AddLean(FPoseContext& Output) const
	{
		FPoseContext Peak(Output);
		Extract(*Frame.Lean, Frame.LeanTime, false, Peak);
		FPoseContext Rest(Output);
		Extract(*Frame.Lean, 0.f, false, Rest);
		const TArray<bool, TInlineAllocator<64>> Upper = UpperBody(Output);
		for (const FCompactPoseBoneIndex Bone : Output.Pose.ForEachBoneIndex())
		{
			if (!Upper[Bone.GetInt()])
			{
				continue;
			}
			const FQuat Delta = Peak.Pose[Bone].GetRotation() * Rest.Pose[Bone].GetRotation().Inverse();
			const FQuat Turn = FQuat::Slerp(FQuat::Identity, Delta, Frame.LeanWeight);
			Output.Pose[Bone].SetRotation((Turn * Output.Pose[Bone].GetRotation()).GetNormalized());
		}
	}

	/** The action over the upper body, and over the legs as far as they are not walking. */
	void BlendAction(FPoseContext& Output, const FPoseContext& Action, float Weight, bool bFullBody) const
	{
		const float Legs = bFullBody ? 1.f : 1.f - Frame.MoveWeight;
		const TArray<bool, TInlineAllocator<64>> Upper = UpperBody(Output);
		for (const FCompactPoseBoneIndex Bone : Output.Pose.ForEachBoneIndex())
		{
			Output.Pose[Bone].BlendWith(Action.Pose[Bone], Weight * (Upper[Bone.GetInt()] ? 1.f : Legs));
		}
	}
};
} // namespace

FAnimInstanceProxy* UJamPoseInstance::CreateAnimInstanceProxy()
{
	return new FJamPoseProxy(this);
}
