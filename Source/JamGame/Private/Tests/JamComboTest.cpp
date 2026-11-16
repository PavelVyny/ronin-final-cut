#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamBossRules.h"
#include "Sim/JamComboRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamSlamRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-056: the Publisher's signature chains. From the first cut he strings his attacks together, and every mechanic he
 * stole adds a chain of its own. Each chain warns first, keeps a gap between its links and ends with his pause.
 * A bug here looks in game like "he dashes and slams in the same instant", "he chains with a mechanic I still have",
 * "the same combo every time", "the fan never goes where I have to stand" or "after a combo he never stops".
 *
 * Run: Automation RunTests Jam.Combo
 */
namespace
{
const EJamCombo EveryCombo[] = { EJamCombo::Pocket, EJamCombo::DashSlam, EJamCombo::FanBurst, EJamCombo::ShieldFan, EJamCombo::FanLine,
	EJamCombo::FanRush };

/** A fight after the first cut, the boss free to act at the origin, his own attacks put away. */
FJamFight ComboFight(const FJamBalanceData& B, const FVector2D& HeroPos)
{
	FJamFight F = JamTest::Fight(B, HeroPos, FVector2D::ZeroVector);
	F.Steal.Done = 1;
	F.Boss.BusyUntilTick = 0;
	F.Boss.SlamReadyTick = F.Boss.VolleyReadyTick = 1000000;
	return F;
}

bool Has(const FJamFight& F, EJamEvent Type, EJamSide Side)
{
	return F.Events.ContainsByPredicate([Type, Side](const FJamEvent& E) { return E.Type == Type && E.Side == Side; });
}

int32 Spawned(const FJamFight& F, EJamProjectile Kind)
{
	int32 Count = 0;
	for (const FJamEvent& E : F.Events)
	{
		const FJamProjectile* P = F.Projectiles.FindByPredicate([&E](const FJamProjectile& X) { return X.Id == E.ProjectileId; });
		Count += E.Type == EJamEvent::ProjectileSpawned && P && P->Kind == Kind && P->Owner == EJamSide::Boss ? 1 : 0;
	}
	return Count;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboOwnedTest, "Jam.Combo.OnlyWhatHeStole", JamTest::Flags)

bool FJamComboOwnedTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight Near = ComboFight(B, FVector2D(-300.0, 0.0));
	FJamFight Uncut = Near;
	Uncut.Steal.Done = 0;
	TestTrue(TEXT("before the first cut he has no chain"), JamComboRules::Available(Uncut, B).IsEmpty());
	const TArray<EJamCombo> Own = JamComboRules::Available(Near, B);
	TestTrue(TEXT("after it, with the hero close, his own: slam then the pocket fan"), Own.Num() == 1 && Own.Contains(EJamCombo::Pocket));

	FJamFight Far = ComboFight(B, FVector2D(-800.0, 0.0));
	TestTrue(TEXT("a far hero and nothing stolen: no chain"), JamComboRules::Available(Far, B).IsEmpty());

	JamLoadoutRules::Give(Far.Ledger, EJamMechanic::Dash, EJamSide::Boss);
	JamLoadoutRules::Give(Near.Ledger, EJamMechanic::Dash, EJamSide::Boss);
	TestTrue(TEXT("the stolen dash closes a far hero in"), JamComboRules::Available(Far, B).Contains(EJamCombo::DashSlam));
	TestFalse(TEXT("but is no chain for a hero who is already close"), JamComboRules::Available(Near, B).Contains(EJamCombo::DashSlam));

