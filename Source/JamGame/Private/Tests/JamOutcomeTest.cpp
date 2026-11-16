#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBossRules.h"
#include "Sim/JamFightOps.h"
#include "Sim/JamOutcomeRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-015, T-022: how the fight ends. The boss at zero is stunned, not dead: everything he took goes back, the meter is
 * full and the Ultimate is the final blow. The hero at zero is a loss.
 * A bug here looks in game like "the boss died while I was a bean and I never got my katana back", "the stunned boss
 * killed me" or "the win came without the last hit".
 *
 * Run: Automation RunTests Jam.Outcome
 */
namespace
{
/** A fight after the last steal: the hero is the bean, no threshold is left, the boss may act. */
FJamFight LastStretch(const FJamBalanceData& B, const FVector2D& HeroPos)
{
	FJamFight F = JamTest::Fight(B, HeroPos, FVector2D::ZeroVector);
	JamTest::Bean(F);
	F.Steal.Done = JamStealRules::ThresholdCount;
	F.Steal.Floor = 0.f;
	F.Boss.BusyUntilTick = 0;
	return F;
}

/** Takes the boss's last health and runs the step that notices it. */
void KnockDown(FJamFight& F, const FJamBalanceData& B)
{
	JamFightOps::Hurt(F, EJamSide::Boss, B.BossHealth, FVector2D::ZeroVector);
	JamFightRules::Step(F, FJamIntents(), B);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeTableTest, "Jam.Outcome.NextPhaseTable", JamTest::Flags)

bool FJamOutcomeTableTest::RunTest(const FString& Parameters)
{
	using JamOutcomeRules::Next;
	TestEqual(TEXT("above the threshold the fight goes on"), Next(EJamPhase::Fight, 50.f, 800.f, 750.f, true), EJamPhase::Fight);
	TestEqual(TEXT("at the threshold the card opens"), Next(EJamPhase::Fight, 50.f, 750.f, 750.f, true), EJamPhase::Steal);
	TestEqual(TEXT("with no steal left there is no card"), Next(EJamPhase::Fight, 50.f, 200.f, 0.f, true), EJamPhase::Fight);
	TestEqual(TEXT("the boss at zero is stunned"), Next(EJamPhase::Fight, 50.f, 0.f, 0.f, true), EJamPhase::Stunned);
	TestEqual(TEXT("the hero at zero has lost"), Next(EJamPhase::Fight, 0.f, 500.f, 250.f, false), EJamPhase::HeroDead);
	TestEqual(TEXT("both at zero on one step is a loss"), Next(EJamPhase::Fight, 0.f, 0.f, 0.f, true), EJamPhase::HeroDead);
	TestEqual(TEXT("a loss comes before a card too"), Next(EJamPhase::Fight, 0.f, 750.f, 750.f, true), EJamPhase::HeroDead);

	TestEqual(TEXT("an untouched stunned boss stays stunned"), Next(EJamPhase::Stunned, 50.f, 0.f, 0.f, false), EJamPhase::Stunned);
	TestEqual(TEXT("the final blow on him is the win"), Next(EJamPhase::Stunned, 50.f, 0.f, 0.f, true), EJamPhase::Won);

	TestEqual(TEXT("the card is closed by the choice, not here"), Next(EJamPhase::Steal, 50.f, 750.f, 750.f, false), EJamPhase::Steal);
	TestEqual(TEXT("a loss stays a loss"), Next(EJamPhase::HeroDead, 0.f, 0.f, 0.f, true), EJamPhase::HeroDead);
	TestEqual(TEXT("a win stays a win"), Next(EJamPhase::Won, 0.f, 0.f, 0.f, true), EJamPhase::Won);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeStunTest, "Jam.Outcome.BossAtZeroIsStunnedNotDead", JamTest::Flags)

bool FJamOutcomeStunTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = LastStretch(B, FVector2D(-600.0, 0.0));
	KnockDown(F, B);
	TestEqual(TEXT("the boss at zero is stunned"), F.Phase, EJamPhase::Stunned);
	TestEqual(TEXT("the step says so once"), JamTest::Count(F, EJamEvent::Stunned), 1);
	TestTrue(TEXT("he does nothing while stunned"), JamBossRules::IsBusy(F));

	FJamIntent Hero;
	int32 BossActs = 0;
	for (int32 i = 0; i < 300; ++i)
	{
		JamFightRules::Play(F, Hero, B);
		BossActs += F.Events.Num();
	}
	TestEqual(TEXT("five seconds later he still stands and waits"), F.Phase, EJamPhase::Stunned);
	TestEqual(TEXT("nothing happened in them"), BossActs, 0);
	TestTrue(TEXT("he did not walk"), F.Fighters[EJamSide::Boss].Pos.IsNearlyZero());
	TestEqual(TEXT("the clock runs: the view keeps moving"), F.Tick, 301);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeReturnTest, "Jam.Outcome.EverythingReturnsOnTheStun", JamTest::Flags)

