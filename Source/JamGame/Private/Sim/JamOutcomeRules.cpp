#include "Sim/JamOutcomeRules.h"

#include "Sim/JamFightOps.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamStealRules.h"
#include "Sim/JamUltimateRules.h"

namespace
{
/** The hero's Ultimate cut the boss on this step: on a stunned boss that is the final blow, nothing else is. */
bool WasFinalBlow(const FJamFight& Fight)
{
	for (const FJamEvent& Event : Fight.Events)
	{
		if (Event.Type == EJamEvent::Hit && Event.Side == EJamSide::Boss && Event.ProjectileId == JamUltimateRules::SlashId)
		{
			return true;
		}
	}
	return false;
}

/** The boss is down: everything goes back to the hero, his meter is full and the stage is cleared for the last blow. */
void Stun(FJamFight& Fight)
{
	for (int32 Index = 0; Index < JamSim::MechanicCount; ++Index)
	{
		JamLoadoutRules::Give(Fight.Ledger, static_cast<EJamMechanic>(Index), EJamSide::Hero);
	}
	Fight.Fighters[EJamSide::Hero].Ultimate.Meter = 1.f;
	JamFightOps::ClearStage(Fight);
	Fight.Phase = EJamPhase::Stunned;
	JamFightOps::AddEvent(Fight, EJamEvent::Stunned, EJamSide::Boss, 0, Fight.Fighters[EJamSide::Boss].Pos, 0.f);
}
} // namespace

EJamPhase JamOutcomeRules::Next(EJamPhase Phase, float HeroHealth, float BossHealth, float StealFloor, bool bFinalBlow)
{
	if (Phase == EJamPhase::Stunned)
	{
		return bFinalBlow ? EJamPhase::Won : EJamPhase::Stunned;
	}
	if (Phase != EJamPhase::Fight)
	{
		return Phase;
	}
	if (HeroHealth <= 0.f)
	{
		return EJamPhase::HeroDead;
	}
	if (BossHealth <= 0.f)
	{
		return EJamPhase::Stunned;
	}
	return StealFloor > 0.f && BossHealth <= StealFloor ? EJamPhase::Steal : EJamPhase::Fight;
}

void JamOutcomeRules::Settle(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const float HeroHealth = Fight.Fighters[EJamSide::Hero].Health;
	const float BossHealth = Fight.Fighters[EJamSide::Boss].Health;
	const EJamPhase Phase = Next(Fight.Phase, HeroHealth, BossHealth, Fight.Steal.Floor, WasFinalBlow(Fight));
	if (Phase == Fight.Phase)
	{
		return;
	}
	if (Phase == EJamPhase::Steal)
	{
		JamStealRules::Open(Fight, Balance);
	}
	else if (Phase == EJamPhase::Stunned)
	{
		Stun(Fight);
	}
	else
	{
		Fight.Phase = Phase;
	}
}
