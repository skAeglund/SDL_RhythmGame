#include "input.h"
#include "musicManager.h"


void handleInputEvents(Player& player, MusicManager& musicManager, bool& gameRunning, bool& gamePaused, float deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		const int scanCode = event.key.keysym.scancode;
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.button == InputKey::laser)
				{
					bool successfulTiming;
					player.shootLaser(event.button.x, event.button.y, musicManager.data, successfulTiming);
					musicManager.playLaserSound(successfulTiming);
				}
				break;
			}
		case SDL_KEYDOWN:
			{
				if (scanCode == InputKey::pauseGame)
				{
					gamePaused = true;
					return;
				}
				else if (scanCode == InputKey::spawnAsteroid)
				{
					// spawn asteroid for debugging - inactivated
					// spawnAsteroids(5);
				}
				else if (scanCode == InputKey::changeBeat)
				{
					musicManager.changeBeat();
				}

				Engine::updateKey(scanCode, true);
				break;
			}

		case SDL_KEYUP:
			{
				Engine::updateKey(scanCode, false);
				break;
			}
		case SDL_QUIT:
			{
				gameRunning = false;
				break;
			}
		}
	}
}

bool isPressingMouseButton()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN)
			return true;
	}
	return false;
}