	JamLoadoutRules::Give(Far.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	TestTrue(TEXT("the shuriken adds the fan and the burst"), JamComboRules::Available(Far, B).Contains(EJamCombo::FanBurst));
	JamLoadoutRules::Give(Far.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	TestTrue(TEXT("the parry adds the stance and the fan"), JamComboRules::Available(Far, B).Contains(EJamCombo::ShieldFan));
	JamLoadoutRules::Give(Far.Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	Far.Fighters[EJamSide::Boss].Ultimate.Meter = 0.5f;
	TestFalse(TEXT("the Ultimate chain waits for a full meter"), JamComboRules::Available(Far, B).Contains(EJamCombo::FanLine));
	Far.Fighters[EJamSide::Boss].Ultimate.Meter = 1.f;
	TestTrue(TEXT("full: the fan and the line"), JamComboRules::Available(Far, B).Contains(EJamCombo::FanLine));
	TestFalse(TEXT("the katana is still the hero's"), JamComboRules::Available(Far, B).Contains(EJamCombo::FanRush));
	JamLoadoutRules::Give(Far.Ledger, EJamMechanic::Katana, EJamSide::Boss);
	TestTrue(TEXT("the final note adds the rush"), JamComboRules::Available(Far, B).Contains(EJamCombo::FanRush));

	FJamFight Cooling = Near;
	Cooling.Boss.ComboReadyTick = Cooling.Tick + 10;
	TestFalse(TEXT("the brain waits out the cooldown"), JamBossRules::Decide(Cooling, B).bCombo);
	TestTrue(TEXT("and asks for a chain once it is over"), JamBossRules::Decide(Near, B).bCombo);
	TestFalse(TEXT("never before the first cut"), JamBossRules::Decide(Uncut, B).bCombo);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboGapTest, "Jam.Combo.LinksKeepTheirGap", JamTest::Flags)

bool FJamComboGapTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	const FJamFight F = ComboFight(B, FVector2D(-800.0, 0.0));
	const int32 Reaction = JamSim::Ticks(0.3f);
	for (const EJamCombo Combo : EveryCombo)
	{
		const TArray<FJamComboLink> Links = JamComboRules::Plan(Combo, F, B);
		const FString Name = FString::Printf(TEXT("chain %d"), static_cast<int32>(Combo));
		if (!TestTrue(Name + TEXT(" has links"), Links.Num() >= 2))
		{
			continue;
		}
		TestEqual(Name + TEXT(": the first link comes after the tell"), Links[0].Tick, F.Tick + JamSim::Ticks(B.ComboTell));
		TestEqual(Name + TEXT(": it ends"), Links.Last().Kind, EJamLink::End);
		for (int32 Index = 1; Index < Links.Num(); ++Index)
		{
			const FJamComboLink& Prev = Links[Index - 1];
			const FJamComboLink& Next = Links[Index];
			TestTrue(Name + TEXT(": links go in order"), Next.Tick > Prev.Tick);
			// The shots of one burst are one link; the end is no attack.
			const bool bSameLink = Prev.Kind == EJamLink::Burst && Next.Kind == EJamLink::Burst;
			if (!bSameLink && Next.Kind != EJamLink::End && Prev.Kind != EJamLink::Rush)
			{
				TestTrue(Name + TEXT(": a gap to react between two links"), Next.Tick - Prev.Tick >= Reaction);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboBurstGapTest, "Jam.Combo.HisBurstIsSpacedLikeHisShurikens", JamTest::Flags)

bool FJamComboBurstGapTest::RunTest(const FString& Parameters)
{
	// T-070: the burst throws on its own clock, so the slower shuriken has to reach it too.
	FJamBalanceData B = JamTest::Balance();
	B.ComboBurstGap = 0.2f;
	B.BossShurikenCooldownScale = 1.3f;
	const FJamFight F = ComboFight(B, FVector2D(-800.0, 0.0));
	TArray<int32> Shots;
	for (const FJamComboLink& Link : JamComboRules::Plan(EJamCombo::FanBurst, F, B))
	{
		if (Link.Kind == EJamLink::Burst)
		{
			Shots.Add(Link.Tick);
		}
	}
	if (TestEqual(TEXT("the burst throws three"), Shots.Num(), 3))
	{
		const int32 Gap = JamSim::Ticks(0.26f);
		TestEqual(TEXT("the second comes 30% later than the plain gap"), Shots[1] - Shots[0], Gap);
		TestEqual(TEXT("and the third as late again"), Shots[2] - Shots[1], Gap);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboPickTest, "Jam.Combo.NeverTheSameTwiceInARow", JamTest::Flags)

bool FJamComboPickTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(7);
	const TArray<EJamCombo> Two = { EJamCombo::Pocket, EJamCombo::FanBurst };
	bool bOnlyTheOther = true;
	for (int32 Draw = 0; Draw < 50; ++Draw)
	{
		bOnlyTheOther = bOnlyTheOther && JamComboRules::Pick(Two, EJamCombo::Pocket, Stream) == EJamCombo::FanBurst;
	}
	TestTrue(TEXT("with another one there, the last chain is not repeated"), bOnlyTheOther);
	TestEqual(TEXT("a single chain is repeated: better than none"), JamComboRules::Pick({ EJamCombo::Pocket }, EJamCombo::Pocket, Stream),
		EJamCombo::Pocket);
	const TArray<EJamCombo> Three = { EJamCombo::Pocket, EJamCombo::FanBurst, EJamCombo::ShieldFan };
	TSet<EJamCombo> Seen;
	for (int32 Draw = 0; Draw < 100; ++Draw)
	{
		Seen.Add(JamComboRules::Pick(Three, EJamCombo::None, Stream));
	}
	TestEqual(TEXT("the stream picks among all of them"), Seen.Num(), 3);
	TestEqual(TEXT("nothing to pick from: none"), JamComboRules::Pick({}, EJamCombo::None, Stream), EJamCombo::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboDashSlamTest, "Jam.Combo.DashSlamLandsOnArrival", JamTest::Flags)

bool FJamComboDashSlamTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ComboFight(B, FVector2D(-800.0, 0.0));
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Dash, EJamSide::Boss);
	int32 DashTick = -1;
	int32 SlamTick = -1;
	for (int32 Step = 0; Step < 180 && SlamTick < 0; ++Step)
	{
		const int32 Tick = F.Tick;
		JamFightRules::Play(F, FJamIntent(), B);
		DashTick = DashTick < 0 && Has(F, EJamEvent::Dash, EJamSide::Boss) ? Tick : DashTick;
		SlamTick = SlamTick < 0 && !F.Hazards.IsEmpty() ? Tick : SlamTick;
	}
	TestEqual(TEXT("he dashes after the tell"), DashTick, JamSim::Ticks(B.ComboTell));
	TestTrue(TEXT("and slams once the dash is over, with a gap to react"),
		SlamTick - DashTick >= JamSim::Ticks(B.DashDuration) + JamSim::Ticks(0.3f));
	TestTrue(TEXT("he came in"), F.Fighters[EJamSide::Boss].Pos.X < -B.DashDistance * 0.9);
	const FVector2D HeroPos = F.Fighters[EJamSide::Hero].Pos;
	TestTrue(TEXT("a circle is on the hero"),
		F.Hazards.ContainsByPredicate([&HeroPos](const FJamHazard& H) { return FVector2D::Distance(H.Pos, HeroPos) < 1.0; }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboPocketTest, "Jam.Combo.PocketFanAimsAtThePocket", JamTest::Flags)

bool FJamComboPocketTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ComboFight(B, FVector2D(-300.0, 0.0));
	int32 PatternTick = -1;
	for (int32 Step = 0; Step < 300; ++Step)
	{
		const FJamFight Before = F;
		JamFightRules::Play(F, FJamIntent(), B);
		PatternTick = PatternTick < 0 && !F.Hazards.IsEmpty() ? Before.Tick : PatternTick;
		if (Spawned(F, EJamProjectile::Memo) == 0)
		{
			continue;
		}
		TestFalse(TEXT("the fan comes while his circles are down"), Before.Hazards.IsEmpty());
		// The pocket the pattern offered the hero when it went down, after the time to read it.
		const TOptional<FVector2D> Spot =
			JamSlamRules::SafeSpot(Before.Hazards, Before.Fighters[EJamSide::Hero].Pos, PatternTick, B.SlamReaction, B);
		if (!TestTrue(TEXT("there is a pocket"), Spot.IsSet()))
		{
			return false;
		}
		const FVector2D Expected = (*Spot - Before.Fighters[EJamSide::Boss].Pos).GetSafeNormal();
		const int32 Middle = FMath::RoundToInt32(B.MemoCount) / 2;
		const FJamProjectile* Centre =
			F.Projectiles.FindByPredicate([&Before, Middle](const FJamProjectile& P) { return P.Id == Before.NextId + Middle; });
		if (TestNotNull(TEXT("the fan's middle memo"), Centre))
		{
			TestTrue(TEXT("flies at the pocket"), Centre->Vel.GetSafeNormal().Equals(Expected, 1e-3));
		}
		return true;
	}
	AddError(TEXT("no fan in five seconds"));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboShieldFanTest, "Jam.Combo.ShieldDropsIntoAFan", JamTest::Flags)

bool FJamComboShieldFanTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ComboFight(B, FVector2D(-800.0, 0.0));
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Parry, EJamSide::Boss);
	int32 ShieldTick = -1;
	int32 FanTick = -1;
	for (int32 Step = 0; Step < 300 && FanTick < 0; ++Step)
	{
		const int32 Tick = F.Tick;
		JamFightRules::Play(F, FJamIntent(), B);
		ShieldTick = ShieldTick < 0 && Has(F, EJamEvent::ParryStarted, EJamSide::Boss) ? Tick : ShieldTick;
		FanTick = FanTick < 0 && Spawned(F, EJamProjectile::Memo) > 0 ? Tick : FanTick;
	}
	TestEqual(TEXT("the shield goes up after the tell"), ShieldTick, JamSim::Ticks(B.ComboTell));
	TestEqual(
		TEXT("the fan flies the moment it drops"), FanTick, ShieldTick + JamSim::Ticks(B.BossParryRaise) + JamSim::Ticks(B.BossParryHold));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComboEndTest, "Jam.Combo.EndsWithHisPause", JamTest::Flags)

bool FJamComboEndTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = ComboFight(B, FVector2D(-800.0, 0.0));
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	int32 Memos = 0;
	int32 Shuriken = 0;
	bool bStarted = false;
	for (int32 Step = 0; Step < 300; ++Step)
	{
		const int32 Tick = F.Tick;
		JamFightRules::Play(F, FJamIntent(), B);
		Memos += Spawned(F, EJamProjectile::Memo);
		Shuriken += Spawned(F, EJamProjectile::Shuriken);
		bStarted = bStarted || JamComboRules::IsRunning(F);
		if (bStarted && !JamComboRules::IsRunning(F))
		{
			TestEqual(TEXT("the chain was the fan"), Memos, FMath::RoundToInt32(B.MemoCount));
			TestEqual(TEXT("and the burst"), Shuriken, FMath::RoundToInt32(B.ComboBurstCount));
			TestEqual(TEXT("the next chain waits its cooldown"), F.Boss.ComboReadyTick, Tick + JamSim::Ticks(B.ComboCooldown));
			TestTrue(TEXT("he stands for his pause"), JamBossRules::IsBusy(F));
			TestEqual(TEXT("a pause as long as after any attack"), F.Boss.BusyUntilTick, Tick + JamSim::Ticks(B.BossRecovery));
			return true;
		}
	}
	AddError(TEXT("the chain never ended"));
	return false;
}

#endif
