#include "Game/JamGameMode.h"

#include "JamGame.h"

#include "Audio/JamMusic.h"
#include "Audio/JamSounds.h"
#include "Audio/JamVoice.h"
#include "Core/JamConfig.h"
#include "Core/JamKeys.h"
#include "Game/JamIntroRules.h"
#include "Game/JamSettings.h"
#include "UI/JamCrackRules.h"
#include "Game/JamPlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sim/JamAimRules.h"
#include "Sim/JamFightSubsystem.h"
#include "UI/JamHUD.h"
#include "UI/JamNeonBarRules.h"
#include "View/JamArenaView.h"
#include "View/JamCameraRig.h"
#include "View/JamEffectsView.h"
#include "View/JamFighterView.h"
#include "View/JamFxView.h"
#include "View/JamLightView.h"
#include "View/JamMoneyView.h"
#include "View/JamProjectilesView.h"

namespace
{
// T-083: how long the Publisher's health bar takes to fade in once the fight starts.
constexpr float BossBarFade = 0.6f;
const FLinearColor HeroBeanColor(0.1f, 0.75f, 1.0f);
const FLinearColor BossBeanColor(0.75f, 0.1f, 0.12f);
// The world runs this slow while an Ultimate winds up: presentation only, the simulation counts its own steps.
constexpr float UltimateTimeDilation = 0.3f;
// The intro (T-030): no skip in the first half second, the title fades over half a second, the camera takes 0.8 s to
// go over to the hero.
constexpr float IntroSkipAfter = 0.5f;
constexpr float IntroTitleFade = 0.5f;
constexpr float IntroReturn = 0.8f;
// T-094: the theme comes in over the ambient, or gives way to it, over this long.
constexpr float ThemeFadeSeconds = 2.5f;
// T-081: one splash of the wet floor per stride, in cm.
constexpr float HeroStride = 110.f;
constexpr float BossStride = 180.f;

} // namespace

AJamGameMode::AJamGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = AJamPlayerController::StaticClass();
	HUDClass = AJamHUD::StaticClass();
	DefaultPawnClass = nullptr;
}

void AJamGameMode::RestartPlayer(AController* NewPlayer)
{
}

void AJamGameMode::StartPlay()
{
	Super::StartPlay();
	Demo.Init();
	LayerSettings = JamConfig::Load<FJamLayerSettings>(JamLayerRules::Section);
	Feedback = JamConfig::Load<FJamFeedbackSettings>(JamFeedbackRules::Section);
	Neon = JamConfig::Load<FJamNeonSettings>(JamNeonRules::Section);
	Teach = JamConfig::Load<FJamTeachSettings>(JamTeachRules::Section);
	UWorld* World = GetWorld();
	// The music starts with the play and runs on real time: the floor lines count its beats from here.
	MusicStart = World->GetRealTimeSeconds();
	const UJamFightSubsystem* Host = World->GetSubsystem<UJamFightSubsystem>();
	if (!Host)
	{
		return;
	}
	const FJamFight& Fight = Host->GetFight();
	Arena = World->SpawnActor<AJamArenaView>();
	Arena->Init(Host->GetBalance().ArenaRadius, Neon);
	Lights = World->SpawnActor<AJamLightView>();
	Lights->Init(Host->GetBalance().ArenaRadius, *Arena);
	HeroView = World->SpawnActor<AJamFighterView>();
	HeroView->Init(EJamSide::Hero, Fight.Fighters[EJamSide::Hero].Radius, Fight.Fighters[EJamSide::Hero].Speed, HeroBeanColor);
	BossView = World->SpawnActor<AJamFighterView>();
	BossView->Init(EJamSide::Boss, Fight.Fighters[EJamSide::Boss].Radius, Fight.Fighters[EJamSide::Boss].Speed, BossBeanColor);
	Projectiles = World->SpawnActor<AJamProjectilesView>();
	Effects = World->SpawnActor<AJamEffectsView>();
	Effects->Init(Host->GetBalance(), Feedback);
	Effects->EffectGlow = Neon.EffectGlow;
	Fx = World->SpawnActor<AJamFxView>();
	Fx->Init();
	Money = World->SpawnActor<AJamMoneyView>();
	Money->Init(Feedback);
	Rig = World->SpawnActor<AJamCameraRig>();
	Music = World->SpawnActor<AJamMusic>();
	Sounds = World->SpawnActor<AJamSounds>();
	Voice = World->SpawnActor<AJamVoice>();
	if (!Demo.IsOn())
	{
		JamSettings::SettleDisplay();
	}
#if !WITH_EDITOR
	// T-102: the jam asks for a Development build; a player of it sees what Shipping shows: no engine messages over the
	// picture and no console on the key left of 1.
	GAreScreenMessagesEnabled = false;
	GetMutableDefault<UInputSettings>()->ConsoleKeys.Reset();
#endif
	Menu.Settings = JamSettings::Load(Resolutions);
	JamSettings::ApplyDlss(Menu.Settings);
	ApplyVolumes();
}

