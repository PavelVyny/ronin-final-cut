#include "View/JamEffectsView.h"

#include "Components/StaticMeshComponent.h"
#include "Sim/JamHazardRules.h"
#include "Sim/JamParryRules.h"
#include "View/JamShapes.h"

namespace
{
// Greybox look of the effects: presentation only. Lives are in seconds, sizes in cm.
constexpr float SwingLife = 0.16f;
constexpr float ExplosionLife = 0.2f;
constexpr float SlamLife = 0.25f;
constexpr float TrailLife = 0.22f;
constexpr float DeflectLife = 0.15f;
constexpr float UltimateLife = 0.35f;
constexpr float UltimateHeight = 60.f;
constexpr float ChargeRingScale = 2.5f;
// One tier down (T-072): the dash leaves a thin streak.
constexpr float DashStreakWidth = 0.3f;
// A deflection flashes where it happened; the open parry window is a shield that closes with it (JamFeedbackRules).
constexpr float DeflectRadius = 60.f;
constexpr float SparkLife = 0.14f;
// The dashed blast ring under an explosive shuriken of the Publisher's.
constexpr int32 RingDashes = 14;
constexpr float RingDashFill = 0.55f;
constexpr float RingDashWidth = 12.f;
constexpr float RingTurnsPerSecond = 0.5f;
// The parry shield: standing panels round the body, as tall as the ronin, gaps between them.
constexpr int32 ShieldPanels = 12;
constexpr float ShieldPanelFill = 0.7f;
constexpr float ShieldHeight = 150.f;
constexpr float ShieldThickness = 4.f;
// The ring round the shot that "LMB PARRY" points at, in shot radii.
constexpr float ParryCueScale = 3.f;
// The ring of a chain's tell (T-056), in body radii.
constexpr float ComboTellScale = 1.8f;
// The cash-stack heal (T-049): a fat green stack with a gold band that floats, bobs and turns over a pulsing ring, the
// only thing in the game that does; it blinks in its last seconds. Sizes in cm, times in seconds.
// SM_CashStack (Hunyuan3D; T-100 took it back from Rodin's) is 102 cm long with its pivot under it: shown at 90 cm.
constexpr float CashScale = 0.9f;
const FVector CashModelCentre(-2.1, -0.5, 22.1);
constexpr float CashFloat = 80.f;
constexpr float CashBob = 12.f;
constexpr float CashBobHz = 1.2f;
constexpr float CashTurnsPerSecond = 0.4f;
constexpr float CashArc = 160.f;
constexpr float CashRingRadius = 80.f;
constexpr float CashRingPulse = 15.f;
constexpr float CashRingHz = 2.f;
constexpr float CashBlinkSeconds = 2.f;
constexpr float CashBlinkHz = 4.f;
// The heal taken: green rings rise round the hero.
constexpr float HealLife = 0.6f;
constexpr float HealRise = 170.f;
const FLinearColor CashColor(0.1f, 1.0f, 0.25f);
const FLinearColor HealColor(0.2f, 1.0f, 0.4f);
// The Ultimate lane: the width of its edges in cm, the share of the colour the idle guide and the fill shine with.
constexpr float LaneEdgeWidth = 10.f;
constexpr float LaneIdleShare = 0.35f;
constexpr float LaneFillShare = 0.45f;
constexpr float BladeHeight = 110.f;
constexpr float BladeWidth = 16.f;
constexpr float BladeThickness = 6.f;
constexpr float FinisherScale = 1.25f;
constexpr float GroundThickness = 2.f;
// Flat shapes lie in layers a little above the floor so they do not flicker through each other.
constexpr float CircleHeight = 3.f;
constexpr float FillHeight = 5.f;
constexpr float FlashHeight = 7.f;
constexpr float TrailHeight = 4.f;
constexpr float ExplosionStartSize = 0.4f;
const FLinearColor CircleColor(0.22f, 0.02f, 0.03f);
const FLinearColor FillColor(1.0f, 0.12f, 0.1f);
const FLinearColor SlamColor(1.0f, 0.85f, 0.7f);
const FLinearColor ExplosionColor(1.0f, 0.55f, 0.08f);
const FLinearColor HeroBladeColor(0.75f, 1.0f, 1.0f);
// The blade of the unfinished game: the grey of the engine's grid, no glow.
const FLinearColor BareBladeColor(0.42f, 0.42f, 0.45f);
const FLinearColor BossBladeColor(1.0f, 0.45f, 0.3f);
const FLinearColor FinisherColor(1.0f, 0.3f, 0.85f);
const FLinearColor HeroTrailColor(0.05f, 0.35f, 0.5f);
const FLinearColor BossTrailColor(0.4f, 0.06f, 0.06f);
const FLinearColor ParryColor(0.9f, 2.5f, 2.5f);
const FLinearColor BlastRingColor(1.0f, 0.9f, 0.05f);
const FLinearColor HeroSparkColor(1.0f, 0.15f, 0.1f);
const FLinearColor BossSparkColor(1.0f, 1.0f, 0.75f);
const FLinearColor DeflectColor(1.0f, 1.0f, 1.0f);
const FLinearColor UltimateColor(1.0f, 0.3f, 0.85f);

/** The stage was cleared by this event: whatever hangs in the air belongs to the fight before it. */
bool ClearsTheAir(EJamEvent Type)
{
	return Type == EJamEvent::StealOpened || Type == EJamEvent::Stunned;
}
} // namespace

