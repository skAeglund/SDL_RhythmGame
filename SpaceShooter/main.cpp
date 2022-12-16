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
#include "game.h"
#include "waveManager.h"
using namespace std;

bool pauseLoop(Button& buttonPressed, Sprite& background, Crosshair& crosshair, const char* path);
float spawnTimer = 0.f;
int main(int argc, char** args) {

	Player player{ };
	initializeEngine(0);
	bool gameRunning = true;

	Position playerPosition(Position(WIDTH / 2 - player.radius / 2, HEIGHT * 0.8f, 45));
	createObject(playerPosition, Rotation(10,0), Velocity(0, 0),  20, "Content/Sprites/player90x90.png", Tag::Player);
	Crosshair crosshair(25, 25, "Content/Sprites/crosshair.png", getRenderer());
	Sprite background(WIDTH, HEIGHT);
	background.load("Content/Sprites/background.png", getRenderer());
	SDL_SetTextureBlendMode(background.texture, SDL_BlendMode::SDL_BLENDMODE_BLEND);

	Beat beats[NUMBER_OF_BEATS] = {
		/*Beat(114,4, "Content/Audio/Intro_Loop.wav"),*/
		Beat(114, 4,  "Content/Audio/Sunset_Loop_16.wav"),
		Beat(120, 4, "Content/Audio/Jupiter_Loop.wav"),
		Beat(133, 4, "Content/Audio/RunningInTheNight_Loop.wav")
	};
	MusicManager musicManager;

	musicManager.initialize(beats);
	Button buttonPressed = Button::none;
	
	//std::cout << "current path: " << filesystem::current_path();

	WaveManager waveManager;
	
	gameRunning = pauseLoop(buttonPressed, background, crosshair, "Content/Sprites/IntroScreen_Temp.png");
	bool gamePaused = false;
	musicManager.startPlaying();
	waveManager.start();
	//spawnAsteroids(1);
	while (gameRunning)
	{
		double deltaTime = updateTicks();
		bool musicPlaying = musicManager.update(deltaTime);
		printTimeStats();
		musicManager.printStats();

		renderClear();
		background.draw(0, 0, WIDTH, HEIGHT, getRenderer());
		drawLines(SDL_Color(0, 200, 255, 255));
		sortObjects();
		moveObjects();
		rotateObjects();
		drawObjects();
		drawHealthLine();

		drawBeatCircles();
		//drawColliders();
		
		player.update(deltaTime);
		//player.draw();
		crosshair.update();
		crosshair.draw(getRenderer());

		handleInputEvents(player, gameRunning, gamePaused, deltaTime);
		renderPresent();
		if (gamePaused)
		{
			waveManager.pause();
			gameRunning = pauseLoop(buttonPressed, background, crosshair, "Content/Sprites/PauseScreen_temp.png");
			gamePaused = false;
			if (gameRunning)
			{
				waveManager.start();
			}
		}
		else if (Player::remainingHealth <= 0)
		{
			waveManager.pause();
			gameRunning = pauseLoop(buttonPressed, background, crosshair, "Content/Sprites/GameOver_Temp.png");
			if (gameRunning)
			{
				Player::remainingHealth = player.maxHealth;
				musicManager.changeBeat(0);
				waveManager.restart();
				//waveManager.start();
			}
			resetKeys();
		}
		delayNextFrame();
	}

	musicManager.destroy();
	background.destroy();
	crosshair.destroy();
	cout << "Quitting ..." << endl;
	quit();

	return 0;
}
bool pauseLoop(Button& buttonPressed, Sprite& background, Crosshair& crosshair, const char* path)
{
	Sprite overlay(WIDTH, HEIGHT);
	overlay.load(path, getRenderer());
	buttonPressed = Button::none;
	while (buttonPressed == Button::none)
	{
		buttonPressed = pausedInputHandler();
		renderClear();
		float deltaTime = updateTicks();

		background.draw(0, 0, WIDTH, HEIGHT, getRenderer());
		getMusicManager()->update(deltaTime);
		drawObjects();
		rotateObjects();
		drawHealthLine();
		overlay.draw(0, 0, WIDTH, HEIGHT, getRenderer());
		crosshair.update();
		crosshair.draw(getRenderer());
		renderPresent();
		SDL_Delay(16);
	}
	overlay.destroy();
	return buttonPressed != Button::quit;
}