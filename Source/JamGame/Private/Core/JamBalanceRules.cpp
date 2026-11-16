#include "Core/JamBalanceRules.h"

#include "UObject/UnrealType.h"

TArray<FString> JamBalanceRules::Problems(const FJamBalanceData& Balance)
{
	TArray<FString> Result;
	// Every number is a size, a speed, a time or an amount: zero means the line is missing.
	for (TFieldIterator<FFloatProperty> It(FJamBalanceData::StaticStruct()); It; ++It)
	{
		if (It->GetPropertyValue_InContainer(&Balance) <= 0.f)
		{
			Result.Add(FString::Printf(TEXT("%s must be positive"), *It->GetName()));
		}
	}
	if (Balance.ArenaRadius <= 2.f * (Balance.HeroRadius + Balance.BossRadius))
	{
		Result.Add(TEXT("ArenaRadius must leave room for both fighters side by side"));
	}
	const bool bThresholdsGoDown = Balance.StealThreshold1 < 1.f && Balance.StealThreshold2 < Balance.StealThreshold1 &&
		Balance.StealThreshold3 < Balance.StealThreshold2 && Balance.StealThreshold4 < Balance.StealThreshold3;
	if (!bThresholdsGoDown)
	{
		Result.Add(TEXT("StealThreshold1 to 4 must go down and start below 1"));
	}
	if (Balance.StealChoiceLock >= Balance.StealChoiceSeconds)
	{
		Result.Add(TEXT("StealChoiceLock must be shorter than StealChoiceSeconds: the card needs time to be answered"));
	}
	return Result;
}
