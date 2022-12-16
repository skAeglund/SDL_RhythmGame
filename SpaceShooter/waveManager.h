#pragma once
#include "engine.h"
#include "player.h"
#include "musicManager.h"
#define WAVECOUNT 6

struct Wave
{
	int quarterNoteInterval; // how many quarternotes between spawns
	int totalSpawns;
	int delayUntilNextWave; // how many quarternotes until next wave begins
	int spawnCount;
	bool bigRockHasBeenSpawned;
};
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

public:
	void start();
	void pause();
	void restart();

private:
	static void spawnAsteroid();
	static void onQuarterNote();
	static void resetWaves();
};