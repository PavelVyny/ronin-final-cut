#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamBotRules.h"
#include "Sim/JamDifficultyRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-016: whole fights played by the bot, with the numbers of the game (the [JamBalance] section, not the round numbers
 * of the rule tests). They answer the main risk of the plan: can a bean beat the boss who holds everything, and how
 * long does the fight last.
 * A bug here looks in game like "the fight is over in a minute", "nobody can win once the Publisher has the katana" or
 * "the same fight plays out differently on a retry with the same seed".
 *
 * Run: Automation RunTests Jam.Bot
 */
namespace
{
constexpr int32 SeedsPerPolicy = 20;
// Far beyond any sane fight: a run that reaches it is a fight that never ends.
constexpr float RunLimitSeconds = 900.f;
// The bot is quicker and more patient than a person, so its fights sit well below a person's.
// T-043: the author's playtest asked for 4000 health and a short final stretch; the bot median fell from 151 to 92 s,
// and the floor (120 s since T-031) went down to 60 s on that word.
// T-051: the Katana is never on a choice card, the bot kills faster (median 83 to 78 s, answering right 67 s); the
// author kept 4000 health and moved the floor to 50 s (chat, Sat 14 Nov 16:45): a person's run is longer anyway.
constexpr float ShortestMedianSeconds = 50.f;
constexpr float LongestMedianSeconds = 300.f;
// T-031: each way of answering the cards keeps this much room above the floor, so one balance change in an unrelated
// number does not drop the whole fight under it.
constexpr float RoomAboveTheFloorSeconds = 15.f;
// Mechanics a choice card deals from: all but the Katana (T-051).
constexpr int32 CardMechanics = JamSim::MechanicCount - 1;
// Orders in which the first two mechanics can go: any of them on the first card, any of the others on the second.
constexpr int32 StealOrders = CardMechanics * (CardMechanics - 1);
// Runs each order gets in Jam.Bot.EveryStealOrderIsWinnable, and the most seeds it may take to get them.
constexpr int32 RunsPerOrder = 2;
constexpr int32 OrderSeedCap = 600;
const EJamStealPolicy EveryPolicy[] = { EJamStealPolicy::Left, EJamStealPolicy::Right, EJamStealPolicy::Mixed };
const TCHAR* const PolicyNames[] = { TEXT("left"), TEXT("right"), TEXT("mixed") };

FJamBalanceData GameBalance()
{
	return JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
}

TArray<FJamBotRun> RunPolicy(const FJamBalanceData& B, EJamStealPolicy Policy)
{
	TArray<FJamBotRun> Runs;
	for (int32 Seed = 1; Seed <= SeedsPerPolicy; ++Seed)
	{
		Runs.Add(JamBotRules::RunFight(B, Seed, Policy, RunLimitSeconds));
	}
	return Runs;
}

TArray<FJamBotRun> RunEveryPolicy(const FJamBalanceData& B)
{
	TArray<FJamBotRun> Runs;
	for (const EJamStealPolicy Policy : EveryPolicy)
	{
		Runs.Append(RunPolicy(B, Policy));
	}
	return Runs;
}

/** Seconds of the won fights, sorted. */
TArray<float> WinSeconds(const TArray<FJamBotRun>& Runs)
{
	TArray<float> Seconds;
	for (const FJamBotRun& Run : Runs)
	{
		if (Run.Outcome == EJamPhase::Won)
		{
			Seconds.Add(Run.Seconds);
		}
	}
	Seconds.Sort();
	return Seconds;
}

float MedianOf(const TArray<float>& Sorted)
{
	return Sorted.IsEmpty() ? 0.f : Sorted[Sorted.Num() / 2];
}

/** One line of the results table. */
FString TableRow(const TCHAR* Name, const TArray<FJamBotRun>& Runs)
{
	const TArray<float> Seconds = WinSeconds(Runs);
	float Closest = TNumericLimits<float>::Max();
	for (const FJamBotRun& Run : Runs)
	{
		Closest = Run.Outcome == EJamPhase::Won ? FMath::Min(Closest, Run.LowestHeroHealth) : Closest;
	}
	return FString::Printf(
		TEXT("bot table | %-5s | wins %2d of %2d | seconds: median %3.0f, from %3.0f to %3.0f | closest win at %2.0f health"), Name,
		Seconds.Num(), Runs.Num(), MedianOf(Seconds), Seconds.IsEmpty() ? 0.f : Seconds[0], Seconds.IsEmpty() ? 0.f : Seconds.Last(),
		Seconds.IsEmpty() ? 0.f : Closest);
}

/** Where the lost fights were lost: how long the hero lasted and what the boss had left. */
FString LossRow(const TArray<FJamBotRun>& Runs)
{
	TArray<float> Seconds;
	TArray<float> BossLeft;
	for (const FJamBotRun& Run : Runs)
	{
		if (Run.Outcome == EJamPhase::HeroDead)
		{
			Seconds.Add(Run.Seconds);
			BossLeft.Add(Run.BossHealthLeft);
		}
	}
	if (Seconds.IsEmpty())
	{
		return FString::Printf(TEXT("bot table | lost  |  0 of %2d"), Runs.Num());
	}
	Seconds.Sort();
	BossLeft.Sort();
	return FString::Printf(
		TEXT("bot table | lost  | %2d of %2d | died after: median %3.0f s | boss health left: median %4.0f, from %4.0f to %4.0f"),
		Seconds.Num(), Runs.Num(), MedianOf(Seconds), MedianOf(BossLeft), BossLeft[0], BossLeft.Last());
}

/** Which of the twelve orders a run went through, -1 when it ended before the second card. */
int32 OrderOf(const FJamBotRun& Run)
{
	if (Run.Stolen.Num() < 2)
	{
		return -1;
	}
	// The Katana is mechanic 0 and never on a card: the others count from 0.
	const int32 First = static_cast<int32>(Run.Stolen[0]) - 1;
	const int32 Second = static_cast<int32>(Run.Stolen[1]) - 1;
	// The second one is one of the others: its place among them.
	return First * (CardMechanics - 1) + (Second > First ? Second - 1 : Second);
}

/** The median share of his health the hero had left at his lowest in the won fights: how close the wins were. */
float MedianLowestShare(const TArray<FJamBotRun>& Runs, float MaxHealth)
{
	TArray<float> Shares;
	for (const FJamBotRun& Run : Runs)
	{
		if (Run.Outcome == EJamPhase::Won)
		{
			Shares.Add(Run.LowestHeroHealth / FMath::Max(MaxHealth, 1.f));
		}
	}
	Shares.Sort();
	return MedianOf(Shares);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotChoiceTest, "Jam.Bot.ChoosesByItsPolicy", JamTest::Flags)

bool FJamBotChoiceTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	TestEqual(TEXT("no card, no answer"), JamBotRules::Choose(F, EJamStealPolicy::Right), -1);

