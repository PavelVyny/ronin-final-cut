#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamFightRules.h"
#include "Sim/JamParryRules.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamTeachRules.h"

/**
 * T-036: the game teaches Parry before the Publisher takes it. A shot about to land while Q is ready marks the shot and
 * puts "Q PARRY" over the hero, until he has sent three back.
 * A bug here looks in game like "I never knew I could deflect" or "the hint nags me for the whole fight".
 *
 * Run: Automation RunTests Jam.Teach
 */
namespace
{
FJamTeachSettings TestTeach()
{
	FJamTeachSettings S;
	S.ParryPromptLead = 0.5f;
	S.ParryLessons = 3;
	return S;
}

/** A shuriken of the Publisher's at From flying at Speed cm/s along Dir. */
FJamProjectile ShotAt(int32 Id, const FVector2D& From, const FVector2D& Dir, float Speed)
{
	FJamProjectile Shot;
	Shot.Id = Id;
	Shot.Kind = EJamProjectile::Shuriken;
	Shot.Owner = EJamSide::Boss;
	Shot.Pos = From;
	Shot.PrevPos = From;
	Shot.Vel = Dir * Speed;
	Shot.Radius = 20.f;
	Shot.DieTick = 1000;
	return Shot;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTeachParryPromptTest, "Jam.Teach.ParryPromptMarksTheShotAboutToLand", JamTest::Flags)

bool FJamTeachParryPromptTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamTeachSettings S = TestTeach();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1500.0, 0.0));
	const FJamLessons Fresh;
	TestEqual(TEXT("nothing in the air, no prompt"), JamTeachRules::ParryPromptFor(F, Fresh, S), INDEX_NONE);

	// 1000 cm/s: 400 cm away lands in 0.4 s, inside the lead; 900 cm away in 0.9 s, outside it.
	F.Projectiles.Add(ShotAt(7, FVector2D(900.0, 0.0), FVector2D(-1.0, 0.0), 1000.f));
	TestEqual(TEXT("a shot still far off is no prompt yet"), JamTeachRules::ParryPromptFor(F, Fresh, S), INDEX_NONE);
	F.Projectiles.Add(ShotAt(8, FVector2D(400.0, 0.0), FVector2D(-1.0, 0.0), 1000.f));
	TestEqual(TEXT("the shot about to land is marked"), JamTeachRules::ParryPromptFor(F, Fresh, S), 8);

	FJamFight Wide = F;
	Wide.Projectiles[1].Vel = FVector2D(0.0, 1000.0);
	TestEqual(TEXT("a shot flying past is no prompt"), JamTeachRules::ParryPromptFor(Wide, Fresh, S), INDEX_NONE);

	FJamFight Own = F;
	Own.Projectiles[1].Owner = EJamSide::Hero;
	TestEqual(TEXT("the hero's own shot is no prompt"), JamTeachRules::ParryPromptFor(Own, Fresh, S), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTeachParryOnlyWhenItCanTest, "Jam.Teach.ParryPromptOnlyWhenQWorks", JamTest::Flags)

bool FJamTeachParryOnlyWhenItCanTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamTeachSettings S = TestTeach();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1500.0, 0.0));
	F.Projectiles.Add(ShotAt(8, FVector2D(400.0, 0.0), FVector2D(-1.0, 0.0), 1000.f));
	const FJamLessons Fresh;
	TestEqual(TEXT("Q ready: the prompt"), JamTeachRules::ParryPromptFor(F, Fresh, S), 8);

	FJamFight Cooling = F;
	Cooling.Fighters[EJamSide::Hero].Parry = JamParryRules::Start(Cooling.Tick, B);
	TestEqual(TEXT("the window already open or cooling: no prompt"), JamTeachRules::ParryPromptFor(Cooling, Fresh, S), INDEX_NONE);

	FJamFight Stolen = F;
	Stolen.Ledger.Owner[static_cast<int32>(EJamMechanic::Parry)] = EJamSide::Boss;
	TestEqual(TEXT("Parry stolen: no prompt"), JamTeachRules::ParryPromptFor(Stolen, Fresh, S), INDEX_NONE);

	FJamFight Card = F;
	Card.Phase = EJamPhase::Steal;
	TestEqual(TEXT("the card is up: no prompt"), JamTeachRules::ParryPromptFor(Card, Fresh, S), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTeachParryLessonsTest, "Jam.Teach.ThreeDeflectsEndTheParryLessons", JamTest::Flags)

