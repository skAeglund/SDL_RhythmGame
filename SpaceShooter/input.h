#pragma once
#include "engine.h"
#include "player.h"
#include "UI.h"

using namespace Collision;

void handleInputEvents(Player& player, MusicManager& musicManager, bool& gameRunning, bool& gamePaused, float deltaTime);

Button handlePausedInput();

bool isPressingMouseButton();

enum InputKey {
	moveUp = SDL_SCANCODE_W,
	moveDown = SDL_SCANCODE_S,
	moveLeft = SDL_SCANCODE_A,
	moveRight = SDL_SCANCODE_D,
	shoot = SDL_BUTTON_RIGHT,
	laser = SDL_BUTTON_LEFT,
	spawnAsteroid = SDL_SCANCODE_Q,
	pauseGame = SDL_SCANCODE_ESCAPE,
	changeBeat = SDL_SCANCODE_I
};