#include "waveManager.h"

#include "engine.h"

using namespace Engine;

void WaveManager::initialize(MusicManager* musicManager)
{
	WaveManager::musicManager = musicManager;
}
void WaveManager::pause()
{
	if (musicManager != nullptr)
		musicManager->onQuarterNote -= &WaveManager::onQuarterNote;

	activated = false;
}
void WaveManager::resetWaves()
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
	musicManager->onQuarterNote += &WaveManager::onQuarterNote;
	//activated = false; // wait until main beat to activate
	quarterNoteCount = 0;
}

void WaveManager::onQuarterNote()
{
	if (!activated)
	{
		if (waitingForLevelChange && Engine::getObjectCount() <= 1)
		{
			musicManager->changeBeat();
			waitingForLevelChange = false;
			return;
		}
		if (musicManager->data->currentQuarterNote == 2 && !waitingForLevelChange && !MusicManager::isTransitioning)
		{
			activated = true;
		}
		else return;
	}
	quarterNoteCount++;
	if (waves[waveIndex].spawnCount == 0 && quarterNoteCount == 1 && musicManager->data->currentQuarterNote == 2)
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
			quarterNoteCount = -waves[waveIndex].delayUntilNextWave + 1;
			waveIndex++;
			if (waveIndex > sizeof(waves) / sizeof(waves[0]) - 1)
			{
				level++;

				waveIndex = 0;
				resetWaves();
				activated = false;
				waitingForLevelChange = true;
				quarterNoteCount = 0;
			}
		}
	}
}

void WaveManager::spawnAsteroid()
{

	if (Player::remainingHealth <= 0) return;

	const bool forceBigSpawn = waves[waveIndex].spawnCount == waves[waveIndex].totalSpawns &&
		!waves[waveIndex].bigRockHasBeenSpawned && waveIndex > 1;

	const bool forceSmallSize = waves[waveIndex].bigRockHasBeenSpawned || waveIndex < 2;

	const int x = rand() % (WIDTH - WIDTH / 3) + WIDTH / 6;
	const int xVelocity = rand() % 80 - 40;
	const int yVelocity = rand() % (10 + 20 * level) + 20 * level;
	float size = static_cast<float>(
		forceSmallSize ? rand() % (MIN_SIZE_WHOLENOTE - MIN_SIZE_HALFNOTE - 4) + MIN_SIZE_HALFNOTE + 5 :
		forceBigSpawn ? MIN_SIZE_WHOLENOTE + 1 : rand() % 30 + MIN_SIZE_HALFNOTE + 1);

	if (size > MIN_SIZE_WHOLENOTE)
	{
		size *= 1.33f;
		waves[waveIndex].bigRockHasBeenSpawned = true;
	}
	const int randomTorque = rand() % 30 - 15;
	const int randomAngle = rand() % 360;
	//Position pos(x, -size, size);
	const Position pos(static_cast<float>(x), static_cast<float>(rand() % 150 + 50), size);
	const Rotation rot(static_cast<float>(randomTorque), static_cast<float>(randomAngle));
	const Velocity vel(static_cast<float>(xVelocity), static_cast<float>(yVelocity));
	Engine::createObject(pos, rot, vel);
}
