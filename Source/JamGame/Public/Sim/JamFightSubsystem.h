#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "Sim/JamDifficultyRules.h"
#include "Subsystems/WorldSubsystem.h"

#include "JamFightSubsystem.generated.h"

/**
 * Host of the fight simulation in a game world: holds the state, the balance numbers and the retry point, and runs the
 * fixed steps.
 *
 * It does not tick by itself. The game mode calls Advance once per frame after it has written the intents, so the order
 * inside a frame is always input, simulation, view. Everything else only reads from here.
 */
UCLASS()
class JAMGAME_API UJamFightSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Starts the fight over with the numbers from the ini, scaled by the difficulty. */
	void Restart();

	/** T-101: the level every Restart from now on plays at; a retry keeps the fight's. NORMAL until set. */
	void SetDifficulty(EJamDifficulty Level);

	/** Starts again from the last steal: the answer to a death. */
	void Retry();

	/** Runs the fixed steps that fit into the frame. */
	void Advance(float FrameDt);

	/** A frame the fight holds (the crack, T-085): no step, so last frame's events are not shown again. */
	void Hold() { FrameEvents.Reset(); }

	/**
	 * Drops the boss to the threshold of his next steal, or to zero when no steal is left. A key for looking at the cards
	 * and the ending without fighting for them.
	 */
	void DebugSkip();

	/** Hands the Publisher one of the hero's mechanics without a cut, so a clip can show him using it from the start. */
	void DebugGive(EJamMechanic Mechanic);

	const FJamFight& GetFight() const { return Fight; }

	const FJamBalanceData& GetBalance() const { return Balance; }

	/** How far the frame is between the last two steps, 0 to 1: the views interpolate with it. */
	float GetAlpha() const { return Alpha; }

	/** Events of every step run by the last Advance. */
	const TArray<FJamEvent>& GetFrameEvents() const { return FrameEvents; }

	/** What the hero wants. Written by the game mode before Advance; the Publisher decides for himself in the step. */
	FJamIntent HeroIntent;

private:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Puts a fight on the stage and forgets everything of the one before. */
	void Begin(const FJamFight& NewFight);

	FJamBalanceData Balance;
	EJamDifficulty Difficulty = EJamDifficulty::Normal;
	FJamFight Fight;
	/** The fight as a retry starts it: taken at the start and after every steal. */
	FJamFight Checkpoint;
	/**
	 * The answer to the card pressed on this frame. A key press lives for one frame and a frame may run no step, so it
	 * is kept until a step has seen it; it never waits longer, or a key mashed before the card would answer it.
	 */
	int32 PendingChoice = -1;
	float Accumulated = 0.f;
	float Alpha = 1.f;
	TArray<FJamEvent> FrameEvents;
};
