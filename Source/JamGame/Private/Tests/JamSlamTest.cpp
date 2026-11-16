#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamBossRules.h"
#include "Sim/JamBotRules.h"
#include "Sim/JamHazardRules.h"
#include "Sim/JamSlamRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-050: the author's request: the slam grows with each cut, 1-3-5 circles that close the floor around the Publisher,
 * the hero's spot and his ways out, landing 0.2 s apart. A bug looks like "there was nowhere to go", "the circles
 * covered my heal", "the slam is the same after the cut" or "the bot stands in the second circle".
 *
 * Run: Automation RunTests Jam.Slam
 */
namespace
{
constexpr int32 SlamTick = 100;

FJamBalanceData SlamGameBalance()
{
	return JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
}

TArray<FJamHazard> MakePattern(const FVector2D& BossPos, const FVector2D& HeroPos, int32 Count, int32 Seed, const FJamBalanceData& B,
	const TArray<FJamPickup>& Pickups = {})
{
	FRandomStream Stream(Seed);
	return JamSlamRules::Pattern(BossPos, HeroPos, Count, Pickups, SlamTick, Stream, B);
}

/**
 * Walks a fighter from Pos straight to Spot at the hero's speed, starting Reaction seconds after Tick, one fixed step
 * at a time, and says whether a circle landed on him on the way. Independent of the rule's own arithmetic.
 */
bool CaughtOnTheWay(const TArray<FJamHazard>& Circles, FVector2D Pos, const FVector2D& Spot, float Reaction, const FJamBalanceData& B)
{
	const int32 Start = SlamTick + JamSim::Ticks(Reaction);
	int32 Last = SlamTick;
	for (const FJamHazard& Circle : Circles)
	{
		Last = FMath::Max(Last, Circle.HitTick);
	}
	const double StepLength = B.HeroSpeed * JamSim::StepDt;
	for (int32 Tick = SlamTick + 1; Tick <= Last; ++Tick)
	{
		if (Tick > Start)
		{
			const FVector2D To = Spot - Pos;
			Pos = To.Size() <= StepLength ? Spot : Pos + To.GetSafeNormal() * StepLength;
		}
		for (const FJamHazard& Circle : Circles)
		{
			if (Circle.HitTick == Tick && JamHazardRules::Hits(Circle, Pos))
			{
				return true;
			}
		}
	}
	return false;
}

/** The Publisher around a hero at Hero: close, halfway and at the edge of his slam range, every 45 degrees, on the floor. */
TArray<FVector2D> BossesAround(const FVector2D& Hero, const FJamBalanceData& B)
{
	TArray<FVector2D> Result;
	const double Distances[] = { B.BossRadius + B.HeroRadius + 10.0, 0.6 * B.SlamRange, B.SlamRange };
	for (int32 Angle = 0; Angle < 360; Angle += 45)
	{
		for (const double Distance : Distances)
		{
			const FVector2D Boss = Hero + FVector2D(1.0, 0.0).GetRotated(static_cast<float>(Angle)) * Distance;
			if (Boss.Size() <= B.ArenaRadius - B.BossRadius)
			{
				Result.Add(Boss);
			}
		}
	}
	return Result;
}

/** Hero and Publisher places over the whole arena: a grid every 250 cm, the points past the wall pulled onto it. */
TArray<TPair<FVector2D, FVector2D>> SweepPlaces(const FJamBalanceData& B)
{
	TArray<TPair<FVector2D, FVector2D>> Result;
	const double HeroRoom = B.ArenaRadius - B.HeroRadius;
	for (double X = -HeroRoom; X <= HeroRoom; X += 250.0)
	{
		for (double Y = -HeroRoom; Y <= HeroRoom; Y += 250.0)
		{
			// The ring of spots against the wall is where the floor runs out first.
			const FVector2D Grid(X, Y);
			const FVector2D Hero = Grid.Size() <= HeroRoom ? Grid : Grid.GetSafeNormal() * HeroRoom;
			for (const FVector2D& Boss : BossesAround(Hero, B))
			{
				Result.Emplace(Hero, Boss);
			}
		}
	}
	return Result;
}

/** The least room between the edges of any two of the circles, and which two they are. */
double NarrowestGap(const TArray<FJamHazard>& Circles, int32& OutFirst, int32& OutSecond)
{
	double Result = TNumericLimits<double>::Max();
	for (int32 First = 0; First < Circles.Num(); ++First)
	{
		for (int32 Second = First + 1; Second < Circles.Num(); ++Second)
		{
			const double Gap =
				FVector2D::Distance(Circles[First].Pos, Circles[Second].Pos) - Circles[First].Radius - Circles[Second].Radius;
			if (Gap < Result)
			{
				Result = Gap;
				OutFirst = First;
				OutSecond = Second;
			}
		}
	}
	return Result;
}

/** Degrees between two directions. */
double DegreesBetween(const FVector2D& A, const FVector2D& Dir)
{
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector2D::DotProduct(A.GetSafeNormal(), Dir.GetSafeNormal()), -1.0, 1.0)));
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamCountTest, "Jam.Slam.TheCountGrowsWithTheCuts", JamTest::Flags)

