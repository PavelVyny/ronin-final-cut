#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamUltimateRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-020: the Ultimate. A meter filled by the damage the fighter deals; when it is full, E winds up and slashes along a
 * long line in front of him. A bug here looks in game like "the meter is full and E does nothing" or "the slash went
 * through the boss and he took nothing".
 *
 * Run: Automation RunTests Jam.Ultimate
 */
namespace
{
FJamIntents Pressing()
{
	FJamIntents Intents;
	Intents[EJamSide::Hero].bUltimate = true;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	return Intents;
}

/** Hero at the origin facing +X with a full meter, the boss on the line, standing. */
FJamFight Ready(const FJamBalanceData& B, const FVector2D& BossPos)
{
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, BossPos);
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 1.f;
	F.Boss.BusyUntilTick = 100000;
	return F;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateLineTest, "Jam.Ultimate.LineTouchesWhatIsInIt", JamTest::Flags)

bool FJamUltimateLineTest::RunTest(const FString& Parameters)
{
	const FVector2D O = FVector2D::ZeroVector;
	const FVector2D X(1.0, 0.0);
	TestTrue(TEXT("straight ahead"), JamUltimateRules::OnLine(O, X, FVector2D(800.0, 0.0), 50.f, 1000.f, 80.f));
	TestTrue(TEXT("beside the line, the body reaches in"), JamUltimateRules::OnLine(O, X, FVector2D(800.0, 120.0), 50.f, 1000.f, 80.f));
	TestFalse(TEXT("too far beside"), JamUltimateRules::OnLine(O, X, FVector2D(800.0, 200.0), 50.f, 1000.f, 80.f));
	TestFalse(TEXT("behind"), JamUltimateRules::OnLine(O, X, FVector2D(-300.0, 0.0), 50.f, 1000.f, 80.f));
	TestFalse(TEXT("beyond the reach"), JamUltimateRules::OnLine(O, X, FVector2D(1200.0, 0.0), 50.f, 1000.f, 80.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateChargeTest, "Jam.Ultimate.MeterFillsByDamageDealt", JamTest::Flags)

bool FJamUltimateChargeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	F.Boss.BusyUntilTick = 100000;
	FJamIntents Shooting;
	Shooting[EJamSide::Hero].bSecondary = true;
	Shooting[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	// The ball: shot once, it reaches the boss well within a second.
	JamTest::Run(F, Shooting, B, 1);
	JamTest::Run(F, FJamIntents(), B, 60);
	const float Dealt = B.BossHealth - F.Fighters[EJamSide::Boss].Health;
	TestTrue(TEXT("the ball hit"), Dealt > 0.f);
	TestEqual(TEXT("the meter grew by the damage"), F.Fighters[EJamSide::Hero].Ultimate.Meter, Dealt * B.UltimateGainPerDamage, 1e-4f);

	F.Fighters[EJamSide::Hero].Ultimate.Meter = 0.99f;
	JamTest::Run(F, Shooting, B, 1);
	JamTest::Run(F, FJamIntents(), B, 60);
	TestEqual(TEXT("it stops at full"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 1.f);
	return true;
}

/** T-037: the step that fills a meter says so once, for the banner and the ready sound. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateReadyTest, "Jam.Ultimate.AFullMeterIsAnnouncedOnce", JamTest::Flags)

bool FJamUltimateReadyTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 0.99f;
	const auto Readies = [&F]()
	{ return F.Events.FilterByPredicate([](const FJamEvent& Event) { return Event.Type == EJamEvent::UltimateReady; }); };

	// A blow on the Publisher big enough to top the meter up.
	JamFightOps::AddEvent(F, EJamEvent::Hit, EJamSide::Boss, 0, F.Fighters[EJamSide::Boss].Pos, 0.02f / B.UltimateGainPerDamage);
	JamUltimateRules::Charge(F, B);
	TestEqual(TEXT("the meter is full"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 1.f);
	const TArray<FJamEvent> First = Readies();
	TestEqual(TEXT("one announcement"), First.Num(), 1);
	TestTrue(TEXT("for the hero"), First.Num() == 1 && First[0].Side == EJamSide::Hero);

	F.Events.Reset();
	JamFightOps::AddEvent(F, EJamEvent::Hit, EJamSide::Boss, 0, F.Fighters[EJamSide::Boss].Pos, 50.f);
	JamUltimateRules::Charge(F, B);
	TestEqual(TEXT("a meter already full is not announced again"), Readies().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateSlashTest, "Jam.Ultimate.SlashHitsAlongTheLine", JamTest::Flags)

bool FJamUltimateSlashTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = Ready(B, FVector2D(700.0, 0.0));
	const int32 Started = JamTest::RunCounting(F, Pressing(), B, 1, EJamEvent::UltimateStarted);
	TestEqual(TEXT("E starts the wind-up"), Started, 1);
	TestEqual(TEXT("nothing lands at once"), F.Fighters[EJamSide::Boss].Health, B.BossHealth);
	const int32 Slashes = JamTest::RunCounting(F, FJamIntents(), B, JamSim::Ticks(B.UltimateWindUp) + 1, EJamEvent::UltimateSlash);
	TestEqual(TEXT("one slash after the wind-up"), Slashes, 1);
	TestEqual(TEXT("the boss took it"), F.Fighters[EJamSide::Boss].Health, B.BossHealth - B.UltimateDamage);
	TestEqual(TEXT("the meter is spent, the slash does not refill it"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateMissTest, "Jam.Ultimate.MissesOffTheLine", JamTest::Flags)

bool FJamUltimateMissTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = Ready(B, FVector2D(0.0, 700.0));
	JamTest::Run(F, Pressing(), B, 1);
	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.UltimateWindUp) + 1);
	TestEqual(TEXT("the boss beside the line took nothing"), F.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestEqual(TEXT("the meter is spent all the same"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUltimateGateTest, "Jam.Ultimate.NeedsAFullMeterAndTheMechanic", JamTest::Flags)

bool FJamUltimateGateTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight Half = Ready(B, FVector2D(700.0, 0.0));
	Half.Fighters[EJamSide::Hero].Ultimate.Meter = 0.5f;
	TestEqual(TEXT("half a meter: E does nothing"), JamTest::RunCounting(Half, Pressing(), B, 1, EJamEvent::UltimateStarted), 0);

	FJamFight Stolen = Ready(B, FVector2D(700.0, 0.0));
	JamLoadoutRules::Give(Stolen.Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	TestEqual(TEXT("stolen: E does nothing"), JamTest::RunCounting(Stolen, Pressing(), B, 1, EJamEvent::UltimateStarted), 0);
	TestEqual(TEXT("the name on the card"), FString(JamLoadoutRules::Name(EJamMechanic::Ultimate)), FString(TEXT("ULTIMATE")));
	return true;
}

#endif
