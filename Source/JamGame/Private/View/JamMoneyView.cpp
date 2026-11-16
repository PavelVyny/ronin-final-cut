#include "View/JamMoneyView.h"

#include "Components/StaticMeshComponent.h"
#include "Sim/JamUltimateRules.h"
#include "View/JamShapes.h"

namespace
{
// Greybox look of a bill: presentation only, sizes in cm.
constexpr float BillLength = 34.f;
constexpr float BillWidth = 16.f;
constexpr float BillThickness = 1.f;
// Bills leave the body at chest height and lie a little above the floor, under the warning circles.
constexpr float ThrowHeight = 120.f;
constexpr float FloorHeight = 2.f;
// How far past his body a bill flies: a hit spills it close, the final blow throws it across the arena.
constexpr float SprayMin = 60.f;
constexpr float SprayMax = 170.f;
constexpr float BurstScale = 2.2f;
// The oldest bills go first beyond this many, so a long fight never fills the screen.
constexpr int32 MaxBills = 200;
constexpr int32 Seed = 4242;
// The dollar card (r5_dollar_card) shines a little: the money is what the player chases.
constexpr float BillGlow = 1.6f;
} // namespace

AJamMoneyView::AJamMoneyView()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	// The pool is filled at runtime: the shapes have to be found here, in a constructor.
	JamShapes::Cube();
	JamShapes::DollarCard();
	Random.Initialize(Seed);
}

void AJamMoneyView::Init(const FJamFeedbackSettings& InFeedback)
{
	Feedback = InFeedback;
}

void AJamMoneyView::Throw(const FJamFighter& Boss, int32 Count, bool bBurst, float Time)
{
	for (int32 Index = 0; Index < Count; ++Index)
	{
		FBill Bill;
		const float Reach = Random.FRandRange(SprayMin, SprayMax) * (bBurst ? BurstScale : 1.f);
		const FJamSpill Spill = JamFeedbackRules::Spill(Boss.Pos, Boss.Radius, Random.FRandRange(0.f, 360.f), Reach);
		Bill.From = FVector(Spill.From.X, Spill.From.Y, ThrowHeight);
		Bill.Kick = Spill.Kick;
		Bill.Phase = Random.FRand();
		Bill.StartTime = Time;
		Alive.Add(Bill);
	}
	if (Alive.Num() > MaxBills)
	{
		Alive.RemoveAt(0, Alive.Num() - MaxBills);
	}
}

void AJamMoneyView::Sync(const FJamFight& Fight, const TArray<FJamEvent>& Events, float Time)
{
	for (const FJamEvent& Event : Events)
	{
		// The cut of the stunned Publisher that won the fight: the money he has left goes up at once.
		const bool bFinalBlow = Fight.Phase == EJamPhase::Won && Event.ProjectileId == JamUltimateRules::SlashId;
		const int32 Count = JamFeedbackRules::BillCount(Event, bFinalBlow, Feedback);
		if (Count > 0)
		{
			Throw(Fight.Fighters[EJamSide::Boss], Count, bFinalBlow, Time);
		}
	}

	const float Life = JamFeedbackRules::BillLife(Feedback);
	Alive.RemoveAll([Time, Life](const FBill& Bill) { return Time - Bill.StartTime >= Life; });
	while (Pool.Num() < Alive.Num())
	{
		UStaticMeshComponent* Card = NewObject<UStaticMeshComponent>(this);
		JamShapes::Setup(*Card, JamShapes::Cube());
		Card->SetupAttachment(RootComponent);
		Card->RegisterComponent();
		// All bills are one card: the material is set once, when the card joins the pool.
		JamShapes::Card(*Card, JamShapes::DollarCard(), BillGlow);
		Pool.Add(Card);
	}
	for (int32 Index = 0; Index < Pool.Num(); ++Index)
	{
		UStaticMeshComponent& Card = *Pool[Index];
		Card.SetVisibility(Index < Alive.Num());
		if (Index >= Alive.Num())
		{
			continue;
		}
		const FBill& Bill = Alive[Index];
		const FJamBillPose Pose =
			JamFeedbackRules::BillPose(Bill.From, Bill.Kick, FloorHeight, Bill.Phase, Time - Bill.StartTime, Feedback);
		const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(Bill.Kick.Y, Bill.Kick.X));
		Card.SetWorldLocationAndRotation(Pose.Pos, FRotator(0.0, Yaw, Pose.Roll));
		Card.SetWorldScale3D(FVector(BillLength, BillWidth, BillThickness) * Pose.Scale / JamShapes::UnitSize);
	}
}
