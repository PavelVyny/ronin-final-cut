#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/JamTestHelpers.h"
#include "UI/JamComicRules.h"
#include "UI/JamTexts.h"

/**
 * T-093: the opening comic after NEW GAME: five panels, the camera moving over each, the voiced lines in balloons.
 * A bug looks in game like "a panel cuts the line short", "the screen shows past the picture's edge", "the picture is
 * squashed", "a balloon runs off the screen" or "the skip key of the menu skips the comic too".
 *
 * Run: Automation RunTests Jam.Comic
 */
namespace
{
constexpr float WideScreen = 16.f / 9.f;
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicStoryTest, "Jam.Comic.FivePanelsTellTheCallInOrder", JamTest::Flags)

bool FJamComicStoryTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("five panels"), JamComicRules::PanelCount, 5);
	TestEqual(TEXT("the room and the call are silent"), JamComicRules::PanelOf(0).Lines.Num() + JamComicRules::PanelOf(1).Lines.Num(), 0);
	TestTrue(TEXT("ship today, and the dev's answer"),
		JamComicRules::PanelOf(2).Lines == TArray<EJamLine>({ EJamLine::ComicBoss1, EJamLine::ComicHero1 }));
	TestTrue(TEXT("enough, I'll cut it"), JamComicRules::PanelOf(3).Lines == TArray<EJamLine>({ EJamLine::ComicBoss2 }));
	TestTrue(TEXT("every pixel"), JamComicRules::PanelOf(4).Lines == TArray<EJamLine>({ EJamLine::ComicHero2 }));
	TestEqual(TEXT("the lines' files"), FString(JamVoiceRules::NameOf(EJamLine::ComicBoss2)), FString(TEXT("vo_comic_3_pub")));
	TestTrue(TEXT("the dev is the hero's voice"), JamVoiceRules::SpeakerOf(EJamLine::ComicHero1) == EJamSide::Hero);
	TestFalse(TEXT("no bark"), JamVoiceRules::IsBark(EJamLine::ComicBoss1));
	TestEqual(
		TEXT("a caption per panel, maybe empty"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::ComicCaptions)), JamComicRules::PanelCount);
	TestEqual(
		TEXT("a sound word per panel, maybe empty"), static_cast<int32>(UE_ARRAY_COUNT(JamTexts::ComicSounds)), JamComicRules::PanelCount);
	float Least = 0.f;
	for (int32 Index = 0; Index < JamComicRules::PanelCount; ++Index)
	{
		Least += JamComicRules::PanelOf(Index).Seconds;
	}
	// The session-5s playtest asked the first panel to hold longer and the call to ring before it is answered: 25 s.
	TestTrue(FString::Printf(TEXT("short: %.1f s without the voice"), Least), Least >= 10.f && Least <= 25.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicWaitTest, "Jam.Comic.APanelWaitsForItsLines", JamTest::Flags)

bool FJamComicWaitTest::RunTest(const FString& Parameters)
{
	const float Seconds = JamComicRules::PanelOf(2).Seconds;
	TestFalse(TEXT("not before its time"), JamComicRules::IsPanelDone(2, 0.5f * Seconds, false));
	TestFalse(TEXT("not while a line is said"), JamComicRules::IsPanelDone(2, Seconds + 3.f, true));
	TestTrue(TEXT("once both are over"), JamComicRules::IsPanelDone(2, Seconds, false));
	TestFalse(TEXT("the menu's Enter does not skip it"), JamComicRules::MaySkip(0.f));
	TestTrue(TEXT("a moment later a key does"), JamComicRules::MaySkip(JamComicRules::SkipAfter));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicCropTest, "Jam.Comic.TheCameraStaysInsideThePicture", JamTest::Flags)

