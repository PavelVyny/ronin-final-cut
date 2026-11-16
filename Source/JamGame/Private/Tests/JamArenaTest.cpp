#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamLayerRules.h"
#include "Tests/JamTestHelpers.h"
#include "View/JamArenaRules.h"

/**
 * T-081: the rooftop shrine arena of the full art, the bare shrine after the first cut, the grid after the second;
 * the wet floor splashes under the fighters' feet. A bug here looks in game like "the shrine is still there in the
 * grey world", "the props stay after the first cut" or "the hero splashes on every frame / never".
 *
 * Run: Automation RunTests Jam.Arena
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamArenaLookTest, "Jam.Arena.EachTierHasItsLook", JamTest::Flags)

bool FJamArenaLookTest::RunTest(const FString& Parameters)
{
	FJamLayers L;
	TestTrue(TEXT("the full art is the shrine"), JamArenaRules::LookFor(L) == EJamArenaLook::Shrine);

	L.bArt = false;
	L.bEffects = false;
	TestTrue(TEXT("the first cut leaves the bare shrine"), JamArenaRules::LookFor(L) == EJamArenaLook::BareShrine);

	L.bMaterials = false;
	TestTrue(TEXT("the second cut leaves the grid"), JamArenaRules::LookFor(L) == EJamArenaLook::Greybox);

	FJamLayers ArtWithoutMaterials;
	ArtWithoutMaterials.bMaterials = false;
	TestTrue(TEXT("no materials is the grid even with the art"), JamArenaRules::LookFor(ArtWithoutMaterials) == EJamArenaLook::Greybox);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamArenaSplashTest, "Jam.Arena.OneSplashPerStride", JamTest::Flags)

bool FJamArenaSplashTest::RunTest(const FString& Parameters)
{
	float Carry = 0.f;
	TestEqual(TEXT("standing still makes no splash"), JamArenaRules::StepSplashes(Carry, 0.f, 100.f), 0);
	TestEqual(TEXT("less than a stride makes no splash"), JamArenaRules::StepSplashes(Carry, 60.f, 100.f), 0);
	TestEqual(TEXT("the stride is completed across frames"), JamArenaRules::StepSplashes(Carry, 60.f, 100.f), 1);
	TestEqual(TEXT("the rest is carried"), Carry, 20.f);
	TestEqual(TEXT("a dash covers several strides at once"), JamArenaRules::StepSplashes(Carry, 380.f, 100.f), 4);

	float Never = 0.f;
	TestEqual(TEXT("a zero stride never splashes"), JamArenaRules::StepSplashes(Never, 500.f, 0.f), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamArenaBackdropTest, "Jam.Arena.TheSkylineCoversAnyScreen", JamTest::Flags)

bool FJamArenaBackdropTest::RunTest(const FString& Parameters)
{
	// T-094: on an ultrawide screen the skyline left strips at both sides and cut off the tower's dollar sign.
	const FVector Eye(0.0, 1200.0, 170.0);
	const FVector LookAt(0.0, 0.0, 125.0);
	constexpr float Fov = 50.f;
	constexpr double Distance = 10200.0;
	constexpr float Picture = 3.f;
	const double HalfTanY = FMath::Tan(FMath::DegreesToRadians(0.5 * Fov)) / (16.0 / 9.0);
	const double CentreZ = Eye.Z + Distance * (LookAt.Z - Eye.Z) / (Eye.Y - LookAt.Y);
	for (const float Aspect : { 4.f / 3.f, 16.f / 9.f, 21.f / 9.f, 32.f / 9.f, 5.f })
	{
		const FJamBackdropPlace Place = JamArenaRules::PlaceBackdrop(Eye, LookAt, Fov, Aspect, Picture, Distance);
		const double HalfH = Distance * HalfTanY;
		const double HalfW = HalfH * Aspect;
		const FString At = FString::Printf(TEXT(" at %.2f:1"), Aspect);
		TestTrue(
			TEXT("the picture keeps its shape") + At, FMath::IsNearlyEqual(Place.Width / Place.Height, static_cast<double>(Picture), 1e-6));
		TestTrue(TEXT("no strip at the sides") + At, 0.5 * Place.Width >= HalfW);
		TestTrue(TEXT("none at the top") + At, Place.CentreZ + 0.5 * Place.Height >= CentreZ + HalfH);
		TestTrue(TEXT("none at the bottom") + At, Place.CentreZ - 0.5 * Place.Height <= CentreZ - HalfH);
		// The tower's crown is in the top tenth of the picture: it has to be on the screen.
		TestTrue(TEXT("the picture's top tenth shows") + At, Place.CentreZ + 0.4 * Place.Height <= CentreZ + HalfH);
	}
	const FJamBackdropPlace Wide = JamArenaRules::PlaceBackdrop(Eye, LookAt, Fov, 16.f / 9.f, Picture, Distance);
	TestTrue(TEXT("on a usual screen the whole height shows"), Wide.Height <= 2.0 * Distance * HalfTanY * 1.05);
	return true;
}

#endif
