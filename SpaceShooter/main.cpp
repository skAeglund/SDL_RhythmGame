// SpaceShooter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
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
#include "musicManager.h"
#include "waveManager.h"
#include "UI.h"

using namespace std;
using namespace Engine;
using namespace Collision;


ButtonType waitForButtonPress(MusicManager& musicManager, Crosshair& crosshair, ButtonType availButtons[], int bCount, Menu menu);

int main(int argc, char** args) {

	
	Engine::initializeEngine(Assets::texturePaths, size(Assets::texturePaths));
	
	Player player{ };
	Position playerPosition(Position(WIDTH / 2 - player.radius / 2, HEIGHT * 0.8f, 45));
	Engine::createObject(playerPosition, Rotation(10,0), Velocity(),  20, "Content/Sprites/player90x90.png", Tag::Player);
	
	Crosshair crosshair(25, 25, "Content/Sprites/crosshair.png", Engine::getRenderer());

	MusicManager musicManager;
	musicManager.initialize(Assets::beats);

	WaveManager::initialize(&musicManager);

	ButtonType availableButtons[] = { ButtonType::start, ButtonType::quit};
	ButtonType buttonPressed = waitForButtonPress(musicManager, crosshair, availableButtons, 2, Menu::intro);
	bool gameRunning = buttonPressed != ButtonType::quit;
	bool gamePaused = false;

	musicManager.startPlaying();
	WaveManager::start();

	while (gameRunning)
	{
		const float deltaTime = Engine::updateTicks();
		Engine::printTimeStats();

		musicManager.update(deltaTime);
		musicManager.printStats();

		Engine::renderClear();
		Engine::drawBackground();
		Engine::drawLasers(SDL_Color(0, 200, 255, 255));
		Engine::sortObjects();
		Engine::moveObjects();
		Engine::rotateObjects();
		Engine::drawObjects();
		Engine::drawHealthLine(musicManager.data);
		Engine::drawBeatCircles(musicManager.data);

		const int playerDamage = Engine::checkForObjectDestruction(musicManager);

		if (playerDamage > 0)
		{
			player.remainingHealth -= playerDamage;
			if (player.remainingHealth <= 0)
			{
				musicManager.stopPlaying();
				Engine::clearObjects();
			}
			musicManager.playGlitchSound();
		}
		
		//drawColliders(); // use this to show colliders in game
		
		player.update(deltaTime);
		crosshair.update();
		crosshair.draw(Engine::getRenderer());

		handleInputEvents(player, musicManager, gameRunning, gamePaused, deltaTime);
		if (gamePaused)
		{
			WaveManager::pause();
			ButtonType availableButtons[] = { ButtonType::resume, ButtonType::quit };
			buttonPressed = waitForButtonPress(musicManager, crosshair, availableButtons, 2, Menu::pause);
			gameRunning = buttonPressed != ButtonType::quit;
			gamePaused = false;
			if (gameRunning)
			{
				WaveManager::start();
			}
		}
		else if (player.remainingHealth <= 0)
		{
			WaveManager::pause();
			ButtonType availableButtons[] = { ButtonType::restart, ButtonType::quit };
			buttonPressed = waitForButtonPress(musicManager, crosshair, availableButtons, 2, Menu::gameOver);
			gameRunning = buttonPressed != ButtonType::quit;
			if (gameRunning)
			{
				player.remainingHealth = player.maxHealth;
				musicManager.changeBeat(0);
				WaveManager::restart();
			}
			Engine::resetKeys();
		}

		Engine::renderPresent();
		Engine::delayNextFrame();
	}
	Engine::unloadTextures();
	musicManager.unload();
	crosshair.destroy();
	cout << "Quitting ..." << endl;
	Engine::quit();

	return 0;
}

// returns false if the player decided to quit during the pause
ButtonType waitForButtonPress(MusicManager& musicManager, Crosshair& crosshair, ButtonType avaiablelButtons[], int bCount, Menu menu)
{

	Buttons buttons(getRenderer(), avaiablelButtons, bCount);

	Sprite overlay(WIDTH, HEIGHT);
	overlay.load(Assets::menuOverlayPaths.at(menu), Engine::getRenderer());
	ButtonType buttonPressed = ButtonType::none;

	int mouseX, mouseY;
	float elapsedFadeTime = 0.f;
	const float fadeOutTime = 0.33f;
	float fadeOutProgress = 0;
	while (buttonPressed == ButtonType::none || elapsedFadeTime < fadeOutTime)
	{
		const float deltaTime = Engine::updateTicks();

		if (buttonPressed == ButtonType::none)
		{
			SDL_GetMouseState(&mouseX, &mouseY);
			if (isPressingMouseButton())
				buttonPressed = buttons.isIntersectingAnyButton(mouseX, mouseY);
		}
		else
		{
			elapsedFadeTime += deltaTime;
			fadeOutProgress = clamp(elapsedFadeTime / (fadeOutTime+0.05f), 0.f, 0.99f);
			overlay.updateOpacity(pow(1.f - fadeOutProgress,2));
		}

		Engine::renderClear();
		Engine::drawBackground();
		musicManager.update(deltaTime);
		Engine::drawObjects();
		Engine::rotateObjects();
		Engine::drawHealthLine(musicManager.data);
		overlay.draw(0, 0, WIDTH, HEIGHT, Engine::getRenderer());

		if (fadeOutProgress == 0)
			buttons.draw(getRenderer(), mouseX, mouseY);
		else 
			buttons.draw(getRenderer(), mouseX, mouseY, buttonPressed, 1 - fadeOutProgress);

		crosshair.update();
		crosshair.draw(Engine::getRenderer());
		Engine::renderPresent();
		SDL_Delay(16);
	}
	overlay.destroy();
	buttons.unloadTextures();
	return buttonPressed;
}

