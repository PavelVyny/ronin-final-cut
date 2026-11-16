#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The katana: a series of three strikes in an arc in front of the fighter, the third one strong. A strike lands once,
 * a wind-up after the press; a press that comes too late starts the series over.
 * Tests: Jam.Katana.*, Jam.Fight.KatanaDamagesOncePerStrike
 */
namespace JamKatanaRules
{
/**
 * True when a round target is touched by the arc: within Range of the attacker's centre (the target's radius counts)
 * and within HalfArcDeg of where he faces (a wide target sticks into the arc with its edge).
 */
JAMGAME_API bool InArc(
	const FVector2D& From, const FVector2D& Facing, const FVector2D& TargetPos, float TargetRadius, float Range, float HalfArcDeg);

/** Which strike a press starts on this step: 1 to 3, or 0 while the previous one is still in progress. */
JAMGAME_API int32 NextStrike(const FJamKatanaState& State, int32 Tick);

/** The state of a strike that starts on this step. */
JAMGAME_API FJamKatanaState Begin(int32 Strike, int32 Tick, const FJamBalanceData& Balance);

/** True for the last, strong strike of the series. */
JAMGAME_API bool IsFinisher(int32 Strike);

JAMGAME_API float Damage(int32 Strike, const FJamBalanceData& Balance);

/** The fighter holds the katana button: starts a strike when one can start. */
JAMGAME_API void Press(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);

/** Lands the strike whose wind-up ends on this step. */
JAMGAME_API void Land(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);
} // namespace JamKatanaRules
