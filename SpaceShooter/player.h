#pragma once
#include "engine.h"
#include "vector2.h"
#include "musicManager.h"
#include "sprite.h"
#include "UI.h"

#if HEIGHT == 1080
	constexpr float playerRadius = 45;
	#define HEALTH_LINE_HEIGHT HEIGHT - 100
#else // 720
	constexpr float playerRadius = 40;
	#define HEALTH_LINE_HEIGHT HEIGHT - 50
#endif


using namespace Vector2D;
/// <summary>
/// This only handles player velocity, health and lasers
/// The movement, rendering & collision is handled by engine.cpp
/// </summary>
struct Player
{
	Vector2 velocity{ 0.f,0.f };
	float radius = playerRadius;
	int maxHealth = 12;
	int remainingHealth;
	float timeSinceLastFail = 2.f;
	float timeSinceLastSuccess = 2.f;
	MusicManager* musicManager;
	Sprite overlay;

	Player(MusicManager* musicManager, SDL_Renderer* renderer);
	void update(float deltaTime, float pulseMultiplier);
	void shootLaser(int mouseX, int mouseY, MusicData* musicData, bool& wasShotSuccessful);
	void reset();
	void takeDamage(int damage);
	void destroy();
};

