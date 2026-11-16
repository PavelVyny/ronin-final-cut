#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Audio/JamSoundRules.h"
#include "Tests/JamTestHelpers.h"

/**
 * T-028: which sound each step of the fight makes.
 * A bug here looks in game like "the katana is silent", "a volley of the Publisher's is a wall of noise" or "the final
 * blow sounds like any other Ultimate".
 *
 * Run: Automation RunTests Jam.Sound
 */
namespace
{
FJamEvent SoundEvent(EJamEvent Type, EJamSide Side, int32 ProjectileId = 0)
{
	FJamEvent Event;
	Event.Type = Type;
	Event.Side = Side;
	Event.ProjectileId = ProjectileId;
	return Event;
}

FJamProjectile Flying(int32 Id, EJamProjectile Kind)
{
	FJamProjectile Projectile;
	Projectile.Id = Id;
	Projectile.Kind = Kind;
	return Projectile;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundEveryActionTest, "Jam.Sound.EveryActionIsHeard", JamTest::Flags)

bool FJamSoundEveryActionTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	const auto Only = [&F](const FJamEvent& Event) { return JamSoundRules::CuesOf({ Event }, F); };
	const auto Is = [this, &Only](const TCHAR* What, const FJamEvent& Event, EJamCue Cue)
	{
		const TArray<EJamCue> Cues = Only(Event);
		TestTrue(What, Cues.Num() == 1 && Cues[0] == Cue);
	};
	Is(TEXT("a swing slashes"), SoundEvent(EJamEvent::Swing, EJamSide::Hero), EJamCue::KatanaSlash);
	Is(TEXT("the Publisher's swing too"), SoundEvent(EJamEvent::Swing, EJamSide::Boss), EJamCue::KatanaSlash);
	Is(TEXT("an explosion"), SoundEvent(EJamEvent::Explosion, EJamSide::Boss), EJamCue::Explosion);
	Is(TEXT("a slam"), SoundEvent(EJamEvent::Slam, EJamSide::Boss), EJamCue::Slam);
	Is(TEXT("a dash"), SoundEvent(EJamEvent::Dash, EJamSide::Hero), EJamCue::Dash);
	Is(TEXT("a deflection is the parry"), SoundEvent(EJamEvent::Deflected, EJamSide::Hero), EJamCue::Parry);
	Is(TEXT("the Ultimate slash"), SoundEvent(EJamEvent::UltimateSlash, EJamSide::Hero), EJamCue::Ultimate);
	Is(TEXT("a hit on the boss"), SoundEvent(EJamEvent::Hit, EJamSide::Boss), EJamCue::BossHit);
	Is(TEXT("the hero hurt"), SoundEvent(EJamEvent::Hit, EJamSide::Hero), EJamCue::HeroHurt);
	Is(TEXT("the card opens"), SoundEvent(EJamEvent::StealOpened, EJamSide::Boss), EJamCue::StealCard);
	Is(TEXT("a mechanic is taken"), SoundEvent(EJamEvent::StealResolved, EJamSide::Boss), EJamCue::Steal);
	Is(TEXT("the stun gives everything back"), SoundEvent(EJamEvent::Stunned, EJamSide::Boss), EJamCue::EverythingBack);
	TestEqual(
		TEXT("an opened parry window is silent until it deflects"), Only(SoundEvent(EJamEvent::ParryStarted, EJamSide::Hero)).Num(), 0);
	TestEqual(TEXT("so is a projectile that ends"), Only(SoundEvent(EJamEvent::ProjectileGone, EJamSide::Hero)).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundThrowTest, "Jam.Sound.OnlyTheShurikenIsThrownAloud", JamTest::Flags)

bool FJamSoundThrowTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	F.Projectiles.Add(Flying(1, EJamProjectile::Shuriken));
	F.Projectiles.Add(Flying(2, EJamProjectile::Memo));
	F.Projectiles.Add(Flying(3, EJamProjectile::Ball));
	const TArray<EJamCue> Shuriken = JamSoundRules::CuesOf({ SoundEvent(EJamEvent::ProjectileSpawned, EJamSide::Boss, 1) }, F);
	TestTrue(TEXT("a shuriken is thrown with a sound"), Shuriken.Num() == 1 && Shuriken[0] == EJamCue::ShurikenThrow);
	TestEqual(TEXT("a memo of the volley is not"),
		JamSoundRules::CuesOf({ SoundEvent(EJamEvent::ProjectileSpawned, EJamSide::Boss, 2) }, F).Num(), 0);
	TestEqual(TEXT("nor the ball"), JamSoundRules::CuesOf({ SoundEvent(EJamEvent::ProjectileSpawned, EJamSide::Hero, 3) }, F).Num(), 0);
	TestEqual(TEXT("one gone already is not looked for"),
		JamSoundRules::CuesOf({ SoundEvent(EJamEvent::ProjectileSpawned, EJamSide::Hero, 9) }, F).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundOncePerFrameTest, "Jam.Sound.ACueSoundsOncePerFrame", JamTest::Flags)

bool FJamSoundOncePerFrameTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	// The FINAL NOTE takes two mechanics at once, and a frame can hold two steps of hits.
	const TArray<FJamEvent> Events = { SoundEvent(EJamEvent::StealResolved, EJamSide::Boss), SoundEvent(EJamEvent::Hit, EJamSide::Boss),
		SoundEvent(EJamEvent::StealResolved, EJamSide::Boss), SoundEvent(EJamEvent::Hit, EJamSide::Boss) };
	const TArray<EJamCue> Cues = JamSoundRules::CuesOf(Events, F);
	TestEqual(TEXT("two cues, not four"), Cues.Num(), 2);
	TestTrue(TEXT("in the order they came"), Cues.Num() == 2 && Cues[0] == EJamCue::Steal && Cues[1] == EJamCue::BossHit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundFinalBlowTest, "Jam.Sound.TheFinalBlowHasItsOwnSound", JamTest::Flags)

bool FJamSoundFinalBlowTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	F.Phase = EJamPhase::Won;
	const TArray<EJamCue> Cues = JamSoundRules::CuesOf({ SoundEvent(EJamEvent::UltimateSlash, EJamSide::Hero) }, F);
	TestTrue(TEXT("the slash that wins is the final blow, not one more Ultimate"), Cues.Num() == 1 && Cues[0] == EJamCue::FinalBlow);
	return true;
}

