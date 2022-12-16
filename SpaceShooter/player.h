#pragma once
#include "engine.h"
#include "vector2.h"
#include "sprite.h"

struct Player
{
	Vector2 velocity{ 0.f,0.f };
	float radius = 45;
	int maxHealth = 12;
	inline static int remainingHealth;

	SDL_Texture* forceBallTexture;

	Player();
	void update(float deltaTime);
	void shoot(int mouseX, int mouseY);
	void laser(int mouseX, int mouseY);
	void reset();
};