#include "View/JamCameraRules.h"

FJamView JamCameraRules::ViewFor(
	EJamCameraMode Mode, const FVector& HeroPos, float LookYaw, float LookPitch, const FJamCameraSettings& Settings)
{
	FJamView View;
	switch (Mode)
	{
		case EJamCameraMode::TopDown:
			View.Rotation = FRotator(Settings.TopDownPitch, Settings.TopDownYaw, 0.0);
			View.Location = HeroPos - View.Rotation.Vector() * Settings.TopDownDistance;
			View.Fov = Settings.TopDownFov;
			break;
		case EJamCameraMode::Shoulder:
		{
			View.Rotation = FRotator(LookPitch, LookYaw, 0.0);
			const FVector Pivot = HeroPos + FVector(0.0, 0.0, Settings.ShoulderHeight);
			const FVector Right = FRotationMatrix(FRotator(0.0, LookYaw, 0.0)).GetUnitAxis(EAxis::Y);
			View.Location = Pivot - View.Rotation.Vector() * Settings.ShoulderDistance + Right * Settings.ShoulderSide;
			View.Fov = Settings.ShoulderFov;
			break;
		}
		case EJamCameraMode::FirstPerson:
			View.Rotation = FRotator(LookPitch, LookYaw, 0.0);
			View.Location = HeroPos + FVector(0.0, 0.0, Settings.EyeHeight);
			View.Fov = Settings.FirstPersonFov;
			break;
	}
	return View;
}

float JamCameraRules::MoveBasisYaw(EJamCameraMode Mode, float LookYaw, const FJamCameraSettings& Settings)
{
	return Mode == EJamCameraMode::TopDown ? Settings.TopDownYaw : LookYaw;
}

float JamCameraRules::ClampPitch(float Pitch, const FJamCameraSettings& Settings)
{
	return FMath::Clamp(Pitch, Settings.MinPitch, Settings.MaxPitch);
}
