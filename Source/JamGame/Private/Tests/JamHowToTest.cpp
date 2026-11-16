#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamKeys.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamHowToRules.h"

/**
 * T-096: the HOW TO PLAY screen. A bug looks in game like "the screen names a key that does nothing", "a control is
 * missing from it", "the click that ended the comic skipped it", "it never goes away", "a label runs off the screen or
 * over another" or "a leader line points at the wrong key".
 *
 * Run: Automation RunTests Jam.HowTo
 */
namespace
{
const FJamHowToCallout* CalloutFor(const TArray<FJamHowToCallout>& Callouts, const FKey& Key)
{
	return Callouts.FindByPredicate([&Key](const FJamHowToCallout& C) { return C.Keys.Contains(Key); });
}

/** About what the HUD measures on a 1080-line screen, 13 px a character and two lines, scaled to Screen. */
TArray<FVector2D> SizesOf(const TArray<FJamHowToCallout>& Callouts, const FVector2D& Screen)
{
	const double Scale = Screen.Y / 1080.0;
	TArray<FVector2D> Sizes;
	for (const FJamHowToCallout& C : Callouts)
	{
		Sizes.Add(FVector2D(13.0 * FMath::Max(C.Caps.Len(), C.Label.Len()), 56.0) * Scale);
	}
	return Sizes;
}

/** Whether From lies on a drawn key of Callout. */
bool StartsAtKey(const FJamHowToLayout& L, const FJamHowToCallout& Callout, const FVector2D& From)
{
	return L.Keys.ContainsByPredicate(
		[&Callout, &From](const FJamHowToKey& K) { return Callout.Keys.Contains(K.Key) && K.Box.ExpandBy(1.0).IsInside(From); });
}

/** Callout Index's label in L: on the screen, clear of the board, the mouse and the labels after it, lines from its keys. */
void CheckLabel(
	FAutomationTestBase& Test, const FJamHowToLayout& L, const TArray<FJamHowToCallout>& Callouts, int32 Index, const FBox2D& Frame)
{
	const FBox2D& Box = L.Labels[Index];
	Test.TestTrue(*FString::Printf(TEXT("'%s' on the screen"), *Callouts[Index].Caps), Frame.IsInside(Box));
	Test.TestFalse(TEXT("off the keyboard"), Box.Intersect(L.Board));
	Test.TestFalse(TEXT("off the mouse"), Box.Intersect(L.Mouse));
	for (int32 Other = Index + 1; Other < Callouts.Num(); ++Other)
	{
		Test.TestFalse(
			*FString::Printf(TEXT("'%s' clear of '%s'"), *Callouts[Index].Caps, *Callouts[Other].Caps), Box.Intersect(L.Labels[Other]));
	}
	for (const FKey& Key : Callouts[Index].Keys)
	{
		const FJamHowToKey* Drawn = L.Keys.FindByPredicate([&Key](const FJamHowToKey& K) { return K.Key == Key; });
		if (Test.TestNotNull(TEXT("its key is drawn"), Drawn))
		{
			Test.TestTrue(TEXT("and lit"), Drawn->bLit);
		}
	}
	// A line from a key of the callout to its label; a row's callout lines only its nearest row (W of W A S D).
	Test.TestTrue(TEXT("lined"), !L.Lines[Index].IsEmpty());
	Test.TestTrue(TEXT("no more lines than keys"), L.Lines[Index].Num() <= Callouts[Index].Keys.Num());
	for (const TPair<FVector2D, FVector2D>& Line : L.Lines[Index])
	{
		Test.TestTrue(TEXT("the line starts at a key of the callout"), StartsAtKey(L, Callouts[Index], Line.Key));
		Test.TestTrue(TEXT("and ends at the label"), Box.ExpandBy(1.0).IsInside(Line.Value));
	}
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHowToKeysTest, "Jam.HowTo.TheLabelsAreTheRealKeys", JamTest::Flags)

bool FJamHowToKeysTest::RunTest(const FString& Parameters)
{
	const TArray<FJamHowToCallout> Callouts = JamHowToRules::Callouts(4);
	const FKey Bound[] = { JamKeys::Up(), JamKeys::Left(), JamKeys::Down(), JamKeys::Right(), JamKeys::Dash(), JamKeys::Ultimate(),
		JamKeys::Retry(), JamKeys::HowTo(), JamKeys::Katana(), JamKeys::Shurikens(), JamKeys::CardLeft(), JamKeys::CardRight(),
		JamKeys::Aim() };
	for (const FKey& Key : Bound)
	{
		TestNotNull(*FString::Printf(TEXT("%s is on the screen"), *Key.ToString()), CalloutFor(Callouts, Key));
	}
	for (const FJamHowToCallout& C : Callouts)
	{
		TestFalse(TEXT("every label says what it does"), C.Label.IsEmpty());
		for (const FKey& Key : C.Keys)
		{
			TestTrue(*FString::Printf(TEXT("'%s' names %s"), *C.Caps, *Key.ToString()), C.Caps.Contains(JamHowToRules::CapOf(Key)));
		}
	}
	TestEqual(TEXT("the dash"), JamHowToRules::CapOf(JamKeys::Dash()), FString(TEXT("SPACE")));
	TestEqual(TEXT("the Ultimate"), JamHowToRules::CapOf(JamKeys::Ultimate()), FString(TEXT("E")));
	TestEqual(TEXT("the katana"), JamHowToRules::CapOf(JamKeys::Katana()), FString(TEXT("LEFT CLICK")));
	TestEqual(TEXT("the shurikens"), JamHowToRules::CapOf(JamKeys::Shurikens()), FString(TEXT("RIGHT CLICK")));
	TestEqual(TEXT("the screen itself"), JamHowToRules::CapOf(JamKeys::HowTo()), FString(TEXT("F1")));
	const FJamHowToCallout* Katana = CalloutFor(Callouts, JamKeys::Katana());
	TestTrue(TEXT("the katana's click parries too"), Katana && Katana->Label.Contains(TEXT("parry")));
	const FJamHowToCallout* Stars = CalloutFor(Callouts, JamKeys::Shurikens());
	TestTrue(TEXT("every 4th shuriken explodes, the balance's number"), Stars && Stars->Label.Contains(TEXT("4th")));
	const FJamHowToCallout* Three = CalloutFor(JamHowToRules::Callouts(3), JamKeys::Shurikens());
	TestTrue(TEXT("the number follows the balance"), Three && Three->Label.Contains(TEXT("3rd")));
	TestEqual(TEXT("2nd"), JamHowToRules::Ordinal(2), FString(TEXT("2nd")));
	TestEqual(TEXT("11th"), JamHowToRules::Ordinal(11), FString(TEXT("11th")));
	TestEqual(TEXT("21st"), JamHowToRules::Ordinal(21), FString(TEXT("21st")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHowToTimeTest, "Jam.HowTo.AKeyOrTheCountdownStartsTheFight", JamTest::Flags)

bool FJamHowToTimeTest::RunTest(const FString& Parameters)
{
	using namespace JamHowToRules;
	TestEqual(TEXT("the countdown starts full"), SecondsLeft(0.f), FMath::CeilToInt32(AutoStartSeconds));
	TestEqual(TEXT("it counts whole seconds down"), SecondsLeft(AutoStartSeconds - 2.5f), 3);
	TestEqual(TEXT("and stops at 0"), SecondsLeft(AutoStartSeconds + 4.f), 0);
	TestFalse(TEXT("the click that ended the comic does not skip it"), IsOver(0.1f, true, true));
	TestTrue(TEXT("a key after the grace starts the fight"), IsOver(KeyGraceSeconds + 0.1f, true, true));
	TestFalse(TEXT("no key, time left: it stays"), IsOver(AutoStartSeconds - 1.f, false, true));
	TestTrue(TEXT("the countdown runs out"), IsOver(AutoStartSeconds, false, true));
	TestFalse(TEXT("over the fight there is no countdown"), IsOver(AutoStartSeconds * 4.f, false, false));
	TestTrue(TEXT("a key closes it there too"), IsOver(KeyGraceSeconds + 0.1f, true, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHowToLayoutTest, "Jam.HowTo.TheSchematicReadsOnEveryScreen", JamTest::Flags)

bool FJamHowToLayoutTest::RunTest(const FString& Parameters)
{
	const TArray<FJamHowToCallout> Callouts = JamHowToRules::Callouts(4);
	for (const FVector2D Screen : { FVector2D(1920.0, 1080.0), FVector2D(1280.0, 720.0), FVector2D(2560.0, 1080.0) })
	{
		const FJamHowToLayout L = JamHowToRules::Layout(Screen, Callouts, SizesOf(Callouts, Screen));
		const FBox2D Frame(FVector2D::ZeroVector, Screen);
		if (!TestEqual(TEXT("a box per label"), L.Labels.Num(), Callouts.Num()) ||
			!TestEqual(TEXT("lines per label"), L.Lines.Num(), Callouts.Num()))
		{
			return false;
		}
		TestTrue(TEXT("the keyboard is on the screen"), Frame.IsInside(L.Board));
		TestTrue(
			TEXT("the whole board, not only our keys"), L.Keys.FilterByPredicate([](const FJamHowToKey& K) { return !K.bLit; }).Num() > 20);
		for (int32 Index = 0; Index < Callouts.Num(); ++Index)
		{
			CheckLabel(*this, L, Callouts, Index, Frame);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHowToRowLineTest, "Jam.HowTo.ARowOfKeysTakesOneLine", JamTest::Flags)

bool FJamHowToRowLineTest::RunTest(const FString& Parameters)
{
	// The first capture: the lines of W A S D fanned through the number row and crossed each other.
	const TArray<FJamHowToCallout> Callouts = JamHowToRules::Callouts(4);
	const FVector2D Screen(1920.0, 1080.0);
	const FJamHowToLayout L = JamHowToRules::Layout(Screen, Callouts, SizesOf(Callouts, Screen));
	const int32 Move = Callouts.IndexOfByPredicate([](const FJamHowToCallout& C) { return C.Keys.Contains(JamKeys::Up()); });
	const int32 Card =
		Callouts.IndexOfByPredicate([](const FJamHowToCallout& C) { return C.Keys.Contains(JamKeys::CardRight()) && C.Keys.Num() == 2; });
	if (!TestTrue(TEXT("both callouts are there"), Move != INDEX_NONE && Card != INDEX_NONE && L.Lines.Num() == Callouts.Num()))
	{
		return false;
	}
	TestEqual(TEXT("W A S D: one line, to W"), L.Lines[Move].Num(), 1);
	TestEqual(TEXT("A / D share a row: a line to each"), L.Lines[Card].Num(), 2);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
