#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamFightOps.h"
#include "Sim/JamParryRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-057: an early parry click waits for the shot. The strike parries a shot about to land (T-044); a click that comes
 * a little too early, while the shot is still on its way, is held for ParryBuffer and opens the parry the moment the
 * shot comes within the window, instead of turning into a swing.
 * A bug here looks in game like "a shot was about to hit me, I clicked, it hit me anyway" or "I clicked at a far memo
 * and my sword froze".
 *
 * Run: Automation RunTests Jam.Parry
 */
namespace
{
/** The hero at the origin with a memo of the boss's on its way, Seconds from touching him. */
FJamFight MemoIn(const FJamBalanceData& B, float Seconds)
{
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(900.0, 0.0));
	FJamShot Memo;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Speed = B.MemoSpeed;
	Memo.Radius = B.MemoRadius;
	Memo.Damage = B.MemoDamage;
	Memo.Life = B.MemoLife;
	JamFightOps::Shoot(F, EJamSide::Boss, FVector2D(-1.0, 0.0), Memo);
	FJamProjectile& Flying = F.Projectiles[0];
	// One step of flight is taken before the hit is checked: the memo touches him Seconds after the next step starts.
	Flying.Pos = Flying.PrevPos = FVector2D(B.HeroRadius + B.MemoRadius + B.MemoSpeed * Seconds, 0.0);
	F.Boss.BusyUntilTick = 100000;
	return F;
}

FJamIntents Click()
{
	FJamIntents Intents;
	Intents[EJamSide::Hero].bPrimary = true;
	return Intents;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryEarlyClickTest, "Jam.Parry.EarlyClickWaitsForTheShot", JamTest::Flags)

bool FJamParryEarlyClickTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = MemoIn(B, B.ParryWindow + 0.5f * B.ParryBuffer);
	TestFalse(TEXT("the memo is not yet within one window"), JamParryRules::ShotInReach(F, EJamSide::Hero, B));
	JamFightRules::Step(F, Click(), B);
	TestEqual(TEXT("too early for the window: no parry yet"), JamTest::Count(F, EJamEvent::ParryStarted), 0);
	TestFalse(TEXT("and no swing: the click waits"), F.Fighters[EJamSide::Hero].Katana.bHitPending);
	int32 Parries = 0;
	int32 Deflects = 0;
	int32 Swings = 0;
	for (int32 Step = 0; Step < 60; ++Step)
	{
		JamFightRules::Step(F, FJamIntents(), B);
		Parries += JamTest::Count(F, EJamEvent::ParryStarted);
		Deflects += JamTest::Count(F, EJamEvent::Deflected);
		Swings += JamTest::Count(F, EJamEvent::Swing);
	}
	TestEqual(TEXT("the held click opens the parry when the memo comes in"), Parries, 1);
	TestEqual(TEXT("which sends it back"), Deflects, 1);
	TestEqual(TEXT("the hero took nothing"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	TestEqual(TEXT("the held click never swung"), Swings, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryHeldLapsesTest, "Jam.Parry.HeldClickLapses", JamTest::Flags)

bool FJamParryHeldLapsesTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = MemoIn(B, B.ParryWindow + 0.5f * B.ParryBuffer);
	JamFightRules::Step(F, Click(), B);
	// The memo is gone (dodged, or cut down): the click has nothing to wait for.
	F.Projectiles.Reset();
	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.ParryBuffer));
	FJamFight Later = MemoIn(B, 0.05f);
	Later.Tick = F.Tick;
	Later.Fighters[EJamSide::Hero].Parry = F.Fighters[EJamSide::Hero].Parry;
	TestEqual(TEXT("a shot after the buffer is not parried by the old click"),
		JamTest::RunCounting(Later, FJamIntents(), B, 10, EJamEvent::ParryStarted), 0);
	TestTrue(TEXT("it hits"), Later.Fighters[EJamSide::Hero].Health < B.HeroHealth);
	TestTrue(TEXT("and the parry is still at hand: nothing was spent"), JamParryRules::CanStart(F.Fighters[EJamSide::Hero].Parry, F.Tick));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryFarClickTest, "Jam.Parry.ClickWithNothingComingSwings", JamTest::Flags)

bool FJamParryFarClickTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight Far = MemoIn(B, B.ParryWindow + B.ParryBuffer + 0.2f);
	JamFightRules::Step(Far, Click(), B);
	TestTrue(TEXT("a memo further than the buffer: the click swings"), Far.Fighters[EJamSide::Hero].Katana.bHitPending);

	FJamFight Empty = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(900.0, 0.0));
	JamFightRules::Step(Empty, Click(), B);
	TestTrue(TEXT("nothing in the air: the click swings"), Empty.Fighters[EJamSide::Hero].Katana.bHitPending);
	TestEqual(TEXT("and opens no parry"), JamTest::Count(Empty, EJamEvent::ParryStarted), 0);
	return true;
}

/**
 * T-059: a parry that sends a shot back is ready again at once. The author's playtest: spamming LMB into a burst sent
 * back every other shot, the cooldown after a good parry let the next one through. A bug here looks in game like "I was
 * clicking the whole time and still got hit by the second memo".
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryBurstTest, "Jam.Parry.SpamSendsBackEveryShotOfABurst", JamTest::Flags)

bool FJamParryBurstTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	// Four memos in a line, landing closer together than the cooldown and further apart than one window; the last one
	// still starts inside the arena.
	const float Gap = B.ParryWindow + 0.1f;
	TestTrue(TEXT("the burst is denser than the cooldown"), Gap < B.ParryCooldown);
	FJamFight F = MemoIn(B, 0.1f);
	for (int32 Index = 1; Index < 4; ++Index)
	{
		FJamProjectile Next = F.Projectiles[0];
		Next.Id = F.Projectiles[0].Id + Index;
		Next.Pos = Next.PrevPos = F.Projectiles[0].Pos + FVector2D(B.MemoSpeed * Gap * Index, 0.0);
		F.Projectiles.Add(Next);
	}
	int32 Deflects = 0;
	for (int32 Step = 0; Step < JamSim::Ticks(4.f * Gap + 0.5f); ++Step)
	{
		JamFightRules::Step(F, Click(), B);
		Deflects += JamTest::Count(F, EJamEvent::Deflected);
	}
	TestEqual(TEXT("every memo of the burst goes back"), Deflects, 4);
	TestEqual(TEXT("the hero took nothing"), F.Fighters[EJamSide::Hero].Health, B.HeroHealth);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamParryWhiffTest, "Jam.Parry.AParryThatSendsNothingBackKeepsItsCooldown", JamTest::Flags)

bool FJamParryWhiffTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	// The memo is in reach when the parry opens, then it is gone (cut down, out of the air): nothing comes back.
	FJamFight F = MemoIn(B, 0.05f);
	JamFightRules::Step(F, Click(), B);
	TestEqual(TEXT("the parry opened"), JamTest::Count(F, EJamEvent::ParryStarted), 1);
	F.Projectiles.Reset();
	JamTest::Run(F, FJamIntents(), B, JamSim::Ticks(B.ParryWindow) + 1);
	TestFalse(TEXT("an empty parry still waits out its cooldown"), JamParryRules::CanStart(F.Fighters[EJamSide::Hero].Parry, F.Tick));
	return true;
}

#endif
