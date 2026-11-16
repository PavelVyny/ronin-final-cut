#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JamMusic.generated.h"

class UAudioComponent;

/**
 * The music in layers: four looping stems of the same length start together and stay running, and a cut only fades
 * one of them out. With no stem left the proof-of-concept metronome fades in. How many stems play is decided by
 * JamLayerRules::StemsFor; this actor only fades.
 *
 * The sounds are /Game/Audio/Music/mus_main_<n>_<name> and mus_poc_loop: stubs from Tools/Audio/make_stub_stems.py until
 * Kirill's files replace them under the same names.
 *
 * T-094: under the comic and the intro a soft ambient loop (mus_ambient) plays instead of the theme; SetTheme sets how
 * far the theme is up over it (the game mode crossfades it with JamIntroRules::Crossfade).
 */
UCLASS()
class JAMGAME_API AJamMusic : public AActor
{
	GENERATED_BODY()

public:
	AJamMusic();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The number of stems that should play, from 0 (the metronome) to all of them. */
	void SetStems(int32 InStems) { Stems = InStems; }

	/** The player's music volume from the menu (T-092), 0 to 1, on top of the stems' own loudness. */
	void SetGain(float InGain);

	/** How far the theme (the stems as SetStems says) is up over the ambient loop, 0 to 1. */
	void SetTheme(float Level);

private:
	UAudioComponent* AddTrack(const TCHAR* Name, const TCHAR* Path);

	/** Every track's loudness from its own fade, the theme's crossfade and the player's gain. */
	void ApplyVolumes();

	/** Drums, bass, pad, lead: the order in which they survive the cuts. */
	UPROPERTY()
	TArray<TObjectPtr<UAudioComponent>> Tracks;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Metronome;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Ambient;

	/** Current loudness of each track and of the metronome, last. */
	TArray<float> Volumes;

	int32 Stems = 0;
	float Gain = 1.f;
	/** How far the theme is up over the ambient, 0 to 1; the menu starts on the theme. */
	float Theme = 1.f;
};
