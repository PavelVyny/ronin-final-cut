#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "View/JamSpotRules.h"

/**
 * T-087: the key light on the Publisher is worked by a hand: it trails him by about half a second and sways a little.
 * A bug here looks in game like "the spot is glued to him", "the spot overshoots and swings back" or "the spot shakes".
 *
 * Run: Automation RunTests Jam.Spot
 */
namespace
{
constexpr float SpotLag = 0.5f;
constexpr float SpotStep = 1.f / 60.f;

FJamSpot FollowFor(FJamSpot Spot, const FVector2D& Target, float Seconds, float Dt)
{
	for (float T = 0.f; T < Seconds - 0.5f * Dt; T += Dt)
	{
		Spot = JamSpotRules::Follow(Spot, Target, Dt, SpotLag);
	}
	return Spot;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSpotTrailTest, "Jam.Spot.TrailsHimByHalfASecondWithoutOvershoot", JamTest::Flags)

bool FJamSpotTrailTest::RunTest(const FString& Parameters)
{
	const FVector2D Target(100.0, 0.0);
	const FJamSpot Start;
	TestTrue(TEXT("not glued: a step moves it a little"), JamSpotRules::Follow(Start, Target, SpotStep, SpotLag).At.X < 5.0);
	const double Half = FollowFor(Start, Target, SpotLag, SpotStep).At.X;
	TestTrue(TEXT("after the lag it has come about half the way"), Half > 45.0 && Half < 75.0);
	double Most = 0.0;
	FJamSpot Spot = Start;
	for (int32 Index = 0; Index < 240; ++Index)
	{
		Spot = JamSpotRules::Follow(Spot, Target, SpotStep, SpotLag);
		Most = FMath::Max(Most, Spot.At.X);
	}
	TestTrue(TEXT("it never swings past him"), Most <= Target.X + 1e-3);
	TestTrue(TEXT("and settles on him"), FMath::Abs(Spot.At.X - Target.X) < 1.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSpotWalkTest, "Jam.Spot.FollowsAWalkHalfASecondBehind", JamTest::Flags)

bool FJamSpotWalkTest::RunTest(const FString& Parameters)
{
	// He walks at 260 cm/s: once the spot has caught his pace it trails him by the distance of half a second.
	constexpr double Speed = 260.0;
	FJamSpot Spot;
	double X = 0.0;
	for (int32 Index = 0; Index < 300; ++Index)
	{
		X += Speed * SpotStep;
		Spot = JamSpotRules::Follow(Spot, FVector2D(X, 0.0), SpotStep, SpotLag);
	}
	const double Behind = X - Spot.At.X;
	TestTrue(
		FString::Printf(TEXT("about half a second behind: %.1f cm"), Behind), FMath::Abs(Behind - Speed * SpotLag) < 0.1 * Speed * SpotLag);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSpotFrameTest, "Jam.Spot.KeepsItsPathAtAnyFrameRate", JamTest::Flags)

bool FJamSpotFrameTest::RunTest(const FString& Parameters)
{
	const FVector2D Target(-300.0, 200.0);
	const FJamSpot Smooth = FollowFor(FJamSpot(), Target, 0.6f, 1.f / 240.f);
	const FJamSpot Choppy = FollowFor(FJamSpot(), Target, 0.6f, 0.1f);
	TestTrue(TEXT("a slow frame rate follows the same path"), (Smooth.At - Choppy.At).Size() < 15.0);
	const FJamSpot Stall = JamSpotRules::Follow(FJamSpot(), Target, 2.f, SpotLag);
	TestTrue(TEXT("a long stall does not throw it past him"), (Stall.At - Target).Size() < (Target).Size());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSpotSwayTest, "Jam.Spot.SwaysALittleLikeAHand", JamTest::Flags)

bool FJamSpotSwayTest::RunTest(const FString& Parameters)
{
	constexpr float Reach = 20.f;
	double Most = 0.0;
	double Jump = 0.0;
	FVector2D Last = JamSpotRules::Sway(0.f, Reach);
	for (float T = SpotStep; T < 30.f; T += SpotStep)
	{
		const FVector2D Now = JamSpotRules::Sway(T, Reach);
		Most = FMath::Max(Most, Now.Size());
		Jump = FMath::Max(Jump, (Now - Last).Size());
		Last = Now;
	}
	TestTrue(TEXT("it sways"), Most > 0.5 * Reach);
	TestTrue(TEXT("only a little"), Most <= Reach + 1e-3);
	TestTrue(TEXT("smoothly, not a shake"), Jump < 0.05 * Reach);
	TestEqual(TEXT("no sway without a reach"), JamSpotRules::Sway(3.f, 0.f).Size(), 0.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSpotSplitTest, "Jam.Spot.TheHerosSpotSplitsOffThePublishers", JamTest::Flags)

bool FJamSpotSplitTest::RunTest(const FString& Parameters)
{
	// T-100: the hero was too dark in the full art. His own spot buds off the Publisher's beam as the intro's camera
	// leaves him, rides with the camera and lands on the hero as he comes into frame.
	const FVector2D Boss(0.0, 0.0);
	const FVector2D Hero(-900.0, 300.0);
	TestTrue(TEXT("before the camera moves it is inside his beam"), JamSpotRules::SplitOff(Boss, Hero, 0.f).Equals(Boss));
	TestTrue(TEXT("halfway with the camera, halfway over"), JamSpotRules::SplitOff(Boss, Hero, 0.5f).Equals((Boss + Hero) * 0.5));
	TestTrue(TEXT("on the hero when the camera is"), JamSpotRules::SplitOff(Boss, Hero, 1.f).Equals(Hero));
	TestTrue(TEXT("never past him"), JamSpotRules::SplitOff(Boss, Hero, 1.5f).Equals(Hero));

	TestEqual(TEXT("dark inside the Publisher's beam: his light does not double"), JamSpotRules::SplitShare(0.f), 0.f);
	TestTrue(TEXT("it comes up as it leaves"), JamSpotRules::SplitShare(0.1f) > 0.f && JamSpotRules::SplitShare(0.1f) < 1.f);
	TestEqual(TEXT("full a quarter of the way over"), JamSpotRules::SplitShare(0.25f), 1.f);
	TestEqual(TEXT("and on the hero"), JamSpotRules::SplitShare(1.f), 1.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
