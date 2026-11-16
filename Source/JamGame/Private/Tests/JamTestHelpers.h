#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "Sim/JamFightRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamStealRules.h"

/**
 * Shared helpers of the project tests.
 *
 * Conventions: test names follow Jam.<Area>.<Case> (build_and_test.sh runs them by the Jam. prefix), every test uses
 * JamTest::Flags, and a header pinned by tests says so with a "Tests: Jam.<Area>.*" line.
 */
namespace JamTest
{
/** Flags of every project test: editor context, Product filter (the Engine filter is for engine tests). */
inline constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

/** The cuts of the Publisher and what a stolen mechanic is worth in his hands, for JamTest::Balance. */
inline void SetStealNumbers(FJamBalanceData& B)
{
	B.StealThreshold1 = 0.75f;
	B.StealThreshold2 = 0.5f;
	B.StealThreshold3 = 0.25f;
	B.StealThreshold4 = 0.1f;
	B.StealChoiceSeconds = 3.f;
	B.StealChoiceLock = 0.5f;
	B.HeroCutDamageScale = 1.4f;
	B.BossStolenDamage = 0.5f;
	B.BossStolenCooldown = 2.f;
	B.BossShurikenCooldownScale = 1.f;
	B.BossShurikenCooldown = 0.5f;
	B.BossShurikenDamage = 8.f;
	B.BossStolenSpeed = 0.5f;
	B.BossKatanaWindUp = 0.5f;
	B.BossParryRaise = 0.25f;
	B.BossParryHold = 1.f;
	B.BossParryCooldown = 3.f;
	B.BossParryDamageScale = 1.5f;
	B.BossShurikenParryMargin = 30.f;
	B.BossUltimateInterval = 10.f;
	B.BossUltimateWindUp = 0.8f;
	B.BossUltimateDamage = 30.f;
	B.ComboCooldown = 4.f;
	B.ComboTell = 0.3f;
	B.ComboLinkGap = 0.4f;
	B.ComboBurstCount = 3.f;
	B.ComboBurstGap = 0.2f;
	B.ComboRushSeconds = 1.f;
	B.RetryLowHealthShare = 0.2f;
	B.RetryLowHealthRestore = 0.5f;
	B.HealAmount = 25.f;
	B.HealDropInterval = 10.f;
	B.HealLife = 5.f;
	B.HealMaxOnFloor = 1.f;
	B.HealToss = 200.f;
	B.HealTossSeconds = 0.5f;
	B.HealPickupRadius = 40.f;
	B.HeroDamageScale = 1.f;
	B.BossDamageScale = 1.f;
	B.EasyHeroHealthScale = 1.5f;
	B.EasyHeroDamageScale = 1.3f;
	B.EasyHealScale = 1.5f;
	B.EasyBossDamageScale = 0.8f;
	B.NormalHeroHealthScale = 1.f;
	B.NormalHeroDamageScale = 1.f;
	B.NormalHealScale = 1.f;
	B.NormalBossDamageScale = 1.f;
	B.HardHeroHealthScale = 0.8f;
	B.HardHeroDamageScale = 0.9f;
	B.HardHealScale = 0.7f;
	B.HardBossDamageScale = 1.25f;
}

/** The two mechanics of block D, for JamTest::Balance. */
inline void SetParryAndUltimate(FJamBalanceData& B)
{
	B.ParryWindow = 0.2f;
	B.ParryCooldown = 1.f;
	B.ParryBuffer = 0.3f;
	B.ParryReturnSpeed = 1500.f;
	B.ParryDamageScale = 2.f;
	B.ParryReturnLife = 2.f;
	B.UltimateGainPerDamage = 0.005f;
	B.UltimateWindUp = 0.3f;
	B.UltimateTurnRate = 120.f;
	B.UltimateRange = 1000.f;
	B.UltimateHalfWidth = 80.f;
	B.UltimateDamage = 200.f;
}

/** The slam that grows with the cuts (T-050), for JamTest::Balance. */
inline void SetSlamPattern(FJamBalanceData& B)
{
	B.SlamCirclesUncut = 1.f;
	B.SlamCirclesFirstCut = 3.f;
	B.SlamCirclesSecondCut = 5.f;
	B.SlamSideRadius = 200.f;
	B.SlamCutterDistance = 380.f;
	B.SlamFlankAngle = 80.f;
	B.SlamGap = 160.f;
	B.SlamSpreadAngle = 55.f;
	B.SlamJitter = 15.f;
	B.SlamRipple = 0.2f;
	B.SlamReaction = 0.3f;
}

/** Round numbers for rule tests. They are not the game's tuning: a test that depends on a number sets it itself. */
inline FJamBalanceData Balance()
{
	FJamBalanceData B;
	B.ArenaRadius = 1000.f;
	B.HeroRadius = 50.f;
	B.HeroSpeed = 600.f;
	B.HeroHealth = 100.f;
	B.BossRadius = 100.f;
	B.BossSpeed = 300.f;
	B.BossHealth = 1000.f;
	B.BallDamage = 10.f;
	B.BallSpeed = 1200.f;
	B.BallCooldown = 0.5f;
	B.BallRadius = 10.f;
	B.BallLife = 2.f;
	B.KatanaRange = 200.f;
	B.KatanaHalfArc = 60.f;
	B.KatanaWindUp = 0.1f;
	B.KatanaRecovery = 0.2f;
	B.KatanaChainWindow = 0.5f;
	B.KatanaStep = 40.f;
	B.KatanaDamage = 10.f;
	B.KatanaFinisherDamage = 30.f;
	B.ShurikenDamage = 8.f;
	B.ShurikenSpeed = 2000.f;
	B.ShurikenCooldown = 0.5f;
	B.ShurikenRadius = 10.f;
	B.ShurikenLife = 1.f;
	B.ShurikenExplosiveEvery = 3.f;
	B.ShurikenExplosionRadius = 200.f;
	B.ShurikenExplosionDamage = 20.f;
	B.DashDistance = 400.f;
	B.DashDuration = 0.2f;
	B.DashInvulnerable = 0.3f;
	B.DashCooldown = 1.f;
	SetParryAndUltimate(B);
	B.MemoCount = 5.f;
	B.MemoSpread = 60.f;
	B.MemoSpeed = 600.f;
	B.MemoDamage = 8.f;
	B.MemoRadius = 20.f;
	B.MemoLife = 4.f;
	B.VolleyCooldown = 3.f;
	B.SlamRange = 400.f;
	B.SlamRadius = 250.f;
	B.SlamTelegraph = 1.f;
	B.SlamDamage = 25.f;
	B.SlamCooldown = 5.f;
	SetSlamPattern(B);
	B.BossRecovery = 1.f;
	SetStealNumbers(B);
	return B;
}

/** A fight with the hero at HeroPos and the boss at BossPos, both standing still. */
inline FJamFight Fight(const FJamBalanceData& B, const FVector2D& HeroPos, const FVector2D& BossPos)
{
	FJamFight F = JamFightRules::Start(B, 1);
	F.Fighters[EJamSide::Hero].Pos = F.Fighters[EJamSide::Hero].PrevPos = HeroPos;
	F.Fighters[EJamSide::Boss].Pos = F.Fighters[EJamSide::Boss].PrevPos = BossPos;
	return F;
}

/** Takes every mechanic from the hero: he is the bean with the ball, as at the end of the fight. */
inline void Bean(FJamFight& F)
{
	for (EJamSide& Owner : F.Ledger.Owner)
	{
		Owner = EJamSide::Boss;
	}
}

/** Brings the boss to the threshold of the next steal and puts the card up, as the step that crossed it would. */
inline void OpenSteal(FJamFight& F, const FJamBalanceData& B)
{
	F.Fighters[EJamSide::Boss].Health = F.Steal.Floor;
	JamStealRules::Open(F, B);
}

/** How many events of a type the last step reported. */
inline int32 Count(const FJamFight& F, EJamEvent Type)
{
	int32 Result = 0;
	for (const FJamEvent& Event : F.Events)
	{
		Result += Event.Type == Type ? 1 : 0;
	}
	return Result;
}

/** Runs Steps fixed steps with the same intents and counts the events of a type over all of them. */
inline int32 RunCounting(FJamFight& F, const FJamIntents& Intents, const FJamBalanceData& B, int32 Steps, EJamEvent Type)
{
	int32 Result = 0;
	for (int32 i = 0; i < Steps; ++i)
	{
		JamFightRules::Step(F, Intents, B);
		Result += Count(F, Type);
	}
	return Result;
}

/** Runs Steps fixed steps with the same intents. */
inline void Run(FJamFight& F, const FJamIntents& Intents, const FJamBalanceData& B, int32 Steps)
{
	for (int32 i = 0; i < Steps; ++i)
	{
		JamFightRules::Step(F, Intents, B);
	}
}
} // namespace JamTest

#endif