	JamTest::OpenSteal(F, B);
	TestEqual(TEXT("the card is still locked: the bot waits"), JamBotRules::Choose(F, EJamStealPolicy::Left), -1);
	F.Tick = F.Steal.OpenTick;
	TestEqual(TEXT("left gives the left option"), JamBotRules::Choose(F, EJamStealPolicy::Left), 0);
	TestEqual(TEXT("right gives the right one"), JamBotRules::Choose(F, EJamStealPolicy::Right), 1);
	TestEqual(TEXT("mixed starts from the left"), JamBotRules::Choose(F, EJamStealPolicy::Mixed), 0);

	JamStealRules::Resolve(F, 0, B);
	JamTest::OpenSteal(F, B);
	F.Tick = F.Steal.OpenTick;
	TestEqual(TEXT("mixed gives the right one on the second card"), JamBotRules::Choose(F, EJamStealPolicy::Mixed), 1);

	JamStealRules::Resolve(F, 1, B);
	JamTest::OpenSteal(F, B);
	JamStealRules::Resolve(F, 0, B);
	JamTest::OpenSteal(F, B);
	F.Tick = F.Steal.OpenTick;
	TestTrue(TEXT("the last card takes everything left"),
		F.Steal.bTakesAll && F.Steal.Offer.Num() == JamSim::MechanicCount - (JamStealRules::ThresholdCount - 1));
	TestEqual(TEXT("the bot hands it over whatever its policy"), JamBotRules::Choose(F, EJamStealPolicy::Right), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotBladeTest, "Jam.Bot.BacksOffFromAWindingKatana", JamTest::Flags)

bool FJamBotBladeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D ToBoss(1.0, 0.0);
	FJamFight F = JamTest::Fight(B, FVector2D(-220.0, 0.0), FVector2D::ZeroVector);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Katana, EJamSide::Boss);
	F.Boss.BusyUntilTick = 0;
	FJamKatanaState& Blade = F.Fighters[EJamSide::Boss].Katana;
	Blade.Strike = 1;
	Blade.bHitPending = true;
	Blade.HitTick = F.Tick + 25;

	const FJamIntent Early = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("his blade is on its way and the bot is in its reach: it backs off"), FVector2D::DotProduct(Early.Move, ToBoss) < -0.9);
	TestFalse(TEXT("there is time to walk out: the dash is kept"), Early.bDash);

	Blade.HitTick = F.Tick + 3;
	TestTrue(TEXT("the blade is about to land: the bot dashes out"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-700.0, 0.0);
	const FJamIntent Far = JamBotRules::Play(F, EJamSide::Hero, B);
	TestFalse(TEXT("out of reach the blade is no reason to dash"), Far.bDash);
	TestTrue(TEXT("nor to run from it: the bot keeps circling"), FMath::Abs(Far.Move.Y) > 0.1);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-220.0, 0.0);
	Blade.bHitPending = false;
	const FJamIntent After = JamBotRules::Play(F, EJamSide::Hero, B);
	TestFalse(TEXT("the blade has landed: nothing to dash from"), After.bDash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotSameSeedTest, "Jam.Bot.SameSeedSameFight", JamTest::Flags)

