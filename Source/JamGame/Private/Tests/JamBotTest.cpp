#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBotRules.h"
#include "Sim/JamParryRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-016: the bot that plays a fighter through the same intent as a person.
 * A bug here looks in clips and in full-fight tests like "the bot shoots past the boss" or "the bot hugs the boss and
 * never leaves".
 *
 * Run: Automation RunTests Jam.Bot
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotAimTest, "Jam.Bot.AimsAtTheOpponent", JamTest::Flags)

bool FJamBotAimTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamFight F = JamTest::Fight(B, FVector2D(-300.0, 0.0), FVector2D(0.0, 400.0));
	const FJamIntent Intent = JamBotRules::Kite(F, EJamSide::Hero, 500.f);
	TestTrue(TEXT("the aim points from the bot at the opponent"), Intent.Aim.Dir.Equals(FVector2D(0.6, 0.8), 1e-3));
	TestTrue(TEXT("the aim point is the opponent"), Intent.Aim.Point.Equals(FVector2D(0.0, 400.0), 1e-3));
	TestTrue(TEXT("it holds the primary button"), Intent.bPrimary);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotRangeTest, "Jam.Bot.KeepsItsRange", JamTest::Flags)

bool FJamBotRangeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D ToBoss(1.0, 0.0);
	const FJamFight Far = JamTest::Fight(B, FVector2D(-900.0, 0.0), FVector2D::ZeroVector);
	const FJamFight Near = JamTest::Fight(B, FVector2D(-200.0, 0.0), FVector2D::ZeroVector);
	const FJamIntent FarIntent = JamBotRules::Kite(Far, EJamSide::Hero, 500.f);
	const FJamIntent NearIntent = JamBotRules::Kite(Near, EJamSide::Hero, 500.f);
	TestTrue(TEXT("too far: it closes in"), FVector2D::DotProduct(FarIntent.Move, ToBoss) > 0.1);
	TestTrue(TEXT("too close: it backs off"), FVector2D::DotProduct(NearIntent.Move, ToBoss) < -0.1);
	TestTrue(TEXT("the move is never longer than a full tilt"), FarIntent.Move.Size() <= 1.0 + 1e-4);
	TestTrue(TEXT("it keeps moving sideways, a standing target is a dead one"), FMath::Abs(NearIntent.Move.Y) > 0.1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotFightTest, "Jam.Bot.WearsDownAStandingBoss", JamTest::Flags)

bool FJamBotFightTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const float Range = 500.f;
	FJamFight F = JamTest::Fight(B, FVector2D(-800.0, 0.0), FVector2D::ZeroVector);
	JamTest::Bean(F);
	FJamIntents Intents;
	// Ten seconds of the bean bot against a boss that only stands.
	double FarthestSide = 0.0;
	for (int32 i = 0; i < 600; ++i)
	{
		Intents[EJamSide::Hero] = JamBotRules::Kite(F, EJamSide::Hero, Range);
		JamFightRules::Step(F, Intents, B);
		FarthestSide = FMath::Max(FarthestSide, FMath::Abs(F.Fighters[EJamSide::Hero].Pos.Y));
	}
	const FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	const FJamFighter& Boss = F.Fighters[EJamSide::Boss];
	const double Gap = FVector2D::Distance(Hero.Pos, Boss.Pos);
	TestTrue(TEXT("most balls land: the boss lost a good part of the 20 shots"), Boss.Health <= Boss.MaxHealth - 10.f * B.BallDamage);
	TestTrue(TEXT("the bot settles near its range"), FMath::Abs(Gap - Range) < 0.3 * Range);
	TestTrue(TEXT("the bot went around the boss, it did not stand"), FarthestSide > 0.5 * Range);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotCircleTest, "Jam.Bot.LeavesTheSlamCircle", JamTest::Flags)

