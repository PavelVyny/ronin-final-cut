#include "View/JamFxView.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "View/JamFxRules.h"

namespace
{
// The systems are made in the editor (Niagara toolset), loaded by path: /Game/FX is always cooked.
const TCHAR* const SparksPath = TEXT("/Game/FX/NS_JamHitSparks.NS_JamHitSparks");
const TCHAR* const DustPath = TEXT("/Game/FX/NS_JamSlamDust.NS_JamSlamDust");
const TCHAR* const CashPath = TEXT("/Game/FX/NS_JamCashBurst.NS_JamCashBurst");
const TCHAR* const EmbersPath = TEXT("/Game/FX/NS_JamEmbers.NS_JamEmbers");
const TCHAR* const ChargePath = TEXT("/Game/FX/NS_JamUltCharge.NS_JamUltCharge");
// The charge aura burns in the Ultimate's magenta; the burst when it is full comes off the chest.
const FLinearColor ChargeMagenta(1.0f, 0.3f, 0.85f);
constexpr float ChestHeight = 120.f;
constexpr float ReadyBurst = 1.6f;
// The dust and the cash start a little above the floor, under the warning circles' glow.
constexpr float GroundHeight = 5.f;
} // namespace

AJamFxView::AJamFxView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AJamFxView::Init()
{
	Sparks = LoadObject<UNiagaraSystem>(nullptr, SparksPath);
	Dust = LoadObject<UNiagaraSystem>(nullptr, DustPath);
	Cash = LoadObject<UNiagaraSystem>(nullptr, CashPath);
	Embers = LoadObject<UNiagaraSystem>(nullptr, EmbersPath);
	Charge = LoadObject<UNiagaraSystem>(nullptr, ChargePath);
	if (Charge)
	{
		ChargeAura = UNiagaraFunctionLibrary::SpawnSystemAttached(Charge, RootComponent, NAME_None, FVector::ZeroVector,
			FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, false);
		ChargeAura->SetVariableLinearColor(TEXT("User.Color"), ChargeMagenta);
	}
	if (Embers)
	{
		Ambient = UNiagaraFunctionLibrary::SpawnSystemAttached(Embers, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, false, false);
	}
}

void AJamFxView::SyncCharge(const FJamFight& Fight, bool bArt, const FVector& HeroAt)
{
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const FJamAura Aura =
		JamFxRules::Aura(Hero.Ultimate.Meter, Fight.Ledger.Has(EJamSide::Hero, EJamMechanic::Ultimate), Hero.Ultimate.bPending);
	const bool bOn = bArt && Aura.bOn;
	if (ChargeAura && bOn != bChargeOn)
	{
		bChargeOn = bOn;
		bOn ? ChargeAura->Activate(true) : ChargeAura->Deactivate();
	}
	if (ChargeAura && bOn)
	{
		ChargeAura->SetWorldLocation(HeroAt);
		ChargeAura->SetVariableFloat(TEXT("User.Size"), Aura.Size);
		ChargeAura->SetVariableFloat(TEXT("User.Rate"), Aura.Rate);
	}
	// The moment the meter fills: one burst off the chest, so "ready" is an event the eye catches, not a slow change.
	if (bOn && Aura.bReady && !bWasReady && Sparks)
	{
		if (UNiagaraComponent* Fx = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Sparks, HeroAt + FVector(0.0, 0.0, ChestHeight)))
		{
			Fx->SetVariableLinearColor(TEXT("User.Color"), ChargeMagenta);
			Fx->SetVariableFloat(TEXT("User.Scale"), ReadyBurst);
		}
	}
	bWasReady = Aura.bReady;
}

void AJamFxView::Sync(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, bool bArt, const FVector& HeroAt)
{
	SyncCharge(Fight, bArt, HeroAt);
	if (Ambient && bArt != bAmbientOn)
	{
		bAmbientOn = bArt;
		// Deactivate lets the embers in the air die out instead of vanishing with the cut.
		bArt ? Ambient->Activate(true) : Ambient->Deactivate();
	}
	TArray<FJamFxCue> Cues;
	JamFxRules::CuesFor(Fight, Events, bArt, Cues);
	for (const FJamFxCue& Cue : Cues)
	{
		UNiagaraSystem* System = Cue.Kind == EJamFx::Sparks ? Sparks.Get() : Cue.Kind == EJamFx::Dust ? Dust.Get() : Cash.Get();
		if (!System)
		{
			continue;
		}
		const float Height = Cue.Kind == EJamFx::Sparks ? Cue.Height : GroundHeight;
		UNiagaraComponent* Fx = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, System, FVector(Cue.Pos.X, Cue.Pos.Y, Height));
		if (!Fx)
		{
			continue;
		}
		Fx->SetVariableLinearColor(TEXT("User.Color"), Cue.Color);
		Fx->SetVariableFloat(Cue.Kind == EJamFx::Dust ? TEXT("User.Radius") : TEXT("User.Scale"), Cue.Size);
	}
}
