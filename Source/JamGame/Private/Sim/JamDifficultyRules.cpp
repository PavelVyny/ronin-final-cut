#include "Sim/JamDifficultyRules.h"

EJamDifficulty JamDifficultyRules::FromIndex(int32 Index)
{
	return Index >= 0 && Index < Count ? static_cast<EJamDifficulty>(Index) : EJamDifficulty::Normal;
}

FJamDifficultyScales JamDifficultyRules::ScalesOf(const FJamBalanceData& Balance, EJamDifficulty Difficulty)
{
	FJamDifficultyScales Scales;
	switch (Difficulty)
	{
		case EJamDifficulty::Easy:
			Scales = { Balance.EasyHeroHealthScale, Balance.EasyHeroDamageScale, Balance.EasyHealScale, Balance.EasyBossDamageScale };
			break;
		case EJamDifficulty::Hard:
			Scales = { Balance.HardHeroHealthScale, Balance.HardHeroDamageScale, Balance.HardHealScale, Balance.HardBossDamageScale };
			break;
		default:
			Scales = { Balance.NormalHeroHealthScale, Balance.NormalHeroDamageScale, Balance.NormalHealScale,
				Balance.NormalBossDamageScale };
			break;
	}
	return Scales;
}

FJamBalanceData JamDifficultyRules::Apply(const FJamBalanceData& Balance, EJamDifficulty Difficulty)
{
	const FJamDifficultyScales Scales = ScalesOf(Balance, Difficulty);
	FJamBalanceData Out = Balance;
	Out.HeroHealth *= Scales.HeroHealth;
	Out.HeroDamageScale *= Scales.HeroDamage;
	Out.HealAmount *= Scales.Heal;
	Out.BossDamageScale *= Scales.BossDamage;
	return Out;
}