bool FJamComicCropTest::RunTest(const FString& Parameters)
{
	FJamComicPanel Panel;
	Panel.Aspect = 1.f;
	Panel.Seconds = 2.f;
	Panel.From = FVector2D(0.5, 0.3);
	Panel.FromZoom = 1.f;
	Panel.To = FVector2D(0.7, 0.4);
	Panel.ToZoom = 2.f;
	const FBox2D Start = JamComicRules::Crop(Panel, 0.f, WideScreen);
	TestTrue(TEXT("inside the picture"), Start.Min.X >= 0.0 && Start.Min.Y >= 0.0 && Start.Max.X <= 1.0 && Start.Max.Y <= 1.0);
	TestTrue(TEXT("a square picture on a wide screen: its full width"), FMath::IsNearlyEqual(Start.GetSize().X, 1.0, 1e-4));
	TestTrue(TEXT("the screen's shape, not squashed"),
		FMath::IsNearlyEqual(Start.GetSize().X * Panel.Aspect / Start.GetSize().Y, static_cast<double>(WideScreen), 1e-3));
	const FBox2D End = JamComicRules::Crop(Panel, Panel.Seconds, WideScreen);
	TestTrue(TEXT("zoomed in twice"), FMath::IsNearlyEqual(End.GetSize().X, 0.5 * Start.GetSize().X, 1e-4));
	TestTrue(TEXT("on its target"), End.GetCenter().Equals(Panel.To, 1e-3));
	const FBox2D Later = JamComicRules::Crop(Panel, 3.f * Panel.Seconds, WideScreen);
	TestTrue(TEXT("holds still while the line runs on"), Later.GetCenter().Equals(End.GetCenter(), 1e-6));
	const FBox2D Mid = JamComicRules::Crop(Panel, 0.5f * Panel.Seconds, WideScreen);
	TestTrue(TEXT("on its way"), Mid.GetCenter().X > Start.GetCenter().X && Mid.GetCenter().X < End.GetCenter().X);
	Panel.Aspect = 3.f;
	const FBox2D Tall = JamComicRules::Crop(Panel, 0.f, WideScreen);
	TestTrue(TEXT("a picture wider than the screen: its full height"), FMath::IsNearlyEqual(Tall.GetSize().Y, 1.0, 1e-4));
	for (int32 Index = 0; Index < JamComicRules::PanelCount; ++Index)
	{
		const FJamComicPanel& Each = JamComicRules::PanelOf(Index);
		for (float Age = 0.f; Age <= Each.Seconds; Age += 0.25f)
		{
			const FBox2D Shown = JamComicRules::Crop(Each, Age, WideScreen);
			TestTrue(TEXT("every panel stays inside its picture"),
				Shown.Min.X >= -1e-6 && Shown.Min.Y >= -1e-6 && Shown.Max.X <= 1.0 + 1e-6 && Shown.Max.Y <= 1.0 + 1e-6);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicSlideTest, "Jam.Comic.APanelSlidesInAndSettles", JamTest::Flags)

bool FJamComicSlideTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("off screen as it starts"), JamComicRules::SlideIn(0.f), 0.f);
	TestEqual(TEXT("in place after the slide"), JamComicRules::SlideIn(JamComicRules::SlideSeconds), 1.f);
	TestTrue(TEXT("on its way"), JamComicRules::SlideIn(0.5f * JamComicRules::SlideSeconds) > 0.f);
	TestEqual(TEXT("and stays"), JamComicRules::SlideIn(5.f), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicBalloonTest, "Jam.Comic.BalloonsFitOnTheScreen", JamTest::Flags)

bool FJamComicBalloonTest::RunTest(const FString& Parameters)
{
	const TArray<FString> Lines = JamComicRules::Wrap(TEXT("Enough! No time, no budget. I'll cut what we don't need... personally."), 24);
	TestTrue(TEXT("more than one row"), Lines.Num() > 1);
	FString Joined;
	for (const FString& Line : Lines)
	{
		TestTrue(TEXT("no row too long"), Line.Len() <= 24);
		TestFalse(TEXT("no empty row"), Line.IsEmpty());
		Joined += (Joined.IsEmpty() ? TEXT("") : TEXT(" ")) + Line;
	}
	TestEqual(
		TEXT("every word kept, in order"), Joined, FString(TEXT("Enough! No time, no budget. I'll cut what we don't need... personally.")));
	TestEqual(TEXT("a word longer than a row stands alone"), JamComicRules::Wrap(TEXT("a Supercalifragilistic b"), 5).Num(), 3);
	for (int32 Index = 0; Index < JamComicRules::PanelCount; ++Index)
	{
		for (const EJamLine Line : JamComicRules::PanelOf(Index).Lines)
		{
			const FVector2D At = JamComicRules::BalloonAt(Line);
			TestTrue(TEXT("each balloon on the screen"), At.X > 0.1 && At.X < 0.9 && At.Y > 0.0 && At.Y < 0.5);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicFrameTest, "Jam.Comic.TheComicKeepsItsShapeOnAnyScreen", JamTest::Flags)

bool FJamComicFrameTest::RunTest(const FString& Parameters)
{
	// T-094: on an ultrawide screen the comic stretched across all of it and the eye had to hunt for the balloons.
	const FBox2D Usual = JamComicRules::Frame(FVector2D(1920.0, 1080.0));
	TestTrue(
		TEXT("a 16:9 screen is all comic"), Usual.Min.Equals(FVector2D(0.0, 0.0), 0.5) && Usual.Max.Equals(FVector2D(1920.0, 1080.0), 0.5));
	const FBox2D Ultra = JamComicRules::Frame(FVector2D(3440.0, 1440.0));
	TestTrue(TEXT("an ultrawide one gets the comic's own shape"),
		FMath::IsNearlyEqual(Ultra.GetSize().X / Ultra.GetSize().Y, static_cast<double>(JamComicRules::FrameAspect), 1e-3));
	TestTrue(TEXT("the full height"), FMath::IsNearlyEqual(Ultra.GetSize().Y, 1440.0, 0.5));
	TestTrue(TEXT("centred, bars at both sides"), FMath::IsNearlyEqual(Ultra.Min.X, 3440.0 - Ultra.Max.X, 0.5) && Ultra.Min.X > 0.0);
	const FBox2D Square = JamComicRules::Frame(FVector2D(1024.0, 768.0));
	TestTrue(TEXT("a narrower screen is all comic"),
		Square.Min.Equals(FVector2D(0.0, 0.0), 0.5) && Square.Max.Equals(FVector2D(1024.0, 768.0), 0.5));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicFirstPanelTest, "Jam.Comic.TheFirstPanelPushesInSlowly", JamTest::Flags)

bool FJamComicFirstPanelTest::RunTest(const FString& Parameters)
{
	// The session-5s playtest: the first panel went by too fast, and the camera jerked left and back as it started.
	const FJamComicPanel& First = JamComicRules::PanelOf(0);
	// The author's play-through: 5 s of typing still dragged; about 2 s before the phone rings.
	TestTrue(TEXT("long enough to see the room"), First.Seconds >= 1.5f);
	TestTrue(TEXT("and no longer"), First.Seconds <= 2.5f);
	TestTrue(TEXT("a push in"), First.ToZoom > First.FromZoom);
	TestTrue(TEXT("a slow one"), First.ToZoom - First.FromZoom <= 0.5f);
	TestFalse(TEXT("the first panel does not slide in"), JamComicRules::SlidesIn(0));
	TestTrue(TEXT("the others do"), JamComicRules::SlidesIn(1));
	TestEqual(TEXT("it fades up from black"), JamComicRules::FadeIn(0, 0.f), 0.f);
	TestEqual(TEXT("and is up soon"), JamComicRules::FadeIn(0, 1.f), 1.f);
	TestEqual(TEXT("the others need no fade"), JamComicRules::FadeIn(1, 0.f), 1.f);
	// No jerk: the frame's centre moves the same way all along, never left then back.
	double LastX = JamComicRules::Crop(First, 0.f, WideScreen).GetCenter().X;
	double Direction = 0.0;
	bool bTurned = false;
	for (float Age = 0.1f; Age <= First.Seconds; Age += 0.1f)
	{
		const double X = JamComicRules::Crop(First, Age, WideScreen).GetCenter().X;
		const double Step = X - LastX;
		bTurned |= FMath::Abs(Step) > 1e-4 && Direction * Step < 0.0;
		Direction = FMath::Abs(Step) > 1e-4 ? Step : Direction;
		TestTrue(TEXT("no sudden move"), FMath::Abs(Step) < 0.01);
		LastX = X;
	}
	TestFalse(TEXT("the camera never turns back"), bTurned);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicSoundTest, "Jam.Comic.TheRingComesBeforeTheCall", JamTest::Flags)

bool FJamComicSoundTest::RunTest(const FString& Parameters)
{
	// Typing under the dev at work, then the phone rings, and only after the ring does the Publisher speak.
	TestEqual(TEXT("the dev types"), JamComicRules::PanelOf(0).Sound, EJamComicSound::Typing);
	TestEqual(TEXT("the call rings"), JamComicRules::PanelOf(1).Sound, EJamComicSound::Ring);
	// The author's play-through: the gauntlet smashing through the monitor shatters glass as the panel appears.
	TestEqual(TEXT("the screen shatters"), JamComicRules::PanelOf(3).Sound, EJamComicSound::Shatter);
	TestTrue(TEXT("the ringing panel outlasts the ring"), JamComicRules::PanelOf(1).Seconds >= JamComicRules::RingSeconds);
	TestEqual(TEXT("nobody speaks over the ring"), JamComicRules::PanelOf(1).Lines.Num(), 0);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		TestEqual(TEXT("nobody speaks before the ring"), JamComicRules::PanelOf(Index).Lines.Num(), 0);
	}
	TestEqual(TEXT("the Publisher's line opens the panel after the ring"), JamComicRules::PanelOf(2).Lines[0], EJamLine::ComicBoss1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicGlassTest, "Jam.Comic.TheGlassIsHeardUnderTheVoice", JamTest::Flags)

bool FJamComicGlassTest::RunTest(const FString& Parameters)
{
	// T-100: the author heard no glass as the gauntlet's panel came up. It plays once with that panel, quiet under the
	// Publisher's line but clearly heard.
	TestEqual(TEXT("the gauntlet's panel brings the glass"), JamComicRules::PanelOf(3).Sound, EJamComicSound::Shatter);
	const float Glass = JamComicRules::SoundVolume(EJamComicSound::Shatter);
	TestTrue(TEXT("clearly heard"), Glass >= 0.4f);
	TestTrue(TEXT("under the voice"), Glass < 0.5f * JamVoiceRules::Boost);
	TestEqual(TEXT("the keyboard as recorded"), JamComicRules::SoundVolume(EJamComicSound::Typing), 1.f);
	TestEqual(TEXT("the ring as recorded"), JamComicRules::SoundVolume(EJamComicSound::Ring), 1.f);
	TestEqual(TEXT("no sound, no volume"), JamComicRules::SoundVolume(EJamComicSound::None), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicCookTest, "Jam.Comic.ItsSoundsAreCooked", JamTest::Flags)

bool FJamComicCookTest::RunTest(const FString& Parameters)
{
	// T-100: AJamSounds loads its sounds by path, so nothing on the map references them; the glass added in T-097 was
	// left out of the packaged build. The packaging always cooks the folders the game loads by path.
	TArray<FString> Folders;
	GConfig->GetArray(TEXT("/Script/UnrealEd.ProjectPackagingSettings"), TEXT("DirectoriesToAlwaysCook"), Folders, GGameIni);
	for (const TCHAR* Path : { TEXT("/Game/Audio"), TEXT("/Game/UI"), TEXT("/Game/Props"), TEXT("/Game/Arena"), TEXT("/Game/FX") })
	{
		TestTrue(FString::Printf(TEXT("%s is always cooked"), Path),
			Folders.ContainsByPredicate([Path](const FString& Entry) { return Entry.Contains(FString::Printf(TEXT("\"%s\""), Path)); }));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
