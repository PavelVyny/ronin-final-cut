#include "View/JamLightView.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "View/JamSpotRules.h"

namespace
{
// T-094: the rain haze is the night's own colour, a dim blue violet, so the neon still carries the frame.
const FLinearColor HazeColor(0.05f, 0.05f, 0.11f);
// Presentation only. One light per post of the arena, in the post's colour, a little inside the edge so it lights the
// floor of the fight and not the dark outside it.
constexpr int32 LampCount = 16;
constexpr float LampCandela = 55.f;
constexpr float LampReach = 1100.f;
constexpr float LampHeight = 300.f;
constexpr float LampInset = 90.f;
// A wide source: the neon spills over the floor as a pool, not a hard dot.
constexpr float LampSize = 40.f;
const FLinearColor LampCyan(0.0f, 0.8f, 1.0f);
const FLinearColor LampMagenta(1.0f, 0.1f, 0.6f);
// The full art is a night: the stage's sun keeps this share of its light, the neon does the rest.
constexpr float NightShare = 0.25f;
// T-082: a key light above and in front of the Publisher follows him, so his show-off reads in the dark.
constexpr float KeyCandela = 1100.f;
constexpr float KeyHeight = 700.f;
constexpr float KeyAhead = 350.f;
constexpr float KeyAimHeight = 120.f;
// T-087: the pool is a fifth wider across than the first cones of 8 and 18 degrees (tan scaled by 1.2).
constexpr float KeyInnerCone = 9.6f;
constexpr float KeyOuterCone = 21.3f;
const FLinearColor KeyWarm(1.0f, 0.9f, 0.85f);
// T-087: a hand works the light: the spot trails him by half a second and wanders up to this far off him.
constexpr float KeyLag = 0.5f;
constexpr float KeySway = 25.f;
// T-100: the hero's own key light, smaller and dimmer than the Publisher's: it lifts him out of the dark without
// flattening the night. Its hand sways out of step with the other one.
constexpr float HeroKeyCandela = 450.f;
constexpr float HeroKeyAimHeight = 90.f;
constexpr float HeroKeyInnerCone = 6.f;
constexpr float HeroKeyOuterCone = 14.f;
constexpr float HeroSwayPhase = 11.3f;
} // namespace

AJamLightView::AJamLightView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	Haze = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Haze"));
	Haze->SetupAttachment(RootComponent);
	Haze->SetFogDensity(0.f);
	Haze->SetVisibility(false);
}

void AJamLightView::Init(float ArenaRadius, const AActor& Stage)
{
	Sun = Stage.FindComponentByClass<UDirectionalLightComponent>();
	if (Sun)
	{
		DaySun = Sun->Intensity;
		DimSun();
	}
	Key = MakeKey(KeyCandela, KeyInnerCone, KeyOuterCone);
	HeroKey = MakeKey(HeroKeyCandela, HeroKeyInnerCone, HeroKeyOuterCone);
	const float Inner = ArenaRadius - LampInset;
	for (int32 Index = 0; Index < LampCount; ++Index)
	{
		const float Angle = UE_TWO_PI * Index / LampCount;
		UPointLightComponent* Lamp = NewObject<UPointLightComponent>(this);
		Lamp->SetupAttachment(RootComponent);
		Lamp->SetMobility(EComponentMobility::Movable);
		// Sixteen shadowed lights would cost more than the whole fight; the neon only has to colour the scene.
		Lamp->SetCastShadows(false);
		Lamp->SetIntensityUnits(ELightUnits::Candelas);
		Lamp->SetIntensity(LampCandela);
		Lamp->SetAttenuationRadius(LampReach);
		Lamp->SetSourceRadius(LampSize);
		Lamp->SetLightColor(Index % 2 == 0 ? LampCyan : LampMagenta);
		Lamp->SetVisibility(Light == EJamLight::Neon);
		Lamp->RegisterComponent();
		Lamp->SetRelativeLocation(FVector(Inner * FMath::Cos(Angle), Inner * FMath::Sin(Angle), LampHeight));
		Lamps.Add(Lamp);
	}
}

