#include "Sim/JamLoadoutRules.h"

EJamAction JamLoadoutRules::ActionFor(EJamButton Button, EJamSide Side, const FJamLedger& Ledger)
{
	// Only the hero has the ball: it is what is left of him when a mechanic is gone.
	const EJamAction Fallback = Side == EJamSide::Hero ? EJamAction::Ball : EJamAction::None;
	switch (Button)
	{
		case EJamButton::Primary:
			return Ledger.Has(Side, EJamMechanic::Katana) ? EJamAction::Katana : Fallback;
		case EJamButton::Secondary:
			return Ledger.Has(Side, EJamMechanic::Shuriken) ? EJamAction::Shuriken : Fallback;
		case EJamButton::Ultimate:
			return Ledger.Has(Side, EJamMechanic::Ultimate) ? EJamAction::Ultimate : EJamAction::None;
		case EJamButton::Parry:
			return Ledger.Has(Side, EJamMechanic::Parry) ? EJamAction::Parry : EJamAction::None;
		default:
			return Ledger.Has(Side, EJamMechanic::Dash) ? EJamAction::Dash : EJamAction::None;
	}
}

void JamLoadoutRules::Give(FJamLedger& Ledger, EJamMechanic Mechanic, EJamSide NewOwner)
{
	Ledger.Owner[static_cast<int32>(Mechanic)] = NewOwner;
}

bool JamLoadoutRules::FirstOwned(const FJamLedger& Ledger, EJamSide Side, EJamMechanic& OutMechanic)
{
	for (int32 Index = 0; Index < JamSim::MechanicCount; ++Index)
	{
		if (Ledger.Owner[Index] == Side)
		{
			OutMechanic = static_cast<EJamMechanic>(Index);
			return true;
		}
	}
	return false;
}

TArray<EJamMechanic> JamLoadoutRules::Owned(const FJamLedger& Ledger, EJamSide Side)
{
	TArray<EJamMechanic> Result;
	for (int32 Index = 0; Index < JamSim::MechanicCount; ++Index)
	{
		if (Ledger.Owner[Index] == Side)
		{
			Result.Add(static_cast<EJamMechanic>(Index));
		}
	}
	return Result;
}

FJamBalanceData JamLoadoutRules::InHandsOf(EJamSide Side, const FJamBalanceData& Balance)
{
	FJamBalanceData Hands = Balance;
	if (Side == EJamSide::Boss)
	{
		Hands.KatanaWindUp = Balance.BossKatanaWindUp;
		Hands.KatanaDamage *= Balance.BossStolenDamage;
		Hands.KatanaFinisherDamage *= Balance.BossStolenDamage;
		Hands.ShurikenDamage = Balance.BossShurikenDamage * Balance.BossStolenDamage;
		Hands.ShurikenExplosionDamage *= Balance.BossStolenDamage;
		Hands.ShurikenSpeed *= Balance.BossStolenSpeed;
		Hands.ShurikenCooldown = Balance.BossShurikenCooldown * Balance.BossStolenCooldown * Balance.BossShurikenCooldownScale;
		Hands.DashCooldown *= Balance.BossStolenCooldown;
		// T-038: his Parry is a stance, the window is the hold after the raise (the raise is passed when he starts it).
		Hands.ParryWindow = Balance.BossParryHold;
		Hands.ParryCooldown = Balance.BossParryCooldown;
		Hands.ParryDamageScale = Balance.BossParryDamageScale;
		Hands.ParryReturnSpeed *= Balance.BossStolenSpeed;
		Hands.UltimateWindUp = Balance.BossUltimateWindUp;
		Hands.UltimateDamage = Balance.BossUltimateDamage;
	}
	return Hands;
}

const TCHAR* JamLoadoutRules::Name(EJamMechanic Mechanic)
{
	switch (Mechanic)
	{
		case EJamMechanic::Katana:
			return TEXT("KATANA");
		case EJamMechanic::Shuriken:
			return TEXT("SHURIKEN");
		case EJamMechanic::Ultimate:
			return TEXT("ULTIMATE");
		case EJamMechanic::Parry:
			return TEXT("PARRY");
		default:
			return TEXT("DASH");
	}
}
