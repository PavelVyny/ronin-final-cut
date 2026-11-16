#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamHealthRules.h"
#include "Sim/JamStealRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-012, T-013: the Publisher's cuts. His health stops at each threshold, he grabs two of the hero's mechanics, the
 * player gives one away on a card while the fight stands still, and the last threshold takes whatever is left.
 * A bug here looks in game like "one big hit skipped a steal", "the card offered the same thing twice", "he took the
 * one I kept", "I died while I was reading the card" or "the key I held for walking picked for me".
 *
 * Run: Automation RunTests Jam.Steal
 */
namespace
{
/** True when the hero still owns the mechanic. */
bool HeroHas(const FJamFight& F, EJamMechanic Mechanic)
{
	return F.Ledger.Has(EJamSide::Hero, Mechanic);
}

/** Steps of the open card with the same answer held. */
void AnswerFor(FJamFight& F, const FJamBalanceData& B, int32 Choice, int32 Steps)
{
	FJamIntents Intents;
	Intents[EJamSide::Hero].Choice = Choice;
	JamTest::Run(F, Intents, B, Steps);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealClampTest, "Jam.Steal.HealthStopsAtTheThreshold", JamTest::Flags)

bool FJamStealClampTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	TestEqual(TEXT("the first steal waits at three quarters"), JamStealRules::ThresholdHealth(0, B), 750.f);
	TestEqual(TEXT("the second at a half"), JamStealRules::ThresholdHealth(1, B), 500.f);
	TestEqual(TEXT("the third at a quarter"), JamStealRules::ThresholdHealth(2, B), 250.f);
	TestEqual(TEXT("the final note at a tenth"), JamStealRules::ThresholdHealth(3, B), 100.f);
	TestEqual(TEXT("after the final note nothing holds the health"), JamStealRules::ThresholdHealth(4, B), 0.f);
	TestEqual(TEXT("health above the floor is left alone"), JamStealRules::ClampToThreshold(800.f, 750.f), 800.f);
	TestEqual(TEXT("health below the floor stops at it"), JamStealRules::ClampToThreshold(640.f, 750.f), 750.f);

	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	TestEqual(TEXT("a fresh fight holds the boss at the first threshold"), F.Steal.Floor, 750.f);
	F.Fighters[EJamSide::Boss].Health = 760.f;
	JamFightOps::Hurt(F, EJamSide::Boss, 100.f, FVector2D::ZeroVector);
	TestEqual(TEXT("a hit across the threshold stops at it"), F.Fighters[EJamSide::Boss].Health, 750.f);
	JamFightOps::Hurt(F, EJamSide::Hero, 30.f, FVector2D::ZeroVector);
	TestEqual(TEXT("the floor is the boss's, the hero takes the whole hit"), F.Fighters[EJamSide::Hero].Health, 70.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealBigHitTest, "Jam.Steal.OneBigHitTriggersOnlyOneSteal", JamTest::Flags)

bool FJamStealBigHitTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamFightOps::Hurt(F, EJamSide::Boss, 900.f, FVector2D::ZeroVector);
	TestEqual(TEXT("a hit of nine tenths of the bar stops at the first threshold"), F.Fighters[EJamSide::Boss].Health, 750.f);
	JamFightRules::Step(F, FJamIntents(), B);
	TestEqual(TEXT("the step ends with the card up"), F.Phase, EJamPhase::Steal);
	TestEqual(TEXT("the card is announced once"), JamTest::Count(F, EJamEvent::StealOpened), 1);
	TestEqual(TEXT("two mechanics are on the table"), F.Steal.Offer.Num(), 2);

	AnswerFor(F, B, 0, JamSim::Ticks(B.StealChoiceLock));
	TestEqual(TEXT("the answer settles it"), F.Phase, EJamPhase::Fight);
	TestEqual(TEXT("one steal is made"), F.Steal.Done, 1);
	TestEqual(TEXT("the health did not move during it"), F.Fighters[EJamSide::Boss].Health, 750.f);
	TestEqual(TEXT("the next steal waits at a half"), F.Steal.Floor, 500.f);
	JamFightRules::Step(F, FJamIntents(), B);
	TestEqual(TEXT("the fight goes on: the same threshold does not fire twice"), F.Phase, EJamPhase::Fight);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealOfferTest, "Jam.Steal.OffersTwoDifferentHeroMechanics", JamTest::Flags)

