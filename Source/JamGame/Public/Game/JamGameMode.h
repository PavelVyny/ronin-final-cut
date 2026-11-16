#pragma once

#include "CoreMinimal.h"
#include "Game/JamDemo.h"
#include "GameFramework/GameModeBase.h"
#include "Sim/JamLayerRules.h"
#include "UI/JamMenuRules.h"
#include "UI/JamTeachRules.h"
#include "View/JamFeedbackRules.h"
#include "View/JamNeonRules.h"

#include "JamGameMode.generated.h"

class AJamArenaView;
class AJamCameraRig;
class AJamEffectsView;
class AJamFighterView;
class AJamMusic;
class AJamSounds;
class AJamPlayerController;
class AJamProjectilesView;
class UJamFightSubsystem;

/**
 * Conductor of a frame: reads the player's intent, advances the simulation, then places the views and the camera, always
 * in this order. It also builds the scene, since the map is empty.
 */
UCLASS()
class JAMGAME_API AJamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The player has no pawn: the hero is a body of the simulation. */
	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	/** On the first frame: the camera, the cursor, and whether the run starts on the menu, the intro or the fight. */
	void WirePlayer(AJamPlayerController& Player);

	/** The keys that are not part of the fight: the answer to an ending and the debug keys. */
	void ReadFightKeys(UJamFightSubsystem& Host, const AJamPlayerController& Player);

	/** Shows the state of the fight after the frame's steps. */
	void SyncViews(const UJamFightSubsystem& Host, float Time);

	/**
	 * The intro before the first fight (T-030): the fight waits while the Publisher shows off and the camera watches
	 * him, then goes over to the hero. False once it is over: it runs once per launch.
	 */
	/**
	 * The music between the ambient and the theme (T-094): IntroTheme's rise under the Publisher's show, else a
	 * crossfade as JamIntroRules::IsThemeUp says. Time is the world's, as RunIntro's.
	 */
	void FadeTheme(float DeltaSeconds, float Time);

	bool RunIntro(const UJamFightSubsystem& Host, const AJamPlayerController& Player, float Time);

	/**
	 * The main menu before the intro (T-092): the two fighters idle face to face under a side camera, the skyline behind
	 * them, the rows over them. False once NEW GAME is picked.
	 */
	bool RunMenu(const UJamFightSubsystem& Host, AJamPlayerController& Player, float Time);

	/**
	 * The opening comic after NEW GAME (T-093): five panels, their lines said by the voice; a key skips it. False once
	 * it is over, then the intro runs.
	 */
	bool RunComic(AJamPlayerController& Player, float Time);

	/**
	 * The HOW TO PLAY screen (T-096): up after the comic with a countdown, or over the fight on F1, which waits under it
	 * without one; a key closes it. True while it is up, and on the frame it closes, so that key does nothing else.
	 */
	bool RunHowTo(const UJamFightSubsystem& Host, const AJamPlayerController& Player, float Time);
	/** The whole-screen pages before or over the fight: the menu, the comic, HOW TO PLAY. True while one is up. */
	bool RunPages(const UJamFightSubsystem& Host, AJamPlayerController& Player, float Time);
	void OpenHowTo(bool bCountdown, float Time);

	/** Starts panel Index of the comic: its lines go to the voice. */
	void StartComicPanel(int32 Index, float Time);

	/** The menu's keys and the mouse of this frame, as presses on the menu, with what they ask the game to do. */
	void ReadMenuInput(AJamPlayerController& Player);

	/** Does what a press of the menu asked for. */
	void ActOnMenu(EJamMenuAction Action, AJamPlayerController& Player);

	/** The music, the voice and the master volume as the menu has them. */
	void ApplyVolumes();

	/** The pose gallery of the demo: the hero holds each pose of a list in turn, seen up close from two sides. */
	void RunGallery(const UJamFightSubsystem& Host, float Time);

	/** The stride gallery (T-078): the hero faces east and runs each of eight ways in turn under a close camera. */
	void RunStrideGallery(const UJamFightSubsystem& Host, float Time);

	/** The stride gallery's second part: the hero circles the standing Publisher facing him, backs off, runs in; then the
	 * Publisher walks (T-078). */
	void RunStrideCircle(const UJamFightSubsystem& Host, float Time, float Age, float Dt);

	/**
	 * Takes the cosmetic layers and the music stems the Publisher has cut, or gives them back. A change goes through the
	 * crack (T-068): the old picture is frozen this frame, the new look comes on the next, under the opening crack.
	 */
	void ApplyLayers(const FJamFight& Fight, float Time);

	/** The crack is opening (or about to): the fight holds. */
	bool IsCracking(float Time) const;

	/** Runs the fight's steps of this frame, or holds it while the crack plays. */
	void StepFight(class UJamFightSubsystem& Host, float DeltaSeconds, float Time) const;

	UPROPERTY()
	TObjectPtr<AJamArenaView> Arena;

	UPROPERTY()
	TObjectPtr<AJamFighterView> HeroView;

	UPROPERTY()
	TObjectPtr<AJamFighterView> BossView;

	UPROPERTY()
	TObjectPtr<AJamProjectilesView> Projectiles;

	UPROPERTY()
	TObjectPtr<AJamEffectsView> Effects;

	UPROPERTY()
	TObjectPtr<class AJamFxView> Fx;

	/** The neon lights of the full-art tier (T-077). */
	UPROPERTY()
	TObjectPtr<class AJamLightView> Lights;

	UPROPERTY()
	TObjectPtr<class AJamMoneyView> Money;

	UPROPERTY()
	TObjectPtr<AJamCameraRig> Rig;

	UPROPERTY()
	TObjectPtr<AJamMusic> Music;

	UPROPERTY()
	TObjectPtr<AJamSounds> Sounds;

	/** The voice-over: the intro, each cut's exchange, the stun, the final blow, the barks. */
	UPROPERTY()
	TObjectPtr<class AJamVoice> Voice;

	/** After which cut each cosmetic layer is gone. */
	FJamLayerSettings LayerSettings;

	/** How hard each blow shows: the flash of a hit and the camera shake. */
	FJamFeedbackSettings Feedback;

	/** The neon look: glow of the arena and the effects, bloom. */
	FJamNeonSettings Neon;

	/** When the in-fight hints show, and what the player has already shown he knows. */
	FJamTeachSettings Teach;
	FJamLessons Lessons;

	/** Real time at which the music started. */
	float MusicStart = 0.f;

	/** The bot run that replaces the player's hands when asked for on the command line. */
	FJamDemo Demo;

	/** The main menu (T-092): up until NEW GAME, its state, the screen's resolutions, where the mouse was last frame. */
	bool bMenu = false;
	FJamMenu Menu;
	TArray<FIntPoint> Resolutions;
	FVector2D MenuMouse = FVector2D(-1.0, -1.0);

	/** The opening comic (T-093): up after NEW GAME until it ends or is skipped; when it and its panel started. */
	bool bComic = false;
	int32 ComicPanel = 0;
	float ComicStart = -1.f;
	float ComicPanelStart = 0.f;

	/** The HOW TO PLAY screen (T-096): up, with a countdown, and since when. */
	bool bHowTo = false;
	bool bHowToCountdown = false;
	float HowToStart = 0.f;

	/** True once the player controller is wired to the camera. */
	bool bPlayerReady = false;
	float IntroStart = -1.f;
	float IntroLength = 0.f;
	/** The hero has started his draw in the intro (T-089). */
	bool bHeroDrawing = false;
	bool bIntroDone = false;
	/** How far the intro's camera has gone over to the hero, his spot with it (T-100); 1 outside the intro. */
	float SpotToHero = 1.f;
	/** How far the theme is up over the ambient, 0 to 1 (AJamMusic::SetTheme); the menu starts on the theme. */
	float ThemeLevel = 1.f;
	/** When the fight began after the intro (negative: not yet): the Publisher's bar fades in from here (T-083). */
	float FightStart = -1.f;
	int32 GalleryPose = INDEX_NONE;
	FVector2D StrideGalleryPos = FVector2D::ZeroVector;
	float StrideGalleryTime = -1.f;
	float StrideGalleryStart = -1.f;
	double StrideCircleAngle = 2.0e9;
	double StrideCircleRadius = 0.0;
	/** The look on screen, the frozen frame waiting for its switch, and when the crack began (negative: none). */
	FJamLayers ShownLayers;
	bool bCrackPending = false;
	float CrackStart = -1.f;
	int32 CrackSeed = 0;
};
