#include "View/JamFighterView.h"

#include "JamGame.h"

#include "Animation/AnimSequence.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "View/JamPoseInstance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "View/JamShapes.h"

namespace
{
// Proportions of the bean, in body radii: presentation only.
constexpr float HeightInRadii = 4.5f;
constexpr float VisorHeightAt = 0.78f;
const FVector VisorSizeInRadii(0.5, 1.3, 0.5);
constexpr float VisorForwardInRadii = 0.75f;
const FLinearColor VisorColor(0.02f, 0.02f, 0.03f);

/** One clip of a character: the asset, where its blow lands and where the action ends, in clip seconds. */
struct FClipInfo
{
	const TCHAR* Asset = nullptr;
	float Impact = 0.f;
	/** 0: the end of the clip. */
	float End = 0.f;
};

/** A character: its model, its clips by EJamClip, how fast its walk moves at the clip's own speed. */
struct FFigureInfo
{
	const TCHAR* Mesh = nullptr;
	/** Its M_JamCharacter instance (Tools/Editor/create_character_materials.py). */
	const TCHAR* Material = nullptr;
	FClipInfo Clips[static_cast<int32>(EJamClip::Throw) + 1];
	/** cm per second the walk or run covers when played at rate 1. */
	float MoveSpeed = 1.f;
	float MaxMoveRate = 1.f;
	/** The diagonal runs by EJamStride ForwardLeft to BackRight; without them only the run forwards and backwards. */
	FJamStrideClip Diagonals[4];
	/** How much of the walk the upper body shows (the rest is the idle's): a sped-up walk's sway calmed down. */
	float UpperWalk = 1.f;
	/** Degrees of yaw the model turns while its Entrance clip plays (T-090); negative turns it counter-clockwise. */
	float EntranceTurn = 0.f;
};

// Impact frames measured with Tools/Art/clip_impacts.py (the frame a hand moves fastest), ends chosen by eye.
const FFigureInfo HeroFigure{
	TEXT("/Game/Characters/Hero/SK_Hero.SK_Hero"),
	TEXT("/Game/Characters/Materials/MI_Hero.MI_Hero"),
	{
		{},
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Idle.A_Hero_Idle") },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Run.A_Hero_Run") },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Right_Hand_Sword_Slash.A_Hero_Right_Hand_Sword_Slash"), 0.5f, 1.f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Left_Slash.A_Hero_Left_Slash"), 0.73f, 1.3f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Sword_Parry.A_Hero_Sword_Parry"), 0.3f, 0.9f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Hit_Reaction.A_Hero_Hit_Reaction"), 0.33f, 1.2f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Dead.A_Hero_Dead") },
		{},
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Right_Hand_Sword_Slash.A_Hero_Right_Hand_Sword_Slash"), 0.5f, 1.f },
		// T-089: his slow draw as the intro's camera comes to him (Meshy's Sword_Shout opening, hero_export.py).
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Draw.A_Hero_Draw") },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Throw_L.A_Hero_Throw_L"), 0.57f, 1.f },
	},
	// T-078: feet speeds from the planted foot in Blender, checked in -JamStrideGallery at his 600 cm/s.
	420.f,
	// The back diagonals cover 170 cm/s at rate 1: at his full 600 they play 3.5 times as fast to keep up.
	3.6f,
	// The Meshy library runs 9, 10 (forward diagonals, from a fighting run) and 5, 6 (back diagonals, a backpedal).
	{
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Run_ForwardLeft.A_Hero_Run_ForwardLeft"), 530.f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Run_ForwardRight.A_Hero_Run_ForwardRight"), 530.f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Run_BackLeft.A_Hero_Run_BackLeft"), 170.f },
		{ TEXT("/Game/Characters/Hero/Anims/A_Hero_Run_BackRight.A_Hero_Run_BackRight"), 170.f },
	},
};

