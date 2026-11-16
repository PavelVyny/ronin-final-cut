#include "Sim/JamDashRules.h"

FVector2D JamDashRules::Direction(const FVector2D& Move, const FVector2D& Facing)
{
	return Move.IsNearlyZero() ? Facing.GetSafeNormal() : Move.GetSafeNormal();
}

FJamDashState JamDashRules::Start(const FVector2D& Dir, int32 Tick, const FJamBalanceData& Balance)
{
	const int32 Steps = JamSim::Ticks(Balance.DashDuration);
	FJamDashState State;
	State.Dir = Dir;
	State.StepLength = Balance.DashDistance / Steps;
	State.EndTick = Tick + Steps;
	State.InvulnerableUntilTick = Tick + JamSim::Ticks(Balance.DashInvulnerable);
	// A cooldown shorter than the dash itself would start a dash inside a dash.
	State.ReadyTick = FMath::Max(State.EndTick, Tick + JamSim::Ticks(Balance.DashCooldown));
	return State;
}

bool JamDashRules::CanStart(const FJamDashState& State, int32 Tick)
{
	return Tick >= State.ReadyTick;
}

bool JamDashRules::IsDashing(const FJamDashState& State, int32 Tick)
{
	return Tick < State.EndTick;
}

bool JamDashRules::IsInvulnerable(const FJamDashState& State, int32 Tick)
{
	return Tick < State.InvulnerableUntilTick;
}
