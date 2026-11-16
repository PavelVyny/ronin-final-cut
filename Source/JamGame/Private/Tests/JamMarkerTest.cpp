#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamMarkerRules.h"

/**
 * T-045: the author's playtest: when the Publisher is off screen, an arrow at the edge shows where he is. A bug looks
 * like "an arrow over the boss I can see", "the arrow points the wrong way" or "it sits off the screen".
 *
 * Run: Automation RunTests Jam.Hud
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudMarkerTest, "Jam.Hud.AnArrowPointsAtTheBossOffScreen", JamTest::Flags)

bool FJamHudMarkerTest::RunTest(const FString& Parameters)
{
	using JamMarkerRules::EdgeMarker;
	const FVector2D View(1000.0, 600.0);
	TestFalse(TEXT("on screen: no arrow"), EdgeMarker(FVector2D(700.0, 200.0), View, 40.f, 40.f).bShow);

	const FJamEdgeMarker Right = EdgeMarker(FVector2D(1500.0, 300.0), View, 40.f, 40.f);
	TestTrue(TEXT("off to the right: an arrow"), Right.bShow);
	TestTrue(TEXT("on the right edge, level with him"), Right.Pos.Equals(FVector2D(960.0, 300.0), 0.5));
	TestEqual(TEXT("pointing right"), Right.Angle, 0.f, 0.5f);

	const FJamEdgeMarker Above = EdgeMarker(FVector2D(500.0, -900.0), View, 40.f, 40.f);
	TestTrue(TEXT("far above: on the top edge"), Above.Pos.Equals(FVector2D(500.0, 40.0), 0.5));
	TestEqual(TEXT("pointing up"), Above.Angle, -90.f, 0.5f);

	const FJamEdgeMarker Corner = EdgeMarker(FVector2D(-2000.0, 1300.0), View, 40.f, 40.f);
	TestTrue(TEXT("down and left: inside the inset screen"),
		Corner.Pos.X >= 40.0 - 0.5 && Corner.Pos.X <= 960.0 + 0.5 && Corner.Pos.Y >= 40.0 - 0.5 && Corner.Pos.Y <= 560.0 + 0.5);
	TestTrue(TEXT("on its edge"), FMath::IsNearlyEqual(Corner.Pos.X, 40.0, 0.5) || FMath::IsNearlyEqual(Corner.Pos.Y, 560.0, 0.5));
	TestTrue(TEXT("pointing down and left"), Corner.Angle > 90.f && Corner.Angle < 180.f);

	TestTrue(TEXT("just inside the margin counts as off screen"), EdgeMarker(FVector2D(980.0, 300.0), View, 40.f, 40.f).bShow);
	return true;
}

/**
 * T-047: the author's playtest: with the Publisher off to the north, the arrow touched his health bar and read as a
 * triangle growing out of it. The top limit of the arrow lies under the bar.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudMarkerBarTest, "Jam.Hud.TheArrowKeepsClearOfTheBossBar", JamTest::Flags)

bool FJamHudMarkerBarTest::RunTest(const FString& Parameters)
{
	using JamMarkerRules::EdgeMarker;
	const FVector2D View(1000.0, 600.0);
	const FJamEdgeMarker North = EdgeMarker(FVector2D(500.0, -900.0), View, 40.f, 120.f);
	TestTrue(TEXT("far north: an arrow"), North.bShow);
	TestTrue(TEXT("under the bar, not on it"), North.Pos.Equals(FVector2D(500.0, 120.0), 0.5));
	TestEqual(TEXT("still pointing up"), North.Angle, -90.f, 0.5f);
	const FJamEdgeMarker NorthEast = EdgeMarker(FVector2D(1600.0, -500.0), View, 40.f, 120.f);
	TestTrue(TEXT("north-east: never above the limit"), NorthEast.Pos.Y >= 120.0 - 0.5);
	TestTrue(TEXT("a point behind the bar counts as off screen"), EdgeMarker(FVector2D(500.0, 80.0), View, 40.f, 120.f).bShow);
	TestEqual(TEXT("the sides keep their margin"), static_cast<float>(EdgeMarker(FVector2D(1500.0, 300.0), View, 40.f, 120.f).Pos.X), 960.f,
		0.5f);
	return true;
}

#endif
