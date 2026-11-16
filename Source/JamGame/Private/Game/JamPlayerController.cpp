#include "Game/JamPlayerController.h"

#include "Core/JamKeys.h"
#include "Sim/JamAimRules.h"
#include "Sim/JamMoveRules.h"
#include "UI/JamCardRules.h"
#include "View/JamCameraRig.h"

AJamPlayerController::AJamPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AJamPlayerController::UseCursor()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

FVector2D AJamPlayerController::LiveAimPoint(const FVector& CameraNow) const
{
	return JamAimRules::FollowCamera(Aim.Point, AimCamera, CameraNow);
}

FJamIntent AJamPlayerController::ReadIntent(const AJamCameraRig& Rig, const FJamFighter& Hero, int32 CardOptions)
{
	FVector Origin;
	FVector Direction;
	if (DeprojectMousePositionToWorld(Origin, Direction) && JamAimRules::FromRay(Origin, Direction, 0.f, Hero.Pos, Aim))
	{
		AimCamera = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : Rig.GetActorLocation();
	}

	FJamIntent Intent;
	const FVector2D Keys((IsInputKeyDown(JamKeys::Right()) ? 1.0 : 0.0) - (IsInputKeyDown(JamKeys::Left()) ? 1.0 : 0.0),
		(IsInputKeyDown(JamKeys::Up()) ? 1.0 : 0.0) - (IsInputKeyDown(JamKeys::Down()) ? 1.0 : 0.0));
	Intent.Move = JamMoveRules::WorldMove(Keys, Rig.GetMoveBasisYaw());
	Intent.Aim = Aim;
	Intent.bPrimary = IsInputKeyDown(JamKeys::Katana());
	Intent.bSecondary = IsInputKeyDown(JamKeys::Shurikens());
	Intent.bDash = IsInputKeyDown(JamKeys::Dash());
	// No parry key (T-044): the strike parries a shot about to land (JamParryRules::ShotInReach).
	Intent.bUltimate = IsInputKeyDown(JamKeys::Ultimate());
	// The steal card is answered with the keys under the left hand: a press, not a hold, so walking left is no answer.
	if (WasInputKeyJustPressed(JamKeys::CardLeft()) || WasInputKeyJustPressed(EKeys::Left))
	{
		Intent.Choice = 0;
	}
	else if (WasInputKeyJustPressed(JamKeys::CardRight()) || WasInputKeyJustPressed(EKeys::Right))
	{
		Intent.Choice = 1;
	}
	float X = 0.f;
	float Y = 0.f;
	int32 Width = 0;
	int32 Height = 0;
	GetViewportSize(Width, Height);
	if (CardOptions > 0 && WasInputKeyJustPressed(JamKeys::Katana()) && GetMousePosition(X, Y))
	{
		const int32 Clicked = JamCardRules::OptionAt(FVector2D(X, Y), CardOptions, FVector2D(Width, Height));
		if (Clicked != INDEX_NONE)
		{
			Intent.Choice = Clicked;
		}
	}
	return Intent;
}

bool AJamPlayerController::AnyKeyJustPressed() const
{
	TArray<FKey> Keys;
	EKeys::GetAllKeys(Keys);
	return Keys.ContainsByPredicate(
		[this](const FKey& Key) { return !Key.IsAxis1D() && !Key.IsAxis2D() && !Key.IsAxis3D() && WasInputKeyJustPressed(Key); });
}
