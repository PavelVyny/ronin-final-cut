#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamHealRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-049: the author's playtest: every so often the Publisher drops a heal, a cash stack the hero picks up. It is
 * knocked out of his pocket by the first hit once a drop is due and lands between him and the hero. A bug looks like
 * "it rains health", "it fell out of the arena", "I walked over it and nothing happened" or "it healed me past full".
 *
 * Run: Automation RunTests Jam.Heal
 */
namespace
{
/** The hero to the west, the Publisher to the east: the cash flies west. */
FJamFight Facing(const FJamBalanceData& B)
{
	return JamTest::Fight(B, FVector2D(-400.0, 0.0), FVector2D(300.0, 0.0));
}

void HitTheBoss(FJamFight& F)
{
	F.Events.Reset();
	JamFightOps::Hurt(F, EJamSide::Boss, 10.f, F.Fighters[EJamSide::Boss].Pos);
}

/** Puts a landed cash stack under the hero's feet. */
FJamPickup& DropOnTheHero(FJamFight& F, const FJamBalanceData& B)
{
	F.Tick = F.NextHealTick;
	HitTheBoss(F);
	JamHealRules::Drop(F, B);
	FJamPickup& Cash = F.Pickups[0];
	F.Tick = Cash.LandTick;
	F.Fighters[EJamSide::Hero].Pos = Cash.Pos;
	F.Events.Reset();
	return Cash;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealDropTest, "Jam.Heal.AHitKnocksOutTheCashOnceItIsDue", JamTest::Flags)

bool FJamHealDropTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = Facing(B);
	TestEqual(TEXT("the first drop is due one interval in"), F.NextHealTick, JamSim::Ticks(B.HealDropInterval));
	HitTheBoss(F);
	JamHealRules::Drop(F, B);
	TestEqual(TEXT("not due yet: no cash"), F.Pickups.Num(), 0);

	F.Tick = F.NextHealTick;
	F.Events.Reset();
	JamHealRules::Drop(F, B);
	TestEqual(TEXT("due, but no hit: no cash"), F.Pickups.Num(), 0);
	JamFightOps::Hurt(F, EJamSide::Hero, 10.f, F.Fighters[EJamSide::Hero].Pos);
	JamHealRules::Drop(F, B);
	TestEqual(TEXT("the hero hurt is no hit on him"), F.Pickups.Num(), 0);

	HitTheBoss(F);
	JamHealRules::Drop(F, B);
	if (!TestEqual(TEXT("due and hit: one stack"), F.Pickups.Num(), 1))
	{
		return true;
	}
	const FJamPickup& Cash = F.Pickups[0];
	const FJamFighter& Boss = F.Fighters[EJamSide::Boss];
	TestTrue(TEXT("it jumps out of his body: from his edge towards where it lands"),
		Cash.From.Equals(Boss.Pos + (Cash.Pos - Boss.Pos).GetSafeNormal() * Boss.Radius, 0.1));
	TestTrue(TEXT("towards the hero, the toss away"), Cash.Pos.Equals(FVector2D(300.0 - B.HealToss, 0.0), 0.1));
	TestEqual(TEXT("it lands after the toss"), Cash.LandTick, F.Tick + JamSim::Ticks(B.HealTossSeconds));
	TestEqual(TEXT("and lies its life"), Cash.DieTick, Cash.LandTick + JamSim::Ticks(B.HealLife));
	TestEqual(TEXT("the next one is due an interval later"), F.NextHealTick, F.Tick + JamSim::Ticks(B.HealDropInterval));
	TestEqual(TEXT("the drop is told"), JamTest::Count(F, EJamEvent::HealDropped), 1);

