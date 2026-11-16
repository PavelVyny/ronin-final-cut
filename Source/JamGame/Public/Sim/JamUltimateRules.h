#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The Ultimate: a meter filled by the damage the fighter deals; when it is full, a press winds up and then slashes along
 * a long line in front of him. The slash itself does not fill the meter.
 * Tests: Jam.Ultimate.*
 */
namespace JamUltimateRules
{
/** The projectile id the slash's hit is reported with: the final blow is told apart from every other hit by it. */
inline constexpr int32 SlashId = -1;

/** True when a round target is touched by the line from Origin along Dir, Range long and HalfWidth to each side. */
JAMGAME_API bool OnLine(
	const FVector2D& Origin, const FVector2D& Dir, const FVector2D& TargetPos, float TargetRadius, float Range, float HalfWidth);

/** Fills the meters: the hero's by the damage he dealt this step, the Publisher's stolen one by the clock (T-038). */
JAMGAME_API void Charge(FJamFight& Fight, const FJamBalanceData& Balance);

/** The fighter presses the Ultimate: starts the wind-up when he owns it and the meter is full. */
JAMGAME_API void Press(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);

/** From turned toward To by MaxDegrees at most, the short way round; a unit vector. */
JAMGAME_API FVector2D TurnToward(const FVector2D& From, const FVector2D& To, double MaxDegrees);

/** Lands the slash whose wind-up ends on this step; before that the hero's line turns toward his aim (T-058). */
JAMGAME_API void Land(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);
} // namespace JamUltimateRules
