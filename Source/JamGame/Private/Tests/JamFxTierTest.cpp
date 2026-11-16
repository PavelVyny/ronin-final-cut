#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "View/JamFxRules.h"

/**
 * T-075: the full-art tier shows its blows with Niagara alone, the flat discs of the plain effects belong to the
 * tiers below. A bug here looks in game like "the full art still flashes a flat red disc under the hero" or "the shape
 * ronin's hits show nothing".
 *
 * Run: Automation RunTests Jam.Fx
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxPlainTierTest, "Jam.Fx.TheFullArtLeavesTheBurstsToNiagara", JamTest::Flags)

bool FJamFxPlainTierTest::RunTest(const FString& Parameters)
{
	const EJamEvent Rich[] = { EJamEvent::Hit, EJamEvent::Slam, EJamEvent::Explosion, EJamEvent::Deflected, EJamEvent::HealTaken };
	for (const EJamEvent Type : Rich)
	{
		const FString Name = FString::Printf(TEXT("event %d"), static_cast<int32>(Type));
		TestFalse(Name + TEXT(": no flat disc in the full art, Niagara shows it"), JamFxRules::DrawsPlain(Type, true));
		TestTrue(Name + TEXT(": the shape ronin and the greybox keep the flat disc"), JamFxRules::DrawsPlain(Type, false));
	}
	// The blade's sweep shows the reach that really hits, the Ultimate's line the lane it cut: gameplay, kept in every tier.
	const EJamEvent Kept[] = { EJamEvent::Swing, EJamEvent::UltimateSlash, EJamEvent::Dash };
	for (const EJamEvent Type : Kept)
	{
		const FString Name = FString::Printf(TEXT("event %d"), static_cast<int32>(Type));
		TestTrue(Name + TEXT(": kept in the full art"), JamFxRules::DrawsPlain(Type, true));
		TestTrue(Name + TEXT(": kept below it"), JamFxRules::DrawsPlain(Type, false));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
