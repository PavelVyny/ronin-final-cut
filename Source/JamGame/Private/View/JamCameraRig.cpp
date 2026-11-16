#include "View/JamCameraRig.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Core/JamConfig.h"

namespace
{
// Scene brightness the exposure is locked to: presentation only, retuned with the art in block E.
constexpr float FixedExposureBrightness = 1.f;
// The grade of the full-art tier (T-077): a touch more colour and contrast, cool shadows, warm highlights, dark corners.
// T-094: the film look's far blur focuses past the arena (cm) with the iris closed, so only the depth blur shows.
constexpr float FilmFocalDistance = 2200.f;
constexpr float FilmFstop = 22.f;
constexpr float NeonVignette = 0.5f;
// T-082: the night read too dark: a little more exposure in the full art.
constexpr float NeonExposureBias = 0.5f;
constexpr float NeonIndirect = 0.4f;
const FVector4 NeonSaturation(1.15, 1.15, 1.15, 1.0);
const FVector4 NeonContrast(1.1, 1.1, 1.1, 1.0);
const FVector4 NeonShadowGain(0.92, 0.97, 1.1, 1.0);
const FVector4 NeonHighlightGain(1.06, 1.0, 0.97, 1.0);
const FVector4 Neutral(1.0, 1.0, 1.0, 1.0);
constexpr float PlainOcclusion = 0.5f;
} // namespace

AJamCameraRig::AJamCameraRig()
{
	PrimaryActorTick.bCanEverTick = false;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);
	// Fast sideways motion is the whole game: blur would smear the things the player has to read.
	Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
	Camera->PostProcessSettings.MotionBlurAmount = 0.f;
	// One exposure for the three cameras, or the same floor is grey from above and dark from the ground.
	Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	Camera->PostProcessSettings.AutoExposureMinBrightness = FixedExposureBrightness;
	Camera->PostProcessSettings.AutoExposureMaxBrightness = FixedExposureBrightness;

	Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
	Capture->SetupAttachment(Camera);
	Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;
	Capture->bAlwaysPersistRenderingState = true;
}

void AJamCameraRig::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Settings = JamConfig::Load<FJamCameraSettings>(JamCameraRules::Section);
	Feedback = JamConfig::Load<FJamFeedbackSettings>(JamFeedbackRules::Section);
}

void AJamCameraRig::SetBloom(float Intensity)
{
	Camera->PostProcessSettings.bOverride_BloomIntensity = true;
	Camera->PostProcessSettings.BloomIntensity = Intensity;
}

void AJamCameraRig::SetLight(EJamLight Light)
{
	FPostProcessSettings& Post = Camera->PostProcessSettings;
	const bool bNeon = Light == EJamLight::Neon;
	Post.bOverride_DynamicGlobalIlluminationMethod = true;
	Post.DynamicGlobalIlluminationMethod = bNeon ? EDynamicGlobalIlluminationMethod::Lumen : EDynamicGlobalIlluminationMethod::None;
	Post.bOverride_ReflectionMethod = true;
	if (bNeon)
	{
		Post.ReflectionMethod = EReflectionMethod::Lumen;
	}
	else
	{
		Post.ReflectionMethod = Light == EJamLight::Flat ? EReflectionMethod::ScreenSpace : EReflectionMethod::None;
	}
	// The emissive grid lights the whole floor through Lumen: kept to a glow, so the floor stays a night floor.
	Post.bOverride_IndirectLightingIntensity = true;
	Post.IndirectLightingIntensity = bNeon ? NeonIndirect : 1.f;
	Post.bOverride_VignetteIntensity = true;
	Post.VignetteIntensity = bNeon ? NeonVignette : 0.f;
	Post.bOverride_ColorSaturation = true;
	Post.ColorSaturation = bNeon ? NeonSaturation : Neutral;
	Post.bOverride_ColorContrast = true;
	Post.ColorContrast = bNeon ? NeonContrast : Neutral;
	Post.bOverride_ColorGainShadows = true;
	Post.ColorGainShadows = bNeon ? NeonShadowGain : Neutral;
	Post.bOverride_ColorGainHighlights = true;
	Post.ColorGainHighlights = bNeon ? NeonHighlightGain : Neutral;
	Post.bOverride_AutoExposureBias = true;
	Post.AutoExposureBias = bNeon ? NeonExposureBias : 0.f;
	// The greybox has no ambient occlusion either: the bare engine look.
	Post.bOverride_AmbientOcclusionIntensity = true;
	Post.AmbientOcclusionIntensity = Light == EJamLight::Bare ? 0.f : PlainOcclusion;
}

