// The rooftop shrine of the full art (T-081): its models are made by Tools/Editor/create_arena_assets.py and loaded by
// path here, so a missing one leaves its place empty instead of stopping the game.
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "View/JamArenaView.h"
#include "View/JamShapes.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamArena, Log, All);

namespace
{
const TCHAR* const PillarPath = TEXT("/Game/Arena/Props/SM_Arena_lantern_pillar.SM_Arena_lantern_pillar");
const TCHAR* const PillarCyanPath = TEXT("/Game/Arena/Materials/MI_Arena_lantern_pillar_Cyan.MI_Arena_lantern_pillar_Cyan");
const TCHAR* const PillarMagentaPath = TEXT("/Game/Arena/Materials/MI_Arena_lantern_pillar_Magenta.MI_Arena_lantern_pillar_Magenta");
const TCHAR* const ToriiPath = TEXT("/Game/Arena/Props/SM_Arena_torii.SM_Arena_torii");
const TCHAR* const LanternPath = TEXT("/Game/Arena/Props/SM_Arena_stone_lantern.SM_Arena_stone_lantern");
const TCHAR* const BonsaiPath = TEXT("/Game/Arena/Props/SM_Arena_bonsai.SM_Arena_bonsai");
const TCHAR* const RailingPath = TEXT("/Game/Arena/Props/SM_Arena_railing.SM_Arena_railing");
const TCHAR* const FloorPath = TEXT("/Game/Arena/Materials/M_JamArenaFloor.M_JamArenaFloor");
const TCHAR* const CityPath = TEXT("/Game/Arena/Materials/M_JamArenaCity.M_JamArenaCity");
const TCHAR* const RainPath = TEXT("/Game/FX/NS_JamRain.NS_JamRain");
const TCHAR* const SplashPath = TEXT("/Game/FX/NS_JamRainSplash.NS_JamRainSplash");

// The rooftop reaches past the fighting circle, so the props stand on it outside the edge band.
constexpr float RimWidth = 450.f;
constexpr float TowerDepth = 4000.f;
constexpr float SlateThickness = 20.f;
constexpr int32 PillarCount = 16;
constexpr float PillarInset = 60.f;
constexpr float PillarSize = 260.f;
constexpr float RailingOut = 220.f;
constexpr float RailingSize = 330.f;
constexpr float ToriiOut = 140.f;
constexpr float ToriiSize = 470.f;
// The camera looks along +X from the -X side: no gate there, it would hide the hero at the near edge.
const float ToriiAngles[] = { 0.f, 90.f, 270.f };
constexpr float LanternOut = 340.f;
constexpr int32 LanternCount = 8;
constexpr float LanternSize = 220.f;
constexpr float BonsaiOut = 340.f;
constexpr float BonsaiSize = 230.f;
// The crest takes the middle of the floor.
constexpr float CrestShare = 0.45f;
// The city lies far below, the size of what the camera sees past the rim.
constexpr float CityDepth = 6000.f;
constexpr float CitySize = 18000.f;
constexpr float RainHeight = 2500.f;
// T-092: the menu's skyline, a 1.75:1 picture standing far past the rim on -Y; it overfills the menu camera's view
// above the rooftop's edge.
const TCHAR* const BackdropPath = TEXT("/Game/Arena/Materials/M_JamMenuBackdrop.M_JamMenuBackdrop");
constexpr float BackdropDistance = 9000.f;
// T-094: the skyline is outpainted to a 3:1 panorama, so a wide screen shows more city instead of the sky's edge.
constexpr float BackdropAspect = 3.f;
// T-103: the menu's rim light grazes the slate from the skyline's side towards the camera (which looks along -Y), off to
// one side, so the stones' relief catches highlights. Only the floor takes lighting channel 1: the fighters keep theirs.
const FRotator RimLightRotation(-7.0, 130.0, 0.0);
constexpr float RimLightLux = 6.f;
const FLinearColor RimLightColor(0.75f, 0.85f, 1.0f);

FVector OnCircle(float Radius, float Degrees, float Z = 0.f)
{
	const float Radians = FMath::DegreesToRadians(Degrees);
	return FVector(Radius * FMath::Cos(Radians), Radius * FMath::Sin(Radians), Z);
}

template <typename T>
T* Load(const TCHAR* Path)
{
	T* Object = LoadObject<T>(nullptr, Path);
	if (!Object)
	{
		UE_LOG(LogJamArena, Warning, TEXT("Shrine asset missing: %s"), Path);
	}
	return Object;
}
} // namespace