float AJamEffectsView::LifeOf(const FJamEvent& Event) const
{
	switch (Event.Type)
	{
		case EJamEvent::Swing:
			return Event.Side == EJamSide::Boss ? BossSwingLife : SwingLife;
		case EJamEvent::Explosion:
			return ExplosionLife;
		case EJamEvent::Slam:
			return SlamLife;
		case EJamEvent::Dash:
			return TrailLife;
		case EJamEvent::Deflected:
			return DeflectLife;
		case EJamEvent::UltimateSlash:
			return UltimateLife;
		case EJamEvent::Hit:
			return SparkLife;
		case EJamEvent::HealTaken:
			return HealLife;
		default:
			return 0.f;
	}
}

AJamEffectsView::AJamEffectsView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	// The pool is filled at runtime: the shapes have to be found here, in a constructor.
	JamShapes::Cylinder();
	JamShapes::Cube();
	JamShapes::Neon();
	JamShapes::CashStack();
}

void AJamEffectsView::Init(const FJamBalanceData& InBalance, const FJamFeedbackSettings& InFeedback)
{
	Balance = InBalance;
	Feedback = InFeedback;
	LoadRunes();
	ExplosionRadius = Balance.ShurikenExplosionRadius;
	ParryRaiseTicks = JamSim::Ticks(Balance.BossParryRaise);
	KatanaRange = Balance.KatanaRange;
	KatanaHalfArc = Balance.KatanaHalfArc;
	BossSwingLife = Balance.BossKatanaWindUp;
	UltimateRange = Balance.UltimateRange;
	UltimateHalfWidth = Balance.UltimateHalfWidth;
}

UStaticMeshComponent& AJamEffectsView::Take(UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (Used == Pool.Num())
	{
		UStaticMeshComponent* Shape = NewObject<UStaticMeshComponent>(this);
		JamShapes::Setup(*Shape, Mesh);
		Shape->SetupAttachment(RootComponent);
		Shape->RegisterComponent();
		Pool.Add(Shape);
	}
	UStaticMeshComponent& Shape = *Pool[Used++];
	if (Shape.GetStaticMesh() != Mesh)
	{
		Shape.SetStaticMesh(Mesh);
	}
	Shape.SetVisibility(true);
	JamShapes::Glow(Shape, Color, EffectGlow);
	return Shape;
}

void AJamEffectsView::DrawDisc(const FVector2D& Pos, float Radius, float Height, const FLinearColor& Color)
{
	UStaticMeshComponent& Disc = Take(JamShapes::Cylinder(), Color);
	Disc.SetWorldLocationAndRotation(FVector(Pos.X, Pos.Y, Height), FRotator::ZeroRotator);
	Disc.SetWorldScale3D(FVector(2.f * Radius, 2.f * Radius, GroundThickness) / JamShapes::UnitSize);
}

void AJamEffectsView::DrawShield(const FVector2D& Pos, float Radius, float Rise)
{
	const float PanelWidth = 2.f * PI * Radius / ShieldPanels * ShieldPanelFill;
	const float Height = ShieldHeight * FMath::Clamp(Rise, 0.05f, 1.f);
	for (int32 Index = 0; Index < ShieldPanels; ++Index)
	{
		const double Angle = 360.0 * Index / ShieldPanels;
		const FVector2D Where = Pos + FVector2D(1.0, 0.0).GetRotated(Angle) * Radius;
		UStaticMeshComponent& Panel = Take(JamShapes::Cube(), ParryColor);
		// Each panel stands across the radius it sits on, so together they close round the body.
		Panel.SetWorldLocationAndRotation(FVector(Where.X, Where.Y, 0.5f * Height), FRotator(0.0, Angle + 90.0, 0.0));
		Panel.SetWorldScale3D(FVector(PanelWidth, ShieldThickness, Height) / JamShapes::UnitSize);
	}
}