const FFigureInfo PublisherFigure{
	TEXT("/Game/Characters/Publisher/SK_Publisher.SK_Publisher"),
	TEXT("/Game/Characters/Materials/MI_Publisher.MI_Publisher"),
	{
		{},
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Idle.A_Publisher_Idle") },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Slow_Orc_Walk.A_Publisher_Slow_Orc_Walk") },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Heavy_Hammer_Swing.A_Publisher_Heavy_Hammer_Swing"), 1.55f },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Heavy_Hammer_Swing.A_Publisher_Heavy_Hammer_Swing"), 1.55f },
		{},
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Hit_Reaction.A_Publisher_Hit_Reaction"), 0.37f },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Knock_Down.A_Publisher_Knock_Down") },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Charged_Ground_Slam.A_Publisher_Charged_Ground_Slam"), 1.85f },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Heavy_Hammer_Swing.A_Publisher_Heavy_Hammer_Swing"), 1.55f },
		{ TEXT("/Game/Characters/Publisher/Anims/A_Publisher_Kung_Fu_Punch.A_Publisher_Kung_Fu_Punch") },
		{},
	},
	// boss_fixed/README.md: the walk covers 43 cm/s, he moves at 260. T-079: it plays as fast as his feet need, the
	// upper body takes a third of its sway.
	43.f,
	6.5f,
	{},
	0.35f,
	// T-090: Kung_Fu_Punch ends turned clockwise off his idle (seen from above: head 98, hips 139, chest 156 degrees,
	// measured in boss_fixed in Blender); 120 counter-clockwise ends it facing the hero, the head a little to his left.
	// The author's play-through: 10 degrees back clockwise.
	-110.f,
};

const TCHAR* const TrailSystem = TEXT("/Game/FX/NS_JamTrail.NS_JamTrail");
const TCHAR* const KatanaMesh = TEXT("/Game/Props/Katana/model_glb/StaticMeshes/SM_Katana.SM_Katana");
const FName HandSocket = TEXT("hand_r");
const FName HeroThrowHand = TEXT("hand_l");
const FName BossThrowHand = TEXT("hand_r");
// The blade in the hero's right hand (T-065), worked out from the bone in the pose gallery: hand_r's +Y runs back to
// the elbow, so the fingers lie along -Y; the blade (the model's -X) points along -Z, where the thumb is when the arm
// hangs; the grip's middle (18 cm along the model's +X at this scale) sits 8 cm down the fingers, in the fist.
const FTransform KatanaInHand(FRotator(90.0, 0.0, -90.0), FVector(0.0, -8.0, -17.9), FVector(0.85));

// The models face +Y; the fight's facing is +X.
const FRotator ModelYaw(0.0, -90.0, 0.0);

// How actions play: the fastest and slowest a clip may be played to land on time, the fade in and out, the shortest
// wind-up shown before a blow that has already left the hand (his throws).
constexpr float MinClipRate = 0.75f;
constexpr float MaxClipRate = 2.5f;
constexpr float BlendSeconds = 0.12f;
constexpr float ShortestLead = 0.15f;
// The Publisher staggers only under hits of this much and more.
constexpr float StaggerHit = 25.f;
// The hit lean (T-064): a snap back over this long, an ease back until the end.
constexpr float LeanRise = 0.05f;
constexpr float LeanLife = 0.35f;
// The hero's afterimages: how many, how far apart they are left, how long each fades, their neon.
constexpr int32 GhostCount = 4;
constexpr float GhostGap = 0.05f;
constexpr float GhostLife = 0.3f;
const FLinearColor GhostCyan(0.25f, 0.95f, 1.0f);
const FLinearColor GhostMagenta(1.0f, 0.25f, 0.75f);
const TCHAR* const GhostMaterial = TEXT("/Game/Characters/Materials/M_JamGhost.M_JamGhost");
// How fast the walk fades in and out, per second.
constexpr float MoveBlendRate = 8.f;
// The lean into a turn: the whole of it at this many degrees per second, eased in and out at this speed.
constexpr float TurnLeanFullAt = 400.f;
constexpr float TurnLeanMax = 7.f;
constexpr float TurnLeanEase = 6.f;