void AJamLightView::SetLight(EJamLight InLight)
{
	if (InLight == Light)
	{
		return;
	}
	Light = InLight;
	for (UPointLightComponent* Lamp : Lamps)
	{
		Lamp->SetVisibility(Light == EJamLight::Neon);
	}
	for (USpotLightComponent* Each : { Key.Get(), HeroKey.Get() })
	{
		if (Each)
		{
			Each->SetVisibility(Light == EJamLight::Neon);
		}
	}
	DimSun();
}

void AJamLightView::SetHaze(const FJamFilmLook& Look)
{
	const bool bOn = Look.FogDensity > 0.f;
	Haze->SetVisibility(bOn);
	if (!bOn)
	{
		return;
	}
	Haze->SetFogDensity(Look.FogDensity);
	Haze->SetFogHeightFalloff(Look.FogFalloff);
	Haze->SetStartDistance(Look.FogStart);
	Haze->SetFogMaxOpacity(Look.FogOpacity);
	Haze->SetFogInscatteringColor(HazeColor);
}

void AJamLightView::Follow(const FVector& BossAt, const FVector& HeroAt, float Time, float ToHero)
{
	if (!Key || !HeroKey)
	{
		return;
	}
	const FVector2D Boss(BossAt);
	const FVector2D Hero(HeroAt);
	if (SpotTime < 0.f)
	{
		// The first frame finds them already lit.
		Spot.At = Boss;
		HeroSpot.At = Hero;
	}
	const float Dt = FMath::Clamp(Time - FMath::Max(SpotTime, 0.f), 0.f, 0.1f);
	SpotTime = Time;
	Spot = JamSpotRules::Follow(Spot, Boss, Dt, KeyLag);
	if (ToHero < 1.f)
	{
		// T-100: the intro's camera is on its way to the hero: his spot rides with it out of the Publisher's beam.
		HeroSpot.At = JamSpotRules::SplitOff(Spot.At, Hero, ToHero);
		HeroSpot.Velocity = FVector2D::ZeroVector;
	}
	else
	{
		HeroSpot = JamSpotRules::Follow(HeroSpot, Hero, Dt, KeyLag);
	}
	HeroKey->SetIntensity(HeroKeyCandela * JamSpotRules::SplitShare(ToHero));
	Aim(*Key, Spot.At + JamSpotRules::Sway(Time, KeySway), KeyAimHeight);
	Aim(*HeroKey, HeroSpot.At + JamSpotRules::Sway(Time + HeroSwayPhase, KeySway), HeroKeyAimHeight);
}

USpotLightComponent* AJamLightView::MakeKey(float Candela, float InnerCone, float OuterCone)
{
	USpotLightComponent* Made = NewObject<USpotLightComponent>(this);
	Made->SetupAttachment(RootComponent);
	Made->SetMobility(EComponentMobility::Movable);
	Made->SetCastShadows(false);
	Made->SetIntensityUnits(ELightUnits::Candelas);
	Made->SetIntensity(Candela);
	Made->SetAttenuationRadius(2.f * KeyHeight);
	Made->SetInnerConeAngle(InnerCone);
	Made->SetOuterConeAngle(OuterCone);
	Made->SetLightColor(KeyWarm);
	Made->SetVisibility(Light == EJamLight::Neon);
	Made->RegisterComponent();
	return Made;
}

void AJamLightView::Aim(USpotLightComponent& Beam, const FVector2D& At, float AimHeight)
{
	// From above and from the camera's side (the camera looks down from -X), aimed at the chest.
	const FVector From(At.X - KeyAhead, At.Y, KeyHeight);
	const FVector Target(At.X, At.Y, AimHeight);
	Beam.SetWorldLocationAndRotation(From, (Target - From).Rotation());
}

void AJamLightView::DimSun()
{
	if (Sun)
	{
		Sun->SetIntensity(Light == EJamLight::Neon ? DaySun * NightShare : DaySun);
	}
}