void AJamGameMode::ReadFightKeys(UJamFightSubsystem& Host, const AJamPlayerController& Player)
{
#if WITH_EDITOR
	// In the editor only: a packaged Development build is what the jam's players get (T-102).
	if (Player.WasInputKeyJustPressed(EKeys::F5))
	{
		Host.DebugSkip();
	}
	if (Player.WasInputKeyJustPressed(EKeys::F6))
	{
		Host.Restart();
	}
#endif
	// R answers an ending only: a slip of the finger in the middle of the fight costs nothing.
	const EJamPhase Phase = Host.GetFight().Phase;
	if (Player.WasInputKeyJustPressed(JamKeys::Retry()) || Player.WasInputKeyJustPressed(EKeys::Enter))
	{
		if (Phase == EJamPhase::HeroDead)
		{
			Host.Retry();
		}
		else if (Phase == EJamPhase::Won)
		{
			Host.Restart();
		}
	}
}

void AJamGameMode::StepFight(UJamFightSubsystem& Host, float DeltaSeconds, float Time) const
{
	// The fight holds while the crack opens between two looks (T-068).
	if (!IsCracking(Time))
	{
		Host.Advance(DeltaSeconds);
		return;
	}
	// T-085: the steal's events stayed in the frame through the crack and replayed its sound every frame.
	Host.Hold();
}

bool AJamGameMode::IsCracking(float Time) const
{
	return bCrackPending || (CrackStart >= 0.f && !JamCrackRules::IsOver(Time - CrackStart, JamCrackRules::Seconds));
}

void AJamGameMode::ApplyLayers(const FJamFight& Fight, float Time)
{
	AJamHUD* Hud = Cast<AJamHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	const FJamLayers Wanted = JamLayerRules::For(Fight.Steal.Done, Fight.Phase, LayerSettings);
	if (!(Wanted == ShownLayers) && CrackStart < 0.f)
	{
		if (!bCrackPending)
		{
			// This frame still shows the old look: it is the picture the crack opens through.
			bCrackPending = true;
			if (Hud)
			{
				Hud->CrackFrame = Rig->Freeze();
			}
		}
		else
		{
			bCrackPending = false;
			ShownLayers = Wanted;
			CrackStart = Time;
			++CrackSeed;
		}
	}
	if (CrackStart >= 0.f && JamCrackRules::IsOver(Time - CrackStart, JamCrackRules::Seconds))
	{
		CrackStart = -1.f;
	}
	if (Hud)
	{
		Hud->CrackAge = CrackStart >= 0.f ? Time - CrackStart : -1.f;
		Hud->CrackSeed = CrackSeed;
		Hud->bBars = ShownLayers.bHud;
		// T-104: the neon bars outlive the world's art by a cut (the author's play-through).
		Hud->bNeonBars = JamNeonBarRules::Shows(ShownLayers);
	}
	const FJamLayers& Left = ShownLayers;
	// The session-5s playtest: the hero keeps his model and his dash's afterimages a cut longer than the world.
	HeroView->SetLayers(JamLayerRules::HeroLayers(Left));
	Arena->SetLook(JamArenaRules::LookFor(Left));
	Effects->bHeroEffects = Left.bEffects;
	Effects->bArt = Left.bArt;
	Rig->SetBloom(JamNeonRules::Bloom(Left, Neon));
	// T-077: Lumen, the neon lights and the grade go with the full art, the rest of the post with the effects.
	const EJamLight Light = JamNeonRules::LightFor(Left);
	Rig->SetLight(Light);
	Lights->SetLight(Light);
	// T-094: the film look of the full art; the menu's skyline stands far out, so the fog stays off under it.
	FJamFilmLook Film = JamNeonRules::FilmLookFor(Light, Neon);
	Rig->SetFilm(Film);
	if (bMenu)
	{
		Film.FogDensity = 0.f;
	}
	Lights->SetHaze(Film);
	Music->SetStems(JamLayerRules::StemsFor(Fight.Steal.Done, Fight.Phase, LayerSettings));
	// T-072: the full Ultimate's cue and the dash degrade with the look.
	Effects->ChargeLook = JamFxRules::ChargeLook(Left);
	Effects->TelegraphLook = JamFxRules::TelegraphLook(Left);
}