bool FJamStealOfferTest::RunTest(const FString& Parameters)
{
	FJamLedger Ledger;
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		FRandomStream Stream(Seed);
		const TArray<EJamMechanic> Offer = JamStealRules::Offer(Ledger, false, Stream);
		if (Offer.Num() != 2 || Offer[0] == Offer[1])
		{
			AddError(FString::Printf(TEXT("seed %d: the offer must be two different mechanics"), Seed));
			return false;
		}
	}

	JamLoadoutRules::Give(Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		FRandomStream Stream(Seed);
		const TArray<EJamMechanic> Offer = JamStealRules::Offer(Ledger, false, Stream);
		const bool bOnlyHis = Offer.Num() == 2 && !Offer.Contains(EJamMechanic::Shuriken) && Offer[0] != Offer[1];
		if (!bOnlyHis)
		{
			AddError(FString::Printf(TEXT("seed %d: what the boss already has is never offered again"), Seed));
			return false;
		}
	}

	JamLoadoutRules::Give(Ledger, EJamMechanic::Katana, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Parry, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	FRandomStream Stream(7);
	const TArray<EJamMechanic> Last = JamStealRules::Offer(Ledger, false, Stream);
	TestEqual(TEXT("a hero with one mechanic left is offered that one"), Last.Num(), 1);
	TestTrue(TEXT("and it is the dash"), Last.Num() == 1 && Last[0] == EJamMechanic::Dash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealKatanaTest, "Jam.Steal.TheKatanaIsNeverOnAChoiceCard", JamTest::Flags)

bool FJamStealKatanaTest::RunTest(const FString& Parameters)
{
	// T-051: with the Katana gone the hero chips at the Publisher with shurikens or nothing at all, so the choice cards
	// deal from the other mechanics and the Katana is the last one left for the final note.
	const FJamBalanceData B = JamTest::Balance();
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
		F.Stream.Initialize(Seed);
		for (int32 Steal = 0; Steal < JamStealRules::ThresholdCount - 1; ++Steal)
		{
			JamTest::OpenSteal(F, B);
			if (F.Steal.Offer.Num() != 2 || F.Steal.Offer.Contains(EJamMechanic::Katana))
			{
				AddError(FString::Printf(TEXT("seed %d, card %d: a choice card is two mechanics, never the Katana"), Seed, Steal));
				return false;
			}
			JamStealRules::Resolve(F, Seed % 2, B);
		}
		const TArray<EJamMechanic> Left = JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero);
		if (Left.Num() != 2 || !Left.Contains(EJamMechanic::Katana))
		{
			AddError(FString::Printf(TEXT("seed %d: the Katana and one more wait for the final note"), Seed));
			return false;
		}
	}

	FJamLedger Ledger;
	JamLoadoutRules::Give(Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Dash, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Parry, EJamSide::Boss);
	FRandomStream Stream(3);
	const TArray<EJamMechanic> Final = JamStealRules::Offer(Ledger, true, Stream);
	TestEqual(TEXT("the final note takes the rest"), Final.Num(), 2);
	TestTrue(TEXT("the Katana with it"), Final.Contains(EJamMechanic::Katana));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealSeedTest, "Jam.Steal.SameSeedSameOffer", JamTest::Flags)

bool FJamStealSeedTest::RunTest(const FString& Parameters)
{
	const FJamLedger Ledger;
	TSet<int32> LeftOptions;
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		FRandomStream First(Seed);
		FRandomStream Second(Seed);
		const TArray<EJamMechanic> A = JamStealRules::Offer(Ledger, false, First);
		const TArray<EJamMechanic> B = JamStealRules::Offer(Ledger, false, Second);
		if (A != B)
		{
			AddError(FString::Printf(TEXT("seed %d gave two different offers"), Seed));
			return false;
		}
		LeftOptions.Add(static_cast<int32>(A[0]));
	}
	TestEqual(
		TEXT("over many seeds every mechanic but the Katana gets to be the left option"), LeftOptions.Num(), JamSim::MechanicCount - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealChosenTest, "Jam.Steal.ChosenMechanicGoesToTheBoss", JamTest::Flags)

bool FJamStealChosenTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	const EJamMechanic Right = F.Steal.Offer[1];
	JamStealRules::Resolve(F, 1, B);
	TestTrue(TEXT("the right option was picked: the boss has it"), F.Ledger.Has(EJamSide::Boss, Right));
	TestEqual(TEXT("the change of hands is reported once"), JamTest::Count(F, EJamEvent::StealResolved), 1);
	const FJamEvent& Event = F.Events.Last();
	TestEqual(TEXT("with the new owner"), Event.Side, EJamSide::Boss);
	TestEqual(TEXT("and with the mechanic"), FMath::RoundToInt32(Event.Amount), static_cast<int32>(Right));
	TestTrue(TEXT("the boss takes a moment before he uses it"), F.Boss.BusyUntilTick > F.Tick);
	TestEqual(TEXT("the table is clear"), F.Steal.Offer.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealKeptTest, "Jam.Steal.TheOtherOneStays", JamTest::Flags)

