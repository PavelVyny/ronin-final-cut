#include "View/JamNeonRules.h"

float JamNeonRules::LineGlow(float MusicTime, const FJamNeonSettings& Settings)
{
	if (Settings.Bpm <= 0.f || Settings.BeatDecay <= 0.f)
	{
		return Settings.LineGlow;
	}
	const float Beat = 60.f / Settings.Bpm;
	float SinceBeat = FMath::Fmod(FMath::Max(0.f, MusicTime), Beat);
	// A hair of float error short of a beat is the beat, not the tail of the one before.
	if (Beat - SinceBeat < KINDA_SMALL_NUMBER)
	{
		SinceBeat = 0.f;
	}
	return Settings.LineGlow + Settings.BeatPulse * FMath::Exp(-SinceBeat / Settings.BeatDecay);
}

float JamNeonRules::Bloom(const FJamLayers& Layers, const FJamNeonSettings& Settings)
{
	if (!Layers.bEffects)
	{
		return Settings.BloomCut;
	}
	return Layers.bArt ? Settings.Bloom : Settings.BloomFlat;
}

EJamLight JamNeonRules::LightFor(const FJamLayers& Shown)
{
	if (!Shown.bEffects)
	{
		return EJamLight::Bare;
	}
	return Shown.bArt ? EJamLight::Neon : EJamLight::Flat;
}

FJamFilmLook JamNeonRules::FilmLookFor(EJamLight Light, const FJamNeonSettings& Settings)
{
	FJamFilmLook Look;
	if (Light != EJamLight::Neon)
	{
		return Look;
	}
	Look.Grain = Settings.FilmGrain;
	Look.Fringe = Settings.Fringe;
	Look.FringeStart = Settings.FringeStart;
	Look.FogDensity = Settings.FogDensity;
	Look.FogFalloff = Settings.FogFalloff;
	Look.FogStart = Settings.FogStart;
	Look.FogOpacity = Settings.FogOpacity;
	Look.FarBlurKm = Settings.FarBlurKm;
	Look.FarBlurRadius = Settings.FarBlurRadius;
	return Look;
}
