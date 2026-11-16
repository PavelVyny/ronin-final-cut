#include "View/JamPoseRules.h"

namespace
{
/** Seconds from the fight's step to a later one. */
float SecondsTo(const FJamFight& Fight, int32 Tick)
{
	return FMath::Max(0, Tick - Fight.Tick) * JamSim::StepDt;
}

/** What started an action: a higher one wins when a step starts several. */
enum class EWhy : int32
{
	Nothing,
	Throw,
	Parry,
	Strike,
	Ultimate,
	Slam,
	Stun
};

/** The best cue of a step so far and why it was started. */
struct FBest
{
	FJamCue Cue;
	EWhy Why = EWhy::Nothing;

	void Offer(EWhy InWhy, EJamClip Clip, float Lead, bool bFullBody = false)
	{
		if (InWhy > Why)
		{
			Why = InWhy;
			Cue.Clip = Clip;
			Cue.Lead = Lead;
			Cue.bFullBody = bFullBody;
		}
	}
};

/** A throw of Side's: the hero's shurikens and balls (T-065), the Publisher's memo fans and the shurikens he stole. */
bool IsThrownBy(const FJamFight& Fight, EJamSide Side, int32 ProjectileId)
{
	const FJamProjectile* Shot =
		Fight.Projectiles.FindByPredicate([ProjectileId](const FJamProjectile& P) { return P.Id == ProjectileId; });
	return Shot && (Side == EJamSide::Hero || Shot->Kind != EJamProjectile::Ball);
}

/** What one event of Side's own starts on Side's body. */
void OfferEvent(FBest& Best, const FJamFight& Fight, EJamSide Side, const FJamEvent& Event)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	switch (Event.Type)
	{
		case EJamEvent::Swing:
			// The finisher is the first slash again: the second one swings back.
			Best.Offer(EWhy::Strike, FMath::RoundToInt32(Event.Amount) == 2 ? EJamClip::StrikeB : EJamClip::StrikeA,
				SecondsTo(Fight, Me.Katana.HitTick));
			break;
		case EJamEvent::ParryStarted:
			Best.Offer(EWhy::Parry, EJamClip::Parry, SecondsTo(Fight, Me.Parry.ActiveFromTick));
			break;
		case EJamEvent::UltimateStarted:
			Best.Offer(
				EWhy::Ultimate, Side == EJamSide::Boss ? EJamClip::Heavy : EJamClip::StrikeA, SecondsTo(Fight, Me.Ultimate.SlashTick));
			break;
		case EJamEvent::ProjectileSpawned:
			if (IsThrownBy(Fight, Side, Event.ProjectileId))
			{
				Best.Offer(EWhy::Throw, Side == EJamSide::Hero ? EJamClip::Throw : EJamClip::Heavy, 0.f);
			}
			break;
		case EJamEvent::Stunned:
			Best.Offer(EWhy::Stun, EJamClip::Hit, 0.f, true);
			break;
		default:
			break;
	}
}
} // namespace

FJamCue JamPoseRules::CueFor(const FJamFight& Fight, EJamSide Side, TConstArrayView<FJamEvent> Events)
{
	FBest Best;
	for (const FJamEvent& Event : Events)
	{
		if (Event.Side == Side)
		{
			OfferEvent(Best, Fight, Side, Event);
		}
	}
	// His slam starts with its circles: the first of the new ones is where the blow lands.
	int32 FirstCircle = MAX_int32;
	for (const FJamHazard& Hazard : Fight.Hazards)
	{
		if (Hazard.Owner == Side && Side == EJamSide::Boss && Hazard.StartTick == Fight.Tick)
		{
			FirstCircle = FMath::Min(FirstCircle, Hazard.HitTick);
		}
	}
	if (FirstCircle != MAX_int32)
	{
		Best.Offer(EWhy::Slam, EJamClip::Slam, SecondsTo(Fight, FirstCircle), true);
	}
	return Best.Cue;
}