// How a hit reads on the body: a short white flash.
constexpr float FlashSeconds = 0.09f;

const FFigureInfo& FigureOf(EJamSide Side)
{
	return Side == EJamSide::Hero ? HeroFigure : PublisherFigure;
}

const FClipInfo& InfoOf(EJamSide Side, EJamClip Clip)
{
	return FigureOf(Side).Clips[static_cast<int32>(Clip)];
}
} // namespace

AJamFighterView::AJamFighterView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	JamShapes::Setup(*Body, JamShapes::Sphere());
	Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visor"));
	Visor->SetupAttachment(RootComponent);
	JamShapes::Setup(*Visor, JamShapes::Cube());

	ConstructRonin();

	Figure = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Figure"));
	Figure->SetupAttachment(RootComponent);
	Figure->SetRelativeRotation(ModelYaw);
	Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Figure->SetGenerateOverlapEvents(false);
	Figure->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	Katana = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Katana"));
	Katana->SetupAttachment(Figure, HandSocket);
	Katana->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Katana->SetGenerateOverlapEvents(false);
	Katana->SetRelativeTransform(KatanaInHand);

	Trail = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	Trail->SetupAttachment(Katana);
	// The blade's tip: the model's -X end, in the katana's own (unscaled) units.
	Trail->SetRelativeLocation(FVector(-80.0, 0.0, 0.0));
	Trail->SetAutoActivate(false);

	for (int32 Index = 0; Index < GhostCount; ++Index)
	{
		UPoseableMeshComponent* Ghost = CreateDefaultSubobject<UPoseableMeshComponent>(FName(TEXT("Ghost"), Index));
		Ghost->SetupAttachment(RootComponent);
		// A ghost stays where it was left while the hero moves on.
		Ghost->SetUsingAbsoluteLocation(true);
		Ghost->SetUsingAbsoluteRotation(true);
		Ghost->SetUsingAbsoluteScale(true);
		Ghost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Ghost->SetCastShadow(false);
		Ghost->SetVisibility(false);
		Ghosts.Add(Ghost);
	}

	// The grid is found here, in a constructor: the layers switch to it at runtime.
	JamShapes::Grid();
}

void AJamFighterView::Init(EJamSide InSide, float Radius, float MaxSpeed, const FLinearColor& Color)
{
	BodyRadius = Radius;
	Side = InSide;
	const float Height = Radius * HeightInRadii;
	BaseColor = Color;
	Speed = FMath::Max(1.f, MaxSpeed);
	const float R = Radius / JamShapes::UnitSize;
	Body->SetRelativeScale3D(FVector(2.f * Radius, 2.f * Radius, Height) / JamShapes::UnitSize);
	Body->SetRelativeLocation(FVector(0.0, 0.0, 0.5 * Height));
	Visor->SetRelativeScale3D(VisorSizeInRadii * R);
	Visor->SetRelativeLocation(FVector(Radius * VisorForwardInRadii, 0.0, Height * VisorHeightAt));

	InitRonin(Radius);

	const FFigureInfo& Info = FigureOf(Side);
	Figure->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, Info.Mesh));
	Figure->SetAnimInstanceClass(UJamPoseInstance::StaticClass());
	Clips.SetNum(UE_ARRAY_COUNT(Info.Clips));
	for (int32 Index = 0; Index < Clips.Num(); ++Index)
	{
		Clips[Index] = Info.Clips[Index].Asset ? LoadObject<UAnimSequence>(nullptr, Info.Clips[Index].Asset) : nullptr;
	}
	InitRuns(Info.Diagonals, Info.MoveSpeed, Info.MaxMoveRate, Info.UpperWalk);
	Skin = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, Info.Material), this);
	if (Skin)
	{
		Skin->GetScalarParameterValue(TEXT("NeonGlow"), NeonGlow);
	}
	GhostBornAt.Init(-UE_BIG_NUMBER, Ghosts.Num());
	if (Side == EJamSide::Hero)
	{
		InitHeroGear();
	}

	// Forces the first SetLayers to apply everything.
	Shown.bModel = !Shown.bModel;
	SetLayers(FJamLayers());
}

