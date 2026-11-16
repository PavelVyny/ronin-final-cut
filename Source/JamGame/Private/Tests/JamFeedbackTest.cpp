#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Tests/JamTestHelpers.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamUltimateRules.h"
#include "View/JamFeedbackRules.h"

/**
 * T-029: what the picture tells the player about a blow: the camera shake, the spark and the flash of a hit, the
 * warning on an explosive shuriken and the closing parry ring.
 * A bug here looks in game like "the camera jumps when I swing", "the boss blinks white all fight long" or "his
 * explosive shuriken looks like any other".
 *
 * Run: Automation RunTests Jam.Feedback
 */
namespace
{
FJamFeedbackSettings TestFeedback()
{
	FJamFeedbackSettings S;
	S.SlamShake = 20.f;
	S.UltimateShake = 30.f;
	S.ExplosionShake = 10.f;
	S.HeroHurtShake = 8.f;
	S.ShakeSeconds = 0.4f;
	S.ShakeFrequency = 25.f;
	S.SparkPerDamage = 2.f;
	S.SparkMinRadius = 20.f;
	S.SparkMaxRadius = 80.f;
	S.BossFlashFullDamage = 50.f;
	S.BossFlashMin = 0.3f;
	S.ParryRingStart = 2.4f;
	S.ParryRingEnd = 1.3f;
	S.DamagePerBill = 20.f;
	S.BillsPerHitMin = 2;
	S.BillsPerHitMax = 12;
	S.BillsFinalBlow = 60;
	S.BillFlutterSeconds = 1.2f;
	S.BillFloorSeconds = 3.f;
	S.BillFadeSeconds = 0.5f;
	S.BillPopHeight = 90.f;
	S.BillSway = 25.f;
	S.BillFlutterHz = 2.f;
	return S;
}

FJamEvent EventOf(EJamEvent Type, EJamSide Side)
{
	FJamEvent Event;
	Event.Type = Type;
	Event.Side = Side;
	return Event;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackShakeKickTest, "Jam.Feedback.OnlyHeavyBlowsShakeTheCamera", JamTest::Flags)

bool FJamFeedbackShakeKickTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	TestEqual(TEXT("the slam shakes"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Slam, EJamSide::Boss), S), 20.f);
	TestEqual(TEXT("the hero's Ultimate shakes"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::UltimateSlash, EJamSide::Hero), S), 30.f);
	TestEqual(TEXT("an explosion shakes"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Explosion, EJamSide::Boss), S), 10.f);
	TestEqual(TEXT("the hero hurt shakes"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Hit, EJamSide::Hero), S), 8.f);
	TestEqual(TEXT("a hit on the boss does not"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Hit, EJamSide::Boss), S), 0.f);
	TestEqual(TEXT("a swing does not"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Swing, EJamSide::Hero), S), 0.f);
	TestEqual(TEXT("a dash does not"), JamFeedbackRules::ShakeKick(EventOf(EJamEvent::Dash, EJamSide::Hero), S), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackShakeFadesTest, "Jam.Feedback.ShakeDiesOut", JamTest::Flags)

bool FJamFeedbackShakeFadesTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	TestEqual(TEXT("full at the blow"), JamFeedbackRules::ShakeLeft(20.f, 0.f, S), 20.f);
	const float Half = JamFeedbackRules::ShakeLeft(20.f, 0.2f, S);
	TestTrue(TEXT("weaker halfway"), Half > 0.f && Half < 20.f);
	TestTrue(TEXT("weaker still later"), JamFeedbackRules::ShakeLeft(20.f, 0.3f, S) < Half);
	TestEqual(TEXT("gone at the end"), JamFeedbackRules::ShakeLeft(20.f, 0.4f, S), 0.f);
	TestEqual(TEXT("and after it"), JamFeedbackRules::ShakeLeft(20.f, 5.f, S), 0.f);

	float Largest = 0.f;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		const FVector2D Offset = JamFeedbackRules::ShakeOffset(15.f, Step * 0.013f, S);
		TestTrue(TEXT("never further than the amplitude"), Offset.Size() <= 15.f + KINDA_SMALL_NUMBER);
		Largest = FMath::Max(Largest, static_cast<float>(Offset.Size()));
	}
	TestTrue(TEXT("but it does move"), Largest > 5.f);
	TestTrue(TEXT("no amplitude, no offset"), JamFeedbackRules::ShakeOffset(0.f, 0.37f, S).IsNearlyZero());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackBlastRingTest, "Jam.Feedback.TheBossExplosiveShowsItsBlast", JamTest::Flags)

