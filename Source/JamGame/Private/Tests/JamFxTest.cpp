#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "Sim/JamHazardRules.h"
#include "View/JamFxRules.h"

/**
 * T-069: the rich effects of the full-art tier. A bug here looks in game like "the sparks of my blade are his red",
 * "the slam's dust stops short of its circle" or "the greybox still throws sparks".
 *
 * Run: Automation RunTests Jam.Fx
 */
namespace
{
FJamEvent FxEvent(EJamEvent Type, EJamSide Side, const FVector2D& Pos, float Amount = 0.f)
{
	FJamEvent E;
	E.Type = Type;
	E.Side = Side;
	E.Pos = Pos;
	E.Amount = Amount;
	return E;
}

FJamFight FxFight()
{
	FJamFight F;
	F.Fighters[EJamSide::Hero].Pos = FVector2D(-300.0, 0.0);
	F.Fighters[EJamSide::Boss].Pos = FVector2D(400.0, 100.0);
	return F;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxHitTest, "Jam.Fx.HitsSparkInTheAttackersColour", JamTest::Flags)

bool FJamFxHitTest::RunTest(const FString& Parameters)
{
	const FJamFight F = FxFight();
	TArray<FJamFxCue> Cues;
	const FJamEvent OnBoss[] = { FxEvent(EJamEvent::Hit, EJamSide::Boss, FVector2D::ZeroVector, 14.f) };
	JamFxRules::CuesFor(F, OnBoss, true, Cues);
	if (TestEqual(TEXT("a hit on the Publisher sparks once"), Cues.Num(), 1))
	{
		TestEqual(TEXT("sparks"), Cues[0].Kind, EJamFx::Sparks);
		TestEqual(TEXT("on him"), Cues[0].Pos, F.Fighters[EJamSide::Boss].Pos);
		TestTrue(TEXT("in the hero's neon"), Cues[0].Color.Equals(JamFxRules::HeroNeon));
		TestTrue(TEXT("above the floor"), Cues[0].Height > 50.f);
	}
	Cues.Reset();
	const FJamEvent Heavy[] = { FxEvent(EJamEvent::Hit, EJamSide::Boss, FVector2D::ZeroVector, 30.f) };
	JamFxRules::CuesFor(F, Heavy, true, Cues);
	TArray<FJamFxCue> Light;
	JamFxRules::CuesFor(F, OnBoss, true, Light);
	if (TestTrue(TEXT("both spark"), Cues.Num() == 1 && Light.Num() == 1))
	{
		TestTrue(TEXT("a finisher sparks bigger than a strike"), Cues[0].Size > Light[0].Size);
	}
	Cues.Reset();
	const FJamEvent OnHero[] = { FxEvent(EJamEvent::Hit, EJamSide::Hero, FVector2D::ZeroVector, 8.f) };
	JamFxRules::CuesFor(F, OnHero, true, Cues);
	TestTrue(TEXT("a hit on the hero sparks in the Publisher's red"), Cues.Num() == 1 && Cues[0].Color.Equals(JamFxRules::BossNeon));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxGroundTest, "Jam.Fx.SlamsAndBlastsRaiseDustToTheirRadius", JamTest::Flags)

bool FJamFxGroundTest::RunTest(const FString& Parameters)
{
	const FJamFight F = FxFight();
	TArray<FJamFxCue> Cues;
	const FJamEvent Slam[] = { FxEvent(EJamEvent::Slam, EJamSide::Boss, FVector2D(50.0, 60.0), 300.f) };
	JamFxRules::CuesFor(F, Slam, true, Cues);
	const FJamFxCue* Dust = Cues.FindByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Dust; });
	if (TestNotNull(TEXT("a slam raises dust"), Dust))
	{
		TestEqual(TEXT("where it landed"), Dust->Pos, FVector2D(50.0, 60.0));
		TestEqual(TEXT("out to its circle"), Dust->Size, 300.f);
	}
	Cues.Reset();
	const FJamEvent Blast[] = { FxEvent(EJamEvent::Explosion, EJamSide::Hero, FVector2D(10.0, 20.0), 230.f) };
	JamFxRules::CuesFor(F, Blast, true, Cues);
	TestEqual(TEXT("a blast sparks and raises dust"), Cues.Num(), 2);
	TestTrue(TEXT("its sparks are yellow"),
		Cues.ContainsByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Sparks && C.Color.Equals(JamFxRules::BlastYellow); }));
	TestTrue(TEXT("its dust reaches its radius"),
		Cues.ContainsByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Dust && C.Size == 230.f; }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxMoreTest, "Jam.Fx.DeflectsCashAndTheUltimate", JamTest::Flags)

