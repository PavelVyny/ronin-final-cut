#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamDashRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-010: the dash, a short burst with a moment of invulnerability.
 * A bug here looks in game like "the dash goes where I look, not where I run", "I dashed through the memo and still
 * lost health" or "holding Space is a teleport across the arena".
 *
 * Run: Automation RunTests Jam.Dash / Jam.Fight.Dash
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashMoveDirTest, "Jam.Dash.GoesWhereTheFighterMoves", JamTest::Flags)

bool FJamDashMoveDirTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the move input wins over the facing"),
		JamDashRules::Direction(FVector2D(0.0, 1.0), FVector2D(1.0, 0.0)).Equals(FVector2D(0.0, 1.0), 1e-4));
	TestTrue(TEXT("the direction is a unit vector whatever the tilt"),
		JamDashRules::Direction(FVector2D(0.3, 0.4), FVector2D(1.0, 0.0)).Equals(FVector2D(0.6, 0.8), 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashAimDirTest, "Jam.Dash.FallsBackToTheAim", JamTest::Flags)

bool FJamDashAimDirTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("standing still: the dash goes where the fighter faces"),
		JamDashRules::Direction(FVector2D::ZeroVector, FVector2D(0.0, -1.0)).Equals(FVector2D(0.0, -1.0), 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashDistanceTest, "Jam.Dash.CoversTheConfiguredDistance", JamTest::Flags)

bool FJamDashDistanceTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Move = FVector2D(0.0, 1.0);
	Intents[EJamSide::Hero].bDash = true;
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the dash is reported"), JamTest::Count(F, EJamEvent::Dash), 1);
	// The player lets go of the button and pulls the stick aside: the dash keeps its line.
	Intents[EJamSide::Hero].Move = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bDash = false;
	JamTest::Run(F, Intents, B, JamSim::Ticks(B.DashDuration) - 1);
	const FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	TestEqual(TEXT("the dash covers its distance exactly"), static_cast<float>(Hero.Pos.Y), B.DashDistance, 0.1f);
	TestEqual(TEXT("and ignores the stick on the way"), static_cast<float>(Hero.Pos.X), 0.f, 0.1f);
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("after the dash the hero walks again"), static_cast<float>(Hero.Pos.X), B.HeroSpeed * JamSim::StepDt, 0.1f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashCooldownTest, "Jam.Dash.CooldownBlocksTheSecondDash", JamTest::Flags)

bool FJamDashCooldownTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	Intents[EJamSide::Hero].bDash = true;
	const int32 Cooldown = JamSim::Ticks(B.DashCooldown);
	int32 Dashes = JamTest::RunCounting(F, Intents, B, Cooldown, EJamEvent::Dash);
	TestEqual(TEXT("holding the button through the cooldown is one dash"), Dashes, 1);
	TestEqual(TEXT("of one dash length"), static_cast<float>(F.Fighters[EJamSide::Hero].Pos.Y), B.DashDistance, 0.1f);
	Dashes += JamTest::RunCounting(F, Intents, B, 1, EJamEvent::Dash);
	TestEqual(TEXT("the second dash comes when the cooldown is over"), Dashes, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashWindowTest, "Jam.Dash.InvulnerableOnlyInTheWindow", JamTest::Flags)

bool FJamDashWindowTest::RunTest(const FString& Parameters)
{
	// Dash 0.2 s is 12 steps, invulnerability 0.3 s is 18 steps.
	const FJamBalanceData B = JamTest::Balance();
	const FJamDashState Fresh;
	TestFalse(TEXT("a fighter who never dashed is not invulnerable"), JamDashRules::IsInvulnerable(Fresh, 0));
	TestFalse(TEXT("and is not dashing"), JamDashRules::IsDashing(Fresh, 0));
	TestTrue(TEXT("and can dash"), JamDashRules::CanStart(Fresh, 0));
	const FJamDashState State = JamDashRules::Start(FVector2D(1.0, 0.0), 100, B);
	TestTrue(TEXT("invulnerable from the first step"), JamDashRules::IsInvulnerable(State, 100));
	TestTrue(TEXT("to the last step of the window"), JamDashRules::IsInvulnerable(State, 117));
	TestFalse(TEXT("and not after it"), JamDashRules::IsInvulnerable(State, 118));
	TestTrue(TEXT("dashing to the last step of the dash"), JamDashRules::IsDashing(State, 111));
	TestFalse(TEXT("and not after it"), JamDashRules::IsDashing(State, 112));
	TestFalse(TEXT("no new dash during the cooldown"), JamDashRules::CanStart(State, 159));
	TestTrue(TEXT("a new dash after it"), JamDashRules::CanStart(State, 160));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDashFightTest, "Jam.Fight.DashThroughAProjectileTakesNoDamage", JamTest::Flags)

bool FJamDashFightTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamProjectile Memo;
	Memo.Id = 1000;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Pos = Memo.PrevPos = FVector2D(300.0, 0.0);
	Memo.Vel = FVector2D(-600.0, 0.0);
	Memo.Radius = B.MemoRadius;
	Memo.Damage = B.MemoDamage;
	Memo.DieTick = 600;

	FJamFight Standing = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	Standing.Projectiles.Add(Memo);
	JamTest::Run(Standing, FJamIntents(), B, 30);
	TestEqual(TEXT("a hero who stands is hit by the memo"), Standing.Fighters[EJamSide::Hero].Health, B.HeroHealth - B.MemoDamage);

	FJamFight Dashing = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	Dashing.Projectiles.Add(Memo);
	FJamIntents Intents;
	Intents[EJamSide::Hero].Move = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bDash = true;
	JamFightRules::Step(Dashing, Intents, B);
	JamTest::Run(Dashing, FJamIntents(), B, 30);
	TestEqual(TEXT("a hero who dashes through it takes nothing"), Dashing.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestEqual(TEXT("and the memo flies on"), Dashing.Projectiles.Num(), 1);
	return true;
}

#endif