void AJamCameraRig::SetFilm(const FJamFilmLook& Look)
{
	FPostProcessSettings& Post = Camera->PostProcessSettings;
	Post.bOverride_FilmGrainIntensity = true;
	Post.FilmGrainIntensity = Look.Grain;
	Post.bOverride_SceneFringeIntensity = true;
	Post.SceneFringeIntensity = Look.Fringe;
	Post.bOverride_ChromaticAberrationStartOffset = true;
	Post.ChromaticAberrationStartOffset = Look.FringeStart;
	// The far blur only: focused past the whole arena with a closed iris, so the fighters stay sharp at any distance.
	const bool bFarBlur = Look.FarBlurKm > 0.f && Look.FarBlurRadius > 0.f;
	Post.bOverride_DepthOfFieldFocalDistance = true;
	Post.DepthOfFieldFocalDistance = bFarBlur ? FilmFocalDistance : 0.f;
	Post.bOverride_DepthOfFieldFstop = true;
	Post.DepthOfFieldFstop = FilmFstop;
	Post.bOverride_DepthOfFieldDepthBlurAmount = true;
	Post.DepthOfFieldDepthBlurAmount = bFarBlur ? Look.FarBlurKm : 1.f;
	Post.bOverride_DepthOfFieldDepthBlurRadius = true;
	Post.DepthOfFieldDepthBlurRadius = bFarBlur ? Look.FarBlurRadius : 0.f;
}

void AJamCameraRig::Kick(float Strength, float Time)
{
	const float Left = JamFeedbackRules::ShakeLeft(KickStrength, Time - KickTime, Feedback);
	if (Strength > Left)
	{
		KickStrength = Strength;
		KickTime = Time;
	}
}

void AJamCameraRig::AddLook(float MouseX, float MouseY)
{
	LookYaw = FRotator::NormalizeAxis(LookYaw + MouseX * Settings.MouseSensitivity);
	LookPitch = JamCameraRules::ClampPitch(LookPitch + MouseY * Settings.MouseSensitivity, Settings);
}

void AJamCameraRig::SetLook(float Yaw)
{
	LookYaw = FRotator::NormalizeAxis(Yaw);
	LookPitch = JamCameraRules::ClampPitch(0.f, Settings);
}

float AJamCameraRig::GetMoveBasisYaw() const
{
	return JamCameraRules::MoveBasisYaw(Mode, LookYaw, Settings);
}

void AJamCameraRig::Sync(const FVector& HeroPos, float Time)
{
	const FJamView View = JamCameraRules::ViewFor(Mode, HeroPos, LookYaw, LookPitch, Settings);
	const float Amplitude = JamFeedbackRules::ShakeLeft(KickStrength, Time - KickTime, Feedback);
	const FVector2D Shake = JamFeedbackRules::ShakeOffset(Amplitude, Time, Feedback);
	SetActorLocationAndRotation(View.Location + FVector(Shake.X, Shake.Y, 0.0), View.Rotation);
	Camera->SetFieldOfView(View.Fov);
}

void AJamCameraRig::Frame(const FVector& Location, const FVector& Target, float Fov)
{
	SetActorLocationAndRotation(Location, (Target - Location).Rotation());
	Camera->SetFieldOfView(Fov);
}

UTextureRenderTarget2D* AJamCameraRig::Freeze()
{
	FVector2D Size(1280.0, 720.0);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Size);
	}
	const int32 Width = FMath::Max(64, FMath::RoundToInt32(Size.X));
	const int32 Height = FMath::Max(64, FMath::RoundToInt32(Size.Y));
	if (!Frozen || Frozen->SizeX != Width || Frozen->SizeY != Height)
	{
		Frozen = UKismetRenderingLibrary::CreateRenderTarget2D(this, Width, Height, ETextureRenderTargetFormat::RTF_RGBA8_SRGB);
		Capture->TextureTarget = Frozen;
	}
	Capture->FOVAngle = Camera->FieldOfView;
	Capture->PostProcessSettings = Camera->PostProcessSettings;
	Capture->PostProcessBlendWeight = 1.f;
	Capture->CaptureScene();
	return Frozen;
}
