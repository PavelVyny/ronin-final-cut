#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamKatanaRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-008: the katana, a series of three strikes in an arc in front of the fighter.
 * A bug here looks in game like "the katana hits behind my back", "one click takes the health three times" or "the
 * big third strike never comes".
 *
 * Run: Automation RunTests Jam.Katana / Jam.Fight.Katana
 */
namespace
{
const FVector2D KatanaOrigin = FVector2D::ZeroVector;
const FVector2D KatanaFacing(1.0, 0.0);

FVector2D AtAngle(double Degrees, double Distance)
{
	return KatanaFacing.GetRotated(Degrees) * Distance;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaFrontTest, "Jam.Katana.HitsInFront", JamTest::Flags)

bool FJamKatanaFrontTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("straight ahead and in reach"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(0.0, 150.0), 0.f, 200.f, 60.f));
	TestTrue(TEXT("near the edge of the arc"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(59.0, 150.0), 0.f, 200.f, 60.f));
	TestTrue(TEXT("and on its other side"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(-59.0, 150.0), 0.f, 200.f, 60.f));
	TestFalse(
		TEXT("straight ahead but out of reach"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(0.0, 250.0), 0.f, 200.f, 60.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaBehindTest, "Jam.Katana.MissesBehind", JamTest::Flags)

bool FJamKatanaBehindTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("right behind"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(180.0, 150.0), 0.f, 200.f, 60.f));
	TestFalse(TEXT("just outside the arc"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(61.0, 150.0), 0.f, 200.f, 60.f));
	TestFalse(TEXT("to the side"), JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(-90.0, 150.0), 0.f, 200.f, 60.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaRadiusTest, "Jam.Katana.TargetRadiusCounts", JamTest::Flags)

bool FJamKatanaRadiusTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the blade reaches the near edge of a big target"),
		JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(0.0, 290.0), 100.f, 200.f, 60.f));
	TestFalse(TEXT("but not a target whose edge is out of reach"),
		JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(0.0, 310.0), 100.f, 200.f, 60.f));
	TestTrue(TEXT("a big target to the side sticks into the arc"),
		JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(90.0, 150.0), 100.f, 200.f, 60.f));
	TestFalse(TEXT("a small one at the same place does not"),
		JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(90.0, 150.0), 20.f, 200.f, 60.f));
	TestTrue(TEXT("a target the fighter stands inside is always hit"),
		JamKatanaRules::InArc(KatanaOrigin, KatanaFacing, AtAngle(180.0, 50.0), 100.f, 200.f, 60.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaSeriesTest, "Jam.Katana.ThreeStrikesThenRestart", JamTest::Flags)

bool FJamKatanaSeriesTest::RunTest(const FString& Parameters)
{
	// Wind-up 0.1 s and recovery 0.2 s: a strike keeps the fighter busy for 18 steps.
	const FJamBalanceData B = JamTest::Balance();
	FJamKatanaState State;
	TestEqual(TEXT("the first press is the first strike"), JamKatanaRules::NextStrike(State, 0), 1);
	State = JamKatanaRules::Begin(1, 0, B);
	TestEqual(TEXT("it lands after the wind-up"), State.HitTick, 6);
	TestTrue(TEXT("and has not landed yet"), State.bHitPending);
	TestEqual(TEXT("a press right after it is the second"), JamKatanaRules::NextStrike(State, 18), 2);
	State = JamKatanaRules::Begin(2, 18, B);
	TestEqual(TEXT("then the third"), JamKatanaRules::NextStrike(State, 36), 3);
	State = JamKatanaRules::Begin(3, 36, B);
	TestEqual(TEXT("after the third the series starts over"), JamKatanaRules::NextStrike(State, 54), 1);
	TestTrue(TEXT("the third strike is the strong one"), JamKatanaRules::Damage(3, B) > JamKatanaRules::Damage(2, B));
	TestEqual(TEXT("the first two are equal"), JamKatanaRules::Damage(1, B), JamKatanaRules::Damage(2, B));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaLateTest, "Jam.Katana.LatePressStartsOver", JamTest::Flags)

bool FJamKatanaLateTest::RunTest(const FString& Parameters)
{
	// Busy until step 18, then the chain window of 0.5 s: up to step 48.
	const FJamBalanceData B = JamTest::Balance();
	const FJamKatanaState State = JamKatanaRules::Begin(1, 0, B);
	TestEqual(TEXT("the last step of the window still continues"), JamKatanaRules::NextStrike(State, 48), 2);
	TestEqual(TEXT("one step later the series is lost"), JamKatanaRules::NextStrike(State, 49), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaBusyTest, "Jam.Katana.CannotStrikeDuringWindUp", JamTest::Flags)

bool FJamKatanaBusyTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamKatanaState State = JamKatanaRules::Begin(1, 0, B);
	TestEqual(TEXT("no new strike during the wind-up"), JamKatanaRules::NextStrike(State, 3), 0);
	TestEqual(TEXT("nor during the recovery"), JamKatanaRules::NextStrike(State, 17), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamKatanaFightTest, "Jam.Fight.KatanaDamagesOncePerStrike", JamTest::Flags)

bool FJamKatanaFightTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(200.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bPrimary = true;
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the swing is reported on the press"), JamTest::Count(F, EJamEvent::Swing), 1);
	TestEqual(TEXT("nothing is hurt before the wind-up ends"), F.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestTrue(TEXT("the strike steps the hero forward"), F.Fighters[EJamSide::Hero].Pos.X > 1.0);
	Intents[EJamSide::Hero].bPrimary = false;
	const int32 Hits = JamTest::RunCounting(F, Intents, B, 30, EJamEvent::Hit);
	TestEqual(TEXT("one press is one hit"), Hits, 1);
	TestEqual(TEXT("of the first strike's damage"), F.Fighters[EJamSide::Boss].Health, B.BossHealth - B.KatanaDamage);
	TestEqual(TEXT("no projectile was shot"), F.Projectiles.Num(), 0);

	// Holding the button for a second: strikes start on steps 0, 18, 36 and 54, three of them land in time.
	FJamFight Held = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(200.0, 0.0));
	Intents[EJamSide::Hero].bPrimary = true;
	JamTest::Run(Held, Intents, B, 60);
	TestEqual(TEXT("a held button gives the series: two strikes and the finisher"), Held.Fighters[EJamSide::Boss].Health,
		B.BossHealth - 2.f * B.KatanaDamage - B.KatanaFinisherDamage);

	FJamFight Away = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(200.0, 0.0));
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(-1.0, 0.0);
	JamTest::Run(Away, Intents, B, 60);
	TestEqual(TEXT("swinging away from the boss hurts nobody"), Away.Fighters[EJamSide::Boss].Health, B.BossHealth);
	return true;
}

#endif
