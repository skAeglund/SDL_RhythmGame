#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#define NUMBER_OF_BEATS 3
#include "delegate.h"

struct Beat
{
	int BPM = 114;
	int timeSignature = 4; // 4/4 - lower signature assumed to be 4
	const char* path;
	Mix_Music* music;

	Beat(int bpm=120, int signature=4, const char* path ="asd");
};
struct MusicData
{
	bool wholeNoteActive = false;
	bool halfNoteActive = false;
	bool quarterNoteActive = false;

	double timeSinceLastWholeNote = 0;
	double timeSinceLastHalfNote = 0;
	double timeSinceLastQuarterNote = 0;

	float halfNoteProgress;
	float wholeNoteProgress;
	float quarterNoteProgress;

	double wholeNoteLength;
	double halfNoteLength;
	double quarterNoteLength;

	int currentQuarterNote = 1;

	// start = 1, middle = 0, end = 1 (over 1 beat)
	float pulseMultiplier;
};


struct MusicManager
{
	Beat currentBeat;
	MusicData* data;
	Beat beats[NUMBER_OF_BEATS]; //todo: make dynamic

	Mix_Chunk* transitionSound;
	Mix_Chunk* laserSounds[4];
	Mix_Chunk* glitchSound;

	int currentBeatIndex = 0;
	
	double acceptedOffset = 0.1;

	Delegate<std::function<void()>> onQuarterNote;

	double loadTime;
	bool isLoading = true;
	bool isChangingBeat = false;

	inline static bool isTransitioning;

	bool initialize(Beat beats[]);
	bool update(double deltaTime);
	void startPlaying();
	void stopPlaying();
	void changeBeat(int index = -1);
	double wholeNoteLength() { return (quarterNoteLength() * currentBeat.timeSignature); };
	double halfNoteLength() { return quarterNoteLength() * 2; };
	double quarterNoteLength() { return (60 / (double)currentBeat.BPM); };
	float wholeNoteProgress() { return (data->timeSinceLastWholeNote / wholeNoteLength()); };
	float halfNoteProgress() { return (data->timeSinceLastHalfNote / halfNoteLength()); };
	float quarterNoteProgress() { return (data->timeSinceLastQuarterNote / quarterNoteLength()); };


	void playLaserSound();
	void playGlitchSound();
	void printStats();
	

	void destroy();
};

MusicManager* getMusicManager();

MusicData* getMusicData();
void setMusicManager(MusicManager* musicManager);



