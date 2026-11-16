#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

#include "JamLayerRules.generated.h"

/**
 * After which of the Publisher's cuts each cosmetic layer of the hero and the world is gone: the number of the cut,
 * 1 for the first. The values live in the [JamLayers] section of Config/DefaultGame.ini.
 */
USTRUCT()
struct JAMGAME_API FJamLayerSettings
{
	GENERATED_BODY()

	/** The full art (T-067): the 3D hero and the rich effects; without it the hero is the shape ronin of before. */
	UPROPERTY()
	int32 ArtLostAt = 0;

	/**
	 * The hero's own art (the session-5s playtest): his 3D model and his dash's afterimages outlive the world's art by a
	 * cut; without it he is the shape ronin.
	 */
	UPROPERTY()
	int32 HeroArtLostAt = 0;

	/** The hero's glow, the sweep of his blade, the trail of his dash. */
	UPROPERTY()
	int32 EffectsLostAt = 0;

	/** Colours of the hero and the arena: the world turns into the engine's grey grid. */
	UPROPERTY()
	int32 MaterialsLostAt = 0;

	/** The hero stops moving his body and stands in a T-pose. */
	UPROPERTY()
	int32 AnimationLostAt = 0;

	/** The ronin turns into a bean. */
	UPROPERTY()
	int32 ModelLostAt = 0;

	/** The bars of the HUD go, plain text is left. */
	UPROPERTY()
	int32 HudLostAt = 0;

	/**
	 * How many music stems still play: before the first cut, then after each one. The stems go in the reverse order of
	 * their numbers (the lead first, the drums last); at 0 only the proof-of-concept metronome is left.
	 */
	UPROPERTY()
	TArray<int32> StemsLeft;
};

/** What the player still sees of the full game. The Publisher keeps all of it, always. */
struct FJamLayers
{
	static constexpr int32 Total = 7;

	/** The full art (T-067): the world's rich look and effects; the hero's is bHeroArt. */
	bool bArt = true;
	/** The hero's 3D model and his dash's afterimages (the session-5s playtest). */
	bool bHeroArt = true;
	bool bEffects = true;
	bool bMaterials = true;
	bool bAnimation = true;
	bool bModel = true;
	bool bHud = true;

	int32 Count() const
	{
		return (bArt ? 1 : 0) + (bHeroArt ? 1 : 0) + (bEffects ? 1 : 0) + (bMaterials ? 1 : 0) + (bAnimation ? 1 : 0) + (bModel ? 1 : 0) +
			(bHud ? 1 : 0);
	}

	bool operator==(const FJamLayers& Other) const
	{
		return bArt == Other.bArt && bHeroArt == Other.bHeroArt && bEffects == Other.bEffects && bMaterials == Other.bMaterials &&
			bAnimation == Other.bAnimation && bModel == Other.bModel && bHud == Other.bHud;
	}
};

/**
 * The degradation of the game: each cut takes layers away, the stun gives everything back.
 * Tests: Jam.Layers.*
 */
namespace JamLayerRules
{
inline const TCHAR* const Section = TEXT("JamLayers");

/** Stems of the full music: drums, bass, pad, lead. */
inline constexpr int32 StemCount = 4;

/** The layers left after CutsDone cuts. While the boss is stunned and after the win everything is back. */
JAMGAME_API FJamLayers For(int32 CutsDone, EJamPhase Phase, const FJamLayerSettings& Settings);

/** The layers the hero is drawn with: the world's, his art standing for the full art. */
JAMGAME_API FJamLayers HeroLayers(const FJamLayers& Shown);

/** Music stems that play after CutsDone cuts; the whole band while the boss is stunned and after the win. */
JAMGAME_API int32 StemsFor(int32 CutsDone, EJamPhase Phase, const FJamLayerSettings& Settings);
} // namespace JamLayerRules
