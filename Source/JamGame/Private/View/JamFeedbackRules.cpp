#include "View/JamFeedbackRules.h"

#include "Sim/JamUltimateRules.h"

float JamFeedbackRules::ShakeKick(const FJamEvent& Event, const FJamFeedbackSettings& Settings)
{
	switch (Event.Type)
	{
		case EJamEvent::Slam:
			return Settings.SlamShake;
		case EJamEvent::UltimateSlash:
			return Settings.UltimateShake;
		case EJamEvent::Explosion:
			return Settings.ExplosionShake;
		case EJamEvent::Hit:
			// The camera follows the hero: it shakes with his pain, never with the Publisher's.
			return Event.Side == EJamSide::Hero ? Settings.HeroHurtShake : 0.f;
		default:
			return 0.f;
	}
}

float JamFeedbackRules::ShakeLeft(float Kick, float Elapsed, const FJamFeedbackSettings& Settings)
{
	if (Settings.ShakeSeconds <= 0.f || Elapsed >= Settings.ShakeSeconds)
	{
		return 0.f;
	}
	// Squared fall-off: a hard knock at once, then a quick settle.
	const float Left = 1.f - FMath::Max(0.f, Elapsed) / Settings.ShakeSeconds;
	return Kick * Left * Left;
}

FVector2D JamFeedbackRules::ShakeOffset(float Amplitude, float Time, const FJamFeedbackSettings& Settings)
{
	// Two sines at frequencies with no common beat read as a jolt, not as a circle; the sum is held to the amplitude.
	const float Phase = 2.f * PI * Settings.ShakeFrequency * Time;
	const FVector2D Raw(FMath::Sin(Phase), FMath::Sin(1.618f * Phase + 1.f));
	return Raw.GetClampedToMaxSize(1.0) * Amplitude;
}

bool JamFeedbackRules::ShowsBlastRing(const FJamProjectile& Projectile)
{
	return Projectile.bExplosive && Projectile.Owner == EJamSide::Boss;
}

float JamFeedbackRules::SparkRadius(float Damage, const FJamFeedbackSettings& Settings)
{
	return FMath::Clamp(Damage * Settings.SparkPerDamage, Settings.SparkMinRadius, Settings.SparkMaxRadius);
}

float JamFeedbackRules::FlashWeight(EJamSide Side, float Damage, const FJamFeedbackSettings& Settings)
{
	if (Side == EJamSide::Hero || Settings.BossFlashFullDamage <= 0.f)
	{
		return 1.f;
	}
	return FMath::Clamp(Damage / Settings.BossFlashFullDamage, Settings.BossFlashMin, 1.f);
}

float JamFeedbackRules::ParryRingScale(float WindowLeft, const FJamFeedbackSettings& Settings)
{
	return FMath::Lerp(Settings.ParryRingEnd, Settings.ParryRingStart, FMath::Clamp(WindowLeft, 0.f, 1.f));
}

EJamEffectLook JamFeedbackRules::LookOf(EJamEvent Type, EJamSide Side, bool bHeroEffects)
{
	if (bHeroEffects)
	{
		return EJamEffectLook::Neon;
	}
	if (Side == EJamSide::Hero && Type == EJamEvent::Swing)
	{
		return EJamEffectLook::Bare;
	}
	// A hit on the Publisher is the hero's blow: its spark goes with the hero's other flourishes.
	const bool bHeroFlourish = (Side == EJamSide::Hero && Type == EJamEvent::Dash) || (Side == EJamSide::Boss && Type == EJamEvent::Hit);
	return bHeroFlourish ? EJamEffectLook::Hidden : EJamEffectLook::Neon;
}

int32 JamFeedbackRules::BillCount(const FJamEvent& Event, bool bFinalBlow, const FJamFeedbackSettings& Settings)
{
	if (Event.Type != EJamEvent::Hit || Event.Side != EJamSide::Boss)
	{
		return 0;
	}
	if (bFinalBlow)
	{
		return Settings.BillsFinalBlow;
	}
	const int32 ByDamage = Settings.DamagePerBill > 0.f ? FMath::CeilToInt32(Event.Amount / Settings.DamagePerBill) : 0;
	return FMath::Clamp(ByDamage, Settings.BillsPerHitMin, Settings.BillsPerHitMax);
}