bool JamPoseRules::IsDown(const FJamFight& Fight, EJamSide Side)
{
	return Side == EJamSide::Hero ? Fight.Phase == EJamPhase::HeroDead : Fight.Phase == EJamPhase::Won;
}

bool JamPoseRules::Replaces(EJamClip Next, EJamClip Playing)
{
	if (Playing == EJamClip::Death)
	{
		return false;
	}
	if (Next == EJamClip::Hit)
	{
		return Playing == EJamClip::None || Playing == EJamClip::Hit;
	}
	return true;
}

FJamPlayback JamPoseRules::Fit(float Impact, float Lead, float MinRate, float MaxRate)
{
	FJamPlayback Out;
	if (Lead <= 0.f)
	{
		Out.From = Impact;
		return Out;
	}
	Out.Rate = FMath::Clamp(Impact / Lead, MinRate, MaxRate);
	Out.From = FMath::Max(0.f, Impact - Lead * Out.Rate);
	return Out;
}

float JamPoseRules::Weight(float Age, float Remaining, float Blend)
{
	if (Blend <= 0.f)
	{
		return Remaining > 0.f ? 1.f : 0.f;
	}
	return FMath::Clamp(FMath::Min(Age, Remaining) / Blend, 0.f, 1.f);
}

bool JamPoseRules::Shakes(const FJamFight& Fight, EJamSide Side, TConstArrayView<FJamEvent> Events, float StaggerFrom)
{
	return Events.ContainsByPredicate([Side, StaggerFrom](const FJamEvent& Event)
		{ return Event.Type == EJamEvent::Hit && Event.Side == Side && (Side == EJamSide::Hero || Event.Amount >= StaggerFrom); });
}

float JamPoseRules::LeanWeight(float Age, float Rise, float Life)
{
	if (Age <= 0.f || Age >= Life)
	{
		return 0.f;
	}
	if (Age < Rise)
	{
		return Age / Rise;
	}
	// Eases back out: fast at first, settling at the end.
	const float Back = (Age - Rise) / FMath::Max(Life - Rise, UE_SMALL_NUMBER);
	return FMath::Square(1.f - Back);
}

int32 JamPoseRules::GhostsBy(float Age, int32 Count, float Gap)
{
	if (Age < 0.f)
	{
		return 0;
	}
	return FMath::Min(Count, 1 + FMath::FloorToInt32(Age / FMath::Max(Gap, UE_SMALL_NUMBER)));
}

float JamPoseRules::GhostAlpha(float Age, float Life)
{
	return FMath::Clamp(1.f - Age / FMath::Max(Life, UE_SMALL_NUMBER), 0.f, 1.f);
}

float JamPoseRules::StrideSign(const FVector2D& Move, const FVector2D& Facing)
{
	// Backwards only well behind the facing: a run turned 100 degrees off still reads better forwards.
	constexpr double BackwardFrom = -0.35;
	const FVector2D Dir = Move.GetSafeNormal();
	return FVector2D::DotProduct(Dir, Facing.GetSafeNormal()) < BackwardFrom ? -1.f : 1.f;
}

float JamPoseRules::MoveAngle(const FVector2D& Move, const FVector2D& Facing)
{
	if (Move.IsNearlyZero() || Facing.IsNearlyZero())
	{
		return 0.f;
	}
	const double Cross = Facing.X * Move.Y - Facing.Y * Move.X;
	const double Dot = FVector2D::DotProduct(Facing, Move);
	const float Degrees = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(Cross, Dot)));
	return Degrees <= -180.f ? 180.f : Degrees;
}

