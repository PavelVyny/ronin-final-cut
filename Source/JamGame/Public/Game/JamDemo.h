#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

class UJamFightSubsystem;

/**
 * The demo run: the bot plays the hero on the top-down camera. It exists so that a clip can be recorded and the game
 * looked at with nobody at the PC. Switched on from the command line:
 *
 *   -JamDemo               the bot plays
 *   -JamCapture=<folder>   every frame is saved there as a PNG, at a fixed 30 frames per second
 *   -JamDemoSeconds=<n>    the game quits after n seconds
 *   -JamDemoHowTo          the HOW TO PLAY screen stays up for the whole run (its picture for the itch page, T-096)
 *   -JamDemoSteal=<names>  the bot gives these away on the cards, the first one on offer (PARRY,ULTIMATE)
 */
struct FJamDemo
{
	/** Reads the command line. */
	void Init();

	bool IsOn() const { return bOn; }

	/** The run starts in the fight, without the intro (-JamDemoNoIntro). */
	bool SkipsIntro() const { return bOn && (bNoIntro || bGallery || bStrides); }

	/** The pose gallery (-JamPoseGallery): the fight stands, the hero holds one pose after another for a close camera. */
	bool IsGallery() const { return bOn && bGallery; }

	/** The stride gallery (-JamStrideGallery): the hero faces east and runs each way in turn, the fight stands (T-078). */
	bool IsStrideGallery() const { return bOn && bStrides; }

	/** -JamDemoHowTo: the HOW TO PLAY screen stays up for the whole run (T-096). */
	bool ShowsHowTo() const { return bOn && bHowTo; }

	/** -JamDemoClose: the camera follows the hero close and low, so his feet show (T-078). */
	bool IsClose() const { return bOn && bClose; }

	/**
	 * The hero's intent of the frame. Also answers an ending the way a player would. The bot answers the steal card only
	 * with -JamDemoSteal; otherwise the Publisher takes the left option when its time runs out.
	 */
	FJamIntent Drive(UJamFightSubsystem& Host, float Time);

	/** Saves the frame if asked to and quits when the time is up. */
	void EndFrame(float Time);

	/**
	 * T-102: with -JamFrameLog=<file>, counts the frame and every 5 s writes the average frame rate since the first 5 s
	 * to the file, so a Development and a Shipping build can be compared. Works without -JamDemo.
	 */
	void CountFrame(double FrameSeconds, double RealTime);

private:
	void Direct(UJamFightSubsystem& Host, float Time);

	bool bOn = false;
	/** -JamDemoHoldUltimate: the bot never presses E, so a full meter and its banner stay on screen for a clip. */
	bool bHoldUltimate = false;
	/** -JamDemoSpamStrike: the bot holds LMB every frame, as a player spamming the strike-parry into a burst (T-059). */
	bool bSpamStrike = false;
	/** -JamDemoNoIntro: the run starts in the fight, without the Publisher's show-off (T-030). */
	bool bNoIntro = false;
	bool bGallery = false;
	bool bClose = false;
	bool bHowTo = false;
	bool bStrides = false;
	/** -JamDemoSteal=PARRY,ULTIMATE: the card option the bot gives away, the first of these that is on the card, so a
	 * clip shows what the Publisher does with them. Names as on the card. */
	TArray<FString> GiveAway;
	/** -JamDemoGive=SHURIKEN: the Publisher owns these from the start, without a cut (the look stays whole), so a clip
	 * shows him using them next to the full art. Names as on the card. */
	TArray<FString> GiveAtStart;
	bool bGiven = false;
	FString CaptureDir;
	FString FrameLog;
	int32 CountedFrames = 0;
	double CountedSeconds = 0.0;
	double NextFrameWrite = 10.0;
	float QuitAfter = 0.f;
	/** -JamDemoSkipEvery=12: every this many seconds the Publisher drops to his next cut (the F5 skip), so a short clip shows
	 * every cut and every layer the hero loses. 0: off. */
	float SkipEvery = 0.f;
	float LastSkip = 0.f;
	int32 Frame = 0;
	/** When the fight on screen ended; negative while it goes on. */
	float EndedAt = -1.f;
};
