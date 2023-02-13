#include "waveManager.h"
#include "engine.h"
#include "player.h"

using namespace Engine;

void WaveManager::initialize(MusicManager* musicManager, int starCount)
{
	WaveManager::musicManager = musicManager;

	for (int i = 0; i < starCount; i++)
	{
		WaveManager::spawnStar();
	}
}
void WaveManager::pause()
{
	if (musicManager != nullptr)
	{
		musicManager->onQuarterNote -= &WaveManager::onQuarterNote;
		musicManager->onQuarterNote -= &WaveManager::spawnStar;
	}

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
	if (musicManager != nullptr)
	{
		musicManager->onQuarterNote += &WaveManager::onQuarterNote;
		musicManager->onQuarterNote += &WaveManager::spawnStar;
	}
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
			if (waveIndex > std::size(waves)/*sizeof(waves) / sizeof(waves[0]) - 1*/)
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
	const bool forceBigSpawn = waves[waveIndex].spawnCount == waves[waveIndex].totalSpawns &&
		!waves[waveIndex].bigRockHasBeenSpawned && waveIndex > 1;

	const bool forceSmallSize = waves[waveIndex].bigRockHasBeenSpawned || waveIndex < 2;

	const int x = rand() % (WIDTH - WIDTH / 3) + WIDTH / 6;
	const int xVelocity = rand() % 80 - 40;
	const int yVelocity = HEIGHT == 1080 ? rand() % (10 + 20 * level) + 20 * level : rand() % (5 + 5 * level) + 5 * level;
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

	const Position pos(static_cast<float>(x), static_cast<float>(rand() % 150 + 50), size);
	const Rotation rot(static_cast<float>(randomTorque), static_cast<float>(randomAngle));
	const Velocity vel(static_cast<float>(xVelocity), static_cast<float>(yVelocity));
	Engine::createObject(pos, rot, vel);
}

void WaveManager::spawnStar()
{
	if (quarterNoteCount % 2 == 0)
	{
		float heightLimit = 0;
		for (int i = 0; i < 10; i++)
		{
			// spawn non-blinking longlived star
			const float randomX = static_cast<float>(rand() % WIDTH);
			const float randomY = static_cast<float>(rand() % HEIGHT -heightLimit);
			const float maxSize = 1 + static_cast<float>(rand() % 100) * 0.01f;
			const float randAlpha = static_cast<float>(rand() % 200);
			Engine::createStar(randomX, randomY, maxSize, Color(randAlpha, 225.f, 255.f, randAlpha), 10);
			if (randomY > HEIGHT - 150) heightLimit = 150;
		}
		return;
	}
	for (int i = 0; i < 5; i++)
	{
		const int rColor = rand() % 100;
		int rRed = rand() % 80 + 20;
		const int rAlpha = rand() % 75 + 180;
		const Color color =
			rColor < 35 ? Color(100, 150, 255, rAlpha) :
			rColor < 70 ? Color(100, 200, 255, rAlpha) :
			rColor < 90 ? Color(200, 255, 255, rAlpha) : Color(255, 100, 100, rAlpha);

		// spawn blinking shortlived star
		const float randomX = static_cast<float>(rand() % (WIDTH - 100) + 50);
		const float randomY = static_cast<float>(rand() % (HEIGHT-150) + 50);
		const float maxSize = 1 + static_cast<float>(rand() % 300) * 0.01f;
		const int lifeTime = rand() % 4 + 1;
		Engine::createStar(randomX, randomY, maxSize, color, lifeTime);
	}
}

