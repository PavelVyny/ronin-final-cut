#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Sim/JamLayerRules.h"
#include "Sim/JamStealRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-023: which cosmetic layers of the hero and the world are left after each of the Publisher's cuts.
 * A bug here looks in game like "the hero is still a polished ronin after the final note" or "the world stays grey
 * after the stun gave everything back".
 *
 * Run: Automation RunTests Jam.Layers
 */
namespace
{
/** One layer per cut, the model last: four cuts. */
FJamLayerSettings OneLayerPerCut()
{
	FJamLayerSettings S;
	// T-067: the full art goes with the first cut too.
	S.ArtLostAt = 1;
	S.EffectsLostAt = 1;
	// The session-5s playtest: the hero keeps his 3D model one cut longer than the world.
	S.HeroArtLostAt = 2;
	S.MaterialsLostAt = 2;
	S.AnimationLostAt = 3;
	S.ModelLostAt = 4;
	S.HudLostAt = 4;
	S.StemsLeft = { 4, 3, 2, 1, 0 };
	return S;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersFullTest, "Jam.Layers.FullBeforeTheFirstCut", JamTest::Flags)

bool FJamLayersFullTest::RunTest(const FString& Parameters)
{
	const FJamLayers L = JamLayerRules::For(0, EJamPhase::Fight, OneLayerPerCut());
	TestEqual(TEXT("every layer is on"), L.Count(), FJamLayers::Total);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersEachCutTest, "Jam.Layers.EachCutDropsItsLayers", JamTest::Flags)

bool FJamLayersEachCutTest::RunTest(const FString& Parameters)
{
	const FJamLayerSettings S = OneLayerPerCut();
	const FJamLayers First = JamLayerRules::For(1, EJamPhase::Fight, S);
	TestFalse(TEXT("cut 1 takes the effects"), First.bEffects);
	TestTrue(TEXT("cut 1 leaves the materials"), First.bMaterials);

	const FJamLayers Second = JamLayerRules::For(2, EJamPhase::Steal, S);
	TestFalse(TEXT("cut 2 takes the materials"), Second.bMaterials);
	TestTrue(TEXT("cut 2 leaves the animation"), Second.bAnimation);

	const FJamLayers Third = JamLayerRules::For(3, EJamPhase::Fight, S);
	TestFalse(TEXT("cut 3 takes the animation"), Third.bAnimation);
	TestTrue(TEXT("cut 3 leaves the model"), Third.bModel);
	TestTrue(TEXT("cut 3 leaves the HUD"), Third.bHud);
	// Three cuts take five layers: the full art went with the effects at the first (T-067), the hero's with the
	// materials at the second.
	TestEqual(TEXT("a lost layer stays lost"), Third.Count(), FJamLayers::Total - 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersBeanTest, "Jam.Layers.BeanAfterTheLastCut", JamTest::Flags)

bool FJamLayersBeanTest::RunTest(const FString& Parameters)
{
	const FJamLayers L = JamLayerRules::For(4, EJamPhase::Fight, OneLayerPerCut());
	TestEqual(TEXT("nothing is left"), L.Count(), 0);
	TestEqual(TEXT("a death keeps the look of the fight"), JamLayerRules::For(4, EJamPhase::HeroDead, OneLayerPerCut()).Count(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersReturnTest, "Jam.Layers.EverythingReturnsAtTheStun", JamTest::Flags)

bool FJamLayersReturnTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("stunned"), JamLayerRules::For(4, EJamPhase::Stunned, OneLayerPerCut()).Count(), FJamLayers::Total);
	TestEqual(TEXT("won"), JamLayerRules::For(4, EJamPhase::Won, OneLayerPerCut()).Count(), FJamLayers::Total);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersIniTest, "Jam.Layers.IniWalksTheWholeRoad", JamTest::Flags)

bool FJamLayersIniTest::RunTest(const FString& Parameters)
{
	TArray<FString> Missing;
	const FJamLayerSettings S = JamConfig::Load<FJamLayerSettings>(JamLayerRules::Section, &Missing);
	TestEqual(TEXT("every layer field has a line in DefaultGame.ini, missing: ") + FString::Join(Missing, TEXT(", ")), Missing.Num(), 0);

	// "Never cut": the way from the full game down to the bean, at least four layers, the bean at the last cut.
	int32 Steps = 0;
	int32 Before = JamLayerRules::For(0, EJamPhase::Fight, S).Count();
	for (int32 Cut = 1; Cut <= JamStealRules::ThresholdCount; ++Cut)
	{
		const int32 After = JamLayerRules::For(Cut, EJamPhase::Fight, S).Count();
		TestTrue(FString::Printf(TEXT("cut %d takes something"), Cut), After < Before);
		Steps += After < Before ? 1 : 0;
		Before = After;
	}
	TestEqual(TEXT("every cut is a step down"), Steps, JamStealRules::ThresholdCount);
	TestEqual(TEXT("the last cut leaves the bean"), Before, 0);
	TestTrue(TEXT("at least four layers on the road"), FJamLayers::Total >= 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersMusicTest, "Jam.Layers.MusicLosesAStemPerCut", JamTest::Flags)

bool FJamLayersMusicTest::RunTest(const FString& Parameters)
{
	const FJamLayerSettings S = OneLayerPerCut();
	TestEqual(TEXT("the whole band before the first cut"), JamLayerRules::StemsFor(0, EJamPhase::Fight, S), JamLayerRules::StemCount);
	TestEqual(TEXT("cut 1"), JamLayerRules::StemsFor(1, EJamPhase::Fight, S), 3);
	TestEqual(TEXT("cut 2, card up"), JamLayerRules::StemsFor(2, EJamPhase::Steal, S), 2);
	TestEqual(TEXT("cut 4: only the metronome"), JamLayerRules::StemsFor(4, EJamPhase::Fight, S), 0);
	TestEqual(TEXT("more cuts than lines keep the last line"), JamLayerRules::StemsFor(9, EJamPhase::Fight, S), 0);
	TestEqual(
		TEXT("no lines: the whole band"), JamLayerRules::StemsFor(2, EJamPhase::Fight, FJamLayerSettings()), JamLayerRules::StemCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersMusicReturnTest, "Jam.Layers.MusicReturnsAtTheStun", JamTest::Flags)

bool FJamLayersMusicReturnTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("stunned"), JamLayerRules::StemsFor(4, EJamPhase::Stunned, OneLayerPerCut()), JamLayerRules::StemCount);
	TestEqual(TEXT("won"), JamLayerRules::StemsFor(4, EJamPhase::Won, OneLayerPerCut()), JamLayerRules::StemCount);
	TestEqual(TEXT("dead keeps the music of the fight"), JamLayerRules::StemsFor(1, EJamPhase::HeroDead, OneLayerPerCut()), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersMusicIniTest, "Jam.Layers.IniMusicEndsInTheMetronome", JamTest::Flags)

bool FJamLayersMusicIniTest::RunTest(const FString& Parameters)
{
	const FJamLayerSettings S = JamConfig::Load<FJamLayerSettings>(JamLayerRules::Section);
	TestEqual(TEXT("a line before the first cut and one after each"), S.StemsLeft.Num(), JamStealRules::ThresholdCount + 1);
	if (S.StemsLeft.Num() == 0)
	{
		return true;
	}
	TestEqual(TEXT("the fight starts with the whole band"), S.StemsLeft[0], JamLayerRules::StemCount);
	TestEqual(TEXT("the last cut leaves the metronome"), S.StemsLeft.Last(), 0);
	for (int32 Index = 1; Index < S.StemsLeft.Num(); ++Index)
	{
		TestTrue(FString::Printf(TEXT("cut %d does not add a stem"), Index), S.StemsLeft[Index] <= S.StemsLeft[Index - 1]);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersArtTest, "Jam.Layers.TheArtGoesWithTheFirstCut", JamTest::Flags)

bool FJamLayersArtTest::RunTest(const FString& Parameters)
{
	// T-067: three tiers. The full art (the 3D hero, the rich effects) goes first; the plain effects stay one cut longer.
	FJamLayerSettings S = OneLayerPerCut();
	S.ArtLostAt = 1;
	S.EffectsLostAt = 2;
	TestTrue(TEXT("the art is there before the first cut"), JamLayerRules::For(0, EJamPhase::Fight, S).bArt);
	const FJamLayers First = JamLayerRules::For(1, EJamPhase::Fight, S);
	TestFalse(TEXT("the first cut takes the art"), First.bArt);
	TestTrue(TEXT("the plain effects stay"), First.bEffects);
	TestTrue(TEXT("the model stays: the shape ronin"), First.bModel);
	TestFalse(TEXT("the second cut takes the plain effects"), JamLayerRules::For(2, EJamPhase::Fight, S).bEffects);
	TestTrue(TEXT("the stun gives the art back"), JamLayerRules::For(3, EJamPhase::Stunned, S).bArt);
	TestFalse(TEXT("a change of the art alone is a change"), First == JamLayerRules::For(0, EJamPhase::Fight, S));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLayersHeroArtTest, "Jam.Layers.TheHeroKeepsHisLookOneCutLonger", JamTest::Flags)

bool FJamLayersHeroArtTest::RunTest(const FString& Parameters)
{
	// The session-5s playtest: at the first cut the world degrades, but the hero keeps his 3D model and his dash's
	// afterimages; he becomes the shape ronin at the second cut.
	const FJamLayerSettings S = JamConfig::Load<FJamLayerSettings>(JamLayerRules::Section);
	const FJamLayers Before = JamLayerRules::For(0, EJamPhase::Fight, S);
	TestTrue(TEXT("both arts before the first cut"), Before.bArt && Before.bHeroArt);
	TestTrue(TEXT("the hero is drawn in his art"), JamLayerRules::HeroLayers(Before).bArt);
	const FJamLayers First = JamLayerRules::For(1, EJamPhase::Fight, S);
	TestFalse(TEXT("the first cut takes the world's art"), First.bArt);
	TestTrue(TEXT("but not the hero's"), First.bHeroArt);
	const FJamLayers HeroFirst = JamLayerRules::HeroLayers(First);
	TestTrue(TEXT("the hero is still the 3D ronin after the first cut"), HeroFirst.bArt && HeroFirst.bModel);
	TestTrue(TEXT("with his dash's afterimages"), HeroFirst.bEffects && HeroFirst.bAnimation);
	const FJamLayers Second = JamLayerRules::For(2, EJamPhase::Fight, S);
	TestFalse(TEXT("the second cut takes the hero's art"), Second.bHeroArt);
	TestFalse(TEXT("he is the shape ronin then"), JamLayerRules::HeroLayers(Second).bArt);
	TestTrue(TEXT("the stun gives it back"), JamLayerRules::For(3, EJamPhase::Stunned, S).bHeroArt);
	FJamLayerSettings Early = S;
	Early.HeroArtLostAt = 1;
	TestFalse(TEXT("a change of the hero's art alone is a change"), First == JamLayerRules::For(1, EJamPhase::Fight, Early));
	return true;
}

#endif
