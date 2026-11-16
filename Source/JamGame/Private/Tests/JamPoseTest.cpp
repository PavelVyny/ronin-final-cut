#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "View/JamPoseRules.h"

/**
 * T-061: what the 3D fighters play. A bug here looks in game like "the blade lands before the sword moves", "the
 * Publisher staggers under every ball and never swings", or "the hero gets up after he died".
 *
 * Run: Automation RunTests Jam.Pose
 */
namespace
{
constexpr float HeavyHit = 25.f;

FJamEvent Event(EJamEvent Type, EJamSide Side, float Amount = 0.f, int32 ProjectileId = 0)
{
	FJamEvent E;
	E.Type = Type;
	E.Side = Side;
	E.Amount = Amount;
	E.ProjectileId = ProjectileId;
	return E;
}

FJamFight Fight()
{
	FJamFight F;
	F.Tick = 600;
	return F;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseStrikeTest, "Jam.Pose.StrikeCuesASlashTimedToItsHit", JamTest::Flags)

bool FJamPoseStrikeTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	F.Fighters[EJamSide::Hero].Katana.HitTick = F.Tick + 6;
	const FJamEvent Swing[] = { Event(EJamEvent::Swing, EJamSide::Hero, 1.f) };
	const FJamCue Cue = JamPoseRules::CueFor(F, EJamSide::Hero, Swing);
	TestEqual(TEXT("the first strike plays the first slash"), Cue.Clip, EJamClip::StrikeA);
	TestEqual(TEXT("its impact is due when the blade lands"), Cue.Lead, 0.1f, 1e-4f);
	TestFalse(TEXT("the legs keep walking"), Cue.bFullBody);

	const FJamEvent Second[] = { Event(EJamEvent::Swing, EJamSide::Hero, 2.f) };
	TestEqual(TEXT("the second strike swings the other way"), JamPoseRules::CueFor(F, EJamSide::Hero, Second).Clip, EJamClip::StrikeB);
	const FJamEvent Finisher[] = { Event(EJamEvent::Swing, EJamSide::Hero, 3.f) };
	TestEqual(TEXT("the finisher is the first slash again"), JamPoseRules::CueFor(F, EJamSide::Hero, Finisher).Clip, EJamClip::StrikeA);
	TestEqual(TEXT("the other fighter's strike moves nothing here"), JamPoseRules::CueFor(F, EJamSide::Boss, Swing).Clip, EJamClip::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseParryTest, "Jam.Pose.ParryRaisesTheGuardByTheWindow", JamTest::Flags)

bool FJamPoseParryTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	F.Fighters[EJamSide::Hero].Parry.ActiveFromTick = F.Tick + 3;
	const FJamEvent Parry[] = { Event(EJamEvent::ParryStarted, EJamSide::Hero) };
	const FJamCue Cue = JamPoseRules::CueFor(F, EJamSide::Hero, Parry);
	TestEqual(TEXT("the parry plays the guard"), Cue.Clip, EJamClip::Parry);
	TestEqual(TEXT("the guard is up when the window opens"), Cue.Lead, 0.05f, 1e-4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseSlamTest, "Jam.Pose.SlamLandsWithItsFirstCircle", JamTest::Flags)

bool FJamPoseSlamTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	for (const int32 Delay : { 78, 66, 90 })
	{
		FJamHazard Circle;
		Circle.Owner = EJamSide::Boss;
		Circle.StartTick = F.Tick;
		Circle.HitTick = F.Tick + Delay;
		F.Hazards.Add(Circle);
	}
	FJamHazard Old;
	Old.Owner = EJamSide::Boss;
	Old.StartTick = F.Tick - 30;
	Old.HitTick = F.Tick + 5;
	F.Hazards.Add(Old);
	const FJamCue Cue = JamPoseRules::CueFor(F, EJamSide::Boss, {});
	TestEqual(TEXT("new circles of his are a slam"), Cue.Clip, EJamClip::Slam);
	TestEqual(TEXT("the blow lands with the first new circle"), Cue.Lead, 1.1f, 1e-4f);
	TestTrue(TEXT("the slam takes the whole body"), Cue.bFullBody);
	TestEqual(TEXT("the hero does not slam"), JamPoseRules::CueFor(F, EJamSide::Hero, {}).Clip, EJamClip::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseThrowTest, "Jam.Pose.HisThrowsSwingTheHeavyBlow", JamTest::Flags)

bool FJamPoseThrowTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	FJamProjectile Memo;
	Memo.Id = 7;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Owner = EJamSide::Boss;
	F.Projectiles.Add(Memo);
	FJamProjectile Ball;
	Ball.Id = 8;
	Ball.Kind = EJamProjectile::Ball;
	Ball.Owner = EJamSide::Hero;
	F.Projectiles.Add(Ball);
	const FJamEvent His[] = { Event(EJamEvent::ProjectileSpawned, EJamSide::Boss, 0.f, 7) };
	const FJamCue Cue = JamPoseRules::CueFor(F, EJamSide::Boss, His);
	TestEqual(TEXT("a fan of memos is thrown with the heavy blow"), Cue.Clip, EJamClip::Heavy);
	TestEqual(TEXT("the throw already left the hand"), Cue.Lead, 0.f);
	const FJamEvent Hers[] = { Event(EJamEvent::ProjectileSpawned, EJamSide::Hero, 0.f, 8) };
	// T-065: what the hero throws leaves his left hand with a throw; the katana stays in the right.
	const FJamCue Throw = JamPoseRules::CueFor(F, EJamSide::Hero, Hers);
	TestEqual(TEXT("the hero's ball leaves with a throw"), Throw.Clip, EJamClip::Throw);
	TestEqual(TEXT("it already left the hand"), Throw.Lead, 0.f);
	TestFalse(TEXT("his legs keep running"), Throw.bFullBody);
	const FJamEvent Strike[] = { Event(EJamEvent::ProjectileSpawned, EJamSide::Hero, 0.f, 8),
		Event(EJamEvent::Swing, EJamSide::Hero, 1.f) };
	TestEqual(TEXT("a strike in the same step wins over a throw"), JamPoseRules::CueFor(F, EJamSide::Hero, Strike).Clip, EJamClip::StrikeA);

