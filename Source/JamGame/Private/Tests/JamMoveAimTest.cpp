#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamAimRules.h"
#include "Sim/JamMoveRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-002 and T-003: how a fighter moves in the round arena and where it aims.
 * A bug here looks in game like "diagonal running is faster", "the bean leaves the arena" or "the hero aims at the
 * sky when the cursor is on him".
 *
 * Run: Automation RunTests Jam.Move / Jam.Aim
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveDiagonalTest, "Jam.Move.DiagonalIsNotFaster", JamTest::Flags)

bool FJamMoveDiagonalTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("straight"), JamMoveRules::Velocity(FVector2D(1.0, 0.0), 600.f).Size(), 600.0, 0.01);
	TestEqual(TEXT("diagonal"), JamMoveRules::Velocity(FVector2D(1.0, 1.0), 600.f).Size(), 600.0, 0.01);
	TestEqual(TEXT("a half-tilted stick is half speed"), JamMoveRules::Velocity(FVector2D(0.5, 0.0), 600.f).Size(), 300.0, 0.01);
	TestEqual(TEXT("no input, no speed"), JamMoveRules::Velocity(FVector2D::ZeroVector, 600.f).Size(), 0.0, 0.01);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveArenaTest, "Jam.Move.StaysInsideArena", JamTest::Flags)

bool FJamMoveArenaTest::RunTest(const FString& Parameters)
{
	const FVector2D Inside = JamMoveRules::ClampToArena(FVector2D(100.0, 200.0), 50.f, 1000.f);
	TestEqual(TEXT("a point inside is untouched"), Inside, FVector2D(100.0, 200.0));
	const FVector2D Clamped = JamMoveRules::ClampToArena(FVector2D(2000.0, 0.0), 50.f, 1000.f);
	TestEqual(TEXT("a body is kept whole inside the edge"), Clamped, FVector2D(950.0, 0.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveSlideTest, "Jam.Move.SlidesAlongTheEdge", JamTest::Flags)

bool FJamMoveSlideTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(B.ArenaRadius - B.HeroRadius, 0.0), FVector2D(-500.0, 0.0));
	FJamIntents Intents;
	// Pushing into the wall at 45 degrees: the outward part is lost, the part along the wall remains.
	Intents[EJamSide::Hero].Move = FVector2D(1.0, 1.0);
	JamTest::Run(F, Intents, B, 30);
	const FVector2D Pos = F.Fighters[EJamSide::Hero].Pos;
	TestTrue(TEXT("the hero moved along the edge"), Pos.Y > 100.0);
	TestTrue(TEXT("and stayed inside"), Pos.Size() <= B.ArenaRadius - B.HeroRadius + 0.01);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveBasisTest, "Jam.Move.InputFollowsCameraBasis", JamTest::Flags)

