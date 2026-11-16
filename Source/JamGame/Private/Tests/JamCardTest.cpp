#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamCardRules.h"

/**
 * T-094: the steal card takes a click as well as A or D, and pops in instead of just appearing. A bug looks in game like
 * "clicking a choice does nothing", "a click between the two picks one" or "the card flickers in at full size".
 *
 * Run: Automation RunTests Jam.Card
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCardClickTest, "Jam.Card.TheChoicesTakeAClick", JamTest::Flags)

bool FJamCardClickTest::RunTest(const FString& Parameters)
{
	for (const FVector2D Screen : { FVector2D(1920.0, 1080.0), FVector2D(3440.0, 1440.0), FVector2D(1280.0, 720.0) })
	{
		const FBox2D Left = JamCardRules::OptionBox(0, 2, Screen);
		const FBox2D Right = JamCardRules::OptionBox(1, 2, Screen);
		TestEqual(TEXT("a click on the left choice"), JamCardRules::OptionAt(Left.GetCenter(), 2, Screen), 0);
		TestEqual(TEXT("on the right one"), JamCardRules::OptionAt(Right.GetCenter(), 2, Screen), 1);
		TestEqual(TEXT("between them, none"), JamCardRules::OptionAt(0.5 * (Left.GetCenter() + Right.GetCenter()), 2, Screen),
			static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("above them, none"), JamCardRules::OptionAt(FVector2D(Left.GetCenter().X, Left.Min.Y - 2.0), 2, Screen),
			static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("left of right, on the screen"), Left.Max.X < Right.Min.X && Left.Min.X > 0.0 && Right.Max.X < Screen.X);
		TestTrue(TEXT("centred"), FMath::IsNearlyEqual(Left.Min.X, Screen.X - Right.Max.X, 0.5));
	}
	const FVector2D Screen(1920.0, 1080.0);
	TestEqual(
		TEXT("the final cut's single option"), JamCardRules::OptionAt(JamCardRules::OptionBox(0, 1, Screen).GetCenter(), 1, Screen), 0);
	TestEqual(TEXT("no card, no option"), JamCardRules::OptionAt(Screen * 0.5, 0, Screen), static_cast<int32>(INDEX_NONE));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCardPopTest, "Jam.Card.PanelsPopInWithABounce", JamTest::Flags)

bool FJamCardPopTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("nothing as it opens"), JamCardRules::Pop(0.f), 0.f);
	float Peak = 0.f;
	for (float Age = 0.f; Age <= JamCardRules::PopSeconds; Age += 0.01f)
	{
		Peak = FMath::Max(Peak, JamCardRules::Pop(Age));
	}
	TestTrue(TEXT("it overshoots a little"), Peak > 1.02f && Peak < 1.2f);
	TestTrue(TEXT("fast: most of the way in a third of its time"), JamCardRules::Pop(JamCardRules::PopSeconds / 3.f) > 0.7f);
	TestEqual(TEXT("then settles"), JamCardRules::Pop(JamCardRules::PopSeconds), 1.f);
	TestEqual(TEXT("and stays"), JamCardRules::Pop(10.f), 1.f);
	TestEqual(TEXT("a choice waits for its beat"), JamCardRules::Pop(0.5f * JamCardRules::OptionDelay, JamCardRules::OptionDelay), 0.f);
	TestTrue(TEXT("then follows"), JamCardRules::Pop(JamCardRules::OptionDelay + 0.1f, JamCardRules::OptionDelay) > 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
