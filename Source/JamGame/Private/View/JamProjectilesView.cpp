#include "View/JamProjectilesView.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "View/JamShapes.h"
#include "View/JamShotLightRules.h"

namespace
{
// The simulation is flat: this is the height the projectiles are drawn at, about the hero's chest.
constexpr float FlightHeight = 110.f;
// The shurikens' ribbon of the full art (T-069), made in the editor.
const TCHAR* const TrailPath = TEXT("/Game/FX/NS_JamTrail.NS_JamTrail");
constexpr float TrailWidth = 10.f;
// A new shot leaves the thrower's hand and is on its flight line after this long (T-065).
constexpr float LaunchSeconds = 0.08f;
const FLinearColor HeroShotColor(0.3f, 0.95f, 1.0f);
const FLinearColor BossShotColor(1.0f, 0.25f, 0.2f);
// Shots shine: in the dark arena they are the brightest things that move.
constexpr float ShotGlow = 2.5f;
// Greybox looks of the kinds, in projectile diameters: presentation only. The shapes are drawn larger than the bodies
// that hit, so they can be read from the top-down camera.
constexpr float ShurikenWidth = 2.2f;
// SM_Shuriken (Hunyuan3D) stands in its XZ plane, 120 cm across, its centre 60 cm above its pivot: a roll lays it
// flat, and it spins about its centre as it flies.
constexpr float ShurikenModelSize = 120.f;
const FVector ShurikenModelCentre(0.0, 0.0, 60.0);
constexpr float ShurikenTurnsPerSecond = 3.f;
// An explosive shuriken is yellow, not orange: orange is too close to the Publisher's red. It pulses as it flies.
const FLinearColor ExplosiveColor(1.0f, 0.9f, 0.05f);
constexpr float ExplosivePulse = 0.35f;
constexpr float ExplosivePulsesPerSecond = 6.f;
const FVector MemoSize(1.5, 1.1, 0.12);
// A memo is a sheet of paper flying edge first, tilted so that every camera sees its face.
constexpr float MemoRoll = 50.f;
// The memo card (r5_memo_card) is lit by itself, a little above its own colour so it reads on the dark floor.
constexpr float MemoGlow = 1.3f;
// T-088: in the full art a flying shuriken lights a small pool of floor under it in its colour. No shadows, a tight
// reach, a few at once (the newest); an explosive one a little brighter.
constexpr int32 ShotLightCap = 6;
constexpr float ShotLightCandela = 12.f;
constexpr float ShotLightReach = 240.f;
constexpr float ShotLightExplosive = 1.5f;

FLinearColor ColorOf(const FJamProjectile& Projectile)
{
	return Projectile.bExplosive ? ExplosiveColor : (Projectile.Owner == EJamSide::Hero ? HeroShotColor : BossShotColor);
}

void SetShape(UStaticMeshComponent& Shape, UStaticMesh* Mesh)
{
	if (Shape.GetStaticMesh() != Mesh)
	{
		Shape.SetStaticMesh(Mesh);
	}
}

/** Gives a pooled shape the look of the projectile's kind. */
void Dress(UStaticMeshComponent& Shape, const FJamProjectile& Projectile, float Time)
{
	const float Pulse = Projectile.bExplosive ? 1.f + ExplosivePulse * FMath::Abs(FMath::Sin(PI * ExplosivePulsesPerSecond * Time)) : 1.f;
	const float Diameter = 2.f * Projectile.Radius * Pulse / JamShapes::UnitSize;
	const FLinearColor SideColor = Projectile.Owner == EJamSide::Hero ? HeroShotColor : BossShotColor;
	switch (Projectile.Kind)
	{
		case EJamProjectile::Shuriken:
		{
			SetShape(Shape, JamShapes::Shuriken());
			const float Scale = Diameter * ShurikenWidth * JamShapes::UnitSize / ShurikenModelSize;
			const FRotator Turn(0.0, 360.0 * ShurikenTurnsPerSecond * Time + 37.0 * Projectile.Id, 90.0);
			Shape.SetWorldRotation(Turn);
			Shape.SetWorldScale3D(FVector(Scale));
			// The pivot sits on the rim: the model moves so its centre is on the flight line.
			Shape.AddWorldOffset(-Turn.RotateVector(ShurikenModelCentre * Scale));
			// The model's shape in the colour of its side: whose it is and whether it explodes must read at a glance.
			JamShapes::Glow(Shape, Projectile.bExplosive ? ExplosiveColor : SideColor, ShotGlow);
			break;
		}
		case EJamProjectile::Memo:
			SetShape(Shape, JamShapes::Cube());
			Shape.SetWorldRotation(FRotator(0.0, FMath::RadiansToDegrees(FMath::Atan2(Projectile.Vel.Y, Projectile.Vel.X)), MemoRoll));
			Shape.SetWorldScale3D(MemoSize * Diameter);
			JamShapes::Card(Shape, JamShapes::MemoCard(), MemoGlow);
			break;
		default:
			SetShape(Shape, JamShapes::Sphere());
			Shape.SetWorldRotation(FRotator::ZeroRotator);
			Shape.SetWorldScale3D(FVector(Diameter));
			JamShapes::Glow(Shape, SideColor, ShotGlow);
			break;
	}
}
} // namespace

