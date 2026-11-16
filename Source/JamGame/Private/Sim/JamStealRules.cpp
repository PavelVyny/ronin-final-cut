#include "Sim/JamStealRules.h"

#include "Sim/JamFightOps.h"
#include "Sim/JamLoadoutRules.h"

namespace
{
// Options of a card that is a choice.
constexpr int32 OptionsOnACard = 2;
} // namespace

float JamStealRules::ThresholdHealth(int32 Index, const FJamBalanceData& Balance)
{
	const float Shares[ThresholdCount] = { Balance.StealThreshold1, Balance.StealThreshold2, Balance.StealThreshold3,
		Balance.StealThreshold4 };
	return Index >= 0 && Index < ThresholdCount ? Shares[Index] * Balance.BossHealth : 0.f;
}

float JamStealRules::ClampToThreshold(float Health, float Floor)
{
	return FMath::Max(Health, Floor);
}

TArray<EJamMechanic> JamStealRules::Offer(const FJamLedger& Ledger, bool bTakesAll, FRandomStream& Stream)
{
	TArray<EJamMechanic> Result = JamLoadoutRules::Owned(Ledger, EJamSide::Hero);
	if (bTakesAll)
	{
		return Result;
	}
	// T-051: the Katana is the hero's way to deal damage, so only the final note takes it.
	Result.Remove(EJamMechanic::Katana);
	for (int32 Index = Result.Num() - 1; Index > 0; --Index)
	{
		Result.Swap(Index, Stream.RandRange(0, Index));
	}
	Result.SetNum(FMath::Min(Result.Num(), OptionsOnACard));
	return Result;
}

void JamStealRules::Open(FJamFight& Fight, const FJamBalanceData& Balance)
{
	FJamStealState& Steal = Fight.Steal;
	Steal.bTakesAll = Steal.Done >= ThresholdCount - 1;
	Steal.Offer = Offer(Fight.Ledger, Steal.bTakesAll, Fight.Stream);
	if (Steal.Offer.IsEmpty())
	{
		++Steal.Done;
		Steal.Floor = ThresholdHealth(Steal.Done, Balance);
		return;
	}
	JamFightOps::ClearStage(Fight);
	Steal.StartTick = Fight.Tick;
	Steal.OpenTick = Fight.Tick + JamSim::Ticks(Balance.StealChoiceLock);
	Steal.EndTick = Fight.Tick + JamSim::Ticks(Balance.StealChoiceSeconds);
	Fight.Phase = EJamPhase::Steal;
	const FVector2D BossPos = Fight.Fighters[EJamSide::Boss].Pos;
	JamFightOps::AddEvent(Fight, EJamEvent::StealOpened, EJamSide::Boss, 0, BossPos, static_cast<float>(Steal.Offer.Num()));
}

void JamStealRules::Advance(FJamFight& Fight, int32 Choice, const FJamBalanceData& Balance)
{
	const FJamStealState& Steal = Fight.Steal;
	if (Fight.Tick >= Steal.EndTick)
	{
		Resolve(Fight, 0, Balance);
	}
	else if (Fight.Tick >= Steal.OpenTick && Steal.Offer.IsValidIndex(Choice))
	{
		Resolve(Fight, Choice, Balance);
	}
}

void JamStealRules::Resolve(FJamFight& Fight, int32 Choice, const FJamBalanceData& Balance)
{
	FJamStealState& Steal = Fight.Steal;
	const int32 Picked = Steal.Offer.IsValidIndex(Choice) ? Choice : 0;
	const FVector2D BossPos = Fight.Fighters[EJamSide::Boss].Pos;
	for (int32 Index = 0; Index < Steal.Offer.Num(); ++Index)
	{
		if (Steal.bTakesAll || Index == Picked)
		{
			const EJamMechanic Mechanic = Steal.Offer[Index];
			JamLoadoutRules::Give(Fight.Ledger, Mechanic, EJamSide::Boss);
			JamFightOps::AddEvent(Fight, EJamEvent::StealResolved, EJamSide::Boss, 0, BossPos, static_cast<float>(Mechanic));
		}
	}
	Steal.Offer.Reset();
	Steal.bTakesAll = false;
	++Steal.Done;
	Steal.Floor = ThresholdHealth(Steal.Done, Balance);
	Fight.Phase = EJamPhase::Fight;
	// He looks at what he got before he uses it: the player gets a breath after the card.
	Fight.Boss.BusyUntilTick = Fight.Tick + JamSim::Ticks(Balance.BossRecovery);
}

float JamStealRules::TimeLeft(const FJamStealState& Steal, float Tick)
{
	const float Whole = static_cast<float>(FMath::Max(1, Steal.EndTick - Steal.StartTick));
	return FMath::Clamp((Steal.EndTick - Tick) / Whole, 0.f, 1.f);
}
