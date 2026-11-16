#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamBalance.h"
#include "Core/JamConfig.h"
#include "Sim/JamLayerRules.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamNeonBarRules.h"

/**
 * T-104: the full art's neon bars (Submission/hud_mock_neon.png). A bug looks like "the bar does not reach its end",
 * "a segment is lit past the health", "the gaps are uneven", "the cut marks are not where he steals" or "the Ultimate
 * pulses before it is full".
 *
 * Run: Automation RunTests Jam.Hud.Neon
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonSegmentsTest, "Jam.Hud.NeonSegmentsTileTheBarUpToTheFill", JamTest::Flags)

bool FJamNeonSegmentsTest::RunTest(const FString& Parameters)
{
	constexpr float Gap = 0.005f;
	const TArray<FJamBarSegment> Full = JamNeonBarRules::Segments(40, Gap, 1.f);
	if (!TestEqual(TEXT("forty segments"), Full.Num(), 40))
	{
		return false;
	}
	TestEqual(TEXT("the first starts at the bar's start"), Full[0].From, 0.f);
	TestEqual(TEXT("the last ends at the bar's end"), Full.Last().To, 1.f, 1e-4f);
	const float Width = Full[0].To - Full[0].From;
	for (int32 Index = 1; Index < Full.Num(); ++Index)
	{
		TestEqual(TEXT("one gap between segments"), Full[Index].From - Full[Index - 1].To, Gap, 1e-4f);
		TestEqual(TEXT("segments of one width"), Full[Index].To - Full[Index].From, Width, 1e-4f);
		TestTrue(TEXT("a full bar is lit to the end"), Full[Index].FillTo == Full[Index].To);
	}

	// 3359 of 4000: the segment the health falls in is lit up to it, the ones after it are empty.
	const float Fill = 3359.f / 4000.f;
	int32 Lit = 0;
	for (const FJamBarSegment& Segment : JamNeonBarRules::Segments(40, Gap, Fill))
	{
		TestTrue(TEXT("nothing is lit past the health"), Segment.FillTo <= FMath::Max(Fill, Segment.From) + 1e-5f);
		TestTrue(TEXT("a segment before the health is lit whole"), Segment.To > Fill || Segment.FillTo == Segment.To);
		Lit += Segment.IsLit() ? 1 : 0;
	}
	TestEqual(TEXT("34 segments lit, the last one in part"), Lit, 34);
	TestFalse(TEXT("an empty bar lights nothing"), JamNeonBarRules::Segments(20, Gap, 0.f)[0].IsLit());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonCutMarksTest, "Jam.Hud.NeonCutMarksAreWhereHeSteals", JamTest::Flags)

bool FJamNeonCutMarksTest::RunTest(const FString& Parameters)
{
	FJamBalanceData Balance = JamTest::Balance();
	Balance.StealThreshold1 = 0.76f;
	Balance.StealThreshold2 = 0.52f;
	Balance.StealThreshold3 = 0.27f;
	Balance.StealThreshold4 = 0.03f;
	const TArray<float> Marks = JamNeonBarRules::CutMarks(Balance);
	if (!TestEqual(TEXT("three marks"), Marks.Num(), JamNeonBarRules::CutMarkCount))
	{
		return false;
	}
	TestEqual(TEXT("the first cut"), Marks[0], 0.76f, 1e-4f);
	TestEqual(TEXT("the second cut"), Marks[1], 0.52f, 1e-4f);
	TestEqual(TEXT("the third cut"), Marks[2], 0.27f, 1e-4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonSlantTest, "Jam.Hud.NeonFrameAndSegmentsSlantAlike", JamTest::Flags)

bool FJamNeonSlantTest::RunTest(const FString& Parameters)
{
	// The boss bar: 20 px tall, its plate 5 px bigger all round, a segment inside; one slope, one bottom line.
	constexpr float Slope = 0.6f;
	constexpr float Top = 50.f;
	constexpr float Bottom = 70.f;
	constexpr float Pad = 5.f;
	const FJamSlantQuad Plate = JamNeonBarRules::Slanted(500.f - Pad, Top - Pad, 1200.f + Pad, Bottom + Pad, Bottom, Slope);
	const FJamSlantQuad Segment = JamNeonBarRules::Slanted(700.f, Top, 716.f, Bottom, Bottom, Slope);
	const auto Cross = [](const FVector2D& A, const FVector2D& B) { return static_cast<float>(A.X * B.Y - A.Y * B.X); };
	const FVector2D PlateLeft = Plate.Corners[0] - Plate.Corners[3];
	const FVector2D PlateRight = Plate.Corners[1] - Plate.Corners[2];
	const FVector2D SegmentLeft = Segment.Corners[0] - Segment.Corners[3];
	TestTrue(TEXT("the edges have a length"), SegmentLeft.Size() > 1.f && PlateLeft.Size() > 1.f);
	TestEqual(TEXT("the frame's and a segment's left edges are parallel"), Cross(PlateLeft, SegmentLeft), 0.f, 1e-2f);
	TestEqual(TEXT("so are the frame's right edge and the segment's"), Cross(PlateRight, SegmentLeft), 0.f, 1e-2f);
	TestEqual(TEXT("a segment's bottom sits on the given x"), static_cast<float>(Segment.Corners[3].X), 700.f, 1e-3f);
	TestEqual(TEXT("its top leans right by height times slope"), static_cast<float>(Segment.Corners[0].X), 700.f + 20.f * Slope, 1e-3f);
	TestEqual(TEXT("the plate's bottom, under the line, leans back"), static_cast<float>(Plate.Corners[3].X), 495.f - Pad * Slope, 1e-3f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonStagesTest, "Jam.Hud.NeonBarsLastTwoStages", JamTest::Flags)

bool FJamNeonStagesTest::RunTest(const FString& Parameters)
{
	// The author's play-through: the neon bars outlive the world's art by a cut, like the hero's own look.
	const FJamLayerSettings S = JamConfig::Load<FJamLayerSettings>(JamLayerRules::Section);
	TestTrue(TEXT("neon before the first cut"), JamNeonBarRules::Shows(JamLayerRules::For(0, EJamPhase::Fight, S)));
	TestTrue(TEXT("and after it"), JamNeonBarRules::Shows(JamLayerRules::For(1, EJamPhase::Fight, S)));
	TestFalse(TEXT("the second cut takes them"), JamNeonBarRules::Shows(JamLayerRules::For(2, EJamPhase::Fight, S)));
	TestTrue(TEXT("the stun gives them back"), JamNeonBarRules::Shows(JamLayerRules::For(3, EJamPhase::Stunned, S)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonPulseTest, "Jam.Hud.NeonUltimatePulsesOnlyWhenFull", JamTest::Flags)

bool FJamNeonPulseTest::RunTest(const FString& Parameters)
{
	bool bSeenBright = false;
	for (int32 Step = 0; Step < 60; ++Step)
	{
		const float Seconds = Step / 60.f;
		TestEqual(TEXT("no pulse at 96%"), JamNeonBarRules::UltimatePulse(0.96f, Seconds, 1.5f), 0.f);
		const float Pulse = JamNeonBarRules::UltimatePulse(1.f, Seconds, 1.5f);
		TestTrue(TEXT("a full bar's pulse stays in 0..1"), Pulse >= 0.f && Pulse <= 1.f);
		bSeenBright |= Pulse > 0.9f;
	}
	TestTrue(TEXT("a full bar pulses"), bSeenBright);
	return true;
}

#endif