bool FJamBotCircleTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(50.0, 0.0), FVector2D(600.0, 0.0));
	FJamHazard Circle;
	Circle.Owner = EJamSide::Boss;
	Circle.Radius = 250.f;
	Circle.HitTick = 60;
	F.Hazards.Add(Circle);
	const FJamIntent Inside = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("inside the circle the bot runs straight out of it"), Inside.Move.Equals(FVector2D(1.0, 0.0), 1e-3));
	TestTrue(TEXT("and dashes if it can"), Inside.bDash);

	F.Fighters[EJamSide::Hero].Pos = FVector2D::ZeroVector;
	const FJamIntent Centre = JamBotRules::Play(F, EJamSide::Hero, B);
	TestEqual(TEXT("in the very centre it still picks a way out"), static_cast<float>(Centre.Move.Size()), 1.f, 1e-3f);

	// Ten steps before the hit, from the centre: the walk alone is too slow, the dash makes it.
	F.Tick = 50;
	for (int32 i = 0; i < 12; ++i)
	{
		FJamIntents Intents;
		Intents[EJamSide::Hero] = JamBotRules::Play(F, EJamSide::Hero, B);
		JamFightRules::Step(F, Intents, B);
	}
	TestEqual(TEXT("the bot got out in time"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotDiveTest, "Jam.Bot.DivesInWhileTheBossRecovers", JamTest::Flags)

bool FJamBotDiveTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D ToBoss(1.0, 0.0);
	FJamFight F = JamTest::Fight(B, FVector2D(-700.0, 0.0), FVector2D::ZeroVector);
	F.Boss.BusyUntilTick = 100;
	const FJamIntent Far = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("the boss is recovering: the bot runs at him"), FVector2D::DotProduct(Far.Move, ToBoss) > 0.9);
	TestFalse(TEXT("it does not swing the katana at the air"), Far.bPrimary);
	TestTrue(TEXT("it throws shurikens on the way"), Far.bSecondary);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-250.0, 0.0);
	const FJamIntent Close = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("in reach it swings"), Close.bPrimary);

	F.Boss.BusyUntilTick = 0;
	const FJamIntent Awake = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("the boss is awake again: the bot backs off"), FVector2D::DotProduct(Awake.Move, ToBoss) < -0.1);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-700.0, 0.0);
	JamTest::Bean(F);
	const FJamIntent Bean = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("with the katana stolen the same button shoots the ball from afar"), Bean.bPrimary);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotNoDashTest, "Jam.Bot.KeepsAwayWithoutTheDash", JamTest::Flags)

bool FJamBotNoDashTest::RunTest(const FString& Parameters)
{
	// Next to the boss a volley cannot be walked around: the dash is the way through it, and without it the dive is a trap.
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D ToBoss(1.0, 0.0);
	FJamFight F = JamTest::Fight(B, FVector2D(-700.0, 0.0), FVector2D::ZeroVector);
	F.Boss.BusyUntilTick = 100;
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Dash, EJamSide::Boss);
	const FJamIntent Far = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("the boss recovers, but the dash is gone: the bot does not run at him"), FVector2D::DotProduct(Far.Move, ToBoss) < 0.9);
	TestTrue(TEXT("it keeps circling"), FMath::Abs(Far.Move.Y) > 0.1);
	TestTrue(TEXT("and throws from where it is"), Far.bSecondary);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-250.0, 0.0);
	const FJamIntent Close = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("a boss who came into reach still gets the katana"), Close.bPrimary);
	TestTrue(TEXT("while the bot backs off"), FVector2D::DotProduct(Close.Move, ToBoss) < -0.1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotDodgeTest, "Jam.Bot.DashesThroughAMemo", JamTest::Flags)

