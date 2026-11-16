#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBossRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-014: the Publisher uses what he stole, through the same buttons and the same code as the hero, with numbers of his
 * own: weaker, slower and with a warning.
 * A bug here looks in game like "he took my katana and never swung it", "his katana hits the moment he raises it",
 * "he throws shurikens he never took" or "the dash I gave away makes him untouchable".
 *
 * Run: Automation RunTests Jam.Boss / Jam.Fight / Jam.Loadout
 */
namespace
{
/** A fight in which the boss holds one stolen mechanic, may act from the first step and has his own attacks put away. */
FJamFight ThiefFight(const FJamBalanceData& B, const FVector2D& HeroPos, EJamMechanic Stolen)
{
	FJamFight F = JamTest::Fight(B, HeroPos, FVector2D::ZeroVector);
	JamLoadoutRules::Give(F.Ledger, Stolen, EJamSide::Boss);
	F.Boss.BusyUntilTick = 0;
	F.Boss.SlamReadyTick = F.Boss.VolleyReadyTick = 1000000;
	return F;
}

bool UsesStolenButtons(const FJamIntent& Intent)
{
	return Intent.bPrimary || Intent.bSecondary || Intent.bDash;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenHandsTest, "Jam.Loadout.StolenIsWeakerInHisHands", JamTest::Flags)

bool FJamStolenHandsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamBalanceData Hero = JamLoadoutRules::InHandsOf(EJamSide::Hero, B);
	TestEqual(TEXT("the hero's katana is the katana"), Hero.KatanaDamage, B.KatanaDamage);
	TestEqual(TEXT("with its quick wind-up"), Hero.KatanaWindUp, B.KatanaWindUp);
	TestEqual(TEXT("his shuriken flies at full speed"), Hero.ShurikenSpeed, B.ShurikenSpeed);

	const FJamBalanceData Boss = JamLoadoutRules::InHandsOf(EJamSide::Boss, B);
	TestEqual(TEXT("the boss's katana warns before it lands"), Boss.KatanaWindUp, B.BossKatanaWindUp);
	TestEqual(TEXT("and cuts weaker"), Boss.KatanaDamage, 5.f);
	TestEqual(TEXT("the finisher too"), Boss.KatanaFinisherDamage, 15.f);
	TestEqual(TEXT("his shuriken is weaker"), Boss.ShurikenDamage, 4.f);
	TestEqual(TEXT("its blast too"), Boss.ShurikenExplosionDamage, 10.f);
	TestEqual(TEXT("it flies slower: there is time to step aside"), Boss.ShurikenSpeed, 1000.f);
	TestEqual(TEXT("and comes less often"), Boss.ShurikenCooldown, 1.f);
	TestEqual(TEXT("his dash comes back slower"), Boss.DashCooldown, 2.f);
	TestEqual(TEXT("the reach of the katana is the same in any hands"), Boss.KatanaRange, B.KatanaRange);
	TestEqual(TEXT("his own attacks keep their numbers"), Boss.SlamDamage, B.SlamDamage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenKatanaBrainTest, "Jam.Boss.SlashesWhenCloseAndOwnsKatana", JamTest::Flags)

bool FJamStolenKatanaBrainTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ThiefFight(B, FVector2D(-240.0, 0.0), EJamMechanic::Katana);
	const FJamIntent Close = JamBossRules::Decide(F, B);
	TestTrue(TEXT("the hero is within the blade's reach: he swings"), Close.bPrimary);
	TestFalse(TEXT("the katana is all he took"), Close.bSecondary || Close.bDash);

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-600.0, 0.0);
	const FJamIntent Far = JamBossRules::Decide(F, B);
	TestFalse(TEXT("he does not swing at the air"), Far.bPrimary);
	TestTrue(TEXT("he walks at the hero instead"), Far.Move.Equals(FVector2D(-1.0, 0.0), 1e-3));

	F.Fighters[EJamSide::Hero].Pos = FVector2D(-240.0, 0.0);
	F.Boss.SlamReadyTick = 0;
	const FJamIntent Slam = JamBossRules::Decide(F, B);
	TestTrue(TEXT("a ready slam still comes first"), Slam.bSlam);
	TestFalse(TEXT("one attack at a time"), Slam.bPrimary);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenShurikenBrainTest, "Jam.Boss.ThrowsWhenFarAndOwnsShuriken", JamTest::Flags)

bool FJamStolenShurikenBrainTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ThiefFight(B, FVector2D(-900.0, 0.0), EJamMechanic::Shuriken);
	const FJamIntent Far = JamBossRules::Decide(F, B);
	TestTrue(TEXT("the hero keeps away: he throws"), Far.bSecondary);
	TestTrue(TEXT("and keeps walking at him"), Far.Move.Equals(FVector2D(-1.0, 0.0), 1e-3));

	FJamFight Near = F;
	Near.Fighters[EJamSide::Hero].Pos = FVector2D(-240.0, 0.0);
	TestFalse(TEXT("a hero at arm's length is not thrown at"), JamBossRules::Decide(Near, B).bSecondary);

	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Dir = FVector2D(-1.0, 0.0);
	Intents[EJamSide::Boss].bSecondary = true;
	const int32 Cooldown = JamSim::Ticks(B.ShurikenCooldown * B.BossStolenCooldown);
	JamFightRules::Step(F, Intents, B);
	if (!TestEqual(TEXT("the button throws one shuriken"), F.Projectiles.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("it is a shuriken"), F.Projectiles[0].Kind, EJamProjectile::Shuriken);
	TestEqual(TEXT("it is his"), F.Projectiles[0].Owner, EJamSide::Boss);
	TestEqual(TEXT("it flies at his speed"), static_cast<float>(F.Projectiles[0].Vel.Size()), 1000.f, 0.1f);
	TestEqual(TEXT("with his damage"), F.Projectiles[0].Damage, 4.f);
	const int32 More = JamTest::RunCounting(F, Intents, B, Cooldown - 1, EJamEvent::ProjectileSpawned);
	TestEqual(TEXT("holding the button through his cooldown throws nothing"), More, 0);
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the next one comes after it"), JamTest::Count(F, EJamEvent::ProjectileSpawned), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenDashBrainTest, "Jam.Boss.DashesOutOfAStrikeWhenOwnsDash", JamTest::Flags)

bool FJamStolenDashBrainTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ThiefFight(B, FVector2D(-240.0, 0.0), EJamMechanic::Dash);
	TestFalse(TEXT("nothing threatens him and the hero is near: he keeps the dash"), JamBossRules::Decide(F, B).bDash);

	FJamFight Struck = F;
	Struck.Fighters[EJamSide::Hero].Katana.bHitPending = true;
	Struck.Fighters[EJamSide::Hero].Katana.HitTick = 5;
	const FJamIntent Out = JamBossRules::Decide(Struck, B);
	TestTrue(TEXT("a strike is about to land on him: he dashes"), Out.bDash);
	TestTrue(TEXT("away from the hero"), Out.Move.Equals(FVector2D(1.0, 0.0), 1e-3));
	Struck.Fighters[EJamSide::Boss].Dash.ReadyTick = 100;
	TestFalse(TEXT("unless the dash is cooling down"), JamBossRules::Decide(Struck, B).bDash);

	FJamFight Far = F;
	Far.Fighters[EJamSide::Hero].Pos = FVector2D(-900.0, 0.0);
	const FJamIntent In = JamBossRules::Decide(Far, B);
	TestTrue(TEXT("a hero who keeps far away gets the dash too"), In.bDash);
	TestTrue(TEXT("straight at him"), In.Move.Equals(FVector2D(-1.0, 0.0), 1e-3));

	// The whole thing: the hero swings, the boss sees the blade coming and leaves.
	FJamIntent Hero;
	Hero.Aim.Dir = FVector2D(1.0, 0.0);
	Hero.bPrimary = true;
	int32 Dashes = 0;
	for (int32 i = 0; i < 12; ++i)
	{
		JamFightRules::Play(F, Hero, B);
		Dashes += JamTest::Count(F, EJamEvent::Dash);
	}
	TestEqual(TEXT("he dashed once"), Dashes, 1);
	TestEqual(TEXT("the strike cut the air"), F.Fighters[EJamSide::Boss].Health, B.BossHealth);
	TestTrue(TEXT("he is out of the blade's reach"), F.Fighters[EJamSide::Boss].Pos.X > 200.0);
	const int32 Cooldown = JamSim::Ticks(B.DashCooldown * B.BossStolenCooldown);
	TestEqual(TEXT("his dash comes back by his own cooldown"), F.Fighters[EJamSide::Boss].Dash.ReadyTick, 1 + Cooldown);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenNothingTest, "Jam.Boss.NeverUsesWhatHeDoesNotOwn", JamTest::Flags)

