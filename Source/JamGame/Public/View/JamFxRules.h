#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "Sim/JamLayerRules.h"

/** The rich effects of the full-art tier (T-069), by what they show; the view maps each to a Niagara system. */
enum class EJamFx : uint8
{
	Sparks,
	Dust,
	Cash
};

/** One effect to start this frame: where, how big, in which colour. */
struct FJamFxCue
{
	EJamFx Kind = EJamFx::Sparks;
	FVector2D Pos = FVector2D::ZeroVector;
	/** Height above the floor, cm. */
	float Height = 0.f;
	FLinearColor Color = FLinearColor::White;
	/** Sparks: how big the burst is, 1 for a plain hit. Dust: the radius it reaches, cm. */
	float Size = 1.f;
};

/**
 * How the hero's full Ultimate and his dash show, by the look the cuts left (T-072, T-073): an aura on the 3D hero and
 * the afterimages alone; the spinning ring under the shape ronin and a thin streak; the plain ring and the dash bar in
 * greybox. The meter bar under his health shows the charge in every tier.
 */
enum class EJamChargeLook : uint8
{
	Aura,
	Glow,
	Bars
};

/** The charge aura on the hero (T-072): how big and how busy it is, and whether it says "ready". */
struct FJamAura
{
	bool bOn = false;
	/** Its size, 1 when the charge is full. */
	float Size = 0.f;
	/** Its particles a second. */
	float Rate = 0.f;
	bool bReady = false;
};

/**
 * Which rich effects a step of the fight starts (T-069). They belong to the full art: once the first cut took it,
 * nothing here starts, and the plain effects of the effects view carry the fight alone.
 * Tests: Jam.Fx.*
 */
/** How the slam's warning circles look (the session-5s playtest): a rune ring in the full art, else the plain disc. */
enum class EJamTelegraphLook : uint8
{
	Rune,
	Plain
};

/** One warning circle as drawn: where, how big (the hit's own radius) and how far it has filled (1 as it hits). */
struct FJamTelegraph
{
	FVector2D Pos = FVector2D::ZeroVector;
	float Radius = 0.f;
	float Fill = 0.f;
};

namespace JamFxRules
{
/** Hero's and Publisher's colours, as the rest of the game paints them. */
inline const FLinearColor HeroNeon(0.3f, 0.95f, 1.0f);
inline const FLinearColor BossNeon(1.0f, 0.25f, 0.2f);
inline const FLinearColor BlastYellow(1.0f, 0.85f, 0.15f);
inline const FLinearColor UltimateMagenta(1.0f, 0.25f, 0.75f);
// Dark enough that the smoke does not white out the bodies under the bloom.
inline const FLinearColor DustGrey(0.4f, 0.4f, 0.45f);
/** The burst of a slam landing at the end of its rune telegraph (the session-5s playtest). */
inline const FLinearColor SlamOrange(1.0f, 0.35f, 0.08f);
inline const FLinearColor CashGreen(0.2f, 1.0f, 0.4f);

/** Appends the effects the step that made Events starts to Out; none when bArt is false. */
JAMGAME_API void CuesFor(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, bool bArt, TArray<FJamFxCue>& Out);

/**
 * Whether the effects view draws its flat shape for an event of this type (T-075). In the full art (bArt) Niagara
 * alone shows the hits, slams, blasts, deflects and heals; the sweep of a blade, the Ultimate's line and the dash carry
 * gameplay and stay in every tier.
 */
JAMGAME_API bool DrawsPlain(EJamEvent Type, bool bArt);

/** The look of the charge and the dash for the layers on show. */
JAMGAME_API EJamChargeLook ChargeLook(const FJamLayers& Shown);

/**
 * The aura for an Ultimate meter of Meter (0 to 1): only once the meter is full (T-073: it says nothing while it fills)
 * and through the wind-up, where it is biggest; none until the hero owns the Ultimate.
 */
JAMGAME_API FJamAura Aura(float Meter, bool bOwns, bool bWindingUp);

/** The look of the warning circles for the layers on show. */
JAMGAME_API EJamTelegraphLook TelegraphLook(const FJamLayers& Shown);

/** Circle as drawn at Tick (a fraction between two steps for the view): the hit's spot and radius, its fill. */
JAMGAME_API FJamTelegraph TelegraphOf(const FJamHazard& Circle, float Tick);
} // namespace JamFxRules