void AJamEffectsView::DrawRing(const FVector2D& Pos, float Radius, const FLinearColor& Color, float Time)
{
	const float DashLength = 2.f * PI * Radius / RingDashes * RingDashFill;
	for (int32 Index = 0; Index < RingDashes; ++Index)
	{
		const double Angle = 360.0 * (Index / static_cast<double>(RingDashes) + RingTurnsPerSecond * Time);
		const FVector2D Where = Pos + FVector2D(Radius, 0.0).GetRotated(Angle);
		UStaticMeshComponent& Dash = Take(JamShapes::Cube(), Color);
		// Each dash lies along the circle: across the radius it points to.
		Dash.SetWorldLocationAndRotation(FVector(Where.X, Where.Y, CircleHeight), FRotator(0.0, Angle + 90.0, 0.0));
		Dash.SetWorldScale3D(FVector(DashLength, RingDashWidth, GroundThickness) / JamShapes::UnitSize);
	}
}

void AJamEffectsView::DrawStrip(
	const FVector2D& From, const FVector2D& Dir, float Length, float Width, float Height, const FLinearColor& Color)
{
	const FVector2D Middle = From + Dir * (0.5 * Length);
	const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
	UStaticMeshComponent& Strip = Take(JamShapes::Cube(), Color);
	Strip.SetWorldLocationAndRotation(FVector(Middle.X, Middle.Y, Height), FRotator(0.0, Yaw, 0.0));
	Strip.SetWorldScale3D(FVector(Length, Width, GroundThickness) / JamShapes::UnitSize);
}

void AJamEffectsView::DrawLane(const FJamFight& Fight, EJamSide Side, float Alpha)
{
	const FJamUltimateLane Lane = JamFeedbackRules::UltimateLane(Fight, Side, Alpha, Balance);
	// The Publisher's lane is his telegraph: it shows only once he winds up, never while he merely holds a full meter.
	if (!Lane.bShow || (Side == EJamSide::Boss && !Lane.bArmed))
	{
		return;
	}
	const FLinearColor Base = Side == EJamSide::Hero ? UltimateColor : FillColor;
	// Ready and aimed at the air: a faint guide; aimed at the enemy or pressed: full colour.
	const FLinearColor Edge = Lane.bOnTarget || Lane.bArmed ? Base : Base * LaneIdleShare;
	const FVector2D Across(-Lane.Dir.Y, Lane.Dir.X);
	for (const float SideSign : { -1.f, 1.f })
	{
		DrawStrip(Lane.Origin + Across * (SideSign * Lane.HalfWidth), Lane.Dir, Lane.Length, LaneEdgeWidth, CircleHeight, Edge);
	}
	DrawStrip(
		Lane.Origin + Lane.Dir * Lane.Length - Across * Lane.HalfWidth, Across, 2.f * Lane.HalfWidth, LaneEdgeWidth, CircleHeight, Edge);
	const bool bAura = Side == EJamSide::Hero && ChargeLook == EJamChargeLook::Aura;
	if (Lane.bArmed && !bAura)
	{
		// The fill runs from the feet to the end of the lane exactly when the cut lands: the player reads the time left.
		DrawStrip(Lane.Origin, Lane.Dir, FMath::Max(1.f, Lane.Length * Lane.Fill), 2.f * Lane.HalfWidth, FillHeight, Base * LaneFillShare);
	}
}

