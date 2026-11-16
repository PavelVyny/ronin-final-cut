#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamCheckpointRules.h"
#include "Sim/JamFightOps.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-015: a death sends the player back to the last steal, not to the start of the fight.
 * A bug here looks in game like "I died as a bean and got the katana back", "the retry starts with a memo in my
 * face" or "the boss is at full health again after two cuts".
 *
 * Run: Automation RunTests Jam.Retry
 */
namespace
{
/** A fight right after the first steal was settled with the left option. */
FJamFight AfterTheFirstCut(const FJamBalanceData& B)
{
	FJamFight F = JamTest::Fight(B, FVector2D(-600.0, 0.0), FVector2D::ZeroVector);
	JamTest::OpenSteal(F, B);
	JamStealRules::Resolve(F, 0, B);
	return F;
}

/** The fight goes on after the checkpoint and goes badly: the boss loses some more, the hero loses everything. */
void PlayOnAndDie(FJamFight& F, const FJamBalanceData& B)
{
	F.Tick += 500;
	F.Boss.BusyUntilTick = 0;
	JamFightOps::Hurt(F, EJamSide::Boss, 100.f, FVector2D::ZeroVector);
	JamFightOps::Hurt(F, EJamSide::Hero, B.HeroHealth, FVector2D::ZeroVector);
	JamFightRules::Step(F, FJamIntents(), B);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRetryBossHealthTest, "Jam.Retry.RestoresBossHealthAtTheThreshold", JamTest::Flags)

bool FJamRetryBossHealthTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = AfterTheFirstCut(B);
	const FJamFight Saved = JamCheckpointRules::Capture(F, B);
	PlayOnAndDie(F, B);
	TestEqual(TEXT("the hero died"), F.Phase, EJamPhase::HeroDead);
	// After the first cut the hero's hit of 100 lands harder (HeroCutDamageScale).
	TestEqual(TEXT("with the boss below the checkpoint"), F.Fighters[EJamSide::Boss].Health, 750.f - 100.f * B.HeroCutDamageScale);

	const FJamFight Again = JamCheckpointRules::Restore(Saved, 7);
	TestEqual(TEXT("the retry is a fight again"), Again.Phase, EJamPhase::Fight);
	TestEqual(TEXT("the boss is back at the threshold of the last cut"), Again.Fighters[EJamSide::Boss].Health, 750.f);
	TestEqual(TEXT("that cut stays made"), Again.Steal.Done, 1);
	TestEqual(TEXT("the next one waits where it waited"), Again.Steal.Floor, 500.f);
	TestEqual(TEXT("no card is up"), Again.Steal.Offer.Num(), 0);
	TestEqual(TEXT("the clock starts over"), Again.Tick, 0);

