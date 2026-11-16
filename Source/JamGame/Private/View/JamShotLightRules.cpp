#include "View/JamShotLightRules.h"

TArray<int32> JamShotLightRules::Lit(TConstArrayView<FJamProjectile> Projectiles, int32 Cap)
{
	TArray<int32> Shurikens;
	for (int32 Index = 0; Index < Projectiles.Num(); ++Index)
	{
		if (Projectiles[Index].Kind == EJamProjectile::Shuriken)
		{
			Shurikens.Add(Index);
		}
	}
	// Ids only grow: the newest shot has the highest, and it is the one the eye follows.
	Shurikens.Sort([Projectiles](int32 A, int32 B) { return Projectiles[A].Id > Projectiles[B].Id; });
	Shurikens.SetNum(FMath::Clamp(Cap, 0, Shurikens.Num()));
	return Shurikens;
}

float JamShotLightRules::Brightness(const FJamProjectile& Projectile, float ExplosiveBoost)
{
	return Projectile.bExplosive ? ExplosiveBoost : 1.f;
}
