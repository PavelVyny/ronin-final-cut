#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBossRules.h"
#include "Sim/JamBotRules.h"
#include "Sim/JamHazardRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-011: the Publisher's brain and his own two attacks, the memo volley and the slam; the end of the fight.
 * A bug here looks in game like "the boss stands in the corner", "the slam hurts the moment the circle appears", "I
 * left the circle and still lost health" or "the fight goes on after I died".
 *
 * Run: Automation RunTests Jam.Boss / Jam.Fight
 */
namespace
{
/** A fight in which the boss may act from the first step. */
FJamFight BossFight(const FJamBalanceData& B, const FVector2D& HeroPos, const FVector2D& BossPos)
{
	FJamFight F = JamTest::Fight(B, HeroPos, BossPos);
	F.Boss.BusyUntilTick = 0;
	return F;
}

constexpr int32 NotSoon = 100000;
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossChaseTest, "Jam.Boss.ChasesWhenFar", JamTest::Flags)

bool FJamBossChaseTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D(-600.0, 0.0), FVector2D(300.0, 0.0));
	F.Boss.VolleyReadyTick = NotSoon;
	const FJamIntent Chase = JamBossRules::Decide(F, B);
	TestTrue(TEXT("with nothing ready he walks at the hero"), Chase.Move.Equals(FVector2D(-1.0, 0.0), 1e-3));
	TestTrue(TEXT("and looks at him"), Chase.Aim.Dir.Equals(FVector2D(-1.0, 0.0), 1e-3));
	TestTrue(TEXT("the aim point is the hero"), Chase.Aim.Point.Equals(FVector2D(-600.0, 0.0), 1e-3));
	TestFalse(TEXT("no volley while it cools down"), Chase.bVolley);
	TestFalse(TEXT("no slam at this distance"), Chase.bSlam);

	F.Boss.VolleyReadyTick = 0;
	const FJamIntent Volley = JamBossRules::Decide(F, B);
	TestTrue(TEXT("a ready volley is thrown from afar"), Volley.bVolley);
	TestFalse(TEXT("the slam still waits for the hero to come close"), Volley.bSlam);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(0.0, 0.0);
	const FJamIntent Slam = JamBossRules::Decide(F, B);
	TestTrue(TEXT("a hero within the slam range gets the slam"), Slam.bSlam);
	TestFalse(TEXT("one attack at a time"), Slam.bVolley);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossRecoverTest, "Jam.Boss.KeepsStillWhileRecovering", JamTest::Flags)

bool FJamBossRecoverTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D(-600.0, 0.0), FVector2D(300.0, 0.0));
	F.Boss.BusyUntilTick = 100;
	const FJamIntent Intent = JamBossRules::Decide(F, B);
	TestTrue(TEXT("he does not walk"), Intent.Move.IsNearlyZero());
	TestFalse(TEXT("he does not throw"), Intent.bVolley);
	TestFalse(TEXT("he does not slam"), Intent.bSlam);

	// Even an intent that says otherwise does not move a busy boss or start his attacks: the pause is the rule.
	FJamIntents Pushy;
	Pushy[EJamSide::Boss].Move = FVector2D(-1.0, 0.0);
	Pushy[EJamSide::Boss].bVolley = true;
	Pushy[EJamSide::Boss].bSlam = true;
	JamTest::Run(F, Pushy, B, 50);
	TestTrue(TEXT("the boss stood"), F.Fighters[EJamSide::Boss].Pos.Equals(FVector2D(300.0, 0.0), 1e-3));
	TestEqual(TEXT("no memos"), F.Projectiles.Num(), 0);
	TestEqual(TEXT("no slam"), F.Hazards.Num(), 0);
	const FJamFight Fresh = JamFightRules::Start(B, 1);
	TestTrue(TEXT("a new fight starts with a pause: the player gets a moment to look around"), JamBossRules::IsBusy(Fresh));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossSeedTest, "Jam.Boss.SameSeedSameDecisions", JamTest::Flags)

