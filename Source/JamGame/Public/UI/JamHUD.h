#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Audio/JamVoiceRules.h"
#include "UI/JamBarRules.h"
#include "UI/JamMenuRules.h"
#include "UI/JamTeachRules.h"

#include "JamHUD.generated.h"

/**
 * The prototype HUD, drawn on the canvas: the health of both fighters, what each button does now, the debug key hint
 * (not in Shipping), the steal card and the title a fight ends with. Its words live in JamTexts. It reads the fight from the
 * simulation host.
 */
UCLASS()
class JAMGAME_API AJamHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	/**
	 * T-099: AHUD's text calls, hidden: the glyphs are rasterised at the size they are shown at (Scale, grown with the
	 * screen: JamHudScaleRules) instead of a 10-point font scaled up, which was soft.
	 */
	void DrawText(const FString& Text, FLinearColor Color, float ScreenX, float ScreenY, class UFont* Font = nullptr, float Scale = 1.f,
		bool bScalePosition = false);
	void GetTextSize(const FString& Text, float& OutWidth, float& OutHeight, class UFont* Font = nullptr, float Scale = 1.f) const;

	/** False once the Publisher took the HUD: the health bars go, the plain text stays. */
	bool bBars = true;

	/** T-104: the neon bars, through the first cut (JamNeonBarRules::Shows); the plain ones after, so the HUD degrades too. */
	bool bNeonBars = true;

	/** An enemy shot is about to land and LMB would send it back (JamTeachRules): "LMB PARRY" over the hero. */
	bool bParryPrompt = false;

	/** The hero's Ultimate is full (JamTeachRules): the pulsing banner, and the call over him until he has used it. */
	EJamUltimateCall UltimateCall = EJamUltimateCall::None;

	/** The crack between two looks (T-068): the frozen old picture, how long the crack has been opening (negative: no
	 * crack), and its shape. */
	UPROPERTY()
	TObjectPtr<UTexture> CrackFrame;
	float CrackAge = -1.f;
	int32 CrackSeed = 0;

	/** How much of the Publisher's health bar shows: none through the intro, fading in after it (T-083). */
	float BossBarAlpha = 1.f;

	/** How visible the intro's title card is (T-030); 0 outside the intro. */
	float IntroAlpha = 0.f;

	/** The subtitle of the voice-over: whether somebody talks, the line (JamTexts::VoiceLine) and who says it. */
	bool bSubtitle = false;
	int32 SubtitleLine = 0;
	EJamSide SubtitleSpeaker = EJamSide::Boss;

	/** T-092: the main menu is up instead of the fight's HUD; its state and the screen's resolutions, as the menu shows them. */
	bool bMenu = false;
	FJamMenu Menu;
	TArray<FIntPoint> Resolutions;

	/**
	 * T-093: the opening comic is up instead of everything else: the panel shown, how long it has been up, the voice line
	 * in a balloon over it (bComicBalloon false: none).
	 */
	bool bComic = false;
	int32 ComicPanel = 0;
	float ComicPanelAge = 0.f;
	bool bComicBalloon = false;
	EJamLine ComicLine = EJamLine::ComicBoss1;

	/**
	 * T-096: the HOW TO PLAY screen is up over everything: whole seconds left before the fight starts on its own (-1:
	 * none, the fight waits under it), and every how-many-th shuriken explodes (the balance's number).
	 */
	bool bHowTo = false;
	int32 HowToSecondsLeft = -1;
	int32 HowToExplosiveEvery = 4;

