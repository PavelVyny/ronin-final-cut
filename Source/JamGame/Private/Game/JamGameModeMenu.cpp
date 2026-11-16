#include "Game/JamGameMode.h"

#include "Audio/JamMusic.h"
#include "Audio/JamSounds.h"
#include "Audio/JamVoice.h"
#include "Game/JamPlayerController.h"
#include "Game/JamSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sim/JamFightSubsystem.h"
#include "UI/JamComicRules.h"
#include "UI/JamHowToRules.h"
#include "UI/JamHUD.h"
#include "View/JamArenaView.h"
#include "View/JamCameraRig.h"
#include "View/JamFighterView.h"
#include "View/JamLightView.h"

namespace
{
// T-092: the menu's shot. The fighters stand this far either side of the centre, face to face across the X axis; the
// camera looks at them from +Y, low, so the hero is on the left of the screen and the Publisher on the right, with the
// skyline (AJamArenaView::ShowBackdrop) behind them on -Y.
constexpr double MenuStandX = 280.0;
const FVector MenuEye(0.0, 1200.0, 170.0);
const FVector MenuLookAt(0.0, 0.0, 125.0);
constexpr float MenuFov = 50.f;

struct FMenuKey
{
	FKey Key;
	EJamMenuKey Press;
};
} // namespace

bool AJamGameMode::RunMenu(const UJamFightSubsystem& Host, AJamPlayerController& Player, float Time)
{
	AJamHUD* Hud = Cast<AJamHUD>(Player.GetHUD());
	ReadMenuInput(Player);
	if (!bMenu)
	{
		return false;
	}
	const FJamFight& Fight = Host.GetFight();
	ApplyLayers(Fight, Time);
	FJamFighter Hero = Fight.Fighters[EJamSide::Hero];
	FJamFighter Boss = Fight.Fighters[EJamSide::Boss];
	Hero.Pos = Hero.PrevPos = FVector2D(-MenuStandX, 0.0);
	Hero.Facing = FVector2D(1.0, 0.0);
	Boss.Pos = Boss.PrevPos = FVector2D(MenuStandX, 0.0);
	Boss.Facing = FVector2D(-1.0, 0.0);
	// Snapped: they face each other from the menu's first frame, not turning to it from where they were spawned.
	HeroView->Sync(Hero, 0.f, Time, true);
	BossView->Sync(Boss, 0.f, Time, true);
	Arena->Pulse(JamNeonRules::LineGlow(GetWorld()->GetRealTimeSeconds() - MusicStart, Neon));
	Lights->Follow(BossView->GetActorLocation(), HeroView->GetActorLocation(), Time);
	Rig->Frame(MenuEye, MenuLookAt, MenuFov);
	int32 Width = 0;
	int32 Height = 0;
	Player.GetViewportSize(Width, Height);
	if (Width > 0 && Height > 0)
	{
		Arena->PlaceBackdrop(MenuEye, MenuLookAt, MenuFov, static_cast<float>(Width) / Height);
	}
	if (Hud)
	{
		Hud->bMenu = true;
		Hud->Menu = Menu;
		Hud->Resolutions = Resolutions;
		Hud->IntroAlpha = 0.f;
		Hud->bSubtitle = false;
	}
	return true;
}

void AJamGameMode::ReadMenuInput(AJamPlayerController& Player)
{
	static const FMenuKey Keys[] = {
		{ EKeys::Up, EJamMenuKey::Up },
		{ EKeys::W, EJamMenuKey::Up },
		{ EKeys::Down, EJamMenuKey::Down },
		{ EKeys::S, EJamMenuKey::Down },
		{ EKeys::Left, EJamMenuKey::Left },
		{ EKeys::A, EJamMenuKey::Left },
		{ EKeys::Right, EJamMenuKey::Right },
		{ EKeys::D, EJamMenuKey::Right },
		{ EKeys::Enter, EJamMenuKey::Confirm },
		{ EKeys::SpaceBar, EJamMenuKey::Confirm },
		{ EKeys::Escape, EJamMenuKey::Back },
		{ EKeys::BackSpace, EJamMenuKey::Back },
	};
	for (const FMenuKey& Key : Keys)
	{
		if (bMenu && Player.WasInputKeyJustPressed(Key.Key))
		{
			ActOnMenu(JamMenuRules::Press(Menu, Key.Press), Player);
		}
	}
	float X = 0.f;
	float Y = 0.f;
	int32 Width = 0;
	int32 Height = 0;
	Player.GetViewportSize(Width, Height);
	if (!bMenu || !Player.GetMousePosition(X, Y))
	{
		return;
	}
	// T-094: the same layout the HUD draws from, so the drawn boxes and tracks are what the mouse hits.
	const FJamMenuLayout Layout = JamMenuRules::Layout(Menu, FVector2D(Width, Height));
	// Only a moving mouse takes the selection, so a pointer resting on a row does not fight the arrow keys.
	const FVector2D Mouse(X, Y);
	if (Mouse != MenuMouse)
	{
		MenuMouse = Mouse;
		JamMenuRules::Hover(Menu, Layout, Mouse);
	}
	if (Player.WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		ActOnMenu(JamMenuRules::Click(Menu, Layout, Mouse), Player);
	}
	else if (Player.IsInputKeyDown(EKeys::LeftMouseButton))
	{
		ActOnMenu(JamMenuRules::Drag(Menu, Layout, Mouse), Player);
	}
	else
	{
		JamMenuRules::Release(Menu);
	}
}

