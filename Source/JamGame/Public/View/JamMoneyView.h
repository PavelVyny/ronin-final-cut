#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "View/JamFeedbackRules.h"

#include "JamMoneyView.generated.h"

class UStaticMeshComponent;

/**
 * The paper dollars the Publisher bleeds when hit (T-042): a spray from the hit point that flutters down, lies on the
 * floor and fades. Each bill is a green stand-in card from a pool that only grows, up to a fixed number. The money is
 * the Publisher's look: the hero's effects cut never takes it.
 */
UCLASS()
class JAMGAME_API AJamMoneyView : public AActor
{
	GENERATED_BODY()

public:
	AJamMoneyView();

	void Init(const FJamFeedbackSettings& InFeedback);

	/** Throws the bills of the frame's hits and draws every bill alive at Time. */
	void Sync(const FJamFight& Fight, const TArray<FJamEvent>& Events, float Time);

private:
	struct FBill
	{
		FVector From = FVector::ZeroVector;
		FVector2D Kick = FVector2D::ZeroVector;
		float Phase = 0.f;
		float StartTime = 0.f;
	};

	/** Count bills spill out of the Publisher's body (JamFeedbackRules::Spill), all round him. */
	void Throw(const FJamFighter& Boss, int32 Count, bool bBurst, float Time);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Pool;

	TArray<FBill> Alive;
	/** Presentation only: the spray need not replay, but a fixed seed keeps the clips alike. */
	FRandomStream Random;
	FJamFeedbackSettings Feedback;
};
