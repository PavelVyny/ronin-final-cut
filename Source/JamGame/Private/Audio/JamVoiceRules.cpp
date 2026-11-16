#include "Audio/JamVoiceRules.h"

#include "Sim/JamStealRules.h"

namespace
{
const TCHAR* const LineNames[] = {
	TEXT("vo_intro_pub"),
	TEXT("vo_intro_ron"),
	TEXT("vo_cut1_pub"),
	TEXT("vo_cut1_ron"),
	TEXT("vo_cut2_pub"),
	TEXT("vo_cut2_ron"),
	TEXT("vo_cut3_pub"),
	TEXT("vo_cut3_ron"),
	TEXT("vo_cut4_pub"),
	TEXT("vo_cut4_ron"),
	TEXT("vo_stun_pub"),
	TEXT("vo_final_ron"),
	TEXT("vo_final_pub"),
	TEXT("vo_bark_1_pub"),
	TEXT("vo_bark_2_pub"),
	TEXT("vo_bark_3_pub"),
	TEXT("vo_bark_4_pub"),
	TEXT("vo_steal_shuriken"),
	TEXT("vo_steal_parry"),
	TEXT("vo_steal_dash_pub"),
	TEXT("vo_steal_dash_ron"),
	TEXT("vo_steal_ultimate"),
	TEXT("vo_comic_1_pub"),
	TEXT("vo_comic_2_ron"),
	TEXT("vo_comic_3_pub"),
	TEXT("vo_comic_4_ron"),
	TEXT("vo_banter_1_pub"),
	TEXT("vo_banter_1_ron"),
	TEXT("vo_banter_2_pub"),
	TEXT("vo_banter_2_ron"),
	TEXT("vo_banter_3_pub"),
	TEXT("vo_banter_3_ron"),
	TEXT("vo_banter_4_pub"),
	TEXT("vo_banter_4_ron"),
	TEXT("vo_banter_5_pub"),
	TEXT("vo_banter_5_ron"),
	TEXT("vo_banter_6_pub"),
	TEXT("vo_banter_6_ron"),
	TEXT("vo_banter_7_pub"),
	TEXT("vo_banter_7_ron"),
	TEXT("vo_banter_8_pub"),
	TEXT("vo_banter_8_ron"),
};
static_assert(UE_ARRAY_COUNT(LineNames) == JamVoiceRules::LineCount, "one name per line");
} // namespace

const TCHAR* JamVoiceRules::NameOf(EJamLine Line)
{
	return LineNames[static_cast<int32>(Line)];
}

EJamSide JamVoiceRules::SpeakerOf(EJamLine Line)
{
	return FString(NameOf(Line)).EndsWith(TEXT("_ron")) ? EJamSide::Hero : EJamSide::Boss;
}

bool JamVoiceRules::IsBark(EJamLine Line)
{
	return Line >= EJamLine::Bark1 && Line <= EJamLine::Bark4;
}

namespace
{
/** The lines one event starts, added to Lines. */
void AddLinesOf(const FJamEvent& Event, const FJamFight& Fight, TArray<EJamLine>& Lines)
{
	if (Event.Type == EJamEvent::StealOpened)
	{
		// The card of cut N opens with N - 1 cuts done: the count goes up when it is settled.
		const int32 Cut = FMath::Clamp(Fight.Steal.Done, 0, JamStealRules::ThresholdCount - 1);
		const int32 First = static_cast<int32>(EJamLine::Cut1Boss) + 2 * Cut;
		Lines.Add(static_cast<EJamLine>(First));
		Lines.Add(static_cast<EJamLine>(First + 1));
	}
	else if (Event.Type == EJamEvent::Stunned)
	{
		Lines.Add(EJamLine::StunBoss);
	}
	else if (Event.Type == EJamEvent::UltimateSlash && Fight.Phase == EJamPhase::Won)
	{
		Lines.Add(EJamLine::FinalHero);
		Lines.Add(EJamLine::FinalBoss);
	}
}
} // namespace

TArray<EJamLine> JamVoiceRules::LinesOf(const TArray<FJamEvent>& Events, const FJamFight& Fight)
{
	TArray<EJamLine> Lines;
	for (const FJamEvent& Event : Events)
	{
		AddLinesOf(Event, Fight, Lines);
	}
	return Lines;
}