AJamProjectilesView::AJamProjectilesView()
{
	PrimaryActorTick.bCanEverTick = false;
	Hands[EJamSide::Hero] = FVector::ZeroVector;
	Hands[EJamSide::Boss] = FVector::ZeroVector;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	// The pool is filled at runtime: the shapes have to be found here, in a constructor.
	JamShapes::Sphere();
	JamShapes::Cylinder();
	JamShapes::Cube();
	JamShapes::Material();
	JamShapes::Neon();
	JamShapes::Shuriken();
	JamShapes::MemoCard();
}

void AJamProjectilesView::Sync(const TArray<FJamProjectile>& Projectiles, float Alpha, float Time)
{
	// Forgets the shots that are gone: ids are never reused, the map stays as small as the fight.
	for (auto It = FirstSeen.CreateIterator(); It; ++It)
	{
		if (!Projectiles.ContainsByPredicate([Id = It.Key()](const FJamProjectile& P) { return P.Id == Id; }))
		{
			It.RemoveCurrent();
		}
	}
	TSet<int32> Kept;
	DrawnAt.Reset();
	while (Pool.Num() < Projectiles.Num())
	{
		UStaticMeshComponent* Shape = NewObject<UStaticMeshComponent>(this);
		JamShapes::Setup(*Shape, JamShapes::Sphere());
		Shape->SetupAttachment(RootComponent);
		Shape->RegisterComponent();
		Pool.Add(Shape);
	}
	for (int32 Index = 0; Index < Pool.Num(); ++Index)
	{
		UStaticMeshComponent& Shape = *Pool[Index];
		const bool bUsed = Index < Projectiles.Num();
		Shape.SetVisibility(bUsed);
		if (!bUsed)
		{
			continue;
		}
		const FJamProjectile& Projectile = Projectiles[Index];
		const FVector2D Pos = FMath::Lerp(Projectile.PrevPos, Projectile.Pos, static_cast<double>(Alpha));
		const float Age = Time - FirstSeen.FindOrAdd(Projectile.Id, Time);
		const FVector OnLine(Pos.X, Pos.Y, FlightHeight);
		const FVector Hand = Hands[Projectile.Owner];
		const bool bLeaving = Age < LaunchSeconds && !Hand.IsNearlyZero();
		const FVector Drawn = bLeaving ? FMath::Lerp(Hand, OnLine, FMath::SmoothStep(0.f, 1.f, Age / LaunchSeconds)) : OnLine;
		Shape.SetWorldLocation(Drawn);
		Dress(Shape, Projectile, Time);
		Trail(Projectile, Drawn, Kept);
		DrawnAt.Add(Drawn);
	}
	LightShots(Projectiles);
	// A shot that is gone lets its ribbon fade out on its own.
	for (auto It = Trails.CreateIterator(); It; ++It)
	{
		if (!Kept.Contains(It.Key()))
		{
			if (It.Value())
			{
				It.Value()->SetAutoDestroy(true);
				It.Value()->Deactivate();
			}
			It.RemoveCurrent();
		}
	}
}

void AJamProjectilesView::LightShots(const TArray<FJamProjectile>& Projectiles)
{
	while (ShotLights.Num() < ShotLightCap)
	{
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetCastShadows(false);
		Light->SetIntensityUnits(ELightUnits::Candelas);
		Light->SetAttenuationRadius(ShotLightReach);
		Light->SetVisibility(false);
		Light->RegisterComponent();
		ShotLights.Add(Light);
	}
	const TArray<int32> Lit = bArt ? JamShotLightRules::Lit(Projectiles, ShotLightCap) : TArray<int32>();
	for (int32 Index = 0; Index < ShotLights.Num(); ++Index)
	{
		UPointLightComponent& Light = *ShotLights[Index];
		Light.SetVisibility(Lit.IsValidIndex(Index));
		if (!Lit.IsValidIndex(Index))
		{
			continue;
		}
		const FJamProjectile& Projectile = Projectiles[Lit[Index]];
		Light.SetWorldLocation(DrawnAt[Lit[Index]]);
		Light.SetLightColor(ColorOf(Projectile));
		Light.SetIntensity(ShotLightCandela * JamShotLightRules::Brightness(Projectile, ShotLightExplosive));
	}
}

void AJamProjectilesView::Trail(const FJamProjectile& Projectile, const FVector& Location, TSet<int32>& Kept)
{
	if (!bArt || Projectile.Kind != EJamProjectile::Shuriken)
	{
		return;
	}
	if (!TrailSystem)
	{
		TrailSystem = LoadObject<UNiagaraSystem>(nullptr, TrailPath);
	}
	if (!TrailSystem)
	{
		return;
	}
	Kept.Add(Projectile.Id);
	TObjectPtr<UNiagaraComponent>& Ribbon = Trails.FindOrAdd(Projectile.Id);
	if (!Ribbon)
	{
		Ribbon = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TrailSystem, Location, FRotator::ZeroRotator, FVector(1.f), false);
		if (Ribbon)
		{
			Ribbon->SetVariableLinearColor(TEXT("User.Color"), ColorOf(Projectile));
			Ribbon->SetVariableFloat(TEXT("User.Width"), TrailWidth);
		}
	}
	if (Ribbon)
	{
		Ribbon->SetWorldLocation(Location);
	}
}