UStaticMeshComponent* AJamArenaView::AddModel(
	UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Location, float Yaw, float Size, EArenaPiece Piece)
{
	if (!Mesh)
	{
		return nullptr;
	}
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	JamShapes::Setup(*Part, Mesh);
	if (Material)
	{
		Part->SetMaterial(0, Material);
	}
	else
	{
		Part->SetMaterial(0, Mesh->GetMaterial(0));
	}
	Part->SetupAttachment(RootComponent);
	Part->RegisterComponent();
	const FBox Bounds = Mesh->GetBoundingBox();
	const float Scale = Size / FMath::Max(Bounds.GetSize().GetMax(), 1.f);
	Part->SetRelativeScale3D(FVector(Scale));
	Part->SetRelativeRotation(FRotator(0.0, Yaw, 0.0));
	// Stand the model on Location: the bottom of its box, not its pivot.
	Part->SetRelativeLocation(
		Location - FRotator(0.0, Yaw, 0.0).RotateVector(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z)) * Scale);
	Parts.Add(Part);
	PartPieces.Add(Piece);
	return Part;
}

void AJamArenaView::BuildShrine(float ArenaRadius)
{
	BuildRooftop(ArenaRadius);
	BuildRim(ArenaRadius);
	BuildSky();
}

void AJamArenaView::BuildRooftop(float ArenaRadius)
{
	const float Rim = ArenaRadius + RimWidth;
	if (UMaterialInterface* Slate = Load<UMaterialInterface>(FloorPath))
	{
		SlateFloor = UMaterialInstanceDynamic::Create(Slate, this);
		SlateFloor->SetScalarParameterValue(TEXT("CrestRadius"), CrestShare * ArenaRadius);
		// The top of the rooftop and the tower under it, one cylinder each.
		const float RimDiameter = 2.f * Rim / JamShapes::UnitSize;
		if (UStaticMeshComponent* Top =
				AddModel(JamShapes::Cylinder(), SlateFloor, FVector(0.0, 0.0, -SlateThickness), 0.f, 2.f * Rim, EArenaPiece::Shrine))
		{
			Top->SetRelativeScale3D(FVector(RimDiameter, RimDiameter, SlateThickness / JamShapes::UnitSize));
			Top->SetRelativeLocation(FVector(0.0, 0.0, -0.5 * SlateThickness));
			Top->SetLightingChannels(true, true, false);
		}
		if (UStaticMeshComponent* Tower =
				AddModel(JamShapes::Cylinder(), SlateFloor, FVector::ZeroVector, 0.f, 2.f * Rim, EArenaPiece::Shrine))
		{
			Tower->SetRelativeScale3D(FVector(RimDiameter, RimDiameter, TowerDepth / JamShapes::UnitSize));
			Tower->SetRelativeLocation(FVector(0.0, 0.0, -SlateThickness - 0.5 * TowerDepth));
		}
	}
	RimLight = NewObject<UDirectionalLightComponent>(this);
	RimLight->SetupAttachment(RootComponent);
	RimLight->SetMobility(EComponentMobility::Movable);
	RimLight->SetRelativeRotation(RimLightRotation);
	RimLight->SetIntensity(RimLightLux);
	RimLight->SetLightColor(RimLightColor);
	RimLight->SetCastShadows(false);
	RimLight->SetLightingChannels(false, true, false);
	// The sun stays the one light of forward shading, the rain's translucency and the fog.
	RimLight->ForwardShadingPriority = -1;
	RimLight->SetVisibility(false);
	RimLight->RegisterComponent();
}