void AJamEffectsView::DrawCash(const FJamFight& Fight, float Alpha, float Time)
{
	// Fight.Tick is the step to come: the picture is between the two before it.
	const float Tick = Fight.Tick - 1.f + Alpha;
	int32 Shown = 0;
	for (const FJamPickup& Cash : Fight.Pickups)
	{
		const float Left = (Cash.DieTick - Tick) * JamSim::StepDt;
		if (Left < CashBlinkSeconds && FMath::Frac(Time * CashBlinkHz) < 0.35f)
		{
			continue;
		}
		const float Flight = FMath::Clamp((Tick - Cash.StartTick) / FMath::Max(1, Cash.LandTick - Cash.StartTick), 0.f, 1.f);
		const FVector2D Pos = FMath::Lerp(Cash.From, Cash.Pos, static_cast<double>(Flight));
		const bool bLanded = Flight >= 1.f;
		const float Height = bLanded ? CashFloat + CashBob * FMath::Sin(2.f * PI * CashBobHz * Time)
									 : FMath::Lerp(BladeHeight, CashFloat, Flight) + CashArc * 4.f * Flight * (1.f - Flight);
		const FRotator Turn(0.0, 360.0 * CashTurnsPerSecond * Time, 0.0);
		if (Shown == Stacks.Num())
		{
			UStaticMeshComponent* Model = NewObject<UStaticMeshComponent>(this);
			JamShapes::Setup(*Model, JamShapes::CashStack());
			Model->SetMaterial(0, JamShapes::CashStack()->GetMaterial(0));
			Model->SetupAttachment(RootComponent);
			Model->RegisterComponent();
			Stacks.Add(Model);
		}
		UStaticMeshComponent& Stack = *Stacks[Shown++];
		Stack.SetVisibility(true);
		Stack.SetWorldLocationAndRotation(FVector(Pos.X, Pos.Y, Height), Turn);
		Stack.SetWorldScale3D(FVector(CashScale));
		// The model's pivot is under it: it moves so its middle is where the stack is.
		Stack.AddWorldOffset(-Turn.RotateVector(CashModelCentre * CashScale));
		if (bLanded)
		{
			DrawRing(Pos, CashRingRadius + CashRingPulse * FMath::Sin(2.f * PI * CashRingHz * Time), CashColor, Time);
		}
	}
	for (int32 Index = Shown; Index < Stacks.Num(); ++Index)
	{
		Stacks[Index]->SetVisibility(false);
	}
}

void AJamEffectsView::DrawCircles(const FJamFight& Fight, float Alpha, float Time)
{
	DrawCash(Fight, Alpha, Time);
	if (Fight.Phase == EJamPhase::Fight || Fight.Phase == EJamPhase::Stunned)
	{
		DrawLane(Fight, EJamSide::Hero, Alpha);
		DrawLane(Fight, EJamSide::Boss, Alpha);
	}
	// Fight.Tick is the step to come: the picture is between the two before it.
	const float Tick = Fight.Tick - 1.f + Alpha;
	int32 Seals = 0;
	for (const FJamHazard& Hazard : Fight.Hazards)
	{
		if (TelegraphLook == EJamTelegraphLook::Rune && DrawRune(Seals, JamFxRules::TelegraphOf(Hazard, Tick)))
		{
			++Seals;
			continue;
		}
		DrawDisc(Hazard.Pos, Hazard.Radius, CircleHeight, CircleColor);
		// The fill grows to the edge of the circle exactly when it hits: the player reads the time left from it.
		DrawDisc(Hazard.Pos, Hazard.Radius * JamHazardRules::Progress(Hazard, Tick), FillHeight, FillColor);
	}
	HideRunes(Seals);
	DrawStances(Fight, Alpha, Time);
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const FVector2D HeroPos = FMath::Lerp(Hero.PrevPos, Hero.Pos, static_cast<double>(Alpha));
	if (bUltimateReady && ChargeLook != EJamChargeLook::Aura)
	{
		DrawRing(HeroPos, Hero.Radius * ChargeRingScale, UltimateColor, Time);
	}
	for (const FJamProjectile& Projectile : Fight.Projectiles)
	{
		const FVector2D Pos = FMath::Lerp(Projectile.PrevPos, Projectile.Pos, static_cast<double>(Alpha));
		if (JamFeedbackRules::ShowsBlastRing(Projectile))
		{
			DrawRing(Pos, ExplosionRadius, BlastRingColor, Time);
		}
		if (Projectile.Id == ParryCue)
		{
			// The parry colour round the shot that the hint is about: this one, LMB sends back.
			DrawRing(Pos, Projectile.Radius * ParryCueScale, ParryColor, Time);
		}
	}
}

