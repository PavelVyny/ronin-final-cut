#include "Sim/JamCheckpointRules.h"

#include "Sim/JamFightRules.h"

FJamFight JamCheckpointRules::Capture(const FJamFight& Fight, const FJamBalanceData& Balance)
{
	// A fresh fight with what must survive a death: nothing in the air, no timers, the hero's health of the cut.
	FJamFight Saved = JamFightRules::Start(Balance, 0);
	Saved.Fighters[EJamSide::Hero].Health = RetryHealth(Fight.Fighters[EJamSide::Hero].Health, Balance);
	Saved.Fighters[EJamSide::Boss].Health = Fight.Fighters[EJamSide::Boss].Health;
	Saved.Ledger = Fight.Ledger;
	Saved.Steal.Done = Fight.Steal.Done;
	Saved.Steal.Floor = Fight.Steal.Floor;
	return Saved;
}

float JamCheckpointRules::RetryHealth(float HealthAtTheCut, const FJamBalanceData& Balance)
{
	// A cut made on the brink would make every retry hopeless: such a hero comes back with a fighting chance.
	if (HealthAtTheCut <= Balance.RetryLowHealthShare * Balance.HeroHealth)
	{
		return Balance.RetryLowHealthRestore * Balance.HeroHealth;
	}
	return FMath::Min(HealthAtTheCut, Balance.HeroHealth);
}

FJamFight JamCheckpointRules::Restore(const FJamFight& Saved, int32 Seed)
{
	FJamFight Fight = Saved;
	Fight.Stream.Initialize(Seed);
	return Fight;
}
