#pragma once

#include "CoreMinimal.h"

#include "JamBalance.generated.h"

/**
 * Every tunable number of the fight. The values live in the [JamBalance] section of Config/DefaultGame.ini, one line per
 * field with the field's name; the defaults here are zeros on purpose, so a missing line is caught by
 * Jam.Balance.IniIsCompleteAndValid instead of hiding behind a second copy of the number.
 *
 * Distances are in cm, times in seconds, speeds in cm per second.
 */
USTRUCT()
struct JAMGAME_API FJamBalanceData
{
	GENERATED_BODY()

	/** Radius of the round arena. */
	UPROPERTY()
	float ArenaRadius = 0.f;

	UPROPERTY()
	float HeroRadius = 0.f;

	UPROPERTY()
	float HeroSpeed = 0.f;

	UPROPERTY()
	float HeroHealth = 0.f;

	UPROPERTY()
	float BossRadius = 0.f;

	UPROPERTY()
	float BossSpeed = 0.f;

	UPROPERTY()
	float BossHealth = 0.f;

	/** The weak ball shot: what a fighter has left when the mechanic of a button is gone. */
	UPROPERTY()
	float BallDamage = 0.f;

	UPROPERTY()
	float BallSpeed = 0.f;

	UPROPERTY()
	float BallCooldown = 0.f;

	UPROPERTY()
	float BallRadius = 0.f;

	/** Seconds a ball flies before it disappears. */
	UPROPERTY()
	float BallLife = 0.f;

	/** Katana: a series of three strikes in an arc in front of the fighter. Reach is measured from the fighter's centre. */
	UPROPERTY()
	float KatanaRange = 0.f;

	/** Half of the arc's angle, degrees. */
	UPROPERTY()
	float KatanaHalfArc = 0.f;

	/** Time from the press to the moment the strike lands. */
	UPROPERTY()
	float KatanaWindUp = 0.f;

	/** Time after the strike lands before the next one can start. */
	UPROPERTY()
	float KatanaRecovery = 0.f;

	/** How long after the recovery a press still continues the series. */
	UPROPERTY()
	float KatanaChainWindow = 0.f;

	/** Step forward on each strike. */
	UPROPERTY()
	float KatanaStep = 0.f;

	/** Damage of the first and the second strike. */
	UPROPERTY()
	float KatanaDamage = 0.f;

	/** Damage of the third strike. */
	UPROPERTY()
	float KatanaFinisherDamage = 0.f;

	/** Shuriken: a fast projectile, every few throws one explodes. */
	UPROPERTY()
	float ShurikenDamage = 0.f;

	UPROPERTY()
	float ShurikenSpeed = 0.f;

	UPROPERTY()
	float ShurikenCooldown = 0.f;

	UPROPERTY()
	float ShurikenRadius = 0.f;

	UPROPERTY()
	float ShurikenLife = 0.f;

	/** Every which throw explodes, a whole number. */
	UPROPERTY()
	float ShurikenExplosiveEvery = 0.f;

	UPROPERTY()
	float ShurikenExplosionRadius = 0.f;

	UPROPERTY()
	float ShurikenExplosionDamage = 0.f;
	/** Dash: a short burst in the move direction, no damage is taken during the invulnerable time. */
	UPROPERTY()
	float DashDistance = 0.f;

	UPROPERTY()
	float DashDuration = 0.f;

	UPROPERTY()
	float DashInvulnerable = 0.f;

	UPROPERTY()
	float DashCooldown = 0.f;

	/** Parry: a window in which a projectile that reaches the fighter flies back at its owner. */
	UPROPERTY()
	float ParryWindow = 0.f;

	UPROPERTY()
	float ParryCooldown = 0.f;

	/** A strike this early before a shot comes within one window is held and parries it (T-057), not a swing. */
	UPROPERTY()
	float ParryBuffer = 0.f;

	/** Speed of a returned projectile, cm/s. */
	UPROPERTY()
	float ParryReturnSpeed = 0.f;

	/** A returned projectile hurts this many times as much as it came. */
	UPROPERTY()
	float ParryDamageScale = 0.f;

	/** Seconds a returned projectile flies. */
	UPROPERTY()
	float ParryReturnLife = 0.f;

	/** Ultimate: the meter grows by this much per point of damage dealt; 1 is full. */
	UPROPERTY()
	float UltimateGainPerDamage = 0.f;

	/** Time between the press and the slash: the moment of slow motion. */
	UPROPERTY()
	float UltimateWindUp = 0.f;

	/** Degrees per second the hero's pressed line turns toward the aim during the wind-up (T-058). */
	UPROPERTY()
	float UltimateTurnRate = 0.f;

