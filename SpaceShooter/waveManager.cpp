#include "waveManager.h"

//Wave waves[6] =
//{
//	Wave(4, 2, 8),
//	Wave(2, 4, 8),
//	Wave(2, 4, 16),
//	Wave(2, 8, 25),
//	Wave(2, 2, 2),
//	Wave(1, 4, 16)
//};


void WaveManager::pause()
{
	getMusicManager()->onQuarterNote -= &WaveManager::onQuarterNote;
	activated = false;
}
void  WaveManager::resetWaves()
{
	for (Wave& wave : waves)
	{
		wave.bigRockHasBeenSpawned = false;
		wave.spawnCount = 0;
	}

}
void WaveManager::restart()
{
	resetWaves();
	quarterNoteCount = 0;
	waveIndex = 0;
	level = 1;
	waitingForLevelChange = false;
	start();
}

void WaveManager::start()
{
	getMusicManager()->onQuarterNote += &WaveManager::onQuarterNote;
	//activated = false; // wait until main beat to activate
	quarterNoteCount = 0;
}


void WaveManager::onQuarterNote()
{
	if (!activated)
	{
		if (waitingForLevelChange && getObjectCount() <= 1)
		{
			getMusicManager()->changeBeat();
			waitingForLevelChange = false;
			return;
		}
		if (getMusicData()->currentQuarterNote == 2 && !waitingForLevelChange && !MusicManager::isTransitioning)
		{
			activated = true;
			//spawnAsteroid();
			//waves[waveIndex].spawnCount++;
		}
		else return;
	}
	quarterNoteCount++;
	if (waves[waveIndex].spawnCount == 0 && quarterNoteCount == 1 && getMusicData()->currentQuarterNote == 2)
	{
		spawnAsteroid();
		waves[waveIndex].spawnCount++;
	}
	if (quarterNoteCount > waves[waveIndex].quarterNoteInterval ||
		(waves[waveIndex].spawnCount == 0 && quarterNoteCount == 1))
	{

		quarterNoteCount = 1;
		waves[waveIndex].spawnCount++;

		spawnAsteroid();
		if (waves[waveIndex].spawnCount >= waves[waveIndex].totalSpawns)
		{
			//waves[waveIndex].spawnCount = 0;
			quarterNoteCount = -waves[waveIndex].delayUntilNextWave + 1;
			waveIndex++;
			if (waveIndex > sizeof(waves) / sizeof(waves[0]) - 1)
			{
				level++;

				waveIndex = 0;
				resetWaves();
				//getMusicManager()->changeBeat(1);
				activated = false;
				waitingForLevelChange = true;
				quarterNoteCount = 0;
				// change music
			}
		}
	}
}

void WaveManager::spawnAsteroid()
{

	if (Player::remainingHealth <= 0) return;

	bool forceBigSpawn = waves[waveIndex].spawnCount == waves[waveIndex].totalSpawns &&
		!waves[waveIndex].bigRockHasBeenSpawned && waveIndex > 1;

	bool forceSmallSize = waves[waveIndex].bigRockHasBeenSpawned || waveIndex < 2;

	int randomSeed = rand() % 100;
	int x = rand() % (WIDTH - WIDTH / 3) + WIDTH / 6;
	float xVelocity = (float)(rand() % 80 - 40);
	float yVelocity = (float)(rand() % (10 + 20 * level) + 20 * level);
	float size = forceSmallSize ? rand() % (MIN_SIZE_WHOLENOTE - MIN_SIZE_HALFNOTE - 4) + MIN_SIZE_HALFNOTE + 5 :
		forceBigSpawn ? MIN_SIZE_WHOLENOTE + 1 : rand() % 30 + MIN_SIZE_HALFNOTE + 1;
	if (size > MIN_SIZE_WHOLENOTE)
	{
		size *= 1.33f;
		waves[waveIndex].bigRockHasBeenSpawned = true;
	}
	int randomTorque = rand() % 30 - 15;
	int randomAngle = rand() % 360;
	//Position pos(x, -size, size);
	Position pos(x, rand() % 150 + 50, size);
	Rotation rot(randomTorque, randomAngle);
	Velocity vel(xVelocity, yVelocity);
	createObject(pos, rot, vel);
}
