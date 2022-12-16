#include "input.h"
#include "musicManager.h"


void handleInputEvents(Player& player, bool& gameRunning, bool& gamePaused, float deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		int scanCode = event.key.keysym.scancode;
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event.button.button == InputKey::shoot)
			{
				player.shoot(event.button.x, event.button.y);
				
			}
			else if (event.button.button == InputKey::laser)
			{
				player.laser(event.button.x, event.button.y);
				getMusicManager()->playLaserSound();
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
				//if (deltaTime < 0.05f)
				//	spawnAsteroids(5);
			}
			else if (scanCode == InputKey::changeBeat)
			{
				getMusicManager()->changeBeat();
			}

			updateKey(scanCode, true);
			break;
		}

		case SDL_KEYUP:
		{
			updateKey(scanCode, false);
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

Button pausedInputHandler()
{
	Button buttonPressed = Button::none;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		int scanCode = event.key.keysym.scancode;
		switch (event.type)
		{
		case SDL_QUIT:
			return Button::quit;
			break;
		case SDL_MOUSEBUTTONDOWN:

			int mouse_x, mouse_y;
			SDL_GetMouseState(&mouse_x, &mouse_y);
			buttonPressed = checkButtonAtPoint(mouse_x, mouse_y);
		}
	}
	return buttonPressed;
}