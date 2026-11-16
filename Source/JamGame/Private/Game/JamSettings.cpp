#include "Game/JamSettings.h"

#include "AudioDevice.h"
#include "DLSSLibrary.h"
#include "HAL/IConsoleManager.h"
#include "JamGame.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Game/JamDisplayRules.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
const TCHAR* const VolumeSection = TEXT("/Script/JamGame.JamVolumes");

/** The menu's window modes in the order of JamTexts::WindowNames. */
const EWindowMode::Type WindowModes[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };

/** The menu's DLSS modes in the order of JamTexts::DlssNames. */
const UDLSSMode DlssModes[] = { UDLSSMode::Off, UDLSSMode::Quality, UDLSSMode::Balanced, UDLSSMode::Performance };

/** The screen percentage before DLSS first changed it, put back when DLSS is turned off; negative while DLSS is off. */
float PercentageBeforeDlss = -1.f;

/** Set once the display was settled: from then on the saved window is the player's own choice. */
const TCHAR* const DisplayKey = TEXT("DisplaySettled");

/**
 * Sets r.ScreenPercentage at the priority it was last set by (the render scale of the user settings sets it above the
 * scalability priority), so the value always takes.
 */
void SetPercentage(IConsoleVariable& Percentage, float Value)
{
	Percentage.Set(Value, static_cast<EConsoleVariableFlags>(Percentage.GetFlags() & ECVF_SetByMask));
}

EJamWindowMode ModeOf(EWindowMode::Type Mode)
{
	const int32 Index = static_cast<int32>(Mode == EWindowMode::Fullscreen ? 0 : (Mode == EWindowMode::WindowedFullscreen ? 1 : 2));
	return static_cast<EJamWindowMode>(Index);
}

FJamDisplay DisplayOf(UGameUserSettings& User)
{
	float Normalized = 0.f;
	float Scale = 0.f;
	float Min = 0.f;
	float Max = 0.f;
	User.GetResolutionScaleInformationEx(Normalized, Scale, Min, Max);
	FJamDisplay Display;
	Display.Mode = ModeOf(User.GetFullscreenMode());
	Display.Resolution = User.GetScreenResolution();
	Display.RenderScale = Scale;
	return Display;
}

/** Puts a settled display on the user settings; ApplySettings shows it. */
void SetDisplay(UGameUserSettings& User, const FJamDisplay& Display)
{
	User.SetFullscreenMode(WindowModes[static_cast<int32>(Display.Mode)]);
	User.SetScreenResolution(Display.Resolution);
	User.SetResolutionScaleValueEx(Display.RenderScale);
}

/** DLSS Super Resolution runs here: an RTX card and driver the plugin accepts, and no -NoDlss on the command line. */
bool IsDlssSupported()
{
	return !FParse::Param(FCommandLine::Get(), TEXT("NoDlss")) && UDLSSLibrary::IsDLSSSupported();
}

int32 LoadVolume(const TCHAR* Key)
{
	int32 Steps = JamMenuRules::VolumeSteps;
	GConfig->GetInt(VolumeSection, Key, Steps, GGameUserSettingsIni);
	return FMath::Clamp(Steps, 0, JamMenuRules::VolumeSteps);
}
} // namespace

FJamMenuSettings JamSettings::Load(TArray<FIntPoint>& OutResolutions)
{
	FJamMenuSettings Settings;
	OutResolutions.Reset();
	TArray<FIntPoint> Offered;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Offered);
	for (const FIntPoint& Size : Offered)
	{
		OutResolutions.AddUnique(Size);
	}
	if (UGameUserSettings* User = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		const int32 Level = User->GetOverallScalabilityLevel();
		// A custom mix of levels reads as -1: the menu shows it as EPIC until the player picks one.
		Settings.Quality = Level < 0 ? JamMenuRules::QualityCount - 1 : FMath::Min(Level, JamMenuRules::QualityCount - 1);
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(WindowModes); ++Index)
		{
			if (WindowModes[Index] == User->GetFullscreenMode())
			{
				Settings.Window = Index;
			}
		}
		const FIntPoint Now = User->GetScreenResolution();
		Settings.Resolution = OutResolutions.AddUnique(Now);
	}
	Settings.ResolutionCount = OutResolutions.Num();
	Settings.Master = LoadVolume(TEXT("Master"));
	Settings.Music = LoadVolume(TEXT("Music"));
	Settings.Voice = LoadVolume(TEXT("Voice"));
	GConfig->GetInt(VolumeSection, TEXT("Difficulty"), Settings.Difficulty, GGameUserSettingsIni);
	Settings.Difficulty = FMath::Clamp(Settings.Difficulty, 0, JamMenuRules::DifficultyCount - 1);
	Settings.bDlssSupported = IsDlssSupported();
	int32 SavedDlss = INDEX_NONE;
	GConfig->GetInt(VolumeSection, TEXT("Dlss"), SavedDlss, GGameUserSettingsIni);
	Settings.Dlss = JamMenuRules::DlssFor(SavedDlss, Settings.bDlssSupported);
	UE_LOG(LogJamGame, Log, TEXT("DLSS Super Resolution %s, mode %d"),
		Settings.bDlssSupported ? TEXT("supported") : TEXT("not supported, off"), Settings.Dlss);
	return Settings;
}