bool FJamSlamCountTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	TestEqual(TEXT("before the first cut: one circle"), JamSlamRules::CircleCount(0, B), 1);
	TestEqual(TEXT("after the first cut: three"), JamSlamRules::CircleCount(1, B), 3);
	TestEqual(TEXT("after the second cut: five"), JamSlamRules::CircleCount(2, B), 5);
	TestEqual(TEXT("and five to the end"), JamSlamRules::CircleCount(4, B), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamShapeTest, "Jam.Slam.ThePatternClosesTheBossTheHeroAndTheWayBack", JamTest::Flags)

bool FJamSlamShapeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D Boss(400.0, 0.0);
	const FVector2D Hero = FVector2D::ZeroVector;
	const FVector2D WayBack(-1.0, 0.0);
	const int32 FirstHit = SlamTick + JamSim::Ticks(B.SlamTelegraph);

	const TArray<FJamHazard> One = MakePattern(Boss, Hero, 1, 1, B);
	if (TestEqual(TEXT("one circle before the cut"), One.Num(), 1))
	{
		TestTrue(TEXT("on the hero, as the old slam"), One[0].Pos.Equals(Hero, 0.5));
		TestEqual(TEXT("of the slam radius"), One[0].Radius, B.SlamRadius);
		TestEqual(TEXT("landing after the telegraph"), One[0].HitTick, FirstHit);
	}

	const TArray<FJamHazard> Three = MakePattern(Boss, Hero, 3, 1, B);
	if (!TestEqual(TEXT("three circles after the first cut"), Three.Num(), 3))
	{
		return false;
	}
	TestTrue(TEXT("the first around the Publisher"), Three[0].Pos.Equals(Boss, 0.5));
	TestEqual(TEXT("of the side radius"), Three[0].Radius, B.SlamSideRadius);
	TestTrue(TEXT("the second on the hero"), Three[1].Pos.Equals(Hero, 0.5));
	TestEqual(TEXT("of the slam radius"), Three[1].Radius, B.SlamRadius);
	const FVector2D Back = Three[2].Pos - Hero;
	TestEqual(TEXT("the third cuts the way back at the cutter distance"), Back.Size(), static_cast<double>(B.SlamCutterDistance), 1.0);
	TestTrue(TEXT("behind the hero, within the jitter"), DegreesBetween(Back, WayBack) <= B.SlamJitter + 0.5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamSpreadTest, "Jam.Slam.FiveSpreadAroundHimAndTheHero", JamTest::Flags)