namespace
{
/** A directional run, the angle it runs at and where its sector of move angles ends (it starts where the last ended). */
struct FStrideSector
{
	EJamStride Run;
	float Anchor;
	float Until;
};

// T-078: the forward diagonals carry the strafes (the library has no plain strafe), up to 105 degrees off the facing,
// so a sidestep never sits on the edge between two runs; the back diagonals go up to 135, the run backwards beyond
// (T-080: at full speed the back diagonals are quick, skittery steps, so a backpedal is the plain run reversed).
const FStrideSector StrideDiagonals[] = {
	{ EJamStride::Back, -180.f, -135.f },
	{ EJamStride::BackLeft, -135.f, -105.f },
	{ EJamStride::ForwardLeft, -45.f, -22.5f },
	{ EJamStride::Forward, 0.f, 22.5f },
	{ EJamStride::ForwardRight, 45.f, 105.f },
	{ EJamStride::BackRight, 135.f, 135.f },
	{ EJamStride::Back, 180.f, 180.f },
};
// Without the diagonals the run turns forwards up to 110 degrees off (StrideSign's edge), backwards beyond.
const FStrideSector StrideStraight[] = {
	{ EJamStride::Back, -180.f, -110.f },
	{ EJamStride::Forward, 0.f, 110.f },
	{ EJamStride::Back, 180.f, 180.f },
};
} // namespace

FJamStride JamPoseRules::StrideFor(float Angle, bool bDiagonals, float MaxTwist, float BlendDegrees)
{
	const TConstArrayView<FStrideSector> Sectors =
		bDiagonals ? TConstArrayView<FStrideSector>(StrideDiagonals) : TConstArrayView<FStrideSector>(StrideStraight);
	const float A = FMath::Clamp(Angle, -180.f, 180.f);
	int32 Index = 0;
	while (Index < Sectors.Num() - 1 && A > Sectors[Index].Until)
	{
		++Index;
	}
	// Near a sector's edge the next or the previous run crosses in over BlendDegrees, half on each side of the edge.
	const float Half = 0.5f * FMath::Max(BlendDegrees, UE_SMALL_NUMBER);
	int32 Other = INDEX_NONE;
	float OtherWeight = 0.f;
	if (Index < Sectors.Num() - 1 && Sectors[Index].Until - A < Half)
	{
		Other = Index + 1;
		OtherWeight = 0.5f * (1.f - (Sectors[Index].Until - A) / Half);
	}
	else if (Index > 0 && A - Sectors[Index - 1].Until < Half)
	{
		Other = Index - 1;
		OtherWeight = 0.5f * (1.f - (A - Sectors[Index - 1].Until) / Half);
	}
	FJamStride Stride;
	Stride.Weights[static_cast<int32>(EJamStride::Forward)] = 0.f;
	Stride.Weights[static_cast<int32>(Sectors[Index].Run)] += 1.f - OtherWeight;
	float Anchor = (1.f - OtherWeight) * Sectors[Index].Anchor;
	if (Other != INDEX_NONE)
	{
		Stride.Weights[static_cast<int32>(Sectors[Other].Run)] += OtherWeight;
		Anchor += OtherWeight * Sectors[Other].Anchor;
	}
	// The hips turn the rest of the way from what the runs show to where the body goes.
	Stride.Twist = FMath::Clamp(A - Anchor, -MaxTwist, MaxTwist);
	return Stride;
}

float JamPoseRules::StrideRate(float GroundSpeed, float ClipSpeed, float MaxRate)
{
	return ClipSpeed > 0.f ? FMath::Clamp(GroundSpeed / ClipSpeed, 0.f, MaxRate) : 0.f;
}

float JamPoseRules::TurnLean(float YawRate, float FullAtRate, float MaxLean)
{
	return MaxLean * FMath::Clamp(YawRate / FMath::Max(FullAtRate, UE_SMALL_NUMBER), -1.f, 1.f);
}

float JamPoseRules::WarpWeight(float ActionWeight, bool bFullBody)
{
	return bFullBody ? 0.f : 1.f - FMath::Clamp(ActionWeight, 0.f, 1.f);
}

float JamPoseRules::ShowOffTurn(float Turn, float ClipWeight)
{
	return Turn * FMath::Clamp(ClipWeight, 0.f, 1.f);
}

float JamPoseRules::LoopTime(float Time, float Length)
{
	if (Length <= 0.f)
	{
		return 0.f;
	}
	const float Wrapped = FMath::Fmod(Time, Length);
	return Wrapped < 0.f ? Wrapped + Length : Wrapped;
}
