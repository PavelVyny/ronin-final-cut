#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBossRules.h"
#include "Sim/JamBotRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamParryRules.h"
#include "Sim/JamUltimateRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-038: the Publisher uses the Parry and the Ultimate he stole. The Parry is a shield stance: a raise that warns, then
 * a long hold that sends the hero's shots back. The Ultimate comes on a timer, winds up long with the line locked, and
 * cuts for his own damage.
 * A bug here looks in game like "he took my parry and never used it", "his shield sends back a shot that hit him before
 * it was up", "his Ultimate one-shots me", "his line follows me round" or "he slashes with an Ultimate he never took".
 *
 * Run: Automation RunTests Jam.Boss / Jam.Bot
 */
namespace
{
/** A fight in which the boss holds one stolen mechanic, may act from the first step and has his own attacks put away. */
FJamFight StanceFight(const FJamBalanceData& B, const FVector2D& HeroPos, EJamMechanic Stolen)
{
	FJamFight F = JamTest::Fight(B, HeroPos, FVector2D::ZeroVector);
	JamLoadoutRules::Give(F.Ledger, Stolen, EJamSide::Boss);
	F.Boss.BusyUntilTick = 0;
	F.Boss.SlamReadyTick = F.Boss.VolleyReadyTick = 1000000;
	return F;
}

/** A hero's ball at Pos flying along Vel. */
FJamProjectile HeroBall(const FJamBalanceData& B, const FVector2D& Pos, const FVector2D& Vel)
{
	FJamProjectile Ball;
	Ball.Id = 99;
	Ball.Owner = EJamSide::Hero;
	Ball.Pos = Ball.PrevPos = Pos;
	Ball.Vel = Vel;
	Ball.Radius = B.BallRadius;
	Ball.Damage = B.BallDamage;
	Ball.DieTick = 1000000;
	return Ball;
}

/** A ball one step away from the boss's body at the origin, coming from the hero's side. */
FJamProjectile BallAboutToHitHim(const FJamBalanceData& B)
{
	return HeroBall(B, FVector2D(-115.0, 0.0), FVector2D(1200.0, 0.0));
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossRaisesShieldTest, "Jam.Boss.RaisesTheShieldAtIncomingShots", JamTest::Flags)

bool FJamBossRaisesShieldTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Parry);
	TestFalse(TEXT("nothing flies at him: no shield"), JamBossRules::Decide(F, B).bParry);

	F.Projectiles.Add(HeroBall(B, FVector2D(-600.0, 0.0), FVector2D(1200.0, 0.0)));
	const FJamIntent Incoming = JamBossRules::Decide(F, B);
	TestTrue(TEXT("a hero's shot flies at him: he raises the shield"), Incoming.bParry);

	FJamFight Away = F;
	Away.Projectiles[0].Vel = FVector2D(-1200.0, 0.0);
	TestFalse(TEXT("a shot flying away is no reason"), JamBossRules::Decide(Away, B).bParry);

	FJamFight Slow = F;
	Slow.Projectiles[0].Vel = FVector2D(150.0, 0.0);
	TestFalse(TEXT("a shot that arrives after the whole stance is not waited for"), JamBossRules::Decide(Slow, B).bParry);

	FJamFight Own = F;
	Own.Projectiles[0].Owner = EJamSide::Boss;
	TestFalse(TEXT("his own memo is never one to parry"), JamBossRules::Decide(Own, B).bParry);

	FJamFight Cooling = F;
	Cooling.Fighters[EJamSide::Boss].Parry.ReadyTick = Cooling.Tick + 10;
	TestFalse(TEXT("not before his cooldown is over"), JamBossRules::Decide(Cooling, B).bParry);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossNoParryTest, "Jam.Boss.NeverParriesWithoutTheMechanic", JamTest::Flags)

bool FJamBossNoParryTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Shuriken);
	F.Projectiles.Add(HeroBall(B, FVector2D(-600.0, 0.0), FVector2D(1200.0, 0.0)));
	TestFalse(TEXT("the hero still has the Parry: the brain does not ask for it"), JamBossRules::Decide(F, B).bParry);

	FJamIntents Intents;
	Intents[EJamSide::Boss].bParry = true;
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("and the button does nothing in his hands"), JamTest::Count(F, EJamEvent::ParryStarted), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossShieldTest, "Jam.Boss.ShieldSendsShotsBack", JamTest::Flags)

