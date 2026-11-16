#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Tests/JamTestHelpers.h"
#include "View/JamCameraRules.h"

/**
 * T-004: where each of the three cameras stands and how the move input is turned for it.
 * A bug here looks in game like "W walks sideways in the top-down view" or "the shoulder camera is in front of the
 * hero".
 *
 * Run: Automation RunTests Jam.Camera
 */
namespace
{
FJamCameraSettings TestCamera()
{
	FJamCameraSettings S;
	S.TopDownYaw = 0.f;
	S.TopDownPitch = -55.f;
	S.TopDownDistance = 2000.f;
	S.TopDownFov = 50.f;
	S.ShoulderDistance = 400.f;
	S.ShoulderHeight = 160.f;
	S.ShoulderSide = 60.f;
	S.ShoulderFov = 80.f;
	S.EyeHeight = 150.f;
	S.FirstPersonFov = 90.f;
	S.MinPitch = -60.f;
	S.MaxPitch = 45.f;
	S.MouseSensitivity = 1.f;
	return S;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraTopDownTest, "Jam.Camera.TopDownLooksAtTheHero", JamTest::Flags)

bool FJamCameraTopDownTest::RunTest(const FString& Parameters)
{
	const FJamCameraSettings S = TestCamera();
	const FVector Hero(300.0, -200.0, 0.0);
	// The look angles of the player do not matter in this mode.
	const FJamView View = JamCameraRules::ViewFor(EJamCameraMode::TopDown, Hero, 123.f, 17.f, S);
	TestEqual(TEXT("it stands at its distance"), FVector::Distance(View.Location, Hero), static_cast<double>(S.TopDownDistance), 0.1);
	TestTrue(TEXT("above the arena"), View.Location.Z > 1000.0);
	const FVector ToHero = (Hero - View.Location).GetSafeNormal();
	TestTrue(TEXT("the hero is in the centre of the view"), FVector::DotProduct(ToHero, View.Rotation.Vector()) > 0.9999);
	TestEqual(TEXT("with the top-down field of view"), View.Fov, S.TopDownFov);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraShoulderTest, "Jam.Camera.ShoulderIsBehindTheHero", JamTest::Flags)

bool FJamCameraShoulderTest::RunTest(const FString& Parameters)
{
	const FJamCameraSettings S = TestCamera();
	const FVector Hero(0.0, 0.0, 0.0);
	const FJamView View = JamCameraRules::ViewFor(EJamCameraMode::Shoulder, Hero, 0.f, 0.f, S);
	TestTrue(TEXT("behind the hero along the look direction"), View.Location.X < -300.0);
	TestTrue(TEXT("shifted to the right shoulder"), View.Location.Y > 0.0);
	TestEqual(TEXT("at shoulder height"), View.Location.Z, static_cast<double>(S.ShoulderHeight), 0.1);
	TestEqual(TEXT("looking where the player looks"), View.Rotation.Yaw, 0.0, 1e-3);
	const FJamView Turned = JamCameraRules::ViewFor(EJamCameraMode::Shoulder, Hero, 90.f, 0.f, S);
	TestTrue(TEXT("turning the look moves the camera around the hero"), Turned.Location.Y < -300.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraFirstPersonTest, "Jam.Camera.FirstPersonIsAtEyeHeight", JamTest::Flags)

bool FJamCameraFirstPersonTest::RunTest(const FString& Parameters)
{
	const FJamCameraSettings S = TestCamera();
	const FJamView View = JamCameraRules::ViewFor(EJamCameraMode::FirstPerson, FVector(10.0, 20.0, 0.0), 45.f, -10.f, S);
	TestEqual(TEXT("at the hero's eyes"), View.Location, FVector(10.0, 20.0, S.EyeHeight));
	TestEqual(TEXT("yaw is the look yaw"), View.Rotation.Yaw, 45.0, 1e-3);
	TestEqual(TEXT("pitch is the look pitch"), View.Rotation.Pitch, -10.0, 1e-3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraBasisTest, "Jam.Camera.TopDownMovesInScreenAxes", JamTest::Flags)

bool FJamCameraBasisTest::RunTest(const FString& Parameters)
{
	FJamCameraSettings S = TestCamera();
	S.TopDownYaw = 30.f;
	TestEqual(TEXT("top-down ignores the look yaw"), JamCameraRules::MoveBasisYaw(EJamCameraMode::TopDown, 200.f, S), 30.f);
	TestEqual(TEXT("the shoulder camera moves along the look"), JamCameraRules::MoveBasisYaw(EJamCameraMode::Shoulder, 200.f, S), 200.f);
	TestEqual(TEXT("first person too"), JamCameraRules::MoveBasisYaw(EJamCameraMode::FirstPerson, 200.f, S), 200.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraPitchTest, "Jam.Camera.PitchIsClamped", JamTest::Flags)

bool FJamCameraPitchTest::RunTest(const FString& Parameters)
{
	const FJamCameraSettings S = TestCamera();
	TestEqual(TEXT("down"), JamCameraRules::ClampPitch(-200.f, S), S.MinPitch);
	TestEqual(TEXT("up"), JamCameraRules::ClampPitch(200.f, S), S.MaxPitch);
	TestEqual(TEXT("inside"), JamCameraRules::ClampPitch(10.f, S), 10.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraIniTest, "Jam.Camera.IniIsComplete", JamTest::Flags)

bool FJamCameraIniTest::RunTest(const FString& Parameters)
{
	TArray<FString> Missing;
	const FJamCameraSettings S = JamConfig::Load<FJamCameraSettings>(JamCameraRules::Section, &Missing);
	TestEqual(TEXT("every camera field has a line in DefaultGame.ini, missing: ") + FString::Join(Missing, TEXT(", ")), Missing.Num(), 0);
	TestTrue(TEXT("the pitch range is a range"), S.MinPitch < S.MaxPitch);
	return true;
}

#endif
