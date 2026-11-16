#pragma once

#include "CoreMinimal.h"

/**
 * How many fixed simulation steps a rendered frame runs.
 * Tests: Jam.Sim.FixedStep.*
 */
namespace JamStepRules
{
struct FFrameSteps
{
	int32 Steps = 0;
	/** Frame time not yet simulated, carried to the next frame. Always less than a step. */
	float Remainder = 0.f;
};

/**
 * Accumulated is the remainder of the previous frame. A frame that would need more than MaxSteps (a hitch, a breakpoint)
 * runs MaxSteps and drops the rest: the fight slows down for a moment instead of replaying the backlog.
 */
JAMGAME_API FFrameSteps StepsFor(float Accumulated, float FrameDt, float StepDt, int32 MaxSteps);
} // namespace JamStepRules