void AJamEffectsView::DrawStances(const FJamFight& Fight, float Alpha, float Time)
{
	const float Tick = Fight.Tick - 1.f + Alpha;
	for (const FJamFighter& Fighter : Fight.Fighters.Items)
	{
		const bool bAura = &Fighter == &Fight.Fighters[EJamSide::Hero] && ChargeLook == EJamChargeLook::Aura;
		if (Fighter.Ultimate.bPending && !bAura)
		{
			DrawDisc(FMath::Lerp(Fighter.PrevPos, Fighter.Pos, static_cast<double>(Alpha)), Fighter.Radius * ChargeRingScale, CircleHeight,
				UltimateColor);
		}
		const FJamParryState& Parry = Fighter.Parry;
		const FVector2D SelfPos = FMath::Lerp(Fighter.PrevPos, Fighter.Pos, static_cast<double>(Alpha));
		// Before the first step nobody has a parry: an untouched state reads as raising, and the intro (T-030) holds
		// the fight on that step for seconds.
		if (Fight.Tick < 1)
		{
			continue;
		}
		if (JamParryRules::IsRaising(Parry, Fight.Tick - 1))
		{
			// The Publisher's stance going up (T-038): the warning before it sends anything back.
			const float Rise = 1.f - (Parry.ActiveFromTick - Tick) / ParryRaiseTicks;
			DrawShield(SelfPos, Fighter.Radius * JamFeedbackRules::ParryRingScale(1.f, Feedback), Rise);
		}
		else if (JamParryRules::IsActive(Parry, Fight.Tick - 1))
		{
			const float WindowLeft = (Parry.ActiveUntilTick - Tick) / FMath::Max(1, Parry.ActiveUntilTick - Parry.ActiveFromTick);
			DrawShield(SelfPos, Fighter.Radius * JamFeedbackRules::ParryRingScale(WindowLeft, Feedback));
		}
	}
	const FJamBossState& Chain = Fight.Boss;
	if (Chain.Combo != EJamCombo::None && !Chain.Links.IsEmpty() && Tick < Chain.Links[0].Tick)
	{
		// T-056: the tell of a chain, a ring that turns under him before the first link.
		const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
		DrawRing(FMath::Lerp(Boss.PrevPos, Boss.Pos, static_cast<double>(Alpha)), Boss.Radius * ComboTellScale, BossBladeColor, Time);
	}
}

void AJamEffectsView::DrawSwing(const FJamEvent& Event, const FJamFighter& Self, const FVector2D& Pos, float Age, bool bBare)
{
	const int32 Strike = FMath::RoundToInt32(Event.Amount);
	const bool bFinisher = Strike >= 3;
	// The strikes of a series go left, right, and across again for the finisher.
	const float Side = Strike == 2 ? -1.f : 1.f;
	const float Length = KatanaRange * (bFinisher ? FinisherScale : 1.f);
	const double Yaw =
		FMath::RadiansToDegrees(FMath::Atan2(Self.Facing.Y, Self.Facing.X)) + Side * FMath::Lerp(-KatanaHalfArc, KatanaHalfArc, Age);
	const FVector2D Along = FVector2D(1.0, 0.0).GetRotated(Yaw);
	const FVector2D Middle = Pos + Along * (0.5 * Length);
	const FLinearColor Neon = bFinisher ? FinisherColor : (Event.Side == EJamSide::Hero ? HeroBladeColor : BossBladeColor);
	// The bare blade does not glow: Take multiplies by EffectGlow, so the grey is divided back to itself.
	const FLinearColor Color = bBare ? BareBladeColor / FMath::Max(EffectGlow, 1.f) : Neon;

	UStaticMeshComponent& Blade = Take(JamShapes::Cube(), Color);
	Blade.SetWorldLocationAndRotation(FVector(Middle.X, Middle.Y, BladeHeight), FRotator(0.0, Yaw, 0.0));
	Blade.SetWorldScale3D(FVector(Length, BladeWidth * (bFinisher ? FinisherScale : 1.f), BladeThickness) / JamShapes::UnitSize);
}

void AJamEffectsView::DrawTrail(const FJamEvent& Event, const FJamFighter& Self, const FVector2D& Pos)
{
	const FVector2D Path = Pos - Event.Pos;
	const double Length = Path.Size();
	if (Length < 1.0)
	{
		return;
	}
	// The hero's dash: the afterimages carry it in the full art, a thin streak one tier down, the bar in greybox (T-072).
	const bool bHero = Event.Side == EJamSide::Hero;
	if (bHero && ChargeLook == EJamChargeLook::Aura)
	{
		return;
	}
	const float Width = bHero && ChargeLook == EJamChargeLook::Glow ? DashStreakWidth : 1.f;
	const FVector2D Middle = Event.Pos + 0.5 * Path;
	const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(Path.Y, Path.X));
	UStaticMeshComponent& Trail = Take(JamShapes::Cube(), bHero ? HeroTrailColor : BossTrailColor);
	Trail.SetWorldLocationAndRotation(FVector(Middle.X, Middle.Y, TrailHeight), FRotator(0.0, Yaw, 0.0));
	Trail.SetWorldScale3D(FVector(Length, 2.f * Self.Radius * Width, GroundThickness) / JamShapes::UnitSize);
}