bool FJamSlamSpreadTest::RunTest(const FString& Parameters)
{
	// T-052: five circles spread out: two at his flanks, three wide, with room to stand between any two of them.
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D Boss(400.0, 0.0);
	const FVector2D Hero = FVector2D::ZeroVector;
	const FVector2D WayBack(-1.0, 0.0);
	const int32 FirstHit = SlamTick + JamSim::Ticks(B.SlamTelegraph);
	const TArray<FJamHazard> Five = MakePattern(Boss, Hero, 5, 1, B);
	if (!TestEqual(TEXT("five circles after the second cut"), Five.Num(), 5))
	{
		return false;
	}
	const FVector2D ToHero = Hero - Boss;
	TestTrue(TEXT("the first two on both sides of him"),
		FVector2D::CrossProduct(ToHero, Five[0].Pos - Boss) * FVector2D::CrossProduct(ToHero, Five[1].Pos - Boss) < 0.0);
	for (const int32 Near : { 0, 1 })
	{
		TestEqual(TEXT("of the side radius"), Five[Near].Radius, B.SlamSideRadius);
		TestTrue(TEXT("right next to him: the edge within a gap of his body"),
			FVector2D::Distance(Five[Near].Pos, Boss) - Five[Near].Radius - B.BossRadius <= B.SlamGap + 0.5);
	}
	TestTrue(TEXT("the third on the hero"), Five[2].Pos.Equals(Hero, 0.5));
	TestEqual(TEXT("of the slam radius"), Five[2].Radius, B.SlamRadius);
	const FVector2D Left = Five[3].Pos - Hero;
	const FVector2D Right = Five[4].Pos - Hero;
	TestTrue(TEXT("the wide two sit on both sides of the way back"),
		FVector2D::CrossProduct(WayBack, Left) * FVector2D::CrossProduct(WayBack, Right) < 0.0);
	for (const int32 Wide : { 3, 4 })
	{
		TestTrue(TEXT("farther from him than the hero"), FVector2D::Distance(Five[Wide].Pos, Boss) > FVector2D::Distance(Hero, Boss));
	}
	int32 First = 0;
	int32 Second = 0;
	const double Gap = NarrowestGap(Five, First, Second);
	TestTrue(FString::Printf(TEXT("every two leave the gap between them (circles %d and %d: %.0f)"), First, Second, Gap),
		Gap >= B.SlamGap - 0.5);
	for (int32 Index = 0; Index < Five.Num(); ++Index)
	{
		TestEqual(TEXT("all appear at once"), Five[Index].StartTick, SlamTick);
		TestEqual(TEXT("and land one after another, a ripple apart"), Five[Index].HitTick, FirstHit + Index * JamSim::Ticks(B.SlamRipple));
		TestEqual(TEXT("they are his"), Five[Index].Owner, EJamSide::Boss);
		TestEqual(TEXT("with the slam's damage"), Five[Index].Damage, B.SlamDamage);
	}
	TestTrue(TEXT("the same seed puts down the same pattern"),
		MakePattern(Boss, Hero, 5, 7, B)[3].Pos.Equals(MakePattern(Boss, Hero, 5, 7, B)[3].Pos, 0.01));
	bool bDiffers = false;
	for (int32 Seed = 2; Seed <= 6; ++Seed)
	{
		bDiffers = bDiffers || !MakePattern(Boss, Hero, 5, Seed, B)[3].Pos.Equals(Five[3].Pos, 1.0);
	}
	TestTrue(TEXT("other seeds move the cutters"), bDiffers);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamSafeTest, "Jam.Slam.EveryPatternLeavesAReachableSafeSpot", JamTest::Flags)

