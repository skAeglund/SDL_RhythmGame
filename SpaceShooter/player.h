#pragma once
#include "engine.h"
#include "vector2.h"
#include "musicManager.h"
#define HEALTH_LINE_HEIGHT HEIGHT - 100

using namespace Vector2D;
/// <summary>
/// This only handles player velocity, health and lasers
/// The movement, rendering & collision is handled by engine.cpp
/// </summary>
struct Player
{
	Vector2 velocity{ 0.f,0.f };
	float radius = 45;
	int maxHealth = 12;
	// this is static so that it can be used by a static function (see wavemanager)
	inline static int remainingHealth;

	//SDL_Texture* forceBallTexture;

	Player();
	void update(float deltaTime);
	void shootBall(int mouseX, int mouseY);
	void shootLaser(int mouseX, int mouseY, MusicData* musicData);
	void reset();
};