bool FJamBossShieldTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Parry);
	FJamIntents Raise;
	Raise[EJamSide::Boss].bParry = true;
	JamFightRules::Step(F, Raise, B);
	TestEqual(TEXT("the button raises his shield"), JamTest::Count(F, EJamEvent::ParryStarted), 1);
	TestTrue(TEXT("he stands while he raises it"), JamBossRules::IsBusy(F));
	TestTrue(TEXT("the raise shows"), JamParryRules::IsRaising(F.Fighters[EJamSide::Boss].Parry, F.Tick));
	TestFalse(TEXT("but it is not a shield yet"), JamParryRules::IsActive(F.Fighters[EJamSide::Boss].Parry, F.Tick));

	FJamFight Early = F;
	Early.Projectiles.Add(BallAboutToHitHim(B));
	JamFightRules::Step(Early, FJamIntents(), B);
	TestEqual(TEXT("a shot that comes during the raise hurts him"), JamTest::Count(Early, EJamEvent::Hit), 1);
	TestEqual(TEXT("it is not sent back"), JamTest::Count(Early, EJamEvent::Deflected), 0);

	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.BossParryRaise) - 1);
	TestTrue(TEXT("raised: now it is a shield"), JamParryRules::IsActive(F.Fighters[EJamSide::Boss].Parry, F.Tick));
	TestTrue(TEXT("and he still stands behind it"), JamBossRules::IsBusy(F));
	FJamFight Held = F;
	Held.Projectiles.Add(BallAboutToHitHim(B));
	JamFightRules::Step(Held, FJamIntents(), B);
	TestEqual(TEXT("a shot that comes now is sent back"), JamTest::Count(Held, EJamEvent::Deflected), 1);
	TestEqual(TEXT("and does not hurt him"), JamTest::Count(Held, EJamEvent::Hit), 0);
	if (TestEqual(TEXT("it flies on"), Held.Projectiles.Num(), 1))
	{
		const FJamProjectile& Back = Held.Projectiles[0];
		TestEqual(TEXT("it is his now"), Back.Owner, EJamSide::Boss);
		TestTrue(TEXT("flying at the hero"), Back.Vel.GetSafeNormal().Equals(FVector2D(-1.0, 0.0), 1e-3));
		TestEqual(
			TEXT("at the return speed in his hands"), static_cast<float>(Back.Vel.Size()), B.ParryReturnSpeed * B.BossStolenSpeed, 0.1f);
		TestEqual(TEXT("with his own scale of the damage"), Back.Damage, B.BallDamage * B.BossParryDamageScale);
	}

	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.BossParryHold));
	TestFalse(TEXT("the hold is over: the shield is down"), JamParryRules::IsActive(F.Fighters[EJamSide::Boss].Parry, F.Tick));
	TestFalse(TEXT("and he moves again"), JamBossRules::IsBusy(F));
	TestFalse(TEXT("but cannot raise it again before his cooldown"), JamParryRules::CanStart(F.Fighters[EJamSide::Boss].Parry, F.Tick));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossUltimateTimerTest, "Jam.Boss.UltimateOnItsInterval", JamTest::Flags)

bool FJamBossUltimateTimerTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const int32 Interval = JamSim::Ticks(B.BossUltimateInterval);
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Ultimate);
	JamTest::Run(F, FJamIntents(), B, Interval - 1);
	TestTrue(TEXT("a step before the interval his meter is not full"), F.Fighters[EJamSide::Boss].Ultimate.Meter < 1.f);
	JamFightRules::Step(F, FJamIntents(), B);
	TestEqual(TEXT("on the interval it is full"), F.Fighters[EJamSide::Boss].Ultimate.Meter, 1.f);
	TestEqual(TEXT("and that is told"), JamTest::Count(F, EJamEvent::UltimateReady), 1);

	FJamFight NotHis = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Shuriken);
	JamTest::Run(NotHis, FJamIntents(), B, Interval);
	TestEqual(TEXT("without the mechanic the clock does not fill his meter"), NotHis.Fighters[EJamSide::Boss].Ultimate.Meter, 0.f);

	FJamFight Hits = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Ultimate);
	Hits.Events.Add({ EJamEvent::Hit, EJamSide::Hero, 0, FVector2D::ZeroVector, 50.f });
	Hits.Events.Add({ EJamEvent::Hit, EJamSide::Boss, 0, FVector2D::ZeroVector, 20.f });
	JamUltimateRules::Charge(Hits, B);
	TestEqual(
		TEXT("what he deals does not fill his meter, only the clock"), Hits.Fighters[EJamSide::Boss].Ultimate.Meter, 1.f / Interval, 1e-5f);
	TestEqual(
		TEXT("the hero's hits still fill the hero's"), Hits.Fighters[EJamSide::Hero].Ultimate.Meter, 20.f * B.UltimateGainPerDamage, 1e-5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossUltimateBrainTest, "Jam.Boss.SlashesWhenHisMeterIsFull", JamTest::Flags)

bool FJamBossUltimateBrainTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Ultimate);
	F.Fighters[EJamSide::Boss].Ultimate.Meter = 0.5f;
	TestFalse(TEXT("half a meter: no slash"), JamBossRules::Decide(F, B).bUltimate);
	F.Fighters[EJamSide::Boss].Ultimate.Meter = 1.f;
	const FJamIntent Full = JamBossRules::Decide(F, B);
	TestTrue(TEXT("full and the hero within the line's reach: he slashes"), Full.bUltimate);
	TestTrue(TEXT("at the hero"), Full.Aim.Dir.Equals(FVector2D(-1.0, 0.0), 1e-3));

	FJamFight Far = F;
	Far.Fighters[EJamSide::Boss].Pos = Far.Fighters[EJamSide::Boss].PrevPos = FVector2D(900.0, 0.0);
	TestFalse(TEXT("out of the line's reach he does not waste it"), JamBossRules::Decide(Far, B).bUltimate);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBossUltimateLockedTest, "Jam.Boss.UltimateLineIsLocked", JamTest::Flags)

bool FJamBossUltimateLockedTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Ultimate);
	F.Fighters[EJamSide::Boss].Ultimate.Meter = 1.f;
	FJamIntents Press;
	Press[EJamSide::Boss].Aim.Dir = FVector2D(-1.0, 0.0);
	Press[EJamSide::Boss].bUltimate = true;
	JamFightRules::Step(F, Press, B);
	const FJamUltimateState& Ultimate = F.Fighters[EJamSide::Boss].Ultimate;
	TestTrue(TEXT("the press starts his wind-up"), Ultimate.bPending);
	TestEqual(TEXT("his wind-up is his own, long one"), Ultimate.SlashTick - (F.Tick - 1), JamSim::Ticks(B.BossUltimateWindUp));
	TestTrue(TEXT("he stands through it"), JamBossRules::IsBusy(F));

	FJamIntents Turn;
	Turn[EJamSide::Boss].Aim.Dir = FVector2D(0.0, 1.0);
	const float HealthBefore = F.Fighters[EJamSide::Hero].Health;
	int32 Steps = 0;
	while (F.Fighters[EJamSide::Boss].Ultimate.bPending && Steps++ < 200)
	{
		TestTrue(
			TEXT("his line keeps the way it was pressed in"), F.Fighters[EJamSide::Boss].Ultimate.Dir.Equals(FVector2D(-1.0, 0.0), 1e-6));
		JamFightRules::Step(F, Turn, B);
	}
	TestEqual(TEXT("the hero who stayed on the line takes his damage, not the hero's"), F.Fighters[EJamSide::Hero].Health,
		HealthBefore - B.BossUltimateDamage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotLaneTest, "Jam.Bot.StepsOffHisUltimateLine", JamTest::Flags)

bool FJamBotLaneTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-700.0, 0.0), EJamMechanic::Ultimate);
	FJamUltimateState& Ultimate = F.Fighters[EJamSide::Boss].Ultimate;
	Ultimate.bPending = true;
	Ultimate.Dir = FVector2D(-1.0, 0.0);
	Ultimate.SlashTick = F.Tick + JamSim::Ticks(B.BossUltimateWindUp);
	const FJamIntent Dodge = JamBotRules::Play(F, EJamSide::Hero, B);
	TestTrue(TEXT("on his locked line the bot walks off it sideways"), FMath::Abs(Dodge.Move.Y) > 0.9);

	FJamFight Aside = F;
	Aside.Fighters[EJamSide::Hero].Pos = Aside.Fighters[EJamSide::Hero].PrevPos = FVector2D(-700.0, 400.0);
	const FJamIntent Free = JamBotRules::Play(Aside, EJamSide::Hero, B);
	TestFalse(TEXT("well off the line it plays on: no dash for it"), Free.bDash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotShieldTest, "Jam.Bot.HoldsFireIntoHisShield", JamTest::Flags)

bool FJamBotShieldTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = StanceFight(B, FVector2D(-700.0, 0.0), EJamMechanic::Parry);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Katana, EJamSide::Boss);
	TestTrue(TEXT("shield down: the bot shoots"), JamBotRules::Play(F, EJamSide::Hero, B).bSecondary);

	F.Fighters[EJamSide::Boss].Parry = JamParryRules::Start(F.Tick, B, B.BossParryRaise);
	const FJamIntent Raised = JamBotRules::Play(F, EJamSide::Hero, B);
	TestFalse(TEXT("his shield goes up: no shuriken into it"), Raised.bSecondary);
	TestFalse(TEXT("nor a ball"), Raised.bPrimary);
	return true;
}

#endif