void AJamArenaView::BuildRim(float ArenaRadius)
{
	UStaticMesh* Pillar = Load<UStaticMesh>(PillarPath);
	UMaterialInterface* PillarGlows[] = { Load<UMaterialInterface>(PillarCyanPath), Load<UMaterialInterface>(PillarMagentaPath) };
	UStaticMesh* Railing = Load<UStaticMesh>(RailingPath);
	for (int32 Index = 0; Index < PillarCount; ++Index)
	{
		const float Degrees = 360.f * Index / PillarCount;
		AddModel(Pillar, PillarGlows[Index % 2], OnCircle(ArenaRadius + PillarInset, Degrees), Degrees, PillarSize, EArenaPiece::Shrine);
		// Two railing pieces between each pair of pillars, along the circle.
		for (const float Quarter : { 0.25f, 0.75f })
		{
			const float RailDegrees = 360.f * (Index + Quarter) / PillarCount;
			AddModel(Railing, nullptr, OnCircle(ArenaRadius + RailingOut, RailDegrees), RailDegrees + 90.f, RailingSize, EArenaPiece::Prop);
		}
	}
	UStaticMesh* Torii = Load<UStaticMesh>(ToriiPath);
	for (const float Degrees : ToriiAngles)
	{
		AddModel(Torii, nullptr, OnCircle(ArenaRadius + ToriiOut, Degrees), Degrees + 90.f, ToriiSize, EArenaPiece::Prop);
	}
	UStaticMesh* Lantern = Load<UStaticMesh>(LanternPath);
	for (int32 Index = 0; Index < LanternCount; ++Index)
	{
		const float Degrees = 360.f * (Index + 0.5f) / LanternCount;
		AddModel(Lantern, nullptr, OnCircle(ArenaRadius + LanternOut, Degrees), Degrees + 180.f, LanternSize, EArenaPiece::Prop);
	}
	UStaticMesh* Bonsai = Load<UStaticMesh>(BonsaiPath);
	for (const float Degrees : { 45.f, 135.f, 225.f, 315.f })
	{
		AddModel(Bonsai, nullptr, OnCircle(ArenaRadius + BonsaiOut, Degrees), Degrees + 90.f, BonsaiSize, EArenaPiece::Prop);
	}
}

void AJamArenaView::BuildSky()
{
	if (UMaterialInterface* City = Load<UMaterialInterface>(CityPath))
	{
		if (UStaticMeshComponent* Ground = AddModel(JamShapes::Cube(), City, FVector::ZeroVector, 0.f, CitySize, EArenaPiece::Prop))
		{
			Ground->SetRelativeScale3D(FVector(CitySize, CitySize, 10.f) / JamShapes::UnitSize);
			Ground->SetRelativeLocation(FVector(0.0, 0.0, -CityDepth));
			Ground->SetCastShadow(false);
		}
	}
	if (UNiagaraSystem* RainSystem = LoadObject<UNiagaraSystem>(nullptr, RainPath))
	{
		Rain = NewObject<UNiagaraComponent>(this);
		Rain->SetAsset(RainSystem);
		Rain->SetupAttachment(RootComponent);
		Rain->SetRelativeLocation(FVector(0.0, 0.0, RainHeight));
		Rain->RegisterComponent();
		Rain->Activate(true);
	}
	SplashSystem = LoadObject<UNiagaraSystem>(nullptr, SplashPath);
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UMaterialInterface* Skyline = Load<UMaterialInterface>(BackdropPath);
	if (Plane && Skyline)
	{
		Backdrop = NewObject<UStaticMeshComponent>(this);
		JamShapes::Setup(*Backdrop, Plane);
		Backdrop->SetMaterial(0, Skyline);
		Backdrop->SetupAttachment(RootComponent);
		Backdrop->RegisterComponent();
		// The engine plane faces +Z with U along +X and V along +Y: stood up facing +Y, the picture's left is the
		// camera's left and its top is up.
		Backdrop->SetRelativeRotation(FRotationMatrix::MakeFromXY(FVector(1.0, 0.0, 0.0), FVector(0.0, 0.0, -1.0)).Rotator());
		// Sized each menu frame by PlaceBackdrop, for the screen it is on.
		Backdrop->SetCastShadow(false);
		// A lit picture this size must not light the scene through Lumen.
		Backdrop->SetAffectDynamicIndirectLighting(false);
		Backdrop->SetVisibility(false);
	}
}

void AJamArenaView::PlaceBackdrop(const FVector& Eye, const FVector& LookAt, float Fov, float Aspect)
{
	if (!Backdrop)
	{
		return;
	}
	const FVector Origin = GetActorLocation();
	const FVector LocalEye = Eye - Origin;
	const FJamBackdropPlace Place =
		JamArenaRules::PlaceBackdrop(LocalEye, LookAt - Origin, Fov, Aspect, BackdropAspect, LocalEye.Y + BackdropDistance);
	Backdrop->SetRelativeScale3D(FVector(Place.Width, Place.Height, 1.0) / JamShapes::UnitSize);
	Backdrop->SetRelativeLocation(FVector(0.0, -BackdropDistance, Place.CentreZ));
}

void AJamArenaView::ShowBackdrop(bool bShown)
{
	if (Backdrop)
	{
		Backdrop->SetVisibility(bShown);
	}
	if (RimLight)
	{
		RimLight->SetVisibility(bShown);
	}
}
