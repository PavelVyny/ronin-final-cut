#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Audio/JamVoiceRules.h"
#include "Tests/JamTestHelpers.h"
#include "UI/JamTexts.h"

/**
 * The voice-over (Audio/Eleven/Voice/VO_LINES.md): which line the Publisher and the hero say on which step of the fight.
 * A bug looks in game like "the Publisher announces the wrong cut", "the hero answers his own line", "the final blow is
 * silent" or "the Publisher barks over the steal card".
 *
 * Run: Automation RunTests Jam.Voice
 */
namespace
{
FJamEvent VoiceEvent(EJamEvent Type)
{
	FJamEvent Event;
	Event.Type = Type;
	return Event;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceNamesTest, "Jam.Voice.EveryLineHasItsFileAndWords", JamTest::Flags)

bool FJamVoiceNamesTest::RunTest(const FString& Parameters)
{
	TSet<FString> Names;
	for (int32 Index = 0; Index < JamVoiceRules::LineCount; ++Index)
	{
		const EJamLine Line = static_cast<EJamLine>(Index);
		const FString Name = JamVoiceRules::NameOf(Line);
		TestTrue(TEXT("a vo_ file"), Name.StartsWith(TEXT("vo_")));
		TestFalse(TEXT("each its own"), Names.Contains(Name));
		Names.Add(Name);
		TestFalse(TEXT("with words for the subtitle"), FString(JamTexts::VoiceLine(Index)).IsEmpty());
		// The file name says who speaks: _ron for the hero, the Publisher's end in _pub or in the stolen move (vo_steal_parry).
		const EJamSide Speaker = JamVoiceRules::SpeakerOf(Line);
		TestTrue(TEXT("the speaker matches the file"), Name.EndsWith(TEXT("_ron")) == (Speaker == EJamSide::Hero));
	}
	TestEqual(TEXT("one subtitle per line"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::VoiceLines)), JamVoiceRules::LineCount);
	TestEqual(
		TEXT("the intro opens with the Publisher"), FString(JamVoiceRules::NameOf(EJamLine::IntroBoss)), FString(TEXT("vo_intro_pub")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceCutsTest, "Jam.Voice.EachCutHasItsExchange", JamTest::Flags)

bool FJamVoiceCutsTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	F.Phase = EJamPhase::Steal;
	const TArray<FJamEvent> Opened = { VoiceEvent(EJamEvent::StealOpened) };
	F.Steal.Done = 0;
	TestTrue(
		TEXT("the first cut: the art"), JamVoiceRules::LinesOf(Opened, F) == TArray<EJamLine>({ EJamLine::Cut1Boss, EJamLine::Cut1Hero }));
	F.Steal.Done = 2;
	TestTrue(TEXT("the third: the animation"),
		JamVoiceRules::LinesOf(Opened, F) == TArray<EJamLine>({ EJamLine::Cut3Boss, EJamLine::Cut3Hero }));
	F.Steal.Done = 3;
	TestTrue(TEXT("the last: the final note"),
		JamVoiceRules::LinesOf(Opened, F) == TArray<EJamLine>({ EJamLine::Cut4Boss, EJamLine::Cut4Hero }));
	F.Steal.Done = 0;
	TestEqual(TEXT("the resolve of a card says nothing"), JamVoiceRules::LinesOf({ VoiceEvent(EJamEvent::StealResolved) }, F).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceEndTest, "Jam.Voice.TheStunAndTheFinalBlowAreSpoken", JamTest::Flags)

bool FJamVoiceEndTest::RunTest(const FString& Parameters)
{
	FJamFight F;
	F.Phase = EJamPhase::Stunned;
	TestTrue(TEXT("out of notes, he bargains"),
		JamVoiceRules::LinesOf({ VoiceEvent(EJamEvent::Stunned) }, F) == TArray<EJamLine>({ EJamLine::StunBoss }));
	F.Phase = EJamPhase::Won;
	const TArray<EJamLine> Final = JamVoiceRules::LinesOf({ VoiceEvent(EJamEvent::UltimateSlash) }, F);
	TestTrue(TEXT("the hero's last word, then his gasp"), Final == TArray<EJamLine>({ EJamLine::FinalHero, EJamLine::FinalBoss }));
	TestTrue(TEXT("the last word cuts in over anything"), JamVoiceRules::CutsIn(EJamLine::FinalHero));
	TestFalse(TEXT("a cut's exchange waits its turn"), JamVoiceRules::CutsIn(EJamLine::Cut1Boss));
	F.Phase = EJamPhase::Fight;
	TestEqual(
		TEXT("an Ultimate in the fight is not the end"), JamVoiceRules::LinesOf({ VoiceEvent(EJamEvent::UltimateSlash) }, F).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceBarksTest, "Jam.Voice.BarksComeNowAndThenInTheFight", JamTest::Flags)

bool FJamVoiceBarksTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(7);
	EJamLine Last = EJamLine::Bark1;
	for (int32 Round = 0; Round < 50; ++Round)
	{
		const float Delay = JamVoiceRules::NextBarkDelay(Stream);
		TestTrue(TEXT("15 to 25 s apart"), Delay >= JamVoiceRules::BarkMinSeconds && Delay <= JamVoiceRules::BarkMaxSeconds);
		const EJamLine Bark = JamVoiceRules::PickBark(Stream, Last);
		TestTrue(TEXT("a bark"), JamVoiceRules::IsBark(Bark));
		TestTrue(TEXT("never the same twice running"), Bark != Last);
		TestTrue(TEXT("the Publisher's"), JamVoiceRules::SpeakerOf(Bark) == EJamSide::Boss);
		Last = Bark;
	}
	FJamFight F;
	F.Phase = EJamPhase::Fight;
	TestTrue(TEXT("in the fight when nobody talks"), JamVoiceRules::MayBark(F, false));
	TestFalse(TEXT("not over another line"), JamVoiceRules::MayBark(F, true));
	F.Phase = EJamPhase::Steal;
	TestFalse(TEXT("not on the card"), JamVoiceRules::MayBark(F, false));
	F.Phase = EJamPhase::Stunned;
	TestFalse(TEXT("not while stunned"), JamVoiceRules::MayBark(F, false));
	return true;
}

namespace
{
FJamEvent Stolen(EJamMechanic Mechanic, EJamSide Owner = EJamSide::Boss)
{
	FJamEvent Event = VoiceEvent(EJamEvent::StealResolved);
	Event.Side = Owner;
	Event.Amount = static_cast<float>(Mechanic);
	return Event;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceStealCommentTest, "Jam.Voice.ThePublisherCommentsOnTheMoveHeStole", JamTest::Flags)

bool FJamVoiceStealCommentTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the shuriken: ranged damage to monetise"),
		JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Shuriken) }) == TArray<EJamLine>({ EJamLine::StealShuriken }));
	TestTrue(TEXT("the parry: blocking ideas"),
		JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Parry) }) == TArray<EJamLine>({ EJamLine::StealParry }));
	TestTrue(TEXT("the dash: a paywall, and the hero answers"),
		JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Dash) }) ==
			TArray<EJamLine>({ EJamLine::StealDashBoss, EJamLine::StealDashHero }));
	TestTrue(TEXT("the Ultimate: his DLC"),
		JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Ultimate) }) == TArray<EJamLine>({ EJamLine::StealUltimate }));
	TestEqual(TEXT("the Katana has no comment"), JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Katana) }).Num(), 0);
	// The last cut takes the Katana with the one move left: he comments on the move.
	TestTrue(TEXT("the last cut: the move, not the Katana"),
		JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Katana), Stolen(EJamMechanic::Parry) }) ==
			TArray<EJamLine>({ EJamLine::StealParry }));
	TestEqual(
		TEXT("a move back with the hero is no steal"), JamVoiceRules::CommentsOf({ Stolen(EJamMechanic::Dash, EJamSide::Hero) }).Num(), 0);
	TestEqual(TEXT("the card opening is no steal yet"), JamVoiceRules::CommentsOf({ VoiceEvent(EJamEvent::StealOpened) }).Num(), 0);
	TestTrue(TEXT("the hero answers the dash"), JamVoiceRules::SpeakerOf(EJamLine::StealDashHero) == EJamSide::Hero);
	TestTrue(TEXT("the Publisher comments"), JamVoiceRules::SpeakerOf(EJamLine::StealUltimate) == EJamSide::Boss);
	TestFalse(TEXT("a comment is no bark"), JamVoiceRules::IsBark(EJamLine::StealShuriken));
	TestFalse(TEXT("and does not cut in"), JamVoiceRules::CutsIn(EJamLine::StealDashBoss));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceStealWaitTest, "Jam.Voice.TheCommentWaitsForTheCrackAndTheCard", JamTest::Flags)

