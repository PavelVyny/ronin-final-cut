#include "View/JamFighterView.h"

#include "Components/StaticMeshComponent.h"
#include "View/JamShapes.h"

// The shape ronin of before the models (T-067): the hero's look after the first cut, his blade and his walk.

namespace
{
// The shape ronin of before the models (T-067), in body radii: sizes of the shapes and where their centres are,
// relative to the feet.
const FVector TorsoSize(1.5, 1.3, 2.8);
constexpr float TorsoAt = 1.4f;
constexpr float HeadSize = 1.f;
constexpr float HeadAt = 3.35f;
const FVector EyesSize(0.25, 0.8, 0.18);
const FVector EyesAt(0.42, 0.0, 3.4);
const FVector HatSize(1.8, 1.8, 0.6);
constexpr float HatAt = 3.85f;
const FVector ShoulderAt(0.0, 0.95, 2.55);
const FVector ArmSize(0.38, 0.38, 1.6);
const FVector BladeSize(2.4, 0.12, 0.3);
const FVector BladeAt(1.2, 0.0, -1.55);
const FLinearColor ClothColor(0.03f, 0.03f, 0.05f);
const FLinearColor SkinColor(0.08f, 0.08f, 0.1f);
constexpr float HatShade = 0.45f;
constexpr float BladeWhiteness = 0.6f;
// Its walk at full speed: steps per second, bob height in radii, lean and arm swing in degrees; idle arms a little out.
constexpr float StepsPerSecond = 2.4f;
constexpr float BobInRadii = 0.18f;
constexpr float LeanDegrees = 12.f;
constexpr float ArmSwingDegrees = 35.f;
constexpr float IdleArmDegrees = 8.f;
} // namespace

void AJamFighterView::ConstructRonin()
{
	Ronin = CreateDefaultSubobject<USceneComponent>(TEXT("Ronin"));
	Ronin->SetupAttachment(RootComponent);
	Torso = AddPart(Ronin, TEXT("Torso"), JamShapes::Cylinder());
	Head = AddPart(Ronin, TEXT("Head"), JamShapes::Sphere());
	Eyes = AddPart(Ronin, TEXT("Eyes"), JamShapes::Cube());
	Hat = AddPart(Ronin, TEXT("Hat"), JamShapes::Cone());
	ShoulderL = CreateDefaultSubobject<USceneComponent>(TEXT("ShoulderL"));
	ShoulderL->SetupAttachment(Ronin);
	ShoulderR = CreateDefaultSubobject<USceneComponent>(TEXT("ShoulderR"));
	ShoulderR->SetupAttachment(Ronin);
	ArmL = AddPart(ShoulderL, TEXT("ArmL"), JamShapes::Cube());
	ArmR = AddPart(ShoulderR, TEXT("ArmR"), JamShapes::Cube());
	Blade = AddPart(ShoulderR, TEXT("Blade"), JamShapes::Cube());
}

UStaticMeshComponent* AJamFighterView::AddPart(USceneComponent* Parent, const TCHAR* Name, UStaticMesh* Mesh)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(Parent);
	JamShapes::Setup(*Part, Mesh);
	return Part;
}

void AJamFighterView::InitRonin(float Radius)
{
	const float R = Radius / JamShapes::UnitSize;
	Torso->SetRelativeScale3D(TorsoSize * R);
	Torso->SetRelativeLocation(FVector(0.0, 0.0, TorsoAt * Radius));
	Head->SetRelativeScale3D(FVector(HeadSize * R));
	Head->SetRelativeLocation(FVector(0.0, 0.0, HeadAt * Radius));
	Eyes->SetRelativeScale3D(EyesSize * R);
	Eyes->SetRelativeLocation(EyesAt * Radius);
	Hat->SetRelativeScale3D(HatSize * R);
	Hat->SetRelativeLocation(FVector(0.0, 0.0, HatAt * Radius));
	ShoulderL->SetRelativeLocation(FVector(ShoulderAt.X, -ShoulderAt.Y, ShoulderAt.Z) * Radius);
	ShoulderR->SetRelativeLocation(ShoulderAt * Radius);
	for (UStaticMeshComponent* Arm : { ArmL.Get(), ArmR.Get() })
	{
		Arm->SetRelativeScale3D(ArmSize * R);
		Arm->SetRelativeLocation(FVector(0.0, 0.0, -0.5 * ArmSize.Z * Radius));
	}
	Blade->SetRelativeScale3D(BladeSize * R);
	Blade->SetRelativeLocation(BladeAt * Radius);
}

void AJamFighterView::PaintRonin(bool bGrid)
{
	const auto Look = [this, bGrid](UStaticMeshComponent& Part, const FLinearColor& Color)
	{ JamShapes::Look(Part, bFlashing ? FMath::Lerp(Color, FLinearColor::White, FlashWeight) : Color, bGrid); };
	Look(*Torso, ClothColor);
	Look(*ArmL, ClothColor);
	Look(*ArmR, ClothColor);
	Look(*Head, SkinColor);
	Look(*Eyes, BaseColor);
	Look(*Hat, BaseColor * HatShade);
	Look(*Blade, FMath::Lerp(BaseColor, FLinearColor::White, BladeWhiteness));
}

void AJamFighterView::AnimateRonin(float Moving, float Time)
{
	if (!Shown.bAnimation)
	{
		// What a character is before anybody animates it.
		Ronin->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		ShoulderL->SetRelativeRotation(FRotator(0.0, 0.0, -90.0));
		ShoulderR->SetRelativeRotation(FRotator(0.0, 0.0, 90.0));
		return;
	}
	const float Phase = UE_TWO_PI * StepsPerSecond * Time;
	const float Swing = FMath::Sin(Phase) * ArmSwingDegrees * Moving;
	Ronin->SetRelativeLocationAndRotation(
		FVector(0.0, 0.0, FMath::Abs(FMath::Sin(Phase)) * BobInRadii * BodyRadius * Moving), FRotator(-LeanDegrees * Moving, 0.0, 0.0));
	ShoulderL->SetRelativeRotation(FRotator(Swing, 0.0, -IdleArmDegrees));
	ShoulderR->SetRelativeRotation(FRotator(-Swing, 0.0, IdleArmDegrees));
}