bool JamVoiceRules::CutsIn(EJamLine Line)
{
	return Line == EJamLine::FinalHero;
}

float JamVoiceRules::NextBarkDelay(FRandomStream& Stream)
{
	return Stream.FRandRange(BarkMinSeconds, BarkMaxSeconds);
}

EJamLine JamVoiceRules::PickBark(FRandomStream& Stream, EJamLine Last)
{
	const int32 First = static_cast<int32>(EJamLine::Bark1);
	// One of the other three: a step of 1 to 3 round the four.
	const int32 From = IsBark(Last) ? static_cast<int32>(Last) - First : 0;
	const int32 Step = IsBark(Last) ? Stream.RandRange(1, BarkCount - 1) : Stream.RandRange(0, BarkCount - 1);
	return static_cast<EJamLine>(First + (From + Step) % BarkCount);
}

TArray<EJamLine> JamVoiceRules::CommentsOf(const TArray<FJamEvent>& Events)
{
	TArray<EJamLine> Lines;
	for (const FJamEvent& Event : Events)
	{
		if (Event.Type != EJamEvent::StealResolved || Event.Side != EJamSide::Boss)
		{
			continue;
		}
		switch (static_cast<EJamMechanic>(FMath::RoundToInt32(Event.Amount)))
		{
			case EJamMechanic::Shuriken:
				Lines.Add(EJamLine::StealShuriken);
				break;
			case EJamMechanic::Parry:
				Lines.Add(EJamLine::StealParry);
				break;
			case EJamMechanic::Dash:
				Lines.Add(EJamLine::StealDashBoss);
				Lines.Add(EJamLine::StealDashHero);
				break;
			case EJamMechanic::Ultimate:
				Lines.Add(EJamLine::StealUltimate);
				break;
			// The Katana is never his to brag about: it only goes with the last cut.
			default:
				break;
		}
	}
	return Lines;
}

void JamVoiceRules::Release(TArray<EJamLine>& Held, TArray<EJamLine>& Queue, bool bCracking)
{
	if (!bCracking)
	{
		Queue.Append(Held);
		Held.Reset();
	}
}

bool JamVoiceRules::MayBark(const FJamFight& Fight, bool bSpeaking, bool bHolding)
{
	return Fight.Phase == EJamPhase::Fight && !bSpeaking && !bHolding;
}

float JamVoiceRules::Loudness(float MenuGain)
{
	return FMath::Max(MenuGain, 0.f) * Boost;
}

bool JamVoiceRules::IsBanter(EJamLine Line)
{
	return Line >= EJamLine::Banter1Boss && Line <= EJamLine::Banter8Hero;
}

bool JamVoiceRules::MayFill(const FJamFight& Fight, bool bSpeaking, bool bHolding, float Silence)
{
	return MayBark(Fight, bSpeaking, bHolding) && Silence > SilenceSeconds;
}

TArray<EJamLine> JamVoiceRules::NextFiller(FRandomStream& Stream, TArray<int32>& Bag, int32& Last)
{
	if (Bag.IsEmpty())
	{
		for (int32 Filler = 0; Filler < FillerCount; ++Filler)
		{
			Bag.Add(Filler);
		}
	}
	// A fresh bag never opens with the one that closed the last.
	int32 Index = Stream.RandRange(0, Bag.Num() - 1);
	if (Bag[Index] == Last && Bag.Num() > 1)
	{
		Index = (Index + 1) % Bag.Num();
	}
	const int32 Pick = Bag[Index];
	Bag.RemoveAt(Index);
	Last = Pick;
	if (Pick < BanterCount)
	{
		const int32 First = static_cast<int32>(EJamLine::Banter1Boss) + 2 * Pick;
		return { static_cast<EJamLine>(First), static_cast<EJamLine>(First + 1) };
	}
	return { static_cast<EJamLine>(static_cast<int32>(EJamLine::Bark1) + Pick - BanterCount) };
}

bool JamVoiceRules::CutsOff(EJamLine Saying, EJamLine Incoming)
{
	return IsBanter(Saying) && !IsBanter(Incoming) && !IsBark(Incoming);
}
