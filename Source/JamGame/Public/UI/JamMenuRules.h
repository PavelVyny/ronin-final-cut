#pragma once

#include "CoreMinimal.h"

enum class EJamMenuPage : uint8
{
	Main,
	Settings
};

/** Every row the menu has, on either page, in the order of their labels (JamTexts::MenuItems). */
enum class EJamMenuItem : uint8
{
	NewGame,
	Settings,
	Exit,
	Quality,
	Window,
	Resolution,
	Master,
	Music,
	Voice,
	Back,
	/** T-101: EASY, NORMAL, HARD on the main page, under NEW GAME. */
	Difficulty,
	/** NVIDIA DLSS Super Resolution on the settings page, under RESOLUTION: JamTexts::DlssNames. */
	Dlss
};

/** What the player pressed: the arrows (or WASD), Enter, Escape. */
enum class EJamMenuKey : uint8
{
	Up,
	Down,
	Left,
	Right,
	Confirm,
	Back
};

/** What the game has to do after a press. */
enum class EJamMenuAction : uint8
{
	None,
	NewGame,
	Exit,
	/** A volume changed: heard at once. */
	Volume,
	/** The settings page closed: the graphics settings are applied and saved. */
	Graphics,
	/** The difficulty changed: saved, and played from the next NEW GAME. */
	Difficulty
};

/** The settings as the menu shows them: indices and volume steps. */
struct FJamMenuSettings
{
	/** 0 LOW to 3 EPIC, the engine's scalability levels. */
	int32 Quality = 3;
	/** JamTexts::WindowNames: fullscreen, borderless, windowed. */
	int32 Window = 1;
	/** Into the screen's resolutions, ResolutionCount long. */
	int32 Resolution = 0;
	int32 ResolutionCount = 0;
	int32 Master = 10;
	int32 Music = 10;
	int32 Voice = 10;
	/** T-101: JamTexts::DifficultyNames, EASY, NORMAL, HARD; NORMAL by default. */
	int32 Difficulty = 1;
	/** JamTexts::DlssNames, OFF, QUALITY, BALANCED, PERFORMANCE; always OFF where DLSS does not run. */
	int32 Dlss = 0;
	/** DLSS Super Resolution runs on this machine: an RTX card with a recent driver. */
	bool bDlssSupported = false;
};

struct FJamMenu
{
	EJamMenuPage Page = EJamMenuPage::Main;
	/** The row under the cursor or the arrows, into ItemsOf(Page). */
	int32 Selected = 0;
	FJamMenuSettings Settings;
	/** T-094: the selected row's dropdown is open, with the option under the arrows or the pointer. */
	bool bListOpen = false;
	int32 ListSelected = 0;
	/** The selected row's slider follows the held mouse button. */
	bool bDragging = false;
};

/**
 * Where everything of a page is on a screen, in pixels (T-094). The HUD draws from it and the mouse is tested against
 * it, so what shows is what takes the click.
 */
struct FJamMenuLayout
{
	/** The dark panel behind the settings page; an empty box on the main page. */
	FBox2D Panel = FBox2D(ForceInit);
	/** Each row's band, top to bottom. */
	TArray<FBox2D> Rows;
	/** Each row's control: the dropdown's box or the slider's track; an empty box for an action. */
	TArray<FBox2D> Controls;
	/** The open dropdown's options, top to bottom; none while no list is open. */
	TArray<FBox2D> Options;
};

/**
 * The main menu before the intro (T-092): which rows a page has and what a key or a click does to them. The quality, the
 * window mode and the resolution are dropdowns, the volumes sliders (T-094). A pick from a dropdown applies at once; the
 * side arrows step a graphics value without applying it until the settings page closes, so stepping through
 * resolutions does not flash the screen. Volumes are heard at once. The main page's DIFFICULTY (T-101) is a selector:
 * the side arrows, Enter and its drawn arrows step it round EASY, NORMAL, HARD.
 * Tests: Jam.Menu.*
 */
namespace JamMenuRules
{
inline constexpr int32 ItemCount = 12;
inline constexpr int32 DifficultyCount = 3;
inline constexpr int32 DlssCount = 4;
inline constexpr int32 QualityCount = 4;
inline constexpr int32 WindowCount = 3;
inline constexpr int32 VolumeSteps = 10;

JAMGAME_API TArray<EJamMenuItem> ItemsOf(EJamMenuPage Page);

/** One key on the menu: moves the selection, steps a value, changes the page or asks the game to act. */
JAMGAME_API EJamMenuAction Press(FJamMenu& Menu, EJamMenuKey Key);

/** The pointer is over Row of the page (INDEX_NONE: over none): that row becomes the selected one. */
JAMGAME_API void Point(FJamMenu& Menu, int32 Row);

/** The row of Count rows, the first at Top, each RowHeight tall, that Y falls in; INDEX_NONE outside them. */
JAMGAME_API int32 RowAt(float Y, float Top, float RowHeight, int32 Count);

/** The volume multiplier of a number of steps. */
JAMGAME_API float Volume(int32 Steps);

/** The quality, the window mode and the resolution open a list; the volumes are sliders. */
JAMGAME_API bool IsList(EJamMenuItem Item);
JAMGAME_API bool IsSlider(EJamMenuItem Item);

/** T-101: the difficulty is a selector, < NORMAL >: its arrows step it round its levels. */
JAMGAME_API bool IsSelector(EJamMenuItem Item);

/** How many options Item's list has: the qualities, the window modes, the screen's resolutions; 0 for other rows. */
JAMGAME_API int32 OptionCount(const FJamMenuSettings& Settings, EJamMenuItem Item);

/** Where the page's rows, controls and open list are on a screen of Screen pixels. */
JAMGAME_API FJamMenuLayout Layout(const FJamMenu& Menu, const FVector2D& Screen);

/** The pointer moved to At: the row, or the option of the open list, under it becomes the selected one. */
JAMGAME_API void Hover(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At);

/**
 * A left click at At: an action row confirms, a dropdown opens its list, a slider's track takes the value under the
 * pointer and starts a drag. With a list open, a click on an option picks it and anywhere else closes the list.
 */
JAMGAME_API EJamMenuAction Click(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At);

/**
 * The size of a window of Size on a desktop of Desktop pixels: as it is if it fits inside the desktop with room for the
 * title bar and the taskbar, else scaled down, its shape kept, until it does (the author's 5v run: a 3840x2160 window
 * hung off the screen with only its top-left corner on it). A desktop of no size leaves Size alone.
 */
JAMGAME_API FIntPoint FitWindow(const FIntPoint& Size, const FIntPoint& Desktop);

/** The mouse button is still held at At: a dragged slider follows it, even off its track. */
JAMGAME_API EJamMenuAction Drag(FJamMenu& Menu, const FJamMenuLayout& Layout, const FVector2D& At);

/**
 * The DLSS mode to start with from the Saved one (INDEX_NONE: never saved): QUALITY on a fresh install, the saved mode
 * kept in range, and OFF wherever DLSS is not supported, so such a machine runs as it did before DLSS.
 */
JAMGAME_API int32 DlssFor(int32 Saved, bool bSupported);

/** The mouse button is up: no slider follows it any more. */
JAMGAME_API void Release(FJamMenu& Menu);
} // namespace JamMenuRules