bool FJamFxMoreTest::RunTest(const FString& Parameters)
{
	const FJamFight F = FxFight();
	TArray<FJamFxCue> Cues;
	const FJamEvent Events[] = { FxEvent(EJamEvent::Deflected, EJamSide::Hero, FVector2D(1.0, 2.0)),
		FxEvent(EJamEvent::HealTaken, EJamSide::Hero, FVector2D(3.0, 4.0), 25.f),
		FxEvent(EJamEvent::UltimateSlash, EJamSide::Hero, FVector2D(5.0, 6.0)),
		FxEvent(EJamEvent::Dash, EJamSide::Hero, FVector2D(7.0, 8.0)) };
	JamFxRules::CuesFor(F, Events, true, Cues);
	TestEqual(TEXT("a deflect, a heal and the Ultimate start one each; a dash none (its afterimages show it)"), Cues.Num(), 3);
	TestTrue(TEXT("the deflect sparks white"),
		Cues.ContainsByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Sparks && C.Color.Equals(FLinearColor::White); }));
	TestTrue(TEXT("the heal bursts cash where it was taken"),
		Cues.ContainsByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Cash && C.Pos == FVector2D(3.0, 4.0); }));
	TestTrue(TEXT("the Ultimate sparks big in magenta"),
		Cues.ContainsByPredicate(
			[](const FJamFxCue& C) { return C.Kind == EJamFx::Sparks && C.Color.Equals(JamFxRules::UltimateMagenta) && C.Size > 2.f; }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxTierTest, "Jam.Fx.NothingRichOnceTheArtIsCut", JamTest::Flags)

bool FJamFxTierTest::RunTest(const FString& Parameters)
{
	const FJamFight F = FxFight();
	TArray<FJamFxCue> Cues;
	const FJamEvent Events[] = { FxEvent(EJamEvent::Hit, EJamSide::Boss, FVector2D::ZeroVector, 14.f),
		FxEvent(EJamEvent::Slam, EJamSide::Boss, FVector2D::ZeroVector, 300.f) };
	JamFxRules::CuesFor(F, Events, false, Cues);
	TestEqual(TEXT("the medium and greybox tiers start no rich effect"), Cues.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxChargeLookTest, "Jam.Fx.TheChargeLooksDegradeWithTheCuts", JamTest::Flags)

bool FJamFxChargeLookTest::RunTest(const FString& Parameters)
{
	// T-072: "the ultimate charge and the dash look low-poly in the full-art tier".
	FJamLayers Shown;
	TestEqual(TEXT("the full art: an aura on the hero, no bars"), JamFxRules::ChargeLook(Shown), EJamChargeLook::Aura);
	Shown.bArt = false;
	TestEqual(TEXT("one tier down, the shape ronin: a simpler glow"), JamFxRules::ChargeLook(Shown), EJamChargeLook::Glow);
	Shown.bEffects = false;
	Shown.bMaterials = false;
	TestEqual(TEXT("two tiers down, greybox: the bars come back"), JamFxRules::ChargeLook(Shown), EJamChargeLook::Bars);
	Shown.bModel = false;
	Shown.bHud = false;
	TestEqual(TEXT("and stay to the end"), JamFxRules::ChargeLook(Shown), EJamChargeLook::Bars);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxAuraTest, "Jam.Fx.TheAuraShowsOnlyWhenFull", JamTest::Flags)

bool FJamFxAuraTest::RunTest(const FString& Parameters)
{
	// T-073, the author on session-5n: the aura says nothing while the meter fills; the meter bar does that again, and
	// the aura is the "ready" cue alone.
	TestFalse(TEXT("no aura before he owns the Ultimate"), JamFxRules::Aura(1.f, false, false).bOn);
	TestFalse(TEXT("none on an empty meter"), JamFxRules::Aura(0.f, true, false).bOn);
	TestFalse(TEXT("none half way"), JamFxRules::Aura(0.5f, true, false).bOn);
	TestFalse(TEXT("none one percent short"), JamFxRules::Aura(0.99f, true, false).bOn);
	const FJamAura Full = JamFxRules::Aura(1.f, true, false);
	const FJamAura WindUp = JamFxRules::Aura(1.f, true, true);
	TestTrue(TEXT("on and ready when full"), Full.bOn && Full.bReady && Full.Size > 0.f && Full.Rate > 0.f);
	TestTrue(TEXT("its biggest while he winds the cut up"), WindUp.bOn && WindUp.Size > Full.Size && WindUp.Rate > Full.Rate);
	TestTrue(TEXT("the wind-up shows even if the meter was spent on it"), JamFxRules::Aura(0.f, true, true).bOn);
	TestTrue(TEXT("an overfull meter reads as full"), FMath::IsNearlyEqual(JamFxRules::Aura(1.5f, true, false).Size, Full.Size));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxTelegraphTest, "Jam.Fx.TheRuneTelegraphMatchesTheHit", JamTest::Flags)

bool FJamFxTelegraphTest::RunTest(const FString& Parameters)
{
	// The session-5s playtest: the slam's circles get a rune telegraph in the full art; what glows is what hits.
	FJamHazard Circle;
	Circle.Pos = FVector2D(120.0, -40.0);
	Circle.Radius = 180.f;
	Circle.StartTick = 100;
	Circle.HitTick = 160;
	const FJamTelegraph Start = JamFxRules::TelegraphOf(Circle, 100.f);
	TestEqual(TEXT("the ring is the hit's own size"), Start.Radius, Circle.Radius);
	TestTrue(TEXT("on the hit's own spot"), Start.Pos.Equals(Circle.Pos));
	TestEqual(TEXT("empty when it appears"), Start.Fill, 0.f);
	TestEqual(TEXT("half full half way"), JamFxRules::TelegraphOf(Circle, 130.f).Fill, 0.5f);
	TestEqual(TEXT("full on the step it hits"), JamFxRules::TelegraphOf(Circle, 160.f).Fill, 1.f);
	const FVector2D Edge = Circle.Pos + FVector2D(Start.Radius - 1.0, 0.0);
	const FVector2D Past = Circle.Pos + FVector2D(Start.Radius + 1.0, 0.0);
	TestTrue(TEXT("just inside the ring is hit"), JamHazardRules::Hits(Circle, Edge));
	TestFalse(TEXT("just outside it is safe"), JamHazardRules::Hits(Circle, Past));
	FJamLayers Shown;
	TestEqual(TEXT("the full art: the rune"), JamFxRules::TelegraphLook(Shown), EJamTelegraphLook::Rune);
	Shown.bArt = false;
	TestEqual(TEXT("after the first cut: the plain red circle"), JamFxRules::TelegraphLook(Shown), EJamTelegraphLook::Plain);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxSlamBurstTest, "Jam.Fx.ASlamLandsWithASharpBurst", JamTest::Flags)

bool FJamFxSlamBurstTest::RunTest(const FString& Parameters)
{
	// The session-5s playtest: the rune telegraph ends in a hot burst of sparks off the floor where the slam lands.
	const FJamFight F = FxFight();
	TArray<FJamFxCue> Cues;
	const FJamEvent Slam[] = { FxEvent(EJamEvent::Slam, EJamSide::Boss, FVector2D(50.0, 60.0), 300.f) };
	JamFxRules::CuesFor(F, Slam, true, Cues);
	TestEqual(TEXT("dust and a burst"), Cues.Num(), 2);
	const FJamFxCue* Burst = Cues.FindByPredicate([](const FJamFxCue& C) { return C.Kind == EJamFx::Sparks; });
	if (TestNotNull(TEXT("a burst of sparks"), Burst))
	{
		TestEqual(TEXT("where it landed"), Burst->Pos, FVector2D(50.0, 60.0));
		TestTrue(TEXT("low, off the floor"), Burst->Height < 60.f);
		TestTrue(TEXT("in the slam's hot orange"), Burst->Color.Equals(JamFxRules::SlamOrange));
		TestTrue(TEXT("bigger than a plain hit's"), Burst->Size > 1.f);
	}
	Cues.Reset();
	JamFxRules::CuesFor(F, Slam, false, Cues);
	TestEqual(TEXT("none without the full art"), Cues.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