void AJamGameMode::ActOnMenu(EJamMenuAction Action, AJamPlayerController& Player)
{
	switch (Action)
	{
		case EJamMenuAction::NewGame:
			// T-101: the fight on the stage was set up on NORMAL; it starts over on the picked level, which every retry
			// and PLAY AGAIN keeps.
			if (UJamFightSubsystem* Host = GetWorld()->GetSubsystem<UJamFightSubsystem>())
			{
				Host->SetDifficulty(JamDifficultyRules::FromIndex(Menu.Settings.Difficulty));
				Host->Restart();
			}
			bMenu = false;
			bComic = true;
			Arena->ShowBackdrop(false);
			if (AJamHUD* Hud = Cast<AJamHUD>(Player.GetHUD()))
			{
				Hud->bMenu = false;
			}
			break;
		case EJamMenuAction::Exit:
			UKismetSystemLibrary::QuitGame(GetWorld(), &Player, EQuitPreference::Quit, false);
			break;
		case EJamMenuAction::Volume:
			ApplyVolumes();
			break;
		case EJamMenuAction::Graphics:
			JamSettings::ApplyGraphics(Menu.Settings, Resolutions);
			break;
		case EJamMenuAction::Difficulty:
			JamSettings::SaveDifficulty(Menu.Settings);
			break;
		default:
			break;
	}
}

void AJamGameMode::ApplyVolumes()
{
	JamSettings::ApplyVolumes(Menu.Settings, this);
	if (Music)
	{
		Music->SetGain(JamMenuRules::Volume(Menu.Settings.Music));
	}
	if (Voice)
	{
		Voice->SetGain(JamMenuRules::Volume(Menu.Settings.Voice));
	}
}

bool AJamGameMode::RunComic(AJamPlayerController& Player, float Time)
{
	if (ComicStart < 0.f)
	{
		ComicStart = Time;
		StartComicPanel(0, Time);
	}
	const bool bSkip = JamComicRules::MaySkip(Time - ComicStart) &&
		(Player.WasInputKeyJustPressed(EKeys::LeftMouseButton) || Player.WasInputKeyJustPressed(EKeys::SpaceBar) ||
			Player.WasInputKeyJustPressed(EKeys::Enter) || Player.WasInputKeyJustPressed(EKeys::Escape));
	if (!bSkip && JamComicRules::IsPanelDone(ComicPanel, Time - ComicPanelStart, Voice->IsSpeaking()))
	{
		++ComicPanel;
		if (ComicPanel < JamComicRules::PanelCount)
		{
			StartComicPanel(ComicPanel, Time);
		}
	}
	AJamHUD* Hud = Cast<AJamHUD>(Player.GetHUD());
	if (bSkip || ComicPanel >= JamComicRules::PanelCount)
	{
		bComic = false;
		Voice->Hush();
		Sounds->PlayComic(INDEX_NONE);
		// T-096: the controls before the fight, once, after the comic.
		OpenHowTo(true, Time);
		if (Hud)
		{
			Hud->bComic = false;
		}
		return false;
	}
	if (Hud)
	{
		Hud->bComic = true;
		Hud->ComicPanel = ComicPanel;
		Hud->ComicPanelAge = Time - ComicPanelStart;
		Hud->bComicBalloon = Voice->Current(Hud->ComicLine);
	}
	return true;
}

void AJamGameMode::StartComicPanel(int32 Index, float Time)
{
	ComicPanelStart = Time;
	// The session-5s playtest: the keyboard under the first panel, the phone's ring under the second.
	const EJamComicSound Sound = JamComicRules::PanelOf(Index).Sound;
	Sounds->PlayComic(static_cast<int32>(Sound) - 1, JamComicRules::SoundVolume(Sound));
	for (const EJamLine Line : JamComicRules::PanelOf(Index).Lines)
	{
		Voice->Say(Line);
	}
}

void AJamGameMode::OpenHowTo(bool bCountdown, float Time)
{
	bHowTo = true;
	bHowToCountdown = bCountdown;
	HowToStart = Time;
}

bool AJamGameMode::RunHowTo(const UJamFightSubsystem& Host, const AJamPlayerController& Player, float Time)
{
	const float Age = Time - HowToStart;
	const bool bOver = !Demo.ShowsHowTo() && JamHowToRules::IsOver(Age, Player.AnyKeyJustPressed(), bHowToCountdown);
	if (AJamHUD* Hud = Cast<AJamHUD>(Player.GetHUD()))
	{
		Hud->bHowTo = !bOver;
		Hud->HowToSecondsLeft = bHowToCountdown ? JamHowToRules::SecondsLeft(Age) : -1;
		Hud->HowToExplosiveEvery = FMath::RoundToInt32(Host.GetBalance().ShurikenExplosiveEvery);
	}
	bHowTo = !bOver;
	return true;
}
