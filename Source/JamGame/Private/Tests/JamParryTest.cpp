#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamParryRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-019: the parry. A short window on Q in which a projectile that reaches the fighter turns round and flies back at
 * the one who threw it, harder. A bug here looks in game like "I pressed Q in time and still took the memo" or "the
 * memo came back and flew past him".
 *
 * Run: Automation RunTests Jam.Parry
 */
namespace
{
FJamBalanceData ParryBalance()
{
	FJamBalanceData B = JamTest::Balance();
	B.ParryWindow = 0.2f;
	B.ParryCooldown = 1.f;
	B.ParryReturnSpeed = 1500.f;
	B.ParryDamageScale = 2.f;
	B.ParryReturnLife = 2.f;
	return B;
}

/** A memo of the boss that will reach the hero at the origin in a few steps, from the boss side. */
FJamFight MemoIncoming(const FJamBalanceData& B)
{
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	FJamShot Memo;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Speed = B.MemoSpeed;
	Memo.Radius = B.MemoRadius;
	Memo.Damage = B.MemoDamage;
	Memo.Life = B.MemoLife;
	JamFightOps::Shoot(F, EJamSide::Boss, FVector2D(-1.0, 0.0), Memo);
	// The boss stands where he is: only the memo moves.
	F.Boss.BusyUntilTick = 100000;
	return F;
}

FJamIntents Parrying()
{
	FJamIntents Intents;
	Intents[EJamSide::Hero].bParry = true;
	return Intents;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryWindowTest, "Jam.Parry.WindowOpensAndCloses", JamTest::Flags)

bool FJamParryWindowTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	const FJamParryState Fresh;
	TestTrue(TEXT("ready from the start"), JamParryRules::CanStart(Fresh, 0));
	const FJamParryState S = JamParryRules::Start(10, B);
	TestTrue(TEXT("active at once"), JamParryRules::IsActive(S, 10));
	TestTrue(TEXT("active at the end of the window"), JamParryRules::IsActive(S, 10 + JamSim::Ticks(B.ParryWindow) - 1));
	TestFalse(TEXT("closed after it"), JamParryRules::IsActive(S, 10 + JamSim::Ticks(B.ParryWindow)));
	TestFalse(TEXT("no second parry inside the cooldown"), JamParryRules::CanStart(S, 10 + JamSim::Ticks(B.ParryWindow)));
	TestTrue(TEXT("ready after the cooldown"), JamParryRules::CanStart(S, 10 + JamSim::Ticks(B.ParryCooldown)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryDeflectTest, "Jam.Parry.DeflectsAMemoBackAtTheBoss", JamTest::Flags)

bool FJamParryDeflectTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	// Let the memo come close, then parry just before it lands.
	JamTest::Run(F, FJamIntents(), B, 33);
	const int32 Deflected = JamTest::RunCounting(F, Parrying(), B, 12, EJamEvent::Deflected);
	TestEqual(TEXT("one deflection"), Deflected, 1);
	TestEqual(TEXT("the hero took nothing"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	if (!TestEqual(TEXT("the memo is still in the air"), F.Projectiles.Num(), 1))
	{
		return true;
	}
	const FJamProjectile& Back = F.Projectiles[0];
	TestEqual(TEXT("it is the hero's now"), Back.Owner, EJamSide::Hero);
	TestTrue(TEXT("it flies at the boss"), Back.Vel.X > 0.0 && FMath::Abs(Back.Vel.Y) < 1.0);
	TestEqual(TEXT("at the return speed"), static_cast<float>(Back.Vel.Size()), B.ParryReturnSpeed, 1.f);
	TestEqual(TEXT("harder than it came"), Back.Damage, B.MemoDamage * B.ParryDamageScale);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryHurtsTest, "Jam.Parry.TheReturnHurtsTheThrower", JamTest::Flags)

bool FJamParryHurtsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	JamTest::Run(F, FJamIntents(), B, 33);
	JamTest::Run(F, Parrying(), B, 12);
	JamTest::Run(F, FJamIntents(), B, 60);
	TestEqual(TEXT("the boss took the doubled memo"), F.Fighters[EJamSide::Boss].Health, B.BossHealth - B.MemoDamage * B.ParryDamageScale);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryLateTest, "Jam.Parry.WithoutTheWindowTheMemoHits", JamTest::Flags)

bool FJamParryLateTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	// Pressed far too early: the window is over long before the memo arrives.
	JamTest::Run(F, Parrying(), B, 1);
	JamTest::Run(F, FJamIntents(), B, 60);
	TestEqual(TEXT("the hero took the memo"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - B.MemoDamage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryOwnerTest, "Jam.Parry.NeedsTheMechanic", JamTest::Flags)

bool FJamParryOwnerTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	TestEqual(TEXT("Q does nothing without the parry"), JamLoadoutRules::ActionFor(EJamButton::Parry, EJamSide::Hero, F.Ledger),
		EJamAction::None);
	JamTest::Run(F, FJamIntents(), B, 33);
	JamTest::Run(F, Parrying(), B, 12);
	TestEqual(TEXT("so the memo hits"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - B.MemoDamage);
	TestEqual(TEXT("the name on the card"), FString(JamLoadoutRules::Name(EJamMechanic::Parry)), FString(TEXT("PARRY")));
	return true;
}

/**
 * T-044: the author's playtest: no separate parry key. LMB is the melee strike all the time; when an enemy shot is about
 * to land on the hero, the same press parries it. A bug looks like "I hit LMB as the memo arrived and took it" or "my
 * swing turned into a parry with nothing near".
 */
namespace
{
FJamIntents Striking()
{
	FJamIntents Intents;
	Intents[EJamSide::Hero].bPrimary = true;
	return Intents;
}

/** Steps the fight until the memo would land inside a parry window; false if it never comes that close. */
bool UntilInReach(FJamFight& F, const FJamBalanceData& B)
{
	for (int32 Step = 0; Step < 200 && !JamParryRules::ShotInReach(F, EJamSide::Hero, B); ++Step)
	{
		JamTest::Run(F, FJamIntents(), B, 1);
	}
	return JamParryRules::ShotInReach(F, EJamSide::Hero, B);
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryOnStrikeTest, "Jam.Parry.TheStrikeParriesAShotAboutToLand", JamTest::Flags)

bool FJamParryOnStrikeTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	TestFalse(TEXT("the memo starts far away"), JamParryRules::ShotInReach(F, EJamSide::Hero, B));
	if (!TestTrue(TEXT("it comes into reach"), UntilInReach(F, B)))
	{
		return true;
	}
	FJamFight Pressed = F;
	TestEqual(TEXT("LMB opens the parry"), JamTest::RunCounting(Pressed, Striking(), B, 1, EJamEvent::ParryStarted), 1);
	FJamFight Swung = F;
	TestEqual(TEXT("instead of a swing"), JamTest::RunCounting(Swung, Striking(), B, 1, EJamEvent::Swing), 0);
	JamTest::Run(Pressed, FJamIntents(), B, 30);
	TestEqual(TEXT("the hero took nothing"), Pressed.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryStrikeSwingsTest, "Jam.Parry.TheStrikeSwingsWithNoShotNear", JamTest::Flags)

bool FJamParryStrikeSwingsTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	FJamFight Swung = F;
	TestEqual(TEXT("a far memo: LMB swings"), JamTest::RunCounting(Swung, Striking(), B, 1, EJamEvent::Swing), 1);
	TestEqual(TEXT("and opens no parry"), JamTest::RunCounting(F, Striking(), B, 1, EJamEvent::ParryStarted), 0);

	FJamFight Own = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(600.0, 0.0));
	FJamShot Ball;
	Ball.Kind = EJamProjectile::Ball;
	Ball.Speed = B.BallSpeed;
	Ball.Radius = B.BallRadius;
	Ball.Damage = B.BallDamage;
	Ball.Life = B.BallLife;
	JamFightOps::Shoot(Own, EJamSide::Hero, FVector2D(1.0, 0.0), Ball);
	TestFalse(TEXT("his own shot is never one to parry"), JamParryRules::ShotInReach(Own, EJamSide::Hero, B));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryStrikeStolenTest, "Jam.Parry.TheStrikeOnlySwingsWithoutTheParry", JamTest::Flags)

bool FJamParryStrikeStolenTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = ParryBalance();
	FJamFight F = MemoIncoming(B);
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	if (!TestTrue(TEXT("the memo comes into reach"), UntilInReach(F, B)))
	{
		return true;
	}
	TestEqual(TEXT("LMB opens no parry he does not own"), JamTest::RunCounting(F, Striking(), B, 1, EJamEvent::ParryStarted), 0);
	JamTest::Run(F, FJamIntents(), B, 30);
	TestEqual(TEXT("so the memo hits"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth - B.MemoDamage);
	return true;
}

#endif