void AJamGameMode::SyncViews(const UJamFightSubsystem& Host, float Time)
{
	const FJamFight& Fight = Host.GetFight();
	const float Alpha = Host.GetAlpha();
	ApplyLayers(Fight, Time);
	for (const FJamEvent& Event : Host.GetFrameEvents())
	{
		if (Event.Type == EJamEvent::Hit)
		{
			(Event.Side == EJamSide::Hero ? HeroView : BossView)
				->Flash(Time, JamFeedbackRules::FlashWeight(Event.Side, Event.Amount, Feedback));
		}
		Rig->Kick(JamFeedbackRules::ShakeKick(Event, Feedback), Time);
	}
	JamTeachRules::Learn(Lessons, Host.GetFrameEvents());
	const int32 ParryCue = JamTeachRules::ParryPromptFor(Fight, Lessons, Teach);
	const EJamUltimateCall UltimateCall = JamTeachRules::UltimateCallFor(Fight, Lessons);
	Effects->ParryCue = ParryCue;
	Effects->bUltimateReady = UltimateCall != EJamUltimateCall::None;
	if (AJamHUD* Hud = Cast<AJamHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		Hud->bParryPrompt = ParryCue != INDEX_NONE;
		Hud->UltimateCall = UltimateCall;
		EJamLine Said;
		Hud->bSubtitle = Voice->Current(Said);
		if (bIntroDone && FightStart < 0.f)
		{
			FightStart = Time;
		}
		Hud->BossBarAlpha = bIntroDone ? JamIntroRules::BossBarAlpha(Time - FightStart, BossBarFade) : 0.f;
		Hud->SubtitleLine = static_cast<int32>(Said);
		Hud->SubtitleSpeaker = JamVoiceRules::SpeakerOf(Said);
	}
	HeroView->Act(Fight, Host.GetFrameEvents(), Time);
	BossView->Act(Fight, Host.GetFrameEvents(), Time);
	HeroView->Sync(Fight.Fighters[EJamSide::Hero], Alpha, Time);
	BossView->Sync(Fight.Fighters[EJamSide::Boss], Alpha, Time);
	Arena->Pulse(JamNeonRules::LineGlow(GetWorld()->GetRealTimeSeconds() - MusicStart, Neon));
	// T-081: the wet shrine floor splashes under both fighters' feet, the Publisher's strides longer.
	Arena->Wade(static_cast<int32>(EJamSide::Hero), HeroView->GetActorLocation(), HeroStride);
	Arena->Wade(static_cast<int32>(EJamSide::Boss), BossView->GetActorLocation(), BossStride);
	Projectiles->Hands[EJamSide::Hero] = HeroView->ThrowingHand();
	Projectiles->Hands[EJamSide::Boss] = BossView->ThrowingHand();
	Projectiles->bArt = ShownLayers.bArt;
	Projectiles->Sync(Fight.Projectiles, Alpha, Time);
	Effects->Sync(Fight, Host.GetFrameEvents(), Alpha, Time);
	Fx->Sync(Fight, Host.GetFrameEvents(), ShownLayers.bArt, HeroView->GetActorLocation());
	Money->Sync(Fight, Host.GetFrameEvents(), Time);
	// The voice first: a line that starts this frame ducks the crack's sound (T-085). A steal's comment waits for the crack.
	Voice->Hear(Host.GetFrameEvents(), Fight, IsCracking(Time));
	Sounds->Play(Host.GetFrameEvents(), Fight, Voice->IsSpeaking());
	Lights->Follow(BossView->GetActorLocation(), HeroView->GetActorLocation(), Time, SpotToHero);
	Rig->Sync(HeroView->GetActorLocation(), Time);
	if (Demo.IsClose())
	{
		// T-078: a camera closer and lower than the game's, behind the hero, so the steps of his feet read.
		const FVector Feet = HeroView->GetActorLocation();
		Rig->Frame(Feet + FVector(-650.0, 0.0, 620.0), Feet + FVector(0.0, 0.0, 60.0), 50.f);
	}
}

void AJamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Demo.CountFrame(FApp::GetDeltaTime(), GetWorld()->GetRealTimeSeconds());
	UJamFightSubsystem* Host = GetWorld()->GetSubsystem<UJamFightSubsystem>();
	AJamPlayerController* Player = Cast<AJamPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!Host || !Player || !Rig)
	{
		return;
	}
	const FJamFighter& Hero = Host->GetFight().Fighters[EJamSide::Hero];
	if (!bPlayerReady)
	{
		WirePlayer(*Player);
	}

	const float Time = GetWorld()->GetTimeSeconds();
	FadeTheme(DeltaSeconds, Time);
	if (Demo.IsGallery() || Demo.IsStrideGallery())
	{
		Demo.IsGallery() ? RunGallery(*Host, Time) : RunStrideGallery(*Host, Time);
		Demo.EndFrame(Time);
		return;
	}
	if (RunPages(*Host, *Player, Time))
	{
		return;
	}
	if (!bIntroDone && RunIntro(*Host, *Player, Time))
	{
		Demo.EndFrame(Time);
		return;
	}
	if (Demo.IsOn())
	{
		Host->HeroIntent = Demo.Drive(*Host, Time);
	}
	else
	{
		ReadFightKeys(*Host, *Player);
		const FJamFight& Fight = Host->GetFight();
		Host->HeroIntent = Player->ReadIntent(*Rig, Hero, Fight.Phase == EJamPhase::Steal ? Fight.Steal.Offer.Num() : 0);
	}
	StepFight(*Host, DeltaSeconds, Time);
	const FJamFight& Now = Host->GetFight();
	// Only the hero's own wind-up slows the world: the Publisher's long one (T-038) is there to be walked out of.
	const bool bWindingUp = Now.Fighters[EJamSide::Hero].Ultimate.bPending;
	UGameplayStatics::SetGlobalTimeDilation(this, bWindingUp ? UltimateTimeDilation : 1.f);
	SyncViews(*Host, Time);
	if (!Demo.IsOn())
	{
		// The model turns to the cursor on every frame, not on the 60 Hz step: the aim never trails the mouse.
		const FVector2D Point = Player->LiveAimPoint(Rig->GetActorLocation());
		const FVector2D Shown(HeroView->GetActorLocation());
		HeroView->Face(JamAimRules::FacingTo(Point, Shown, Now.Fighters[EJamSide::Hero].Facing));
	}
	Demo.EndFrame(Time);
}

bool AJamGameMode::RunPages(const UJamFightSubsystem& Host, AJamPlayerController& Player, float Time)
{
	if (bMenu && RunMenu(Host, Player, Time))
	{
		// -JamCapture saves the menu's frames too (the floor's look, T-103).
		Demo.EndFrame(Time);
		return true;
	}
	if (bComic && RunComic(Player, Time))
	{
		return true;
	}
	// T-096: F1 over the fight (not the intro) brings the controls back; the bot's run can hold them up for a picture.
	const bool bF1 = !Demo.IsOn() && bIntroDone && Player.WasInputKeyJustPressed(JamKeys::HowTo());
	if (!bHowTo && (bF1 || Demo.ShowsHowTo()))
	{
		OpenHowTo(false, Time);
	}
	if (bHowTo && RunHowTo(Host, Player, Time))
	{
		Demo.EndFrame(Time);
		return true;
	}
	return false;
}