	const FJamFight Fresh = JamCheckpointRules::Restore(JamCheckpointRules::Capture(JamFightRules::Start(B, 1), B), 7);
	TestEqual(TEXT("a death before the first cut restarts the whole fight"), Fresh.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestEqual(TEXT("with the first threshold ahead"), Fresh.Steal.Floor, 750.f);
	return true;
}

/**
 * T-048: the author's playtest: coming back whole felt unfair. The checkpoint keeps the hero's health of the moment the
 * cut was made; only a hero at 20% or less comes back with half, so a retry is never hopeless.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRetryHealthTest, "Jam.Retry.HeroComesBackWithTheHealthOfTheCut", JamTest::Flags)

bool FJamRetryHealthTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const auto RetryWith = [&B](float HealthAtTheCut)
	{
		FJamFight F = AfterTheFirstCut(B);
		F.Fighters[EJamSide::Hero].Health = HealthAtTheCut;
		const FJamFight Saved = JamCheckpointRules::Capture(F, B);
		PlayOnAndDie(F, B);
		return JamCheckpointRules::Restore(Saved, 7).Fighters[EJamSide::Hero].Health;
	};
	TestEqual(TEXT("whole at the cut: whole again"), RetryWith(100.f), 100.f);
	TestEqual(TEXT("hurt at the cut: just as hurt"), RetryWith(60.f), 60.f);
	TestEqual(TEXT("just above a fifth: as he was"), RetryWith(21.f), 21.f);
	TestEqual(TEXT("a fifth or less: half"), RetryWith(20.f), 50.f);
	TestEqual(TEXT("nearly dead: half"), RetryWith(3.f), 50.f);
	TestEqual(TEXT("the rule itself"), JamCheckpointRules::RetryHealth(10.f, B), 50.f);
	TestEqual(TEXT("a fresh fight starts whole"),
		JamCheckpointRules::Restore(JamCheckpointRules::Capture(JamFightRules::Start(B, 1), B), 7).Fighters[EJamSide::Hero].Health,
		B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRetryHeroTest, "Jam.Retry.HeroKeepsOnlyUnstolenMechanics", JamTest::Flags)

bool FJamRetryHeroTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = AfterTheFirstCut(B);
	const FJamLedger Ledger = F.Ledger;
	const FJamFight Saved = JamCheckpointRules::Capture(F, B);
	// One more cut after the checkpoint was taken: it is not part of that checkpoint.
	JamTest::OpenSteal(F, B);
	JamStealRules::Resolve(F, 0, B);
	PlayOnAndDie(F, B);

	const FJamFight Again = JamCheckpointRules::Restore(Saved, 7);
	const FJamFight Start = JamFightRules::Start(B, 7);
	TestTrue(TEXT("he stands on his mark"), Again.Fighters[EJamSide::Hero].Pos.Equals(Start.Fighters[EJamSide::Hero].Pos));
	TestTrue(TEXT("the boss on his"), Again.Fighters[EJamSide::Boss].Pos.Equals(Start.Fighters[EJamSide::Boss].Pos));
	TestEqual(TEXT("the hero has all but one, as after the first cut"), JamLoadoutRules::Owned(Again.Ledger, EJamSide::Hero).Num(),
		JamSim::MechanicCount - 1);
	for (int32 Index = 0; Index < JamSim::MechanicCount; ++Index)
	{
		TestEqual(TEXT("every mechanic is with who had it at the checkpoint"), Again.Ledger.Owner[Index], Ledger.Owner[Index]);
	}
	TestEqual(TEXT("the boss waits a moment before his first move"), Again.Boss.BusyUntilTick, JamSim::Ticks(B.BossRecovery));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRetryStageTest, "Jam.Retry.ClearsProjectilesAndHazards", JamTest::Flags)

bool FJamRetryStageTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = AfterTheFirstCut(B);
	// The checkpoint is taken from a fight in full swing: none of it may leak into the retry.
	F.Tick = 900;
	F.Projectiles.Add(FJamProjectile());
	F.Hazards.Add(FJamHazard());
	F.Fighters[EJamSide::Hero].Dash.ReadyTick = 950;
	F.Fighters[EJamSide::Hero].Katana.bHitPending = true;
	F.Fighters[EJamSide::Hero].Shuriken.Throws = 2;
	F.Boss.SlamReadyTick = 1200;
	F.Boss.VolleyReadyTick = 1000;

	const FJamFight Again = JamCheckpointRules::Restore(JamCheckpointRules::Capture(F, B), 7);
	TestEqual(TEXT("nothing flies"), Again.Projectiles.Num(), 0);
	TestEqual(TEXT("no circle waits on the floor"), Again.Hazards.Num(), 0);
	TestEqual(TEXT("the dash is ready"), Again.Fighters[EJamSide::Hero].Dash.ReadyTick, 0);
	TestFalse(TEXT("no strike is on its way"), Again.Fighters[EJamSide::Hero].Katana.bHitPending);
	TestEqual(TEXT("the shuriken count starts over"), Again.Fighters[EJamSide::Hero].Shuriken.Throws, 0);
	TestEqual(TEXT("the slam is not on a cooldown from the other life"), Again.Boss.SlamReadyTick, 0);
	TestEqual(TEXT("the volley neither"), Again.Boss.VolleyReadyTick, 0);
	TestEqual(TEXT("no event is left over"), Again.Events.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRetryLuckTest, "Jam.Retry.NewSeedNewLuck", JamTest::Flags)

bool FJamRetryLuckTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamFight Saved = JamCheckpointRules::Capture(JamFightRules::Start(B, 1), B);
	// The same seed is the same fight, and across seeds the next card is not always the same one.
	TSet<int32> LeftOptions;
	for (int32 Seed = 1; Seed <= 40; ++Seed)
	{
		FJamFight First = JamCheckpointRules::Restore(Saved, Seed);
		FJamFight Second = JamCheckpointRules::Restore(Saved, Seed);
		JamTest::OpenSteal(First, B);
		JamTest::OpenSteal(Second, B);
		if (!TestEqual(TEXT("a card of two"), First.Steal.Offer.Num(), 2))
		{
			return false;
		}
		TestTrue(TEXT("the same seed deals the same card"), First.Steal.Offer == Second.Steal.Offer);
		LeftOptions.Add(static_cast<int32>(First.Steal.Offer[0]));
	}
	TestEqual(TEXT("over forty retries every mechanic but the Katana was the left option at least once"), LeftOptions.Num(),
		JamSim::MechanicCount - 1);
	return true;
}

#endif