bool FJamStealKeptTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	const EJamMechanic Left = F.Steal.Offer[0];
	const EJamMechanic Right = F.Steal.Offer[1];
	JamStealRules::Resolve(F, 0, B);
	TestTrue(TEXT("the left option went to the boss"), F.Ledger.Has(EJamSide::Boss, Left));
	TestTrue(TEXT("the right one stayed with the hero"), HeroHas(F, Right));
	TestEqual(TEXT("the hero has all but one left"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero).Num(), JamSim::MechanicCount - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealLastTest, "Jam.Steal.LastThresholdTakesTheRest", JamTest::Flags)

bool FJamStealLastTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	for (int32 Steal = 0; Steal < JamStealRules::ThresholdCount - 1; ++Steal)
	{
		JamTest::OpenSteal(F, B);
		TestFalse(TEXT("every card but the last is a choice"), F.Steal.bTakesAll);
		TestEqual(TEXT("of two"), F.Steal.Offer.Num(), 2);
		JamStealRules::Resolve(F, 0, B);
	}
	TestEqual(TEXT("two are left for the final note"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero).Num(), 2);
	TestTrue(TEXT("the Katana one of them"), HeroHas(F, EJamMechanic::Katana));

	JamTest::OpenSteal(F, B);
	TestEqual(TEXT("the last threshold opens a card too"), F.Phase, EJamPhase::Steal);
	TestEqual(TEXT("at the share of the final note"), F.Fighters[EJamSide::Boss].Health, B.StealThreshold4 * B.BossHealth);
	TestTrue(TEXT("but it is not a choice"), F.Steal.bTakesAll);
	// Nobody answers: the card runs out.
	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.StealChoiceSeconds));
	TestEqual(TEXT("the fight goes on"), F.Phase, EJamPhase::Fight);
	TestEqual(TEXT("the hero is a bean: nothing is his"), JamLoadoutRules::Owned(F.Ledger, EJamSide::Hero).Num(), 0);
	TestEqual(TEXT("no steal is left"), F.Steal.Floor, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealStandsTest, "Jam.Steal.TheFightStandsDuringTheChoice", JamTest::Flags)

bool FJamStealStandsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	FJamProjectile Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Pos = Memo.PrevPos = FVector2D(-200.0, 300.0);
	Memo.DieTick = 600;
	F.Projectiles.Add(Memo);
	F.Hazards.Add(FJamHazard());
	F.Hazards[0].HitTick = 600;
	F.Fighters[EJamSide::Hero].Katana.bHitPending = true;
	F.Fighters[EJamSide::Hero].Katana.HitTick = 600;
	JamTest::OpenSteal(F, B);
	TestEqual(TEXT("the grab clears the air of projectiles"), F.Projectiles.Num(), 0);
	TestEqual(TEXT("and of warning circles"), F.Hazards.Num(), 0);
	TestFalse(TEXT("a strike in its wind-up is dropped"), F.Fighters[EJamSide::Hero].Katana.bHitPending);

	const int32 Tick = F.Tick;
	FJamIntents Intents;
	Intents[EJamSide::Hero].Move = FVector2D(0.0, 1.0);
	Intents[EJamSide::Hero].bPrimary = true;
	Intents[EJamSide::Hero].bSecondary = true;
	Intents[EJamSide::Boss].bVolley = true;
	Intents[EJamSide::Boss].Move = FVector2D(-1.0, 0.0);
	JamTest::Run(F, Intents, B, 20);
	TestEqual(TEXT("the card is still up"), F.Phase, EJamPhase::Steal);
	TestEqual(TEXT("its clock runs"), F.Tick, Tick + 20);
	TestTrue(TEXT("the hero stands"), F.Fighters[EJamSide::Hero].Pos.Equals(FVector2D(-500.0, 0.0), 1e-3));
	TestTrue(TEXT("the boss stands"), F.Fighters[EJamSide::Boss].Pos.IsNearlyZero());
	TestEqual(TEXT("nobody shoots"), F.Projectiles.Num(), 0);
	TestEqual(TEXT("the timer on the card starts full"), JamStealRules::TimeLeft(F.Steal, static_cast<float>(F.Steal.StartTick)), 1.f);
	TestEqual(TEXT("and is empty when the time is up"), JamStealRules::TimeLeft(F.Steal, static_cast<float>(F.Steal.EndTick)), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealAnswerTest, "Jam.Steal.ChoiceResolvesAtOnce", JamTest::Flags)