void AJamGameMode::FadeTheme(float DeltaSeconds, float Time)
{
	// T-094: the ambient under the comic; the theme rises under the Publisher's show, full as the camera goes to the
	// hero, and crossfades in from wherever it is on a skip.
	const bool bShow = !bMenu && !bComic && !bHowTo && !bIntroDone && IntroStart >= 0.f;
	ThemeLevel = bShow
		? JamIntroRules::IntroTheme(Time - IntroStart, IntroLength, IntroReturn)
		: JamIntroRules::Crossfade(ThemeLevel, JamIntroRules::IsThemeUp(bMenu, bComic, bIntroDone), DeltaSeconds, ThemeFadeSeconds);
	if (Music)
	{
		Music->SetTheme(ThemeLevel);
	}
}

void AJamGameMode::WirePlayer(AJamPlayerController& Player)
{
	// The controller ticks first, so the keys read here are this frame's keys.
	AddTickPrerequisiteActor(&Player);
	Player.SetViewTarget(Rig);
	Player.UseCursor();
	bPlayerReady = true;
	bIntroDone = Demo.SkipsIntro();
	// A bot run (a clip, a smoke test) starts without the menu.
	bMenu = !Demo.IsOn();
	Arena->ShowBackdrop(bMenu);
}

bool AJamGameMode::RunIntro(const UJamFightSubsystem& Host, const AJamPlayerController& Player, float Time)
{
	AJamHUD* Hud = Cast<AJamHUD>(Player.GetHUD());
	if (IntroStart < 0.f)
	{
		IntroStart = Time;
		IntroLength = BossView->LengthOf(EJamClip::Entrance);
		BossView->Perform(EJamClip::Entrance, Time);
		Voice->Say(EJamLine::IntroBoss, true);
	}
	const float Age = Time - IntroStart;
	// T-089: the hero draws as the camera comes to him, and the fight waits for his draw to end.
	const float Draw = HeroView->LengthOf(EJamClip::Entrance);
	if (!bHeroDrawing && Draw > 0.f && Age >= JamIntroRules::HeroDrawStart(IntroLength, IntroReturn))
	{
		bHeroDrawing = true;
		HeroView->Perform(EJamClip::Entrance, Time);
	}
	const bool bSkip = Player.WasInputKeyJustPressed(EKeys::LeftMouseButton) || Player.WasInputKeyJustPressed(EKeys::SpaceBar) ||
		Player.WasInputKeyJustPressed(EKeys::Enter) || Player.WasInputKeyJustPressed(EKeys::Escape);
	const float Full = JamIntroRules::FullLength(IntroLength, IntroReturn, Draw);
	if (IntroLength <= 0.f || JamIntroRules::IsOver(Age, Full, bSkip, IntroSkipAfter))
	{
		bIntroDone = true;
		SpotToHero = 1.f;
		BossView->Interrupt(Time);
		HeroView->Interrupt(Time);
		// The hero answers as the fight starts: after the Publisher's line, or over it when the player skipped the show.
		Voice->Say(EJamLine::IntroHero, bSkip);
		if (Hud)
		{
			Hud->IntroAlpha = 0.f;
		}
		return false;
	}
	if (Hud)
	{
		Hud->IntroAlpha = JamIntroRules::TitleAlpha(Age, IntroLength, IntroTitleFade);
	}
	// The simulation is not advanced: the fight waits for the show to end. The hero's spot rides with the camera (T-100).
	const float ToHero = JamIntroRules::CameraToHero(Age, IntroLength, IntroReturn);
	SpotToHero = ToHero;
	SyncViews(Host, Time);
	Rig->Sync(FMath::Lerp(BossView->GetActorLocation(), HeroView->GetActorLocation(), ToHero), Time);
	return true;
}

