#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamHealthRules.h"
#include "Sim/JamProjectileRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-005 and T-006: the weak ball shot, projectiles and health.
 * A bug here looks in game like "the ball flies through the boss", "holding the button is a machine gun" or "health
 * goes below zero".
 *
 * Run: Automation RunTests Jam.Shot / Jam.Projectile / Jam.Health
 */
namespace
{
int32 CountEvents(const FJamFight& F, EJamEvent Type)
{
	int32 Count = 0;
	for (const FJamEvent& Event : F.Events)
	{
		Count += Event.Type == Type ? 1 : 0;
	}
	return Count;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShotSpawnTest, "Jam.Shot.BallSpawnsAlongTheAim", JamTest::Flags)

bool FJamShotSpawnTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	JamTest::Bean(F);
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	Intents[EJamSide::Hero].bPrimary = true;
	JamFightRules::Step(F, Intents, B);
	if (!TestEqual(TEXT("one press, one ball"), F.Projectiles.Num(), 1))
	{
		return false;
	}
	const FJamProjectile& Ball = F.Projectiles[0];
	TestEqual(TEXT("it is a ball"), Ball.Kind, EJamProjectile::Ball);
	TestEqual(TEXT("it belongs to the hero"), Ball.Owner, EJamSide::Hero);
	TestTrue(TEXT("it flies along the aim at the ball speed"), Ball.Vel.Equals(FVector2D(0.0, B.BallSpeed), 1e-2));
	TestTrue(TEXT("it starts outside the hero's body"), Ball.Pos.Y >= B.HeroRadius);
	TestEqual(TEXT("the step reports the spawn"), CountEvents(F, EJamEvent::ProjectileSpawned), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShotCooldownTest, "Jam.Shot.CooldownBlocksTheSecondShot", JamTest::Flags)

bool FJamShotCooldownTest::RunTest(const FString& Parameters)
{
	FJamBalanceData B = JamTest::Balance();
	B.BallCooldown = 0.5f;
	B.BallLife = 10.f;
	B.ArenaRadius = 100000.f;
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	JamTest::Bean(F);
	FJamIntents Intents;
	Intents[EJamSide::Hero].bPrimary = true;
	JamTest::Run(F, Intents, B, 29);
	TestEqual(TEXT("holding the button for less than the cooldown gives one ball"), F.Projectiles.Num(), 1);
	JamTest::Run(F, Intents, B, 2);
	TestEqual(TEXT("the second ball comes after the cooldown"), F.Projectiles.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamProjectileSweepTest, "Jam.Projectile.HitsACircleOnItsPath", JamTest::Flags)

bool FJamProjectileSweepTest::RunTest(const FString& Parameters)
{
	// The step jumps over the whole target: the path counts, not the end point.
	TestTrue(TEXT("a path through the circle hits"),
		JamProjectileRules::SweepHitsCircle(FVector2D(-500.0, 0.0), FVector2D(500.0, 0.0), 10.f, FVector2D(0.0, 30.0), 50.f));
	TestTrue(TEXT("a graze counts with the projectile's own radius"),
		JamProjectileRules::SweepHitsCircle(FVector2D(-500.0, 59.0), FVector2D(500.0, 59.0), 10.f, FVector2D::ZeroVector, 50.f));
	TestFalse(TEXT("a path beside the circle misses"),
		JamProjectileRules::SweepHitsCircle(FVector2D(-500.0, 61.0), FVector2D(500.0, 61.0), 10.f, FVector2D::ZeroVector, 50.f));
	TestFalse(TEXT("a path that ends before the circle misses"),
		JamProjectileRules::SweepHitsCircle(FVector2D(-500.0, 0.0), FVector2D(-100.0, 0.0), 10.f, FVector2D::ZeroVector, 50.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamProjectileOwnerTest, "Jam.Projectile.IgnoresItsOwner", JamTest::Flags)

bool FJamProjectileOwnerTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	// A hero ball placed inside the hero and flying across him.
	FJamProjectile Ball;
	Ball.Id = F.NextId++;
	Ball.Owner = EJamSide::Hero;
	Ball.Pos = FVector2D(-10.0, 0.0);
	Ball.Vel = FVector2D(600.0, 0.0);
	Ball.Radius = B.BallRadius;
	Ball.Damage = B.BallDamage;
	Ball.DieTick = 600;
	F.Projectiles.Add(Ball);
	JamTest::Run(F, FJamIntents(), B, 5);
	TestEqual(TEXT("the hero is not hurt by his own ball"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestEqual(TEXT("and the ball flies on"), F.Projectiles.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamProjectileEdgeTest, "Jam.Projectile.DiesOutsideTheArenaOrOfAge", JamTest::Flags)

bool FJamProjectileEdgeTest::RunTest(const FString& Parameters)
{
	FJamProjectile Ball;
	Ball.Pos = FVector2D(1001.0, 0.0);
	Ball.DieTick = 100;
	TestTrue(TEXT("outside the arena"), JamProjectileRules::IsGone(Ball, 0, 1000.f));
	Ball.Pos = FVector2D(0.0, 0.0);
	TestFalse(TEXT("inside and young"), JamProjectileRules::IsGone(Ball, 99, 1000.f));
	TestTrue(TEXT("inside and old"), JamProjectileRules::IsGone(Ball, 100, 1000.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealthDamageTest, "Jam.Health.DamageReducesHealth", JamTest::Flags)

bool FJamHealthDamageTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("damage is subtracted"), JamHealthRules::Apply(100.f, 30.f, false), 70.f);
	TestEqual(TEXT("health never goes below zero"), JamHealthRules::Apply(10.f, 30.f, false), 0.f);
	TestEqual(TEXT("an invulnerable fighter takes nothing"), JamHealthRules::Apply(100.f, 30.f, true), 100.f);
	TestEqual(TEXT("negative damage does not heal"), JamHealthRules::Apply(50.f, -30.f, false), 50.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealthBallTest, "Jam.Health.BallDamagesTheBoss", JamTest::Flags)

bool FJamHealthBallTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	JamTest::Bean(F);
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bPrimary = true;
	JamFightRules::Step(F, Intents, B);
	Intents[EJamSide::Hero].bPrimary = false;
	int32 Hits = 0;
	for (int32 i = 0; i < JamSim::StepsPerSecond; ++i)
	{
		JamFightRules::Step(F, Intents, B);
		Hits += CountEvents(F, EJamEvent::Hit);
	}
	TestEqual(TEXT("the boss lost the ball damage"), F.Fighters[EJamSide::Boss].Health, B.BossHealth - B.BallDamage);
	TestEqual(TEXT("exactly one hit was reported"), Hits, 1);
	TestEqual(TEXT("the ball is gone"), F.Projectiles.Num(), 0);
	return true;
}

#endif
