// Made by: Anders Hägglund, 2022

// Note: This was an assignment in data-oriented programming
// which is why a data-oriented approach was used in some cases
// where an object oriented approach probably would make more sense

#include <algorithm>
#include <iostream>
#include <map>
#include <SDL.h>
#include "assets.h"
#include "engine.h"
#include "player.h"
#include "sprite.h"
#include "crosshair.h"
#include "input.h"
#include "waveManager.h"
#include "UI.h"
#include "rendering.h"

using namespace std;
using namespace Engine;

UI::Button waitForButtonPress(UI::Menu menu, MusicManager& musicManager, Crosshair& crosshair, int playerHealth);

int main(int argc, char** args)
{
	Engine::initializeEngine(Assets::texturePaths, size(Assets::texturePaths));
	
	
	Crosshair crosshair(25, 25, Assets::crosshairTexturePath, Rendering::getRenderer());

	MusicManager musicManager{ Assets::beats };
	Player player{ &musicManager , Rendering::getRenderer() };

	WaveManager::initialize(&musicManager, 15);

	UI::Button buttonPressed = waitForButtonPress(UI::Menu::intro, musicManager, crosshair, player.remainingHealth);
	bool gameRunning = buttonPressed != UI::Button::quit;
	bool gamePaused = false;

	musicManager.startPlaying();
	WaveManager::start();

	// MAIN GAME LOOP
	while (gameRunning)
	{
		const float deltaTime = Engine::updateTicks();
		Engine::updateObjects(musicManager.data->wholeNoteLength);
		Engine::printTimeStats();

		musicManager.update(deltaTime);
		musicManager.printStats();

		Rendering::renderClear();
		Engine::drawEverything(musicManager.data, player.remainingHealth);
		Engine::checkForObjectDestruction(&player);
		
		player.update(deltaTime, musicManager.data->pulseMultiplier);
		crosshair.draw(Rendering::getRenderer(), musicManager.data->quarterNoteProgress, player.timeSinceLastFail);

		handleInputEvents(player, musicManager, gameRunning, gamePaused, deltaTime);
		if (gamePaused)
		{
			WaveManager::pause();
			buttonPressed = waitForButtonPress(UI::Menu::pause, musicManager, crosshair, player.remainingHealth);
			gameRunning = buttonPressed != UI::Button::quit;
			gamePaused = false;
			if (gameRunning)
			{
				WaveManager::start();
			}
		}
		else if (player.remainingHealth <= 0)
		{
			WaveManager::pause();
			buttonPressed = waitForButtonPress(UI::Menu::gameOver, musicManager, crosshair, player.remainingHealth);
			gameRunning = buttonPressed != UI::Button::quit;
			if (!gameRunning) break;
			
			player.reset();
			musicManager.changeBeat(0);
			WaveManager::restart();
			Engine::resetKeys();
		}

		Rendering::renderPresent();
		Engine::delayNextFrame();
	}
	Engine::unloadTextures();
	musicManager.unload();
	crosshair.destroy();
	player.destroy();
	cout << "Quitting ..." << endl;
	Engine::quit();

	return 0;
}

// Loops during intro, pause and game over menu until player presses a button
UI::Button waitForButtonPress(UI::Menu menu, MusicManager& musicManager, Crosshair& crosshair, int playerHealth)
{
	SDL_Renderer* renderer = Rendering::getRenderer();
	UI::Buttons buttons(renderer, menu);

	Sprite overlay(WIDTH, HEIGHT);
	overlay.load(Assets::menuOverlayPaths.at(menu), Rendering::getRenderer());
	UI::Button buttonPressed = UI::Button::none;

	int mouseX, mouseY;
	float elapsedFadeTime = 0.f;
	const float fadeOutTime = 0.33f;
	float fadeOutProgress = 0;
	while (buttonPressed == UI::Button::none || elapsedFadeTime < fadeOutTime)
	{
		Rendering::renderClear();
		const float deltaTime = Engine::updateTicks();
		Engine::rotateObjects();
		Engine::updateObjectsLifetime(musicManager.data->wholeNoteLength);
		Engine::drawEverything(musicManager.data, playerHealth, true);
		musicManager.update(deltaTime);

		if (buttonPressed == UI::Button::none)
		{
			SDL_GetMouseState(&mouseX, &mouseY);
			if (isPressingMouseButton())
				buttonPressed = buttons.isIntersectingAnyButton(mouseX, mouseY);
		}
		else
		{
			elapsedFadeTime += deltaTime;
			fadeOutProgress = clamp(elapsedFadeTime / (fadeOutTime+0.05f), 0.f, 0.99f);
			overlay.updateOpacity(powf(1.f - fadeOutProgress,2));
		}

		overlay.draw(0, 0, WIDTH, HEIGHT, renderer);

		if (fadeOutProgress <= 0.001f)
			buttons.draw(renderer, mouseX, mouseY);
		else 
			buttons.draw(renderer, mouseX, mouseY, buttonPressed, 1 - fadeOutProgress);

		crosshair.draw(renderer, 1.f, 2.f);
		Rendering::renderPresent();
		SDL_Delay(16);
	}
	overlay.destroy();
	buttons.unloadTextures();
	return buttonPressed;
}

