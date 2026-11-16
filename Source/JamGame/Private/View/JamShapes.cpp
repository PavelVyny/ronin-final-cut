#include "View/JamShapes.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UStaticMesh* JamShapes::Sphere()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	return Finder.Object;
}

UStaticMesh* JamShapes::Cylinder()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	return Finder.Object;
}

UStaticMesh* JamShapes::Cube()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	return Finder.Object;
}

UStaticMesh* JamShapes::Cone()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	return Finder.Object;
}

UMaterialInterface* JamShapes::Grid()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(
		TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));
	return Finder.Object;
}

UMaterialInterface* JamShapes::Neon()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(TEXT("/Game/Materials/M_JamNeon.M_JamNeon"));
	return Finder.Object;
}

UStaticMesh* JamShapes::Shuriken()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(
		TEXT("/Game/Props/Shuriken/model_glb/StaticMeshes/SM_Shuriken.SM_Shuriken"));
	return Finder.Object;
}

UStaticMesh* JamShapes::CashStack()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(
		TEXT("/Game/Props/CashStack/model_glb/StaticMeshes/SM_CashStack.SM_CashStack"));
	return Finder.Object;
}

UMaterialInterface* JamShapes::MemoCard()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(TEXT("/Game/Props/Materials/MI_MemoCard.MI_MemoCard"));
	return Finder.Object;
}

UMaterialInterface* JamShapes::DollarCard()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(TEXT("/Game/Props/Materials/MI_DollarCard.MI_DollarCard"));
	return Finder.Object;
}

namespace
{
/** The component's own instance of Parent: made anew when it wears anything else (the grid or the other material). */
UMaterialInstanceDynamic* InstanceOf(UStaticMeshComponent& Component, UMaterialInterface* Parent)
{
	UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(Component.GetMaterial(0));
	if (!Dynamic || Dynamic->Parent != Parent)
	{
		Component.SetMaterial(0, Parent);
		Dynamic = Component.CreateDynamicMaterialInstance(0);
	}
	return Dynamic;
}
} // namespace

UMaterialInterface* JamShapes::Material()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	return Finder.Object;
}

void JamShapes::Setup(UStaticMeshComponent& Component, UStaticMesh* Mesh)
{
	Component.SetStaticMesh(Mesh);
	Component.SetMaterial(0, Material());
	Component.SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component.SetGenerateOverlapEvents(false);
	Component.SetMobility(EComponentMobility::Movable);
}

void JamShapes::Paint(UStaticMeshComponent& Component, const FLinearColor& Color)
{
	if (UMaterialInstanceDynamic* Dynamic = InstanceOf(Component, Material()))
	{
		Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void JamShapes::Glow(UStaticMeshComponent& Component, const FLinearColor& Color, float Strength)
{
	if (UMaterialInstanceDynamic* Dynamic = InstanceOf(Component, Neon()))
	{
		Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
		Dynamic->SetScalarParameterValue(TEXT("Glow"), Strength);
	}
}

void JamShapes::Card(UStaticMeshComponent& Component, UMaterialInterface* Card, float Glow)
{
	if (UMaterialInstanceDynamic* Dynamic = InstanceOf(Component, Card))
	{
		Dynamic->SetScalarParameterValue(TEXT("Glow"), Glow);
	}
}

void JamShapes::Look(UStaticMeshComponent& Component, const FLinearColor& Color, bool bGrid)
{
	if (bGrid)
	{
		Component.SetMaterial(0, Grid());
		return;
	}
	Paint(Component, Color);
}