/** T-037: the hero hears his Ultimate fill; the Publisher's filling stays quiet. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundUltimateReadyTest, "Jam.Sound.TheHeroHearsHisUltimateFill", JamTest::Flags)

bool FJamSoundUltimateReadyTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	const TArray<EJamCue> Hero = JamSoundRules::CuesOf({ SoundEvent(EJamEvent::UltimateReady, EJamSide::Hero) }, F);
	TestTrue(TEXT("the hero's meter fills aloud"), Hero.Num() == 1 && Hero[0] == EJamCue::UltimateReady);
	TestEqual(
		TEXT("the Publisher's in silence"), JamSoundRules::CuesOf({ SoundEvent(EJamEvent::UltimateReady, EJamSide::Boss) }, F).Num(), 0);
	TestEqual(TEXT("the file of the cue"), FString(JamSoundRules::NameOf(EJamCue::UltimateReady)), FString(TEXT("sfx_ultimate_ready_01")));
	return true;
}

/** T-049: taking the Publisher's cash heal rings a cash register. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundHealTest, "Jam.Sound.TheHealRingsTheTill", JamTest::Flags)

bool FJamSoundHealTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	const TArray<EJamCue> Taken = JamSoundRules::CuesOf({ SoundEvent(EJamEvent::HealTaken, EJamSide::Hero) }, F);
	TestTrue(TEXT("taking it: ka-ching"), Taken.Num() == 1 && Taken[0] == EJamCue::HealCash);
	TestEqual(TEXT("its drop makes no sound of its own"),
		JamSoundRules::CuesOf({ SoundEvent(EJamEvent::HealDropped, EJamSide::Boss) }, F).Num(), 0);
	TestEqual(TEXT("the file of the cue"), FString(JamSoundRules::NameOf(EJamCue::HealCash)), FString(TEXT("sfx_heal_cash_01")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundNamesTest, "Jam.Sound.NamesFollowTheDelivery", JamTest::Flags)

bool FJamSoundNamesTest::RunTest(const FString& Parameters)
{
	TSet<FString> Seen;
	for (int32 Index = 0; Index < JamSoundRules::CueCount; ++Index)
	{
		const FString Name = JamSoundRules::NameOf(static_cast<EJamCue>(Index));
		TestTrue(FString::Printf(TEXT("cue %d is named like Kirill's files: sfx_<what>_01, got '%s'"), Index, *Name),
			Name.StartsWith(TEXT("sfx_")) && Name.EndsWith(TEXT("_01")));
		Seen.Add(Name);
	}
	TestEqual(TEXT("no two cues share a file"), Seen.Num(), JamSoundRules::CueCount);
	TestEqual(TEXT("the katana keeps the name of the plan"), FString(JamSoundRules::NameOf(EJamCue::KatanaSlash)),
		FString(TEXT("sfx_katana_slash_01")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSoundCrackTest, "Jam.Sound.TheCrackIsShortQuietAndDucksUnderTheVoice", JamTest::Flags)

bool FJamSoundCrackTest::RunTest(const FString& Parameters)
{
	// T-085, the author: the crack's sound is too loud and too long and clashes with the voice line.
	const FJamMix Plain = JamSoundRules::MixOf(EJamCue::KatanaSlash, false);
	TestTrue(TEXT("other cues play whole and full"), Plain.Volume == 1.f && Plain.CutAfter == 0.f);
	const FJamMix Crack = JamSoundRules::MixOf(EJamCue::Steal, false);
	TestTrue(TEXT("the crack is much quieter"), Crack.Volume > 0.f && Crack.Volume <= 0.4f);
	TestTrue(TEXT("and much shorter than its 1.2 s"), Crack.CutAfter > 0.f && Crack.CutAfter <= 0.5f);
	const FJamMix Ducked = JamSoundRules::MixOf(EJamCue::Steal, true);
	TestTrue(TEXT("quieter still under a voice line"), Ducked.Volume > 0.f && Ducked.Volume < Crack.Volume);
	TestEqual(TEXT("a voice line does not duck the rest"), JamSoundRules::MixOf(EJamCue::Slam, true).Volume, 1.f);
	return true;
}

#endif
