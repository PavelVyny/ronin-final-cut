#include "Sim/JamLayerRules.h"

FJamLayers JamLayerRules::For(int32 CutsDone, EJamPhase Phase, const FJamLayerSettings& Settings)
{
	FJamLayers Layers;
	if (Phase == EJamPhase::Stunned || Phase == EJamPhase::Won)
	{
		return Layers;
	}
	Layers.bArt = CutsDone < Settings.ArtLostAt;
	Layers.bHeroArt = CutsDone < Settings.HeroArtLostAt;
	Layers.bEffects = CutsDone < Settings.EffectsLostAt;
	Layers.bMaterials = CutsDone < Settings.MaterialsLostAt;
	Layers.bAnimation = CutsDone < Settings.AnimationLostAt;
	Layers.bModel = CutsDone < Settings.ModelLostAt;
	Layers.bHud = CutsDone < Settings.HudLostAt;
	return Layers;
}

int32 JamLayerRules::StemsFor(int32 CutsDone, EJamPhase Phase, const FJamLayerSettings& Settings)
{
	const TArray<int32>& Left = Settings.StemsLeft;
	if (Phase == EJamPhase::Stunned || Phase == EJamPhase::Won || Left.Num() == 0)
	{
		return StemCount;
	}
	return FMath::Clamp(Left[FMath::Clamp(CutsDone, 0, Left.Num() - 1)], 0, StemCount);
}

FJamLayers JamLayerRules::HeroLayers(const FJamLayers& Shown)
{
	FJamLayers Hero = Shown;
	Hero.bArt = Shown.bHeroArt;
	return Hero;
}