void AJamFighterView::InitHeroGear()
{
	if (UNiagaraSystem* Ribbon = LoadObject<UNiagaraSystem>(nullptr, TrailSystem))
	{
		Trail->SetAsset(Ribbon);
		Trail->SetVariableLinearColor(TEXT("User.Color"), FLinearColor(0.3f, 0.95f, 1.0f));
	}
	Katana->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, KatanaMesh));
#if !UE_BUILD_SHIPPING
	// -JamKatana=X,Y,Z,Pitch,Yaw,Roll: the blade's place in the hand, tried out without a build (the pose gallery).
	FString Grip;
	TArray<FString> Parts;
	if (FParse::Value(FCommandLine::Get(), TEXT("JamKatana="), Grip, false) && Grip.ParseIntoArray(Parts, TEXT(",")) == 6)
	{
		Katana->SetRelativeLocationAndRotation(FVector(FCString::Atod(*Parts[0]), FCString::Atod(*Parts[1]), FCString::Atod(*Parts[2])),
			FRotator(FCString::Atod(*Parts[3]), FCString::Atod(*Parts[4]), FCString::Atod(*Parts[5])));
	}
#endif
	UMaterialInterface* GhostLook = LoadObject<UMaterialInterface>(nullptr, GhostMaterial);
	for (int32 Index = 0; Index < Ghosts.Num(); ++Index)
	{
		Ghosts[Index]->SetSkinnedAssetAndUpdate(Figure->GetSkinnedAsset());
		UMaterialInstanceDynamic* GhostSkin = UMaterialInstanceDynamic::Create(GhostLook, this);
		GhostSkin->SetVectorParameterValue(TEXT("Color"), Index % 2 == 0 ? GhostCyan : GhostMagenta);
		Ghosts[Index]->SetMaterial(0, GhostSkin);
		GhostSkins.Add(GhostSkin);
	}
}

void AJamFighterView::SetLayers(const FJamLayers& InLayers)
{
	if (InLayers == Shown)
	{
		return;
	}
	Shown = InLayers;
	// Three looks of the hero (T-067): the 3D art, the shape ronin of before the models, the bean.
	Figure->SetVisibility(Shown.bModel && Shown.bArt, true);
	Ronin->SetVisibility(Shown.bModel && !Shown.bArt, true);
	Body->SetVisibility(!Shown.bModel);
	Visor->SetVisibility(!Shown.bModel);
	if (Skin)
	{
		Skin->SetScalarParameterValue(TEXT("NeonGlow"), Shown.bEffects ? NeonGlow : 0.f);
	}
	Repaint();
}

void AJamFighterView::Repaint()
{
	const bool bGrid = !Shown.bMaterials && !bFlashing;
	const FLinearColor Flashed = bFlashing ? FMath::Lerp(BaseColor, FLinearColor::White, FlashWeight) : BaseColor;
	if (!Shown.bModel)
	{
		JamShapes::Look(*Body, Flashed, bGrid);
		JamShapes::Look(*Visor, VisorColor, bGrid);
		return;
	}
	if (!Shown.bArt)
	{
		PaintRonin(bGrid);
		return;
	}
	UMaterialInterface* Grid = JamShapes::Grid();
	Figure->SetMaterial(0, bGrid ? Grid : Skin.Get());
	if (Katana->GetStaticMesh())
	{
		Katana->SetMaterial(0, bGrid ? Grid : Katana->GetStaticMesh()->GetMaterial(0));
	}
	if (Skin)
	{
		Skin->SetScalarParameterValue(TEXT("Flash"), bFlashing ? FlashWeight : 0.f);
	}
}

