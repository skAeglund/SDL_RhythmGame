// SpaceShooter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <SDL.h>
#include "engine.h"
#include "player.h"
#include "sprite.h"
#include "crosshair.h"
#include "input.h"
#include "musicManager.h"
#include "waveManager.h"

using namespace std;
using namespace Engine;
using namespace Collision;

bool pauseLoop(Button& buttonPressed, MusicManager& musicManager, Crosshair& crosshair, const char* path);
float spawnTimer = 0.f;
int main(int argc, char** args) {

	const char* textures[] = {
		"Content/Sprites/background.png",
		"Content/Sprites/meteor1_blue.png",
		"Content/Sprites/meteor2.png",
		"Content/Sprites/meteor3.png",
		"Content/Sprites/meteor4.png"
	};
	Beat beats[NUMBER_OF_BEATS] = {
		Beat(114, 4,  "Content/Audio/Sunset_Loop_16.wav"),
		Beat(120, 4, "Content/Audio/Jupiter_Loop.wav"),
		Beat(133, 4, "Content/Audio/RunningInTheNight_Loop.wav")
	};

	Engine::initializeEngine(textures, sizeof(textures) / sizeof(textures[0]));
	
	Player player{ };
	Position playerPosition(Position(WIDTH / 2 - player.radius / 2, HEIGHT * 0.8f, 45));
	Engine::createObject(playerPosition, Rotation(10,0), Velocity(0, 0),  20, "Content/Sprites/player90x90.png", Tag::Player);
	
	Crosshair crosshair(25, 25, "Content/Sprites/crosshair.png", Engine::getRenderer());

	MusicManager musicManager;
	musicManager.initialize(beats);

	WaveManager waveManager;
	waveManager.initialize(&musicManager);

	Button buttonPressed = Button::none;
	bool gameRunning = pauseLoop(buttonPressed, musicManager, crosshair, "Content/Sprites/IntroScreen_Temp.png");
	bool gamePaused = false;

	musicManager.startPlaying();
	waveManager.start();

	while (gameRunning)
	{
		double deltaTime = Engine::updateTicks();
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

		int playerDamage = Engine::checkForObjectDestruction(musicManager);

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
			waveManager.pause();
			gameRunning = pauseLoop(buttonPressed, musicManager, crosshair, "Content/Sprites/PauseScreen_temp.png");
			gamePaused = false;
			if (gameRunning)
			{
				waveManager.start();
			}
		}
		else if (player.remainingHealth <= 0)
		{
			waveManager.pause();
			gameRunning = pauseLoop(buttonPressed, musicManager, crosshair, "Content/Sprites/GameOver_Temp.png");
			if (gameRunning)
			{
				player.remainingHealth = player.maxHealth;
				musicManager.changeBeat(0);
				waveManager.restart();
			}
			Engine::resetKeys();
		}

		Engine::renderPresent();
		Engine::delayNextFrame();
	}

	musicManager.unload();
	crosshair.destroy();
	cout << "Quitting ..." << endl;
	Engine::quit();

	return 0;
}
bool pauseLoop(Button& buttonPressed, MusicManager& musicManager, Crosshair& crosshair, const char* path)
{
	Sprite overlay(WIDTH, HEIGHT);
	overlay.load(path, Engine::getRenderer());
	buttonPressed = Button::none;
	while (buttonPressed == Button::none)
	{
		buttonPressed = pausedInputHandler();
		Engine::renderClear();
		float deltaTime = Engine::updateTicks();
		
		Engine::drawBackground();
		musicManager.update(deltaTime);
		Engine::drawObjects();
		Engine::rotateObjects();
		Engine::drawHealthLine(musicManager.data);
		overlay.draw(0, 0, WIDTH, HEIGHT, Engine::getRenderer());
		crosshair.update();
		crosshair.draw(Engine::getRenderer());
		Engine::renderPresent();
		SDL_Delay(16);
	}
	overlay.destroy();
	return buttonPressed != Button::quit;

}