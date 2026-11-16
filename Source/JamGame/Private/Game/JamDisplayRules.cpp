#include "Game/JamDisplayRules.h"

#include "UI/JamMenuRules.h"

FJamDisplay JamDisplayRules::Settle(const FJamDisplay& Saved, bool bSaved, const FIntPoint& Desktop)
{
	const bool bDesktopKnown = Desktop.X > 0 && Desktop.Y > 0;
	FJamDisplay Out = Saved;
	if (!bSaved)
	{
		Out.Mode = EJamWindowMode::Borderless;
	}
	switch (Out.Mode)
	{
		case EJamWindowMode::Borderless:
			// Smaller than the desktop, Windows stretches it over the screen: the blur of the author's first launch.
			Out.Resolution = bDesktopKnown ? Desktop : Out.Resolution;
			break;
		case EJamWindowMode::Windowed:
			Out.Resolution = JamMenuRules::FitWindow(Out.Resolution, Desktop);
			break;
		default:
			Out.Resolution = Out.Resolution.X > 0 && Out.Resolution.Y > 0 ? Out.Resolution : Desktop;
			break;
	}
	Out.RenderScale = FullRenderScale;
	return Out;
}