void JamSettings::ApplyDlss(const FJamMenuSettings& Settings)
{
	// Without DLSS nothing is touched: the game renders as it did before the plugin.
	if (!Settings.bDlssSupported)
	{
		return;
	}
	GConfig->SetInt(VolumeSection, TEXT("Dlss"), Settings.Dlss, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	IConsoleVariable* Percentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	const UDLSSMode Mode = DlssModes[FMath::Clamp(Settings.Dlss, 0, JamMenuRules::DlssCount - 1)];
	bool bModeSupported = false;
	float Optimal = 100.f;
	bool bFixed = false;
	float Min = 0.f;
	float Max = 0.f;
	float Sharpness = 0.f;
	if (Mode != UDLSSMode::Off)
	{
		const FIntPoint Screen =
			GEngine && GEngine->GetGameUserSettings() ? GEngine->GetGameUserSettings()->GetScreenResolution() : FIntPoint(1920, 1080);
		UDLSSLibrary::GetDLSSModeInformation(Mode, FVector2D(Screen), bModeSupported, Optimal, bFixed, Min, Max, Sharpness);
	}
	if (!bModeSupported || Optimal <= 0.f)
	{
		UDLSSLibrary::EnableDLSS(false);
		if (Percentage && PercentageBeforeDlss > 0.f)
		{
			SetPercentage(*Percentage, PercentageBeforeDlss);
		}
		PercentageBeforeDlss = -1.f;
		UE_LOG(LogJamGame, Log, TEXT("DLSS Super Resolution off"));
		return;
	}
	if (Percentage && PercentageBeforeDlss < 0.f)
	{
		PercentageBeforeDlss = Percentage->GetFloat();
	}
	UDLSSLibrary::EnableDLSS(true);
	if (Percentage)
	{
		SetPercentage(*Percentage, Optimal);
	}
	UE_LOG(LogJamGame, Log,
		TEXT("DLSS Super Resolution on: mode %d, screen percentage %.1f (r.ScreenPercentage %.1f), r.NGX.DLSS.Enable %d"), Settings.Dlss,
		Optimal, Percentage ? Percentage->GetFloat() : -1.f, UDLSSLibrary::IsDLSSEnabled() ? 1 : 0);
}

void JamSettings::ApplyGraphics(const FJamMenuSettings& Settings, const TArray<FIntPoint>& Resolutions)
{
	UGameUserSettings* User = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!User)
	{
		return;
	}
	// A quality also sets the scene's share of the window (LOW renders half of it); the display rule puts it back to full.
	User->SetOverallScalabilityLevel(FMath::Clamp(Settings.Quality, 0, JamMenuRules::QualityCount - 1));
	FJamDisplay Picked = DisplayOf(*User);
	Picked.Mode = static_cast<EJamWindowMode>(FMath::Clamp(Settings.Window, 0, JamMenuRules::WindowCount - 1));
	Picked.Resolution = Resolutions.IsValidIndex(Settings.Resolution) ? Resolutions[Settings.Resolution] : Picked.Resolution;
	// Borderless always covers the desktop; the picked resolution is for fullscreen and the window.
	SetDisplay(*User, JamDisplayRules::Settle(Picked, true, User->GetDesktopResolution()));
	User->ApplySettings(false);
	ApplyDlss(Settings);
}

void JamSettings::SettleDisplay()
{
	UGameUserSettings* User = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!User)
	{
		return;
	}
	int32 Settled = 0;
	const bool bSaved = GConfig->GetInt(VolumeSection, DisplayKey, Settled, GGameUserSettingsIni) && Settled != 0;
	const FJamDisplay Before = DisplayOf(*User);
	const FIntPoint Desktop = User->GetDesktopResolution();
	FJamDisplay After = JamDisplayRules::Settle(Before, bSaved, Desktop);
	const TCHAR* const Line = FCommandLine::Get();
	if (FParse::Param(Line, TEXT("windowed")) || FParse::Param(Line, TEXT("fullscreen")) || FParse::Value(Line, TEXT("ResX="), Settled))
	{
		After.Mode = Before.Mode;
		After.Resolution = Before.Resolution;
	}
	SetDisplay(*User, After);
	User->ApplySettings(false);
	GConfig->SetInt(VolumeSection, DisplayKey, 1, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	// The numbers of the blur report (T-102): Development builds write them to the log.
	UE_LOG(LogJamGame, Log, TEXT("Display: %s, mode %d %dx%d scale %.0f -> mode %d %dx%d scale %.0f on a %dx%d desktop"),
		bSaved ? TEXT("saved") : TEXT("first launch"), static_cast<int32>(Before.Mode), Before.Resolution.X, Before.Resolution.Y,
		Before.RenderScale, static_cast<int32>(After.Mode), After.Resolution.X, After.Resolution.Y, After.RenderScale, Desktop.X,
		Desktop.Y);
}

void JamSettings::SaveDifficulty(const FJamMenuSettings& Settings)
{
	GConfig->SetInt(VolumeSection, TEXT("Difficulty"), Settings.Difficulty, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void JamSettings::ApplyVolumes(const FJamMenuSettings& Settings, const UObject* WorldContext)
{
	GConfig->SetInt(VolumeSection, TEXT("Master"), Settings.Master, GGameUserSettingsIni);
	GConfig->SetInt(VolumeSection, TEXT("Music"), Settings.Music, GGameUserSettingsIni);
	GConfig->SetInt(VolumeSection, TEXT("Voice"), Settings.Voice, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (FAudioDevice* Device = World ? World->GetAudioDeviceRaw() : nullptr)
	{
		Device->SetTransientPrimaryVolume(JamMenuRules::Volume(Settings.Master));
	}
}
