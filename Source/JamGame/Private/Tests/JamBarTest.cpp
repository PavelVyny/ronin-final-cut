#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamBarRules.h"

/**
 * T-053: the author's playtest: the health bars felt instant. A hit leaves the lost part white for a moment, a heal
 * fills the bar up to a light preview. A bug looks like "the bar jumps", "the white part never goes", "the white part
 * shrinks while he keeps hitting me" or "the heal is not shown".
 *
 * Run: Automation RunTests Jam.Hud
 */
namespace
{
constexpr float Frame = 1.f / 60.f;

FJamBarSettings Settings()
{
	FJamBarSettings S;
	S.ChunkHold = 0.6f;
	S.ChunkDrain = 1.f;
	S.HealFill = 0.5f;
	return S;
}

/** A bar that has seen the health Fraction. */
FJamBarState BarAt(float Fraction)
{
	FJamBarState Bar;
	JamBarRules::Track(Bar, Fraction, Frame, Settings());
	return Bar;
}

/** Frames of the bar with the health held at Fraction. */
void Hold(FJamBarState& Bar, float Fraction, float Seconds)
{
	for (float Time = 0.f; Time < Seconds - 0.5f * Frame; Time += Frame)
	{
		JamBarRules::Track(Bar, Fraction, Frame, Settings());
	}
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarFirstTest, "Jam.Hud.TheBarStartsAtTheHealth", JamTest::Flags)

bool FJamBarFirstTest::RunTest(const FString& Parameters)
{
	const FJamBarState Bar = BarAt(0.4f);
	TestEqual(TEXT("the first look shows the health"), Bar.Fill, 0.4f);
	TestEqual(TEXT("with no white part"), Bar.Chunk, 0.4f);
	TestEqual(TEXT("and no preview"), Bar.Preview, 0.4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarChunkTest, "Jam.Hud.DamageLeavesAWhiteChunkThatGrows", JamTest::Flags)

bool FJamBarChunkTest::RunTest(const FString& Parameters)
{
	FJamBarState Bar = BarAt(1.f);
	JamBarRules::Track(Bar, 0.7f, Frame, Settings());
	TestEqual(TEXT("a hit drops the solid part at once"), Bar.Fill, 0.7f);
	TestEqual(TEXT("and leaves the lost part white"), Bar.Chunk, 1.f);

	Hold(Bar, 0.7f, 0.4f);
	JamBarRules::Track(Bar, 0.5f, Frame, Settings());
	TestEqual(TEXT("the next hit inside the pause drops the solid part again"), Bar.Fill, 0.5f);
	TestEqual(TEXT("and the white part grows from the same top"), Bar.Chunk, 1.f);

	Hold(Bar, 0.5f, 0.55f);
	TestEqual(TEXT("before the pause is over the white part stays whole"), Bar.Chunk, 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarDrainTest, "Jam.Hud.TheChunkDrainsAfterAPause", JamTest::Flags)

bool FJamBarDrainTest::RunTest(const FString& Parameters)
{
	FJamBarState Bar = BarAt(1.f);
	JamBarRules::Track(Bar, 0.6f, Frame, Settings());
	Hold(Bar, 0.6f, 0.7f);
	TestTrue(TEXT("after the pause it drains, not vanishes"), Bar.Chunk < 1.f && Bar.Chunk > 0.6f);
	Hold(Bar, 0.6f, 0.5f);
	TestEqual(TEXT("and is gone at the solid part"), Bar.Chunk, 0.6f);
	TestEqual(TEXT("the solid part never moved"), Bar.Fill, 0.6f);

	JamBarRules::Track(Bar, 0.3f, Frame, Settings());
	TestEqual(TEXT("a new hit starts a new chunk from where the bar stood"), Bar.Chunk, 0.6f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarHealTest, "Jam.Hud.AHealShowsAPreviewAndFillsUp", JamTest::Flags)

bool FJamBarHealTest::RunTest(const FString& Parameters)
{
	FJamBarState Bar = BarAt(0.5f);
	JamBarRules::Track(Bar, 0.75f, Frame, Settings());
	TestEqual(TEXT("the gained part shows at once as a preview"), Bar.Preview, 0.75f);
	TestTrue(TEXT("the solid part does not jump"), Bar.Fill < 0.52f);

	Hold(Bar, 0.75f, 0.25f);
	TestTrue(TEXT("it fills up smoothly"), Bar.Fill > 0.55f && Bar.Fill < 0.7f);
	TestTrue(TEXT("the white part never sits under the solid part"), Bar.Chunk >= Bar.Fill);
	Hold(Bar, 0.75f, 0.4f);
	TestEqual(TEXT("and reaches the preview"), Bar.Fill, 0.75f);

	JamBarRules::Track(Bar, 0.9f, Frame, Settings());
	JamBarRules::Track(Bar, 0.6f, Frame, Settings());
	TestEqual(TEXT("a hit during a heal drops the solid part to the health"), Bar.Fill, 0.6f);
	TestEqual(TEXT("and the preview with it"), Bar.Preview, 0.6f);
	TestTrue(TEXT("the lost part is white"), Bar.Chunk >= 0.75f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarGameTest, "Jam.Hud.TheBarNumbersAreSet", JamTest::Flags)

bool FJamBarGameTest::RunTest(const FString& Parameters)
{
	const FJamBarSettings S = JamConfig::Load<FJamBarSettings>(JamBarRules::Section);
	TestTrue(TEXT("the chunk waits half a second to a second"), S.ChunkHold >= 0.5f && S.ChunkHold <= 1.f);
	TestTrue(TEXT("and drains"), S.ChunkDrain > 0.f);
	TestTrue(TEXT("a heal fills in under a second for a quarter of the bar"), S.HealFill >= 0.25f);
	return true;
}

#endif