void AJamFighterView::Flash(float Time, float Weight)
{
	// A light hit inside the flash of a heavy one does not dim it.
	const bool bStillFlashing = Time < FlashUntil;
	const float Next = bStillFlashing ? FMath::Max(FlashWeight, Weight) : Weight;
	FlashUntil = Time + FlashSeconds;
	if (Next != FlashWeight)
	{
		FlashWeight = Next;
		// Sync repaints only when the flash starts or ends: a new weight inside one is painted here.
		if (bFlashing)
		{
			Repaint();
		}
	}
}

void AJamFighterView::Start(EJamClip Clip, float Lead, bool bFullBody, float Time)
{
	const UAnimSequence* Sequence = Clips.IsValidIndex(static_cast<int32>(Clip)) ? Clips[static_cast<int32>(Clip)].Get() : nullptr;
	if (!Sequence)
	{
		return;
	}
	const FClipInfo& Info = InfoOf(Side, Clip);
	// The action cut short fades out under the new one (T-066): a chain of strikes flows instead of popping.
	if (Action.Clip != EJamClip::None)
	{
		FadingWeight = WeightOf(Action, Time);
		FadingAt = Time;
		Fading = Action;
	}
	Action.Clip = Clip;
	Action.StartedAt = Time;
	Action.bFullBody = bFullBody;
	Action.bHold = Clip == EJamClip::Death;
	Action.End = Info.End > 0.f ? Info.End : Sequence->GetPlayLength();
	// A held clip and the show-off play whole at their own speed: nothing in the fight lands with them.
	const bool bWhole = Action.bHold || Clip == EJamClip::Entrance;
	// A throw has left the hand already: it shows from the release on, so the shot is drawn leaving that hand.
	const float MinLead = Clip == EJamClip::Throw ? 0.f : ShortestLead;
	Action.Playback = bWhole ? FJamPlayback() : JamPoseRules::Fit(Info.Impact, FMath::Max(Lead, MinLead), MinClipRate, MaxClipRate);
}

void AJamFighterView::HoldPose(EJamClip Clip, float ClipTime, float Time)
{
	Start(Clip, 0.f, true, Time);
	Action.bHold = true;
	Action.End = UE_BIG_NUMBER;
	Action.Playback.From = ClipTime;
	Action.Playback.Rate = 0.f;
}

FString AJamFighterView::DescribeHand() const
{
	const FTransform Hand = Figure->GetSocketTransform(HandSocket);
	const FVector Tip = Figure->GetSocketLocation(TEXT("RightHand_End"));
	const FVector Elbow = Figure->GetSocketLocation(TEXT("lowerarm_r"));
	return FString::Printf(TEXT("hand %s X %s Y %s Z %s tip %s elbow %s forward %s"), *Hand.GetLocation().ToString(),
		*Hand.GetUnitAxis(EAxis::X).ToString(), *Hand.GetUnitAxis(EAxis::Y).ToString(), *Hand.GetUnitAxis(EAxis::Z).ToString(),
		*Tip.ToString(), *Elbow.ToString(), *GetActorForwardVector().ToString());
}

FVector AJamFighterView::ThrowingHand() const
{
	if (!Shown.bModel || !Figure->GetSkinnedAsset())
	{
		return FVector::ZeroVector;
	}
	return Figure->GetSocketLocation(Side == EJamSide::Hero ? HeroThrowHand : BossThrowHand);
}

void AJamFighterView::Perform(EJamClip Clip, float Time)
{
	Start(Clip, 0.f, true, Time);
}

float AJamFighterView::LengthOf(EJamClip Clip) const
{
	const int32 Index = static_cast<int32>(Clip);
	return Clips.IsValidIndex(Index) && Clips[Index] ? Clips[Index]->GetPlayLength() : 0.f;
}