bool FJamOutcomeReturnTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = LastStretch(B, FVector2D(-600.0, 0.0));
	F.Fighters[EJamSide::Hero].Health = 3.f;
	FJamProjectile Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Pos = Memo.PrevPos = FVector2D(0.0, 700.0);
	Memo.DieTick = 1000;
	F.Projectiles.Add(Memo);
	FJamHazard Circle;
	Circle.Pos = FVector2D(-600.0, 0.0);
	Circle.Radius = B.SlamRadius;
	Circle.Damage = B.SlamDamage;
	Circle.HitTick = 1;
	F.Hazards.Add(Circle);
	TestEqual(TEXT("before the stun the hero owns nothing"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero).Num(), 0);

	KnockDown(F, B);
	TestEqual(TEXT("everything is back with the hero"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero).Num(), JamSim::MechanicCount);
	TestEqual(TEXT("the boss keeps nothing"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Boss).Num(), 0);
	TestEqual(TEXT("his memos are gone"), F.Projectiles.Num(), 0);
	TestEqual(TEXT("his slam is called off"), F.Hazards.Num(), 0);

	JamTest::Run(F, FJamIntents(), B, 10);
	TestEqual(TEXT("the slam that was a step away never landed"), F.Fighters[EJamSide::Hero].Health, 3.f);
	TestFalse(TEXT("nothing hurts the hero any more"), JamFightOps::Hurt(F, EJamSide::Hero, 10.f, FVector2D::ZeroVector));
	TestEqual(TEXT("his health is untouched"), F.Fighters[EJamSide::Hero].Health, 3.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeMeterTest, "Jam.Outcome.TheStunFillsTheMeter", JamTest::Flags)

bool FJamOutcomeMeterTest::RunTest(const FString& Parameters)
{
	// T-022: the bean has dealt little; the final blow must not wait for a meter it never filled.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = LastStretch(B, FVector2D(-600.0, 0.0));
	TestEqual(TEXT("the bean's meter is empty"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 0.f);
	KnockDown(F, B);
	TestEqual(TEXT("the stun fills it"), F.Fighters[EJamSide::Hero].Ultimate.Meter, 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeFinalBlowTest, "Jam.Outcome.TheUltimateLandsTheFinalBlow", JamTest::Flags)

bool FJamOutcomeFinalBlowTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = LastStretch(B, FVector2D(-240.0, 0.0));
	KnockDown(F, B);

	// The katana is back in the hero's hands and it hits, but a stunned Publisher only falls to the Ultimate.
	FJamIntent Hero;
	Hero.Aim.Dir = FVector2D(1.0, 0.0);
	Hero.bPrimary = true;
	int32 Hits = 0;
	for (int32 i = 0; i <= JamSim::Ticks(B.KatanaWindUp); ++i)
	{
		JamFightRules::Play(F, Hero, B);
		Hits += JamTest::Count(F, EJamEvent::Hit);
	}
	TestEqual(TEXT("the button swings the katana again and it lands"), Hits, 1);
	TestEqual(TEXT("no ball flies"), F.Projectiles.Num(), 0);
	TestEqual(TEXT("a katana hit is not the end"), F.Phase, EJamPhase::Stunned);

	Hero.bPrimary = false;
	Hero.bUltimate = true;
	int32 Steps = 0;
	while (F.Phase == EJamPhase::Stunned && Steps < 120)
	{
		JamFightRules::Play(F, Hero, B);
		++Steps;
	}
	TestEqual(TEXT("the slash is the final blow"), F.Phase, EJamPhase::Won);
	TestEqual(TEXT("it lands by the Ultimate's wind-up"), Steps, JamSim::Ticks(B.UltimateWindUp) + 1);
	TestEqual(TEXT("the last step reports the slash"), JamTest::Count(F, EJamEvent::UltimateSlash), 1);

	const int32 Tick = F.Tick;
	JamFightRules::Play(F, Hero, B);
	TestEqual(TEXT("after the win time stands still"), F.Tick, Tick);
	TestEqual(TEXT("and nothing happens"), F.Events.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamOutcomeWholeFightTest, "Jam.Outcome.EveryCutComesBeforeTheStun", JamTest::Flags)

bool FJamOutcomeWholeFightTest::RunTest(const FString& Parameters)
{
	// However hard the hero hits, the fight walks through all three cards, then the stun, then the win.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-240.0, 0.0), FVector2D::ZeroVector);
	// The hero swings whatever he has at a boss who only stands; the test adds the big hits on top.
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(1.0, 0.0);
	Intents[EJamSide::Hero].bPrimary = true;
	Intents[EJamSide::Hero].bUltimate = true;
	TArray<EJamPhase> Seen;
	int32 Cards = 0;
	for (int32 i = 0; i < 2000 && F.Phase != EJamPhase::Won; ++i)
	{
		JamFightOps::Hurt(F, EJamSide::Boss, 10.f * B.BossHealth, FVector2D::ZeroVector);
		JamFightRules::Step(F, Intents, B);
		Cards += JamTest::Count(F, EJamEvent::StealOpened);
		if (Seen.Num() == 0 || Seen.Last() != F.Phase)
		{
			Seen.Add(F.Phase);
		}
	}
	TestEqual(TEXT("four cards opened"), Cards, 4);
	TArray<EJamPhase> Expected;
	for (int32 Card = 0; Card < 4; ++Card)
	{
		Expected.Append({ EJamPhase::Steal, EJamPhase::Fight });
	}
	Expected.Append({ EJamPhase::Stunned, EJamPhase::Won });
	TestTrue(TEXT("four times a card and the fight, then the stun and the win"), Seen == Expected);
	TestEqual(TEXT("the boss ended with everything taken back from him"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Boss).Num(), 0);
	return true;
}

#endif