bool FJamVoiceStealWaitTest::RunTest(const FString& Parameters)
{
	// The card's exchange is still in the queue when the steal settles and the crack opens.
	TArray<EJamLine> Queue = { EJamLine::Cut2Hero };
	TArray<EJamLine> Held = { EJamLine::StealDashBoss, EJamLine::StealDashHero };
	JamVoiceRules::Release(Held, Queue, true);
	TestEqual(TEXT("held through the crack"), Held.Num(), 2);
	TestTrue(TEXT("the queue untouched"), Queue == TArray<EJamLine>({ EJamLine::Cut2Hero }));
	JamVoiceRules::Release(Held, Queue, false);
	TestEqual(TEXT("let go when the crack is over"), Held.Num(), 0);
	TestTrue(TEXT("after the card's lines, in order"),
		Queue == TArray<EJamLine>({ EJamLine::Cut2Hero, EJamLine::StealDashBoss, EJamLine::StealDashHero }));
	JamVoiceRules::Release(Held, Queue, false);
	TestEqual(TEXT("said once"), Queue.Num(), 3);
	FJamFight F;
	F.Phase = EJamPhase::Fight;
	TestFalse(TEXT("no bark while a comment waits"), JamVoiceRules::MayBark(F, false, true));
	TestTrue(TEXT("barks again once it is said"), JamVoiceRules::MayBark(F, false, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceLoudTest, "Jam.Voice.LinesPlayLouderThanRecorded", JamTest::Flags)

bool FJamVoiceLoudTest::RunTest(const FString& Parameters)
{
	// T-094: the author heard the voices too quiet, in the comic and in the fight: 25 to 30% louder.
	TestTrue(TEXT("full voice volume is 25 to 30% louder"), JamVoiceRules::Loudness(1.f) >= 1.25f && JamVoiceRules::Loudness(1.f) <= 1.3f);
	TestEqual(TEXT("silence stays silent"), JamVoiceRules::Loudness(0.f), 0.f);
	TestTrue(TEXT("half the menu's volume is half as loud"),
		FMath::IsNearlyEqual(JamVoiceRules::Loudness(0.5f), 0.5f * JamVoiceRules::Loudness(1.f), 1e-5f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceBanterSilenceTest, "Jam.Voice.BanterFillsTheSilence", JamTest::Flags)

bool FJamVoiceBanterSilenceTest::RunTest(const FString& Parameters)
{
	// The session-5s playtest: too much silence. After more than 5 s without a line, a pair of banter plays.
	FJamFight F;
	F.Phase = EJamPhase::Fight;
	TestFalse(TEXT("not before 5 s of silence"), JamVoiceRules::MayFill(F, false, false, 4.9f));
	TestTrue(TEXT("after more than 5 s"), JamVoiceRules::MayFill(F, false, false, 5.1f));
	TestFalse(TEXT("not over a line"), JamVoiceRules::MayFill(F, true, false, 9.f));
	TestFalse(TEXT("not while a comment waits for the crack"), JamVoiceRules::MayFill(F, false, true, 9.f));
	F.Phase = EJamPhase::Steal;
	TestFalse(TEXT("not on the card"), JamVoiceRules::MayFill(F, false, false, 9.f));
	F.Phase = EJamPhase::Won;
	TestFalse(TEXT("not after the fight"), JamVoiceRules::MayFill(F, false, false, 9.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceBanterBagTest, "Jam.Voice.EveryPairBeforeARepeat", JamTest::Flags)

bool FJamVoiceBanterBagTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(11);
	TArray<int32> Bag;
	int32 Last = INDEX_NONE;
	TArray<EJamLine> Previous;
	for (int32 Round = 0; Round < 3; ++Round)
	{
		TMap<EJamLine, int32> Seen;
		int32 Pairs = 0;
		for (int32 Draw = 0; Draw < JamVoiceRules::FillerCount; ++Draw)
		{
			const TArray<EJamLine> Lines = JamVoiceRules::NextFiller(Stream, Bag, Last);
			TestTrue(TEXT("one or two lines"), Lines.Num() == 1 || Lines.Num() == 2);
			if (Lines.IsEmpty())
			{
				return false;
			}
			TestFalse(TEXT("never the same twice running"), Lines == Previous);
			Previous = Lines;
			Seen.FindOrAdd(Lines[0])++;
			if (Lines.Num() == 2)
			{
				++Pairs;
				TestTrue(TEXT("banter"), JamVoiceRules::IsBanter(Lines[0]) && JamVoiceRules::IsBanter(Lines[1]));
				TestEqual(TEXT("the Publisher first"), JamVoiceRules::SpeakerOf(Lines[0]), EJamSide::Boss);
				TestEqual(TEXT("Ronin answers"), JamVoiceRules::SpeakerOf(Lines[1]), EJamSide::Hero);
			}
			else
			{
				TestTrue(TEXT("a single line is one of the old barks"), JamVoiceRules::IsBark(Lines[0]));
			}
		}
		TestEqual(TEXT("every pair once"), Pairs, JamVoiceRules::BanterCount);
		TestEqual(TEXT("no repeat before all have played"), Seen.Num(), JamVoiceRules::FillerCount);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVoiceBanterCutTest, "Jam.Voice.EventLinesCutTheBanterOff", JamTest::Flags)

bool FJamVoiceBanterCutTest::RunTest(const FString& Parameters)
{
	const EJamLine Banter[] = { EJamLine::Banter1Boss, EJamLine::Banter8Hero };
	for (const EJamLine Saying : Banter)
	{
		TestTrue(TEXT("a cut's exchange"), JamVoiceRules::CutsOff(Saying, EJamLine::Cut2Boss));
		TestTrue(TEXT("a steal comment"), JamVoiceRules::CutsOff(Saying, EJamLine::StealParry));
		TestTrue(TEXT("the stun"), JamVoiceRules::CutsOff(Saying, EJamLine::StunBoss));
		TestTrue(TEXT("the finale"), JamVoiceRules::CutsOff(Saying, EJamLine::FinalHero));
		TestFalse(TEXT("its own answer goes on"), JamVoiceRules::CutsOff(Saying, EJamLine::Banter3Hero));
		TestFalse(TEXT("a bark waits its turn"), JamVoiceRules::CutsOff(Saying, EJamLine::Bark2));
	}
	TestFalse(TEXT("an event line is never cut off by another"), JamVoiceRules::CutsOff(EJamLine::Cut1Boss, EJamLine::StealShuriken));
	TestEqual(
		TEXT("the pairs are voiced in order"), FString(JamVoiceRules::NameOf(EJamLine::Banter1Boss)), FString(TEXT("vo_banter_1_pub")));
	TestEqual(TEXT("and answered"), FString(JamVoiceRules::NameOf(EJamLine::Banter8Hero)), FString(TEXT("vo_banter_8_ron")));
	return true;
}

#endif
