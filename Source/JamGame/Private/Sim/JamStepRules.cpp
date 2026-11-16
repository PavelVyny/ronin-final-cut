#include "Sim/JamStepRules.h"

JamStepRules::FFrameSteps JamStepRules::StepsFor(float Accumulated, float FrameDt, float StepDt, int32 MaxSteps)
{
	FFrameSteps Result;
	const float Total = Accumulated + FMath::Max(0.f, FrameDt);
	const int32 Whole = FMath::FloorToInt32(Total / StepDt);
	if (Whole > MaxSteps)
	{
		Result.Steps = MaxSteps;
		return Result;
	}
	Result.Steps = Whole;
	Result.Remainder = Total - Whole * StepDt;
	return Result;
}
