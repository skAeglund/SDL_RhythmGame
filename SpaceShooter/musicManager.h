#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include "delegate.h"
#define NUMBER_OF_BEATS 3

struct Beat
{
	int BPM = 114;
	int timeSignature = 4; // beats per bar
	const char* path = "";
	Mix_Music* music = nullptr;

	Beat(int bpm=120, int signature=4, const char* path ="");
};

// contains data used for rhythm game features
struct MusicData
{
	bool wholeNoteActive = false;
	bool halfNoteActive = false;
	bool quarterNoteActive = false;

	float timeSinceLastWholeNote{};
	float timeSinceLastHalfNote{};
	float timeSinceLastQuarterNote{};

	// start of note = 0, end of note = 1
	float halfNoteProgress{};
	float wholeNoteProgress{};
	float quarterNoteProgress{};

	float wholeNoteLength{};
	float halfNoteLength{};
	float quarterNoteLength{};

	int currentQuarterNote = 1;

	// a multiplier that's synched with halfnotes
	// start = 1, middle = 0, end = 1
	float pulseMultiplier{};
};

// handles all audio / music 
struct MusicManager
{
	MusicData* data = nullptr;
	inline static bool isTransitioning = false;
	
private:
	Beat currentBeat{};
	Beat beats[NUMBER_OF_BEATS]{};
	Mix_Chunk* transitionSound = nullptr;
	Mix_Chunk* laserSounds[4] = { nullptr };
	Mix_Chunk* badLaserSound = nullptr;
	Mix_Chunk* glitchSound = nullptr;
	
	int currentBeatIndex = 0;
	float acceptedOffset = 0.1f;
	float loadTime = 0.2f;
	bool isLoading = true;
	bool isChangingBeat = false;
	int glitchChannel = 0;
	int transitionChannel = 1;

public:
	Delegate<std::function<void()>> onQuarterNote;

	bool initialize(Beat inputBeats[NUMBER_OF_BEATS]);
	bool update(float deltaTime);
	void startPlaying();
	void stopPlaying();
	void changeBeat(int index = -1);

	void playLaserSound(bool successfulLaser);
	void playGlitchSound();

	void printStats();
	void unload();
};
