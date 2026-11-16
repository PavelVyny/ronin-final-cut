#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamBalanceRules.h"
#include "Core/JamConfig.h"
#include "Sim/JamStepRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-001: the fixed step of the simulation and the balance data.
 * A bug here looks in game like "the fight runs faster on a fast PC" or "a missing number makes the boss die at once".
 *
 * Run: Automation RunTests Jam.Sim / Jam.Balance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFixedStepShortFrameTest, "Jam.Sim.FixedStep.ShortFrameKeepsTheRemainder", JamTest::Flags)

bool FJamFixedStepShortFrameTest::RunTest(const FString& Parameters)
{
	const JamStepRules::FFrameSteps Result = JamStepRules::StepsFor(0.f, 0.004f, 1.f / 60.f, 8);
	TestEqual(TEXT("a frame shorter than a step runs no step"), Result.Steps, 0);
	TestEqual(TEXT("its time is kept for the next frame"), Result.Remainder, 0.004f, 1e-6f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFixedStepWholeStepsTest, "Jam.Sim.FixedStep.RunsWholeSteps", JamTest::Flags)

bool FJamFixedStepWholeStepsTest::RunTest(const FString& Parameters)
{
	const float Step = 1.f / 60.f;
	const JamStepRules::FFrameSteps Result = JamStepRules::StepsFor(Step * 0.5f, Step * 2.f, Step, 8);
	TestEqual(TEXT("half a step kept plus two steps of frame time is two steps"), Result.Steps, 2);
	TestEqual(TEXT("and half a step is left"), Result.Remainder, Step * 0.5f, 1e-5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFixedStepHitchTest, "Jam.Sim.FixedStep.HitchIsCapped", JamTest::Flags)

bool FJamFixedStepHitchTest::RunTest(const FString& Parameters)
{
	const JamStepRules::FFrameSteps Result = JamStepRules::StepsFor(0.f, 2.f, 1.f / 60.f, 8);
	TestEqual(TEXT("a two second hitch runs only the allowed steps"), Result.Steps, 8);
	TestEqual(TEXT("and the backlog is dropped, not replayed"), Result.Remainder, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceIniTest, "Jam.Balance.IniIsCompleteAndValid", JamTest::Flags)

bool FJamBalanceIniTest::RunTest(const FString& Parameters)
{
	TArray<FString> Missing;
	const FJamBalanceData Balance = JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection, &Missing);
	TestEqual(TEXT("every balance field has a line in DefaultGame.ini, missing: ") + FString::Join(Missing, TEXT(", ")), Missing.Num(), 0);
	const TArray<FString> Problems = JamBalanceRules::Problems(Balance);
	TestEqual(TEXT("the shipped numbers are valid: ") + FString::Join(Problems, TEXT("; ")), Problems.Num(), 0);
	return true;
}

/**
 * T-041: the Publisher is the heavy fat-cat boss of concept C3b (the author's pick, Sat 14 Nov): slow and big, his
 * blows wound up long enough to read and hard when they land. Checked against the hero, so a retune keeps the shape.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceHeavyTest, "Jam.Balance.ThePublisherIsHeavy", JamTest::Flags)

bool FJamBalanceHeavyTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
	TestTrue(TEXT("slower than half the hero"), B.BossSpeed <= 0.45f * B.HeroSpeed);
	TestTrue(TEXT("more than twice his width"), B.BossRadius >= 2.2f * B.HeroRadius);
	TestTrue(TEXT("a slam wound up for a full second"), B.SlamTelegraph >= 1.f);
	TestTrue(TEXT("a sword lifted slowly"), B.BossKatanaWindUp >= 0.55f);
	// T-100: the author gave the hero a fifth more health (a third of 100 was the line before): a slam still takes over a quarter.
	TestTrue(TEXT("a slam takes over a quarter of the hero"), B.SlamDamage >= B.HeroHealth / 4.f);
	return true;
}

/**
 * T-043: the author's playtest (Sat 14 Nov 14:20): the fight back to 4000 health, the stretch after the FINAL NOTE
 * (the ball only) short, and time to think on the steal card. A bug looks like "the last part drags and frustrates".
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceFinalStretchTest, "Jam.Balance.TheFinalStretchIsShort", JamTest::Flags)

bool FJamBalanceFinalStretchTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
	TestEqual(TEXT("the Publisher has 4000 health"), B.BossHealth, 4000.f);
	const float LastStretch = B.StealThreshold4 * B.BossHealth;
	const float FiringSeconds = LastStretch / B.BallDamage * B.BallCooldown;
	TestTrue(FString::Printf(TEXT("the ball alone ends it in %.0f s of steady fire, at most 12"), FiringSeconds), FiringSeconds <= 12.f);
	const float ShortestOther = FMath::Min(B.StealThreshold3 - B.StealThreshold4, 1.f - B.StealThreshold1) * B.BossHealth;
	TestTrue(TEXT("it is the shortest stretch by far"), LastStretch * 4.f <= ShortestOther);
	TestTrue(TEXT("the steal card waits long enough to read both cards"), B.StealChoiceSeconds >= 6.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceZeroTest, "Jam.Balance.ReportsAMissingNumber", JamTest::Flags)

bool FJamBalanceZeroTest::RunTest(const FString& Parameters)
{
	FJamBalanceData Balance = JamTest::Balance();
	TestEqual(TEXT("the test balance is valid"), JamBalanceRules::Problems(Balance).Num(), 0);
	Balance.BossHealth = 0.f;
	const TArray<FString> Problems = JamBalanceRules::Problems(Balance);
	TestEqual(TEXT("a zero is one problem"), Problems.Num(), 1);
	TestTrue(TEXT("and it names the field"), Problems.Num() == 1 && Problems[0].Contains(TEXT("BossHealth")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceArenaTest, "Jam.Balance.FightersMustFitTheArena", JamTest::Flags)

bool FJamBalanceArenaTest::RunTest(const FString& Parameters)
{
	FJamBalanceData Balance = JamTest::Balance();
	Balance.ArenaRadius = Balance.BossRadius;
	TestTrue(TEXT("an arena no wider than the boss is a problem"), JamBalanceRules::Problems(Balance).Num() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBalanceThresholdsTest, "Jam.Balance.StealThresholdsGoDown", JamTest::Flags)

bool FJamBalanceThresholdsTest::RunTest(const FString& Parameters)
{
	FJamBalanceData Swapped = JamTest::Balance();
	Swapped.StealThreshold2 = 0.8f;
	TestEqual(TEXT("a second threshold above the first is a problem"), JamBalanceRules::Problems(Swapped).Num(), 1);

	FJamBalanceData Last = JamTest::Balance();
	Last.StealThreshold4 = Last.StealThreshold3;
	TestEqual(TEXT("a final note that is not below the third cut is a problem"), JamBalanceRules::Problems(Last).Num(), 1);

	FJamBalanceData Full = JamTest::Balance();
	Full.StealThreshold1 = 1.f;
	TestEqual(TEXT("a steal at full health is a problem: the fight would open with the card"), JamBalanceRules::Problems(Full).Num(), 1);

	FJamBalanceData Locked = JamTest::Balance();
	Locked.StealChoiceLock = Locked.StealChoiceSeconds;
	TestEqual(TEXT("a card that is locked for its whole time is a problem"), JamBalanceRules::Problems(Locked).Num(), 1);
	return true;
}

#endif