bool FJamBossSeedTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight First = JamFightRules::Start(B, 7);
	FJamFight Second = JamFightRules::Start(B, 7);
	for (int32 i = 0; i < 20 * JamSim::StepsPerSecond; ++i)
	{
		JamFightRules::Play(First, JamBotRules::Kite(First, EJamSide::Hero, 500.f), B);
		JamFightRules::Play(Second, JamBotRules::Kite(Second, EJamSide::Hero, 500.f), B);
	}
	TestTrue(
		TEXT("the boss ended in the same place"), First.Fighters[EJamSide::Boss].Pos.Equals(Second.Fighters[EJamSide::Boss].Pos, 1e-6));
	TestEqual(TEXT("the hero has the same health"), First.Fighters[EJamSide::Hero].Health, Second.Fighters[EJamSide::Hero].Health);
	TestEqual(TEXT("the same projectiles are in the air"), First.Projectiles.Num(), Second.Projectiles.Num());
	TestEqual(TEXT("the same number of things was spawned"), First.NextId, Second.NextId);
	TestTrue(TEXT("and the boss did attack in these 20 seconds"), First.NextId > 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossVolleyTest, "Jam.Boss.VolleyIsAFanAroundTheAim", JamTest::Flags)

bool FJamBossVolleyTest::RunTest(const FString& Parameters)
{
	const FVector2D Aim(0.0, 1.0);
	const TArray<FVector2D> Fan = JamBossRules::VolleyDirections(Aim, 5, 60.f);
	if (!TestEqual(TEXT("five memos"), Fan.Num(), 5))
	{
		return false;
	}
	TestTrue(TEXT("the middle one flies along the aim"), Fan[2].Equals(Aim, 1e-4));
	TestTrue(TEXT("the outer ones are half the spread away"), Fan[0].Equals(Aim.GetRotated(-30.0), 1e-4));
	TestTrue(TEXT("on both sides"), Fan[4].Equals(Aim.GetRotated(30.0), 1e-4));
	for (const FVector2D& Dir : Fan)
	{
		TestEqual(TEXT("every direction is a unit vector"), static_cast<float>(Dir.Size()), 1.f, 1e-4f);
	}
	const TArray<FVector2D> Single = JamBossRules::VolleyDirections(Aim, 1, 60.f);
	TestTrue(TEXT("a volley of one flies along the aim"), Single.Num() == 1 && Single[0].Equals(Aim, 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossMemosTest, "Jam.Boss.VolleyThrowsTheMemosOncePerCooldown", JamTest::Flags)

bool FJamBossMemosTest::RunTest(const FString& Parameters)
{
	FJamBalanceData B = JamTest::Balance();
	B.MemoLife = 100.f;
	B.ArenaRadius = 100000.f;
	FJamFight F = BossFight(B, FVector2D(-5000.0, 0.0), FVector2D::ZeroVector);
	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Dir = FVector2D(0.0, 1.0);
	Intents[EJamSide::Boss].bVolley = true;
	JamFightRules::Step(F, Intents, B);
	if (!TestEqual(TEXT("one volley is the configured number of memos"), F.Projectiles.Num(), FMath::RoundToInt32(B.MemoCount)))
	{
		return false;
	}
	TestEqual(TEXT("they are memos"), F.Projectiles[0].Kind, EJamProjectile::Memo);
	TestEqual(TEXT("they belong to the boss"), F.Projectiles[0].Owner, EJamSide::Boss);
	TestEqual(TEXT("they fly at the memo speed"), static_cast<float>(F.Projectiles[0].Vel.Size()), B.MemoSpeed, 0.1f);
	TestTrue(TEXT("the boss pauses after the volley"), JamBossRules::IsBusy(F));
	JamTest::Run(F, Intents, B, JamSim::Ticks(B.VolleyCooldown) - 1);
	TestEqual(TEXT("holding the order through the cooldown adds nothing"), F.Projectiles.Num(), FMath::RoundToInt32(B.MemoCount));
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the next volley comes after the cooldown"), F.Projectiles.Num(), 2 * FMath::RoundToInt32(B.MemoCount));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossLeadTest, "Jam.Boss.LeadsARunningTarget", JamTest::Flags)

bool FJamBossLeadTest::RunTest(const FString& Parameters)
{
	// T-033. A bug here looks in game like "I just hold one direction and he never hits me".
	const FVector2D From = FVector2D::ZeroVector;
	const FVector2D Target(800.0, 0.0);
	const FVector2D Direct(1.0, 0.0);
	// 500 a second against 300 across: they meet after 2 seconds, 600 to the side.
	const FVector2D Lead = JamBossRules::LeadDirection(From, Target, FVector2D(0.0, 300.0), 500.f, 0.f);
	TestTrue(TEXT("the throw goes where the target will be"), Lead.Equals(FVector2D(0.8, 0.6), 1e-3));

	// The projectile appears 100 in front of the thrower: 700 left to fly, the target is met sooner and nearer.
	const FVector2D Head = JamBossRules::LeadDirection(From, Target, FVector2D(0.0, 300.0), 500.f, 100.f);
	TestTrue(TEXT("a head start shortens the lead"), Head.Y > 0.1 && Head.Y < Lead.Y);
	// The target crosses the line of the throw this many seconds after it.
	const double Seconds = 800.0 * Head.Y / Head.X / 300.0;
	const FVector2D Meeting = Target + FVector2D(0.0, 300.0) * Seconds;
	TestEqual(TEXT("and the two still meet"), static_cast<float>(Meeting.Size()), static_cast<float>(100.0 + 500.0 * Seconds), 1.f);

	TestTrue(TEXT("a standing target is aimed at directly"),
		JamBossRules::LeadDirection(From, Target, FVector2D::ZeroVector, 500.f, 0.f).Equals(Direct, 1e-4));
	TestTrue(TEXT("a target that outruns the projectile is aimed at directly"),
		JamBossRules::LeadDirection(From, Target, FVector2D(0.0, 600.0), 500.f, 0.f).Equals(Direct, 1e-4));
	TestTrue(TEXT("a target that runs straight at the thrower needs no lead"),
		JamBossRules::LeadDirection(From, Target, FVector2D(-300.0, 0.0), 500.f, 0.f).Equals(Direct, 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossLeadVolleyTest, "Jam.Boss.VolleyLeadsTheHero", JamTest::Flags)

bool FJamBossLeadVolleyTest::RunTest(const FString& Parameters)
{
	FJamBalanceData B = JamTest::Balance();
	B.MemoSpeed = 1000.f;
	B.ArenaRadius = 100000.f;
	FJamFight F = BossFight(B, FVector2D(-700.0, 0.0), FVector2D::ZeroVector);
	// The hero is already running when the Publisher looks up: across his line of sight, and he never turns.
	F.Boss.VolleyReadyTick = 5;
	F.Boss.SlamReadyTick = NotSoon;
	FJamIntent Runner;
	Runner.Move = FVector2D(0.0, 1.0);
	bool bThrown = false;
	for (int32 i = 0; i < 2 * JamSim::StepsPerSecond; ++i)
	{
		JamFightRules::Play(F, Runner, B);
		bThrown |= JamTest::Count(F, EJamEvent::ProjectileSpawned) > 0;
	}
	TestTrue(TEXT("he threw his volley"), bThrown);
	TestTrue(TEXT("a hero who keeps his course runs into a memo"), F.Fighters[EJamSide::Hero].Health < B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossStraightShurikenTest, "Jam.Boss.StolenShurikenIsNotLed", JamTest::Flags)

bool FJamBossStraightShurikenTest::RunTest(const FString& Parameters)
{
	// T-033: only his own volley leads. A bug here looks in game like "one of his shuriken always finds me while I run".
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D(-700.0, 0.0), FVector2D::ZeroVector);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	F.Boss.SlamReadyTick = NotSoon;
	// The hero runs across the line of sight.
	F.Fighters[EJamSide::Hero].PrevPos = FVector2D(-700.0, -10.0);
	const FVector2D ToHero(-1.0, 0.0);

	F.Boss.VolleyReadyTick = 0;
	const FJamIntent Volley = JamBossRules::Decide(F, B);
	TestTrue(TEXT("the volley is ready: he throws it ahead of the hero"), Volley.bVolley && Volley.Aim.Dir.Y > 0.1);
	TestFalse(TEXT("and no shuriken goes along that aim"), Volley.bSecondary);

	F.Boss.VolleyReadyTick = NotSoon;
	const FJamIntent Between = JamBossRules::Decide(F, B);
	TestTrue(TEXT("between the volleys the shuriken is thrown"), Between.bSecondary);
	TestTrue(TEXT("straight at the hero"), Between.Aim.Dir.Equals(ToHero, 0.001));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossSlamTest, "Jam.Boss.SlamHurtsOnlyAfterTheTelegraph", JamTest::Flags)

bool FJamBossSlamTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D::ZeroVector, FVector2D(300.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Point = FVector2D::ZeroVector;
	Intents[EJamSide::Boss].bSlam = true;
	JamFightRules::Step(F, Intents, B);
	if (!TestEqual(TEXT("the order puts a warning circle"), F.Hazards.Num(), 1))
	{
		return false;
	}
	const FJamHazard Circle = F.Hazards[0];
	TestTrue(TEXT("on the aimed point"), Circle.Pos.IsNearlyZero());
	TestEqual(TEXT("of the slam radius"), Circle.Radius, B.SlamRadius);
	TestEqual(TEXT("the warning starts empty"), JamHazardRules::Progress(Circle, static_cast<float>(Circle.StartTick)), 0.f);
	TestEqual(TEXT("and is full at the hit"), JamHazardRules::Progress(Circle, static_cast<float>(Circle.HitTick)), 1.f);

	Intents[EJamSide::Boss].bSlam = false;
	const int32 Telegraph = JamSim::Ticks(B.SlamTelegraph);
	JamTest::Run(F, Intents, B, Telegraph - 1);
	TestEqual(TEXT("nothing hurts during the warning"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestEqual(TEXT("the circle is still there"), F.Hazards.Num(), 1);
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the slam is reported"), JamTest::Count(F, EJamEvent::Slam), 1);
	TestEqual(TEXT("a hero who stayed takes the slam"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - B.SlamDamage);
	TestEqual(TEXT("the circle is gone"), F.Hazards.Num(), 0);
	TestEqual(TEXT("the boss is not hurt by his own slam"), F.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestTrue(TEXT("the boss still stands after it: the player's opening"), JamBossRules::IsBusy(F));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossSlamMissTest, "Jam.Boss.SlamMissesAHeroWhoLeftTheCircle", JamTest::Flags)

bool FJamBossSlamMissTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D::ZeroVector, FVector2D(300.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Point = FVector2D::ZeroVector;
	Intents[EJamSide::Boss].bSlam = true;
	JamFightRules::Step(F, Intents, B);
	Intents[EJamSide::Boss].bSlam = false;
	Intents[EJamSide::Hero].Move = FVector2D(-1.0, 0.0);
	const int32 Slams = JamTest::RunCounting(F, Intents, B, JamSim::Ticks(B.SlamTelegraph) + 5, EJamEvent::Slam);
	TestEqual(TEXT("the slam lands"), Slams, 1);
	TestEqual(TEXT("on an empty circle: the hero walked out"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);

	FJamHazard Circle;
	Circle.Radius = 250.f;
	Circle.HitTick = 60;
	TestTrue(TEXT("the circle hurts a fighter whose centre is inside"), JamHazardRules::Hits(Circle, FVector2D(240.0, 0.0)));
	TestFalse(TEXT("and spares one whose centre is outside"), JamHazardRules::Hits(Circle, FVector2D(260.0, 0.0)));
	TestFalse(TEXT("it is not due before its step"), JamHazardRules::IsDue(Circle, 59));
	TestTrue(TEXT("it is due on its step"), JamHazardRules::IsDue(Circle, 60));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFightHeroDeadTest, "Jam.Fight.HeroDiesAtZeroHealth", JamTest::Flags)

bool FJamFightHeroDeadTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = BossFight(B, FVector2D::ZeroVector, FVector2D(300.0, 0.0));
	F.Fighters[EJamSide::Hero].Health = 5.f;
	TestEqual(TEXT("the fight is on"), F.Phase, EJamPhase::Fight);
	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Point = FVector2D::ZeroVector;
	Intents[EJamSide::Boss].bSlam = true;
	JamTest::Run(F, Intents, B, JamSim::Ticks(B.SlamTelegraph) + 1);
	TestEqual(TEXT("the hero is at zero"), F.Fighters[EJamSide::Hero].Health, 0.f);
	TestEqual(TEXT("and the fight says so"), F.Phase, EJamPhase::HeroDead);

	const int32 Tick = F.Tick;
	Intents[EJamSide::Hero].Move = FVector2D(-1.0, 0.0);
	JamTest::Run(F, Intents, B, 30);
	TestEqual(TEXT("time stands still after the end"), F.Tick, Tick);
	TestTrue(TEXT("and nobody moves"), F.Fighters[EJamSide::Hero].Pos.IsNearlyZero());
	return true;
}

#endif