void AJamFighterView::Interrupt(float Time)
{
	if (Action.Clip == EJamClip::None || Action.bHold)
	{
		return;
	}
	// The clip ends one fade from now, wherever it is.
	const float Age = Time - Action.StartedAt;
	Action.End = FMath::Min(Action.End, Action.Playback.From + (Age + BlendSeconds) * Action.Playback.Rate);
}

void AJamFighterView::Act(const FJamFight& Fight, TConstArrayView<FJamEvent> Events, float Time)
{
	if (JamPoseRules::IsDown(Fight, Side))
	{
		if (Action.Clip != EJamClip::Death)
		{
			Start(EJamClip::Death, 0.f, true, Time);
		}
		return;
	}
	if (Action.Clip == EJamClip::Death)
	{
		// The fight started again: he is up at once.
		Action = FAction();
	}
	if (JamPoseRules::Shakes(Fight, Side, Events, StaggerHit))
	{
		HitAt = Time;
		GhostAt = Time;
		GhostsLeft = 0;
		UE_LOG(LogJamGame, Verbose, TEXT("%s shaken by a hit at %.2f s"), Side == EJamSide::Hero ? TEXT("Hero") : TEXT("Publisher"), Time);
	}
	// A dash of the hero's leaves its afterimages too (T-066): the Sandevistan trail of the hit, without the lean.
	if (Events.ContainsByPredicate([this](const FJamEvent& E) { return E.Type == EJamEvent::Dash && E.Side == Side; }))
	{
		GhostAt = Time;
		GhostsLeft = 0;
	}
	const FJamCue Cue = JamPoseRules::CueFor(Fight, Side, Events);
	if (Cue.Clip != EJamClip::None && JamPoseRules::Replaces(Cue.Clip, Action.Clip))
	{
		Start(Cue.Clip, Cue.Lead, Cue.bFullBody, Time);
	}
}

float AJamFighterView::ClipTimeOf(const FAction& Of, float Time) const
{
	return FMath::Min(Of.Playback.From + (Time - Of.StartedAt) * Of.Playback.Rate, Of.End);
}

void AJamFighterView::TurnForShowOff(float Time)
{
	const float Weight = Action.Clip == EJamClip::Entrance ? WeightOf(Action, Time) : 0.f;
	const float Turn = JamPoseRules::ShowOffTurn(FigureOf(Side).EntranceTurn, Weight);
	if (Turn != ModelTurn)
	{
		ModelTurn = Turn;
		Figure->SetRelativeRotation(ModelYaw + FRotator(0.0, Turn, 0.0));
	}
}

float AJamFighterView::WeightOf(const FAction& Of, float Time) const
{
	const float Age = Time - Of.StartedAt;
	const float Remaining = Of.bHold || Of.Playback.Rate <= 0.f ? UE_BIG_NUMBER : (Of.End - Of.Playback.From) / Of.Playback.Rate - Age;
	return JamPoseRules::Weight(Age, Remaining, BlendSeconds);
}

void AJamFighterView::ShowFading(FJamPoseFrame& Out, float Time)
{
	const float FadeLeft = Fading.Clip == EJamClip::None ? 0.f : FadingWeight * (1.f - (Time - FadingAt) / BlendSeconds);
	if (FadeLeft <= 0.f)
	{
		Fading = FAction();
	}
	Out.Fading = FadeLeft > 0.f ? Clips[static_cast<int32>(Fading.Clip)].Get() : nullptr;
	Out.FadingTime = FadeLeft > 0.f ? ClipTimeOf(Fading, Time) : 0.f;
	Out.FadingWeight = FMath::Max(0.f, FadeLeft);
	Out.bFadingFullBody = Fading.bFullBody;
}

