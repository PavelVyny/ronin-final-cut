#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/** The clips of a fighter by what they show; the view maps each to a sequence of the hero or of the Publisher. */
enum class EJamClip : uint8
{
	None,
	Idle,
	Move,
	/** The katana strikes: the first and the finisher, and the second of the series. */
	StrikeA,
	StrikeB,
	Parry,
	Hit,
	Death,
	/** The Publisher's own: the ground slam, and the heavy two-hand blow his throws and his Ultimate use. */
	Slam,
	Heavy,
	/** His show-off before the first fight (T-030), played whole at its own speed. */
	Entrance,
	/** The hero's throw of a shuriken or a ball, with the left hand (T-065). */
	Throw
};

/** An action the fight starts on a fighter's body. */
struct FJamCue
{
	EJamClip Clip = EJamClip::None;
	/** Seconds from now to the moment the clip's impact frame has to show: the blade lands, the guard is up. */
	float Lead = 0.f;
	/** The legs play it too; otherwise they keep walking while the fighter moves. */
	bool bFullBody = false;
};

/** The directional runs of the legs (T-078), by where the body moves against where it faces. */
enum class EJamStride : uint8
{
	/** The run straight ahead; Back is the same run played backwards. */
	Forward,
	ForwardLeft,
	ForwardRight,
	BackLeft,
	BackRight,
	Back,
	Num
};

/** How the legs run on this frame: which directional runs show, and how far the hips turn off the facing. */
struct FJamStride
{
	/** How much of each run the legs show, by EJamStride; they sum to 1. */
	float Weights[static_cast<int32>(EJamStride::Num)] = { 1.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	/**
	 * Degrees the hips and the legs turn from the facing toward the move (positive: to the right), so the feet step
	 * where the body goes; the spine turns back as much, so the chest and the head keep facing the aim.
	 */
	float Twist = 0.f;
};

/** Where to start a clip and how fast to play it. */
struct FJamPlayback
{
	float From = 0.f;
	float Rate = 1.f;
};

/**
 * What the 3D fighters play (T-061): which clip a step of the fight starts, how it is timed to the simulation, how it
 * blends. The simulation decides when a blow lands; the clip is fitted to it, never the other way round.
 * Tests: Jam.Pose.*
 */
namespace JamPoseRules
{
/**
 * The action Side's body starts after the step that made Events, or a cue with Clip None. When several things
 * happened, the strongest wins: the stun, then the slam, the Ultimate, a strike, the parry, a throw. A hit starts
 * nothing here: it shakes the body over whatever plays (Shakes, T-064).
 */
JAMGAME_API FJamCue CueFor(const FJamFight& Fight, EJamSide Side, TConstArrayView<FJamEvent> Events);

/**
 * Whether the step's events shake Side (T-064): a hit leans the upper body back over whatever plays, never cutting
 * the legs or an action. Any hit on the hero; on the Publisher only hits of StaggerFrom and more.
 */
JAMGAME_API bool Shakes(const FJamFight& Fight, EJamSide Side, TConstArrayView<FJamEvent> Events, float StaggerFrom);

/** How far into the hit lean the body is Age seconds after the hit: a snap in over Rise, an ease back until Life. */
JAMGAME_API float LeanWeight(float Age, float Rise, float Life);

/** How many afterimages of the hero are left Age seconds after a hit: one at once, one more every Gap, Count at most. */
JAMGAME_API int32 GhostsBy(float Age, int32 Count, float Gap);

/** How bright an afterimage is Age seconds after it was left: it fades out over Life. */
JAMGAME_API float GhostAlpha(float Age, float Life);

/**
 * Which way the run plays (T-066): backwards when the body moves mostly against its facing (the hero faces the
 * cursor and may walk away from it), forwards otherwise, sideways included.
 */
JAMGAME_API float StrideSign(const FVector2D& Move, const FVector2D& Facing);

/** Degrees from Facing to Move, in (-180, 180]: positive when the body moves to its right. 0 when it stands. */
JAMGAME_API float MoveAngle(const FVector2D& Move, const FVector2D& Facing);

/**
 * How the legs run at Angle (MoveAngle) off the facing (T-078). With bDiagonals the four diagonal runs carry the
 * strafes and the backpedal between them; without them (the Publisher) only the run forwards and backwards. Each run
 * covers a sector and the hips twist the rest of the way, at most MaxTwist degrees; at a sector's edge the two runs
 * cross over BlendDegrees, so neither the weights nor the twist jump.
 */
JAMGAME_API FJamStride StrideFor(float Angle, bool bDiagonals, float MaxTwist, float BlendDegrees);

/** How fast to play a run whose own speed is ClipSpeed so the feet keep up with GroundSpeed: at most MaxRate. */
JAMGAME_API float StrideRate(float GroundSpeed, float ClipSpeed, float MaxRate);

/**
 * Degrees the upper body leans into a turn of YawRate degrees per second (positive: turning right, leaning right):
 * in proportion up to FullAtRate, MaxLean at most.
 */
JAMGAME_API float TurnLean(float YawRate, float FullAtRate, float MaxLean);

/** T-084: how much of the hip twist and the spine's turn back (T-078) shows under an action: an upper-body action
 * owns the chest, so the warp fades out as it fades in; a full-body action takes it all. */
JAMGAME_API float WarpWeight(float ActionWeight, bool bFullBody);

/** T-090: degrees the Publisher's model turns while his show-off shows at ClipWeight (0..1): Turn whole, eased out. */
JAMGAME_API float ShowOffTurn(float Turn, float ClipWeight);

/** Side lost the fight and lies down until it starts again. */
JAMGAME_API bool IsDown(const FJamFight& Fight, EJamSide Side);

/** Whether Next cuts off the action Playing now: a hit never interrupts a blow or a guard, the rest always does. */
JAMGAME_API bool Replaces(EJamClip Next, EJamClip Playing);

/**
 * Where and how fast to play a clip whose impact frame is Impact seconds in, so the impact shows Lead seconds from
 * now. The rate stays between MinRate and MaxRate: a wind-up too short to show whole starts later in the clip.
 */
JAMGAME_API FJamPlayback Fit(float Impact, float Lead, float MinRate, float MaxRate);

/** How much of an action shows Age seconds after it started with Remaining seconds to go: it fades in and out over Blend. */
JAMGAME_API float Weight(float Age, float Remaining, float Blend);

/** T-094: where a looping clip of Length seconds is Time seconds after it started, so the idle never stops on its last frame. */
JAMGAME_API float LoopTime(float Time, float Length);
} // namespace JamPoseRules