void AJamEffectsView::DrawSlash(const FJamEvent& Event, float Age)
{
	const FVector2D Along = FVector2D(1.0, 0.0).GetRotated(Event.Amount);
	const FVector2D Middle = Event.Pos + Along * (0.5 * UltimateRange);
	UStaticMeshComponent& Line = Take(JamShapes::Cube(), UltimateColor);
	Line.SetWorldLocationAndRotation(FVector(Middle.X, Middle.Y, UltimateHeight), FRotator(0.0, Event.Amount, 0.0));
	// The line is full width at once and thins out: the cut is the moment, the fade is the afterglow.
	Line.SetWorldScale3D(FVector(UltimateRange, 2.f * UltimateHalfWidth * (1.f - Age), BladeThickness) / JamShapes::UnitSize);
}

void AJamEffectsView::DrawBurst(const FJamEvent& Event, const FJamFight& Fight, float Alpha, float Age)
{
	const EJamEffectLook Look = JamFeedbackRules::LookOf(Event.Type, Event.Side, bHeroEffects);
	if (Look == EJamEffectLook::Hidden || !JamFxRules::DrawsPlain(Event.Type, bArt))
	{
		return;
	}
	const FJamFighter& Self = Fight.Fighters[Event.Side];
	const FVector2D SelfPos = FMath::Lerp(Self.PrevPos, Self.Pos, static_cast<double>(Alpha));
	switch (Event.Type)
	{
		case EJamEvent::Swing:
			DrawSwing(Event, Self, SelfPos, Age, Look == EJamEffectLook::Bare);
			break;
		case EJamEvent::Explosion:
			DrawDisc(Event.Pos, Event.Amount * FMath::Lerp(ExplosionStartSize, 1.f, Age), FlashHeight, ExplosionColor);
			break;
		case EJamEvent::Slam:
			DrawDisc(Event.Pos, Event.Amount, FlashHeight, SlamColor);
			break;
		case EJamEvent::Dash:
			DrawTrail(Event, Self, SelfPos);
			break;
		case EJamEvent::UltimateSlash:
			DrawSlash(Event, Age);
			break;
		case EJamEvent::Deflected:
			DrawDisc(Event.Pos, DeflectRadius * (1.f - Age), FlashHeight, DeflectColor);
			break;
		case EJamEvent::HealTaken:
			// Two thin rings climb the body one after the other: health going back in.
			for (const float Lag : { 0.f, 0.35f })
			{
				const float Climb = FMath::Clamp((Age - Lag) / (1.f - Lag), 0.f, 1.f);
				DrawDisc(SelfPos, Self.Radius * (1.4f - 0.3f * Climb), HealRise * Climb, HealColor);
			}
			break;
		case EJamEvent::Hit:
			// Around the body, not under it: a ground flash smaller than the fighter would never be seen.
			DrawDisc(SelfPos, Self.Radius + JamFeedbackRules::SparkRadius(Event.Amount, Feedback) * (1.f - Age), FlashHeight,
				Event.Side == EJamSide::Hero ? HeroSparkColor : BossSparkColor);
			break;
		default:
			break;
	}
}

void AJamEffectsView::Sync(const FJamFight& Fight, const TArray<FJamEvent>& Events, float Alpha, float Time)
{
	for (const FJamEvent& Event : Events)
	{
		const float Life = LifeOf(Event);
		if (ClearsTheAir(Event.Type))
		{
			Bursts.Reset();
		}
		else if (Life > 0.f)
		{
			Bursts.Add({ Event, Time, Life });
		}
	}
	Used = 0;
	DrawCircles(Fight, Alpha, Time);
	for (int32 Index = Bursts.Num() - 1; Index >= 0; --Index)
	{
		const float Age = (Time - Bursts[Index].StartTime) / Bursts[Index].Life;
		if (Age >= 1.f)
		{
			Bursts.RemoveAtSwap(Index);
			continue;
		}
		DrawBurst(Bursts[Index].Event, Fight, Alpha, Age);
	}
	for (int32 Index = Used; Index < Pool.Num(); ++Index)
	{
		Pool[Index]->SetVisibility(false);
	}
}
