#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamLoadoutRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-074: the hero throws 30% more shurikens a second for the same damage a second, and the Publisher's stolen copy
 * keeps exactly what it was. These read the game's own [JamBalance], not the round test numbers. A bug here looks in
 * game like "his shurikens got faster again" or "my shurikens hit harder per second than before".
 *
 * Run: Automation RunTests Jam.ShurikenBalance
 */
namespace
{
FJamBalanceData ShurikenGameBalance()
{
	return JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
}

// The numbers before T-074: the hero's 9 damage every 0.4 s, an explosion every 3rd.
constexpr float HeroDpsBefore = 9.f / 0.4f;
constexpr float BlastsPerSecondBefore = 1.f / (0.4f * 3.f);
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenHisPinTest, "Jam.ShurikenBalance.HisStolenCopyKeepsItsNumbers", JamTest::Flags)

bool FJamShurikenHisPinTest::RunTest(const FString& Parameters)
{
	// Pinned on Sun 15 Nov from session-5n: 0.4 s x1 x1.3, 9 x0.9, 2400 x0.8.
	const FJamBalanceData His = JamLoadoutRules::InHandsOf(EJamSide::Boss, ShurikenGameBalance());
	TestEqual(TEXT("his throws stay 0.52 s apart"), JamSim::Ticks(His.ShurikenCooldown), JamSim::Ticks(0.52f));
	TestEqual(TEXT("his shuriken still deals 8.1"), His.ShurikenDamage, 8.1f, 1e-3f);
	TestEqual(TEXT("and flies at 1920"), His.ShurikenSpeed, 1920.f, 1e-2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamShurikenHeroRateTest, "Jam.ShurikenBalance.HeroThrowsFasterForTheSameDps", JamTest::Flags)

bool FJamShurikenHeroRateTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData Hero = JamLoadoutRules::InHandsOf(EJamSide::Hero, ShurikenGameBalance());
	const float Every = JamSim::Ticks(Hero.ShurikenCooldown) * JamSim::StepDt;
	const float PerSecond = 1.f / Every;
	TestTrue(TEXT("about 30% more throws a second"), PerSecond >= 1.27f / 0.4f && PerSecond <= 1.35f / 0.4f);
	TestTrue(TEXT("the same damage a second, within 2%"), FMath::Abs(Hero.ShurikenDamage * PerSecond / HeroDpsBefore - 1.f) <= 0.02f);
	// The author's answer: keep the explosions a second the same, so the blast comes every 4th instead of every 3rd.
	const float BlastsPerSecond = PerSecond / FMath::RoundToInt32(Hero.ShurikenExplosiveEvery);
	TestTrue(TEXT("about as many blasts a second as before"), FMath::Abs(BlastsPerSecond / BlastsPerSecondBefore - 1.f) <= 0.05f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
