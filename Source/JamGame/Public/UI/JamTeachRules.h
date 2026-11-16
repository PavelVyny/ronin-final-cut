#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

#include "JamTeachRules.generated.h"

/**
 * Numbers of the in-fight hints. The values live in the [JamTeach] section of Config/DefaultGame.ini, one line per
 * field.
 */
USTRUCT()
struct JAMGAME_API FJamTeachSettings
{
	GENERATED_BODY()

	/** Seconds before an enemy shot lands that "LMB PARRY" shows: about the time a person needs to press. */
	UPROPERTY()
	float ParryPromptLead = 0.f;

	/** The hero's deflects after which the game stops teaching Parry. */
	UPROPERTY()
	int32 ParryLessons = 0;
};

/** What the player has shown he knows, over the whole session: retries and new fights keep it. */
struct FJamLessons
{
	int32 Deflects = 0;
	int32 Ultimates = 0;
};

/** How loudly the HUD calls out a full Ultimate. */
enum class EJamUltimateCall : uint8
{
	None,
	/** The pulsing banner at the bottom of the screen. */
	Banner,
	/** The banner and the call over the hero's head: he has never used it. */
	BannerAndPrompt,
};

/**
 * The game teaches its mechanics before the Publisher takes them (the playtest found Parry and the Ultimate only when
 * they were gone). Presentation only: the simulation never asks.
 * Tests: Jam.Teach.*
 */
namespace JamTeachRules
{
inline const TCHAR* const Section = TEXT("JamTeach");

/** Counts what the hero did in these events: his deflects and his Ultimate slashes. */
JAMGAME_API void Learn(FJamLessons& Lessons, TConstArrayView<FJamEvent> Events);

/**
 * The call of a full Ultimate: while the hero owns it, the meter is full, it is not winding up already and the fight
 * runs or the Publisher is stunned (the moment of the final blow).
 */
JAMGAME_API EJamUltimateCall UltimateCallFor(const FJamFight& Fight, const FJamLessons& Lessons);

/**
 * The enemy projectile that "LMB PARRY" points at: the first one to land on the hero within the lead, while the fight
 * runs, the hero owns a Parry that would start now and still has lessons to take. INDEX_NONE when there is none.
 */
JAMGAME_API int32 ParryPromptFor(const FJamFight& Fight, const FJamLessons& Lessons, const FJamTeachSettings& Settings);
} // namespace JamTeachRules
