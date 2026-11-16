#include "Audio/JamSoundRules.h"

namespace
{
const TCHAR* const CueNames[] = {
	TEXT("sfx_katana_slash_01"),
	TEXT("sfx_shuriken_throw_01"),
	TEXT("sfx_explosion_01"),
	TEXT("sfx_slam_01"),
	TEXT("sfx_dash_01"),
	TEXT("sfx_parry_01"),
	TEXT("sfx_ultimate_01"),
	TEXT("sfx_boss_hit_01"),
	TEXT("sfx_hero_hurt_01"),
	TEXT("sfx_steal_card_01"),
	TEXT("sfx_steal_01"),
	TEXT("sfx_everything_back_01"),
	TEXT("sfx_final_blow_01"),
	TEXT("sfx_ultimate_ready_01"),
	TEXT("sfx_heal_cash_01"),
};
static_assert(UE_ARRAY_COUNT(CueNames) == JamSoundRules::CueCount, "one name per cue");

/** The cue of one event, false when it makes no sound. */
bool CueOf(const FJamEvent& Event, const FJamFight& Fight, EJamCue& OutCue)
{
	switch (Event.Type)
	{
		case EJamEvent::Swing:
			OutCue = EJamCue::KatanaSlash;
			return true;
		case EJamEvent::ProjectileSpawned:
		{
			// Only the shuriken is thrown aloud: a volley of memos would be a wall of noise, the ball is the fallback.
			const FJamProjectile* Spawned = Fight.Projectiles.FindByPredicate(
				[&Event](const FJamProjectile& Projectile) { return Projectile.Id == Event.ProjectileId; });
			OutCue = EJamCue::ShurikenThrow;
			return Spawned && Spawned->Kind == EJamProjectile::Shuriken;
		}
		case EJamEvent::Explosion:
			OutCue = EJamCue::Explosion;
			return true;
		case EJamEvent::Slam:
			OutCue = EJamCue::Slam;
			return true;
		case EJamEvent::Dash:
			OutCue = EJamCue::Dash;
			return true;
		case EJamEvent::Deflected:
			OutCue = EJamCue::Parry;
			return true;
		case EJamEvent::UltimateSlash:
			OutCue = Fight.Phase == EJamPhase::Won ? EJamCue::FinalBlow : EJamCue::Ultimate;
			return true;
		case EJamEvent::Hit:
			OutCue = Event.Side == EJamSide::Hero ? EJamCue::HeroHurt : EJamCue::BossHit;
			return true;
		case EJamEvent::StealOpened:
			OutCue = EJamCue::StealCard;
			return true;
		case EJamEvent::StealResolved:
			OutCue = EJamCue::Steal;
			return true;
		case EJamEvent::Stunned:
			OutCue = EJamCue::EverythingBack;
			return true;
		case EJamEvent::UltimateReady:
			// The Publisher's meter is his business: the player is only told about his own.
			OutCue = EJamCue::UltimateReady;
			return Event.Side == EJamSide::Hero;
		case EJamEvent::HealTaken:
			OutCue = EJamCue::HealCash;
			return true;
		default:
			return false;
	}
}
} // namespace

const TCHAR* JamSoundRules::NameOf(EJamCue Cue)
{
	return CueNames[static_cast<int32>(Cue)];
}

TArray<EJamCue> JamSoundRules::CuesOf(const TArray<FJamEvent>& Events, const FJamFight& Fight)
{
	TArray<EJamCue> Cues;
	for (const FJamEvent& Event : Events)
	{
		EJamCue Cue;
		if (CueOf(Event, Fight, Cue))
		{
			Cues.AddUnique(Cue);
		}
	}
	return Cues;
}

FJamMix JamSoundRules::MixOf(EJamCue Cue, bool bVoiceSpeaking)
{
	FJamMix Mix;
	if (Cue == EJamCue::Steal)
	{
		// The crack's tear: a short, quiet accent under the screen's crack, never over the Publisher's line.
		Mix.Volume = bVoiceSpeaking ? 0.12f : 0.3f;
		Mix.CutAfter = 0.35f;
	}
	return Mix;
}
