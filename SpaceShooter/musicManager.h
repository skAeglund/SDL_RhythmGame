#pragma once
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
	// note active states - decides when certain input actions are acceptable
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

	MusicManager(Beat inputBeats[NUMBER_OF_BEATS]);
	bool update(float deltaTime);
	void startPlaying();
	void stopPlaying() const;
	void changeBeat(int index = -1);
	bool getBeatActiveState(float timeSinceLastBeat, float noteLength) const;

	void playLaserSound(bool successfulLaser) const;
	void playGlitchSound() const;

	void printStats() const;
	void unload() const;
};