bool FJamFeedbackBlastRingTest::RunTest(const FString& Parameters)
{
	FJamProjectile Shuriken;
	Shuriken.Kind = EJamProjectile::Shuriken;
	Shuriken.Owner = EJamSide::Boss;
	TestFalse(TEXT("a plain one of his carries no ring"), JamFeedbackRules::ShowsBlastRing(Shuriken));
	Shuriken.bExplosive = true;
	TestTrue(TEXT("an explosive one of his does"), JamFeedbackRules::ShowsBlastRing(Shuriken));
	Shuriken.Owner = EJamSide::Hero;
	TestFalse(TEXT("the hero's own, or one he parried back, does not: it cannot hurt him"), JamFeedbackRules::ShowsBlastRing(Shuriken));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackHitTest, "Jam.Feedback.HitsShowTheirWeight", JamTest::Flags)

bool FJamFeedbackHitTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	TestEqual(TEXT("a small blow gets the smallest spark"), JamFeedbackRules::SparkRadius(2.f, S), 20.f);
	TestEqual(TEXT("a middle blow grows with the damage"), JamFeedbackRules::SparkRadius(25.f, S), 50.f);
	TestEqual(TEXT("a huge one stops at the largest"), JamFeedbackRules::SparkRadius(500.f, S), 80.f);

	TestEqual(TEXT("the hurt hero always flashes fully"), JamFeedbackRules::FlashWeight(EJamSide::Hero, 1.f, S), 1.f);
	TestEqual(TEXT("a heavy blow flashes the boss fully"), JamFeedbackRules::FlashWeight(EJamSide::Boss, 60.f, S), 1.f);
	TestEqual(TEXT("a light one only tints him"), JamFeedbackRules::FlashWeight(EJamSide::Boss, 1.f, S), 0.3f);
	const float Middle = JamFeedbackRules::FlashWeight(EJamSide::Boss, 25.f, S);
	TestTrue(TEXT("a middle one in between"), Middle > 0.3f && Middle < 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackParryRingTest, "Jam.Feedback.ParryRingClosesWithTheWindow", JamTest::Flags)

bool FJamFeedbackParryRingTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	TestEqual(TEXT("wide when it opens"), JamFeedbackRules::ParryRingScale(1.f, S), 2.4f);
	TestEqual(TEXT("tight when it shuts"), JamFeedbackRules::ParryRingScale(0.f, S), 1.3f);
	TestEqual(TEXT("halfway in between"), JamFeedbackRules::ParryRingScale(0.5f, S), 1.85f);
	TestEqual(TEXT("out of range is held to the ends"), JamFeedbackRules::ParryRingScale(3.f, S), 2.4f);
	return true;
}

