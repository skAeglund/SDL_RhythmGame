#pragma once
#include "musicManager.h"
#define WAVECOUNT 6

struct Wave
{
	int quarterNoteInterval; // how many quarter notes between spawns
	int totalSpawns;
	int delayUntilNextWave; // how many quarter notes until next wave begins
	int spawnCount;
	bool bigRockHasBeenSpawned;
};
/// <summary>
/// This handles the spawning of asteroids, in synch with music.
/// 
/// Everything being static is a workaround to make the function
/// "onQuarterNote()" work with a custom delegate (see musicManager.h onQuarterNote).
///
///	If I had more time, I'd try to make the delegate work with non-static methods
/// </summary>
struct WaveManager
{
private:
	inline static int waveIndex;
	inline static int level;
	inline static int quarterNoteCount;
	inline static bool activated;
	inline static bool waitingForLevelChange;
	inline static Wave waves[WAVECOUNT] = {
		Wave(4, 2, 8),
		Wave(2, 4, 8),
		Wave(2, 4, 16),
		Wave(2, 8, 25),
		Wave(2, 2, 2),
		Wave(1, 4, 16)
	};
	inline static MusicManager* musicManager;

public:
	static void initialize(MusicManager* musicManager, int starCount);
	static void start();
	static void pause();
	static void restart();
	static void spawnStar();

private:
	static void spawnAsteroid();
	static void onQuarterNote();
	static void resetWaves();
};