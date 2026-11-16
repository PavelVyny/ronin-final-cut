#include "UI/JamCrackRules.h"
#include "UI/JamHUD.h"

#include "Engine/Canvas.h"

namespace
{
// The crack (T-068, T-071): how many strips make the old frame's torn halves and the fault's path, the side branches
// off the fault, the flashes.
constexpr int32 FaultStrips = 96;
constexpr int32 FaultBranches = 6;
constexpr float FaultStartFlashSeconds = 0.12f;
constexpr float FaultPartFlashSeconds = 0.3f;
// A branch grows over this share of the height after the crack's head passes it.
constexpr float FaultBranchGrowth = 0.12f;
// The old halves dim as they slide off, so the new look reads as the one in front, and lean to their edge's neon.
constexpr float FaultDimAtOnce = 0.2f;
constexpr float FaultDim = 0.35f;
constexpr float FaultTint = 0.3f;
const FLinearColor FaultCyan(0.25f, 0.95f, 1.0f);
const FLinearColor FaultMagenta(1.0f, 0.25f, 0.75f);

/** A hash in 0..1: the branches keep their shape every frame without state. */
float FaultHash(int32 A, int32 B)
{
	const uint32 Hash = HashCombineFast(GetTypeHash(A * 92821 + 7), GetTypeHash(B * 7919 + 13));
	return static_cast<float>(Hash % 10007u) / 10006.f;
}

/** A neon line along Points: a wide halo, a coloured body, a white-hot core. */
void GlowLine(AHUD& Hud, TConstArrayView<FVector2D> Points, const FLinearColor& Neon, float Alpha, float Thick)
{
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		const FVector2D A = Points[Index - 1];
		const FVector2D B = Points[Index];
		// T-085: a thin, quiet line: a faint halo, a fine coloured body, a hairline core.
		Hud.DrawLine(A.X, A.Y, B.X, B.Y, Neon * FLinearColor(1.f, 1.f, 1.f, 0.12f * Alpha), 6.f * Thick);
		Hud.DrawLine(A.X, A.Y, B.X, B.Y, Neon * FLinearColor(1.f, 1.f, 1.f, 0.5f * Alpha), 2.5f * Thick);
		Hud.DrawLine(A.X, A.Y, B.X, B.Y, FLinearColor(1.f, 1.f, 1.f, 0.7f * Alpha), 1.f * Thick);
	}
}

/** The old frame: whole until the halves move, then torn along the fault, each half Shift pixels off its side. */
void TornHalves(AHUD& Hud, UTexture* OldFrame, float W, float H, float Shift, int32 Seed, float Lit)
{
	if (Shift <= 0.f)
	{
		Hud.DrawTexture(OldFrame, 0.f, 0.f, W, H, 0.f, 0.f, 1.f, 1.f);
		return;
	}
	// Each half takes a tint of its own edge's neon and darkens as it goes: the eye follows which half went where.
	const FLinearColor LeftTint = FLinearColor::LerpUsingHSV(FLinearColor::White, FaultCyan, FaultTint) * Lit;
	const FLinearColor RightTint = FLinearColor::LerpUsingHSV(FLinearColor::White, FaultMagenta, FaultTint) * Lit;
	const float StripH = H / FaultStrips;
	for (int32 Strip = 0; Strip < FaultStrips; ++Strip)
	{
		const float Y = Strip * StripH;
		const float X = JamCrackRules::FaultX((Strip + 0.5f) / FaultStrips, Seed) * W;
		// The left half moves left: screen x shows the picture at x + Shift; the right half the other way.
		if (X - Shift > 0.f)
		{
			Hud.DrawTexture(
				OldFrame, 0.f, Y, X - Shift, StripH, Shift / W, Y / H, (X - Shift) / W, StripH / H, LeftTint.CopyWithNewOpacity(1.f));
		}
		if (X + Shift < W)
		{
			Hud.DrawTexture(OldFrame, X + Shift, Y, W - X - Shift, StripH, X / W, Y / H, (W - X - Shift) / W, StripH / H,
				RightTint.CopyWithNewOpacity(1.f));
		}
	}
}

/** The fault from the top down to Reach, in pixels, moved Offset pixels sideways. */
TArray<FVector2D> FaultPath(float W, float H, float Reach, float Offset, int32 Seed)
{
	TArray<FVector2D> Path;
	Path.Add(FVector2D(JamCrackRules::FaultX(0.f, Seed) * W + Offset, 0.f));
	for (int32 Strip = 0; Strip < FaultStrips && (Strip + 0.5f) / FaultStrips < Reach; ++Strip)
	{
		const float V = (Strip + 0.5f) / FaultStrips;
		Path.Add(FVector2D(JamCrackRules::FaultX(V, Seed) * W + Offset, V * H));
	}
	Path.Add(FVector2D(JamCrackRules::FaultX(Reach, Seed) * W + Offset, Reach * H));
	return Path;
}

