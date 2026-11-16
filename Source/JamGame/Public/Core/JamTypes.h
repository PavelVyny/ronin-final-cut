#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

/**
 * Plain data of the fight simulation.
 *
 * The fight is flat: simulation X and Y are world X and Y, height exists only in the view. Nothing here knows about
 * actors, cameras or input devices: the hero and the boss are driven by the same FJamIntent, whoever writes it.
 */

enum class EJamSide : uint8
{
	Hero,
	Boss
};

/** The fixed step of the simulation. Every duration inside the simulation is a whole number of steps. */
namespace JamSim
{
inline constexpr int32 StepsPerSecond = 60;
inline constexpr float StepDt = 1.f / StepsPerSecond;

/** Seconds from the balance data as steps, at least one. */
inline int32 Ticks(float Seconds)
{
	return FMath::Max(1, FMath::RoundToInt32(Seconds * StepsPerSecond));
}

inline EJamSide Other(EJamSide Side)
{
	return Side == EJamSide::Hero ? EJamSide::Boss : EJamSide::Hero;
}
} // namespace JamSim

/** One value for the hero and one for the boss. */
template <typename T>
struct TJamPerSide
{
	T Items[2];

	T& operator[](EJamSide Side) { return Items[static_cast<int32>(Side)]; }

	const T& operator[](EJamSide Side) const { return Items[static_cast<int32>(Side)]; }
};

/** Where a fighter aims. A zero Dir means "no aim this step": the fighter keeps its facing. */
struct FJamAim
{
	FVector2D Dir = FVector2D::ZeroVector;
	FVector2D Point = FVector2D::ZeroVector;
};

/** What a fighter wants to do this step. Buttons are "held" states, the simulation decides when they fire. */
struct FJamIntent
{
	/** World-space move direction, length up to 1. */
	FVector2D Move = FVector2D::ZeroVector;
	FJamAim Aim;
	bool bPrimary = false;
	bool bSecondary = false;
	bool bDash = false;
	bool bParry = false;
	bool bUltimate = false;
	/** The Publisher's own attacks. Nobody else has them, the hero's are ignored. */
	bool bVolley = false;
	bool bSlam = false;
	/** Start one of his signature chains (T-056). */
	bool bCombo = false;
	/** Index of the picked option of the steal choice, -1 when nothing is picked. */
	int32 Choice = -1;
};

using FJamIntents = TJamPerSide<FJamIntent>;

/** The mechanics that can change hands. */
enum class EJamMechanic : uint8
{
	Katana,
	Shuriken,
	Dash,
	Parry,
	Ultimate
};

namespace JamSim
{
inline constexpr int32 MechanicCount = 5;
} // namespace JamSim

/** Who owns each mechanic. A steal is a change of owner here and nothing else. */
struct FJamLedger
{
	EJamSide Owner[JamSim::MechanicCount] = { EJamSide::Hero, EJamSide::Hero, EJamSide::Hero, EJamSide::Hero, EJamSide::Hero };

	bool Has(EJamSide Side, EJamMechanic Mechanic) const { return Owner[static_cast<int32>(Mechanic)] == Side; }
};

/**
 * State of each mechanic on a fighter. Both fighters carry all of them, so a mechanic that changes its owner needs no
 * copying: the new owner simply starts using his own state.
 */
struct FJamKatanaState
{
	/** The strike of the series swung last, 1 to 3; 0 before the first one. */
	int32 Strike = 0;
	/** Step on which that strike lands. */
	int32 HitTick = 0;
	/** First step on which the next strike can start. */
	int32 BusyUntilTick = 0;
	/** Last step on which a press still continues the series. */
	int32 ChainUntilTick = 0;
	bool bHitPending = false;
};

struct FJamShurikenState
{
	int32 ReadyTick = 0;
	/** Throws made so far: decides which one explodes. */
	int32 Throws = 0;
};

struct FJamDashState
{
	FVector2D Dir = FVector2D::ZeroVector;
	/** Distance covered per step while dashing. */
	float StepLength = 0.f;
	/** First step after the dash. */
	int32 EndTick = 0;
	/** First step on which damage is taken again. */
	int32 InvulnerableUntilTick = 0;
	int32 ReadyTick = 0;
};

