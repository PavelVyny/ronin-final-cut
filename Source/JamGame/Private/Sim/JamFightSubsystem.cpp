#include "Sim/JamFightSubsystem.h"

#include "Core/JamBalanceRules.h"
#include "Core/JamConfig.h"
#include "JamGame.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sim/JamCheckpointRules.h"
#include "Sim/JamFightRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamStepRules.h"

namespace
{
// A frame longer than this many steps is a hitch: the fight slows down instead of catching up.
constexpr int32 MaxStepsPerFrame = 8;

int32 NewSeed()
{
#if !UE_BUILD_SHIPPING
	// -JamSeed=7 replays the same fight: with the demo bot and the capture's fixed step, a clip can show the same
	// moment in every tier.
	int32 Fixed = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("JamSeed="), Fixed))
	{
		return Fixed;
	}
#endif
	return static_cast<int32>(FPlatformTime::Cycles());
}

bool HasStealResolved(const TArray<FJamEvent>& Events)
{
	return Events.ContainsByPredicate([](const FJamEvent& Event) { return Event.Type == EJamEvent::StealResolved; });
}
} // namespace

void UJamFightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Restart();
}

bool UJamFightSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UJamFightSubsystem::Begin(const FJamFight& NewFight)
{
	Fight = NewFight;
	HeroIntent = FJamIntent();
	PendingChoice = -1;
	Accumulated = 0.f;
	Alpha = 1.f;
	FrameEvents.Reset();
}

void UJamFightSubsystem::Restart()
{
	const FJamBalanceData Loaded = JamConfig::Load<FJamBalanceData>(JamConfig::BalanceSection);
	for (const FString& Problem : JamBalanceRules::Problems(Loaded))
	{
		UE_LOG(LogJamGame, Error, TEXT("Balance: %s"), *Problem);
	}
	// T-101: the checkpoint is taken from this balance too, so a retry plays on the same level.
	Balance = JamDifficultyRules::Apply(Loaded, Difficulty);
	Begin(JamFightRules::Start(Balance, NewSeed()));
	Checkpoint = JamCheckpointRules::Capture(Fight, Balance);
}

void UJamFightSubsystem::SetDifficulty(EJamDifficulty Level)
{
	Difficulty = Level;
}

void UJamFightSubsystem::Retry()
{
	Begin(JamCheckpointRules::Restore(Checkpoint, NewSeed()));
}

void UJamFightSubsystem::Advance(float FrameDt)
{
	const JamStepRules::FFrameSteps Frame = JamStepRules::StepsFor(Accumulated, FrameDt, JamSim::StepDt, MaxStepsPerFrame);
	if (HeroIntent.Choice >= 0)
	{
		PendingChoice = HeroIntent.Choice;
	}
	FJamIntent Intent = HeroIntent;
	Intent.Choice = PendingChoice;
	FrameEvents.Reset();
	for (int32 Index = 0; Index < Frame.Steps; ++Index)
	{
		JamFightRules::Play(Fight, Intent, Balance);
		FrameEvents.Append(Fight.Events);
		if (HasStealResolved(Fight.Events))
		{
			// The cut is made: from here on a death comes back to this moment.
			Checkpoint = JamCheckpointRules::Capture(Fight, Balance);
		}
	}
	if (Frame.Steps > 0)
	{
		PendingChoice = -1;
	}
	Accumulated = Frame.Remainder;
	Alpha = Accumulated / JamSim::StepDt;
}

void UJamFightSubsystem::DebugGive(EJamMechanic Mechanic)
{
	JamLoadoutRules::Give(Fight.Ledger, Mechanic, EJamSide::Boss);
}

void UJamFightSubsystem::DebugSkip()
{
	if (Fight.Phase == EJamPhase::Fight)
	{
		FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
		Boss.Health = FMath::Min(Boss.Health, Fight.Steal.Floor);
	}
}