bool FJamSlamSafeTest::RunTest(const FString& Parameters)
{
	// The game's own numbers: the guarantee is about the fight people play.
	const FJamBalanceData B = SlamGameBalance();
	const double HeroRoom = B.ArenaRadius - B.HeroRadius;
	int32 Patterns = 0;
	int32 Failures = 0;
	int32 Fives = 0;
	int32 FullFives = 0;
	FString FirstFailure;
	for (const TPair<FVector2D, FVector2D>& Place : SweepPlaces(B))
	{
		// Each count with two seeds: the seed only moves the cutters.
		for (const int32 Run : { 0, 1, 2, 3, 4, 5 })
		{
			const int32 Count = 1 + 2 * (Run / 2);
			const int32 Seed = 1 + Run % 2;
			const TArray<FJamHazard> Circles = MakePattern(Place.Value, Place.Key, Count, Seed, B);
			const TOptional<FVector2D> Spot = JamSlamRules::SafeSpot(Circles, Place.Key, SlamTick, B.SlamReaction, B);
			++Patterns;
			Fives += Count == 5 ? 1 : 0;
			FullFives += Count == 5 && Circles.Num() == 5 ? 1 : 0;
			const bool bSafe = Circles.Num() > 0 && Spot.IsSet() && Spot->Size() <= HeroRoom + 0.5 &&
				!CaughtOnTheWay(Circles, Place.Key, *Spot, B.SlamReaction, B);
			if (!bSafe && Failures++ == 0)
			{
				FirstFailure = FString::Printf(
					TEXT("hero %s, boss %s, %d circles, seed %d"), *Place.Key.ToString(), *Place.Value.ToString(), Count, Seed);
			}
		}
	}
	AddInfo(FString::Printf(TEXT("%d patterns, %d of %d fives kept all five circles"), Patterns, FullFives, Fives));
	TestTrue(TEXT("the sweep covers the arena"), Patterns > 1000);
	TestEqual(FString::Printf(TEXT("every pattern leaves a spot to walk to in time (first failure: %s)"), *FirstFailure), Failures, 0);
	TestTrue(TEXT("and most fives stay whole: the guarantee is not bought by dropping circles"), FullFives * 10 >= Fives * 7);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamGapTest, "Jam.Slam.FiveCirclesKeepAGapBetweenThem", JamTest::Flags)

bool FJamSlamGapTest::RunTest(const FString& Parameters)
{
	// T-052: five circles that merge into one patch leave only the way out of the whole group. With a gap the hero
	// fits in, there are pockets to stand in between them.
	const FJamBalanceData B = SlamGameBalance();
	TestTrue(TEXT("the gap fits the hero with room on both sides"), B.SlamGap >= 2.f * B.HeroRadius + 40.f);
	int32 Patterns = 0;
	double Narrowest = TNumericLimits<double>::Max();
	FString FirstFailure;
	for (const TPair<FVector2D, FVector2D>& Place : SweepPlaces(B))
	{
		for (const int32 Seed : { 1, 2 })
		{
			const TArray<FJamHazard> Circles = MakePattern(Place.Value, Place.Key, 5, Seed, B);
			++Patterns;
			int32 First = 0;
			int32 Second = 0;
			const double Gap = NarrowestGap(Circles, First, Second);
			if (Gap < Narrowest)
			{
				Narrowest = Gap;
				FirstFailure = FString::Printf(TEXT("hero %s, boss %s, seed %d, circles %d and %d"), *Place.Key.ToString(),
					*Place.Value.ToString(), Seed, First, Second);
			}
		}
	}
	AddInfo(FString::Printf(TEXT("%d five-circle patterns, the narrowest gap %.0f cm (%s)"), Patterns, Narrowest, *FirstFailure));
	TestTrue(TEXT("the sweep covers the arena"), Patterns > 1000);
	TestTrue(
		FString::Printf(TEXT("no two circles come closer than the gap (narrowest at %s)"), *FirstFailure), Narrowest >= B.SlamGap - 0.5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSlamCashTest, "Jam.Slam.TheCashStaysClear", JamTest::Flags)

bool FJamSlamCashTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D Boss(400.0, 0.0);
	const FVector2D Hero = FVector2D::ZeroVector;
	// One stack where the cutter of the way back lands, one at the Publisher's feet.
	TArray<FJamPickup> Pickups;
	Pickups.AddDefaulted(2);
	Pickups[0].Pos = FVector2D(-B.SlamCutterDistance, 0.0);
	Pickups[1].Pos = Boss + FVector2D(0.0, 100.0);
	for (int32 Seed = 1; Seed <= 5; ++Seed)
	{
		const TArray<FJamHazard> Circles = MakePattern(Boss, Hero, 5, Seed, B, Pickups);
		TestTrue(TEXT("the pattern is still put down"), Circles.Num() >= 3);
		for (const FJamHazard& Circle : Circles)
		{
			for (const FJamPickup& Cash : Pickups)
			{
				TestTrue(TEXT("no circle covers a cash stack, with room to stand on it"),
					FVector2D::Distance(Circle.Pos, Cash.Pos) >= Circle.Radius + B.HealPickupRadius - 0.5);
			}
		}
		TestTrue(TEXT("and a safe spot is left"), JamSlamRules::SafeSpot(Circles, Hero, SlamTick, B.SlamReaction, B).IsSet());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossSlamGrowsTest, "Jam.Boss.SlamGrowsWithTheCuts", JamTest::Flags)

bool FJamBossSlamGrowsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	for (const int32 Cuts : { 0, 1, 2 })
	{
		FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(300.0, 0.0));
		F.Boss.BusyUntilTick = 0;
		F.Steal.Done = Cuts;
		FJamIntents Intents;
		Intents[EJamSide::Boss].Aim.Point = FVector2D::ZeroVector;
		Intents[EJamSide::Boss].bSlam = true;
		JamFightRules::Step(F, Intents, B);
		TestEqual(FString::Printf(TEXT("after %d cuts one slam puts down its whole pattern"), Cuts), F.Hazards.Num(),
			JamSlamRules::CircleCount(Cuts, B));
		int32 LastHit = 0;
		TSet<int32> Ids;
		for (const FJamHazard& Circle : F.Hazards)
		{
			LastHit = FMath::Max(LastHit, Circle.HitTick);
			Ids.Add(Circle.Id);
		}
		TestEqual(TEXT("each circle has its own id"), Ids.Num(), F.Hazards.Num());
		TestEqual(
			TEXT("he stands until the last circle lands, then his pause"), F.Boss.BusyUntilTick, LastHit + JamSim::Ticks(B.BossRecovery));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotPatternTest, "Jam.Bot.WalksOutOfEveryPattern", JamTest::Flags)