bool FJamBotDodgeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(700.0, 0.0));
	F.Boss.BusyUntilTick = 0;
	// Without the parry: with it in hand a lone memo is parried (Jam.Bot.ParriesAMemoBeforeDashing).
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	TestFalse(TEXT("nothing flies at the bot: it keeps the dash"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);

	FJamProjectile Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Pos = Memo.PrevPos = FVector2D(120.0, 0.0);
	Memo.Vel = FVector2D(-600.0, 0.0);
	Memo.Radius = B.MemoRadius;
	Memo.DieTick = 600;
	F.Projectiles.Add(Memo);
	TestTrue(TEXT("a memo about to hit: the bot dashes"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);

	F.Projectiles[0].Vel = FVector2D(600.0, 0.0);
	TestFalse(TEXT("a memo flying away is no threat"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);

	F.Projectiles[0].Vel = FVector2D(-600.0, 0.0);
	F.Projectiles[0].Pos = FVector2D(120.0, 300.0);
	TestFalse(TEXT("a memo that passes beside is no threat"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);

	F.Projectiles[0].Pos = FVector2D(120.0, 0.0);
	F.Projectiles[0].Owner = EJamSide::Hero;
	TestFalse(TEXT("its own projectile is no threat"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotParryTest, "Jam.Bot.ParriesAMemoBeforeDashing", JamTest::Flags)

bool FJamBotParryTest::RunTest(const FString& Parameters)
{
	// T-019: the parry sends the memo back and keeps the dash for the slam. T-044: the hero parries with his strike.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(700.0, 0.0));
	F.Boss.BusyUntilTick = 0;
	FJamProjectile Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Pos = Memo.PrevPos = FVector2D(120.0, 0.0);
	Memo.Vel = FVector2D(-600.0, 0.0);
	Memo.Radius = B.MemoRadius;
	Memo.DieTick = 600;
	F.Projectiles.Add(Memo);
	const FJamIntent Ready = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(
		TEXT("a memo about to hit: the bot strikes, which parries"), Ready.bPrimary && JamParryRules::ShotInReach(F, EJamSide::Hero, B));
	TestFalse(TEXT("there is no parry key for the hero"), Ready.bParry);
	TestFalse(TEXT("and keeps the dash"), Ready.bDash);

	F.Fighters[EJamSide::Hero].Parry.ReadyTick = F.Tick + 30;
	const FJamIntent Cooling = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("the parry is cooling down: the bot dashes"), Cooling.bDash);

	F.Fighters[EJamSide::Hero].Parry.ReadyTick = 0;
	FJamProjectile Second = F.Projectiles[0];
	Second.Pos = Second.PrevPos = FVector2D(110.0, 15.0);
	F.Projectiles.Add(Second);
	const FJamIntent Fan = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("two at once: the bot dashes through"), Fan.bDash && !Fan.bParry);
	F.Projectiles.SetNum(1);

	F.Fighters[EJamSide::Hero].Parry.ReadyTick = 0;
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	TestTrue(TEXT("without the parry the bot dashes"), JamBotRules::Play(F, EJamSide::Hero, B).bDash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotUltimateTest, "Jam.Bot.SlashesWhenTheBossIsInReach", JamTest::Flags)

bool FJamBotUltimateTest::RunTest(const FString& Parameters)
{
	// T-020: a full meter is spent on a slash that lands, never on the air.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(700.0, 0.0));
	TestFalse(TEXT("an empty meter: no slash"), JamBotRules::Play(F, EJamSide::Hero, B).bUltimate);
	F.Fighters[EJamSide::Hero].Ultimate.Meter = 1.f;
	TestTrue(TEXT("full, the boss in reach: slash"), JamBotRules::Play(F, EJamSide::Hero, B).bUltimate);
	F.Fighters[EJamSide::Boss].Pos = FVector2D(B.UltimateRange + 400.0, 0.0);
	TestFalse(TEXT("full, the boss out of reach: wait"), JamBotRules::Play(F, EJamSide::Hero, B).bUltimate);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotTurnTest, "Jam.Bot.TurnsItsCircleBeforeAMemo", JamTest::Flags)

bool FJamBotTurnTest::RunTest(const FString& Parameters)
{
	// T-033: the Publisher throws where the hero will be. A bug here looks in clips like "the bot runs into every volley".
	const FJamBalanceData B = JamTest::Balance();
	// The boss is exactly at the range the bot holds: its way is the circle and nothing else.
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1.5 * B.SlamRange, 0.0));
	F.Boss.BusyUntilTick = 0;
	FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	TestTrue(TEXT("from a standstill the bot circles to its left"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);

	Hero.PrevPos = FVector2D(0.0, 10.0);
	TestTrue(TEXT("a bot that circles to its right keeps doing so"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y < -0.1);

	// A memo that will be where the bot is going: the two close in on each other.
	Hero.PrevPos = FVector2D(0.0, -10.0);
	FJamProjectile Memo;
	Memo.Owner = EJamSide::Boss;
	Memo.Pos = Memo.PrevPos = FVector2D(0.0, 700.0);
	Memo.Vel = FVector2D(0.0, -600.0);
	Memo.Radius = B.MemoRadius;
	Memo.DieTick = 600;
	F.Projectiles.Add(Memo);
	const FJamIntent Turned = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("a memo on its course: the bot turns its circle"), Turned.Move.Y < -0.1);
	TestFalse(TEXT("the memo is still far: the dash is kept"), Turned.bDash);

	F.Projectiles[0].Pos = FVector2D(0.0, -700.0);
	TestTrue(TEXT("a memo that flies away behind it changes nothing"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);

	// Memos on both ways: turning gains nothing, the dash will have to do.
	F.Projectiles[0].Pos = FVector2D(0.0, 700.0);
	Memo.Pos = Memo.PrevPos = FVector2D(0.0, -700.0);
	Memo.Vel = FVector2D(0.0, 600.0);
	F.Projectiles.Add(Memo);
	TestTrue(TEXT("with both ways shut the bot keeps its course"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);

	F.Projectiles.SetNum(1);
	F.Projectiles[0].Owner = EJamSide::Hero;
	TestTrue(TEXT("its own projectile is nothing to turn from"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotWallTest, "Jam.Bot.KeepsOffTheWall", JamTest::Flags)

bool FJamBotWallTest::RunTest(const FString& Parameters)
{
	// T-033: what the Publisher throws past the hero ends on the wall, and an explosive shuriken goes off there. A bug
	// here looks in clips like "the bot hugs the wall and every blast behind it lands".
	const FJamBalanceData B = JamTest::Balance();
	const FVector2D Outward(-1.0, 0.0);
	const FVector2D AtTheWall = Outward * (B.ArenaRadius - B.HeroRadius);
	// The boss is closer than the bot likes: it wants to back off, and the wall is right behind it.
	FJamFight F = JamTest::Fight(B, AtTheWall, AtTheWall - Outward * B.SlamRange);
	F.Boss.BusyUntilTick = 0;
	const FJamIntent Pinned = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("at the wall the bot turns back into the open"), FVector2D::DotProduct(Pinned.Move, Outward) < -0.1);
	TestTrue(TEXT("around the boss, not through him"), FMath::Abs(Pinned.Move.Y) > 0.1);
	TestTrue(TEXT("at full speed"), FMath::IsNearlyEqual(Pinned.Move.Size(), 1.0, 0.001));

	FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	Hero.Pos = Hero.PrevPos = Outward * 100.0;
	F.Fighters[EJamSide::Boss].Pos = Hero.Pos - Outward * B.SlamRange;
	const FJamIntent Free = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("in the open it backs off from him as before"), FVector2D::DotProduct(Free.Move, Outward) > 0.1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotBlastTest, "Jam.Bot.KeepsOutOfABlast", JamTest::Flags)

bool FJamBotBlastTest::RunTest(const FString& Parameters)
{
	// T-033: an explosive shuriken hurts where it ends, not only where it flies. A bug here looks in clips like "the bot
	// turns away from a memo straight into an explosion".
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(1.5 * B.SlamRange, 0.0));
	F.Boss.BusyUntilTick = 0;
	// It passes the bot's way too far ahead to hit, and its life ends half a second from now, right next to that way.
	FJamProjectile Shuriken;
	Shuriken.Owner = EJamSide::Boss;
	Shuriken.Pos = Shuriken.PrevPos = FVector2D(450.0, 300.0);
	Shuriken.Vel = FVector2D(-600.0, 0.0);
	Shuriken.Radius = B.ShurikenRadius;
	Shuriken.DieTick = F.Tick + JamSim::Ticks(0.5f);
	F.Projectiles.Add(Shuriken);
	TestTrue(TEXT("a plain shuriken that will miss changes nothing"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);

	F.Projectiles[0].bExplosive = true;
	TestTrue(TEXT("an explosive one that ends on the bot's way turns it"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y < -0.1);

	F.Projectiles[0].DieTick = F.Tick + JamSim::Ticks(0.1f);
	TestTrue(TEXT("one that goes off before the bot is near changes nothing"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y > 0.1);

	// The wall ends a shuriken too: this one would fly on for seconds, but the wall is where the bot is going.
	FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	Hero.Pos = Hero.PrevPos = FVector2D(0.0, B.ArenaRadius - 350.0);
	F.Fighters[EJamSide::Boss].Pos = Hero.Pos + FVector2D(1.5 * B.SlamRange, 0.0);
	F.Projectiles[0].Pos = FVector2D(150.0, B.ArenaRadius - 600.0);
	F.Projectiles[0].Vel = FVector2D(0.0, 1200.0);
	F.Projectiles[0].DieTick = F.Tick + JamSim::Ticks(5.f);
	TestTrue(TEXT("a blast at the wall ahead turns the bot as well"), JamBotRules::Play(F, EJamSide::Hero, B).Move.Y < -0.1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotDuelTest, "Jam.Bot.HoldsItsOwnAgainstThePublisher", JamTest::Flags)

bool FJamBotDuelTest::RunTest(const FString& Parameters)
{
	// A minute of the whole block B fight: the bot with all three mechanics against the boss's brain.
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamFightRules::Start(B, 3);
	for (int32 i = 0; i < 60 * JamSim::StepsPerSecond; ++i)
	{
		JamFightRules::Play(F, JamBotRules::Play(F, EJamSide::Hero, B), B);
	}
	const FJamFighter& Hero = F.Fighters[EJamSide::Hero];
	const FJamFighter& Boss = F.Fighters[EJamSide::Boss];
	AddInfo(FString::Printf(
		TEXT("after a minute: hero %.0f of %.0f, boss %.0f of %.0f"), Hero.Health, Hero.MaxHealth, Boss.Health, Boss.MaxHealth));
	TestTrue(TEXT("the bot is alive"), F.Phase != EJamPhase::HeroDead);
	TestTrue(TEXT("the boss lost a good part of his health"), Boss.Health <= Boss.MaxHealth - 200.f);
	TestTrue(
		TEXT("and the boss did land something: the fight is not a walkover"), Hero.Health < Hero.MaxHealth || F.Phase == EJamPhase::Won);
	return true;
}

#endif
