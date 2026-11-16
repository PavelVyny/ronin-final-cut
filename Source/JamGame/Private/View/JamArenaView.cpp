#include "View/JamArenaView.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "View/JamShapes.h"

namespace
{
// Greybox look of the arena: the floor and the posts the grid goes on after the second cut.
constexpr float FloorThickness = 20.f;
constexpr int32 PostCount = 16;
constexpr float PostWidth = 60.f;
constexpr float PostHeight = 400.f;
const FLinearColor FloorColor(0.012f, 0.014f, 0.022f);
const FLinearColor PostColorA(0.0f, 0.8f, 1.0f);
const FLinearColor PostColorB(1.0f, 0.1f, 0.6f);
// A neon band along the edge of the floor: the arena's end reads at a glance.
constexpr int32 EdgePieces = 64;
constexpr float EdgeWidth = 14.f;
constexpr float EdgeHeight = 2.f;
const FLinearColor EdgeColor(1.0f, 0.1f, 0.6f);
const FRotator SunRotation(-50.0, -35.0, 0.0);
// The splashes of the wet floor: a move longer than a dash in one frame is a teleport (a retry), no splash.
constexpr float MaxWadeStep = 600.f;
constexpr int32 MaxSplashesPerFrame = 2;
constexpr float SplashHeight = 2.f;
} // namespace

AJamArenaView::AJamArenaView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetupAttachment(RootComponent);
	JamShapes::Setup(*Floor, JamShapes::Cylinder());
	// The posts are made in Init and the grid comes with the layers: both have to be found here, in a constructor.
	JamShapes::Cube();
	JamShapes::Grid();
	JamShapes::Neon();

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(RootComponent);
	Sun->SetMobility(EComponentMobility::Movable);
	Sun->SetRelativeRotation(SunRotation);

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	Atmosphere->SetupAttachment(RootComponent);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetRealTimeCapture(true);
}

void AJamArenaView::Init(float ArenaRadius, const FJamNeonSettings& InNeon)
{
	Neon = InNeon;
	const float Diameter = 2.f * ArenaRadius / JamShapes::UnitSize;
	Floor->SetRelativeScale3D(FVector(Diameter, Diameter, FloorThickness / JamShapes::UnitSize));
	Floor->SetRelativeLocation(FVector(0.0, 0.0, -0.5 * FloorThickness));
	JamShapes::Look(*Floor, FloorColor, true);

	for (int32 Index = 0; Index < PostCount; ++Index)
	{
		const float Angle = UE_TWO_PI * Index / PostCount;
		const FVector Location(ArenaRadius * FMath::Cos(Angle), ArenaRadius * FMath::Sin(Angle), 0.5f * PostHeight);
		AddBox(Location, FMath::RadiansToDegrees(Angle), FVector(PostWidth, PostWidth, PostHeight),
			Index % 2 == 0 ? PostColorA : PostColorB, EArenaPiece::Post);
	}
	const float EdgeLength = UE_TWO_PI * ArenaRadius / EdgePieces;
	for (int32 Index = 0; Index < EdgePieces; ++Index)
	{
		const float Angle = UE_TWO_PI * (Index + 0.5f) / EdgePieces;
		const FVector Location(ArenaRadius * FMath::Cos(Angle), ArenaRadius * FMath::Sin(Angle), EdgeHeight);
		AddBox(Location, FMath::RadiansToDegrees(Angle) + 90.f, FVector(EdgeLength, EdgeWidth, EdgeHeight), EdgeColor, EArenaPiece::Edge);
	}
	BuildShrine(ArenaRadius);
	// Init built the shrine's look; SetLook only acts on a change, so the first look is applied here.
	const EJamArenaLook First = Look;
	Look = EJamArenaLook::Greybox;
	SetLook(First);
}

bool AJamArenaView::Shows(EArenaPiece Piece) const
{
	switch (Piece)
	{
		case EArenaPiece::Post:
			return Look == EJamArenaLook::Greybox;
		case EArenaPiece::Prop:
			return Look == EJamArenaLook::Shrine;
		default:
			return Look != EJamArenaLook::Greybox;
	}
}

void AJamArenaView::SetLook(EJamArenaLook InLook)
{
	if (InLook == Look)
	{
		return;
	}
	Look = InLook;
	Floor->SetVisibility(Look == EJamArenaLook::Greybox);
	for (int32 Index = 0; Index < Boxes.Num(); ++Index)
	{
		Dress(Index);
	}
	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		Parts[Index]->SetVisibility(Shows(PartPieces[Index]));
	}
	if (SlateFloor)
	{
		SlateFloor->SetScalarParameterValue(TEXT("CrestStrength"), Look == EJamArenaLook::Shrine ? 0.8f : 0.f);
	}
	if (Rain)
	{
		Rain->SetVisibility(Look == EJamArenaLook::Shrine);
		Rain->SetPaused(Look != EJamArenaLook::Shrine);
	}
}

void AJamArenaView::Pulse(float LineGlow)
{
	if (Look == EJamArenaLook::Greybox)
	{
		return;
	}
	// The kick of the beat over the lines' resting glow, laid on the edge's own.
	const float Glow = Neon.EdgeGlow + FMath::Max(LineGlow - Neon.LineGlow, 0.f);
	for (int32 Index = 0; Index < Boxes.Num(); ++Index)
	{
		if (Pieces[Index] == EArenaPiece::Edge)
		{
			JamShapes::Glow(*Boxes[Index], BoxColors[Index], Glow);
		}
	}
}

void AJamArenaView::Wade(int32 Who, const FVector& At, float Stride)
{
	FWader& Wader = Waders.FindOrAdd(Who);
	const float Walked = FVector::Dist2D(At, Wader.Last);
	const bool bWalked = Wader.bSeen && Walked < MaxWadeStep;
	Wader.Last = At;
	Wader.bSeen = true;
	if (!bWalked)
	{
		return;
	}
	const int32 Count = JamArenaRules::StepSplashes(Wader.Carry, Walked, Stride);
	if (Look != EJamArenaLook::Shrine || !SplashSystem)
	{
		return;
	}
	for (int32 Index = 0; Index < FMath::Min(Count, MaxSplashesPerFrame); ++Index)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SplashSystem, FVector(At.X, At.Y, SplashHeight));
	}
}

void AJamArenaView::Dress(int32 Index)
{
	UStaticMeshComponent& Box = *Boxes[Index];
	Box.SetVisibility(Shows(Pieces[Index]));
	if (Pieces[Index] == EArenaPiece::Post)
	{
		JamShapes::Look(Box, BoxColors[Index], true);
	}
	else
	{
		JamShapes::Glow(Box, BoxColors[Index], Neon.EdgeGlow);
	}
}

void AJamArenaView::AddBox(const FVector& Location, float Yaw, const FVector& Size, const FLinearColor& Color, EArenaPiece Piece)
{
	UStaticMeshComponent* Box = NewObject<UStaticMeshComponent>(this);
	JamShapes::Setup(*Box, JamShapes::Cube());
	Box->SetupAttachment(RootComponent);
	Box->RegisterComponent();
	Box->SetRelativeLocation(Location);
	Box->SetRelativeRotation(FRotator(0.0, Yaw, 0.0));
	Box->SetRelativeScale3D(Size / JamShapes::UnitSize);
	Boxes.Add(Box);
	BoxColors.Add(Color);
	Pieces.Add(Piece);
	Dress(Boxes.Num() - 1);
}
