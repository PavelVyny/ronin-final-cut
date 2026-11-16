#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Sim/JamLoadoutRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-007: every mechanic has an owner, and a button does what its owner's loadout says.
 * A bug here looks in game like "the Publisher took my katana and I still swing it" or "a stolen dash still dashes".
 *
 * Run: Automation RunTests Jam.Loadout
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutStartTest, "Jam.Loadout.HeroStartsWithEverything", JamTest::Flags)

bool FJamLoadoutStartTest::RunTest(const FString& Parameters)
{
	const FJamFight F = JamFightRules::Start(JamTest::Balance(), 1);
	for (const EJamMechanic Mechanic : { EJamMechanic::Katana, EJamMechanic::Shuriken, EJamMechanic::Dash })
	{
		TestTrue(TEXT("the hero owns it"), F.Ledger.Has(EJamSide::Hero, Mechanic));
		TestFalse(TEXT("the boss does not"), F.Ledger.Has(EJamSide::Boss, Mechanic));
	}
	TestEqual(
		TEXT("left button is the katana"), JamLoadoutRules::ActionFor(EJamButton::Primary, EJamSide::Hero, F.Ledger), EJamAction::Katana);
	TestEqual(TEXT("right button is the shuriken"), JamLoadoutRules::ActionFor(EJamButton::Secondary, EJamSide::Hero, F.Ledger),
		EJamAction::Shuriken);
	TestEqual(TEXT("the dash button dashes"), JamLoadoutRules::ActionFor(EJamButton::Dash, EJamSide::Hero, F.Ledger), EJamAction::Dash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutBallTest, "Jam.Loadout.StolenAttackButtonFiresTheBall", JamTest::Flags)

bool FJamLoadoutBallTest::RunTest(const FString& Parameters)
{
	FJamLedger Ledger;
	JamLoadoutRules::Give(Ledger, EJamMechanic::Katana, EJamSide::Boss);
	TestEqual(TEXT("no katana: the left button shoots the ball"), JamLoadoutRules::ActionFor(EJamButton::Primary, EJamSide::Hero, Ledger),
		EJamAction::Ball);
	TestEqual(
		TEXT("the shuriken is still his"), JamLoadoutRules::ActionFor(EJamButton::Secondary, EJamSide::Hero, Ledger), EJamAction::Shuriken);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	TestEqual(TEXT("no shuriken: the right button shoots the ball too"),
		JamLoadoutRules::ActionFor(EJamButton::Secondary, EJamSide::Hero, Ledger), EJamAction::Ball);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutDashTest, "Jam.Loadout.StolenDashDoesNothing", JamTest::Flags)

bool FJamLoadoutDashTest::RunTest(const FString& Parameters)
{
	const FJamBalanceData B = JamTest::Balance();
	FJamFight F = JamTest::Fight(B, FVector2D::ZeroVector, FVector2D(-800.0, 0.0));
	JamLoadoutRules::Give(F.Ledger, EJamMechanic::Dash, EJamSide::Boss);
	TestEqual(TEXT("the button has no action"), JamLoadoutRules::ActionFor(EJamButton::Dash, EJamSide::Hero, F.Ledger), EJamAction::None);
	FJamIntents Intents;
	Intents[EJamSide::Hero].Aim.Dir = FVector2D(0.0, 1.0);
	Intents[EJamSide::Hero].bDash = true;
	const int32 Dashes = JamTest::RunCounting(F, Intents, B, 10, EJamEvent::Dash);
	TestEqual(TEXT("no dash starts"), Dashes, 0);
	TestTrue(TEXT("the hero stays where he was"), F.Fighters[EJamSide::Hero].Pos.IsNearlyZero());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutBossTest, "Jam.Loadout.BossOwnsWhatWasGiven", JamTest::Flags)

bool FJamLoadoutBossTest::RunTest(const FString& Parameters)
{
	FJamLedger Ledger;
	TestEqual(TEXT("the boss has no katana of his own, and no ball"),
		JamLoadoutRules::ActionFor(EJamButton::Primary, EJamSide::Boss, Ledger), EJamAction::None);
	TestEqual(TEXT("no shuriken"), JamLoadoutRules::ActionFor(EJamButton::Secondary, EJamSide::Boss, Ledger), EJamAction::None);
	TestEqual(TEXT("no dash"), JamLoadoutRules::ActionFor(EJamButton::Dash, EJamSide::Boss, Ledger), EJamAction::None);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Katana, EJamSide::Boss);
	TestEqual(TEXT("a given katana is his"), JamLoadoutRules::ActionFor(EJamButton::Primary, EJamSide::Boss, Ledger), EJamAction::Katana);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutShurikenGapTest, "Jam.Loadout.HisShurikensComeSlowerThanTheHeros", JamTest::Flags)

bool FJamLoadoutShurikenGapTest::RunTest(const FString& Parameters)
{
	// T-070: "his stolen shurikens come one after another and are almost impossible to dodge".
	FJamBalanceData B = JamTest::Balance();
	B.ShurikenCooldown = 0.4f;
	B.BossShurikenCooldown = 0.4f;
	B.DashCooldown = 1.f;
	B.BossStolenCooldown = 1.f;
	B.BossShurikenCooldownScale = 1.3f;
	const FJamBalanceData His = JamLoadoutRules::InHandsOf(EJamSide::Boss, B);
	TestTrue(TEXT("his shurikens wait 30% longer"), FMath::IsNearlyEqual(His.ShurikenCooldown, 0.52f));
	TestTrue(TEXT("the scale is for the shuriken only"), FMath::IsNearlyEqual(His.DashCooldown, 1.f));
	TestTrue(TEXT("the hero's cooldown stays"), FMath::IsNearlyEqual(JamLoadoutRules::InHandsOf(EJamSide::Hero, B).ShurikenCooldown, 0.4f));
	B.BossStolenCooldown = 2.f;
	TestTrue(TEXT("on top of the scale of every stolen move"),
		FMath::IsNearlyEqual(JamLoadoutRules::InHandsOf(EJamSide::Boss, B).ShurikenCooldown, 1.04f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLoadoutGiveTest, "Jam.Loadout.GiveMovesExactlyOneMechanic", JamTest::Flags)

bool FJamLoadoutGiveTest::RunTest(const FString& Parameters)
{
	FJamLedger Ledger;
	JamLoadoutRules::Give(Ledger, EJamMechanic::Shuriken, EJamSide::Boss);
	TestTrue(TEXT("the shuriken moved"), Ledger.Has(EJamSide::Boss, EJamMechanic::Shuriken));
	TestFalse(TEXT("the hero has it no more"), Ledger.Has(EJamSide::Hero, EJamMechanic::Shuriken));
	TestTrue(TEXT("the katana stayed"), Ledger.Has(EJamSide::Hero, EJamMechanic::Katana));
	TestTrue(TEXT("the dash stayed"), Ledger.Has(EJamSide::Hero, EJamMechanic::Dash));
	EJamMechanic Next = EJamMechanic::Dash;
	TestTrue(TEXT("the hero still owns something"), JamLoadoutRules::FirstOwned(Ledger, EJamSide::Hero, Next));
	TestEqual(TEXT("the first of it is the katana"), Next, EJamMechanic::Katana);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Katana, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Dash, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Parry, EJamSide::Boss);
	JamLoadoutRules::Give(Ledger, EJamMechanic::Ultimate, EJamSide::Boss);
	TestFalse(TEXT("with everything given away the hero owns nothing"), JamLoadoutRules::FirstOwned(Ledger, EJamSide::Hero, Next));
	return true;
}

#endif
