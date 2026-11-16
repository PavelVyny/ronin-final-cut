#include "Sim/JamUltimateRules.h"

#include "Sim/JamFightOps.h"
#include "Sim/JamLoadoutRules.h"

bool JamUltimateRules::OnLine(
	const FVector2D& Origin, const FVector2D& Dir, const FVector2D& TargetPos, float TargetRadius, float Range, float HalfWidth)
{
	const FVector2D Along = Dir.GetSafeNormal();
	const FVector2D ToTarget = TargetPos - Origin;
	const double Forward = FVector2D::DotProduct(ToTarget, Along);
	const double Sideways = FMath::Abs(FVector2D::CrossProduct(Along, ToTarget));
	return Forward >= -TargetRadius && Forward <= Range + TargetRadius && Sideways <= HalfWidth + TargetRadius;
}

void JamUltimateRules::Charge(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const bool bWasFull[] = { Fight.Fighters[EJamSide::Hero].Ultimate.Meter >= 1.f, Fight.Fighters[EJamSide::Boss].Ultimate.Meter >= 1.f };
	for (const FJamEvent& Event : Fight.Events)
	{
		// Only the hero's hits fill a meter: the Publisher's runs on a clock.
		if (Event.Type == EJamEvent::Hit && Event.Side == EJamSide::Boss)
		{
			FJamUltimateState& Ultimate = Fight.Fighters[EJamSide::Hero].Ultimate;
			Ultimate.Meter = FMath::Min(1.f, Ultimate.Meter + Event.Amount * Balance.UltimateGainPerDamage);
		}
	}
	// T-038: a 100-health hero would never fill his meter by damage, so the stolen Ultimate fills by the seconds of fight.
	FJamUltimateState& Clock = Fight.Fighters[EJamSide::Boss].Ultimate;
	if (Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Ultimate) && !Clock.bPending)
	{
		Clock.Meter = Clock.Meter + 1.f / JamSim::Ticks(Balance.BossUltimateInterval);
		// The sum of the steps drifts under 1 by a rounding: the last step fills it.
		Clock.Meter = Clock.Meter > 1.f - UE_KINDA_SMALL_NUMBER ? 1.f : Clock.Meter;
	}
	// Announced after the loop: a new event would move the array the loop walks.
	for (const EJamSide Side : { EJamSide::Hero, EJamSide::Boss })
	{
		const FJamFighter& Self = Fight.Fighters[Side];
		if (!bWasFull[static_cast<int32>(Side)] && Self.Ultimate.Meter >= 1.f)
		{
			JamFightOps::AddEvent(Fight, EJamEvent::UltimateReady, Side, 0, Self.Pos, 0.f);
		}
	}
}

void JamUltimateRules::Press(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	FJamUltimateState& Ultimate = Self.Ultimate;
	const bool bOwns = JamLoadoutRules::ActionFor(EJamButton::Ultimate, Side, Fight.Ledger) == EJamAction::Ultimate;
	if (!bOwns || Ultimate.bPending || Ultimate.Meter < 1.f)
	{
		return;
	}
	Ultimate.bPending = true;
	Ultimate.SlashTick = Fight.Tick + JamSim::Ticks(Balance.UltimateWindUp);
	Ultimate.Dir = Self.Facing.GetSafeNormal();
	JamFightOps::AddEvent(Fight, EJamEvent::UltimateStarted, Side, 0, Self.Pos, 0.f);
}

FVector2D JamUltimateRules::TurnToward(const FVector2D& From, const FVector2D& To, double MaxDegrees)
{
	const FVector2D Start = From.GetSafeNormal();
	const FVector2D Goal = To.GetSafeNormal();
	if (Goal.IsNearlyZero())
	{
		return Start;
	}
	const double Degrees = FMath::RadiansToDegrees(FMath::Atan2(FVector2D::CrossProduct(Start, Goal), FVector2D::DotProduct(Start, Goal)));
	return FMath::Abs(Degrees) <= MaxDegrees ? Goal : Start.GetRotated(FMath::Sign(Degrees) * MaxDegrees);
}

void JamUltimateRules::Land(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	FJamUltimateState& Ultimate = Self.Ultimate;
	if (!Ultimate.bPending)
	{
		return;
	}
	// The Publisher's line stays where he pressed it (T-038): only the hero's follows the aim.
	if (Side == EJamSide::Hero)
	{
		Ultimate.Dir = TurnToward(Ultimate.Dir, Self.Facing, Balance.UltimateTurnRate * JamSim::StepDt);
	}
	if (Fight.Tick < Ultimate.SlashTick)
	{
		return;
	}
	Ultimate.bPending = false;
	Ultimate.Meter = 0.f;
	const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(Ultimate.Dir.Y, Ultimate.Dir.X));
	JamFightOps::AddEvent(Fight, EJamEvent::UltimateSlash, Side, 0, Self.Pos, static_cast<float>(Yaw));
	const EJamSide TargetSide = JamSim::Other(Side);
	const FJamFighter& Target = Fight.Fighters[TargetSide];
	if (OnLine(Self.Pos, Ultimate.Dir, Target.Pos, Target.Radius, Balance.UltimateRange, Balance.UltimateHalfWidth))
	{
		JamFightOps::Hurt(Fight, TargetSide, Balance.UltimateDamage, Target.Pos, SlashId);
	}
}