void AJamGameMode::RunStrideGallery(const UJamFightSubsystem& Host, float Time)
{
	// Move angles off the facing, to the right positive: ahead, the diagonals, the strafes, the backpedal.
	static const float Angles[] = { 0.f, 45.f, 90.f, 135.f, 180.f, -135.f, -90.f, -45.f };
	constexpr float EachSeconds = 1.5f;
	if (StrideGalleryStart < 0.f)
	{
		StrideGalleryStart = Time;
	}
	const float Age = Time - StrideGalleryStart;
	if (Age >= EachSeconds * UE_ARRAY_COUNT(Angles))
	{
		const float CircleDt = FMath::Clamp(Time - StrideGalleryTime, 0.f, 0.1f);
		StrideGalleryTime = Time;
		RunStrideCircle(Host, Time, Age - EachSeconds * UE_ARRAY_COUNT(Angles), CircleDt);
		return;
	}
	const int32 Index = FMath::Min(FMath::FloorToInt32(Age / EachSeconds), static_cast<int32>(UE_ARRAY_COUNT(Angles)) - 1);
	FJamFighter Hero = Host.GetFight().Fighters[EJamSide::Hero];
	const float Dt = StrideGalleryTime < 0.f ? 0.f : FMath::Clamp(Time - StrideGalleryTime, 0.f, 0.1f);
	// Each way starts again from the hero's own place, so the run stays inside the arena.
	if (StrideGalleryTime < 0.f || Index != GalleryPose)
	{
		StrideGalleryPos = Hero.Pos -
			FVector2D(FMath::Cos(FMath::DegreesToRadians(Angles[Index])), FMath::Sin(FMath::DegreesToRadians(Angles[Index]))) * 400.0;
		GalleryPose = Index;
	}
	StrideGalleryTime = Time;
	const float Radians = FMath::DegreesToRadians(Angles[Index]);
	const FVector2D Dir(FMath::Cos(Radians), FMath::Sin(Radians));
	Hero.Facing = FVector2D(1.0, 0.0);
	Hero.PrevPos = StrideGalleryPos;
	StrideGalleryPos += Dir * Hero.Speed * Dt;
	Hero.Pos = Hero.PrevPos + Dir * Hero.Speed / JamSim::StepsPerSecond;
	HeroView->Sync(Hero, 0.f, Time);
	UE_LOG(LogJamGame, Verbose, TEXT("Stride trace %.0f %.3f %s"), Angles[Index], Age, *HeroView->DescribeStride());
	if (FMath::Frac(Age / EachSeconds) < 0.02f)
	{
		UE_LOG(LogJamGame, Display, TEXT("Stride gallery: moving %.0f degrees off the facing"), Angles[Index]);
	}
	// The camera in front of the hero (or above him with -JamStrideTop): the chest faces it, the feet step sideways.
	const FVector Feet = HeroView->GetActorLocation();
	Rig->Frame(
		Feet + (FParse::Param(FCommandLine::Get(), TEXT("JamStrideTop")) ? FVector(-60.0, -60.0, 560.0) : FVector(420.0, -70.0, 190.0)),
		Feet + FVector(0.0, 0.0, 85.0), 50.f);
}