	/** The line of the slash: its length and half its width, cm. */
	UPROPERTY()
	float UltimateRange = 0.f;

	UPROPERTY()
	float UltimateHalfWidth = 0.f;

	UPROPERTY()
	float UltimateDamage = 0.f;

	/** The Publisher's own attacks. Memo volley: a fan of slow projectiles. */
	UPROPERTY()
	float MemoCount = 0.f;

	/** Angle between the outer memos of a volley, degrees. */
	UPROPERTY()
	float MemoSpread = 0.f;

	UPROPERTY()
	float MemoSpeed = 0.f;

	UPROPERTY()
	float MemoDamage = 0.f;

	UPROPERTY()
	float MemoRadius = 0.f;

	UPROPERTY()
	float MemoLife = 0.f;

	UPROPERTY()
	float VolleyCooldown = 0.f;

	/** The boss slams when the hero is closer than this. */
	UPROPERTY()
	float SlamRange = 0.f;

	UPROPERTY()
	float SlamRadius = 0.f;

	/** Time between the warning circle and the hit. */
	UPROPERTY()
	float SlamTelegraph = 0.f;

	UPROPERTY()
	float SlamDamage = 0.f;

	UPROPERTY()
	float SlamCooldown = 0.f;

	/** Circles of one slam (T-050) before the first cut, after it, and from the second cut on. */
	UPROPERTY()
	float SlamCirclesUncut = 0.f;

	UPROPERTY()
	float SlamCirclesFirstCut = 0.f;

	UPROPERTY()
	float SlamCirclesSecondCut = 0.f;

	/** Radius of the pattern's circles other than the one on the hero (that one is SlamRadius). */
	UPROPERTY()
	float SlamSideRadius = 0.f;

	/** How far from the hero the circles that cut off his ways out are centred. */
	UPROPERTY()
	float SlamCutterDistance = 0.f;

	/** Degrees between the hero's way back and each of the two side cutters. */
	UPROPERTY()
	float SlamFlankAngle = 0.f;

	/** Five circles (T-052): the least room between the edges of any two, so the hero has pockets to stand in. */
	UPROPERTY()
	float SlamGap = 0.f;

	/** Five circles: degrees between the hero's way back and each of the two wide circles behind him. */
	UPROPERTY()
	float SlamSpreadAngle = 0.f;

	/** Seeded spread of each cutter's direction, in degrees either way. */
	UPROPERTY()
	float SlamJitter = 0.f;

	/** The circles land one after another, this many seconds apart, from him outwards. */
	UPROPERTY()
	float SlamRipple = 0.f;

	/** Seconds a person needs to read the pattern before walking: the safe spot must be reachable after them. */
	UPROPERTY()
	float SlamReaction = 0.f;

	/** The boss stands still this long after each of his attacks: the player's opening. */
	UPROPERTY()
	float BossRecovery = 0.f;

	/** The Publisher takes a cut each time his health drops to one of these shares of the full bar. The last one takes whatever the hero still has. */
	UPROPERTY()
	float StealThreshold1 = 0.f;

	UPROPERTY()
	float StealThreshold2 = 0.f;

	UPROPERTY()
	float StealThreshold3 = 0.f;

	/** The FINAL NOTE: the last cut takes whatever the hero still has. */
	UPROPERTY()
	float StealThreshold4 = 0.f;

	/** Time the choice card waits for an answer; then the Publisher takes the left option. */
	UPROPERTY()
	float StealChoiceSeconds = 0.f;

	/** The card ignores the keys this long after it opens, so a key pressed for the fight does not choose. */
	UPROPERTY()
	float StealChoiceLock = 0.f;

	/** From the first cut on, every move of the hero's hits this much harder (JamHealthRules::HeroDamageScale). */
	UPROPERTY()
	float HeroCutDamageScale = 0.f;

	/** A stolen mechanic in the Publisher's hands: scales of the hero's numbers. */
	UPROPERTY()
	float BossStolenDamage = 0.f;

	UPROPERTY()
	float BossStolenCooldown = 0.f;

	/**
	 * The base numbers of his stolen shuriken (T-074): the hero's own were retuned, his copy keeps what it had. Scaled
	 * by BossStolenCooldown, BossShurikenCooldownScale and BossStolenDamage like before.
	 */
	UPROPERTY()
	float BossShurikenCooldown = 0.f;

	UPROPERTY()
	float BossShurikenDamage = 0.f;

	/** His stolen shurikens only: the time between two throws, his cooldown and his burst's gap, scaled on top (T-070). */
	UPROPERTY()
	float BossShurikenCooldownScale = 0.f;