	FJamFighter& Boss = F.Fighters[EJamSide::Boss];
	Boss.Ultimate.SlashTick = F.Tick + 21;
	const FJamEvent Line[] = { Event(EJamEvent::UltimateStarted, EJamSide::Boss) };
	const FJamCue Ultimate = JamPoseRules::CueFor(F, EJamSide::Boss, Line);
	TestEqual(TEXT("the Ultimate is a heavy blow"), Ultimate.Clip, EJamClip::Heavy);
	TestEqual(TEXT("timed to the slash"), Ultimate.Lead, 0.35f, 1e-4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseHitTest, "Jam.Pose.HitsLeanWithoutCuttingAnAction", JamTest::Flags)

bool FJamPoseHitTest::RunTest(const FString& Parameters)
{
	// T-064: a hit is a lean over whatever plays, never an action of its own; only the stun takes the body.
	const FJamFight F = Fight();
	const FJamEvent Hero[] = { Event(EJamEvent::Hit, EJamSide::Hero, 8.f) };
	TestEqual(TEXT("a hit starts no action on the hero"), JamPoseRules::CueFor(F, EJamSide::Hero, Hero).Clip, EJamClip::None);
	TestTrue(TEXT("but any hit shakes him"), JamPoseRules::Shakes(F, EJamSide::Hero, Hero, HeavyHit));

	const FJamEvent Ball[] = { Event(EJamEvent::Hit, EJamSide::Boss, 5.f) };
	TestFalse(TEXT("a ball does not shake the Publisher"), JamPoseRules::Shakes(F, EJamSide::Boss, Ball, HeavyHit));
	const FJamEvent Finisher[] = { Event(EJamEvent::Hit, EJamSide::Boss, 30.f) };
	TestTrue(TEXT("a finisher does"), JamPoseRules::Shakes(F, EJamSide::Boss, Finisher, HeavyHit));
	TestEqual(TEXT("and starts no action either"), JamPoseRules::CueFor(F, EJamSide::Boss, Finisher).Clip, EJamClip::None);
	TestFalse(TEXT("the other fighter's hit shakes nobody here"), JamPoseRules::Shakes(F, EJamSide::Boss, Hero, HeavyHit));

	const FJamEvent Stun[] = { Event(EJamEvent::Hit, EJamSide::Boss, 5.f), Event(EJamEvent::Stunned, EJamSide::Boss) };
	const FJamCue Stunned = JamPoseRules::CueFor(F, EJamSide::Boss, Stun);
	TestEqual(TEXT("the stun staggers him"), Stunned.Clip, EJamClip::Hit);
	TestTrue(TEXT("with the whole body"), Stunned.bFullBody);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseLeanTest, "Jam.Pose.TheHitLeanSnapsInAndEases", JamTest::Flags)

bool FJamPoseLeanTest::RunTest(const FString& Parameters)
{
	constexpr float Rise = 0.05f;
	constexpr float Life = 0.35f;
	TestEqual(TEXT("nothing before the hit"), JamPoseRules::LeanWeight(-0.01f, Rise, Life), 0.f);
	TestEqual(TEXT("nothing at the hit"), JamPoseRules::LeanWeight(0.f, Rise, Life), 0.f);
	TestEqual(TEXT("whole at the top of the snap"), JamPoseRules::LeanWeight(Rise, Rise, Life), 1.f, 1e-4f);
	const float Mid = JamPoseRules::LeanWeight(0.2f, Rise, Life);
	TestTrue(TEXT("easing back afterwards"), Mid > 0.f && Mid < 1.f);
	TestTrue(TEXT("falling, not rising"), JamPoseRules::LeanWeight(0.25f, Rise, Life) < Mid);
	TestEqual(TEXT("gone at the end"), JamPoseRules::LeanWeight(Life, Rise, Life), 0.f, 1e-4f);
	TestEqual(TEXT("and stays gone"), JamPoseRules::LeanWeight(1.f, Rise, Life), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseGhostTest, "Jam.Pose.AfterimagesTrailTheHitAndFade", JamTest::Flags)

bool FJamPoseGhostTest::RunTest(const FString& Parameters)
{
	constexpr int32 Count = 4;
	constexpr float Gap = 0.04f;
	TestEqual(TEXT("the first afterimage is left at the hit"), JamPoseRules::GhostsBy(0.f, Count, Gap), 1);
	TestEqual(TEXT("one more every gap"), JamPoseRules::GhostsBy(Gap * 2.f + 0.001f, Count, Gap), 3);
	TestEqual(TEXT("never more than the count"), JamPoseRules::GhostsBy(1.f, Count, Gap), Count);
	TestEqual(TEXT("none before the hit"), JamPoseRules::GhostsBy(-0.1f, Count, Gap), 0);

	constexpr float Life = 0.3f;
	TestEqual(TEXT("a new afterimage is bright"), JamPoseRules::GhostAlpha(0.f, Life), 1.f);
	TestEqual(TEXT("half way it is half"), JamPoseRules::GhostAlpha(0.15f, Life), 0.5f, 1e-4f);
	TestEqual(TEXT("gone after its life"), JamPoseRules::GhostAlpha(0.31f, Life), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPosePriorityTest, "Jam.Pose.TheStrongestActionOfAStepWins", JamTest::Flags)

bool FJamPosePriorityTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	F.Fighters[EJamSide::Hero].Katana.HitTick = F.Tick + 6;
	const FJamEvent Both[] = { Event(EJamEvent::Hit, EJamSide::Hero, 8.f), Event(EJamEvent::Swing, EJamSide::Hero, 1.f) };
	TestEqual(
		TEXT("a strike in the step of a hit is still a strike"), JamPoseRules::CueFor(F, EJamSide::Hero, Both).Clip, EJamClip::StrikeA);

	TestFalse(TEXT("a hit does not cut a strike short"), JamPoseRules::Replaces(EJamClip::Hit, EJamClip::StrikeA));
	TestFalse(TEXT("nor the guard"), JamPoseRules::Replaces(EJamClip::Hit, EJamClip::Parry));
	TestTrue(TEXT("a new hit restarts the last one"), JamPoseRules::Replaces(EJamClip::Hit, EJamClip::Hit));
	TestTrue(TEXT("a strike cuts a hit short"), JamPoseRules::Replaces(EJamClip::StrikeB, EJamClip::Hit));
	TestTrue(TEXT("the next strike cuts the last one"), JamPoseRules::Replaces(EJamClip::StrikeB, EJamClip::StrikeA));
	TestTrue(TEXT("death cuts everything"), JamPoseRules::Replaces(EJamClip::Death, EJamClip::Slam));
	TestFalse(TEXT("nothing gets the dead up"), JamPoseRules::Replaces(EJamClip::StrikeA, EJamClip::Death));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseDownTest, "Jam.Pose.TheLoserLiesDown", JamTest::Flags)

bool FJamPoseDownTest::RunTest(const FString& Parameters)
{
	FJamFight F = Fight();
	TestFalse(TEXT("nobody is down in the fight"), JamPoseRules::IsDown(F, EJamSide::Hero) || JamPoseRules::IsDown(F, EJamSide::Boss));
	F.Phase = EJamPhase::HeroDead;
	TestTrue(TEXT("the dead hero is down"), JamPoseRules::IsDown(F, EJamSide::Hero));
	TestFalse(TEXT("the Publisher stands over him"), JamPoseRules::IsDown(F, EJamSide::Boss));
	F.Phase = EJamPhase::Won;
	TestTrue(TEXT("after the win the Publisher is down"), JamPoseRules::IsDown(F, EJamSide::Boss));
	TestFalse(TEXT("the winner stands"), JamPoseRules::IsDown(F, EJamSide::Hero));
	F.Phase = EJamPhase::Stunned;
	TestFalse(TEXT("stunned is not down yet"), JamPoseRules::IsDown(F, EJamSide::Boss));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseFitTest, "Jam.Pose.ClipsAreFittedToTheBlow", JamTest::Flags)

bool FJamPoseFitTest::RunTest(const FString& Parameters)
{
	const FJamPlayback Slam = JamPoseRules::Fit(1.85f, 1.1f, 0.75f, 2.f);
	TestEqual(TEXT("a long wind-up shows whole"), Slam.From, 0.f);
	TestEqual(TEXT("sped up to land on time"), Slam.Rate, 1.85f / 1.1f, 1e-4f);

	const FJamPlayback Slash = JamPoseRules::Fit(0.5f, 0.1f, 0.75f, 2.f);
	TestEqual(TEXT("a short one plays at the top rate"), Slash.Rate, 2.f);
	TestEqual(TEXT("and starts late enough to land on time"), Slash.From, 0.3f, 1e-4f);

	const FJamPlayback Now = JamPoseRules::Fit(0.5f, 0.f, 0.75f, 2.f);
	TestEqual(TEXT("no lead starts at the impact"), Now.From, 0.5f);
	TestEqual(TEXT("at the clip's own speed"), Now.Rate, 1.f);

	const FJamPlayback Slow = JamPoseRules::Fit(0.3f, 1.f, 0.75f, 2.f);
	TestEqual(TEXT("a lead longer than the clip slows it down to the floor"), Slow.Rate, 0.75f);
	TestEqual(TEXT("from the start"), Slow.From, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseWeightTest, "Jam.Pose.ActionsFadeInAndOut", JamTest::Flags)

bool FJamPoseWeightTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("nothing at the start"), JamPoseRules::Weight(0.f, 1.f, 0.1f), 0.f);
	TestEqual(TEXT("half way in"), JamPoseRules::Weight(0.05f, 1.f, 0.1f), 0.5f, 1e-4f);
	TestEqual(TEXT("whole in the middle"), JamPoseRules::Weight(0.5f, 0.5f, 0.1f), 1.f);
	TestEqual(TEXT("half way out"), JamPoseRules::Weight(1.f, 0.05f, 0.1f), 0.5f, 1e-4f);
	TestEqual(TEXT("gone at the end"), JamPoseRules::Weight(1.f, -0.2f, 0.1f), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseStrideTest, "Jam.Pose.MovingAgainstTheFacingBackpedals", JamTest::Flags)

bool FJamPoseStrideTest::RunTest(const FString& Parameters)
{
	// T-066: the hero faces the cursor; walking away from it plays the run backwards instead of a forward run on the spot.
	const FVector2D East(1.0, 0.0);
	TestEqual(TEXT("forward runs forward"), JamPoseRules::StrideSign(FVector2D(1.0, 0.1), East), 1.f);
	TestEqual(TEXT("backward runs backward"), JamPoseRules::StrideSign(FVector2D(-1.0, 0.2), East), -1.f);
	TestEqual(TEXT("sideways keeps running forward"), JamPoseRules::StrideSign(FVector2D(0.0, 1.0), East), 1.f);
	TestEqual(TEXT("standing still is forward"), JamPoseRules::StrideSign(FVector2D::ZeroVector, East), 1.f);
	return true;
}

namespace
{
constexpr float StrideTwist = 60.f;
constexpr float StrideBlend = 16.f;

float StrideWeight(const FJamStride& Stride, EJamStride Run)
{
	return Stride.Weights[static_cast<int32>(Run)];
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseMoveAngleTest, "Jam.Pose.MoveAngleIsToTheRightOfTheFacing", JamTest::Flags)

bool FJamPoseMoveAngleTest::RunTest(const FString& Parameters)
{
	// T-078: the fight's +Y is the right of a body facing +X (Unreal's yaw), so a step to +Y is a step to the right.
	const FVector2D East(1.0, 0.0);
	TestNearlyEqual(TEXT("ahead"), JamPoseRules::MoveAngle(FVector2D(3.0, 0.0), East), 0.f, 0.01f);
	TestNearlyEqual(TEXT("right"), JamPoseRules::MoveAngle(FVector2D(0.0, 2.0), East), 90.f, 0.01f);
	TestNearlyEqual(TEXT("left"), JamPoseRules::MoveAngle(FVector2D(0.0, -2.0), East), -90.f, 0.01f);
	TestNearlyEqual(TEXT("back"), JamPoseRules::MoveAngle(FVector2D(-1.0, 0.0), East), 180.f, 0.01f);
	TestNearlyEqual(TEXT("facing north, moving east is to the right"), JamPoseRules::MoveAngle(East, FVector2D(0.0, -1.0)), 90.f, 0.01f);
	TestEqual(TEXT("standing still"), JamPoseRules::MoveAngle(FVector2D::ZeroVector, East), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseStrafeTest, "Jam.Pose.StrafingRunsTheDiagonalAndTwistsTheHips", JamTest::Flags)

bool FJamPoseStrafeTest::RunTest(const FString& Parameters)
{
	// T-078: the author, Sun 15 Nov: "the feet should step where he actually moves". Ahead is the plain run; a strafe
	// is the forward diagonal run with the hips turned the rest of the way; a backpedal is the run played backwards.
	const FJamStride Ahead = JamPoseRules::StrideFor(0.f, true, StrideTwist, StrideBlend);
	TestEqual(TEXT("ahead: the run"), StrideWeight(Ahead, EJamStride::Forward), 1.f);
	TestEqual(TEXT("ahead: no twist"), Ahead.Twist, 0.f);
	const FJamStride Right = JamPoseRules::StrideFor(90.f, true, StrideTwist, StrideBlend);
	TestEqual(TEXT("right: the forward-right run"), StrideWeight(Right, EJamStride::ForwardRight), 1.f);
	TestNearlyEqual(TEXT("right: the hips turn 45 more to the right"), Right.Twist, 45.f, 0.01f);
	const FJamStride Left = JamPoseRules::StrideFor(-90.f, true, StrideTwist, StrideBlend);
	TestEqual(TEXT("left: the forward-left run"), StrideWeight(Left, EJamStride::ForwardLeft), 1.f);
	TestNearlyEqual(TEXT("left: the hips turn 45 to the left"), Left.Twist, -45.f, 0.01f);
	// T-080 moved the back diagonals' edge to 135: they run between 105 and 135, the hips turned back to the move.
	const FJamStride BackRight = JamPoseRules::StrideFor(120.f, true, StrideTwist, StrideBlend);
	TestEqual(TEXT("back right: its own run"), StrideWeight(BackRight, EJamStride::BackRight), 1.f);
	TestNearlyEqual(TEXT("back right: the hips turn 15 back toward the move"), BackRight.Twist, -15.f, 0.01f);
	const FJamStride Back = JamPoseRules::StrideFor(-172.f, true, StrideTwist, StrideBlend);
	TestEqual(TEXT("back: the run backwards"), StrideWeight(Back, EJamStride::Back), 1.f);
	TestNearlyEqual(TEXT("back: the hips turn 8 toward the move"), Back.Twist, 8.f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseBackpedalTest, "Jam.Pose.ABackpedalPlaysTheRunBackwards", JamTest::Flags)

bool FJamPoseBackpedalTest::RunTest(const FString& Parameters)
{
	// T-080, the author on session-5n's afternoon branch: the back diagonals at full speed are quick, skittery steps;
	// backpedalling uses the plain run played backwards. Everything within 45 degrees of straight back is a backpedal.
	for (const float Angle : { 150.f, -150.f, 165.f, 180.f })
	{
		const FJamStride Stride = JamPoseRules::StrideFor(Angle, true, StrideTwist, StrideBlend);
		TestEqual(FString::Printf(TEXT("%.0f: the run backwards"), Angle), StrideWeight(Stride, EJamStride::Back), 1.f);
		TestTrue(FString::Printf(TEXT("%.0f: the hips turn at most 30"), Angle), FMath::Abs(Stride.Twist) <= 30.01f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseStrideSmoothTest, "Jam.Pose.StrideNeverJumpsAndTwistsAtMostTheLimit", JamTest::Flags)

bool FJamPoseStrideSmoothTest::RunTest(const FString& Parameters)
{
	// T-078: the move angle sweeps all the way round as the hero circles; the legs must cross between the runs, not pop.
	// Without the diagonals the cross is between the run and the same run backwards: the twist swings faster there.
	for (const bool bDiagonals : { true, false })
	{
		const float MaxTwistStep = bDiagonals ? 6.f : 12.f;
		FJamStride Last = JamPoseRules::StrideFor(-180.f, bDiagonals, StrideTwist, StrideBlend);
		for (int32 Degrees = -179; Degrees <= 180; ++Degrees)
		{
			const FJamStride Stride = JamPoseRules::StrideFor(static_cast<float>(Degrees), bDiagonals, StrideTwist, StrideBlend);
			float Sum = 0.f;
			float Step = 0.f;
			for (int32 Run = 0; Run < static_cast<int32>(EJamStride::Num); ++Run)
			{
				Sum += Stride.Weights[Run];
				Step = FMath::Max(Step, FMath::Abs(Stride.Weights[Run] - Last.Weights[Run]));
			}
			const FString At = FString::Printf(TEXT("%d degrees, diagonals %d"), Degrees, bDiagonals ? 1 : 0);
			TestNearlyEqual(*(At + TEXT(": the weights sum to 1")), Sum, 1.f, 0.001f);
			TestTrue(*(At + TEXT(": no weight jumps")), Step <= 1.f / StrideBlend + 0.001f);
			TestTrue(*(At + TEXT(": the twist stays in the limit")), FMath::Abs(Stride.Twist) <= StrideTwist + 0.001f);
			TestTrue(*(At + TEXT(": the twist never jumps")), FMath::Abs(Stride.Twist - Last.Twist) <= MaxTwistStep);
			Last = Stride;
		}
	}
	// Without the diagonals (the Publisher) a strafe is the forward run turned as far as the hips go.
	const FJamStride Side = JamPoseRules::StrideFor(90.f, false, StrideTwist, StrideBlend);
	TestEqual(TEXT("no diagonals, right: the run"), StrideWeight(Side, EJamStride::Forward), 1.f);
	TestNearlyEqual(TEXT("no diagonals, right: the hips turn to the limit"), Side.Twist, StrideTwist, 0.01f);
	TestEqual(TEXT("no diagonals, back: the run backwards"),
		StrideWeight(JamPoseRules::StrideFor(160.f, false, StrideTwist, StrideBlend), EJamStride::Back), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseStrideRateTest, "Jam.Pose.TheRunKeepsUpWithTheGround", JamTest::Flags)

bool FJamPoseStrideRateTest::RunTest(const FString& Parameters)
{
	// T-078: stride warping by play rate: a run whose own speed is half the ground speed plays twice as fast.
	TestNearlyEqual(TEXT("twice as fast"), JamPoseRules::StrideRate(400.f, 200.f, 3.f), 2.f, 0.001f);
	TestNearlyEqual(TEXT("at most the cap"), JamPoseRules::StrideRate(900.f, 100.f, 3.f), 3.f, 0.001f);
	TestEqual(TEXT("standing"), JamPoseRules::StrideRate(0.f, 200.f, 3.f), 0.f);
	TestEqual(TEXT("a clip without speed does not play"), JamPoseRules::StrideRate(300.f, 0.f, 3.f), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseTurnLeanTest, "Jam.Pose.TheBodyLeansIntoTheTurn", JamTest::Flags)

bool FJamPoseTurnLeanTest::RunTest(const FString& Parameters)
{
	// T-078: a small lean into a turn, never more than the cap however fast the cursor swings.
	TestEqual(TEXT("no turn, no lean"), JamPoseRules::TurnLean(0.f, 360.f, 8.f), 0.f);
	TestNearlyEqual(TEXT("half the rate, half the lean"), JamPoseRules::TurnLean(180.f, 360.f, 8.f), 4.f, 0.001f);
	TestNearlyEqual(TEXT("turning left leans left"), JamPoseRules::TurnLean(-180.f, 360.f, 8.f), -4.f, 0.001f);
	TestNearlyEqual(TEXT("a snap leans the cap"), JamPoseRules::TurnLean(5000.f, 360.f, 8.f), 8.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseWarpTest, "Jam.Pose.AThrowTakesTheChestFromTheWarp", JamTest::Flags)

bool FJamPoseWarpTest::RunTest(const FString& Parameters)
{
	// T-084, the author: a shuriken thrown while running backwards made the upper body vanish or twist away.
	TestEqual(TEXT("no action: the whole warp"), JamPoseRules::WarpWeight(0.f, false), 1.f);
	TestEqual(TEXT("a throw in full: no warp on top of it"), JamPoseRules::WarpWeight(1.f, false), 0.f);
	TestEqual(TEXT("half way in: half"), JamPoseRules::WarpWeight(0.5f, false), 0.5f, 1e-4f);
	TestEqual(TEXT("a full-body action: none"), JamPoseRules::WarpWeight(0.3f, true), 0.f);
	TestEqual(TEXT("an overweight action reads as full"), JamPoseRules::WarpWeight(1.4f, false), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPoseLoopTest, "Jam.Pose.TheIdleLoopsForever", JamTest::Flags)

bool FJamPoseLoopTest::RunTest(const FString& Parameters)
{
	// T-094: on the menu both fighters froze on the idle's last frame after its first loop.
	TestEqual(TEXT("inside the first loop"), JamPoseRules::LoopTime(1.5f, 4.f), 1.5f);
	TestTrue(TEXT("the second loop starts again"), FMath::IsNearlyEqual(JamPoseRules::LoopTime(5.f, 4.f), 1.f, 1e-4f));
	TestTrue(TEXT("an hour in it still moves"), JamPoseRules::LoopTime(3601.f, 4.f) < 4.f);
	TestTrue(TEXT("and differs a moment later"),
		!FMath::IsNearlyEqual(JamPoseRules::LoopTime(3601.f, 4.f), JamPoseRules::LoopTime(3601.5f, 4.f), 1e-3f));
	TestEqual(TEXT("a clip without length stays at its start"), JamPoseRules::LoopTime(7.f, 0.f), 0.f);
	TestTrue(TEXT("never negative"), JamPoseRules::LoopTime(-1.f, 4.f) >= 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
