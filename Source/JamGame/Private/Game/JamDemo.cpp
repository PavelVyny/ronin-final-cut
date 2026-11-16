#include "Game/JamDemo.h"

#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sim/JamBotRules.h"
#include "Sim/JamFightSubsystem.h"
#include "Sim/JamLoadoutRules.h"
#include "UnrealClient.h"

namespace
{
constexpr double CaptureFps = 30.0;
// How long the ending title stays before the fight starts again.
constexpr float DemoEndingSeconds = 2.5f;
} // namespace

void FJamDemo::Init()
{
	const TCHAR* CommandLine = FCommandLine::Get();
	bOn = FParse::Param(CommandLine, TEXT("JamDemo"));
	bHoldUltimate = FParse::Param(CommandLine, TEXT("JamDemoHoldUltimate"));
	bSpamStrike = FParse::Param(CommandLine, TEXT("JamDemoSpamStrike"));
	bNoIntro = FParse::Param(CommandLine, TEXT("JamDemoNoIntro"));
	bGallery = FParse::Param(CommandLine, TEXT("JamPoseGallery"));
	bClose = FParse::Param(CommandLine, TEXT("JamDemoClose"));
	bHowTo = FParse::Param(CommandLine, TEXT("JamDemoHowTo"));
	bStrides = FParse::Param(CommandLine, TEXT("JamStrideGallery"));
	FParse::Value(CommandLine, TEXT("JamDemoSeconds="), QuitAfter);
	FParse::Value(CommandLine, TEXT("JamDemoSkipEvery="), SkipEvery);
	FString Steal;
	// The list holds commas: read past them.
	if (FParse::Value(CommandLine, TEXT("JamDemoSteal="), Steal, false))
	{
		Steal.ToUpper().ParseIntoArray(GiveAway, TEXT(","));
	}
	FString Give;
	if (FParse::Value(CommandLine, TEXT("JamDemoGive="), Give, false))
	{
		Give.ToUpper().ParseIntoArray(GiveAtStart, TEXT(","));
	}
	FParse::Value(CommandLine, TEXT("JamFrameLog="), FrameLog);
	if (FParse::Value(CommandLine, TEXT("JamCapture="), CaptureDir))
	{
		// A fixed step makes the saved frames an even 30 per second however long a frame takes to save.
		FApp::SetFixedDeltaTime(1.0 / CaptureFps);
		FApp::SetUseFixedTimeStep(true);
	}
}

void FJamDemo::Direct(UJamFightSubsystem& Host, float Time)
{
	if (!bGiven)
	{
		bGiven = true;
		for (int32 Index = 0; Index < JamSim::MechanicCount; ++Index)
		{
			const EJamMechanic Mechanic = static_cast<EJamMechanic>(Index);
			if (GiveAtStart.Contains(JamLoadoutRules::Name(Mechanic)))
			{
				Host.DebugGive(Mechanic);
			}
		}
	}
	if (SkipEvery > 0.f && Time - LastSkip >= SkipEvery)
	{
		LastSkip = Time;
		Host.DebugSkip();
	}
	const EJamPhase Phase = Host.GetFight().Phase;
	if (Phase != EJamPhase::HeroDead && Phase != EJamPhase::Won)
	{
		EndedAt = -1.f;
	}
	else if (EndedAt < 0.f)
	{
		EndedAt = Time;
	}
	else if (Time - EndedAt >= DemoEndingSeconds)
	{
		// The same answers a player gives: a loss is retried from the last cut, a win starts a new fight.
		if (Phase == EJamPhase::Won)
		{
			Host.Restart();
		}
		else
		{
			Host.Retry();
		}
		EndedAt = -1.f;
	}
}

FJamIntent FJamDemo::Drive(UJamFightSubsystem& Host, float Time)
{
	Direct(Host, Time);
	FJamIntent Intent = JamBotRules::Play(Host.GetFight(), EJamSide::Hero, Host.GetBalance());
	Intent.bUltimate = Intent.bUltimate && !bHoldUltimate;
	Intent.bPrimary = Intent.bPrimary || bSpamStrike;
	const FJamStealState& Steal = Host.GetFight().Steal;
	for (const FString& Wanted : GiveAway)
	{
		const int32 Index = Steal.Offer.IndexOfByPredicate([&Wanted](EJamMechanic M) { return Wanted == JamLoadoutRules::Name(M); });
		if (Index != INDEX_NONE)
		{
			Intent.Choice = Index;
			break;
		}
	}
	return Intent;
}

void FJamDemo::CountFrame(double FrameSeconds, double RealTime)
{
	// The first seconds load and compile: they are not the game's frame rate.
	constexpr double WarmUpSeconds = 5.0;
	constexpr double WriteEvery = 5.0;
	if (FrameLog.IsEmpty() || RealTime < WarmUpSeconds)
	{
		return;
	}
	++CountedFrames;
	CountedSeconds += FrameSeconds;
	if (RealTime >= NextFrameWrite && CountedSeconds > 0.0)
	{
		NextFrameWrite = RealTime + WriteEvery;
		FFileHelper::SaveStringToFile(
			FString::Printf(TEXT("frames %d seconds %.1f fps %.1f\n"), CountedFrames, CountedSeconds, CountedFrames / CountedSeconds),
			*FrameLog);
	}
}

void FJamDemo::EndFrame(float Time)
{
	if (!CaptureDir.IsEmpty())
	{
		FScreenshotRequest::RequestScreenshot(CaptureDir / FString::Printf(TEXT("f_%05d.png"), Frame++), true, false);
	}
	// While frames are saved the clip's own clock counts: the world clock ran ahead of the saved frames and the clip came
	// out short.
	const float Elapsed = CaptureDir.IsEmpty() ? Time : static_cast<float>(Frame / CaptureFps);
	if (QuitAfter > 0.f && Elapsed >= QuitAfter)
	{
		RequestEngineExit(TEXT("The demo run is over"));
	}
}
