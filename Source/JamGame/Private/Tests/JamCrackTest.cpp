#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamCrackRules.h"

/**
 * T-068, T-071: the crack between two looks. A bug here looks in game like "the screen splits before I see what
 * happened", "the split is a straight line", "the old picture never goes away" or "the crack's edge flickers".
 *
 * Run: Automation RunTests Jam.Crack
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCrackRunTest, "Jam.Crack.RunsDownThenHoldsThenParts", JamTest::Flags)

bool FJamCrackRunTest::RunTest(const FString& Parameters)
{
	using namespace JamCrackRules;
	TestEqual(TEXT("not started at the start"), Reach(0.f), 0.f);
	TestTrue(TEXT("runs down"), Reach(0.25f * RunSeconds) > 0.f && Reach(0.75f * RunSeconds) > Reach(0.25f * RunSeconds));
	TestEqual(TEXT("at the bottom when it has run"), Reach(RunSeconds), 1.f, 1e-4f);
	TestEqual(TEXT("and stays there"), Reach(Seconds), 1.f, 1e-4f);
	TestEqual(TEXT("the halves stay put while it runs"), Shift(RunSeconds), 0.f);
	TestEqual(TEXT("and while it glows whole"), Shift(RunSeconds + HoldSeconds), 0.f);
	TestFalse(TEXT("not over while the halves slide"), IsOver(RunSeconds + HoldSeconds + 0.5f * PartSeconds, Seconds));
	TestTrue(TEXT("over at the end"), IsOver(Seconds, Seconds));
	TestTrue(TEXT("two to three times the first crack's 0.7 s"), Seconds >= 1.4f && Seconds <= 2.1f + 1e-4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCrackPartTest, "Jam.Crack.HalvesPartSlowlyThenLeave", JamTest::Flags)

bool FJamCrackPartTest::RunTest(const FString& Parameters)
{
	using namespace JamCrackRules;
	const float From = RunSeconds + HoldSeconds;
	const float Early = Shift(From + 0.1f * PartSeconds);
	const float Mid = Shift(From + 0.5f * PartSeconds);
	const float Late = Shift(From + 0.9f * PartSeconds);
	TestTrue(TEXT("they move"), Early > 0.f && Mid > Early && Late > Mid);
	TestTrue(TEXT("slow at first: the eye follows which half goes where"), Early < 0.1f * Mid);
	TestTrue(TEXT("both leave the screen: each slides past the fault's furthest place"), Shift(Seconds) >= 1.f - FaultMargin - 1e-4f);
	TestEqual(TEXT("and stay gone"), Shift(2.f * Seconds), Shift(Seconds), 1e-4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCrackFaultTest, "Jam.Crack.TheFaultWandersAndKeepsItsShape", JamTest::Flags)

bool FJamCrackFaultTest::RunTest(const FString& Parameters)
{
	using namespace JamCrackRules;
	constexpr int32 Samples = 96;
	for (int32 Seed = 1; Seed <= 6; ++Seed)
	{
		const FString Name = FString::Printf(TEXT("seed %d"), Seed);
		const float Top = FaultX(0.f, Seed);
		const float Bottom = FaultX(1.f, Seed);
		float Stray = 0.f;
		float Jump = 0.f;
		bool bInside = true;
		for (int32 Index = 0; Index <= Samples; ++Index)
		{
			const float V = static_cast<float>(Index) / Samples;
			const float X = FaultX(V, Seed);
			bInside &= X >= FaultMargin && X <= 1.f - FaultMargin;
			Stray = FMath::Max(Stray, FMath::Abs(X - FMath::Lerp(Top, Bottom, V)));
			if (Index > 0)
			{
				Jump = FMath::Max(Jump, FMath::Abs(X - FaultX(static_cast<float>(Index - 1) / Samples, Seed)));
			}
		}
		TestTrue(Name + TEXT(": clear of the screen's edges"), bInside);
		TestTrue(Name + TEXT(": not a straight line, it wanders off it"), Stray > 0.06f);
		TestTrue(Name + TEXT(": one line, no jumps across the screen"), Jump < 0.06f);
		TestEqual(Name + TEXT(": the same place every frame"), FaultX(0.37f, Seed), FaultX(0.37f, Seed));
	}
	TestNotEqual(TEXT("another crack has another path"), FaultX(0.37f, 7), FaultX(0.37f, 8));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