private:
	/** The HOW TO PLAY schematic (JamHUDHowTo.cpp): the keyboard and the mouse, our keys lit, the labels, the prompt. */
	void DrawHowTo();
	/** Every key of the schematic's board and mouse: lit ones filled and cyan, the rest dim. */
	void DrawHowToKeys(const struct FJamHowToLayout& Layout, float Scale);
	/** The mouse: its generated neon picture (T_HowToMouse) over its box, or a plain outline without it. */
	void DrawHowToMouse(const struct FJamHowToLayout& Layout);
	/** The leader lines under the keys (bDots false), or the dots where they meet a key and a label over them. */
	void DrawHowToLines(const struct FJamHowToLayout& Layout, float Scale, bool bDots);
	/** A thin outline round Box, no glow. */
	void DrawBox(const FBox2D& Box, const FLinearColor& Color, float Thickness);
	/** A neon outline round Box: a faint wide glow under a thin bright line. */
	void DrawNeonBox(const FBox2D& Box, const FLinearColor& Color, float Thickness);
	void DrawNeonLine(const FVector2D& From, const FVector2D& To, const FLinearColor& Color, float Thickness);

	/** The comic: the panel sliding in over the last one, then its caption, sound word, balloon and the skip hint. */
	void DrawComic();

	/**
	 * Panel Index in the comic's Frame (JamComicRules::Frame), Age into its camera move, its left edge Offset pixels into
	 * the frame (it slides in from the right; the part past the frame is not drawn).
	 */
	void DrawComicPanel(int32 Index, float Age, float Offset, const FBox2D& Frame);

	/** A speech balloon: the words in capitals, wrapped, in a white box with a tail, centred at At (fractions of Frame). */
	void DrawBalloon(const FString& Words, const FVector2D& At, const FBox2D& Frame);

	/** A panel's caption box and sound word (JamTexts::ComicCaptions, ComicSounds), placed in Frame. */
	void DrawComicWords(int32 Index, const FBox2D& Frame);

	/** The comic's pictures, /Game/UI/Comic/T_Comic_1 to 5. */
	void LoadComic();

	UPROPERTY()
	TArray<TObjectPtr<class UTexture2D>> ComicPictures;

	/** HOW TO PLAY's mouse picture, loaded on its first draw. */
	UPROPERTY()
	TObjectPtr<class UTexture2D> HowToMouse;

	/** The title over the two fighters, the rows of the page in the middle, the hint at the bottom. */
	void DrawMenu();

	/** The game's name over the menu, in the cover's colours. */
	void DrawMenuTitle();

	/** What a row shows: the dropdown's value, the slider's percentage or the difficulty's level; empty for an action. */
	FString MenuValue(EJamMenuItem Item) const;

	/** Option Index of Item's dropdown, as its list shows it (T-094). */
	FString OptionName(EJamMenuItem Item, int32 Index) const;

	/** One row of the menu on its band: the label, and the dropdown or the slider of a setting. */
	void DrawMenuRow(EJamMenuItem Item, const FBox2D& Row, const FBox2D& Control, bool bSelected);

	/** A setting's dropdown: its value in a framed box and the arrow that says a list opens. */
	void DrawDropdown(EJamMenuItem Item, const FBox2D& Box, bool bSelected);

	/** A volume's slider: the track, filled to the value, the knob, and the percentage left of it. */
	void DrawSlider(EJamMenuItem Item, const FBox2D& Track, bool bSelected);

	/** T-101: the difficulty's selector: an arrow at each end of its box, the level between them. */
	void DrawSelector(EJamMenuItem Item, const FBox2D& Box, bool bSelected);

	/** The menu's text scale, made smaller for Text only when it would not fit Room pixels across. */
	float FittedMenuScale(const FString& Text, float Room);

	/** The open dropdown's options over the page: the current value in cyan, the one under the arrows or pointer lit. */
	void DrawMenuList(const struct FJamMenuLayout& Layout);

	/** A filled box with a thin frame round it. */
	void DrawFramedBox(const FBox2D& Box, const FLinearColor& Fill, const FLinearColor& Frame);

	/** The hero's health bar and numbers, his loadout over them and the Ultimate meter under them. */
	void DrawHeroBar(const FJamFight& Fight);

	/** The line being said, low in the middle: the speaker's name in his colour, then the words. */
	void DrawSubtitle();

	/** The Ultimate meter under the health bar, and the banner when it is full. */
	void DrawUltimate(const FJamFight& Fight, float X, float Width);

	/** The Publisher's health bar and numbers, faded by BossBarAlpha. */
	void DrawBossBar(const struct FJamFighter& Boss);

	/** A short call in the world over the hero's head, centred on him. */
	void DrawOverHero(const FJamFighter& Hero, const FString& Text, const FLinearColor& Color);

	void DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& Color);

	/** One neon bar of the full art (JamHUDNeon.cpp): its box, the slant of its ends and what it shows. */
	struct FJamNeonBar
	{
		FBox2D Box = FBox2D(ForceInit);
		/** x per pixel of height, from the bar's bottom line, for all its shapes (JamNeonBarRules::Slanted). */
		float Slope = 0.f;
		FLinearColor Color = FLinearColor::White;
		int32 Segments = 1;
		float Fill = 0.f;
		float Chunk = 0.f;
		float Preview = 0.f;
		float Alpha = 1.f;
		/** How far the lit segments go towards white (the full Ultimate's pulse) and how strong the glow is. */
		float Whiten = 0.f;
		float Glow = 1.f;
	};
	void DrawNeonBar(const FJamNeonBar& Bar);

	/** The Publisher's neon bar: his name tab, the cut marks where he steals and his numbers. */
	void DrawNeonBoss(const struct FJamFighter& Boss, const struct FJamBalanceData& Balance);

	/** The hero's neon bar, his name and health over it, the Ultimate bar under it; returns the top of his name. */
	float DrawNeonHero(const FJamFight& Fight);

	/** A health bar (JamBarRules): the solid part, the white chunk of health just lost, the light preview of a heal. */
	void DrawHealthBar(float X, float Y, float Width, float Height, const FJamBarState& Bar, const FLinearColor& Color, float Alpha = 1.f);

	/** What each health bar shows, moved on every frame towards the fighter's health. */
	FJamBarSettings BarSettings;
	FJamBarState HeroBar;
	FJamBarState BossBar;

	/** One line per button: what it does for the hero now, and what the Publisher took from it. */
	void DrawLoadout(const FJamLedger& Ledger, float Meter, float X, float Bottom);

	/** An arrow at the screen edge towards the Publisher while the camera has lost him (JamMarkerRules). */
	void DrawBossMarker(const FJamFighter& Boss);

	/** A line of small text at the right edge; rows count up from the bottom one. */
	void DrawHint(const FString& Text, int32 Row);

	/** What the phase of the fight puts over the screen: the steal card, the stun call or an ending. */
	void DrawPhase(const FJamFight& Fight);

	/**
	 * The Publisher's grab: the options side by side (JamCardRules::OptionBox, where a click picks them), the key of each
	 * and the time left to answer. Age seconds after it opened: the band pops in, the options a beat later (T-094).
	 */
	void DrawStealCard(const FJamFight& Fight, float Age);

	/** Option Index of the card in Box, Grow of the way through its pop. */
	void DrawStealOption(const FJamFight& Fight, int32 Index, const FBox2D& Box, float Grow);

	/** The boss is down and everything is back: the call for the final blow, its band Grow of the way through its pop. */
	void DrawStun(float Grow);

	/** The title over the screen once the fight is over, its band Grow of the way through its pop. */
	void DrawEnding(bool bWon, float Grow);

	/** A dark band across the screen, Grow of the way through its pop, grown from its middle line. */
	void DrawBand(float Top, float Height, float Grow);

	/** The phase the last frame showed and when it began, in real seconds: the panels pop in from there. */
	EJamPhase ShownPhase = EJamPhase::Fight;
	float PhaseStart = 0.f;

	/** The old picture on both sides of the opening crack, and the crack's glowing edges. */
	void DrawCrack();

	/** The intro's title card over the Publisher's show-off, IntroAlpha of the way in. */
	void DrawIntro();

	void DrawCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color);
};
