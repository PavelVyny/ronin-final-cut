#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamDifficultyRules.h"
#include "Sim/JamFightOps.h"
#include "Sim/JamFightSubsystem.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-101: EASY, NORMAL and HARD scale numbers the balance already has. A bug here looks in game like "NORMAL plays
 * differently from the last build", "EASY is no easier" or "a retry forgot the level I picked".
 *
 * Run: Automation RunTests Jam.Difficulty
 */
namespace
{
/** How much a hit of Amount on Target takes in a fresh fight of Balance, before any cut. */
float HitOn(const FJamBalanceData& Balance, EJamSide Target, float Amount)
{
	FJamFight F = JamTest::Fight(Balance, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	const float Before = F.Fighters[Target].Health;
	JamFightOps::Hurt(F, Target, Amount, FVector2D::ZeroVector);
	return Before - F.Fighters[Target].Health;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDifficultyNormalTest, "Jam.Difficulty.NormalLeavesEveryNumberUnchanged", JamTest::Flags)

bool FJamDifficultyNormalTest::RunTest(const FString& Parameters)
{
	for (const FJamBalanceData& B : { JamTest::Balance(), JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection) })
	{
		const FJamBalanceData Normal = JamDifficultyRules::Apply(B, EJamDifficulty::Normal);
		for (TFieldIterator<FFloatProperty> It(FJamBalanceData::StaticStruct()); It; ++It)
		{
			TestEqual(FString::Printf(TEXT("NORMAL keeps %s"), *It->GetName()), It->GetPropertyValue_InContainer(&Normal),
				It->GetPropertyValue_InContainer(&B));
		}
		TestEqual(TEXT("a hit on the Publisher takes what it says"), HitOn(Normal, EJamSide::Boss, 10.f), 10.f);
		TestEqual(TEXT("and one on the hero"), HitOn(Normal, EJamSide::Hero, 10.f), 10.f);
	}
	TestEqual(TEXT("the menu's middle is NORMAL"), JamDifficultyRules::FromIndex(1), EJamDifficulty::Normal);
	TestEqual(TEXT("its first EASY"), JamDifficultyRules::FromIndex(0), EJamDifficulty::Easy);
	TestEqual(TEXT("its last HARD"), JamDifficultyRules::FromIndex(2), EJamDifficulty::Hard);
	TestEqual(TEXT("anything else NORMAL"), JamDifficultyRules::FromIndex(7), EJamDifficulty::Normal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDifficultyScalesTest, "Jam.Difficulty.EasyAndHardApplyTheirScales", JamTest::Flags)

bool FJamDifficultyScalesTest::RunTest(const FString& Parameters)
{
	// The rule tests' round numbers: health 100, heal 25; EASY 1.5, 1.3, 1.5, 0.8 and HARD 0.8, 0.9, 0.7, 1.25.
	const FJamBalanceData B = JamTest::Balance();
	const FJamBalanceData Easy = JamDifficultyRules::Apply(B, EJamDifficulty::Easy);
	TestEqual(TEXT("EASY: half again the health"), Easy.HeroHealth, 150.f);
	TestEqual(TEXT("a heal half again as big"), Easy.HealAmount, 37.5f);
	TestTrue(TEXT("his hits land 1.3 times as hard"), FMath::IsNearlyEqual(HitOn(Easy, EJamSide::Boss, 10.f), 13.f, 0.01f));
	TestTrue(TEXT("the Publisher's at 0.8"), FMath::IsNearlyEqual(HitOn(Easy, EJamSide::Hero, 10.f), 8.f, 0.01f));
	TestEqual(TEXT("a fight on EASY starts with that health"),
		JamTest::Fight(Easy, FVector2D(-500.0, 0.0), FVector2D::ZeroVector).Fighters[EJamSide::Hero].MaxHealth, 150.f);

	const FJamBalanceData Hard = JamDifficultyRules::Apply(B, EJamDifficulty::Hard);
	TestEqual(TEXT("HARD: four fifths of the health"), Hard.HeroHealth, 80.f);
	TestTrue(TEXT("a smaller heal"), FMath::IsNearlyEqual(Hard.HealAmount, 17.5f, 0.01f));
	TestTrue(TEXT("his hits at 0.9"), FMath::IsNearlyEqual(HitOn(Hard, EJamSide::Boss, 10.f), 9.f, 0.01f));
	TestTrue(TEXT("the Publisher's at 1.25"), FMath::IsNearlyEqual(HitOn(Hard, EJamSide::Hero, 10.f), 12.5f, 0.01f));

	FJamFight Cut = JamTest::Fight(Easy, FVector2D(-500.0, 0.0), FVector2D::ZeroVector);
	Cut.Steal.Done = 1;
	Cut.Steal.Floor = 0.f;
	const float Before = Cut.Fighters[EJamSide::Boss].Health;
	JamFightOps::Hurt(Cut, EJamSide::Boss, 10.f, FVector2D::ZeroVector);
	TestTrue(TEXT("after the first cut the level's scale and the cut's multiply"),
		FMath::IsNearlyEqual(Before - Cut.Fighters[EJamSide::Boss].Health, 10.f * 1.3f * B.HeroCutDamageScale, 0.01f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDifficultyRetryTest, "Jam.Difficulty.TheChoiceSurvivesARetry", JamTest::Flags)

bool FJamDifficultyRetryTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData Game = JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
	const FJamDifficultyScales Easy = JamDifficultyRules::ScalesOf(Game, EJamDifficulty::Easy);
	TestTrue(TEXT("EASY is told apart from NORMAL by its health"), Easy.HeroHealth > 1.f);
	UJamFightSubsystem* Host = NewObject<UJamFightSubsystem>(GetTransientPackage());
	Host->SetDifficulty(EJamDifficulty::Easy);
	Host->Restart();
	const float Health = Game.HeroHealth * Easy.HeroHealth;
	TestEqual(TEXT("the fight starts on EASY"), Host->GetFight().Fighters[EJamSide::Hero].MaxHealth, Health);
	Host->Retry();
	TestEqual(TEXT("a retry keeps it"), Host->GetFight().Fighters[EJamSide::Hero].MaxHealth, Health);
	TestEqual(TEXT("with its hits"), Host->GetFight().HeroDamageScale, Easy.HeroDamage);
	Host->Restart();
	TestEqual(TEXT("so does PLAY AGAIN"), Host->GetFight().Fighters[EJamSide::Hero].MaxHealth, Health);
	TestEqual(TEXT("and the Publisher's"), Host->GetFight().BossDamageScale, Easy.BossDamage);
	Host->SetDifficulty(EJamDifficulty::Normal);
	Host->Restart();
	TestEqual(TEXT("NORMAL again: the game's health"), Host->GetFight().Fighters[EJamSide::Hero].MaxHealth, Game.HeroHealth);
	return true;
}

#endif