	/** Scale of the speed of what he throws: slow enough to step aside. */
	UPROPERTY()
	float BossStolenSpeed = 0.f;

	/** His katana warns this long before it lands. */
	UPROPERTY()
	float BossKatanaWindUp = 0.f;

	/** The stolen Parry in his hands is a stance (T-038): the shield warns this long before it sends shots back... */
	UPROPERTY()
	float BossParryRaise = 0.f;

	/** ...then holds this long; he stands through both. */
	UPROPERTY()
	float BossParryHold = 0.f;

	UPROPERTY()
	float BossParryCooldown = 0.f;

	/** Scale of the damage of what his shield sends back. */
	UPROPERTY()
	float BossParryDamageScale = 0.f;

	/** The stolen Shuriken in his hands never explodes (T-060); the hero's parry catches one of his this far, in cm, past
	 * the hero's body. */
	UPROPERTY()
	float BossShurikenParryMargin = 0.f;

	/** The stolen Ultimate in his hands (T-038): his meter fills by the clock, full after this many seconds of fight. */
	UPROPERTY()
	float BossUltimateInterval = 0.f;

	/** His slash winds up this long, the line locked from the press. */
	UPROPERTY()
	float BossUltimateWindUp = 0.f;

	UPROPERTY()
	float BossUltimateDamage = 0.f;

	/** His signature chains (T-056): seconds between the end of one and the start of the next. */
	UPROPERTY()
	float ComboCooldown = 0.f;

	/** He stands and glows this long before the first link: the warning. */
	UPROPERTY()
	float ComboTell = 0.f;

	/** The least time between two links of a chain, to react. */
	UPROPERTY()
	float ComboLinkGap = 0.f;

	/** Shuriken in the burst of the stolen-shuriken chain, and the seconds between them. */
	UPROPERTY()
	float ComboBurstCount = 0.f;

	UPROPERTY()
	float ComboBurstGap = 0.f;

	/** How long the katana rush of the final chain lasts. */
	UPROPERTY()
	float ComboRushSeconds = 0.f;

	/** A retry brings back the hero's health of the last cut; at this share of full or less, he comes back with
	 * RetryLowHealthRestore of full instead. */
	UPROPERTY()
	float RetryLowHealthShare = 0.f;

	UPROPERTY()
	float RetryLowHealthRestore = 0.f;

	/** The heal the Publisher drops (T-049): the health it gives back, never past full. */
	UPROPERTY()
	float HealAmount = 0.f;

	/** Seconds of fight between drops: once one is due, the next hit on him knocks it out. */
	UPROPERTY()
	float HealDropInterval = 0.f;

	/** Seconds it lies on the floor before it is gone. */
	UPROPERTY()
	float HealLife = 0.f;

	/** Most heals on the floor at once. */
	UPROPERTY()
	float HealMaxOnFloor = 0.f;

	/** How far from him it lands, towards the hero, in cm, and how long it flies there. */
	UPROPERTY()
	float HealToss = 0.f;

	UPROPERTY()
	float HealTossSeconds = 0.f;

	/** How close past the hero's body it is picked up, in cm. */
	UPROPERTY()
	float HealPickupRadius = 0.f;

	/**
	 * T-101: every hit of the hero's and every hit of the Publisher's is scaled by these; 1 in the ini, the difficulty
	 * multiplies them (JamDifficultyRules::Apply).
	 */
	UPROPERTY()
	float HeroDamageScale = 0.f;

	UPROPERTY()
	float BossDamageScale = 0.f;

	/** T-101: EASY's block: the hero's max health, his damage, the cash stack's heal and the Publisher's damage. */
	UPROPERTY()
	float EasyHeroHealthScale = 0.f;

	UPROPERTY()
	float EasyHeroDamageScale = 0.f;

	UPROPERTY()
	float EasyHealScale = 0.f;

	UPROPERTY()
	float EasyBossDamageScale = 0.f;

	/** NORMAL's block: all ones, the game as it was before the levels. */
	UPROPERTY()
	float NormalHeroHealthScale = 0.f;

	UPROPERTY()
	float NormalHeroDamageScale = 0.f;

	UPROPERTY()
	float NormalHealScale = 0.f;

	UPROPERTY()
	float NormalBossDamageScale = 0.f;

	/** HARD's block. */
	UPROPERTY()
	float HardHeroHealthScale = 0.f;

	UPROPERTY()
	float HardHeroDamageScale = 0.f;

	UPROPERTY()
	float HardHealScale = 0.f;

	UPROPERTY()
	float HardBossDamageScale = 0.f;
};