bool FJamMoveBasisTest::RunTest(const FString& Parameters)
{
	// Input: X is right, Y is forward. World: yaw 0 looks along +X, +Y is to the right of it.
	TestTrue(TEXT("forward at yaw 0 is +X"), JamMoveRules::WorldMove(FVector2D(0.0, 1.0), 0.f).Equals(FVector2D(1.0, 0.0), 1e-4));
	TestTrue(TEXT("right at yaw 0 is +Y"), JamMoveRules::WorldMove(FVector2D(1.0, 0.0), 0.f).Equals(FVector2D(0.0, 1.0), 1e-4));
	TestTrue(TEXT("forward at yaw 90 is +Y"), JamMoveRules::WorldMove(FVector2D(0.0, 1.0), 90.f).Equals(FVector2D(0.0, 1.0), 1e-4));
	TestTrue(TEXT("right at yaw 90 is -X"), JamMoveRules::WorldMove(FVector2D(1.0, 0.0), 90.f).Equals(FVector2D(-1.0, 0.0), 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveWalkTest, "Jam.Move.HeroWalksAtHisSpeed", JamTest::Flags)

bool FJamMoveWalkTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(0.0, -500.0), FVector2D(500.0, 500.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Move = FVector2D(1.0, 0.0);
	JamTest::Run(F, Intents, B, JamSim::StepsPerSecond);
	TestEqual(TEXT("one second of walking covers the speed"), F.Fighters[EJamSide::Hero].Pos.X, static_cast<double>(B.HeroSpeed), 1.0);
	TestEqual(TEXT("the boss without an intent stands"), F.Fighters[EJamSide::Boss].Pos, FVector2D(500.0, 500.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMoveBodiesTest, "Jam.Move.FightersDoNotOverlap", JamTest::Flags)

bool FJamMoveBodiesTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-300.0, 0.0), FVector2D(0.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Move = FVector2D(1.0, 0.0);
	JamTest::Run(F, Intents, B, 2 * JamSim::StepsPerSecond);
	const double Gap = FVector2D::Distance(F.Fighters[EJamSide::Hero].Pos, F.Fighters[EJamSide::Boss].Pos);
	TestTrue(TEXT("the hero stops at the boss's body"), Gap >= B.HeroRadius + B.BossRadius - 0.01);
	TestEqual(TEXT("the boss is not pushed"), F.Fighters[EJamSide::Boss].Pos, FVector2D(0.0, 0.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimRayTest, "Jam.Aim.CursorRayHitsThePlane", JamTest::Flags)

bool FJamAimRayTest::RunTest(const FString& Parameters)
{
	FJamAim Aim;
	// A camera 1000 above the point (300, 400) looking straight down, the fighter at the origin.
	const bool bHit = JamAimRules::FromRay(FVector(300.0, 400.0, 1000.0), FVector(0.0, 0.0, -1.0), 0.f, FVector2D::ZeroVector, Aim);
	TestTrue(TEXT("the ray hits the plane"), bHit);
	TestTrue(TEXT("the aim point is under the cursor"), Aim.Point.Equals(FVector2D(300.0, 400.0), 1e-3));
	TestTrue(TEXT("the direction is a unit vector to it"), Aim.Dir.Equals(FVector2D(0.6, 0.8), 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimParallelTest, "Jam.Aim.RayThatMissesThePlaneKeepsLastAim", JamTest::Flags)

bool FJamAimParallelTest::RunTest(const FString& Parameters)
{
	FJamAim Aim;
	Aim.Dir = FVector2D(0.0, 1.0);
	Aim.Point = FVector2D(0.0, 77.0);
	TestFalse(TEXT("a ray along the plane"),
		JamAimRules::FromRay(FVector(0.0, 0.0, 100.0), FVector(1.0, 0.0, 0.0), 0.f, FVector2D::ZeroVector, Aim));
	TestFalse(TEXT("a ray pointing away from it"),
		JamAimRules::FromRay(FVector(0.0, 0.0, 100.0), FVector(0.0, 0.0, 1.0), 0.f, FVector2D::ZeroVector, Aim));
	TestEqual(TEXT("the last aim stays"), Aim.Dir, FVector2D(0.0, 1.0));
	TestEqual(TEXT("with its point"), Aim.Point, FVector2D(0.0, 77.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimOnFighterTest, "Jam.Aim.CursorOnTheFighterKeepsLastDirection", JamTest::Flags)

bool FJamAimOnFighterTest::RunTest(const FString& Parameters)
{
	FJamAim Aim;
	Aim.Dir = FVector2D(0.0, -1.0);
	const bool bHit = JamAimRules::FromRay(FVector(50.0, 50.0, 500.0), FVector(0.0, 0.0, -1.0), 0.f, FVector2D(50.0, 50.0), Aim);
	TestTrue(TEXT("the point is known"), bHit);
	TestEqual(TEXT("the direction is not a zero vector: the last one stays"), Aim.Dir, FVector2D(0.0, -1.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimYawTest, "Jam.Aim.YawGivesDirection", JamTest::Flags)

bool FJamAimYawTest::RunTest(const FString& Parameters)
{
	const FJamAim Aim = JamAimRules::FromYaw(90.f, FVector2D(100.0, 0.0), 500.f);
	TestTrue(TEXT("yaw 90 aims along +Y"), Aim.Dir.Equals(FVector2D(0.0, 1.0), 1e-4));
	TestTrue(TEXT("the point is Reach ahead of the fighter"), Aim.Point.Equals(FVector2D(100.0, 500.0), 1e-2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimFacingTest, "Jam.Aim.FighterFacesTheAim", JamTest::Flags)

bool FJamAimFacingTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(500.0, 0.0));
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the hero turned to the aim"), F.Fighters[EJamSide::Hero].Facing, FVector2D(0.0, 1.0));
	return true;
}

/**
 * T-034: the hero model turns to the cursor on every rendered frame, from where he is drawn, not from the last step.
 * A bug here looks in game like "the hero turns late" or "spins when the cursor is on him".
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimShownFacingTest, "Jam.Aim.ShownHeroFacesTheCursor", JamTest::Flags)

bool FJamAimShownFacingTest::RunTest(const FString& Parameters)
{
	const FVector2D Keep(1.0, 0.0);
	TestTrue(TEXT("from where he is drawn to the cursor"),
		JamAimRules::FacingTo(FVector2D(100.0, 300.0), FVector2D(100.0, 100.0), Keep).Equals(FVector2D(0.0, 1.0), 1e-6));
	TestTrue(TEXT("the cursor behind him turns him round at once"),
		JamAimRules::FacingTo(FVector2D(-500.0, 0.0), FVector2D::ZeroVector, Keep).Equals(FVector2D(-1.0, 0.0), 1e-6));
	TestEqual(
		TEXT("the cursor on him keeps the last facing"), JamAimRules::FacingTo(FVector2D(0.3, 0.2), FVector2D::ZeroVector, Keep), Keep);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAimCameraMoveTest, "Jam.Aim.CursorPointFollowsTheCamera", JamTest::Flags)

bool FJamAimCameraMoveTest::RunTest(const FString& Parameters)
{
	// The cursor was read through last frame's camera; the camera then moved with the hero. Under a camera that only
	// slides, the floor point under a still cursor slides with it.
	const FVector2D Moved = JamAimRules::FollowCamera(FVector2D(300.0, 200.0), FVector(0.0, 0.0, 1500.0), FVector(12.0, -5.0, 1500.0));
	TestTrue(TEXT("the point slides with the camera"), Moved.Equals(FVector2D(312.0, 195.0), 1e-6));
	const FVector2D Still = JamAimRules::FollowCamera(FVector2D(300.0, 200.0), FVector(7.0, 7.0, 1500.0), FVector(7.0, 7.0, 1500.0));
	TestEqual(TEXT("a still camera leaves it"), Still, FVector2D(300.0, 200.0));
	return true;
}

#endif