FJamBillPose JamFeedbackRules::BillPose(
	const FVector& From, const FVector2D& Kick, float FloorZ, float Phase, float Age, const FJamFeedbackSettings& Settings)
{
	// How far a bill tips while it rocks in the air, in degrees.
	constexpr float MaxRoll = 50.f;
	const float Flight = Settings.BillFlutterSeconds > 0.f ? FMath::Clamp(Age / Settings.BillFlutterSeconds, 0.f, 1.f) : 1.f;
	// Thrown fast, slowed by the air; the rocking swings it across the throw, never at the start or on the floor.
	const FVector2D Travel = Kick * (1.f - FMath::Square(1.f - Flight));
	const float Rock = FMath::Sin(2.f * PI * (Phase + Settings.BillFlutterHz * Age));
	const float Envelope = 4.f * Flight * (1.f - Flight);
	const FVector2D Across = Kick.GetSafeNormal().GetRotated(90.0);
	const FVector2D Ground = FVector2D(From) + Travel + Across * (Rock * Settings.BillSway * Envelope);

	FJamBillPose Pose;
	Pose.Pos = FVector(Ground.X, Ground.Y, FMath::Lerp(From.Z, FloorZ, Flight) + Settings.BillPopHeight * Envelope);
	Pose.Roll = Rock * MaxRoll * (1.f - Flight);
	const float Fading = Age - Settings.BillFlutterSeconds - Settings.BillFloorSeconds;
	Pose.Scale = Fading <= 0.f ? 1.f : FMath::Max(0.f, 1.f - Fading / FMath::Max(Settings.BillFadeSeconds, KINDA_SMALL_NUMBER));
	return Pose;
}

FJamUltimateLane JamFeedbackRules::UltimateLane(const FJamFight& Fight, EJamSide Side, float Alpha, const FJamBalanceData& Balance)
{
	FJamUltimateLane Lane;
	const FJamFighter& Self = Fight.Fighters[Side];
	const FJamUltimateState& Ultimate = Self.Ultimate;
	const bool bReady = Fight.Ledger.Has(Side, EJamMechanic::Ultimate) && Ultimate.Meter >= 1.f;
	if (!Ultimate.bPending && !bReady)
	{
		return Lane;
	}
	Lane.bShow = true;
	Lane.bArmed = Ultimate.bPending;
	Lane.Origin = FMath::Lerp(Self.PrevPos, Self.Pos, static_cast<double>(Alpha));
	Lane.Dir = (Ultimate.bPending ? Ultimate.Dir : Self.Facing).GetSafeNormal();
	Lane.Length = Balance.UltimateRange;
	Lane.HalfWidth = Balance.UltimateHalfWidth;
	const FJamFighter& Target = Fight.Fighters[JamSim::Other(Side)];
	Lane.bOnTarget = JamUltimateRules::OnLine(Lane.Origin, Lane.Dir, Target.Pos, Target.Radius, Lane.Length, Lane.HalfWidth);
	if (Ultimate.bPending)
	{
		// Fight.Tick is the step to come: the picture is between the two before it.
		const float Tick = Fight.Tick - 1.f + Alpha;
		const int32 WindUp = FMath::Max(1, JamSim::Ticks(Balance.UltimateWindUp));
		Lane.Fill = FMath::Clamp((Tick - (Ultimate.SlashTick - WindUp)) / WindUp, 0.f, 1.f);
	}
	return Lane;
}

float JamFeedbackRules::BillLife(const FJamFeedbackSettings& Settings)
{
	return Settings.BillFlutterSeconds + Settings.BillFloorSeconds + Settings.BillFadeSeconds;
}

FJamSpill JamFeedbackRules::Spill(const FVector2D& Centre, float BodyRadius, float Degrees, float Reach)
{
	const FVector2D Dir = FVector2D(1.0, 0.0).GetRotated(Degrees);
	FJamSpill Result;
	Result.From = Centre + Dir * BodyRadius;
	Result.Kick = Dir * FMath::Max(0.f, Reach);
	return Result;
}