bool FJamStolenNothingTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D(-240.0, 0.0), FVector2D::ZeroVector);
	F.Boss.BusyUntilTick = 0;
	F.Boss.SlamReadyTick = F.Boss.VolleyReadyTick = 1000000;
	F.Fighters[EJamSide::Hero].Katana.bHitPending = true;
	F.Fighters[EJamSide::Hero].Katana.HitTick = 1000000;
	TestFalse(TEXT("close, with a strike coming: no stolen button is asked for"), UsesStolenButtons(JamBossRules::Decide(F, B)));
	F.Fighters[EJamSide::Hero].Pos = FVector2D(-900.0, 0.0);
	TestFalse(TEXT("far away: none either"), UsesStolenButtons(JamBossRules::Decide(F, B)));

	// Even an intent that holds every button gives him nothing: the ledger is the rule, not the brain.
	FJamIntents Pushy;
	Pushy[EJamSide::Boss].Aim.Dir = FVector2D(-1.0, 0.0);
	Pushy[EJamSide::Boss].bPrimary = true;
	Pushy[EJamSide::Boss].bSecondary = true;
	Pushy[EJamSide::Boss].bDash = true;
	int32 Used = 0;
	for (int32 i = 0; i < 30; ++i)
	{
		JamFightRules::Step(F, Pushy, B);
		Used += JamTest::Count(F, EJamEvent::Swing) + JamTest::Count(F, EJamEvent::Dash) + JamTest::Count(F, EJamEvent::ProjectileSpawned);
	}
	TestEqual(TEXT("no swing, no dash, no throw, not even the ball"), Used, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamStolenKatanaTest, "Jam.Fight.BossKatanaDamagesTheHero", JamTest::Flags)

bool FJamStolenKatanaTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ThiefFight(B, FVector2D(-240.0, 0.0), EJamMechanic::Katana);
	FJamIntents Intents;
	Intents[EJamSide::Boss].Aim.Dir = FVector2D(-1.0, 0.0);
	Intents[EJamSide::Boss].Move = FVector2D(-1.0, 0.0);
	Intents[EJamSide::Boss].bPrimary = true;
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("the press starts his strike"), JamTest::Count(F, EJamEvent::Swing), 1);
	TestTrue(TEXT("he stands through the wind-up"), JamBossRules::IsBusy(F));
	const FVector2D Stance = F.Fighters[EJamSide::Boss].Pos;

	const int32 WindUp = JamSim::Ticks(B.BossKatanaWindUp);
	JamTest::Run(F, Intents, B, WindUp - 1);
	TestEqual(TEXT("nothing hurts during the warning"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestTrue(TEXT("and he did not walk during it"), F.Fighters[EJamSide::Boss].Pos.Equals(Stance, 1e-3));
	JamFightRules::Step(F, Intents, B);
	TestEqual(TEXT("then the strike lands with his share of the damage"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - 5.f);

	// He goes on to the finisher, and after it he rests like after any attack of his own.
	const int32 Strike = WindUp + JamSim::Ticks(B.KatanaRecovery);
	JamTest::Run(F, Intents, B, 2 * Strike);
	TestEqual(TEXT("the series of three took 5, 5 and 15"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - 25.f);
	TestTrue(TEXT("after the finisher he stands"), JamBossRules::IsBusy(F));
	const int32 Swings = JamTest::RunCounting(F, Intents, B, JamSim::Ticks(B.BossRecovery) - 2, EJamEvent::Swing);
	TestEqual(TEXT("and starts nothing through the pause: the player's opening"), Swings, 0);
	return true;
}

#endif