bool FJamTeachParryLessonsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamTeachSettings S = TestTeach();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1500.0, 0.0));
	F.Projectiles.Add(ShotAt(8, FVector2D(400.0, 0.0), FVector2D(-1.0, 0.0), 1000.f));

	FJamEvent HeroDeflect;
	HeroDeflect.Type = EJamEvent::Deflected;
	HeroDeflect.Side = EJamSide::Hero;
	FJamEvent BossDeflect = HeroDeflect;
	BossDeflect.Side = EJamSide::Boss;

	FJamLessons Lessons;
	JamTeachRules::Learn(Lessons, { HeroDeflect, BossDeflect, HeroDeflect });
	TestEqual(TEXT("only the hero's deflects count"), Lessons.Deflects, 2);
	TestEqual(TEXT("two lessons in, the prompt still shows"), JamTeachRules::ParryPromptFor(F, Lessons, S), 8);
	JamTeachRules::Learn(Lessons, { HeroDeflect });
	TestEqual(TEXT("the third deflect ends it"), JamTeachRules::ParryPromptFor(F, Lessons, S), INDEX_NONE);
	return true;
}

/** T-037: a full Ultimate is called out until the hero has used it; the call over his head only the first time. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTeachUltimateCallTest, "Jam.Teach.AFullUltimateIsCalledOut", JamTest::Flags)

bool FJamTeachUltimateCallTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1500.0, 0.0));
	FJamLessons Fresh;
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 0.99f;
	TestTrue(TEXT("not full: no call"), JamTeachRules::UltimateCallFor(F, Fresh) == EJamUltimateCall::None);
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 1.f;
	TestTrue(TEXT("full, never used: the banner and the call over the hero"),
		JamTeachRules::UltimateCallFor(F, Fresh) == EJamUltimateCall::BannerAndPrompt);

	FJamEvent Slash;
	Slash.Type = EJamEvent::UltimateSlash;
	Slash.Side = EJamSide::Hero;
	FJamLessons Used;
	JamTeachRules::Learn(Used, { Slash });
	TestEqual(TEXT("the hero's slash is learned"), Used.Ultimates, 1);
	TestTrue(TEXT("used once: the banner alone"), JamTeachRules::UltimateCallFor(F, Used) == EJamUltimateCall::Banner);

	FJamFight Stolen = F;
	Stolen.Ledger.Owner[static_cast<int32>(EJamMechanic::Ultimate)] = EJamSide::Boss;
	TestTrue(TEXT("stolen: no call"), JamTeachRules::UltimateCallFor(Stolen, Fresh) == EJamUltimateCall::None);
	FJamFight Winding = F;
	Winding.Fighters[EJamSide::Hero].Ultimate.bPending = true;
	TestTrue(TEXT("already winding up: no call"), JamTeachRules::UltimateCallFor(Winding, Fresh) == EJamUltimateCall::None);
	FJamFight Stunned = F;
	Stunned.Phase = EJamPhase::Stunned;
	TestTrue(
		TEXT("the stun is the moment for it: the call stays"), JamTeachRules::UltimateCallFor(Stunned, Used) == EJamUltimateCall::Banner);
	FJamFight Card = F;
	Card.Phase = EJamPhase::Steal;
	TestTrue(TEXT("the card is up: no call"), JamTeachRules::UltimateCallFor(Card, Fresh) == EJamUltimateCall::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTeachIniTest, "Jam.Teach.IniIsComplete", JamTest::Flags)

bool FJamTeachIniTest::RunTest(const FString& Parameters)
{
	const FJamTeachSettings S = JamConfig::Load<FJamTeachSettings>(JamTeachRules::Section);
	TestTrue(TEXT("ParryPromptLead"), S.ParryPromptLead > 0.f);
	TestTrue(TEXT("ParryLessons"), S.ParryLessons > 0);
	return true;
}

#endif