bool FJamBotSameSeedTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = GameBalance();
	const FJamBotRun First = JamBotRules::RunFight(B, 7, EJamStealPolicy::Mixed, RunLimitSeconds);
	const FJamBotRun Second = JamBotRules::RunFight(B, 7, EJamStealPolicy::Mixed, RunLimitSeconds);
	TestEqual(TEXT("the same outcome"), Second.Outcome, First.Outcome);
	TestEqual(TEXT("to the step"), Second.Seconds, First.Seconds);
	TestEqual(TEXT("with the same wounds"), Second.LowestHeroHealth, First.LowestHeroHealth);
	TestTrue(TEXT("and the same cuts in the same order"), Second.Stolen == First.Stolen);

	bool bAnyOther = false;
	for (int32 Seed = 8; Seed < 12; ++Seed)
	{
		bAnyOther |= JamBotRules::RunFight(B, Seed, EJamStealPolicy::Mixed, RunLimitSeconds).Seconds != First.Seconds;
	}
	TestTrue(TEXT("another seed is another fight"), bAnyOther);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotEndsTest, "Jam.Bot.FightAlwaysEnds", JamTest::Flags)

bool FJamBotEndsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = GameBalance();
	for (const FJamBotRun& Run : RunPolicy(B, EJamStealPolicy::Mixed))
	{
		const bool bEnded = Run.Outcome == EJamPhase::Won || Run.Outcome == EJamPhase::HeroDead;
		TestTrue(TEXT("the fight ends with a title, not with the clock"), bEnded);
		TestTrue(TEXT("the hero's lowest health is a health"), Run.LowestHeroHealth <= B.HeroHealth);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotOrdersTest, "Jam.Bot.EveryStealOrderIsWinnable", JamTest::Flags)

