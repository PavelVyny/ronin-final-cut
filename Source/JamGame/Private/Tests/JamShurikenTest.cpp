#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamShurikenRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-009: the shuriken, a fast projectile; every third throw explodes.
 * A bug here looks in game like "the explosion never comes", "the boss's throws use up my explosive one" or "the
 * blast is drawn over the boss and does nothing".
 *
 * Run: Automation RunTests Jam.Shuriken / Jam.Fight
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenThirdTest, "Jam.Shuriken.EveryThirdExplodes", JamTest::Flags)

bool FJamShurikenThirdTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("the first throw is plain"), JamShurikenRules::IsExplosive(1, 3));
	TestFalse(TEXT("the second too"), JamShurikenRules::IsExplosive(2, 3));
	TestTrue(TEXT("the third explodes"), JamShurikenRules::IsExplosive(3, 3));
	TestFalse(TEXT("the fourth is plain again"), JamShurikenRules::IsExplosive(4, 3));
	TestTrue(TEXT("the sixth explodes"), JamShurikenRules::IsExplosive(6, 3));
	TestTrue(TEXT("every first means all of them"), JamShurikenRules::IsExplosive(1, 1));
	TestFalse(TEXT("a zero in the data means none, not a crash"), JamShurikenRules::IsExplosive(3, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenBlastTest, "Jam.Shuriken.ExplosionReachesTheEdgeOfATarget", JamTest::Flags)

bool FJamShurikenBlastTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the blast touches the near edge of the target"),
		JamShurikenRules::ExplosionHits(FVector2D::ZeroVector, 200.f, FVector2D(290.0, 0.0), 100.f));
	TestFalse(TEXT("a target a little farther is safe"),
		JamShurikenRules::ExplosionHits(FVector2D::ZeroVector, 200.f, FVector2D(310.0, 0.0), 100.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenFightTest, "Jam.Fight.ExplosiveShurikenDamagesInRadius", JamTest::Flags)

bool FJamShurikenFightTest::RunTest(const FString& Parameters)
{
	FJamBalanceData B = JamTest::Balance();
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bSecondary = true;

	FJamFight Plain = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	JamFightRules::Step(Plain, Intents, B);
	TestEqual(TEXT("the right button throws a shuriken"), Plain.Projectiles.Num(), 1);
	TestEqual(TEXT("of the shuriken kind"), Plain.Projectiles[0].Kind, EJamProjectile::Shuriken);
	TestFalse(TEXT("the first one is plain"), Plain.Projectiles[0].bExplosive);
	JamTest::Run(Plain, FJamIntents(), B, 30);
	TestEqual(TEXT("a plain hit takes the shuriken damage"), Plain.Fighters[EJamSide::Boss].Health, B.BossHealth - B.ShurikenDamage);

	// Two throws are behind: the next one explodes.
	FJamFight Direct = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	Direct.Fighters[EJamSide::Hero].Shuriken.Throws = 2;
	JamFightRules::Step(Direct, Intents, B);
	TestTrue(TEXT("the third throw is explosive"), Direct.Projectiles[0].bExplosive);
	const int32 Blasts = JamTest::RunCounting(Direct, FJamIntents(), B, 30, EJamEvent::Explosion);
	TestEqual(TEXT("it goes off once"), Blasts, 1);
	TestEqual(TEXT("and takes the explosion damage"), Direct.Fighters[EJamSide::Boss].Health, B.BossHealth - B.ShurikenExplosionDamage);

	// The boss stands beside the line of flight: no direct hit, the shuriken ends its flight next to him.
	B.ShurikenLife = 0.25f;
	FJamFight Near = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(500.0, 250.0));
	Near.Fighters[EJamSide::Hero].Shuriken.Throws = 2;
	JamFightRules::Step(Near, Intents, B);
	JamTest::Run(Near, FJamIntents(), B, 30);
	TestEqual(TEXT("a blast at the end of the flight reaches a boss standing near"), Near.Fighters[EJamSide::Boss].Health,
		B.BossHealth - B.ShurikenExplosionDamage);

	FJamFight Far = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(500.0, 600.0));
	Far.Fighters[EJamSide::Hero].Shuriken.Throws = 2;
	JamFightRules::Step(Far, Intents, B);
	JamTest::Run(Far, FJamIntents(), B, 30);
	TestEqual(TEXT("and does not reach one who stands far"), Far.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestEqual(TEXT("the hero is never hurt by his own blast"), Far.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenCounterTest, "Jam.Fight.CounterBelongsToTheThrower", JamTest::Flags)

bool FJamShurikenCounterTest::RunTest(const FString& Parameters)
{
	// Everybody throws up the Y axis, away from each other: nothing is hit, only the counters matter.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(300.0, 0.0), FVector2D(-300.0, 0.0));
	F.Boss.BusyUntilTick = 0;
	const int32 Cooldown = JamSim::Ticks(B.ShurikenCooldown);
	FJamIntents HeroThrows;
	HeroThrows[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	HeroThrows[EJamSide::Hero].bSecondary = true;
	FJamIntents BossThrows;
	BossThrows[EJamSide::Boss].Aim.Dir = FVector2D(0.0, 1.0);
	BossThrows[EJamSide::Boss].bSecondary = true;

	JamFightRules::Step(F, HeroThrows, B);
	JamTest::Run(F, FJamIntents(), B, Cooldown);
	JamFightRules::Step(F, HeroThrows, B);
	TestEqual(TEXT("the hero threw twice"), F.Fighters[EJamSide::Hero].Shuriken.Throws, 2);

	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	JamFightRules::Step(F, BossThrows, B);
	TestEqual(TEXT("the boss threw his first"), F.Fighters[EJamSide::Boss].Shuriken.Throws, 1);
	TestFalse(TEXT("which is plain: the hero's two throws are not his"), F.Projectiles.Last().bExplosive);
	TestEqual(TEXT("it belongs to the boss"), F.Projectiles.Last().Owner, EJamSide::Boss);

	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Hero);
	JamTest::Run(F, FJamIntents(), B, Cooldown);
	JamFightRules::Step(F, HeroThrows, B);
	TestTrue(TEXT("the hero's next throw is his third: explosive"), F.Projectiles.Last().bExplosive);
	return true;
}

/**
 * T-060: the Publisher's stolen shurikens never explode and the strike catches them from a little further out; the
 * hero's own keep every third explosive. A bug here looks in game like "his shuriken blew up in my face" or "I parried
 * his shuriken at the same distance as a memo".
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenHisPlainTest, "Jam.Shuriken.HisStolenOnesNeverExplode", JamTest::Flags)

bool FJamShurikenHisPlainTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(300.0, 0.0), FVector2D(-300.0, 0.0));
	F.Boss.BusyUntilTick = 0;
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	FJamIntents BossThrows;
	BossThrows[EJamSide::Boss].Aim.Dir = FVector2D(0.0, 1.0);
	BossThrows[EJamSide::Boss].bSecondary = true;
	int32 Explosive = 0;
	for (int32 Throw = 0; Throw < 6; ++Throw)
	{
		JamFightRules::Step(F, BossThrows, B);
		Explosive += F.Projectiles.Last().bExplosive ? 1 : 0;
		TestEqual(TEXT("each of his reaches out for the parry"), F.Projectiles.Last().ParryMargin, B.BossShurikenParryMargin);
		JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.ShurikenCooldown * B.BossStolenCooldown));
	}
	TestEqual(TEXT("he threw six"), F.Fighters[EJamSide::Boss].Shuriken.Throws, 6);
	TestEqual(TEXT("none of them explodes"), Explosive, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenWiderCatchTest, "Jam.Parry.HisShurikenIsCaughtFromFurtherOut", JamTest::Flags)

bool FJamShurikenWiderCatchTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	// A shuriken of his flies past the hero's shoulder: half its margin wide of the body, so it would not touch him.
	auto Pass = [&B](float Margin)
	{
		FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(900.0, 0.0));
		F.Boss.BusyUntilTick = 100000;
		FJamShot Shot;
		Shot.Kind = EJamProjectile::Shuriken;
		Shot.Speed = B.ShurikenSpeed * B.BossStolenSpeed;
		Shot.Radius = B.ShurikenRadius;
		Shot.Damage = B.ShurikenDamage;
		Shot.Life = 2.f;
		Shot.ParryMargin = Margin;
		JamFightOps::Shoot(F, EJamSide::Boss, FVector2D(-1.0, 0.0), Shot);
		FJamProjectile& Flying = F.Projectiles[0];
		Flying.Pos = Flying.PrevPos = FVector2D(300.0, B.HeroRadius + B.ShurikenRadius + 0.5 * B.BossShurikenParryMargin);
		FJamIntents Spam;
		Spam[EJamSide::Hero].bPrimary = true;
		return JamTest::RunCounting(F, Spam, B, JamSim::Ticks(1.f), EJamEvent::Deflected);
	};
	TestEqual(TEXT("his shuriken is caught by the wider reach"), Pass(B.BossShurikenParryMargin), 1);
	TestEqual(TEXT("one without the margin flies past"), Pass(0.f), 0);
	return true;
}

#endif