struct FJamParryState
{
	/** First step on which the window is open: later than the start when the shield is raised first (T-038). */
	int32 ActiveFromTick = 0;
	/** First step on which the window is closed again. */
	int32 ActiveUntilTick = 0;
	int32 ReadyTick = 0;
	/** An early strike waits for its shot until this step (T-057). */
	int32 HeldUntilTick = 0;
};

struct FJamUltimateState
{
	/** 0 to 1: full is ready. */
	float Meter = 0.f;
	/** Step on which the slash lands. */
	int32 SlashTick = 0;
	FVector2D Dir = FVector2D(1.0, 0.0);
	bool bPending = false;
};

struct FJamFighter
{
	FVector2D Pos = FVector2D::ZeroVector;
	/** Position before the last step, for the view to interpolate from. */
	FVector2D PrevPos = FVector2D::ZeroVector;
	FVector2D Facing = FVector2D(1.0, 0.0);
	float Radius = 0.f;
	float Speed = 0.f;
	float Health = 0.f;
	float MaxHealth = 0.f;
	/** First step on which the ball can be shot again. */
	int32 BallReadyTick = 0;
	FJamKatanaState Katana;
	FJamShurikenState Shuriken;
	FJamDashState Dash;
	FJamParryState Parry;
	FJamUltimateState Ultimate;
};

enum class EJamProjectile : uint8
{
	Ball,
	Shuriken,
	Memo
};

struct FJamProjectile
{
	int32 Id = 0;
	EJamProjectile Kind = EJamProjectile::Ball;
	EJamSide Owner = EJamSide::Hero;
	FVector2D Pos = FVector2D::ZeroVector;
	FVector2D PrevPos = FVector2D::ZeroVector;
	FVector2D Vel = FVector2D::ZeroVector;
	float Radius = 0.f;
	float Damage = 0.f;
	/** Step on which it disappears on its own. */
	int32 DieTick = 0;
	bool bExplosive = false;
	/** How far past the target's body a parry still catches it, in cm (T-060: the Publisher's shurikens). */
	float ParryMargin = 0.f;
};

/** A place that will hurt: shown as a warning first, hits once at HitTick. */
struct FJamHazard
{
	int32 Id = 0;
	EJamSide Owner = EJamSide::Boss;
	FVector2D Pos = FVector2D::ZeroVector;
	float Radius = 0.f;
	float Damage = 0.f;
	/** Step on which the warning appeared. */
	int32 StartTick = 0;
	int32 HitTick = 0;
};

/** A heal the Publisher dropped (T-049): a cash stack tossed from From that lands at Pos and lies until DieTick. */
struct FJamPickup
{
	int32 Id = 0;
	FVector2D From = FVector2D::ZeroVector;
	FVector2D Pos = FVector2D::ZeroVector;
	/** Step on which it was knocked out of his pocket. */
	int32 StartTick = 0;
	/** From this step on it lies on the floor and can be taken. */
	int32 LandTick = 0;
	int32 DieTick = 0;
};

/** The Publisher's signature chains (T-056): his own from the first cut, one more per mechanic he stole. */
enum class EJamCombo : uint8
{
	None,
	/** His own: the slam pattern, then a fan into the safe pocket. */
	Pocket,
	/** Dash: in, and the slam goes down on arrival. */
	DashSlam,
	/** Shuriken: a fan, then a burst of shuriken led at the hero. */
	FanBurst,
	/** Parry: the shield stance, then a fan the moment it drops. */
	ShieldFan,
	/** Ultimate: a fan, then the line along it. */
	FanLine,
	/** Katana: a fan, then a rush with the blade. */
	FanRush
};

/** One link of a chain and the step it goes on. */
enum class EJamLink : uint8
{
	Slam,
	Fan,
	PocketFan,
	Dash,
	Burst,
	Shield,
	Line,
	/** From this step until the end he walks at the hero and swings when in reach. */
	Rush,
	End
};

struct FJamComboLink
{
	EJamLink Kind = EJamLink::End;
	int32 Tick = 0;
};

