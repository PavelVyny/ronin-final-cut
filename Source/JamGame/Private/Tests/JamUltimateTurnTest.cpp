#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamLoadoutRules.h"
#include "Sim/JamUltimateRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-058: once pressed, the hero's Ultimate line keeps turning toward the aim through the wind-up, at a limited rate, so
 * a slash pressed a little off can still be put on the Publisher. His own stolen line stays locked (T-038).
 * A bug here looks in game like "I pressed E and he stepped out of my line" or "the line snaps to the cursor at once".
 *
 * Run: Automation RunTests Jam.Ultimate
 */
namespace
{
double YawOf(const FVector2D& Dir)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateTurnTest, "Jam.Ultimate.LineTurnsTowardTheAim", JamTest::Flags)

bool FJamUltimateTurnTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const double PerStep = B.UltimateTurnRate * JamSim::StepDt;
	TestTrue(TEXT("a small turn is taken whole"),
		JamUltimateRules::TurnToward(FVector2D(1.0, 0.0), FVector2D(1.0, 0.0).GetRotated(PerStep * 0.5), PerStep)
			.Equals(FVector2D(1.0, 0.0).GetRotated(PerStep * 0.5), 1e-6));
	TestEqual(TEXT("a big one only up to the step's share"),
		YawOf(JamUltimateRules::TurnToward(FVector2D(1.0, 0.0), FVector2D(0.0, 1.0), PerStep)), PerStep, 1e-4);
	TestEqual(TEXT("the short way round"), YawOf(JamUltimateRules::TurnToward(FVector2D(1.0, 0.0), FVector2D(0.0, -1.0), PerStep)),
		-PerStep, 1e-4);

	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	F.Boss.BusyUntilTick = 100000;
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 1.f;
	FJamIntents Press;
	Press[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	Press[EJamSide::Hero].bUltimate = true;
	JamFightRules::Step(F, Press, B);
	FJamIntents Turn;
	Turn[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	JamFightRules::Step(F, Turn, B);
	TestEqual(TEXT("one step after the press the line turned one step's share toward the aim"),
		YawOf(F.Fighters[EJamSide::Hero].Ultimate.Dir), PerStep, 1e-4);
	int32 Steps = 1;
	while (F.Fighters[EJamSide::Hero].Ultimate.bPending && Steps < 100)
	{
		JamFightRules::Step(F, Turn, B);
		++Steps;
	}
	const double Most = PerStep * JamSim::Ticks(B.UltimateWindUp);
	TestTrue(TEXT("over the wind-up it turns no more than the rate allows"), YawOf(F.Fighters[EJamSide::Hero].Ultimate.Dir) <= Most + 1e-3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateBossLockedTest, "Jam.Ultimate.BossLineStaysLocked", JamTest::Flags)

bool FJamUltimateBossLockedTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-600.0, 0.0), FVector2D::ZeroVector);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	F.Fighters[EJamSide::Boss].Ultimate.Meter = 1.f;
	// Past his opening pause: he may press at once.
	F.Boss.BusyUntilTick = 0;
	FJamIntents Press;
	Press[EJamSide::Boss].Aim.Dir = FVector2D(-1.0, 0.0);
	Press[EJamSide::Boss].bUltimate = true;
	JamFightRules::Step(F, Press, B);
	FJamIntents Turn;
	Turn[EJamSide::Boss].Aim.Dir = FVector2D(0.0, 1.0);
	JamTest::Run(F, Turn, B, 5);
	TestTrue(TEXT("he pressed"), F.Fighters[EJamSide::Boss].Ultimate.bPending);
	TestTrue(TEXT("his line does not follow his aim"), F.Fighters[EJamSide::Boss].Ultimate.Dir.Equals(FVector2D(-1.0, 0.0), 1e-6));
	return true;
}

#endif
