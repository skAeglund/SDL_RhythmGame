#include "input.h"
#include "musicManager.h"


void handleInputEvents(Player& player, MusicManager& musicManager, bool& gameRunning, bool& gamePaused, float deltaTime)
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
				// shoot ability inactivated for now
				//player.shoot(event.button.x, event.button.y); 
			}
			else if (event.button.button == InputKey::laser)
			{
				bool successfulShot = false;
				player.shootLaser(event.button.x, event.button.y, musicManager.data, successfulShot);
				musicManager.playLaserSound(successfulShot);
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

Button handlePausedInput()
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
			//buttonPressed = buttons.isIntersectingAnyButton(mouse_x, mouse_y);
		}
	}
	return buttonPressed;
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