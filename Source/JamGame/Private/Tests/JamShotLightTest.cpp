#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "View/JamShotLightRules.h"

/**
 * T-088: a flying shuriken lights the floor under it in its own colour. A bug here looks in game like "a memo or a
 * ball lights the floor", "a volley of shurikens costs a hundred lights" or "the newest shuriken is the dark one".
 *
 * Run: Automation RunTests Jam.ShotLight
 */
namespace
{
FJamProjectile ShotOfKind(int32 Id, EJamProjectile Kind, bool bExplosive = false)
{
	FJamProjectile Projectile;
	Projectile.Id = Id;
	Projectile.Kind = Kind;
	Projectile.bExplosive = bExplosive;
	return Projectile;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShotLightWhichTest, "Jam.ShotLight.OnlyShurikensUpToTheCapNewestFirst", JamTest::Flags)

bool FJamShotLightWhichTest::RunTest(const FString& Parameters)
{
	const TArray<FJamProjectile> Few = { ShotOfKind(1, EJamProjectile::Shuriken), ShotOfKind(2, EJamProjectile::Memo),
		ShotOfKind(3, EJamProjectile::Ball), ShotOfKind(4, EJamProjectile::Shuriken, true) };
	const TArray<int32> Lit = JamShotLightRules::Lit(Few, 6);
	TestEqual(TEXT("one light per shuriken"), Lit.Num(), 2);
	TestTrue(TEXT("the plain one"), Lit.Contains(0));
	TestTrue(TEXT("and the explosive one"), Lit.Contains(3));
	TArray<FJamProjectile> Volley;
	for (int32 Id = 10; Id < 30; ++Id)
	{
		Volley.Add(ShotOfKind(Id, EJamProjectile::Shuriken));
	}
	const TArray<int32> Capped = JamShotLightRules::Lit(Volley, 6);
	TestEqual(TEXT("never more than the cap at once"), Capped.Num(), 6);
	TestTrue(TEXT("the newest shot keeps its light"), Capped.Contains(Volley.Num() - 1));
	TestFalse(TEXT("the oldest gives its light up"), Capped.Contains(0));
	TestEqual(TEXT("no cap, no lights"), JamShotLightRules::Lit(Volley, 0).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShotLightBrightTest, "Jam.ShotLight.TheExplosiveOneIsALittleBrighter", JamTest::Flags)

bool FJamShotLightBrightTest::RunTest(const FString& Parameters)
{
	constexpr float Boost = 1.5f;
	TestEqual(TEXT("a plain shuriken at the base"), JamShotLightRules::Brightness(ShotOfKind(1, EJamProjectile::Shuriken), Boost), 1.f);
	TestEqual(TEXT("an explosive one a little brighter"),
		JamShotLightRules::Brightness(ShotOfKind(2, EJamProjectile::Shuriken, true), Boost), Boost);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