/** Timers of the Publisher's own attacks. */
struct FJamBossState
{
	/** He stands until this step: an attack in progress or the pause after it. */
	int32 BusyUntilTick = 0;
	int32 VolleyReadyTick = 0;
	int32 SlamReadyTick = 0;
	/** The chain running now (T-056), its links in order, and the one before it. */
	EJamCombo Combo = EJamCombo::None;
	TArray<FJamComboLink> Links;
	EJamCombo LastCombo = EJamCombo::None;
	/** First step on which a new chain may start. */
	int32 ComboReadyTick = 0;
};

/** The Publisher's cuts: the ones already made and the one that waits for the player's answer. */
struct FJamStealState
{
	/** Steals already made. */
	int32 Done = 0;
	/** The boss cannot drop below this health until the next steal is settled; 0 when no steal is left. */
	float Floor = 0.f;
	/** What he grabbed this time, left to right on the card. The player gives one away; with bTakesAll all of it goes. */
	TArray<EJamMechanic> Offer;
	bool bTakesAll = false;
	/** Step on which the card opened. */
	int32 StartTick = 0;
	/** First step on which an answer counts. */
	int32 OpenTick = 0;
	/** Step on which the Publisher stops waiting and takes the left option. */
	int32 EndTick = 0;
};

enum class EJamPhase : uint8
{
	Fight,
	/** The choice card is up: the fight stands still until the steal is settled. */
	Steal,
	/** The boss is at zero and stunned, everything is back with the hero: one more hit ends it. */
	Stunned,
	HeroDead,
	Won
};

enum class EJamEvent : uint8
{
	ProjectileSpawned,
	ProjectileGone,
	/** Somebody lost health. Side is who, Amount is how much. */
	Hit,
	/** A katana strike landed or cut the air. Side is who swung, Amount is the strike of the series. */
	Swing,
	/** An explosive shuriken went off at Pos. Amount is the radius. */
	Explosion,
	/** A slam landed at Pos. Amount is the radius. */
	Slam,
	/** Side started a dash at Pos. */
	Dash,
	/** Side opened a parry window at Pos. */
	ParryStarted,
	/** Side turned a projectile round at Pos: it is Side's now. */
	Deflected,
	/** Side started the wind-up of the Ultimate at Pos. */
	UltimateStarted,
	/** Side's Ultimate slashed from Pos; Amount is the yaw of the line in degrees. */
	UltimateSlash,
	/** The Publisher grabbed mechanics and the choice card opened. Amount is the number of options. */
	StealOpened,
	/** A mechanic changed hands. Side is the new owner, Amount is the mechanic. */
	StealResolved,
	/** The boss is stunned and everything went back to the hero. */
	Stunned,
	/** Side's Ultimate meter filled up on this step. */
	UltimateReady,
	/** The Publisher dropped a heal: ProjectileId is the pickup, Pos where it lands. */
	HealDropped,
	/** The hero took a heal at Pos; Amount is the health it gave back. */
	HealTaken
};

/** Something the view, the sound and the tests may want to know about a step. */
struct FJamEvent
{
	EJamEvent Type = EJamEvent::Hit;
	/** Whose event it is: who shot, swung or dashed, or who was hit. */
	EJamSide Side = EJamSide::Hero;
	int32 ProjectileId = 0;
	FVector2D Pos = FVector2D::ZeroVector;
	float Amount = 0.f;
};

/** The whole state of a fight. Copying it is a snapshot. */
struct FJamFight
{
	int32 Tick = 0;
	TJamPerSide<FJamFighter> Fighters;
	TArray<FJamProjectile> Projectiles;
	TArray<FJamHazard> Hazards;
	TArray<FJamPickup> Pickups;
	/** From this step on, the next hit on the Publisher knocks a heal out of his pocket. */
	int32 NextHealTick = 0;
	FJamLedger Ledger;
	FJamBossState Boss;
	FJamStealState Steal;
	/** The balance's HeroCutDamageScale, copied at the start: every hit of the hero's after the first cut is scaled by it. */
	float HeroCutDamageScale = 1.f;
	/** T-101: the balance's HeroDamageScale and BossDamageScale, the difficulty's, copied at the start. */
	float HeroDamageScale = 1.f;
	float BossDamageScale = 1.f;
	EJamPhase Phase = EJamPhase::Fight;
	int32 NextId = 1;
	/** Events of the last step only. */
	TArray<FJamEvent> Events;
	/** The only source of randomness of the simulation. */
	FRandomStream Stream;
};
