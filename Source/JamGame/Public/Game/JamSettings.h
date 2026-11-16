#pragma once

#include "CoreMinimal.h"
#include "UI/JamMenuRules.h"

/**
 * The menu's settings in the engine (T-092): graphics quality, window mode and resolution through UGameUserSettings,
 * the three volumes and the difficulty in GameUserSettings.ini under [/Script/JamGame.JamVolumes]. Glue only: the menu's decisions are in
 * JamMenuRules.
 */
namespace JamSettings
{
/** What is set now, and the resolutions the screen offers (the current one among them). */
JAMGAME_API FJamMenuSettings Load(TArray<FIntPoint>& OutResolutions);

/**
 * Turns NVIDIA DLSS Super Resolution on in the picked mode at its optimal screen percentage, or off, and saves the
 * mode. Does nothing where DLSS is not supported (non-RTX card, old driver, -NoDlss), so such a run renders as before.
 */
JAMGAME_API void ApplyDlss(const FJamMenuSettings& Settings);

/** Applies and saves the quality, the window mode, the resolution and the DLSS mode. */
JAMGAME_API void ApplyGraphics(const FJamMenuSettings& Settings, const TArray<FIntPoint>& Resolutions);

/**
 * T-102: the display a run starts with (JamDisplayRules::Settle): a first launch is borderless at the desktop's size, a
 * borderless window covers the desktop, a window fits it (the author's 5v run), and the scene renders at full size. A
 * run whose window the command line sets keeps its window. Logs the numbers before and after.
 */
JAMGAME_API void SettleDisplay();

/** T-101: saves the difficulty; the fight takes it at the next NEW GAME. */
JAMGAME_API void SaveDifficulty(const FJamMenuSettings& Settings);

/** Saves the three volumes; the master one is applied here, the music and the voice by their actors. */
JAMGAME_API void ApplyVolumes(const FJamMenuSettings& Settings, const UObject* WorldContext);
} // namespace JamSettings
