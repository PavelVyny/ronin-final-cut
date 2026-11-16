#include "Sim/JamHealthRules.h"

float JamHealthRules::Apply(float Health, float Damage, bool bInvulnerable)
{
	if (bInvulnerable || Damage <= 0.f)
	{
		return Health;
	}
	return FMath::Max(0.f, Health - Damage);
}

float JamHealthRules::HeroDamageScale(int32 StealsDone, float CutScale)
{
	return StealsDone >= 1 ? CutScale : 1.f;
}