void AJamFighterView::Animate(float Moving, float MoveAngle, float YawRate, float Time)
{
	const float Dt = FMath::Clamp(Time - LastTime, 0.f, 0.1f);
	LastTime = Time;
	UJamPoseInstance* Pose = Cast<UJamPoseInstance>(Figure->GetAnimInstance());
	if (!Pose)
	{
		return;
	}
	FJamPoseFrame& Out = Pose->Frame;
	Out.bRefPose = !Shown.bAnimation;
	MoveWeight = FMath::FInterpConstantTo(MoveWeight, Moving, Dt, MoveBlendRate);
	AnimateStride(Out, Moving, MoveAngle, Dt);
	TurnLeanShown =
		FMath::FInterpTo(TurnLeanShown, JamPoseRules::TurnLean(YawRate, TurnLeanFullAt, TurnLeanMax) * MoveWeight, Dt, TurnLeanEase);
	Out.TurnLean = TurnLeanShown;
	Out.Idle = Clips[static_cast<int32>(EJamClip::Idle)];
	// T-094: wrapped here: the extraction clamps a time past the clip's end, so the menu's idle froze after one loop.
	Out.IdleTime = JamPoseRules::LoopTime(Time, Out.Idle ? Out.Idle->GetPlayLength() : 0.f);
	Out.MoveWeight = MoveWeight;
	Out.Lean = Clips[static_cast<int32>(EJamClip::Hit)];
	Out.LeanTime = InfoOf(Side, EJamClip::Hit).Impact;
	Out.LeanWeight = HitAt < 0.f ? 0.f : JamPoseRules::LeanWeight(Time - HitAt, LeanRise, LeanLife);

	ShowFading(Out, Time);
	TurnForShowOff(Time);

	Out.Action = nullptr;
	Out.ActionWeight = 0.f;
	if (Action.Clip == EJamClip::None)
	{
		return;
	}
	const float Age = Time - Action.StartedAt;
	const float ClipTime = ClipTimeOf(Action, Time);
	const float Weight = WeightOf(Action, Time);
	if (Weight <= 0.f && Age > BlendSeconds)
	{
		Action = FAction();
		return;
	}
	// The blade leaves its ribbon while it swings, in the full art only (T-069).
	const bool bSwinging = Action.Clip == EJamClip::StrikeA || Action.Clip == EJamClip::StrikeB || Action.Clip == EJamClip::Heavy;
	SetTrail(Shown.bArt && bSwinging && Weight > 0.2f);
	Out.Action = Clips[static_cast<int32>(Action.Clip)];
	Out.ActionTime = ClipTime;
	Out.ActionWeight = Weight;
	Out.bFullBody = Action.bFullBody;
}

void AJamFighterView::SetTrail(bool bOn)
{
	if (bOn != bTrailOn && Trail->GetAsset())
	{
		bTrailOn = bOn;
		bOn ? Trail->Activate(true) : Trail->Deactivate();
	}
}

void AJamFighterView::UpdateGhosts(float Time)
{
	if (GhostSkins.IsEmpty())
	{
		return;
	}
	// The afterimages are an effect of a moving body: gone with the effects, the animation or the model.
	const bool bShow = Shown.bArt && Shown.bEffects && Shown.bAnimation && Shown.bModel;
	const int32 Due = bShow && GhostAt >= 0.f ? JamPoseRules::GhostsBy(Time - GhostAt, Ghosts.Num(), GhostGap) : 0;
	for (; GhostsLeft < Due; ++GhostsLeft)
	{
		UPoseableMeshComponent& Ghost = *Ghosts[GhostsLeft];
		Ghost.SetWorldTransform(Figure->GetComponentTransform());
		Ghost.CopyPoseFromSkeletalComponent(Figure);
		GhostBornAt[GhostsLeft] = Time;
	}
	for (int32 Index = 0; Index < Ghosts.Num(); ++Index)
	{
		const float Alpha = bShow ? JamPoseRules::GhostAlpha(Time - GhostBornAt[Index], GhostLife) : 0.f;
		Ghosts[Index]->SetVisibility(Alpha > 0.f);
		GhostSkins[Index]->SetScalarParameterValue(TEXT("Fade"), Alpha);
	}
}