bool FJamBotPatternTest::RunTest(const FString& Parameters)
{
	// The game's numbers and no dash: the Publisher may have stolen it, and the walk alone has to do.
	const FJamBalanceData B = SlamGameBalance();
	const double Wall = B.ArenaRadius - B.HeroRadius;
	const TPair<FVector2D, FVector2D> Places[] = {
		{ FVector2D::ZeroVector, FVector2D(400.0, 0.0) },
		{ FVector2D(-Wall, 0.0), FVector2D(-Wall + 400.0, 0.0) },
		{ FVector2D(0.0, Wall - 200.0), FVector2D(0.0, Wall - 600.0) },
		{ FVector2D(800.0, 800.0), FVector2D(800.0, 500.0) },
	};
	for (const TPair<FVector2D, FVector2D>& Place : Places)
	{
		for (int32 Seed = 1; Seed <= 6; ++Seed)
		{
			FJamFight F = JamTest::Fight(B, Place.Key, Place.Value);
			JamTest::Bean(F);
			F.Tick = SlamTick;
			TArray<FJamHazard> Circles = MakePattern(Place.Value, Place.Key, 5, Seed, B);
			int32 Steps = 0;
			for (FJamHazard& Circle : Circles)
			{
				Circle.Id = F.NextId++;
				Steps = FMath::Max(Steps, Circle.HitTick - SlamTick + 1);
			}
			F.Hazards = Circles;
			for (int32 Step = 0; Step < Steps; ++Step)
			{
				FJamIntents Intents;
				Intents[EJamSide::Hero] = JamBotRules::Play(F, EJamSide::Hero, B);
				JamFightRules::Step(F, Intents, B);
			}
			TestEqual(FString::Printf(TEXT("the bot walked out of the pattern at %s, seed %d"), *Place.Key.ToString(), Seed),
				F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
		}
	}
	return true;
}

#endif
