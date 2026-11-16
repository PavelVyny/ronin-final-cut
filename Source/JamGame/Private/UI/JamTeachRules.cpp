#include "UI/JamTeachRules.h"

#include "Sim/JamParryRules.h"

namespace
{
/** Seconds until a projectile flying straight on touches a still body; a negative number when it never does. */
double SecondsToTouch(const FJamProjectile& Shot, const FJamFighter& Body)
{
	const FVector2D Gap = Shot.Pos - Body.Pos;
	const double Reach = Shot.Radius + Body.Radius;
	const double C = Gap.SizeSquared() - Reach * Reach;
	if (C <= 0.0)
	{
		return 0.0;
	}
	const double A = Shot.Vel.SizeSquared();
	const double B = 2.0 * FVector2D::DotProduct(Gap, Shot.Vel);
	const double Discriminant = B * B - 4.0 * A * C;
	if (A <= UE_SMALL_NUMBER || Discriminant < 0.0)
	{
		return -1.0;
	}
	// The first of the two moments the distance equals the reach: the shot entering the body.
	return (-B - FMath::Sqrt(Discriminant)) / (2.0 * A);
}
} // namespace

void JamTeachRules::Learn(FJamLessons& Lessons, TConstArrayView<FJamEvent> Events)
{
	for (const FJamEvent& Event : Events)
	{
		if (Event.Side != EJamSide::Hero)
		{
			continue;
		}
		Lessons.Deflects += Event.Type == EJamEvent::Deflected ? 1 : 0;
		Lessons.Ultimates += Event.Type == EJamEvent::UltimateSlash ? 1 : 0;
	}
}

int32 JamTeachRules::ParryPromptFor(const FJamFight& Fight, const FJamLessons& Lessons, const FJamTeachSettings& Settings)
{
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const bool bTeaches = Lessons.Deflects < Settings.ParryLessons && Fight.Phase == EJamPhase::Fight &&
		Fight.Ledger.Has(EJamSide::Hero, EJamMechanic::Parry) && JamParryRules::CanStart(Hero.Parry, Fight.Tick);
	if (!bTeaches)
	{
		return INDEX_NONE;
	}
	int32 Marked = INDEX_NONE;
	double Soonest = Settings.ParryPromptLead;
	for (const FJamProjectile& Shot : Fight.Projectiles)
	{
		if (Shot.Owner == EJamSide::Hero)
		{
			continue;
		}
		const double Seconds = SecondsToTouch(Shot, Hero);
		if (Seconds >= 0.0 && Seconds <= Soonest)
		{
			Soonest = Seconds;
			Marked = Shot.Id;
		}
	}
	return Marked;
}

EJamUltimateCall JamTeachRules::UltimateCallFor(const FJamFight& Fight, const FJamLessons& Lessons)
{
	const FJamUltimateState& Ultimate = Fight.Fighters[EJamSide::Hero].Ultimate;
	const bool bPhase = Fight.Phase == EJamPhase::Fight || Fight.Phase == EJamPhase::Stunned;
	if (!bPhase || !Fight.Ledger.Has(EJamSide::Hero, EJamMechanic::Ultimate) || Ultimate.bPending || Ultimate.Meter < 1.f)
	{
		return EJamUltimateCall::None;
	}
	return Lessons.Ultimates > 0 ? EJamUltimateCall::Banner : EJamUltimateCall::BannerAndPrompt;
}