bool FJamStealAnswerTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	const EJamMechanic Right = F.Steal.Offer[1];
	const int32 Lock = JamSim::Ticks(B.StealChoiceLock);
	AnswerFor(F, B, 1, Lock - 1);
	TestEqual(TEXT("a key pressed while the card is still locked does not choose"), F.Phase, EJamPhase::Steal);
	AnswerFor(F, B, 5, 1);
	TestEqual(TEXT("an answer that is not on the card is no answer"), F.Phase, EJamPhase::Steal);
	AnswerFor(F, B, 1, 1);
	TestEqual(TEXT("the first answer after the lock settles the steal on the same step"), F.Phase, EJamPhase::Fight);
	TestTrue(TEXT("with the picked option"), F.Ledger.Has(EJamSide::Boss, Right));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealTimeoutTest, "Jam.Steal.TimeoutGivesTheDefault", JamTest::Flags)

bool FJamStealTimeoutTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	const EJamMechanic Left = F.Steal.Offer[0];
	const EJamMechanic Right = F.Steal.Offer[1];
	const int32 Wait = JamSim::Ticks(B.StealChoiceSeconds);
	JamTest::Run(F, FJamIntents(), B, Wait - 1);
	TestEqual(TEXT("the card waits its whole time"), F.Phase, EJamPhase::Steal);
	JamTest::Run(F, FJamIntents(), B, 1);
	TestEqual(TEXT("then the Publisher stops waiting"), F.Phase, EJamPhase::Fight);
	TestTrue(TEXT("and takes the left option"), F.Ledger.Has(EJamSide::Boss, Left));
	TestTrue(TEXT("the right one stays"), HeroHas(F, Right));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealSafeTest, "Jam.Steal.NoDamageDuringTheChoice", JamTest::Flags)

bool FJamStealSafeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	TestFalse(TEXT("the hero cannot be hurt while he reads the card"), JamFightOps::Hurt(F, EJamSide::Hero, 50.f, FVector2D::ZeroVector));
	TestFalse(TEXT("and neither can the boss"), JamFightOps::Hurt(F, EJamSide::Boss, 50.f, FVector2D::ZeroVector));
	TestEqual(TEXT("the hero's health is whole"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestEqual(TEXT("the boss stays at the threshold"), F.Fighters[EJamSide::Boss].Health, 750.f);
	TestEqual(TEXT("no hit is reported"), JamTest::Count(F, EJamEvent::Hit), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStealCutDamageTest, "Jam.Steal.TheHeroHitsHarderAfterTheFirstCut", JamTest::Flags)

bool FJamStealCutDamageTest::RunTest(const FString& Parameters)
{
	// The author's play-throughs: the fight was too hard after the first cut. From it on, all his moves hit harder (1.2 in
	// T-099, 1.4 in T-100); before it nothing changes.
	const FJamBalanceData B = JamTest::Balance();
	TestTrue(TEXT("two fifths harder"), FMath::IsNearlyEqual(B.HeroCutDamageScale, 1.4f, 0.05f));
	TestEqual(TEXT("before the first cut: as listed"), JamHealthRules::HeroDamageScale(0, 1.4f), 1.f);
	TestEqual(TEXT("from the first cut on"), JamHealthRules::HeroDamageScale(1, 1.4f), 1.4f);
	TestEqual(TEXT("and after the others"), JamHealthRules::HeroDamageScale(3, 1.4f), 1.4f);

	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	const float Start = F.Fighters[EJamSide::Boss].Health;
	JamFightOps::Hurt(F, EJamSide::Boss, 100.f, FVector2D::ZeroVector);
	TestEqual(TEXT("a hit before the first cut takes what it says"), Start - F.Fighters[EJamSide::Boss].Health, 100.f);
	F.Steal.Done = 1;
	F.Steal.Floor = 0.f;
	const float Before = F.Fighters[EJamSide::Boss].Health;
	JamFightOps::Hurt(F, EJamSide::Boss, 100.f, FVector2D::ZeroVector);
	TestTrue(TEXT("after it, the same hit takes two fifths more"),
		FMath::IsNearlyEqual(Before - F.Fighters[EJamSide::Boss].Health, 100.f * B.HeroCutDamageScale, 0.01f));
	const float Hero = F.Fighters[EJamSide::Hero].Health;
	JamFightOps::Hurt(F, EJamSide::Hero, 30.f, FVector2D::ZeroVector);
	TestEqual(TEXT("the Publisher's hits on the hero do not change"), Hero - F.Fighters[EJamSide::Hero].Health, 30.f);
	return true;
}

#endif