bool FJamBotOrdersTest::RunTest(const FString& Parameters)
{
	// With four mechanics on the cards the twelve orders are not all hit by the seeds of the other bot tests: seeds go on until
	// every order has its runs, or the cap says the cards never deal one.
	const FJamBalanceData B = GameBalance();
	int32 Runs[StealOrders] = {};
	int32 Wins[StealOrders] = {};
	for (int32 Seed = 1; Seed <= OrderSeedCap; ++Seed)
	{
		const FJamBotRun Run = JamBotRules::RunFight(B, Seed, EveryPolicy[Seed % UE_ARRAY_COUNT(EveryPolicy)], RunLimitSeconds);
		const int32 Order = OrderOf(Run);
		if (Order >= 0)
		{
			++Runs[Order];
			Wins[Order] += Run.Outcome == EJamPhase::Won ? 1 : 0;
		}
		bool bCovered = true;
		for (int32 Index = 0; Index < StealOrders; ++Index)
		{
			bCovered &= Runs[Index] >= RunsPerOrder;
		}
		if (bCovered)
		{
			break;
		}
	}
	for (int32 Order = 0; Order < StealOrders; ++Order)
	{
		const EJamMechanic First = static_cast<EJamMechanic>(Order / (CardMechanics - 1) + 1);
		AddInfo(FString::Printf(
			TEXT("bot table | %s first, order %d | wins %d of %d"), JamLoadoutRules::Name(First), Order, Wins[Order], Runs[Order]));
		TestTrue(TEXT("the seeds cover every order of the cuts"), Runs[Order] > 0);
		TestTrue(TEXT("and every order can be won"), Wins[Order] > 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotLengthTest, "Jam.Bot.FightLengthIsInRange", JamTest::Flags)

bool FJamBotLengthTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = GameBalance();
	TArray<FJamBotRun> All;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(EveryPolicy); ++Index)
	{
		const TArray<FJamBotRun> Runs = RunPolicy(B, EveryPolicy[Index]);
		AddInfo(TableRow(PolicyNames[Index], Runs));
		All.Append(Runs);
	}
	AddInfo(TableRow(TEXT("all"), All));
	AddInfo(LossRow(All));
	const TArray<float> Seconds = WinSeconds(All);
	TestTrue(TEXT("the bot wins often enough for a median to mean something"), Seconds.Num() * 4 >= All.Num());
	TestTrue(TEXT("a won fight is not over in a moment"), MedianOf(Seconds) >= ShortestMedianSeconds);
	TestTrue(TEXT("and does not drag"), MedianOf(Seconds) <= LongestMedianSeconds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotRoomTest, "Jam.Bot.EveryPolicyLeavesRoomAboveTheFloor", JamTest::Flags)

bool FJamBotRoomTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = GameBalance();
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(EveryPolicy); ++Index)
	{
		const TArray<FJamBotRun> Runs = RunPolicy(B, EveryPolicy[Index]);
		const TArray<float> Seconds = WinSeconds(Runs);
		TestEqual(FString::Printf(TEXT("the bot still wins every fight answering %s"), PolicyNames[Index]), Seconds.Num(), Runs.Num());
		TestTrue(FString::Printf(TEXT("answering %s, the median %.0f s keeps %.0f s above the %.0f s floor"), PolicyNames[Index],
					 MedianOf(Seconds), RoomAboveTheFloorSeconds, ShortestMedianSeconds),
			MedianOf(Seconds) >= ShortestMedianSeconds + RoomAboveTheFloorSeconds);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotEasierTest, "Jam.Bot.TheFinalBuildIsEasier", JamTest::Flags)

bool FJamBotEasierTest::RunTest(const FString& Parameters)
{
	// T-100, the author's last play-through: without a dash the melee was scary and play turned into shuriken clicking.
	// The hero has a fifth more health in every run, and after the first cut he hits harder again on top of T-099's 1.2:
	// 1.4, the most that keeps every way of answering the cards above the floor (Jam.Bot.EveryPolicyLeavesRoomAboveTheFloor;
	// 1.5 took the mixed answers to a 60 s median). The bot's closest win went from 43 to 72 health.
	const FJamBalanceData B = GameBalance();
	TestEqual(TEXT("a fifth more health than the 100 before"), B.HeroHealth, 120.f);
	TestEqual(TEXT("from the first cut on his hits land 1.4 times as hard"), B.HeroCutDamageScale, 1.4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotDifficultyTest, "Jam.Bot.EveryDifficultyIsWinnable", JamTest::Flags)

bool FJamBotDifficultyTest::RunTest(const FString& Parameters)
{
	// T-101: EASY a comfortable win, NORMAL the game as it was, HARD a real fight the bot still wins.
	const TCHAR* const Names[] = { TEXT("easy"), TEXT("norm"), TEXT("hard") };
	int32 Wins[JamDifficultyRules::Count] = {};
	float Lowest[JamDifficultyRules::Count] = {};
	for (int32 Level = 0; Level < JamDifficultyRules::Count; ++Level)
	{
		const FJamBalanceData B = JamDifficultyRules::Apply(GameBalance(), JamDifficultyRules::FromIndex(Level));
		const TArray<FJamBotRun> Runs = RunEveryPolicy(B);
		Wins[Level] = WinSeconds(Runs).Num();
		Lowest[Level] = MedianLowestShare(Runs, B.HeroHealth);
		AddInfo(
			TableRow(Names[Level], Runs) + FString::Printf(TEXT(" | median lowest %.0f%% of %.0f"), 100.f * Lowest[Level], B.HeroHealth));
		AddInfo(LossRow(Runs));
	}
	const int32 RunsPerLevel = SeedsPerPolicy * UE_ARRAY_COUNT(EveryPolicy);
	TestEqual(TEXT("EASY: the bot wins every fight"), Wins[0], RunsPerLevel);
	TestTrue(TEXT("with more health to spare than on NORMAL"), Lowest[0] > Lowest[1]);
	TestTrue(TEXT("HARD: the bot still wins most fights"), Wins[2] * 2 > RunsPerLevel);
	TestTrue(TEXT("but closer than on NORMAL"), Lowest[2] < Lowest[1]);
	return true;
}

#endif