	F.Tick = F.NextHealTick;
	HitTheBoss(F);
	JamHealRules::Drop(F, B);
	TestEqual(TEXT("never more than the cap on the floor"), F.Pickups.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealClearTest, "Jam.Heal.TheCashLandsClearOfHim", JamTest::Flags)

bool FJamHealClearTest::RunTest(const FString& Parameters)
{
	// T-054: the cash never shows up inside the Publisher: it leaves his body and lands where it can be seen and taken.
	const FJamBalanceData B = JamTest::Balance();
	const double Wall = B.ArenaRadius - B.BossRadius;
	const FVector2D Bosses[] = { FVector2D::ZeroVector, FVector2D(Wall, 0.0), FVector2D(0.0, -Wall), FVector2D(0.7 * Wall, 0.7 * Wall) };
	for (const FVector2D& BossPos : Bosses)
	{
		for (int32 Angle = 0; Angle < 360; Angle += 30)
		{
			const FVector2D HeroPos = BossPos + FVector2D(B.BossRadius + B.HeroRadius + 20.0, 0.0).GetRotated(static_cast<float>(Angle));
			FJamFight F = JamTest::Fight(B, HeroPos, BossPos);
			F.Tick = F.NextHealTick;
			HitTheBoss(F);
			JamHealRules::Drop(F, B);
			if (F.Pickups.Num() != 1)
			{
				AddError(FString::Printf(TEXT("boss %s, angle %d: no stack"), *BossPos.ToString(), Angle));
				return false;
			}
			const FJamPickup& Cash = F.Pickups[0];
			const double FromCentre = FVector2D::Distance(Cash.From, BossPos);
			const double LandCentre = FVector2D::Distance(Cash.Pos, BossPos);
			if (!FMath::IsNearlyEqual(FromCentre, static_cast<double>(B.BossRadius), 0.1) || LandCentre < B.BossRadius + B.HealPickupRadius)
			{
				AddError(FString::Printf(
					TEXT("boss %s, angle %d: from %.0f, lands %.0f from his centre"), *BossPos.ToString(), Angle, FromCentre, LandCentre));
				return false;
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealTakeTest, "Jam.Heal.TheHeroHealsOnTouch", JamTest::Flags)

bool FJamHealTakeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = Facing(B);
	FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	DropOnTheHero(F, B);
	Hero.Health = 50.f;
	F.Tick -= 1;
	JamHealRules::Collect(F, B);
	TestEqual(TEXT("in the air it cannot be caught"), Hero.Health, 50.f);
	F.Tick += 1;
	JamHealRules::Collect(F, B);
	TestEqual(TEXT("landed: it heals"), Hero.Health, 50.f + B.HealAmount);
	TestEqual(TEXT("and is gone"), F.Pickups.Num(), 0);
	TestEqual(TEXT("the heal is told"), JamTest::Count(F, EJamEvent::HealTaken), 1);

	FJamFight Near = Facing(B);
	DropOnTheHero(Near, B);
	Near.Fighters[EJamSide::Hero].Health = B.HeroHealth - 5.f;
	JamHealRules::Collect(Near, B);
	TestEqual(TEXT("never past full"), Near.Fighters[EJamSide::Hero].Health, B.HeroHealth);

	FJamFight Whole = Facing(B);
	DropOnTheHero(Whole, B);
	JamHealRules::Collect(Whole, B);
	TestEqual(TEXT("a whole hero leaves it for later"), Whole.Pickups.Num(), 1);

	FJamFight Far = Facing(B);
	DropOnTheHero(Far, B);
	Far.Fighters[EJamSide::Hero].Health = 50.f;
	Far.Fighters[EJamSide::Hero].Pos += FVector2D(0.0, B.HeroRadius + B.HealPickupRadius + 10.0);
	JamHealRules::Collect(Far, B);
	TestEqual(TEXT("out of reach: nothing"), Far.Fighters[EJamSide::Hero].Health, 50.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHealFadeTest, "Jam.Heal.TheCashFadesAndStaysInTheArena", JamTest::Flags)

bool FJamHealFadeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = Facing(B);
	FJamPickup& Cash = DropOnTheHero(F, B);
	F.Fighters[EJamSide::Hero].Pos = FVector2D(-800.0, 0.0);
	F.Tick = Cash.DieTick;
	JamHealRules::Collect(F, B);
	TestEqual(TEXT("its life over: gone"), F.Pickups.Num(), 0);

	// The Publisher at the east wall, the hero beside him to the north-east: the toss would leave the arena.
	const double Edge = B.ArenaRadius - B.BossRadius;
	FJamFight Wall = JamTest::Fight(B, FVector2D(Edge, 300.0), FVector2D(Edge, 0.0));
	Wall.Tick = Wall.NextHealTick;
	HitTheBoss(Wall);
	JamHealRules::Drop(Wall, B);
	if (TestEqual(TEXT("a stack"), Wall.Pickups.Num(), 1))
	{
		TestTrue(TEXT("inside the arena"), Wall.Pickups[0].Pos.Size() <= B.ArenaRadius - B.HealPickupRadius + 0.5);
	}
	return true;
}

#endif
