#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/JamConfig.h"
#include "Tests/JamTestHelpers.h"
#include "View/JamNeonRules.h"

/**
 * T-027: the neon look of the stand-ins: the floor lines pulse with the music, the bloom goes with the effects layer.
 * A bug here looks in game like "the floor flickers off the beat", "the lines go dark between beats" or "the world
 * still glows after the Publisher took the effects".
 *
 * Run: Automation RunTests Jam.Neon
 */
namespace
{
FJamNeonSettings TestNeon()
{
	FJamNeonSettings S;
	S.Bpm = 120.f;
	S.LineGlow = 2.f;
	S.BeatPulse = 3.f;
	S.BeatDecay = 0.1f;
	S.PostGlow = 4.f;
	S.EdgeGlow = 5.f;
	S.EffectGlow = 3.f;
	S.Bloom = 1.5f;
	S.BloomCut = 0.2f;
	return S;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonBeatTest, "Jam.Neon.LinesPulseOnTheBeat", JamTest::Flags)

bool FJamNeonBeatTest::RunTest(const FString& Parameters)
{
	FJamNeonSettings S = TestNeon();
	TestEqual(TEXT("brightest on the first beat"), JamNeonRules::LineGlow(0.f, S), 5.f);
	TestEqual(TEXT("and on every beat after it (120 BPM: every half second)"), JamNeonRules::LineGlow(1.5f, S), 5.f, 0.001f);
	const float Soon = JamNeonRules::LineGlow(0.05f, S);
	TestTrue(TEXT("falling right after a beat"), Soon < 5.f && Soon > 2.f);
	TestEqual(TEXT("back near the base just before the next"), JamNeonRules::LineGlow(0.49f, S), 2.f, 0.1f);
	TestTrue(TEXT("never below the base"), JamNeonRules::LineGlow(0.3f, S) >= 2.f);
	S.Bpm = 0.f;
	TestEqual(TEXT("no tempo, no pulse: a steady glow"), JamNeonRules::LineGlow(0.f, S), 2.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonBloomTest, "Jam.Neon.BloomGoesWithTheEffects", JamTest::Flags)

bool FJamNeonBloomTest::RunTest(const FString& Parameters)
{
	const FJamNeonSettings S = TestNeon();
	FJamLayers Full;
	TestEqual(TEXT("the full look blooms"), JamNeonRules::Bloom(Full, S), 1.5f);
	FJamLayers Cut = Full;
	Cut.bEffects = false;
	TestEqual(TEXT("without the effects the bloom is cut down"), JamNeonRules::Bloom(Cut, S), 0.2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonLightTest, "Jam.Neon.TheLightFallsWithTheTiers", JamTest::Flags)

bool FJamNeonLightTest::RunTest(const FString& Parameters)
{
	// T-077: "the top tier should be as beautiful as we can make it so the fall to the lower tiers is felt".
	FJamNeonSettings S = TestNeon();
	S.BloomFlat = 0.4f;
	FJamLayers Shown;
	TestEqual(TEXT("the full art: Lumen, neon lights and the grade"), JamNeonRules::LightFor(Shown), EJamLight::Neon);
	TestEqual(TEXT("and the full bloom"), JamNeonRules::Bloom(Shown, S), 1.5f);
	Shown.bArt = false;
	TestEqual(TEXT("the shape ronin: a flat light"), JamNeonRules::LightFor(Shown), EJamLight::Flat);
	TestEqual(TEXT("and a hint of bloom"), JamNeonRules::Bloom(Shown, S), 0.4f);
	Shown.bEffects = false;
	Shown.bMaterials = false;
	TestEqual(TEXT("greybox: the bare engine look"), JamNeonRules::LightFor(Shown), EJamLight::Bare);
	TestEqual(TEXT("with the bloom cut"), JamNeonRules::Bloom(Shown, S), 0.2f);
	Shown.bModel = false;
	Shown.bHud = false;
	TestEqual(TEXT("and it stays bare to the end"), JamNeonRules::LightFor(Shown), EJamLight::Bare);
	FJamLayers Odd;
	Odd.bEffects = false;
	TestEqual(TEXT("no effects means bare, whatever the art says"), JamNeonRules::LightFor(Odd), EJamLight::Bare);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonIniTest, "Jam.Neon.IniIsComplete", JamTest::Flags)

bool FJamNeonIniTest::RunTest(const FString& Parameters)
{
	TArray<FString> Missing;
	const FJamNeonSettings S = JamConfig::Load<FJamNeonSettings>(JamNeonRules::Section, &Missing);
	TestEqual(TEXT("every neon field has a line in DefaultGame.ini, missing: ") + FString::Join(Missing, TEXT(", ")), Missing.Num(), 0);
	TestTrue(TEXT("the lines pulse to the 120 BPM of the stems"), FMath::IsNearlyEqual(S.Bpm, 120.f));
	TestTrue(TEXT("the cut look blooms less than the full one"), S.BloomCut < S.Bloom);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNeonFilmTest, "Jam.Neon.TheFilmLookIsTheFullArtsOnly", JamTest::Flags)

bool FJamNeonFilmTest::RunTest(const FString& Parameters)
{
	// T-094: grain, fringes, rain haze and a soft distance over the full art, so the generated textures read as atmosphere.
	const FJamNeonSettings S = JamConfig::Load<FJamNeonSettings>(JamNeonRules::Section);
	const FJamFilmLook Full = JamNeonRules::FilmLookFor(EJamLight::Neon, S);
	TestTrue(TEXT("the full art has grain"), Full.Grain > 0.f);
	TestTrue(TEXT("fringes"), Full.Fringe > 0.f);
	TestTrue(TEXT("only at the edges"), Full.FringeStart > 0.f);
	TestTrue(TEXT("haze"), Full.FogDensity > 0.f && Full.FogOpacity > 0.f && Full.FogOpacity < 1.f);
	TestTrue(TEXT("a soft distance"), Full.FarBlurKm > 0.f && Full.FarBlurRadius > 0.f);
	TestTrue(TEXT("a light touch: the grain is fine"), Full.Grain <= 0.4f);
	TestTrue(TEXT("the fighters stay clear: the haze starts past them"), Full.FogStart >= 1500.f);
	for (const EJamLight Lower : { EJamLight::Flat, EJamLight::Bare })
	{
		const FJamFilmLook None = JamNeonRules::FilmLookFor(Lower, S);
		TestTrue(TEXT("a lower tier stays clean"),
			None.Grain == 0.f && None.Fringe == 0.f && None.FogDensity == 0.f && None.FarBlurRadius == 0.f);
	}
	return true;
}

#endif