void AJamGameMode::RunStrideCircle(const UJamFightSubsystem& Host, float Time, float Age, float Dt)
{
	constexpr float CircleSeconds = 7.f;
	constexpr float BackSeconds = 1.2f;
	constexpr float InSeconds = 1.2f;
	constexpr double Radius = 380.0;
	const FJamFight& Fight = Host.GetFight();
	FJamFighter Hero = Fight.Fighters[EJamSide::Hero];
	FJamFighter Boss = Fight.Fighters[EJamSide::Boss];
	if (Age < CircleSeconds + BackSeconds + InSeconds)
	{
		// The hero circles the Publisher, then backs straight off, then runs back in; he faces him all along.
		if (StrideCircleAngle > 1.0e9)
		{
			StrideCircleAngle = 0.0;
			StrideCircleRadius = Radius;
		}
		const bool bCircling = Age < CircleSeconds;
		const bool bBacking = !bCircling && Age < CircleSeconds + BackSeconds;
		const double Step = Hero.Speed * Dt;
		StrideCircleAngle += bCircling ? Step / StrideCircleRadius : 0.0;
		StrideCircleRadius += bBacking ? Step : (bCircling ? 0.0 : -Step);
		const FVector2D Out(FMath::Cos(StrideCircleAngle), FMath::Sin(StrideCircleAngle));
		const FVector2D Tangent(-Out.Y, Out.X);
		const FVector2D Move = bCircling ? Tangent : (bBacking ? Out : -Out);
		Hero.PrevPos = Boss.Pos + Out * StrideCircleRadius;
		Hero.Pos = Hero.PrevPos + Move * Hero.Speed / JamSim::StepsPerSecond;
		Hero.Facing = -Out;
		Boss.PrevPos = Boss.Pos;
		HeroView->Sync(Hero, 0.f, Time);
		BossView->Sync(Boss, 0.f, Time);
		const FVector Between = 0.5 * (HeroView->GetActorLocation() + BossView->GetActorLocation());
		Rig->Frame(Between + FVector(-850.0, 0.0, 800.0), Between + FVector(0.0, 0.0, 80.0), 50.f);
		return;
	}
	// The Publisher walks east at his own speed, the hero stands by.
	const float Walked = Age - (CircleSeconds + BackSeconds + InSeconds);
	Boss.PrevPos = Boss.Pos + FVector2D(Boss.Speed * Walked - 600.0, 0.0);
	Boss.Pos = Boss.PrevPos + FVector2D(Boss.Speed / JamSim::StepsPerSecond, 0.0);
	Boss.Facing = FVector2D(1.0, 0.0);
	Hero.PrevPos = Hero.Pos;
	HeroView->Sync(Hero, 0.f, Time);
	BossView->Sync(Boss, 0.f, Time);
	const FVector Feet = BossView->GetActorLocation();
	Rig->Frame(Feet + FVector(150.0, -620.0, 260.0), Feet + FVector(0.0, 0.0, 110.0), 50.f);
}

void AJamGameMode::RunGallery(const UJamFightSubsystem& Host, float Time)
{
	struct FGalleryPose
	{
		EJamClip Clip;
		float ClipTime;
	};
	// Every pose the katana and the hands have to look right in: idle, run, both strikes, the guard, the throw, a hit.
	static const FGalleryPose Poses[] = {
		{ EJamClip::Idle, 0.5f },
		{ EJamClip::Idle, 2.5f },
		{ EJamClip::Move, 0.1f },
		{ EJamClip::Move, 0.4f },
		{ EJamClip::StrikeA, 0.35f },
		{ EJamClip::StrikeA, 0.5f },
		{ EJamClip::StrikeB, 0.73f },
		{ EJamClip::Parry, 0.3f },
		{ EJamClip::Throw, 0.4f },
		{ EJamClip::Throw, 0.57f },
		{ EJamClip::Hit, 0.33f },
		{ EJamClip::Move, 0.25f },
	};
	constexpr float PoseSeconds = 0.5f;
	const int32 Index = FMath::Min(FMath::FloorToInt32(Time / PoseSeconds), static_cast<int32>(UE_ARRAY_COUNT(Poses)) - 1);
	if (Index != GalleryPose)
	{
		GalleryPose = Index;
		HeroView->HoldPose(Poses[Index].Clip, Poses[Index].ClipTime, Time);
	}
	SyncViews(Host, Time);
	if (FMath::Frac(Time / PoseSeconds) > 0.45f && FMath::Frac(Time / PoseSeconds) < 0.5f)
	{
		UE_LOG(LogJamGame, Display, TEXT("Gallery pose %d: %s"), GalleryPose, *HeroView->DescribeHand());
	}
	// The camera stands at the hero's right front, then at his left front for the second half of each pose.
	const FVector Body = HeroView->GetActorLocation() + FVector(0.0, 0.0, 110.0);
	const bool bRightSide = FMath::Frac(Time / PoseSeconds) < 0.5f;
	const FVector Forward = HeroView->GetActorForwardVector();
	const FVector Side = HeroView->GetActorRightVector() * (bRightSide ? 1.0 : -1.0);
	Rig->Frame(Body + Forward * 220.0 + Side * 160.0 + FVector(0.0, 0.0, 60.0), Body, 45.f);
}
