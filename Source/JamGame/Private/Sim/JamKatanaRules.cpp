#include "Sim/JamKatanaRules.h"

#include "Sim/JamFightOps.h"

namespace
{
constexpr int32 StrikesInSeries = 3;
} // namespace

bool JamKatanaRules::InArc(
	const FVector2D& From, const FVector2D& Facing, const FVector2D& TargetPos, float TargetRadius, float Range, float HalfArcDeg)
{
	const FVector2D To = TargetPos - From;
	const double Distance = To.Size();
	if (Distance <= TargetRadius)
	{
		// The attacker stands inside the target: there is no direction to miss in.
		return true;
	}
	if (Distance > Range + TargetRadius)
	{
		return false;
	}
	const double Cos = FMath::Clamp(FVector2D::DotProduct(Facing.GetSafeNormal(), To / Distance), -1.0, 1.0);
	const double Angle = FMath::RadiansToDegrees(FMath::Acos(Cos));
	const double EdgeAngle = FMath::RadiansToDegrees(FMath::Asin(TargetRadius / Distance));
	return Angle <= HalfArcDeg + EdgeAngle;
}

int32 JamKatanaRules::NextStrike(const FJamKatanaState& State, int32 Tick)
{
	if (Tick < State.BusyUntilTick)
	{
		return 0;
	}
	const bool bContinues = State.Strike > 0 && State.Strike < StrikesInSeries && Tick <= State.ChainUntilTick;
	return bContinues ? State.Strike + 1 : 1;
}

FJamKatanaState JamKatanaRules::Begin(int32 Strike, int32 Tick, const FJamBalanceData& Balance)
{
	FJamKatanaState State;
	State.Strike = Strike;
	State.HitTick = Tick + JamSim::Ticks(Balance.KatanaWindUp);
	State.BusyUntilTick = State.HitTick + JamSim::Ticks(Balance.KatanaRecovery);
	State.ChainUntilTick = State.BusyUntilTick + JamSim::Ticks(Balance.KatanaChainWindow);
	State.bHitPending = true;
	return State;
}

bool JamKatanaRules::IsFinisher(int32 Strike)
{
	return Strike >= StrikesInSeries;
}

float JamKatanaRules::Damage(int32 Strike, const FJamBalanceData& Balance)
{
	return IsFinisher(Strike) ? Balance.KatanaFinisherDamage : Balance.KatanaDamage;
}

void JamKatanaRules::Press(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	const int32 Strike = NextStrike(Self.Katana, Fight.Tick);
	if (Strike == 0)
	{
		return;
	}
	Self.Katana = Begin(Strike, Fight.Tick, Balance);
	JamFightOps::Shift(Fight, Side, Self.Facing * Balance.KatanaStep, Balance.ArenaRadius);
	JamFightOps::AddEvent(Fight, EJamEvent::Swing, Side, 0, Self.Pos, static_cast<float>(Strike));
}

void JamKatanaRules::Land(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	if (!Self.Katana.bHitPending || Fight.Tick < Self.Katana.HitTick)
	{
		return;
	}
	Self.Katana.bHitPending = false;
	const EJamSide TargetSide = JamSim::Other(Side);
	const FJamFighter& Target = Fight.Fighters[TargetSide];
	if (InArc(Self.Pos, Self.Facing, Target.Pos, Target.Radius, Balance.KatanaRange, Balance.KatanaHalfArc))
	{
		JamFightOps::Hurt(Fight, TargetSide, Damage(Self.Katana.Strike, Balance), Target.Pos);
	}
}
