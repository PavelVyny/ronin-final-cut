#include "View/JamFxRules.h"

#include "Sim/JamHazardRules.h"

namespace
{
// Heights of the sparks: about the chest of each body; a blast or a slash at the hero's chest height.
constexpr float HeroChest = 120.f;
constexpr float BossChest = 160.f;
// A plain hit's damage: sparks of size 1; bigger hits throw bigger bursts, within these bounds.
constexpr float PlainHit = 14.f;
constexpr float SmallestSparks = 0.6f;
constexpr float BiggestSparks = 1.8f;
constexpr float BlastSparks = 1.5f;
constexpr float DeflectSparks = 1.f;
constexpr float UltimateSparks = 2.2f;
constexpr float SlamSparks = 1.6f;
constexpr float SlamSparksHeight = 20.f;
// The charge aura (T-072, T-073): its size and its particles a second when the meter is full, and the wind-up's flare.
constexpr float AuraReadySize = 1.f;
constexpr float AuraReadyRate = 90.f;
constexpr float AuraWindUpSize = 1.4f;
constexpr float AuraWindUpRate = 180.f;

FJamFxCue Cue(EJamFx Kind, const FVector2D& Pos, float Height, const FLinearColor& Color, float Size)
{
	FJamFxCue Out;
	Out.Kind = Kind;
	Out.Pos = Pos;
	Out.Height = Height;
	Out.Color = Color;
	Out.Size = Size;
	return Out;
}
} // namespace

void JamFxRules::CuesFor(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, bool bArt, TArray<FJamFxCue>& Out)
{
	if (!bArt)
	{
		return;
	}
	for (const FJamEvent& Event : Events)
	{
		switch (Event.Type)
		{
			case EJamEvent::Hit:
			{
				// Sparks fly off the body that was hit, in the colour of the one who hit it.
				const bool bOnBoss = Event.Side == EJamSide::Boss;
				const float Size = FMath::Clamp(Event.Amount / PlainHit, SmallestSparks, BiggestSparks);
				Out.Add(Cue(
					EJamFx::Sparks, Fight.Fighters[Event.Side].Pos, bOnBoss ? BossChest : HeroChest, bOnBoss ? HeroNeon : BossNeon, Size));
				break;
			}
			case EJamEvent::Explosion:
				Out.Add(Cue(EJamFx::Sparks, Event.Pos, HeroChest, BlastYellow, BlastSparks));
				Out.Add(Cue(EJamFx::Dust, Event.Pos, 0.f, DustGrey, Event.Amount));
				break;
			case EJamEvent::Slam:
				// The session-5s playtest: the rune telegraph ends in a hot burst off the floor, then the dust's ring.
				Out.Add(Cue(EJamFx::Sparks, Event.Pos, SlamSparksHeight, SlamOrange, SlamSparks));
				Out.Add(Cue(EJamFx::Dust, Event.Pos, 0.f, DustGrey, Event.Amount));
				break;
			case EJamEvent::Deflected:
				Out.Add(Cue(EJamFx::Sparks, Event.Pos, HeroChest, FLinearColor::White, DeflectSparks));
				break;
			case EJamEvent::HealTaken:
				Out.Add(Cue(EJamFx::Cash, Event.Pos, 0.f, CashGreen, 1.f));
				break;
			case EJamEvent::UltimateSlash:
				Out.Add(Cue(EJamFx::Sparks, Event.Pos, HeroChest, UltimateMagenta, UltimateSparks));
				break;
			default:
				break;
		}
	}
}

EJamChargeLook JamFxRules::ChargeLook(const FJamLayers& Shown)
{
	// The aura belongs to the full art; the glow to the plain effects of the middle tier; greybox gets the bars back.
	if (Shown.bArt)
	{
		return EJamChargeLook::Aura;
	}
	return Shown.bEffects ? EJamChargeLook::Glow : EJamChargeLook::Bars;
}

bool JamFxRules::DrawsPlain(EJamEvent Type, bool bArt)
{
	if (!bArt)
	{
		return true;
	}
	// The events CuesFor gives a Niagara burst: the flat disc under it would only say "greybox".
	switch (Type)
	{
		case EJamEvent::Hit:
		case EJamEvent::Slam:
		case EJamEvent::Explosion:
		case EJamEvent::Deflected:
		case EJamEvent::HealTaken:
			return false;
		default:
			return true;
	}
}

FJamAura JamFxRules::Aura(float Meter, bool bOwns, bool bWindingUp)
{
	// T-073: the aura is the "ready" cue alone; the meter bar under his health shows the charge filling.
	FJamAura Out;
	if (!bOwns || (!bWindingUp && Meter < 1.f))
	{
		return Out;
	}
	Out.bOn = true;
	Out.bReady = true;
	Out.Size = bWindingUp ? AuraWindUpSize : AuraReadySize;
	Out.Rate = bWindingUp ? AuraWindUpRate : AuraReadyRate;
	return Out;
}

EJamTelegraphLook JamFxRules::TelegraphLook(const FJamLayers& Shown)
{
	return Shown.bArt ? EJamTelegraphLook::Rune : EJamTelegraphLook::Plain;
}

FJamTelegraph JamFxRules::TelegraphOf(const FJamHazard& Circle, float Tick)
{
	// The very spot and radius the hit uses, and the fill the plain disc always had: what glows is what hurts.
	FJamTelegraph Telegraph;
	Telegraph.Pos = Circle.Pos;
	Telegraph.Radius = Circle.Radius;
	Telegraph.Fill = JamHazardRules::Progress(Circle, Tick);
	return Telegraph;
}