/** The short cracks that split off the fault into one half: three kinked segments, grown as the head passes. */
void Branches(AHUD& Hud, float W, float H, float Reach, float Shift, int32 Seed, float Alpha)
{
	for (int32 Branch = 0; Branch < FaultBranches; ++Branch)
	{
		const float V = 0.08f + 0.84f * FaultHash(Branch, Seed);
		const float Grown = FMath::Clamp((Reach - V) / FaultBranchGrowth, 0.f, 1.f);
		if (Grown <= 0.f)
		{
			continue;
		}
		const float Side = FaultHash(Branch + 100, Seed) < 0.5f ? -1.f : 1.f;
		const float Length = (0.025f + 0.04f * FaultHash(Branch + 200, Seed)) * W * Grown;
		FVector2D Point(JamCrackRules::FaultX(V, Seed) * W + Side * Shift, V * H);
		TArray<FVector2D, TInlineAllocator<4>> Path = { Point };
		for (int32 Kink = 0; Kink < 3; ++Kink)
		{
			const float Down = (FaultHash(Branch * 3 + Kink, Seed + 500) - 0.35f) * 0.9f;
			Point += FVector2D(Side, Down) * (Length / 3.f);
			Path.Add(Point);
		}
		GlowLine(Hud, Path, Side < 0.f ? FaultCyan : FaultMagenta, Alpha, 0.45f);
	}
}
} // namespace

void AJamHUD::DrawCrack()
{
	if (CrackAge < 0.f || !CrackFrame)
	{
		return;
	}
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float Reach = JamCrackRules::Reach(CrackAge);
	const float Shift = JamCrackRules::Shift(CrackAge) * W;
	const float Gone = Shift / ((1.f - JamCrackRules::FaultMargin) * W);
	TornHalves(*this, CrackFrame, W, H, Shift, CrackSeed, 1.f - FaultDimAtOnce - FaultDim * Gone);
	// A white blink as the old picture freezes: something broke.
	const float StartFlash = 1.f - FMath::Clamp(CrackAge / FaultStartFlashSeconds, 0.f, 1.f);
	if (StartFlash > 0.f)
	{
		DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.35f * StartFlash), 0.f, 0.f, W, H);
	}
	// While it runs and holds, one white-hot line that flickers; then each half carries its own neon edge away.
	const float PartAge = CrackAge - JamCrackRules::RunSeconds - JamCrackRules::HoldSeconds;
	if (Shift <= 0.f)
	{
		const float Flicker = PartAge > -JamCrackRules::HoldSeconds ? 0.8f + 0.2f * FMath::Sin(CrackAge * 55.f) : 1.f;
		const TArray<FVector2D> Path = FaultPath(W, H, Reach, 0.f, CrackSeed);
		GlowLine(*this, Path, FaultMagenta, Flicker, 1.f);
		Branches(*this, W, H, Reach, 0.f, CrackSeed, Flicker);
		const FVector2D Head = Path.Last();
		DrawLine(Head.X - 8.f, Head.Y, Head.X + 8.f, Head.Y, FLinearColor(1.f, 1.f, 1.f, 0.8f), 1.5f);
		DrawLine(Head.X, Head.Y - 8.f, Head.X, Head.Y + 8.f, FLinearColor(1.f, 1.f, 1.f, 0.8f), 1.5f);
		return;
	}
	const float PartFlash = 1.f - FMath::Clamp(PartAge / FaultPartFlashSeconds, 0.f, 1.f);
	if (PartFlash > 0.f)
	{
		const float StripH = H / FaultStrips;
		for (int32 Strip = 0; Strip < FaultStrips; ++Strip)
		{
			const float X = JamCrackRules::FaultX((Strip + 0.5f) / FaultStrips, CrackSeed) * W;
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.7f * PartFlash), X - Shift, Strip * StripH, 2.f * Shift, StripH);
		}
	}
	const float Glow = 1.f - 0.5f * Gone;
	GlowLine(*this, FaultPath(W, H, 1.f, -Shift, CrackSeed), FaultCyan, Glow, 1.f);
	GlowLine(*this, FaultPath(W, H, 1.f, Shift, CrackSeed), FaultMagenta, Glow, 1.f);
	Branches(*this, W, H, 1.f, Shift, CrackSeed, Glow);
}