/**
 * T-035: the effects cut takes the neon off the hero's blade, never the blade itself. The playtest read a swing with no
 * picture as a bug.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackSwingSeenTest, "Jam.Feedback.MeleeIsAlwaysSeen", JamTest::Flags)

bool FJamFeedbackSwingSeenTest::RunTest(const FString& Parameters)
{
	using JamFeedbackRules::LookOf;
	TestTrue(TEXT("the full game: a neon blade"), LookOf(EJamEvent::Swing, EJamSide::Hero, true) == EJamEffectLook::Neon);
	TestTrue(TEXT("after the effects cut: a bare grey blade"), LookOf(EJamEvent::Swing, EJamSide::Hero, false) == EJamEffectLook::Bare);
	TestTrue(TEXT("the Publisher keeps his neon"), LookOf(EJamEvent::Swing, EJamSide::Boss, false) == EJamEffectLook::Neon);
	TestTrue(TEXT("the hero's dash trail goes with the cut"), LookOf(EJamEvent::Dash, EJamSide::Hero, false) == EJamEffectLook::Hidden);
	TestTrue(
		TEXT("so does the spark of his hit on the Publisher"), LookOf(EJamEvent::Hit, EJamSide::Boss, false) == EJamEffectLook::Hidden);
	TestTrue(TEXT("the hero's own hurt spark stays"), LookOf(EJamEvent::Hit, EJamSide::Hero, false) == EJamEffectLook::Neon);
	TestTrue(TEXT("the slam stays"), LookOf(EJamEvent::Slam, EJamSide::Boss, false) == EJamEffectLook::Neon);
	return true;
}

/**
 * T-042: the Publisher bleeds paper dollars. A bug looks like "a hit on him shows no money", "the screen drowns in
 * bills" or "the last blow is as small as any other".
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackDollarCountTest, "Jam.Feedback.ThePublisherBleedsDollarsByTheDamage", JamTest::Flags)

bool FJamFeedbackDollarCountTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	FJamEvent Hit = EventOf(EJamEvent::Hit, EJamSide::Boss);
	Hit.Amount = 120.f;
	TestEqual(TEXT("one bill per 20 damage"), JamFeedbackRules::BillCount(Hit, false, S), 6);
	Hit.Amount = 14.f;
	TestEqual(TEXT("a light cut still bleeds a little"), JamFeedbackRules::BillCount(Hit, false, S), 2);
	Hit.Amount = 1000.f;
	TestEqual(TEXT("a huge hit stops at the most"), JamFeedbackRules::BillCount(Hit, false, S), 12);
	Hit.Amount = 150.f;
	TestEqual(TEXT("the final blow bursts"), JamFeedbackRules::BillCount(Hit, true, S), 60);
	TestEqual(
		TEXT("the hurt hero has no money to lose"), JamFeedbackRules::BillCount(EventOf(EJamEvent::Hit, EJamSide::Hero), false, S), 0);
	TestEqual(TEXT("a swing is no hit"), JamFeedbackRules::BillCount(EventOf(EJamEvent::Swing, EJamSide::Hero), false, S), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackBillFlightTest, "Jam.Feedback.BillsFlutterDownAndFade", JamTest::Flags)

bool FJamFeedbackBillFlightTest::RunTest(const FString& Parameters)
{
	const FJamFeedbackSettings S = TestFeedback();
	const FVector From(100.f, 50.f, 120.f);
	const FVector2D Kick(80.f, 0.f);
	const float Floor = 6.f;

	const FJamBillPose Start = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 0.f, S);
	TestTrue(TEXT("it starts at the hit point"), Start.Pos.Equals(From, 0.1f));
	TestEqual(TEXT("at full size"), Start.Scale, 1.f);

	const FJamBillPose Up = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 0.2f, S);
	TestTrue(TEXT("it pops up first"), Up.Pos.Z > From.Z);
	const FJamBillPose Mid = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 0.6f, S);
	TestTrue(TEXT("still in the air halfway"), Mid.Pos.Z > Floor);
	TestTrue(TEXT("it flutters: tipped in the air"), FMath::Abs(Mid.Roll) > 1.f || FMath::Abs(Up.Roll) > 1.f);

	const FJamBillPose Landed = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 1.2f, S);
	TestEqual(TEXT("on the floor when the flutter ends"), static_cast<float>(Landed.Pos.Z), Floor);
	TestEqual(TEXT("flat on it"), Landed.Roll, 0.f);
	TestTrue(TEXT("thrown the way the kick points"), Landed.Pos.X - From.X > 40.f);
	const FJamBillPose Lying = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 3.5f, S);
	TestTrue(TEXT("it lies still"), Lying.Pos.Equals(Landed.Pos, 0.1f));
	TestEqual(TEXT("and whole"), Lying.Scale, 1.f);

	const FJamBillPose Fading = JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 4.45f, S);
	TestTrue(TEXT("then it fades"), Fading.Scale > 0.f && Fading.Scale < 1.f);
	TestEqual(TEXT("gone at the end of its life"), JamFeedbackRules::BillPose(From, Kick, Floor, 0.3f, 4.7f, S).Scale, 0.f);
	TestEqual(TEXT("its life"), JamFeedbackRules::BillLife(S), 4.7f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackSpillTest, "Jam.Feedback.BillsSpillOutOfHisBody", JamTest::Flags)

bool FJamFeedbackSpillTest::RunTest(const FString& Parameters)
{
	// T-054: the author's playtest: the dollars appeared inside the Publisher. They leave his body and land outside it.
	const FJamFeedbackSettings S = TestFeedback();
	const FVector2D Centre(300.0, -200.0);
	const float Body = 90.f;
	for (int32 Degrees = 0; Degrees < 360; Degrees += 45)
	{
		for (const float Reach : { 0.f, 60.f, 170.f })
		{
			const FJamSpill Bill = JamFeedbackRules::Spill(Centre, Body, static_cast<float>(Degrees), Reach);
			if (!FMath::IsNearlyEqual(FVector2D::Distance(Bill.From, Centre), static_cast<double>(Body), 0.1))
			{
				AddError(FString::Printf(TEXT("%d degrees: a bill leaves the edge of his body"), Degrees));
				return false;
			}
			const FVector2D Land = Bill.From + Bill.Kick;
			if (!FMath::IsNearlyEqual(FVector2D::Distance(Land, Centre), static_cast<double>(Body + Reach), 0.1))
			{
				AddError(FString::Printf(TEXT("%d degrees, reach %.0f: it lands the reach past his edge"), Degrees, Reach));
				return false;
			}
		}
	}
	const FJamSpill East = JamFeedbackRules::Spill(Centre, Body, 0.f, 100.f);
	TestTrue(TEXT("the way the angle points"), East.Kick.X > 99.0 && FMath::Abs(East.Kick.Y) < 0.1);

	const FVector From(East.From.X, East.From.Y, 120.f);
	const FJamBillPose Top = JamFeedbackRules::BillPose(From, East.Kick, 2.f, 0.f, 0.5f * S.BillFlutterSeconds, S);
	TestTrue(TEXT("it flies an arc: above where it left at the top"), Top.Pos.Z > From.Z);
	return true;
}

/**
 * T-046: the author's playtest: "the Ultimate is hard to use, I could not see where it lands and when". A lane on the
 * floor shows its cut while it is ready, locks when it is pressed and fills up to the moment it lands.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackUltimateLaneTest, "Jam.Feedback.TheUltimateShowsWhereAndWhen", JamTest::Flags)

bool FJamFeedbackUltimateLaneTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(0.0, 700.0));
	F.Tick = 100;
	FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	Hero.Facing = FVector2D(1.0, 0.0);
	TestFalse(TEXT("an empty meter shows nothing"), JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 1.f, B).bShow);

	Hero.Ultimate.Meter = 1.f;
	const FJamUltimateLane Ready = JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 1.f, B);
	TestTrue(TEXT("full: the lane shows"), Ready.bShow);
	TestFalse(TEXT("not yet locked"), Ready.bArmed);
	TestTrue(TEXT("along the aim"), Ready.Dir.Equals(FVector2D(1.0, 0.0), 0.01));
	TestEqual(TEXT("as long as the cut"), Ready.Length, B.UltimateRange);
	TestEqual(TEXT("as wide"), Ready.HalfWidth, B.UltimateHalfWidth);
	TestFalse(TEXT("the boss beside it is not in it"), Ready.bOnTarget);
	Hero.Facing = FVector2D(0.0, 1.0);
	TestTrue(TEXT("the boss in it lights it up"), JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 1.f, B).bOnTarget);

	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	TestFalse(TEXT("not when the Publisher holds it"), JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 1.f, B).bShow);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Ultimate, EJamSide::Hero);

	JamUltimateRules::Press(F, EJamSide::Hero, B);
	Hero.Facing = FVector2D(-1.0, 0.0);
	const int32 WindUp = JamSim::Ticks(B.UltimateWindUp);
	F.Tick = 101;
	const FJamUltimateLane Pressed = JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 0.f, B);
	TestTrue(TEXT("pressed: locked"), Pressed.bArmed);
	TestTrue(TEXT("on the way it was pressed, whatever the aim does now"), Pressed.Dir.Equals(FVector2D(0.0, 1.0), 0.01));
	TestEqual(TEXT("empty at the press"), Pressed.Fill, 0.f, 0.01f);
	F.Tick = 101 + WindUp / 2;
	const float Half = JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 0.f, B).Fill;
	TestTrue(TEXT("filling during the wind-up"), Half > 0.3f && Half < 0.7f);
	F.Tick = 101 + WindUp;
	TestEqual(TEXT("full when it lands"), JamFeedbackRules::UltimateLane(F, EJamSide::Hero, 0.f, B).Fill, 1.f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFeedbackIniTest, "Jam.Feedback.IniIsComplete", JamTest::Flags)

bool FJamFeedbackIniTest::RunTest(const FString& Parameters)
{
	TArray<FString> Missing;
	const FJamFeedbackSettings S = JamConfig::Load<FJamFeedbackSettings>(JamFeedbackRules::Section, &Missing);
	TestEqual(TEXT("every feedback field has a line in DefaultGame.ini, missing: ") + FString::Join(Missing, TEXT(", ")), Missing.Num(), 0);
	TestTrue(TEXT("the Ultimate shakes harder than the slam"), S.UltimateShake > S.SlamShake);
	TestTrue(TEXT("a shake ends"), S.ShakeSeconds > 0.f);
	TestTrue(TEXT("the spark limits are a range"), S.SparkMinRadius <= S.SparkMaxRadius);
	TestTrue(TEXT("the parry ring closes"), S.ParryRingStart > S.ParryRingEnd);
	return true;
}

#endif
