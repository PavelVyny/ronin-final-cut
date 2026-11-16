#include "View/JamEffectsView.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "View/JamShapes.h"

// The slam's rune telegraph of the full art (the session-5s playtest): a rune seal on the floor that lights from its rim
// inward to the moment of impact (M_JamSlamRune), embers rising off it (NS_JamRuneEmbers). The burst when it lands is
// JamFxRules::CuesFor's.

namespace
{
// Made by Tools/Editor/create_slam_rune.py and in the editor's Niagara toolset; /Game/FX is always cooked.
const TCHAR* const RuneMaterialPath = TEXT("/Game/FX/M_JamSlamRune.M_JamSlamRune");
const TCHAR* const RuneEmbersPath = TEXT("/Game/FX/NS_JamRuneEmbers.NS_JamRuneEmbers");
const TCHAR* const PlanePath = TEXT("/Engine/BasicShapes/Plane.Plane");
// Over the plain circles' layers, under the flashes.
constexpr float RuneHeight = 6.f;
const FLinearColor RuneColor(1.0f, 0.22f, 0.05f);
constexpr float EmberRingAtSizeOne = 60.f;
// Embers a second per cm of the circle's radius: about 75 round a slam.
constexpr float EmbersPerCm = 0.25f;
} // namespace

void AJamEffectsView::LoadRunes()
{
	RuneMaterial = LoadObject<UMaterialInterface>(nullptr, RuneMaterialPath);
	RuneEmberSystem = LoadObject<UNiagaraSystem>(nullptr, RuneEmbersPath);
	RunePlane = LoadObject<UStaticMesh>(nullptr, PlanePath);
}

bool AJamEffectsView::DrawRune(int32 Index, const FJamTelegraph& Telegraph)
{
	if (!RuneMaterial || !RunePlane)
	{
		return false;
	}
	if (Index == Runes.Num())
	{
		UStaticMeshComponent* Plane = NewObject<UStaticMeshComponent>(this);
		Plane->SetStaticMesh(RunePlane);
		Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Plane->SetCastShadow(false);
		Plane->SetupAttachment(RootComponent);
		Plane->RegisterComponent();
		UMaterialInstanceDynamic* Look = UMaterialInstanceDynamic::Create(RuneMaterial, this);
		Look->SetVectorParameterValue(TEXT("Color"), RuneColor);
		Plane->SetMaterial(0, Look);
		Runes.Add(Plane);
		RuneLooks.Add(Look);
		UNiagaraComponent* Embers = RuneEmberSystem
			? UNiagaraFunctionLibrary::SpawnSystemAttached(RuneEmberSystem, RootComponent, NAME_None, FVector::ZeroVector,
				  FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, false)
			: nullptr;
		RuneEmbers.Add(Embers);
	}
	const FVector At(Telegraph.Pos.X, Telegraph.Pos.Y, RuneHeight);
	UStaticMeshComponent& Plane = *Runes[Index];
	Plane.SetVisibility(true);
	Plane.SetWorldLocationAndRotation(At, FRotator::ZeroRotator);
	// The seal's outer ring touches the edges of its square: the plane is as wide as the hit's circle.
	Plane.SetWorldScale3D(FVector(2.f * Telegraph.Radius / JamShapes::UnitSize, 2.f * Telegraph.Radius / JamShapes::UnitSize, 1.f));
	RuneLooks[Index]->SetScalarParameterValue(TEXT("Fill"), Telegraph.Fill);
	if (UNiagaraComponent* Embers = RuneEmbers[Index])
	{
		Embers->SetWorldLocation(At);
		// NS_JamRuneEmbers (the charge aura's motes, made in the editor) rises off a ring 60 cm wide at Size 1.
		Embers->SetVariableFloat(TEXT("User.Size"), Telegraph.Radius / EmberRingAtSizeOne);
		Embers->SetVariableFloat(TEXT("User.Rate"), Telegraph.Radius * EmbersPerCm);
		Embers->SetVariableLinearColor(TEXT("User.Color"), RuneColor);
		if (!Embers->IsActive())
		{
			Embers->Activate(true);
		}
	}
	return true;
}

void AJamEffectsView::HideRunes(int32 From)
{
	for (int32 Index = From; Index < Runes.Num(); ++Index)
	{
		Runes[Index]->SetVisibility(false);
		if (RuneEmbers[Index] && RuneEmbers[Index]->IsActive())
		{
			// The embers in the air die out on their own.
			RuneEmbers[Index]->Deactivate();
		}
	}
}
